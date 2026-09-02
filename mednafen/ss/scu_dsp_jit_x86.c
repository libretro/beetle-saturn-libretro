/******************************************************************************/
/* Mednafen Sega Saturn Emulation Module                                      */
/******************************************************************************/
/* scu_dsp_jit_x86.c - SCU DSP JIT, x86 / x86-64 backend
**
** Same contract as the aarch64 backend in scu_dsp_jit.c: one code slot
** per (pc, looped) program-RAM entry, the slot's address stored in
** ProgRAM[].low32 / NextInstr.low32 as an offset from DSP_Init, C-side
** decodes routed through a permanent fallback thunk, LPS handled by a
** helper that compiles the looped variant lazily, and the segment
** rewound (every slot recompiled) when it fills, deferred while a chain
** is live.
**
** What differs is the execution model.  The aarch64 backend keeps the
** hot DSP state in eleven pinned registers, and every crossing into C
** (thunk, DMA fallback, helper call, cold entry) is a flush-or-reload
** protocol.  On x86 every DSPS field is one [EBX + disp32] operand away,
** so this backend keeps the DSP state memory-resident: the C handlers,
** the helpers and the JIT slots all see the same current memory at all
** times, nothing is flushed or reloaded anywhere, and the identical code
** serves x86-32 with its six spare registers.  What the JIT keeps is the
** substance -- one specialised straight-line body per instruction with
** every field folded, no table dispatch, no generic-handler branching
** and no C call/return per instruction.  Register pinning on x86-64 is a
** separate, measurable optimisation on top of this base.
**
** Slot layout: 16 bytes of cold prelude (MOV RAX, body ; JMP prelude
** stub) for C callers, then the body at SCU_JIT_SLOT_PRELUDE_BYTES for
** JIT tail dispatches.  A C caller (DSP_TailDispatch in a handler, or
** the entry stub) enters at offset 0 as a normal function; the prelude
** stub saves callee-saved registers, loads EBX, CALLs the body, and
** returns when the chain RETs.  Slots reach each other by JMP; a chain
** ends in the shared dispatch stub's RET, which pops the prelude's
** return address.  The frame therefore nests naturally through
** C -> JIT -> C -> JIT, bounded by the cycle budget.
**
** Frame (established by the prelude stub, seen by every body):
**   x86-64: push rbx,r12,r13,r14 ; sub rsp,24 ; mov rbx,arg ; load pins ; call body
**           body: rsp = 8 mod 16, scratch qwords at [rsp+8], [rsp+16]
**   x86-32: push ebx,esi,edi,ebp ; sub esp,28 ; mov ebx,[esp+48] ; call body
**           body: esp = 12 mod 16, [esp+4] = outgoing C argument slot,
**           scratch dwords at [esp+8 .. esp+31]
** Helper calls from a body re-align to 16 with the sub/push shown in
** emit_call_helper; a tail-jump into a C handler leaves the frame exactly
** as a fresh call would (argument in RDI/RCX or at [esp+4]).
**
** Semantics are transcribed statement by statement from the C handlers
** in scu_dsp_gen.c / scu_dsp_mvi.c / scu_dsp_jmp.c / scu_dsp_misc.c;
** tests/scu_dsp_difftest.c is the oracle.
*/

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include "ss.h"
#include "scu.h"
#include "scu_dsp_jit.h"
#include "x86emit.h"
#include "jitdump.h"

#if defined(WANT_JIT) && X86EMIT_HOST

#include "scu_dsp_common.inc"

#define SCU_JIT_CODE_SEGMENT_SIZE  ((size_t)0x100000)
#define SCU_JIT_SLOT_MAX_BYTES     ((size_t)1024)
#define SCU_JIT_SLOT_PRELUDE_BYTES 16u

#define O(field) ((int32_t)offsetof(struct DSPS, field))
#define M_D(disp)                 X86_EBX, X86_NOIDX, 0, (disp)
#define M_DIDX(idx, scale, disp)  X86_EBX, (idx), (scale), (disp)

#if X86EMIT_64
 #define SCR_DISP 8
 #define M_SCR(i) X86_ESP, X86_NOIDX, 0, (SCR_DISP + 4 * (i))
 #if defined(_WIN32) || defined(SCU_JIT_X86_MSABI)
  #define ARG0 X86_ECX
  #define ARG1 X86_EDX
  #define CALL_SHADOW 40
 #else
  #define ARG0 X86_EDI
  #define ARG1 X86_ESI
  #define CALL_SHADOW 8
 #endif
#else
 #define SCR_DISP 8
 #define M_SCR(i) X86_ESP, X86_NOIDX, 0, (SCR_DISP + 4 * (i))
 #define M_CARG   X86_ESP, X86_NOIDX, 0, 4
#endif

enum { EAX = X86_EAX, ECX = X86_ECX, EDX = X86_EDX, EBX = X86_EBX,
       ESP = X86_ESP, EBP = X86_EBP, ESI = X86_ESI, EDI = X86_EDI };

/* Scratch registers beyond EAX/ECX/EDX: R8/R9 on x86-64 (caller-saved
 * under both SysV and Win64), ESI/EDI on x86-32 (saved by the prelude). */
#if X86EMIT_64
 #define S0 X86_R8
 #define S1 X86_R9
#else
 #define S0 X86_ESI
 #define S1 X86_EDI
#endif

/* Byte-sized DSPS fields. */
#define O_PC      O(PC)
#define O_TOP     O(TOP)
#define O_LOP     O(LOP)
#define O_FZ      O(FlagZ)
#define O_FS      O(FlagS)
#define O_FV      O(FlagV)
#define O_FC      O(FlagC)
#define O_NIL     O(NextInstrLooped)
#define O_CT(n)   (O(CT32) + (int32_t)(n))
#define O_DRAM(n) (O(DataRAM) + (int32_t)(n) * 256)

static x86_codegen* g_cg = NULL;
static void*  g_seg_start = NULL;
static size_t g_post_stub_byte_offset = 0;
static const void* g_dispatch_stub = NULL;
static const void* g_prelude_stub  = NULL;
static const void* g_fallback_thunk[2] = { NULL, NULL };
static void (*g_entry_stub)(struct DSPS*) = NULL;

static bool g_chain_live     = false;
static bool g_rewind_pending = false;
static bool g_in_rewind      = false;
static bool g_in_rewind_slots = false;
static const void* g_exit_stub = NULL;        /* flush pins ; ret */
static const void* g_dispatch_nijump = NULL;  /* dispatch stub past its CC/State checks */

/* --- Block linking -----------------------------------------------------
 * A slot entered with PC == pc+1 has just fetched NI = ProgRAM[pc+1], so
 * if it cannot replace NI itself (is_linkable_slot) its tail can JMP
 * straight to slot pc+1's hot entry instead of dispatching on NI.  The
 * static filter (predecessor cannot perturb PC) picks the slots where
 * that is the common case; a runtime guard at the hot entry (PC == pc+1,
 * else bail to the fallback thunk, which flushes and runs the same
 * instruction through its C handler) makes it correct for the rest: a
 * branch executing in another branch's delay slot lands here with PC
 * elsewhere, and BTM's TOP is dynamic.  Successor addresses are known
 * only once every slot is compiled and move on each rewind, so linked
 * tails emit a placeholder JMP recorded here for rewind_locked's second
 * pass.  Any compile while a chain is live re-points every site at the
 * dispatch stub's NI jump (unlink_all), because a PRAM DMA finishing
 * mid-chain may have replaced a linked successor. */
static void*   g_link_site[256];
static bool    g_link_this_slot = false;
static uint8_t g_link_this_pc   = 0;

static bool may_perturb_pc(uint32_t instr)
{
 const unsigned top = (instr >> 28) & 0xF;
 if(top <= 0x3) return false;                 /* gen */
 if(top >= 0x8 && top <= 0xB)                 /* MVI: only dest 6/7/C move PC */
 {
  const unsigned dest = (instr >> 26) & 0xF;
  return dest == 0x6 || dest == 0x7 || dest == 0xC;
 }
 return true;                                 /* JMP / DMA / MISC / reserved */
}

static bool is_linkable_slot(uint32_t instr)
{
 const unsigned top = (instr >> 28) & 0xF;
 if(top <= 0x3)               return true;    /* gen */
 if(top == 0xD)               return true;    /* JMP (delayed branch) */
 if(top == 0xE || top == 0xF) return ((instr >> 27) & 0x3) == 0x0;   /* BTM only */
 if(top >= 0x8 && top <= 0xB)
 {
  const unsigned dest = (instr >> 26) & 0xF;
  return !(dest == 0x6 || dest == 0x7 || dest == 0xC);
 }
 return false;
}

static void unlink_all(void)
{
 unsigned i;
 for(i = 0; i < 256; i++)
  if(g_link_site[i])
  {
   x86_patch_jmp_abs(g_link_site[i], g_dispatch_nijump);
   g_link_site[i] = NULL;
  }
}

typedef struct {
 void (*entry)(struct DSPS*);
 uint32_t instr;
} LoopedSlot;
static LoopedSlot g_looped_cache[256];

/* --- register pins ---------------------------------------------------------
 * x86-64: R12 = NextInstr (64), R13D = PC (0..255), R14D = CycleCounter.
 * x86-32: EBP = CycleCounter only.
 * Every pin is callee-saved under cdecl, SysV and Win64, so C helpers
 * preserve the register; what a helper may change is the memory copy,
 * hence flush before / reload after every crossing into C, and a load
 * in the prelude for every entry from C.  Measured on the transform
 * kernel in tests/scu_dsp_difftest.c --bench: the three x86-64 pins take
 * the JIT from 1.22x to 1.53x over the interpreter, block linking to
 * 2.0x; on x86-32 the CC pin is worth 13%, linking nothing (the indirect
 * jump was already well predicted), and pinning PC as well would cost
 * the 48-bit add its scratch registers for a few percent at best. */
#if X86EMIT_64
 #define PIN_NI X86_R12
 #define PIN_PC X86_R13
 #define PIN_CC X86_R14
#else
 #define PIN_CC X86_EBP     /* x86-32: CycleCounter only; EBP is saved by the prelude */
#endif

static void emit_flush_pins(void)
{
#ifdef PIN_NI
 x86_mov_mr64(g_cg, X86_EBX, X86_NOIDX, 0, (int32_t)offsetof(struct DSPS, NextInstr), PIN_NI);
 x86_mov_m8r8(g_cg, X86_EBX, X86_NOIDX, 0, (int32_t)offsetof(struct DSPS, PC), PIN_PC);
#endif
#ifdef PIN_CC
 x86_mov_mr  (g_cg, X86_EBX, X86_NOIDX, 0, (int32_t)offsetof(struct DSPS, CycleCounter), PIN_CC);
#endif
}

static void emit_load_pins(void)
{
#ifdef PIN_NI
 x86_mov_rm64 (g_cg, PIN_NI, X86_EBX, X86_NOIDX, 0, (int32_t)offsetof(struct DSPS, NextInstr));
 x86_movzx_rm8(g_cg, PIN_PC, X86_EBX, X86_NOIDX, 0, (int32_t)offsetof(struct DSPS, PC));
#endif
#ifdef PIN_CC
 x86_mov_rm   (g_cg, PIN_CC, X86_EBX, X86_NOIDX, 0, (int32_t)offsetof(struct DSPS, CycleCounter));
#endif
}


/* --- labels ------------------------------------------------------------ */

#define LABEL_POOL 64
static x86_label g_labels[LABEL_POOL];
static unsigned  g_label_count;

static x86_label* label_new(void)
{
 x86_label* l = &g_labels[g_label_count < LABEL_POOL ? g_label_count : LABEL_POOL - 1];
 if(g_label_count < LABEL_POOL) g_label_count++;
 x86_label_init(l);
 return l;
}
static void labels_reset(void) { g_label_count = 0; }

/* --- C-handler selection ------------------------------------------------ */

static void (*pick_c_handler(bool looped, uint32_t instr))(struct DSPS*)
{
 const unsigned li  = looped ? 1u : 0u;
 const unsigned top = (instr >> 28) & 0xF;

 switch(top)
 {
  case 0x0: case 0x1: case 0x2: case 0x3:
   return DSP_GenFuncTable[li][(instr >> 26) & 0xF][(instr >> 23) & 0x7][(instr >> 17) & 0x7][(instr >> 12) & 0x3];
  case 0x8: case 0x9: case 0xA: case 0xB:
   return DSP_MVIFuncTable[li][(instr >> 26) & 0xF][(instr >> 19) & 0x7F];
  case 0xC:
   return DSP_DMAFuncTable[li][(instr >> 12) & 0x7][(instr >> 8) & 0x7];
  case 0xD:
   return DSP_JMPFuncTable[li][(instr >> 19) & 0x7F];
  case 0xE: case 0xF:
   return DSP_MiscFuncTable[li][(instr >> 27) & 0x3];
  default:
   return DSP_GenFuncTable[li][0][0][0][0];
 }
}

static bool is_general_instr(uint32_t instr) { return ((instr >> 28) & 0xF) <= 0x3; }
static bool is_mvi_instr(uint32_t instr)     { const unsigned t = (instr >> 28) & 0xF; return t >= 0x8 && t <= 0xB; }
static bool is_jmp_instr(uint32_t instr)     { return ((instr >> 28) & 0xF) == 0xD; }
static bool is_misc_instr(uint32_t instr)    { const unsigned t = (instr >> 28) & 0xF; return t == 0xE || t == 0xF; }

/* --- C helpers reached from slot code ------------------------------------ */

static void fallback_shim_normal(struct DSPS* dsp)
{
 pick_c_handler(false, (uint32_t)(dsp->NextInstr >> 32))(dsp);
}

static void fallback_shim_looped(struct DSPS* dsp)
{
 pick_c_handler(true, (uint32_t)(dsp->NextInstr >> 32))(dsp);
}

/* LPS: re-decode NextInstr as the looped variant, through the per-pc
 * looped-slot cache.  Mirrors the aarch64 helper. */
static void lps_helper(struct DSPS* dsp)
{
 const uint32_t instr = dsp->NextInstr >> 32;
 const uint8_t  pc    = (dsp->PC - 1) & 0xFFu;
 LoopedSlot* cached = &g_looped_cache[pc];

 if(MDFN_UNLIKELY(!cached->entry || cached->instr != instr))
 {
  void (*entry)(struct DSPS*) = SCU_DSP_JIT_CompileSlot(pc, true, instr);
  if(MDFN_UNLIKELY(!entry))
  {
   dsp->NextInstr = DSP_DecodeInstruction(instr, true);
   dsp->NextInstrLooped = true;
   return;
  }
  cached->entry = entry;
  cached->instr = instr;
 }

 dsp->NextInstr = ((uint64_t)instr << 32)
                | (uint32_t)((uintptr_t)cached->entry - DSP_INSTR_BASE_UIPT);
 dsp->NextInstrLooped = true;
}

/* END / ENDI, transcribed from MiscInstr_BODY. */
static void misc_end_helper(struct DSPS* dsp, uint32_t is_endi)
{
 if(is_endi)
 {
  dsp->FlagEnd = true;
  SCU_SetInt(SCU_INT_DSP, true);
 }
 if(dsp->PRAMDMABufCount)
  DSP_FinishPRAMDMA();
 else
 {
  dsp->State &= ~STATE_MASK_EXECUTE;
  dsp->CycleCounter -= DSP_EndCCSubVal;
 }
}

/* --- frame helpers -------------------------------------------------------- */

/* call helper(dsp[, arg1]) from a body, keeping the stack 16-aligned at
 * the CALL.  EBX survives (callee-saved everywhere); every other
 * register is clobbered. */
static void emit_call_helper(const void* fn, bool two_args, uint32_t arg1)
{
 emit_flush_pins();
#if X86EMIT_64
 x86_alu_ri64(g_cg, X86_SUB, ESP, CALL_SHADOW);
 x86_mov_rr64(g_cg, ARG0, EBX);
 if(two_args) x86_mov_ri(g_cg, ARG1, arg1);
 x86_call_abs(g_cg, fn);
 x86_alu_ri64(g_cg, X86_ADD, ESP, CALL_SHADOW);
 emit_load_pins();
#else
 if(two_args)
 {
  x86_alu_ri(g_cg, X86_SUB, ESP, 4);
  x86_mov_ri(g_cg, EAX, arg1);
  x86_push(g_cg, EAX);
 }
 else
  x86_alu_ri(g_cg, X86_SUB, ESP, 8);
 x86_push(g_cg, EBX);
 x86_call_abs(g_cg, fn);
 x86_alu_ri(g_cg, X86_ADD, ESP, 12);
 emit_load_pins();
#endif
}

/* call helper(void). */
static void emit_call_void(const void* fn)
{
 emit_flush_pins();
#if X86EMIT_64
 x86_alu_ri64(g_cg, X86_SUB, ESP, CALL_SHADOW);
 x86_call_abs(g_cg, fn);
 x86_alu_ri64(g_cg, X86_ADD, ESP, CALL_SHADOW);
 emit_load_pins();
#else
 x86_alu_ri(g_cg, X86_SUB, ESP, 12);
 x86_call_abs(g_cg, fn);
 x86_alu_ri(g_cg, X86_ADD, ESP, 12);
 emit_load_pins();
#endif
}

/* Tail-jump into a C function taking (struct DSPS*): the C code runs
 * with exactly the frame a fresh call would give it, and its RET pops
 * the prelude's return address. */
static void emit_tailjump_c(const void* fn)
{
 emit_flush_pins();
#if X86EMIT_64
 x86_mov_rr64(g_cg, ARG0, EBX);
 x86_mov_ri64(g_cg, EAX, (uint64_t)(uintptr_t)fn);
 x86_jmp_r(g_cg, EAX);
#else
 x86_mov_mr(g_cg, M_CARG, EBX);
 x86_jmp_abs(g_cg, fn);
#endif
}

/* --- cold prelude -------------------------------------------------------- */

/* 16 bytes at every slot start: MOV EAX/RAX, body ; JMP prelude_stub ; pad. */
static void emit_cold_entry(void)
{
 uint8_t* start = (uint8_t*)x86_codegen_wptr(g_cg);
 const void* body = start + SCU_JIT_SLOT_PRELUDE_BYTES;
 /* Fixed-length MOV so the hot entry is always at +16. */
 x86_movabs(g_cg, EAX, (uint64_t)(uintptr_t)body);
 x86_jmp_abs(g_cg, g_prelude_stub);
 while((uint8_t*)x86_codegen_wptr(g_cg) < start + SCU_JIT_SLOT_PRELUDE_BYTES)
  x86_int3(g_cg);
}

/* The prelude stub: a C-callable function whose body address arrives in
 * EAX/RAX.  Also the shape of the entry stub (body = entry dispatch). */
static void emit_prelude_stub(void)
{
#if X86EMIT_64
 /* entry rsp = 8 mod 16; four pushes keep it at 8; sub 24 -> 0; the CALL
  * leaves the body at 8 with 24 bytes of scratch above its return slot. */
 x86_push(g_cg, EBX);
 x86_push(g_cg, PIN_NI);
 x86_push(g_cg, PIN_PC);
 x86_push(g_cg, PIN_CC);
 x86_alu_ri64(g_cg, X86_SUB, ESP, 24);
 x86_mov_rr64(g_cg, EBX, ARG0);
 emit_load_pins();
 x86_call_r(g_cg, EAX);
 x86_alu_ri64(g_cg, X86_ADD, ESP, 24);
 x86_pop(g_cg, PIN_CC);
 x86_pop(g_cg, PIN_PC);
 x86_pop(g_cg, PIN_NI);
 x86_pop(g_cg, EBX);
 x86_ret(g_cg);
#else
 x86_push(g_cg, EBX);
 x86_push(g_cg, ESI);
 x86_push(g_cg, EDI);
 x86_push(g_cg, EBP);
 x86_alu_ri(g_cg, X86_SUB, ESP, 28);
 x86_mov_rm(g_cg, EBX, X86_ESP, X86_NOIDX, 0, 48);
 emit_load_pins();
 x86_call_r(g_cg, EAX);
 x86_alu_ri(g_cg, X86_ADD, ESP, 28);
 x86_pop(g_cg, EBP);
 x86_pop(g_cg, EDI);
 x86_pop(g_cg, ESI);
 x86_pop(g_cg, EBX);
 x86_ret(g_cg);
#endif
}

/* Jump to DSP_INSTR_BASE + sext(NextInstr.low32) + PRELUDE (hot entry). */
static void emit_ni_jump(void)
{
#if X86EMIT_64
 x86_movsxd(g_cg, EAX, PIN_NI);
 x86_lea_rip(g_cg, ECX, (const void*)(DSP_INSTR_BASE_UIPT + SCU_JIT_SLOT_PRELUDE_BYTES));
 x86_alu_rr64(g_cg, X86_ADD, EAX, ECX);
#else
 x86_mov_rm(g_cg, EAX, M_D(O(NextInstr)));          /* low32 on LSB-first hosts */
 x86_alu_ri(g_cg, X86_ADD, EAX, (int32_t)(DSP_INSTR_BASE_UIPT + SCU_JIT_SLOT_PRELUDE_BYTES));
#endif
 x86_jmp_r(g_cg, EAX);
}

/* CycleCounter -= 2 ; jle out ; State <= 0 ; jle out */
static void emit_cc_state_check(x86_label* out)
{
#ifdef PIN_CC
 x86_alu_ri(g_cg, X86_SUB, PIN_CC, 2);
#else
 x86_alu_mi32(g_cg, X86_SUB, M_D(O(CycleCounter)), 2);
#endif
 x86_jcc(g_cg, X86_CC_LE, out);
 x86_cmp_mi32(g_cg, M_D(O(State)), 0);
 x86_jcc(g_cg, X86_CC_LE, out);
}

/* Shared tail: CycleCounter -= 2; stop if <= 0 or !running; else NI jump. */
static void emit_dispatch_stub(void)
{
 x86_label* out = label_new();
 emit_cc_state_check(out);
 g_dispatch_nijump = x86_codegen_wptr(g_cg);
 emit_ni_jump();
 x86_label_bind(g_cg, out);
 g_exit_stub = x86_codegen_wptr(g_cg);
 emit_flush_pins();
 x86_ret(g_cg);
}

/* Entry from SCU_UpdateDSP: no cycle charge before the first instruction. */
static void emit_entry_dispatch(void)
{
 emit_ni_jump();
}

static void emit_tail_dispatch(void)
{
 if(g_link_this_slot)
 {
#ifdef PIN_CC
  x86_alu_ri(g_cg, X86_SUB, PIN_CC, 2);
#else
  x86_alu_mi32(g_cg, X86_SUB, M_D(O(CycleCounter)), 2);
#endif
  x86_jcc_abs(g_cg, X86_CC_LE, g_exit_stub);
  x86_cmp_mi32(g_cg, M_D(O(State)), 0);
  x86_jcc_abs(g_cg, X86_CC_LE, g_exit_stub);
  g_link_site[g_link_this_pc] = x86_codegen_wptr(g_cg);
  x86_jmp_abs(g_cg, g_dispatch_nijump);      /* placeholder; patched by rewind pass 2 */
  return;
 }
 x86_jmp_abs(g_cg, g_dispatch_stub);
}

/* Linear-entry guard at the hot entry of a linked slot. */
static void emit_link_guard(void)
{
 if(!g_link_this_slot) return;
#if X86EMIT_64
 x86_alu_ri(g_cg, X86_CMP, PIN_PC, (int32_t)((g_link_this_pc + 1u) & 0xFFu));
#else
 x86_cmp_mi8(g_cg, M_D(O_PC), (uint8_t)(g_link_this_pc + 1u));
#endif
 x86_jcc_abs(g_cg, X86_CC_NE, (const char*)g_fallback_thunk[0] + SCU_JIT_SLOT_PRELUDE_BYTES);
}

/* Fallback thunk: slot-shaped, hot entry tail-jumps into the shim that
 * re-dispatches on NextInstr.high32. */
static void emit_fallback_thunk(void (*shim)(struct DSPS*))
{
 emit_cold_entry();
 emit_tailjump_c((const void*)shim);
}

/* --- instruction pieces --------------------------------------------------- */

/* DSP_InstrPre. */
static void emit_instr_pre(bool looped)
{
 x86_label* skip = NULL;
 if(looped)
 {
  skip = label_new();
  x86_movzx_rm16(g_cg, EAX, M_D(O_LOP));
  x86_test_rr(g_cg, EAX, EAX);
  x86_jcc(g_cg, X86_CC_NE, skip);
 }
#if X86EMIT_64
 x86_mov_rm64(g_cg, PIN_NI, M_DIDX(PIN_PC, 3, O(ProgRAM)));
 x86_alu_ri(g_cg, X86_ADD, PIN_PC, 1);
 x86_alu_ri(g_cg, X86_AND, PIN_PC, 0xFF);
#else
 x86_movzx_rm8(g_cg, ECX, M_D(O_PC));
 x86_mov_rm(g_cg, EDX, M_DIDX(ECX, 3, O(ProgRAM)));
 x86_mov_mr(g_cg, M_D(O(NextInstr)), EDX);
 x86_mov_rm(g_cg, EDX, M_DIDX(ECX, 3, O(ProgRAM) + 4));
 x86_mov_mr(g_cg, M_D(O(NextInstr) + 4), EDX);
 x86_inc_m8(g_cg, M_D(O_PC));
#endif
 if(looped)
 {
  x86_mov_mi8(g_cg, M_D(O_NIL), 0);
  x86_label_bind(g_cg, skip);
  x86_alu_ri(g_cg, X86_SUB, EAX, 1);
  x86_alu_ri(g_cg, X86_AND, EAX, 0xFFF);
  x86_mov_mr16(g_cg, M_D(O_LOP), EAX);
 }
}

/* DSP_TestCond: falls through when true, jumps to `skip` when false. */
static void emit_test_cond(unsigned cond, x86_label* skip)
{
 bool have = false;
 if(!(cond & 0x40))
  return;
 if(!(cond & 0xF))
 {
  /* ret == false; result = (false == (cond & 0x20)) */
  if(cond & 0x20)
   x86_jmp(g_cg, skip);
  return;
 }
#define TERM(off) do { if(!have) { x86_movzx_rm8(g_cg, EAX, M_D(off)); have = true; } \
                       else { x86_movzx_rm8(g_cg, ECX, M_D(off)); x86_alu_rr(g_cg, X86_OR, EAX, ECX); } } while(0)
 if(cond & 0x1) TERM(O_FZ);
 if(cond & 0x2) TERM(O_FS);
 if(cond & 0x4) TERM(O_FC);
#undef TERM
 if(cond & 0x8)
 {
  /* (T0_Until < CycleCounter) */
#ifdef PIN_CC
  x86_cmp_mr(g_cg, M_D(O(T0_Until)), PIN_CC);
#else
  x86_mov_rm(g_cg, ECX, M_D(O(CycleCounter)));
  x86_cmp_mr(g_cg, M_D(O(T0_Until)), ECX);
#endif
  x86_setcc_r8(g_cg, X86_CC_L, ECX);
  x86_movzx_rr8(g_cg, ECX, ECX);
  if(!have) { x86_mov_rr(g_cg, EAX, ECX); have = true; }
  else       x86_alu_rr(g_cg, X86_OR, EAX, ECX);
 }
 x86_alu_ri(g_cg, X86_CMP, EAX, (int32_t)((cond >> 5) & 1));
 x86_jcc(g_cg, X86_CC_NE, skip);
}

/* EAX = DataRAM[bank][CT[bank]]; ECX = CT[bank]. */
static void emit_dram_load(unsigned bank)
{
 x86_movzx_rm8(g_cg, ECX, M_D(O_CT(bank)));
 x86_mov_rm(g_cg, EAX, M_DIDX(ECX, 2, O_DRAM(bank)));
}

/* P.T = (int32)EAX */
static void emit_store_p_sext(void)
{
#if X86EMIT_64
 x86_movsxd(g_cg, EAX, EAX);
 x86_mov_mr64(g_cg, M_D(O(P)), EAX);
#else
 x86_mov_mr(g_cg, M_D(O(P)), EAX);
 x86_mov_rr(g_cg, EDX, EAX);
 x86_shift_ri(g_cg, X86_SAR, EDX, 31);
 x86_mov_mr(g_cg, M_D(O(P) + 4), EDX);
#endif
}

/* AC.T = (int32)EAX */
static void emit_store_ac_sext(void)
{
#if X86EMIT_64
 x86_movsxd(g_cg, EAX, EAX);
 x86_mov_mr64(g_cg, M_D(O(AC)), EAX);
#else
 x86_mov_mr(g_cg, M_D(O(AC)), EAX);
 x86_mov_rr(g_cg, EDX, EAX);
 x86_shift_ri(g_cg, X86_SAR, EDX, 31);
 x86_mov_mr(g_cg, M_D(O(AC) + 4), EDX);
#endif
}

/* ALU.T lives in scratch: [SCR0] low32, [SCR1] high32 (one qword on x86-64). */
static void emit_alu_save_full(void)
{
#if X86EMIT_64
 x86_mov_mr64(g_cg, M_SCR(0), EAX);
#else
 x86_mov_mr(g_cg, M_SCR(0), EAX);
 x86_mov_mr(g_cg, M_SCR(1), S1);
#endif
}

/* CalcZS32 from EAX (clobbers flags). */
static void emit_zs32(void)
{
 x86_test_rr(g_cg, EAX, EAX);
 x86_setcc_m8(g_cg, X86_CC_E, M_D(O_FZ));
 x86_setcc_m8(g_cg, X86_CC_S, M_D(O_FS));
}

typedef struct { uint32_t dr_read; uint32_t ct_inc; } GenMeta;

/* Compile-time replica of the dr_read / ct_inc bookkeeping in
 * GeneralInstr_BODY. */
static GenMeta compute_meta(unsigned x_op, unsigned y_op, unsigned d1_op, uint32_t instr)
{
 GenMeta m; m.dr_read = 0; m.ct_inc = 0;
 if(x_op >= 0x3)
 {
  const unsigned s = (instr >> 20) & 0x7, drw = s & 3;
  m.dr_read |= 1u << drw;
  if(s & 4) m.ct_inc |= 1u << (drw * 8);
 }
 if(y_op >= 0x3)
 {
  const unsigned s = (instr >> 14) & 0x7, drw = s & 3;
  m.dr_read |= 1u << drw;
  if(s & 4) m.ct_inc |= 1u << (drw * 8);
 }
 if(d1_op & 0x1)
 {
  const unsigned d = (instr >> 8) & 0xF;
  if(d1_op & 0x2)
  {
   switch(instr & 0xF)
   {
    case 0x0: m.dr_read |= 0x01; break;
    case 0x1: m.dr_read |= 0x02; break;
    case 0x2: m.dr_read |= 0x04; break;
    case 0x3: m.dr_read |= 0x08; break;
    case 0x4: m.dr_read |= 0x01; if(d != 0) m.ct_inc |= 1u <<  0; break;
    case 0x5: m.dr_read |= 0x02; if(d != 1) m.ct_inc |= 1u <<  8; break;
    case 0x6: m.dr_read |= 0x04; if(d != 2) m.ct_inc |= 1u << 16; break;
    case 0x7: m.dr_read |= 0x08; if(d != 3) m.ct_inc |= 1u << 24; break;
    default: break;
   }
  }
  switch(d)
  {
   case 0x0: if(!(m.dr_read & 0x01)) m.ct_inc |= 1u <<  0; break;
   case 0x1: if(!(m.dr_read & 0x02)) m.ct_inc |= 1u <<  8; break;
   case 0x2: if(!(m.dr_read & 0x04)) m.ct_inc |= 1u << 16; break;
   case 0x3: if(!(m.dr_read & 0x08)) m.ct_inc |= 1u << 24; break;
   case 0xC: m.ct_inc &= ~0x000000FFu; break;
   case 0xD: m.ct_inc &= ~0x0000FF00u; break;
   case 0xE: m.ct_inc &= ~0x00FF0000u; break;
   case 0xF: m.ct_inc &= ~0xFF000000u; break;
   default: break;
  }
 }
 return m;
}

/* --- general instruction ------------------------------------------------- */

static void emit_gen(bool looped, uint32_t instr)
{
 const unsigned alu_op = (instr >> 26) & 0xF;
 const unsigned x_op   = (instr >> 23) & 0x7;
 const unsigned y_op   = (instr >> 17) & 0x7;
 const unsigned d1_op  = (instr >> 12) & 0x3;
 const GenMeta meta = compute_meta(x_op, y_op, d1_op, instr);
 /* ALU.T is consumed whole by Y op 2 and D1 source 0xA. */
 const bool need_alu_t = ((y_op & 3) == 2) || ((d1_op & 3) == 3 && (instr & 0xF) == 0xA);
 const bool need_alu_l = need_alu_t || ((d1_op & 3) == 3 && (instr & 0xF) == 0x9);

 emit_cold_entry();
 emit_link_guard();
 emit_instr_pre(looped);

 /* --- ALU: ALU = AC; op; leaves ALU.L in EAX, ALU.high32 in S1 (x86-32)
  *     or the full ALU.T in RAX (x86-64, for AD2) / merged below. --- */
 x86_mov_rm(g_cg, EAX, M_D(O(AC)));
 switch(alu_op)
 {
  case 0x01: case 0x02: case 0x03:
   x86_alu_rm(g_cg, alu_op == 1 ? X86_AND : alu_op == 2 ? X86_OR : X86_XOR, EAX, M_D(O(P)));
   x86_setcc_m8(g_cg, X86_CC_E, M_D(O_FZ));
   x86_setcc_m8(g_cg, X86_CC_S, M_D(O_FS));
   x86_mov_mi8(g_cg, M_D(O_FC), 0);
   break;
  case 0x04: case 0x05:
   x86_alu_rm(g_cg, alu_op == 4 ? X86_ADD : X86_SUB, EAX, M_D(O(P)));
   /* Read every flag before the OR into FlagV, which rewrites them. */
   x86_setcc_m8(g_cg, X86_CC_B, M_D(O_FC));
   x86_setcc_m8(g_cg, X86_CC_E, M_D(O_FZ));
   x86_setcc_m8(g_cg, X86_CC_S, M_D(O_FS));
   x86_setcc_r8(g_cg, X86_CC_O, ECX);
   x86_or_m8r8(g_cg, M_D(O_FV), ECX);
   break;
  case 0x06:
#if X86EMIT_64
   x86_mov_rm64(g_cg, EAX, M_D(O(AC)));
   x86_mov_rm64(g_cg, ECX, M_D(O(P)));
   x86_shift_ri64(g_cg, X86_SHL, EAX, 16);
   x86_shift_ri64(g_cg, X86_SHL, ECX, 16);
   x86_alu_rr64(g_cg, X86_ADD, EAX, ECX);
   x86_setcc_m8(g_cg, X86_CC_B, M_D(O_FC));
   x86_setcc_m8(g_cg, X86_CC_E, M_D(O_FZ));
   x86_setcc_m8(g_cg, X86_CC_S, M_D(O_FS));
   x86_setcc_r8(g_cg, X86_CC_B, ECX);
   x86_setcc_r8(g_cg, X86_CC_O, EDX);
   x86_or_m8r8(g_cg, M_D(O_FV), EDX);          /* flags dead from here */
   x86_movzx_rr8(g_cg, EDX, ECX);
   x86_shift_ri64(g_cg, X86_SHL, EDX, 48);
   x86_shift_ri64(g_cg, X86_SHR, EAX, 16);
   x86_alu_rr64(g_cg, X86_OR, EAX, EDX);      /* RAX = ALU.T */
#else
   /* 48-bit add as a 32-bit ADD plus a 16-bit ADC on the H halves: the
    * 16-bit ADC's CF/OF/SF/ZF are exactly the bit-47 semantics. */
   x86_mov_rm(g_cg, S1, M_D(O(AC) + 4));
   x86_mov_rm(g_cg, ECX, M_D(O(P)));
   x86_mov_rm(g_cg, S0, M_D(O(P) + 4));
   x86_alu_rr(g_cg, X86_ADD, EAX, ECX);
   x86_alu_rr16(g_cg, X86_ADC, S1, S0);
   x86_setcc_m8(g_cg, X86_CC_B, M_D(O_FC));
   x86_setcc_m8(g_cg, X86_CC_S, M_D(O_FS));
   x86_setcc_r8(g_cg, X86_CC_E, EDX);
   x86_setcc_r8(g_cg, X86_CC_O, ECX);
   x86_or_m8r8(g_cg, M_D(O_FV), ECX);          /* flags dead from here */
   x86_test_rr(g_cg, EAX, EAX);
   x86_setcc_r8(g_cg, X86_CC_E, ECX);
   x86_movzx_rr8(g_cg, EDX, EDX);
   x86_movzx_rr8(g_cg, ECX, ECX);
   x86_alu_rr(g_cg, X86_AND, EDX, ECX);
   x86_mov_m8r8(g_cg, M_D(O_FZ), EDX);
   x86_movzx_rm8(g_cg, ECX, M_D(O_FC));
   x86_shift_ri(g_cg, X86_SHL, ECX, 16);
   x86_movzx_rr16(g_cg, S1, S1);
   x86_alu_rr(g_cg, X86_OR, S1, ECX);        /* S1 = ALU.high32 = H | carry<<16 */
#endif
   break;
  case 0x08:
   x86_shift_ri(g_cg, X86_SAR, EAX, 1);
   x86_setcc_m8(g_cg, X86_CC_B, M_D(O_FC));
   emit_zs32();
   break;
  case 0x09:
   x86_ror_ri(g_cg, EAX, 1);
   x86_setcc_m8(g_cg, X86_CC_B, M_D(O_FC));
   emit_zs32();
   break;
  case 0x0A:
   x86_shift_ri(g_cg, X86_SHL, EAX, 1);
   x86_setcc_m8(g_cg, X86_CC_B, M_D(O_FC));
   emit_zs32();
   break;
  case 0x0B:
   x86_rol_ri(g_cg, EAX, 1);
   x86_setcc_m8(g_cg, X86_CC_B, M_D(O_FC));
   emit_zs32();
   break;
  case 0x0F:
   x86_rol_ri(g_cg, EAX, 8);
   x86_setcc_m8(g_cg, X86_CC_B, M_D(O_FC));
   emit_zs32();
   break;
  default:
   break;                                       /* NOP: ALU = AC */
 }

 /* Materialise ALU.T in scratch when anything downstream needs it. */
 if(need_alu_l || need_alu_t)
 {
#if X86EMIT_64
  if(alu_op != 0x06)
  {
   /* RAX = (AC.T & ~0xFFFFFFFF) | zext(EAX) */
   x86_mov_rm64(g_cg, ECX, M_D(O(AC)));
   x86_shift_ri64(g_cg, X86_SHR, ECX, 32);
   x86_shift_ri64(g_cg, X86_SHL, ECX, 32);
   x86_mov_rr(g_cg, EAX, EAX);                 /* zero-extend */
   x86_alu_rr64(g_cg, X86_OR, EAX, ECX);
  }
#else
  if(alu_op != 0x06)
   x86_mov_rm(g_cg, S1, M_D(O(AC) + 4));
#endif
  emit_alu_save_full();
 }

 /* --- X op --- */
 if((x_op & 0x3) == 0x2)
 {
#if X86EMIT_64
  x86_movsxd_rm(g_cg, EAX, M_D(O(RX)));
  x86_movsxd_rm(g_cg, ECX, M_D(O(RY)));
  x86_imul_rr64(g_cg, EAX, ECX);
  x86_mov_mr64(g_cg, M_D(O(P)), EAX);
#else
  x86_mov_rm(g_cg, EAX, M_D(O(RX)));
  x86_imul_m(g_cg, M_D(O(RY)));
  x86_mov_mr(g_cg, M_D(O(P)), EAX);
  x86_mov_mr(g_cg, M_D(O(P) + 4), EDX);
#endif
 }
 if(x_op >= 0x3)
 {
  const unsigned bank = ((instr >> 20) & 0x7) & 0x3;
  emit_dram_load(bank);
  if((x_op & 0x3) == 0x3)
   emit_store_p_sext();
  if(x_op & 0x4)
   x86_mov_mr(g_cg, M_D(O(RX)), EAX);
 }

 /* --- Y op --- */
 if((y_op & 0x3) == 0x1)
 {
  x86_mov_mi32(g_cg, M_D(O(AC)), 0);
  x86_mov_mi32(g_cg, M_D(O(AC) + 4), 0);
 }
 else if((y_op & 0x3) == 0x2)
 {
#if X86EMIT_64
  x86_mov_rm64(g_cg, EAX, M_SCR(0));
  x86_mov_mr64(g_cg, M_D(O(AC)), EAX);
#else
  x86_mov_rm(g_cg, EAX, M_SCR(0));
  x86_mov_mr(g_cg, M_D(O(AC)), EAX);
  x86_mov_rm(g_cg, EAX, M_SCR(1));
  x86_mov_mr(g_cg, M_D(O(AC) + 4), EAX);
#endif
 }
 if(y_op >= 0x3)
 {
  const unsigned bank = ((instr >> 14) & 0x7) & 0x3;
  emit_dram_load(bank);
  if((y_op & 0x3) == 0x3)
   emit_store_ac_sext();
  if(y_op & 0x4)
   x86_mov_mr(g_cg, M_D(O(RY)), EAX);
 }

 /* --- D1 op --- */
 if(d1_op & 0x1)
 {
  const unsigned d = (instr >> 8) & 0xF;
  bool src_in_eax = false;

  if(d1_op & 0x2)
  {
   switch(instr & 0xF)
   {
    case 0x8: case 0xB: case 0xC: case 0xD: case 0xE: case 0xF:
     x86_mov_ri(g_cg, EAX, 0xFFFFFFFFu); src_in_eax = true; break;
    case 0x0: case 0x1: case 0x2: case 0x3:
    case 0x4: case 0x5: case 0x6: case 0x7:
     emit_dram_load(instr & 0x3); src_in_eax = true; break;
    case 0x9:
     x86_mov_rm(g_cg, EAX, M_SCR(0)); src_in_eax = true; break;
    case 0xA:
#if X86EMIT_64
     x86_mov_rm64(g_cg, EAX, M_SCR(0));
     x86_shift_ri64(g_cg, X86_SHR, EAX, 16);
#else
     x86_mov_rm(g_cg, EAX, M_SCR(0));
     x86_mov_rm(g_cg, EDX, M_SCR(1));
     x86_shrd_rri(g_cg, EAX, EDX, 16);
#endif
     src_in_eax = true; break;
    default: break;
   }
  }
  if(!src_in_eax)
   x86_mov_ri(g_cg, EAX, (uint32_t)(int32_t)(int8_t)instr);

  switch(d)
  {
   case 0x0: case 0x1: case 0x2: case 0x3:
    if(!(meta.dr_read & (1u << d)))
    {
     x86_movzx_rm8(g_cg, ECX, M_D(O_CT(d)));
     x86_mov_mr(g_cg, M_DIDX(ECX, 2, O_DRAM(d)), EAX);
    }
    break;
   case 0x4: x86_mov_mr(g_cg, M_D(O(RX)), EAX); break;
   case 0x5: emit_store_p_sext(); break;
   case 0x6: x86_mov_mr(g_cg, M_D(O(RAO)), EAX); break;
   case 0x7: x86_mov_mr(g_cg, M_D(O(WAO)), EAX); break;
   case 0xA:
   {
    x86_label* skip = NULL;
    if(looped)
    {
     skip = label_new();
     x86_movzx_rm16(g_cg, ECX, M_D(O_LOP));
     x86_alu_ri(g_cg, X86_CMP, ECX, 0xFFF);
     x86_jcc(g_cg, X86_CC_NE, skip);
    }
    x86_alu_ri(g_cg, X86_AND, EAX, 0xFFF);
    x86_mov_mr16(g_cg, M_D(O_LOP), EAX);
    if(looped) x86_label_bind(g_cg, skip);
    break;
   }
   case 0xB: x86_mov_m8r8(g_cg, M_D(O_TOP), EAX); break;
   case 0xC: case 0xD: case 0xE: case 0xF:
    x86_mov_m8r8(g_cg, M_D(O_CT(d - 0xC)), EAX);
    break;
   default: break;                              /* 0x8, 0x9 */
  }
 }

 /* --- CT32 update --- */
 if(x_op >= 0x3 || y_op >= 0x3 || (d1_op & 0x1))
 {
  x86_mov_rm(g_cg, EAX, M_D(O(CT32)));
  if(meta.ct_inc)
   x86_alu_ri(g_cg, X86_ADD, EAX, (int32_t)meta.ct_inc);
  x86_alu_ri(g_cg, X86_AND, EAX, 0x3F3F3F3F);
  x86_mov_mr(g_cg, M_D(O(CT32)), EAX);
 }

 emit_tail_dispatch();
}

/* --- MVI ----------------------------------------------------------------- */

static void emit_mvi(bool looped, uint32_t instr)
{
 const unsigned dest = (instr >> 26) & 0xF;
 const unsigned cond = (instr >> 19) & 0x7F;
 const uint32_t imm  = (cond & 0x40) ? (uint32_t)sign_x_to_s32(19, instr)
                                     : (uint32_t)sign_x_to_s32(25, instr);
 x86_label* skip = label_new();

 emit_cold_entry();
 emit_link_guard();
 emit_instr_pre(looped);
 emit_test_cond(cond, skip);

 if(dest == 0x6 || dest == 0x7)
 {
  x86_label* nodma = label_new();
  x86_cmp_mi32(g_cg, M_D(O(PRAMDMABufCount)), 0);
  x86_jcc(g_cg, X86_CC_E, nodma);
#if X86EMIT_64
  x86_alu_ri(g_cg, X86_SUB, PIN_PC, 1);
  x86_alu_ri(g_cg, X86_AND, PIN_PC, 0xFF);
#else
  x86_dec_m8(g_cg, M_D(O_PC));
#endif
  emit_call_void((const void*)&DSP_FinishPRAMDMA);
  x86_label_bind(g_cg, nodma);
 }

 switch(dest)
 {
  case 0x0: case 0x1: case 0x2: case 0x3:
   x86_movzx_rm8(g_cg, ECX, M_D(O_CT(dest)));
   x86_mov_mi32(g_cg, M_DIDX(ECX, 2, O_DRAM(dest)), imm);
   x86_alu_ri(g_cg, X86_ADD, ECX, 1);
   x86_alu_ri(g_cg, X86_AND, ECX, 0x3F);
   x86_mov_m8r8(g_cg, M_D(O_CT(dest)), ECX);
   break;
  case 0x4: x86_mov_mi32(g_cg, M_D(O(RX)), imm); break;
  case 0x5:
   x86_mov_mi32(g_cg, M_D(O(P)), imm);
   x86_mov_mi32(g_cg, M_D(O(P) + 4), (uint32_t)((int32_t)imm >> 31));
   break;
  case 0x6: x86_mov_mi32(g_cg, M_D(O(RAO)), imm); break;
  case 0x7: x86_mov_mi32(g_cg, M_D(O(WAO)), imm); break;
  case 0xA:
  {
   x86_label* s2 = NULL;
   if(looped)
   {
    s2 = label_new();
    x86_movzx_rm16(g_cg, ECX, M_D(O_LOP));
    x86_alu_ri(g_cg, X86_CMP, ECX, 0xFFF);
    x86_jcc(g_cg, X86_CC_NE, s2);
   }
   x86_mov_mi16(g_cg, M_D(O_LOP), (uint16_t)(imm & 0xFFF));
   if(looped) x86_label_bind(g_cg, s2);
   break;
  }
  case 0xC:
  {
   x86_label* nodma = label_new();
#if X86EMIT_64
   x86_lea(g_cg, EAX, PIN_PC, X86_NOIDX, 0, -1);
   x86_mov_m8r8(g_cg, M_D(O_TOP), EAX);
   x86_mov_ri(g_cg, PIN_PC, imm & 0xFFu);
#else
   x86_movzx_rm8(g_cg, EAX, M_D(O_PC));
   x86_alu_ri(g_cg, X86_SUB, EAX, 1);
   x86_mov_m8r8(g_cg, M_D(O_TOP), EAX);
   x86_mov_mi8(g_cg, M_D(O_PC), (uint8_t)imm);
#endif
   x86_cmp_mi32(g_cg, M_D(O(PRAMDMABufCount)), 0);
   x86_jcc(g_cg, X86_CC_E, nodma);
   emit_call_void((const void*)&DSP_FinishPRAMDMA);
   x86_label_bind(g_cg, nodma);
   break;
  }
  default: break;
 }

 x86_label_bind(g_cg, skip);
 emit_tail_dispatch();
}

/* --- JMP ------------------------------------------------------------------ */

static void emit_jmp(bool looped, uint32_t instr)
{
 const unsigned cond = (instr >> 19) & 0x7F;
 x86_label* skip = label_new();

 emit_cold_entry();
 emit_link_guard();
 emit_instr_pre(looped);
 emit_test_cond(cond, skip);
#if X86EMIT_64
 x86_mov_ri(g_cg, PIN_PC, (uint32_t)(uint8_t)instr);
#else
 x86_mov_mi8(g_cg, M_D(O_PC), (uint8_t)instr);
#endif
 x86_label_bind(g_cg, skip);
 emit_tail_dispatch();
}

/* --- MISC ----------------------------------------------------------------- */

static void emit_misc(bool looped, uint32_t instr)
{
 const unsigned op = (instr >> 27) & 0x3;

 emit_cold_entry();
 emit_link_guard();
 emit_instr_pre(looped);

 if(op == 2 || op == 3)
  emit_call_helper((const void*)&misc_end_helper, true, op & 1);
 else if(op == 0)                                /* BTM */
 {
  x86_label* nolop = label_new();
  x86_movzx_rm16(g_cg, EAX, M_D(O_LOP));
  x86_test_rr(g_cg, EAX, EAX);
  x86_jcc(g_cg, X86_CC_E, nolop);
#if X86EMIT_64
  x86_movzx_rm8(g_cg, PIN_PC, M_D(O_TOP));
#else
  x86_movzx_rm8(g_cg, ECX, M_D(O_TOP));
  x86_mov_m8r8(g_cg, M_D(O_PC), ECX);
#endif
  x86_label_bind(g_cg, nolop);
  x86_alu_ri(g_cg, X86_SUB, EAX, 1);
  x86_alu_ri(g_cg, X86_AND, EAX, 0xFFF);
  x86_mov_mr16(g_cg, M_D(O_LOP), EAX);
 }
 else                                            /* LPS */
  emit_call_helper((const void*)&lps_helper, false, 0);

 emit_tail_dispatch();
}

/* --- rewind ---------------------------------------------------------------- */

static void rewind_locked(void)
{
 unsigned i;
 if(!g_cg) return;
 g_rewind_pending = false;
 g_in_rewind = true;
 x86_codegen_set_wptr(g_cg, (char*)g_seg_start + g_post_stub_byte_offset);
 labels_reset();
 for(i = 0; i < 256; ++i)
 {
  g_looped_cache[i].entry = NULL;
  g_link_site[i] = NULL;
 }

 g_in_rewind_slots = true;
 for(i = 0; i < 256; ++i)
 {
  const uint32_t instr = (uint32_t)(DSP.ProgRAM[i] >> 32);
  DSP.ProgRAM[i] = DSP_DecodeSlotInstruction((uint8_t)i, instr, false);
 }
 g_in_rewind_slots = false;

 /* Pass 2: every slot exists now; point each linked tail at its
  * successor's hot entry. */
 for(i = 0; i < 256; ++i)
 {
  const unsigned succ = (i + 1u) & 0xFFu;
  const int32_t  succ_off = (int32_t)(uint32_t)(DSP.ProgRAM[succ] & 0xFFFFFFFFu);
  if(!g_link_site[i]) continue;
  x86_patch_jmp_abs(g_link_site[i],
                    (const char*)DSP_INSTR_BASE_UIPT + succ_off + (int)SCU_JIT_SLOT_PRELUDE_BYTES);
 }
 {
  const uint32_t instr = (uint32_t)(DSP.NextInstr >> 32);
  const bool looped = DSP.NextInstrLooped;
  DSP.NextInstr = looped
   ? DSP_DecodeInstruction(instr, true)
   : DSP_DecodeSlotInstruction(0, instr, false);
 }
 g_in_rewind = false;
}

/* --- public API ------------------------------------------------------------- */

static void jit_entry(struct DSPS* dsp)
{
 if(MDFN_UNLIKELY(g_rewind_pending))
  rewind_locked();
 g_chain_live = true;
 g_entry_stub(dsp);
 g_chain_live = false;
}

/* Defined unconditionally in scu_dsp_jit.c; shared by every backend. */

void SCU_DSP_JIT_Init(void)
{
 if(!g_cg)
 {
  void* stubs_start;
  void* prelude_addr;
  void* dispatch_addr;
  void* entry_disp_addr;
  void* entry_addr;
  void* thunk_n_addr;
  void* thunk_l_addr;
  void* post;

  g_cg = x86_codegen_create_near(SCU_JIT_CODE_SEGMENT_SIZE, (const void*)DSP_INSTR_BASE_UIPT);
  if(!g_cg) return;
  g_seg_start = x86_codegen_wptr(g_cg);
  stubs_start = g_seg_start;

  prelude_addr = x86_codegen_wptr(g_cg);
  emit_prelude_stub();
  g_prelude_stub = prelude_addr;

  dispatch_addr = x86_codegen_wptr(g_cg);
  emit_dispatch_stub();
  g_dispatch_stub = dispatch_addr;

  entry_disp_addr = x86_codegen_wptr(g_cg);
  emit_entry_dispatch();

  /* Entry stub: a C-callable function running the chain from NextInstr. */
  entry_addr = x86_codegen_wptr(g_cg);
#if X86EMIT_64
  x86_mov_ri64(g_cg, EAX, (uint64_t)(uintptr_t)entry_disp_addr);
#else
  x86_mov_ri(g_cg, EAX, (uint32_t)(uintptr_t)entry_disp_addr);
#endif
  x86_jmp_abs(g_cg, g_prelude_stub);
  g_entry_stub = (void (*)(struct DSPS*))entry_addr;
  SCU_DSP_JIT_Entry = &jit_entry;

  thunk_n_addr = x86_codegen_wptr(g_cg);
  emit_fallback_thunk(&fallback_shim_normal);
  thunk_l_addr = x86_codegen_wptr(g_cg);
  emit_fallback_thunk(&fallback_shim_looped);
  g_fallback_thunk[0] = thunk_n_addr;
  g_fallback_thunk[1] = thunk_l_addr;

  post = x86_codegen_wptr(g_cg);
  g_post_stub_byte_offset = (size_t)((char*)post - (char*)stubs_start);

  SS_JitDump_Open();
  SS_JitDump_Emit("dsp_x86_stubs", stubs_start, g_post_stub_byte_offset);
 }
 rewind_locked();
}

void SCU_DSP_JIT_Reset(void)
{
 if(!g_cg)
  SCU_DSP_JIT_Init();
 else
  rewind_locked();
}

uint64_t SCU_DSP_JIT_FallbackNI(uint32_t instr, bool looped)
{
 const void* thunk = g_fallback_thunk[looped];
 if(!thunk)
  return 0;
 return ((uint64_t)instr << 32) | (uint32_t)((uintptr_t)thunk - DSP_INSTR_BASE_UIPT);
}

void (*SCU_DSP_JIT_CompileSlot(uint8_t pc, bool looped, uint32_t instr))(struct DSPS*)
{
 void* start;
 typedef void (*EmitFn)(bool, uint32_t);
 EmitFn emit_inline;

 if(!g_cg)
  SCU_DSP_JIT_Init();
 if(!g_cg)
  return NULL;

 if(x86_codegen_offset(g_cg) + SCU_JIT_SLOT_MAX_BYTES > SCU_JIT_CODE_SEGMENT_SIZE)
 {
  if(MDFN_UNLIKELY(g_chain_live))
  {
   g_rewind_pending = true;
   return NULL;
  }
  rewind_locked();
 }

 start = x86_codegen_wptr(g_cg);
 labels_reset();

 g_link_this_slot = false;
 if(g_in_rewind_slots && !looped
    && !may_perturb_pc((uint32_t)(DSP.ProgRAM[(uint8_t)(pc - 1u)] >> 32))
    && is_linkable_slot(instr))
 {
  g_link_this_slot = true;
  g_link_this_pc   = pc;
 }

 if(!g_in_rewind && !looped)
 {
  /* This normal slot replaces one that linked tails may still target
   * (looped variants never are: links go to ProgRAM[pc+1]'s normal
   * slot).  Unlink now if a chain is running, and rewind at the next
   * entry to re-establish links through the new slot. */
  if(g_chain_live)
   unlink_all();
  g_rewind_pending = true;
 }

 emit_inline = is_general_instr(instr) ? &emit_gen  :
               is_mvi_instr(instr)     ? &emit_mvi  :
               is_jmp_instr(instr)     ? &emit_jmp  :
               is_misc_instr(instr)    ? &emit_misc : NULL;
 if(emit_inline)
  emit_inline(looped, instr);
 else
 {
  /* DMA and reserved encodings: the C handler runs the whole
   * instruction and tail-dispatches itself; memory is already current. */
  emit_cold_entry();
  emit_tailjump_c((const void*)pick_c_handler(looped, instr));
 }

 if(MDFN_UNLIKELY(x86_codegen_overflowed(g_cg)))
 {
  x86_codegen_set_wptr(g_cg, start);
  g_link_site[pc] = NULL;
  return NULL;
 }

#ifdef WANT_DSP_JIT_PERF_DUMP
 {
  char nm[40];
  snprintf(nm, sizeof(nm), "dsp_%c_pc%02x", looped ? 'l' : 'n', (unsigned)pc);
  SS_JitDump_Emit(nm, start, (size_t)((char*)x86_codegen_wptr(g_cg) - (char*)start));
 }
#endif

 return (void (*)(struct DSPS*))start;
}

#endif /* WANT_JIT && X86EMIT_HOST */
