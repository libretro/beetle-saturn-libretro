/******************************************************************************/
/* Mednafen Sega Saturn Emulation Module                                      */
/******************************************************************************/
/* sh7095_jit_x86.c - SH7095 per-instruction JIT, x86 / x86-64 backend
**
** See sh7095_jit.h for the model.  Each handler is a C-callable function
**
**     void handler(SH7095* z)
**
** whose body is the interpreter's op body for one specific instruction
** word, transcribed from sh7095_ops.inc with n, m and imm folded:
**
**     DoIDIF(z)                          -- the interpreter's own fetch
**     WB_EX_CHECK(r) for each r the body checks:
**         if(timestamp < WB_until[r]) timestamp = WB_until[r]
**     the operation on R[], SR
**     PC += 2
**
** State stays memory-resident in the SH7095 block ([EBX + disp32]); the
** only pinned value is z.  The frame keeps the stack 16-aligned at the
** DoIDIF call under cdecl, SysV and Win64.
**
** First subset: register/immediate ALU, logic, shifts and compares.
** Delay-slot variants, memory ops, branches and everything else fall
** back to the interpreter by returning NULL from compile().
*/

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "ss.h"
#include "sh7095.h"
#include "sh7095_jit.h"
#include "x86emit.h"
#include "ss_init.h"   /* SH7095_FastMap, SH7095_EXT_MAP_GRAN_BITS */

void (*SH2JIT_Table[65536])(struct SH7095*);
uint8_t SH2JIT_Pure[65536];
uint8_t SH2JIT_OpID[65536];
uint64_t SH2JIT_NativeCount, SH2JIT_ChainCount, SH2JIT_FallbackCount;
static uint8_t g_counting;

#if defined(WANT_JIT) && X86EMIT_HOST

#define SH2JIT_CODE_BYTES ((size_t)16 << 20)

static x86_codegen* g_cg = NULL;
static const void* g_dispatch_stub = NULL;
static const void* g_exit_stub = NULL;
static void (*g_entry_stub)(struct SH7095*) = NULL;
static const int32_t* g_slave_ts = NULL;      /* &CPU[1].timestamp */
static struct SH7095* g_master = NULL;        /* &CPU[0] */
static struct SH7095* g_slave  = NULL;        /* &CPU[1] */

/* Everything the emitted code needs that is not in the SH7095 block,
 * gathered so one register can reach it.  On x86-64 the entry stub pins
 * R12 = &g_env, R13 = SH2JIT_Table, R14 = SH7095_FastMap, R15 = opid
 * table (all callee-saved, all constant: nothing to flush at C calls). */
typedef struct
{
 struct SH7095* master;
 struct SH7095* slave;
 int32_t* mem_ts;
 const int32_t* next_event_ts;
 uint8_t single_step;
 uint8_t counting;
 int32_t quantum;       /* master may run this far ahead of the slave */
 uint8_t self_write;    /* a block stored into its own address range */
} SH2JitEnv;
static SH2JitEnv g_env;
#define E(field) ((int32_t)offsetof(SH2JitEnv, field))
#if X86EMIT_64
 #define PIN_ENV   X86_R12
 #define PIN_TABLE X86_R13
 #define PIN_FMAP  X86_R14
 #define PIN_OPID  X86_R15
#endif
static int32_t* g_mem_ts = NULL;              /* &SH7095_mem_timestamp */
static const int32_t* g_next_event_ts = NULL; /* &next_event_ts */
static uint8_t g_status[65536];    /* 0 = unknown, 1 = compiled, 2 = unsupported */
static uint32_t g_blk_lo, g_blk_hi;
static bool     g_blk_self_write_check;
static bool     g_body_wrote_memory;       /* the body emitted a store (self-write check needed) */
static bool     g_blk_allow_movb;      /* block compiler: next word is not CMP/EQ #imm / TST #imm (no triplet fusion possible) */
static bool     g_in_block;            /* compiling for the block compiler (master, slave off), not the chain */
static int      g_probe_placement;
static uint32_t g_fold_pc; static uint16_t g_fold_nw, g_fold_nnw;
static void emit_folded_fetch(uint32_t pc_at_body, uint16_t next_w, uint16_t nextnext_w);
static void emit_folded_fetch_from_globals(void) { emit_folded_fetch(g_fold_pc, g_fold_nw, g_fold_nnw); }
#define g_opid SH2JIT_OpID

#define O(field) ((int32_t)offsetof(SH7095, field))
#define O_R(n)   (O(R) + 4 * (int32_t)(n))
#define O_WB(n)  (O(WB_until) + 4 * (int32_t)(n))
#define M_Z(d)   X86_EBX, X86_NOIDX, 0, (d)

enum { EAX = X86_EAX, ECX = X86_ECX, EDX = X86_EDX, EBX = X86_EBX, ESP = X86_ESP };
/* Extra scratch: caller-saved on x86-64; on x86-32 ESI/EDI are saved by the prologue. */
#if X86EMIT_64
 #define S0 X86_R9
 #define S1 X86_R10
#else
 #define S0 X86_ESI
 #define S1 X86_EDI
#endif

#if X86EMIT_64
 #if defined(_WIN32)
  #define ARG0 X86_ECX
  #define CALL_SHADOW 32
 #else
  #define ARG0 X86_EDI
  #define CALL_SHADOW 0
 #endif
#endif

/* --- frame ------------------------------------------------------------- */

/* x86-64: entry rsp = 8 mod 16; push rbx -> 0; a CALL is then aligned.
 * Win64 additionally needs 32 bytes of shadow space: sub rsp,32 keeps
 * the alignment.  x86-32: entry esp = 12 mod 16; push ebx -> 8;
 * sub esp,4 -> 4; push z -> 0 at the CALL. */
/* Chain frame.  The entry stub (C-callable) establishes it once; handlers
 * reach each other by JMP through the dispatch stub and never touch it;
 * the exit stub tears it down and returns to the C loop. */
static void emit_entry_frame(void)
{
 x86_push(g_cg, EBX);
#if X86EMIT_64
 x86_push(g_cg, PIN_ENV); x86_push(g_cg, PIN_TABLE); x86_push(g_cg, PIN_FMAP); x86_push(g_cg, PIN_OPID);   /* rsp: 8 - 40 = 0 mod 16 */
 x86_mov_rr64(g_cg, EBX, ARG0);
 x86_mov_ri64(g_cg, PIN_ENV,   (uint64_t)(uintptr_t)&g_env);
 x86_mov_ri64(g_cg, PIN_TABLE, (uint64_t)(uintptr_t)SH2JIT_Table);
 x86_mov_ri64(g_cg, PIN_FMAP,  (uint64_t)(uintptr_t)SH7095_FastMap);
 x86_mov_ri64(g_cg, PIN_OPID,  (uint64_t)(uintptr_t)g_opid);
#else
 x86_push(g_cg, X86_ESI);
 x86_push(g_cg, X86_EDI);                       /* esp = 12 - 12 = 0 mod 16 */
 x86_mov_rm(g_cg, EBX, X86_ESP, X86_NOIDX, 0, 16);
#endif
}

static void emit_exit_frame(void)
{
#if X86EMIT_64
 x86_pop(g_cg, PIN_OPID); x86_pop(g_cg, PIN_FMAP); x86_pop(g_cg, PIN_TABLE); x86_pop(g_cg, PIN_ENV);
#else
 x86_pop(g_cg, X86_EDI);
 x86_pop(g_cg, X86_ESI);
#endif
 x86_pop(g_cg, EBX);
 x86_ret(g_cg);
}

/* Reach a g_env field / a table by pinned register on x86-64, by
 * absolute address in a scratch register on x86-32. */
static void emit_env_ptr(unsigned r, int32_t field)      /* r = *(void**)&g_env.field */
{
#if X86EMIT_64
 x86_mov_rm64(g_cg, r, PIN_ENV, X86_NOIDX, 0, field);
#else
 x86_mov_ri(g_cg, r, (uint32_t)(uintptr_t)&g_env);
 x86_mov_rm(g_cg, r, r, X86_NOIDX, 0, field);
#endif
}
static void emit_env_base(unsigned r)                    /* r = &g_env */
{
#if X86EMIT_64
 x86_mov_rr64(g_cg, r, PIN_ENV);
#else
 x86_mov_ri(g_cg, r, (uint32_t)(uintptr_t)&g_env);
#endif
}

/* Load an absolute address into a register (for the loop's globals). */
static void emit_abs(unsigned r, const void* p)
{
#if X86EMIT_64
 x86_mov_ri64(g_cg, r, (uint64_t)(uintptr_t)p);
#else
 x86_mov_ri(g_cg, r, (uint32_t)(uintptr_t)p);
#endif
}

/*
 * Dispatch stub: what the C loop does between two master instructions,
 * then the next handler.  Any condition the loop would act on exits to
 * C, whose own copies of these checks are idempotent:
 *
 *   SH7095_DMA_BusTimingKludge: timestamp += accum; accum = 0
 *   FRT/WDT due:                timestamp >= FRT_WDT_NextTS      -> exit
 *   slave must run:             timestamp > CPU[1].timestamp     -> exit
 *   eff_ts = max(timestamp, mem_ts); mem_ts = eff_ts if it was <=
 *   event due:                  eff_ts >= next_event_ts          -> exit
 *   next instruction:           op byte (sans delay flag) == opid[word]
 *                               and a handler exists              -> JMP
 *                               otherwise                        -> exit
 */
static uint8_t g_single_step = 0;   /* SH2JIT_SetSingleStep: exit after every instruction (verification) */
static const void* g_dispatch_next = NULL;   /* the lookup-and-jump tail of the dispatch stub: the entry stub's target */
static x86_label g_dispatch_next_lbl;

/* Shared lookup-and-jump for whichever CPU EBX points at. */
static void emit_dispatch_lookup(x86_label* exit)
{
 x86_mov_rm(g_cg, EAX, M_Z(O(Pipe_ID)));
 x86_mov_rr(g_cg, EDX, EAX);
 x86_alu_ri(g_cg, X86_AND, EDX, 0xFFFF);
 x86_shift_ri(g_cg, X86_SHR, EAX, 24);
 x86_alu_ri(g_cg, X86_AND, EAX, 0x7F);
#if X86EMIT_64
 x86_movzx_rm8(g_cg, ECX, PIN_OPID, EDX, 0, 0);
 x86_alu_rr(g_cg, X86_CMP, EAX, ECX);
 x86_jcc(g_cg, X86_CC_NE, exit);
 x86_mov_rm64(g_cg, EAX, PIN_TABLE, EDX, 3, 0);
 x86_alu_rr64(g_cg, X86_OR, EAX, EAX);
#else
 emit_abs(S0, g_opid);
 x86_movzx_rm8(g_cg, ECX, S0, EDX, 0, 0);
 x86_alu_rr(g_cg, X86_CMP, EAX, ECX);
 x86_jcc(g_cg, X86_CC_NE, exit);
 emit_abs(S0, SH2JIT_Table);
 x86_mov_rm(g_cg, EAX, S0, EDX, 2, 0);
 x86_test_rr(g_cg, EAX, EAX);
#endif
 x86_jcc(g_cg, X86_CC_E, exit);
 x86_jmp_r(g_cg, EAX);
}

/*
 * Two-CPU dispatch stub: the C loop's whole per-iteration body in
 * emitted code, with EBX switching between the master and the slave.
 * Every handler ends by jumping here with EBX = the CPU it ran on;
 * the master and slave phases are told apart by comparing EBX.
 *
 *   after a master instruction:
 *     DMA bus-timing kludge; FRT due -> exit; single-step -> exit
 *     while(master.ts > slave.ts): run slave instructions (below)
 *     eff_ts = max(master.ts, mem_ts); mem_ts = eff_ts if it was <=
 *     eff_ts >= next_event_ts -> exit
 *     next master instruction (lookup) or exit
 *   after a slave instruction:
 *     FRT due -> exit; single-step -> exit
 *     master.ts > slave.ts ? next slave instruction : back to master
 *
 * Any exit returns to the C loop with the frame restored; the loop's
 * own kludge, slave while-loop and event check are idempotent for what
 * the chain already did, and a slave-phase exit resumes the slave in C.
 */
static void emit_dispatch_stub(void)
{
 x86_label exit, memts_ahead, slave_after, master_events, slave_enter, slave_done;
 x86_label_init(&exit); x86_label_init(&memts_ahead); x86_label_init(&slave_after);
 x86_label_init(&master_events); x86_label_init(&slave_enter); x86_label_init(&slave_done); x86_label_init(&g_dispatch_next_lbl);

 if(g_counting)
 {
  emit_abs(S0, &SH2JIT_NativeCount);
  x86_alu_mi32(g_cg, X86_ADD, S0, X86_NOIDX, 0, 0, 1);
 }
 /* which CPU just ran? */
 emit_env_ptr(S0, E(slave));
#if X86EMIT_64
 x86_alu_rr64(g_cg, X86_CMP, EBX, S0);
#else
 x86_alu_rr(g_cg, X86_CMP, EBX, S0);
#endif
 x86_jcc(g_cg, X86_CC_E, &slave_after);

 /* --- master phase --- */
 x86_mov_rm(g_cg, EAX, M_Z(O(DMA_PenaltyKludgeAccum)));
 x86_alu_mr(g_cg, X86_ADD, M_Z(O(timestamp)), EAX);
 x86_mov_mi32(g_cg, M_Z(O(DMA_PenaltyKludgeAccum)), 0);
 x86_mov_rm(g_cg, EAX, M_Z(O(timestamp)));
 x86_alu_rm(g_cg, X86_CMP, EAX, M_Z(O(FRT_WDT_NextTS)));
 x86_jcc(g_cg, X86_CC_GE, &exit);
 emit_env_base(S0);
 x86_cmp_mi8(g_cg, S0, X86_NOIDX, 0, E(single_step), 0);
 x86_jcc(g_cg, X86_CC_NE, &exit);
 x86_mov_rr(g_cg, ECX, EAX);
 emit_env_base(S0);
 x86_alu_rm(g_cg, X86_SUB, ECX, S0, X86_NOIDX, 0, E(quantum));   /* master.ts - quantum (the slave side may sit near INT32_MAX) */
 emit_env_ptr(S0, E(slave));
 x86_alu_rm(g_cg, X86_CMP, ECX, S0, X86_NOIDX, 0, O(timestamp));
 x86_jcc(g_cg, X86_CC_G, g_env.quantum ? &exit : &slave_enter);   /* quantised: the slave runs in the C loop, not the JIT chain */

 x86_label_bind(g_cg, &master_events);         /* EBX = master */
 x86_mov_rm(g_cg, EAX, M_Z(O(timestamp)));
 emit_env_ptr(S0, E(mem_ts));
 x86_mov_rm(g_cg, ECX, S0, X86_NOIDX, 0, 0);
 x86_alu_rr(g_cg, X86_CMP, ECX, EAX);
 x86_jcc(g_cg, X86_CC_G, &memts_ahead);
 x86_mov_mr(g_cg, S0, X86_NOIDX, 0, 0, EAX);
 x86_mov_rr(g_cg, ECX, EAX);
 x86_label_bind(g_cg, &memts_ahead);
 emit_env_ptr(S0, E(next_event_ts));
 x86_alu_rm(g_cg, X86_CMP, ECX, S0, X86_NOIDX, 0, 0);
 x86_jcc(g_cg, X86_CC_GE, &exit);
 x86_label_bind(g_cg, &g_dispatch_next_lbl);
 g_dispatch_next = x86_codegen_wptr(g_cg);
 emit_dispatch_lookup(&exit);

 /* --- slave phase --- */
 x86_label_bind(g_cg, &slave_enter);
 emit_env_ptr(EBX, E(slave));
 emit_dispatch_lookup(&exit);                  /* first slave instruction */

 x86_label_bind(g_cg, &slave_after);           /* EBX = slave */
 x86_mov_rm(g_cg, EAX, M_Z(O(timestamp)));
 x86_alu_rm(g_cg, X86_CMP, EAX, M_Z(O(FRT_WDT_NextTS)));
 x86_jcc(g_cg, X86_CC_GE, &exit);
 emit_env_base(S0);
 x86_cmp_mi8(g_cg, S0, X86_NOIDX, 0, E(single_step), 0);
 x86_jcc(g_cg, X86_CC_NE, &exit);
 emit_env_ptr(S0, E(master));
 x86_mov_rm(g_cg, ECX, S0, X86_NOIDX, 0, O(timestamp));   /* master.ts */
 x86_alu_rr(g_cg, X86_CMP, ECX, EAX);
 x86_jcc(g_cg, X86_CC_LE, &slave_done);
 emit_dispatch_lookup(&exit);                  /* next slave instruction */
 x86_label_bind(g_cg, &slave_done);
 emit_env_ptr(EBX, E(master));
 x86_jmp(g_cg, &master_events);

 x86_label_bind(g_cg, &exit);
 x86_jmp_abs(g_cg, g_exit_stub);
}

static void emit_call_z(const void* fn)
{
#if X86EMIT_64
 if(CALL_SHADOW) x86_alu_ri64(g_cg, X86_SUB, ESP, CALL_SHADOW);
 x86_mov_rr64(g_cg, ARG0, EBX);
 x86_call_abs(g_cg, fn);
 if(CALL_SHADOW) x86_alu_ri64(g_cg, X86_ADD, ESP, CALL_SHADOW);
#else
 x86_alu_ri(g_cg, X86_SUB, ESP, 12);
 x86_push(g_cg, EBX);
 x86_call_abs(g_cg, fn);
 x86_alu_ri(g_cg, X86_ADD, ESP, 16);
#endif
}

/* --- pieces ------------------------------------------------------------- */

/* WB_EX_CHECK(r): timestamp = max(timestamp, WB_until[r]) */
static void emit_wb_check(unsigned r)
{
 x86_mov_rm(g_cg, EAX, M_Z(O(timestamp)));
 x86_mov_rm(g_cg, ECX, M_Z(O_WB(r)));
 x86_alu_rr(g_cg, X86_CMP, EAX, ECX);
 x86_cmov(g_cg, X86_CC_L, EAX, ECX);
 x86_mov_mr(g_cg, M_Z(O(timestamp)), EAX);
}

/* SetT(cc): SR = (SR & ~1) | cc, with cc the condition currently in flags. */
static void emit_set_t_cc(unsigned cc)
{
 x86_setcc_r8(g_cg, cc, EDX);
 x86_movzx_rr8(g_cg, EDX, EDX);
 x86_mov_rm(g_cg, EAX, M_Z(O(SR)));
 x86_alu_ri(g_cg, X86_AND, EAX, ~1);
 x86_alu_rr(g_cg, X86_OR, EAX, EDX);
 x86_mov_mr(g_cg, M_Z(O(SR)), EAX);
}

/* SetT(EDX & 1) */
static void emit_set_t_edx(void)
{
 x86_alu_ri(g_cg, X86_AND, EDX, 1);
 x86_mov_rm(g_cg, EAX, M_Z(O(SR)));
 x86_alu_ri(g_cg, X86_AND, EAX, ~1);
 x86_alu_rr(g_cg, X86_OR, EAX, EDX);
 x86_mov_mr(g_cg, M_Z(O(SR)), EAX);
}

/* FUSE_COND_BRANCH(op_id, cond): if the next op byte is op_id, call the
 * fusion tail with cond = T (cond_is_t) or !T.  Emitted after SetT. */
#define OP_BF_ID 0x64
#define OP_BT_ID 0x66
static void emit_fuse_cond_branch(unsigned op_id, bool cond_is_t)
{
 x86_label skip; x86_label_init(&skip);
 x86_mov_rm(g_cg, EAX, M_Z(O(Pipe_ID)));
 x86_shift_ri(g_cg, X86_SHR, EAX, 24);
 x86_alu_ri(g_cg, X86_CMP, EAX, (int32_t)op_id);
 x86_jcc(g_cg, X86_CC_NE, &skip);
 x86_mov_rm(g_cg, EDX, M_Z(O(SR)));
 x86_alu_ri(g_cg, X86_AND, EDX, 1);
 if(!cond_is_t) x86_alu_ri(g_cg, X86_XOR, EDX, 1);
#if X86EMIT_64
 if(CALL_SHADOW) x86_alu_ri64(g_cg, X86_SUB, ESP, CALL_SHADOW);
 x86_mov_rr64(g_cg, ARG0, EBX);
 #if defined(_WIN32)
 x86_mov_rr(g_cg, X86_EDX, EDX);
 #else
 x86_mov_rr(g_cg, X86_ESI, EDX);
 #endif
 x86_call_abs(g_cg, (const void*)&SH7095_JIT_FusedCondBranch_C0);
 if(CALL_SHADOW) x86_alu_ri64(g_cg, X86_ADD, ESP, CALL_SHADOW);
#else
 x86_alu_ri(g_cg, X86_SUB, ESP, 8);
 x86_push(g_cg, EDX);
 x86_push(g_cg, EBX);
 x86_call_abs(g_cg, (const void*)&SH7095_JIT_FusedCondBranch_C0);
 x86_alu_ri(g_cg, X86_ADD, ESP, 16);
#endif
 x86_label_bind(g_cg, &skip);
}

/*
 * The interpreter's DoIDIF(false) for the master with cache emulation
 * off, inlined (SH7095_DoIDIF_NI_C0_I0 is the reference):
 *
 *   DoID:    Pipe_ID = Pipe_IF | (opid[Pipe_IF] << 24) | EPending
 *   FetchIF: if(timestamp < MA_until - ((int32_t)(PC & 2) << 28)) timestamp = MA_until
 *            (the wait applies to the even halfword of each longword fetch)
 *            Pipe_IF = *(uint16_t*)(FastMap[PC >> 16] + PC)
 *            if((int32_t)PC < 0) Pipe_IF = Cache_ReadDataArray_u16(PC)
 *            timestamp++
 *
 * The negative-PC case (cache address/data array space) is rare and is
 * routed whole through the C function before any state is touched, so
 * the two paths are identical statement for statement.
 */
static bool g_fetch_int_prevent;   /* emit the DoID with IntPreventNext (PART_OP_INTDIS) */

static void emit_fetch_inline(void)
{
 x86_label slow, done; x86_label_init(&slow); x86_label_init(&done);
 x86_mov_rm(g_cg, EDX, M_Z(O(PC)));
 x86_test_rr(g_cg, EDX, EDX);
 x86_jcc(g_cg, X86_CC_S, &slow);
 /* DoID */
 x86_movzx_rm16(g_cg, EAX, M_Z(O(Pipe_IF)));
#if X86EMIT_64
 x86_movzx_rm8(g_cg, ECX, PIN_OPID, EAX, 0, 0);
#else
 x86_mov_ri(g_cg, ECX, (uint32_t)(uintptr_t)g_opid);
 x86_movzx_rm8(g_cg, ECX, ECX, EAX, 0, 0);
#endif
 x86_shift_ri(g_cg, X86_SHL, ECX, 24);
 x86_alu_rr(g_cg, X86_OR, EAX, ECX);
 x86_mov_rm(g_cg, ECX, M_Z(O(EPending)));
 if(g_fetch_int_prevent)
 {
  /* epo &= ~INT; if no PEX bits remain, epo = 0 (drops the 0xFF op-or) */
  x86_label keep; x86_label_init(&keep);
  x86_alu_ri(g_cg, X86_AND, ECX, ~(1 << (SH7095_PEX_INT + SH7095_EPENDING_PEXBITS_SHIFT)));
  x86_test_ri(g_cg, ECX, 0xFF << SH7095_EPENDING_PEXBITS_SHIFT);
  x86_jcc(g_cg, X86_CC_NE, &keep);
  x86_alu_rr(g_cg, X86_XOR, ECX, ECX);
  x86_label_bind(g_cg, &keep);
 }
 x86_alu_rr(g_cg, X86_OR, EAX, ECX);
 x86_mov_mr(g_cg, M_Z(O(Pipe_ID)), EAX);
 /* FetchIF */
 x86_mov_rm(g_cg, EAX, M_Z(O(timestamp)));
 x86_mov_rm(g_cg, ECX, M_Z(O(MA_until)));
 /* threshold = MA_until - ((PC & 2) << 28) */
 x86_mov_rr(g_cg, S0, EDX);
 x86_alu_ri(g_cg, X86_AND, S0, 2);
 x86_shift_ri(g_cg, X86_SHL, S0, 28);
 x86_mov_rr(g_cg, S1, ECX);
 x86_alu_rr(g_cg, X86_SUB, S1, S0);
 x86_alu_rr(g_cg, X86_CMP, EAX, S1);
 x86_cmov(g_cg, X86_CC_L, EAX, ECX);
 x86_alu_ri(g_cg, X86_ADD, EAX, 1);
 x86_mov_mr(g_cg, M_Z(O(timestamp)), EAX);
 x86_mov_rr(g_cg, ECX, EDX);
 x86_shift_ri(g_cg, X86_SHR, ECX, SH7095_EXT_MAP_GRAN_BITS);
#if X86EMIT_64
 x86_mov_rm64(g_cg, ECX, PIN_FMAP, ECX, 3, 0);
 x86_movzx_rm16(g_cg, EAX, ECX, EDX, 0, 0);
#else
 x86_mov_ri(g_cg, EAX, (uint32_t)(uintptr_t)SH7095_FastMap);
 x86_mov_rm(g_cg, ECX, EAX, ECX, 2, 0);
 x86_movzx_rm16(g_cg, EAX, ECX, EDX, 0, 0);
#endif
 x86_mov_mr(g_cg, M_Z(O(Pipe_IF)), EAX);
 x86_jmp(g_cg, &done);
 x86_label_bind(g_cg, &slow);
 emit_call_z((const void*)&SH7095_DoIDIF_NI_C0_I0);
 x86_label_bind(g_cg, &done);
}

static void emit_pc_advance(void)
{
 x86_alu_mi32(g_cg, X86_ADD, M_Z(O(PC)), 2);
}

/* --- memory access through the CPU's region tables ------------------------
 * MemRead32(A, v): v = z->MRFP32[A >> 29](A); the entries are the
 * out-of-line C_MemReadRT_* functions with every timing side effect
 * inside.  MDFN_FASTCALL on x86-32: A in ECX, V in EDX. */
typedef enum { MEM_8 = 0, MEM_16 = 1, MEM_32 = 2 } MemSize;

static int32_t read_table_off(MemSize sz, bool instr_space)
{
 if(instr_space) return sz == MEM_16 ? O(MRFP16_I) : O(MRFP32_I);
 return sz == MEM_8 ? O(MRFP8) : sz == MEM_16 ? O(MRFP16) : O(MRFP32);
}
static int32_t write_table_off(MemSize sz)
{
 return sz == MEM_8 ? O(MWFP8) : sz == MEM_16 ? O(MWFP16) : O(MWFP32);
}

/* EAX = address in -> EAX = value read (zero-extended to 32). */
static void emit_mem_read(MemSize sz, bool instr_space)
{
 const int32_t tab = read_table_off(sz, instr_space);
#if X86EMIT_64
 /* The function pointer must not sit in an argument register: on Win64
  * ARG0 is RCX, and loading the argument would overwrite it (this was
  * a crash in the wild: CALL into the guest address being read).  R11
  * is caller-saved and never an argument under SysV or Win64. */
 x86_mov_rr(g_cg, X86_R11, EAX);
 x86_shift_ri(g_cg, X86_SHR, X86_R11, 29);
 x86_mov_rm64(g_cg, X86_R11, EBX, X86_R11, 3, tab);
 if(CALL_SHADOW) x86_alu_ri64(g_cg, X86_SUB, ESP, CALL_SHADOW);
 x86_mov_rr(g_cg, ARG0, EAX);
 x86_call_r(g_cg, X86_R11);
 if(CALL_SHADOW) x86_alu_ri64(g_cg, X86_ADD, ESP, CALL_SHADOW);
#else
 x86_mov_rr(g_cg, EDX, EAX);
 x86_shift_ri(g_cg, X86_SHR, EDX, 29);
 x86_mov_rm(g_cg, EDX, EBX, EDX, 2, tab);
 x86_mov_rr(g_cg, ECX, EAX);                    /* fastcall arg0 */
 x86_call_r(g_cg, EDX);
#endif
 if(sz == MEM_8)  x86_movzx_rr8(g_cg, EAX, EAX);
 if(sz == MEM_16) x86_movzx_rr16(g_cg, EAX, EAX);
}

/* EAX = address, EDX = value: z->MWFP##sz[A >> 29](A, V). */
static void emit_mem_write(MemSize sz)
{
 const int32_t tab = write_table_off(sz);
 g_body_wrote_memory = true;
 if(g_blk_self_write_check)
 {
  /* (A & 0x0FFFFFFF) in [lo, hi) -> env.self_write = 1 */
  x86_label no; x86_label_init(&no);
  x86_mov_rr(g_cg, S1, EAX);
  x86_alu_ri(g_cg, X86_AND, S1, 0x0FFFFFFF);
  x86_alu_ri(g_cg, X86_SUB, S1, (int32_t)(g_blk_lo & 0x0FFFFFFF));
  x86_alu_ri(g_cg, X86_CMP, S1, (int32_t)((g_blk_hi - g_blk_lo) + 4));
  x86_jcc(g_cg, X86_CC_AE, &no);
  emit_env_base(S1);
  x86_mov_mi8(g_cg, S1, X86_NOIDX, 0, E(self_write), 1);
  x86_label_bind(g_cg, &no);
 }
#if X86EMIT_64
 x86_mov_rr(g_cg, X86_R11, EAX);                /* see emit_mem_read: never RCX */
 x86_shift_ri(g_cg, X86_SHR, X86_R11, 29);
 x86_mov_rm64(g_cg, X86_R11, EBX, X86_R11, 3, tab);
 if(CALL_SHADOW) x86_alu_ri64(g_cg, X86_SUB, ESP, CALL_SHADOW);
 #if defined(_WIN32)
 x86_mov_rr(g_cg, ARG0, EAX);                   /* RCX = A; V already in RDX */
 #else
 x86_mov_rr(g_cg, X86_ESI, EDX);
 x86_mov_rr(g_cg, ARG0, EAX);
 #endif
 x86_call_r(g_cg, X86_R11);
 if(CALL_SHADOW) x86_alu_ri64(g_cg, X86_ADD, ESP, CALL_SHADOW);
#else
 x86_mov_rr(g_cg, S0, EAX);
 x86_shift_ri(g_cg, X86_SHR, S0, 29);
 x86_mov_rm(g_cg, S0, EBX, S0, 2, tab);
 x86_mov_rr(g_cg, ECX, EAX);                    /* fastcall: ECX = A, EDX = V */
 x86_call_r(g_cg, S0);
#endif
}

/* WB_READ{8,16,32}(r, ea) with ea in EAX: read, sign-extend for 8/16,
 * R[r] = v; WB_until[r] = MA_until + 1. */
static void emit_wb_read(unsigned r, MemSize sz, bool instr_space)
{
 emit_mem_read(sz, instr_space);
 if(sz == MEM_8)  x86_movsx_rr8(g_cg, EAX, EAX);
 if(sz == MEM_16) x86_movsx_rr16(g_cg, EAX, EAX);
 x86_mov_mr(g_cg, M_Z(O_R(r)), EAX);
 x86_mov_rm(g_cg, EAX, M_Z(O(MA_until)));
 x86_alu_ri(g_cg, X86_ADD, EAX, 1);
 x86_mov_mr(g_cg, M_Z(O_WB(r)), EAX);
}

/* Memory-op bodies, transcribed from sh7095_ops.inc.  Loads and stores
 * check no WB hazards in the interpreter.  The two ops with triplet
 * fusion (MOV.B @Rm,Rn and MOV.B @(disp,Rm),R0) are left to the
 * interpreter. */
static bool compile_mem(uint32_t instr)
{
 const unsigned n = (instr >> 8) & 0xF;
 const unsigned m = (instr >> 4) & 0xF;
 const unsigned top = instr >> 12;
 const unsigned low = instr & 0xF;
 const unsigned d8 = instr & 0xFF;

 switch(top)
 {
  case 0x0:
   if(low >= 0x4 && low <= 0x6)                                       /* MOV.x Rm,@(R0,Rn) */
   {
    x86_mov_rm(g_cg, EAX, M_Z(O_R(0))); x86_alu_rm(g_cg, X86_ADD, EAX, M_Z(O_R(n)));
    x86_mov_rm(g_cg, EDX, M_Z(O_R(m)));
    emit_mem_write((MemSize)(low - 0x4)); return true;
   }
   if(low >= 0xC && low <= 0xE)                                       /* MOV.x @(R0,Rm),Rn */
   {
    x86_mov_rm(g_cg, EAX, M_Z(O_R(0))); x86_alu_rm(g_cg, X86_ADD, EAX, M_Z(O_R(m)));
    emit_wb_read(n, (MemSize)(low - 0xC), false); return true;
   }
   return false;
  case 0x1:                                                            /* MOV.L Rm,@(disp*4,Rn) */
   x86_mov_rm(g_cg, EAX, M_Z(O_R(n))); x86_alu_ri(g_cg, X86_ADD, EAX, (int32_t)(low << 2));
   x86_mov_rm(g_cg, EDX, M_Z(O_R(m)));
   emit_mem_write(MEM_32); return true;
  case 0x2:
   if(low <= 0x2)                                                      /* MOV.x Rm,@Rn */
   {
    x86_mov_rm(g_cg, EDX, M_Z(O_R(m)));
    x86_mov_rm(g_cg, EAX, M_Z(O_R(n)));
    emit_mem_write((MemSize)low); return true;
   }
   if(low >= 0x4 && low <= 0x6)                                       /* MOV.x Rm,@-Rn */
   {
    const unsigned sz = low - 0x4;
    x86_mov_rm(g_cg, EDX, M_Z(O_R(m)));                                /* val before the decrement */
    x86_mov_rm(g_cg, EAX, M_Z(O_R(n)));
    x86_alu_ri(g_cg, X86_SUB, EAX, 1 << sz);
    x86_mov_mr(g_cg, M_Z(O_R(n)), EAX);
    emit_mem_write((MemSize)sz); return true;
   }
   return false;
  case 0x5:                                                            /* MOV.L @(disp*4,Rm),Rn */
   x86_mov_rm(g_cg, EAX, M_Z(O_R(m))); x86_alu_ri(g_cg, X86_ADD, EAX, (int32_t)(low << 2));
   emit_wb_read(n, MEM_32, false); return true;
  case 0x6:
   if(low == 0x0 && g_blk_allow_movb)                                 /* MOV.B @Rm,Rn: blocks only, when no triplet follows */
   {
    x86_mov_rm(g_cg, EAX, M_Z(O_R(m)));
    emit_wb_read(n, MEM_8, false); return true;
   }
   if(low == 0x1 || low == 0x2)                                       /* MOV.W/L @Rm,Rn (MOV.B has triplet fusion) */
   {
    x86_mov_rm(g_cg, EAX, M_Z(O_R(m)));
    emit_wb_read(n, (MemSize)low, false); return true;
   }
   if(low >= 0x4 && low <= 0x6)                                       /* MOV.x @Rm+,Rn */
   {
    const unsigned sz = low - 0x4;
    x86_mov_rm(g_cg, EAX, M_Z(O_R(m)));
    x86_alu_mi32(g_cg, X86_ADD, M_Z(O_R(m)), 1 << sz);
    emit_wb_read(n, (MemSize)sz, false); return true;
   }
   return false;
  case 0x8:
   switch((instr >> 8) & 0xF)
   {
    case 0x0: case 0x1:                                                /* MOV.B/W R0,@(disp,Rn) -- n is nyb1 here */
    {
     const unsigned sz = (instr >> 8) & 1;
     x86_mov_rm(g_cg, EAX, M_Z(O_R(m))); x86_alu_ri(g_cg, X86_ADD, EAX, (int32_t)(low << sz));
     x86_mov_rm(g_cg, EDX, M_Z(O_R(0)));
     emit_mem_write((MemSize)sz); return true;
    }
    case 0x4:                                                          /* MOV.B @(disp,Rm),R0: blocks only, when no triplet follows */
     if(!g_blk_allow_movb) return false;
     x86_mov_rm(g_cg, EAX, M_Z(O_R(m))); x86_alu_ri(g_cg, X86_ADD, EAX, (int32_t)low);
     emit_wb_read(0, MEM_8, false); return true;
    case 0x5:                                                          /* MOV.W @(disp*2,Rm),R0 (MOV.B has triplet fusion) */
     x86_mov_rm(g_cg, EAX, M_Z(O_R(m))); x86_alu_ri(g_cg, X86_ADD, EAX, (int32_t)(low << 1));
     emit_wb_read(0, MEM_16, false); return true;
    default: return false;
   }
  case 0x9:                                                            /* MOV.W @(disp*2,PC),Rn */
   x86_mov_rm(g_cg, EAX, M_Z(O(PC))); x86_alu_ri(g_cg, X86_ADD, EAX, (int32_t)(d8 << 1));
   emit_wb_read(n, MEM_16, true); return true;
  case 0xC:
   switch((instr >> 8) & 0xF)
   {
    case 0x0: case 0x1: case 0x2:                                      /* MOV.x R0,@(disp,GBR) */
    {
     const unsigned sz = (instr >> 8) & 0xF;
     x86_mov_rm(g_cg, EAX, M_Z(O(GBR))); x86_alu_ri(g_cg, X86_ADD, EAX, (int32_t)(d8 << sz));
     x86_mov_rm(g_cg, EDX, M_Z(O_R(0)));
     emit_mem_write((MemSize)sz); return true;
    }
    case 0x4: case 0x5: case 0x6:                                      /* MOV.x @(disp,GBR),R0 */
    {
     const unsigned sz = ((instr >> 8) & 0xF) - 4;
     x86_mov_rm(g_cg, EAX, M_Z(O(GBR))); x86_alu_ri(g_cg, X86_ADD, EAX, (int32_t)(d8 << sz));
     emit_wb_read(0, (MemSize)sz, false); return true;
    }
    case 0x7:                                                          /* MOVA @(disp*4,PC),R0 */
     emit_wb_check(0);
     x86_mov_rm(g_cg, EAX, M_Z(O(PC))); x86_alu_ri(g_cg, X86_AND, EAX, ~3); x86_alu_ri(g_cg, X86_ADD, EAX, (int32_t)(d8 << 2));
     x86_mov_mr(g_cg, M_Z(O_R(0)), EAX); return true;
    default: return false;
   }
  case 0xD:                                                            /* MOV.L @(disp*4,PC),Rn */
   x86_mov_rm(g_cg, EAX, M_Z(O(PC))); x86_alu_ri(g_cg, X86_AND, EAX, ~3); x86_alu_ri(g_cg, X86_ADD, EAX, (int32_t)(d8 << 2));
   emit_wb_read(n, MEM_32, true); return true;
  default:
   return false;
 }
}

/* --- branches ------------------------------------------------------------- */

/* call fn(z, EAX) */
static void emit_call_z_eax(const void* fn)
{
#if X86EMIT_64
 if(CALL_SHADOW) x86_alu_ri64(g_cg, X86_SUB, ESP, CALL_SHADOW);
 #if defined(_WIN32)
 x86_mov_rr(g_cg, X86_EDX, EAX);
 #else
 x86_mov_rr(g_cg, X86_ESI, EAX);
 #endif
 x86_mov_rr64(g_cg, ARG0, EBX);
 x86_call_abs(g_cg, fn);
 if(CALL_SHADOW) x86_alu_ri64(g_cg, X86_ADD, ESP, CALL_SHADOW);
#else
 x86_alu_ri(g_cg, X86_SUB, ESP, 8);
 x86_push(g_cg, EAX);
 x86_push(g_cg, EBX);
 x86_call_abs(g_cg, fn);
 x86_alu_ri(g_cg, X86_ADD, ESP, 16);
#endif
}

/* FUSE_DELAY_SLOT_NOP: if the slot is a NOP, consume it now. */
#define OP_NOP_ID 0x75
static void emit_fuse_delay_slot_nop(void)
{
 x86_label skip; x86_label_init(&skip);
 x86_mov_rm(g_cg, EAX, M_Z(O(Pipe_ID)));
 x86_shift_ri(g_cg, X86_SHR, EAX, 24);
 x86_alu_ri(g_cg, X86_CMP, EAX, OP_NOP_ID | 0x80);
 x86_jcc(g_cg, X86_CC_NE, &skip);
 x86_alu_mi32(g_cg, X86_ADD, M_Z(O(PC)), 2);
 emit_fetch_inline();
 x86_label_bind(g_cg, &skip);
}

/* Returns 1 if compiled; sets *prefetch = whether BEGIN_OP's DoIDIF
 * precedes the body (DLYIDIF ops have none). */
static bool compile_branch(uint32_t instr, bool* prefetch)
{
 const unsigned n = (instr >> 8) & 0xF;
 const uint32_t d12 = (uint32_t)(((int32_t)(instr << 20)) >> 20) << 1;   /* sign_x_to_s32(12) << 1 */
 const uint32_t d8  = (uint32_t)(int32_t)(int8_t)instr << 1;

 *prefetch = false;
 switch(instr >> 12)
 {
  case 0xA: case 0xB:                                                  /* BRA / BSR */
   if((instr >> 12) == 0xB) { x86_mov_rm(g_cg, EAX, M_Z(O(PC))); x86_mov_mr(g_cg, M_Z(O(PR)), EAX); }
   x86_mov_rm(g_cg, EAX, M_Z(O(PC))); x86_alu_ri(g_cg, X86_ADD, EAX, (int32_t)d12);
   emit_call_z_eax((const void*)&SH7095_JIT_UCDelayBranch_C0);
   emit_fuse_delay_slot_nop();
   return true;
  case 0x4:
   if((instr & 0xFF) == 0x2B || (instr & 0xFF) == 0x0B)                /* JMP @Rn / JSR @Rn */
   {
     if((instr & 0xFF) == 0x0B) { x86_mov_rm(g_cg, EAX, M_Z(O(PC))); x86_mov_mr(g_cg, M_Z(O(PR)), EAX); }
    x86_mov_rm(g_cg, EAX, M_Z(O_R(n)));
    emit_call_z_eax((const void*)&SH7095_JIT_UCDelayBranch_C0);
    emit_fuse_delay_slot_nop();
    return true;
   }
   return false;
  case 0x0:
   if(instr == 0x000B)                                                 /* RTS */
   {
     x86_mov_rm(g_cg, EAX, M_Z(O(PR)));
    emit_call_z_eax((const void*)&SH7095_JIT_UCDelayBranch_C0);
    emit_fuse_delay_slot_nop();
    return true;
   }
   return false;
  case 0x8:
  {
   const unsigned sub = (instr >> 8) & 0xF;
   x86_label skip; x86_label_init(&skip);
   if(sub != 0x9 && sub != 0xB && sub != 0xD && sub != 0xF) return false;
   *prefetch = true;                                                   /* BEGIN_OP_FUDGEIF: DoIDIF, no icache kludge here */
   /* cond: T for BT/BT/S (9, D), !T for BF/BF/S (B, F) */
   x86_mov_rm(g_cg, EAX, M_Z(O(SR)));
   x86_alu_ri(g_cg, X86_AND, EAX, 1);
   x86_alu_ri(g_cg, X86_CMP, EAX, (sub == 0x9 || sub == 0xD) ? 1 : 0);
   x86_jcc(g_cg, X86_CC_NE, &skip);
   x86_mov_rm(g_cg, EAX, M_Z(O(PC))); x86_alu_ri(g_cg, X86_ADD, EAX, (int32_t)d8);
   if(sub == 0x9 || sub == 0xB)                                        /* BT / BF */
    emit_call_z_eax((const void*)&SH7095_JIT_Branch_C0);
   else                                                                /* BT/S / BF/S */
    emit_call_z_eax((const void*)&SH7095_JIT_DelayBranch_C0);
   x86_label_bind(g_cg, &skip);
   if(sub == 0xD || sub == 0xF)
    emit_fuse_delay_slot_nop();
   return true;
  }
  default:
   return false;
 }
}

/* --- per-instruction bodies ----------------------------------------------- */

typedef enum { B_MOV, B_ADD, B_SUB, B_AND, B_OR, B_XOR, B_NEG, B_NOT,
               B_EXTUB, B_EXTUW, B_EXTSB, B_EXTSW, B_SWAPB, B_SWAPW, B_XTRCT,
               B_CMPEQ, B_CMPHS, B_CMPGE, B_CMPHI, B_CMPGT, B_TST } RegOp;

/* Two-register forms: Rn = f(Rn, Rm) / T = pred(Rn, Rm).  Every one of
 * these checks WB for both n and m in the interpreter, in that order. */
static void emit_reg_reg(RegOp op, unsigned n, unsigned m)
{
 emit_wb_check(n);
 emit_wb_check(m);
 switch(op)
 {
  case B_MOV:  x86_mov_rm(g_cg, EAX, M_Z(O_R(m))); break;
  case B_ADD:  x86_mov_rm(g_cg, EAX, M_Z(O_R(n))); x86_alu_rm(g_cg, X86_ADD, EAX, M_Z(O_R(m))); break;
  case B_SUB:  x86_mov_rm(g_cg, EAX, M_Z(O_R(n))); x86_alu_rm(g_cg, X86_SUB, EAX, M_Z(O_R(m))); break;
  case B_AND:  x86_mov_rm(g_cg, EAX, M_Z(O_R(n))); x86_alu_rm(g_cg, X86_AND, EAX, M_Z(O_R(m))); break;
  case B_OR:   x86_mov_rm(g_cg, EAX, M_Z(O_R(n))); x86_alu_rm(g_cg, X86_OR,  EAX, M_Z(O_R(m))); break;
  case B_XOR:  x86_mov_rm(g_cg, EAX, M_Z(O_R(n))); x86_alu_rm(g_cg, X86_XOR, EAX, M_Z(O_R(m))); break;
  case B_NEG:  x86_mov_rm(g_cg, EAX, M_Z(O_R(m))); x86_neg(g_cg, EAX); break;
  case B_NOT:  x86_mov_rm(g_cg, EAX, M_Z(O_R(m))); x86_alu_ri(g_cg, X86_XOR, EAX, -1); break;
  case B_EXTUB: x86_movzx_rm8(g_cg, EAX, M_Z(O_R(m))); break;               /* LSB-first host: low byte */
  case B_EXTUW: x86_movzx_rm16(g_cg, EAX, M_Z(O_R(m))); break;
  case B_EXTSB: x86_movsx_rm8(g_cg, EAX, M_Z(O_R(m))); break;
  case B_EXTSW: x86_movsx_rm16(g_cg, EAX, M_Z(O_R(m))); break;
  case B_SWAPB:
   /* (Rm & 0xFFFF0000) | ((Rm >> 8) & 0xFF) | ((Rm << 8) & 0xFF00) */
   x86_mov_rm(g_cg, EAX, M_Z(O_R(m)));
   x86_mov_rr(g_cg, ECX, EAX);
   x86_alu_ri(g_cg, X86_AND, EAX, (int32_t)0xFFFF0000);
   x86_mov_rr(g_cg, EDX, ECX);
   x86_shift_ri(g_cg, X86_SHR, EDX, 8);
   x86_alu_ri(g_cg, X86_AND, EDX, 0xFF);
   x86_alu_rr(g_cg, X86_OR, EAX, EDX);
   x86_shift_ri(g_cg, X86_SHL, ECX, 8);
   x86_alu_ri(g_cg, X86_AND, ECX, 0xFF00);
   x86_alu_rr(g_cg, X86_OR, EAX, ECX);
   break;
  case B_SWAPW: x86_mov_rm(g_cg, EAX, M_Z(O_R(m))); x86_rol_ri(g_cg, EAX, 16); break;
  case B_XTRCT:
   x86_mov_rm(g_cg, EAX, M_Z(O_R(n)));
   x86_shift_ri(g_cg, X86_SHR, EAX, 16);
   x86_mov_rm(g_cg, ECX, M_Z(O_R(m)));
   x86_shift_ri(g_cg, X86_SHL, ECX, 16);
   x86_alu_rr(g_cg, X86_OR, EAX, ECX);
   break;
  case B_CMPEQ: case B_CMPHS: case B_CMPGE: case B_CMPHI: case B_CMPGT:
   x86_mov_rm(g_cg, EAX, M_Z(O_R(n)));
   x86_alu_rm(g_cg, X86_CMP, EAX, M_Z(O_R(m)));
   emit_set_t_cc(op == B_CMPEQ ? X86_CC_E : op == B_CMPHS ? X86_CC_AE : op == B_CMPGE ? X86_CC_GE :
                 op == B_CMPHI ? X86_CC_A : X86_CC_G);
   if(op == B_CMPGT) emit_fuse_cond_branch(OP_BT_ID, true);
   return;
  case B_TST:
   x86_mov_rm(g_cg, EAX, M_Z(O_R(n)));
   x86_alu_rm(g_cg, X86_AND, EAX, M_Z(O_R(m)));
   emit_set_t_cc(X86_CC_E);
   emit_fuse_cond_branch(OP_BF_ID, false);
   emit_fuse_cond_branch(OP_BT_ID, true);
   return;
 }
 x86_mov_mr(g_cg, M_Z(O_R(n)), EAX);
}

/* Compile one instruction word; returns false for words without a body. */
static bool g_body_touches_memory;

/* Where the instruction's fetch goes relative to its body, set by
 * compile_body for the caller: most ops prefetch (BEGIN_OP); the
 * system/control register moves fetch afterwards with the interrupt bit
 * masked (BEGIN_OP_DLYIDIF + PART_OP_INTDIS); the long multiplies adjust
 * MA/MM before a normal prefetch, which compile_body emits itself. */
typedef enum { FETCH_BEFORE = 0, FETCH_AFTER_INTDIS = 1, FETCH_EMITTED_BY_BODY = 2 } FetchPlacement;
static FetchPlacement g_fetch_placement;
static void (*g_fetch_emitter)(void);          /* used by FETCH_EMITTED_BY_BODY */

#define SR_T 0x1
#define SR_Q 0x100
#define SR_M 0x200
#define SYSREG_OFF(i) (O(SysRegs) + 4 * (int32_t)(i))
#define CTRLREG_OFF(i) (O(CtrlRegs) + 4 * (int32_t)(i))

/* MUL.L / DMULS / DMULU pre-fetch timing:
 *   timestamp++; MA_until = max(MA_until, MM_until, timestamp + 1); MM_until = MA_until + 4 */
static void emit_mul32_timing_then_fetch(void)
{
 x86_alu_mi32(g_cg, X86_ADD, M_Z(O(timestamp)), 1);
 x86_mov_rm(g_cg, EAX, M_Z(O(MA_until)));
 x86_mov_rm(g_cg, ECX, M_Z(O(MM_until)));
 x86_alu_rr(g_cg, X86_CMP, EAX, ECX); x86_cmov(g_cg, X86_CC_L, EAX, ECX);
 x86_mov_rm(g_cg, ECX, M_Z(O(timestamp))); x86_alu_ri(g_cg, X86_ADD, ECX, 1);
 x86_alu_rr(g_cg, X86_CMP, EAX, ECX); x86_cmov(g_cg, X86_CC_L, EAX, ECX);
 x86_mov_mr(g_cg, M_Z(O(MA_until)), EAX);
 x86_alu_ri(g_cg, X86_ADD, EAX, 4);
 x86_mov_mr(g_cg, M_Z(O(MM_until)), EAX);
 g_fetch_emitter();
}

/* MULS.W / MULU.W post-fetch timing: MA_until = max(MA_until, MM_until, ts + 1); MM_until = MA_until + 2 */
static void emit_mul16_timing(void)
{
 x86_mov_rm(g_cg, EAX, M_Z(O(MA_until)));
 x86_mov_rm(g_cg, ECX, M_Z(O(MM_until)));
 x86_alu_rr(g_cg, X86_CMP, EAX, ECX); x86_cmov(g_cg, X86_CC_L, EAX, ECX);
 x86_mov_rm(g_cg, ECX, M_Z(O(timestamp))); x86_alu_ri(g_cg, X86_ADD, ECX, 1);
 x86_alu_rr(g_cg, X86_CMP, EAX, ECX); x86_cmov(g_cg, X86_CC_L, EAX, ECX);
 x86_mov_mr(g_cg, M_Z(O(MA_until)), EAX);
 x86_alu_ri(g_cg, X86_ADD, EAX, 2);
 x86_mov_mr(g_cg, M_Z(O(MM_until)), EAX);
}

/* SetT(bit0 of EDX) where EDX already holds 0/1 */
static void emit_set_t_from_edx01(void)
{
 x86_mov_rm(g_cg, EAX, M_Z(O(SR)));
 x86_alu_ri(g_cg, X86_AND, EAX, ~SR_T);
 x86_alu_rr(g_cg, X86_OR, EAX, EDX);
 x86_mov_mr(g_cg, M_Z(O(SR)), EAX);
}

static bool compile_sys(uint32_t instr)
{
 const unsigned n = (instr >> 8) & 0xF;
 const unsigned m = (instr >> 4) & 0xF;
 const unsigned top = instr >> 12;
 const unsigned low = instr & 0xF;
 const unsigned sri = (instr >> 4) & 0x3;

 switch(top)
 {
  case 0x0:
   if(low == 0xF && g_in_block)                                                                                         /* MAC.L @Rm+,@Rn+ */
   {
    /* Blocks only (master, slave off).  As a chain handler it is bit-exact
     * on the master but diverges when the slave runs it (Daytona USA's
     * race demo, frames 1221-1250); until that is traced the chain leaves
     * MAC.L to the interpreter. */
    /* read m0 from R[m], R[m] += 4, read m1 from R[n], R[n] += 4 -- in that
     * order (n == m reads the incremented address); operands parked in the
     * interpreter's own resume fields, arithmetic in the C helper. */
    x86_mov_rm(g_cg, EAX, M_Z(O_R(m))); emit_mem_read(MEM_32, false); x86_mov_mr(g_cg, M_Z(O(Resume_MAC_L_m0)), EAX);
    x86_alu_mi32(g_cg, X86_ADD, M_Z(O_R(m)), 4);
    x86_mov_rm(g_cg, EAX, M_Z(O_R(n))); emit_mem_read(MEM_32, false); x86_mov_mr(g_cg, M_Z(O(Resume_MAC_L_m1)), EAX);
    x86_alu_mi32(g_cg, X86_ADD, M_Z(O_R(n)), 4);
    emit_call_z((const void*)&SH7095_JIT_MACL_Arith);
    x86_alu_mi32(g_cg, X86_ADD, M_Z(O(timestamp)), 1);
    g_body_touches_memory = true; return true;
   }
   if(instr == 0x0028) { x86_mov_mi32(g_cg, M_Z(O(MACH)), 0); x86_mov_mi32(g_cg, M_Z(O(MACL)), 0); return true; }   /* CLRMAC */
   if(instr == 0x0019) { x86_alu_mi32(g_cg, X86_AND, M_Z(O(SR)), ~(SR_Q | SR_M | SR_T)); return true; }               /* DIV0U */
   if((instr & 0xFF) == 0x0A || (instr & 0xFF) == 0x1A || (instr & 0xFF) == 0x2A)                                     /* STS MACH/MACL/PR,Rn */
   {
    x86_mov_rm(g_cg, EAX, M_Z(SYSREG_OFF(sri))); x86_mov_mr(g_cg, M_Z(O_R(n)), EAX);
    g_fetch_placement = FETCH_AFTER_INTDIS; return true;
   }
   if((instr & 0xFF) == 0x02 || (instr & 0xFF) == 0x12 || (instr & 0xFF) == 0x22)                                     /* STC SR/GBR/VBR,Rn */
   {
    x86_mov_rm(g_cg, EAX, M_Z(CTRLREG_OFF(sri))); x86_mov_mr(g_cg, M_Z(O_R(n)), EAX);
    g_fetch_placement = FETCH_AFTER_INTDIS; return true;
   }
   if(low == 0x7)                                                                                                       /* MUL.L Rm,Rn */
   {
    emit_mul32_timing_then_fetch();
    x86_mov_rm(g_cg, EAX, M_Z(O_R(n))); x86_imul_rm(g_cg, EAX, M_Z(O_R(m))); x86_mov_mr(g_cg, M_Z(O(MACL)), EAX);
    g_fetch_placement = FETCH_EMITTED_BY_BODY; return true;
   }
   return false;
  case 0x2:
   if(low == 0x7)                                                                                                       /* DIV0S */
   {
    emit_wb_check(n); emit_wb_check(m);
    x86_mov_rm(g_cg, EAX, M_Z(O_R(n))); x86_shift_ri(g_cg, X86_SHR, EAX, 31);        /* new_Q */
    x86_mov_rm(g_cg, ECX, M_Z(O_R(m))); x86_shift_ri(g_cg, X86_SHR, ECX, 31);        /* new_M */
    x86_mov_rr(g_cg, EDX, EAX); x86_alu_rr(g_cg, X86_XOR, EDX, ECX);                 /* T = Q != M */
    x86_shift_ri(g_cg, X86_SHL, EAX, 8); x86_shift_ri(g_cg, X86_SHL, ECX, 9);
    x86_alu_rr(g_cg, X86_OR, EAX, ECX); x86_alu_rr(g_cg, X86_OR, EAX, EDX);
    x86_mov_rm(g_cg, ECX, M_Z(O(SR))); x86_alu_ri(g_cg, X86_AND, ECX, ~(SR_Q | SR_M | SR_T)); x86_alu_rr(g_cg, X86_OR, ECX, EAX);
    x86_mov_mr(g_cg, M_Z(O(SR)), ECX); return true;
   }
   if(low == 0xE || low == 0xF)                                                                                         /* MULU.W / MULS.W */
   {
    emit_mul16_timing();
    if(low == 0xF) { x86_movsx_rm16(g_cg, EAX, M_Z(O_R(n))); x86_movsx_rm16(g_cg, ECX, M_Z(O_R(m))); }
    else           { x86_movzx_rm16(g_cg, EAX, M_Z(O_R(n))); x86_movzx_rm16(g_cg, ECX, M_Z(O_R(m))); }
    x86_imul_rr(g_cg, EAX, ECX); x86_mov_mr(g_cg, M_Z(O(MACL)), EAX); return true;
   }
   return false;
  case 0x3:
   switch(low)
   {
    case 0x4:                                                                                                          /* DIV1 */
    {
     x86_label sub_path, done; x86_label_init(&sub_path); x86_label_init(&done);
     emit_wb_check(n); emit_wb_check(m);
     /* S1 = new_Q = Rn >> 31; Rn = (Rn << 1) | T; ECX = tmp = Rn; new_Q ^= M */
     x86_mov_rm(g_cg, EAX, M_Z(O_R(n)));
     x86_mov_rr(g_cg, S1, EAX); x86_shift_ri(g_cg, X86_SHR, S1, 31);
     x86_shift_ri(g_cg, X86_SHL, EAX, 1);
     x86_mov_rm(g_cg, EDX, M_Z(O(SR))); x86_mov_rr(g_cg, S0, EDX);        /* S0 = SR */
     x86_alu_ri(g_cg, X86_AND, EDX, SR_T); x86_alu_rr(g_cg, X86_OR, EAX, EDX);
     x86_mov_rr(g_cg, ECX, EAX);                                            /* tmp */
     x86_mov_rr(g_cg, EDX, S0); x86_shift_ri(g_cg, X86_SHR, EDX, 9); x86_alu_ri(g_cg, X86_AND, EDX, 1);   /* M */
     x86_alu_rr(g_cg, X86_XOR, S1, EDX);
     /* if(!(Q ^ M)) Rn -= Rm, new_Q ^= (Rn > tmp)  else Rn += Rm, new_Q ^= (Rn < tmp) */
     x86_mov_rr(g_cg, EDX, S0); x86_shift_ri(g_cg, X86_SHR, EDX, 8); x86_alu_ri(g_cg, X86_AND, EDX, 3);   /* bits: Q | M<<1 */
     x86_alu_ri(g_cg, X86_CMP, EDX, 1); x86_jcc(g_cg, X86_CC_E, &sub_path);       /* Q=1,M=0 -> Q^M -> add path? no: see below */
     x86_alu_ri(g_cg, X86_CMP, EDX, 2); x86_jcc(g_cg, X86_CC_E, &sub_path);
     /* Q == M: subtract */
     x86_alu_rm(g_cg, X86_SUB, EAX, M_Z(O_R(m)));
     x86_mov_mr(g_cg, M_Z(O_R(n)), EAX);
     x86_alu_rr(g_cg, X86_CMP, EAX, ECX); x86_setcc_r8(g_cg, X86_CC_A, EDX); x86_movzx_rr8(g_cg, EDX, EDX);
     x86_alu_rr(g_cg, X86_XOR, S1, EDX);
     x86_jmp(g_cg, &done);
     x86_label_bind(g_cg, &sub_path);                                       /* Q != M: add */
     x86_alu_rm(g_cg, X86_ADD, EAX, M_Z(O_R(m)));
     x86_mov_mr(g_cg, M_Z(O_R(n)), EAX);
     x86_alu_rr(g_cg, X86_CMP, EAX, ECX); x86_setcc_r8(g_cg, X86_CC_B, EDX); x86_movzx_rr8(g_cg, EDX, EDX);
     x86_alu_rr(g_cg, X86_XOR, S1, EDX);
     x86_label_bind(g_cg, &done);
     /* SetQ(new_Q); SetT(new_Q == M) */
     x86_mov_rr(g_cg, EDX, S0); x86_shift_ri(g_cg, X86_SHR, EDX, 9); x86_alu_ri(g_cg, X86_AND, EDX, 1);   /* M */
     x86_alu_rr(g_cg, X86_XOR, EDX, S1); x86_alu_ri(g_cg, X86_XOR, EDX, 1);                                 /* T = (new_Q == M) */
     x86_mov_rr(g_cg, EAX, S0); x86_alu_ri(g_cg, X86_AND, EAX, ~(SR_Q | SR_T));
     x86_alu_rr(g_cg, X86_OR, EAX, EDX);
     x86_shift_ri(g_cg, X86_SHL, S1, 8); x86_alu_rr(g_cg, X86_OR, EAX, S1);
     x86_mov_mr(g_cg, M_Z(O(SR)), EAX);
     return true;
    }
    case 0x5: case 0xD:                                                                                                /* DMULU.L / DMULS.L */
     emit_mul32_timing_then_fetch();
     x86_mov_rm(g_cg, EAX, M_Z(O_R(n)));
     if(low == 0xD) { x86_movsxd_rr(g_cg, EAX, EAX); x86_movsxd_rm(g_cg, ECX, M_Z(O_R(m))); x86_imul_rr64(g_cg, EAX, ECX); }
     else           { x86_mov_rm(g_cg, ECX, M_Z(O_R(m))); x86_imul_rr64(g_cg, EAX, ECX); }   /* both zero-extended: unsigned product fits */
     x86_mov_mr(g_cg, M_Z(O(MACL)), EAX);
     x86_shift_ri64(g_cg, X86_SHR, EAX, 32);
     x86_mov_mr(g_cg, M_Z(O(MACH)), EAX);
     g_fetch_placement = FETCH_EMITTED_BY_BODY; return true;
    case 0xA: case 0xE:                                                                                                /* SUBC / ADDC: 64-bit result, T = bit 32 */
     emit_wb_check(n); emit_wb_check(m);
     x86_mov_rm(g_cg, EAX, M_Z(O_R(n))); x86_mov_rm(g_cg, ECX, M_Z(O_R(m)));
     x86_mov_rm(g_cg, EDX, M_Z(O(SR))); x86_alu_ri(g_cg, X86_AND, EDX, SR_T);
     if(low == 0xE) { x86_alu_rr64(g_cg, X86_ADD, EAX, ECX); x86_alu_rr64(g_cg, X86_ADD, EAX, EDX); }
     else           { x86_alu_rr64(g_cg, X86_SUB, EAX, ECX); x86_alu_rr64(g_cg, X86_SUB, EAX, EDX); }
     x86_mov_mr(g_cg, M_Z(O_R(n)), EAX);
     x86_shift_ri64(g_cg, X86_SHR, EAX, 32); x86_alu_ri(g_cg, X86_AND, EAX, 1); x86_mov_rr(g_cg, EDX, EAX);
     emit_set_t_from_edx01(); return true;
    case 0xB: case 0xF:                                                                                                /* SUBV / ADDV: T = signed overflow */
     emit_wb_check(n); emit_wb_check(m);
     x86_mov_rm(g_cg, EAX, M_Z(O_R(n)));
     if(low == 0xF) x86_alu_rm(g_cg, X86_ADD, EAX, M_Z(O_R(m))); else x86_alu_rm(g_cg, X86_SUB, EAX, M_Z(O_R(m)));
     x86_setcc_r8(g_cg, X86_CC_O, EDX); x86_movzx_rr8(g_cg, EDX, EDX);
     x86_mov_mr(g_cg, M_Z(O_R(n)), EAX);
     emit_set_t_from_edx01(); return true;
    default: return false;
   }
  case 0x4:
   if(low == 0xF)                                                                                                       /* MAC.W @Rm+,@Rn+ */
   {
    x86_mov_rm(g_cg, EAX, M_Z(O_R(m))); emit_mem_read(MEM_16, false); x86_mov_mr16(g_cg, M_Z(O(Resume_MAC_W_m0)), EAX);
    x86_alu_mi32(g_cg, X86_ADD, M_Z(O_R(m)), 2);
    x86_mov_rm(g_cg, EAX, M_Z(O_R(n))); emit_mem_read(MEM_16, false); x86_mov_mr16(g_cg, M_Z(O(Resume_MAC_W_m1)), EAX);
    x86_alu_mi32(g_cg, X86_ADD, M_Z(O_R(n)), 2);
    emit_call_z((const void*)&SH7095_JIT_MACW_Arith);
    x86_alu_mi32(g_cg, X86_ADD, M_Z(O(timestamp)), 1);
    g_body_touches_memory = true; return true;
   }
   if((instr & 0xFF) == 0x0A || (instr & 0xFF) == 0x1A || (instr & 0xFF) == 0x2A)                                     /* LDS Rm,MACH/MACL/PR */
   {
    x86_mov_rm(g_cg, EAX, M_Z(O_R(n))); x86_mov_mr(g_cg, M_Z(SYSREG_OFF(sri)), EAX);
    g_fetch_placement = FETCH_AFTER_INTDIS; return true;
   }
   if((instr & 0xFF) == 0x02 || (instr & 0xFF) == 0x12 || (instr & 0xFF) == 0x22)                                     /* STS.L MACH/MACL/PR,@-Rn */
   {
    x86_mov_rm(g_cg, EDX, M_Z(SYSREG_OFF(sri)));
    x86_mov_rm(g_cg, EAX, M_Z(O_R(n))); x86_alu_ri(g_cg, X86_SUB, EAX, 4); x86_mov_mr(g_cg, M_Z(O_R(n)), EAX);
    emit_mem_write(MEM_32);
    g_fetch_placement = FETCH_AFTER_INTDIS; g_body_touches_memory = true; return true;
   }
   if((instr & 0xFF) == 0x06 || (instr & 0xFF) == 0x16 || (instr & 0xFF) == 0x26)                                     /* LDS.L @Rm+,MACH/MACL/PR */
   {
    x86_mov_rm(g_cg, EAX, M_Z(O_R(n)));
    x86_alu_mi32(g_cg, X86_ADD, M_Z(O_R(n)), 4);
    emit_mem_read(MEM_32, false);
    x86_mov_mr(g_cg, M_Z(SYSREG_OFF(sri)), EAX);
    g_fetch_placement = FETCH_AFTER_INTDIS; g_body_touches_memory = true; return true;
   }
   return false;
  case 0x6:
   if(low == 0xA)                                                                                                       /* NEGC */
   {
    emit_wb_check(n); emit_wb_check(m);
    x86_alu_rr(g_cg, X86_XOR, EAX, EAX);
    x86_mov_rm(g_cg, ECX, M_Z(O_R(m)));
    x86_mov_rm(g_cg, EDX, M_Z(O(SR))); x86_alu_ri(g_cg, X86_AND, EDX, SR_T);
    x86_alu_rr64(g_cg, X86_SUB, EAX, ECX); x86_alu_rr64(g_cg, X86_SUB, EAX, EDX);
    x86_mov_mr(g_cg, M_Z(O_R(n)), EAX);
    x86_shift_ri64(g_cg, X86_SHR, EAX, 32); x86_alu_ri(g_cg, X86_AND, EAX, 1); x86_mov_rr(g_cg, EDX, EAX);
    emit_set_t_from_edx01(); return true;
   }
   return false;
  default:
   return false;
 }
}

static bool compile_body(uint32_t instr)
{
 g_fetch_placement = FETCH_BEFORE;
 g_body_wrote_memory = false;
 g_body_touches_memory = false;
 if(compile_mem(instr)) { g_body_touches_memory = true; return true; }
 if(compile_sys(instr)) return true;

 const unsigned n = (instr >> 8) & 0xF;
 const unsigned m = (instr >> 4) & 0xF;
 const unsigned top = instr >> 12;
 const unsigned low = instr & 0xF;
 const uint32_t imm8s = (uint32_t)(int32_t)(int8_t)instr;
 const unsigned imm8u = instr & 0xFF;

 switch(top)
 {
  case 0x0:
   if(instr == 0x0009) return true;                                   /* NOP */
   if(low == 0x9 && m == 0x2) { emit_wb_check(n);                      /* MOVT Rn */
    x86_mov_rm(g_cg, EAX, M_Z(O(SR))); x86_alu_ri(g_cg, X86_AND, EAX, 1); x86_mov_mr(g_cg, M_Z(O_R(n)), EAX); return true; }
   return false;

  case 0x2:
   switch(low)
   {
    case 0x8: emit_reg_reg(B_TST,   n, m); return true;
    case 0x9: emit_reg_reg(B_AND,   n, m); return true;
    case 0xA: emit_reg_reg(B_XOR,   n, m); return true;
    case 0xB: emit_reg_reg(B_OR,    n, m); return true;
    case 0xD: emit_reg_reg(B_XTRCT, n, m); return true;
    default: return false;                      /* 0/1/2/4/5/6 memory, 7 DIV0S, C CMP/STR, E/F MUL */
   }

  case 0x3:
   switch(low)
   {
    case 0x0: emit_reg_reg(B_CMPEQ, n, m); return true;
    case 0x2: emit_reg_reg(B_CMPHS, n, m); return true;
    case 0x3: emit_reg_reg(B_CMPGE, n, m); return true;
    case 0x6: emit_reg_reg(B_CMPHI, n, m); return true;
    case 0x7: emit_reg_reg(B_CMPGT, n, m); return true;
    case 0x8: emit_reg_reg(B_SUB,   n, m); return true;
    case 0xC: emit_reg_reg(B_ADD,   n, m); return true;
    default: return false;
   }

  case 0x4:
   /* single-register shifts / DT / CMP_PZ / CMP_PL, encoded 0100nnnn____ */
   switch(instr & 0xFF)
   {
    case 0x00: /* SHLL */ case 0x01: /* SHLR */ case 0x04: /* ROTL */ case 0x05: /* ROTR */
    case 0x20: /* SHAL */ case 0x21: /* SHAR */ case 0x24: /* ROTCL */ case 0x25: /* ROTCR */
    {
     const unsigned sub = instr & 0xFF;
     /* shbit/rotbit is read before the WB stall in the interpreter; the
      * value cannot change across the stall, so read it after. */
     emit_wb_check(n);
     x86_mov_rm(g_cg, EAX, M_Z(O_R(n)));
     x86_mov_rr(g_cg, EDX, EAX);
     if(sub == 0x00 || sub == 0x20 || sub == 0x04 || sub == 0x24) x86_shift_ri(g_cg, X86_SHR, EDX, 31);   /* bit 31 */
     else                                                         x86_alu_ri(g_cg, X86_AND, EDX, 1);      /* bit 0 */
     switch(sub)
     {
      case 0x00: case 0x20: x86_shift_ri(g_cg, X86_SHL, EAX, 1); break;
      case 0x01:            x86_shift_ri(g_cg, X86_SHR, EAX, 1); break;
      case 0x21:            x86_shift_ri(g_cg, X86_SAR, EAX, 1); break;
      case 0x04:            x86_rol_ri(g_cg, EAX, 1); break;
      case 0x05:            x86_ror_ri(g_cg, EAX, 1); break;
      case 0x24: /* (Rn << 1) | T */
       x86_shift_ri(g_cg, X86_SHL, EAX, 1);
       x86_mov_rm(g_cg, ECX, M_Z(O(SR))); x86_alu_ri(g_cg, X86_AND, ECX, 1); x86_alu_rr(g_cg, X86_OR, EAX, ECX); break;
      case 0x25: /* (Rn >> 1) | (T << 31) */
       x86_shift_ri(g_cg, X86_SHR, EAX, 1);
       x86_mov_rm(g_cg, ECX, M_Z(O(SR))); x86_shift_ri(g_cg, X86_SHL, ECX, 31); x86_alu_rr(g_cg, X86_OR, EAX, ECX); break;
     }
     x86_mov_mr(g_cg, M_Z(O_R(n)), EAX);
     emit_set_t_edx();
     return true;
    }
    case 0x08: case 0x18: case 0x28:                                   /* SHLL2/8/16 */
    case 0x09: case 0x19: case 0x29:                                   /* SHLR2/8/16 */
    {
     const unsigned amt = (instr & 0xF0) == 0x00 ? 2 : (instr & 0xF0) == 0x10 ? 8 : 16;
     emit_wb_check(n);
     x86_mov_rm(g_cg, EAX, M_Z(O_R(n)));
     x86_shift_ri(g_cg, (instr & 1) ? X86_SHR : X86_SHL, EAX, amt);
     x86_mov_mr(g_cg, M_Z(O_R(n)), EAX);
     return true;
    }
    case 0x10:                                                         /* DT */
     emit_wb_check(n);
     x86_mov_rm(g_cg, EAX, M_Z(O_R(n)));
     x86_alu_ri(g_cg, X86_SUB, EAX, 1);
     x86_mov_mr(g_cg, M_Z(O_R(n)), EAX);
     emit_set_t_cc(X86_CC_E);
     emit_fuse_cond_branch(OP_BF_ID, false);
     return true;
    case 0x11:                                                         /* CMP/PZ */
     emit_wb_check(n);
     x86_cmp_mi32(g_cg, M_Z(O_R(n)), 0);
     emit_set_t_cc(X86_CC_GE);
     return true;
    case 0x15:                                                         /* CMP/PL */
     emit_wb_check(n);
     x86_cmp_mi32(g_cg, M_Z(O_R(n)), 0);
     emit_set_t_cc(X86_CC_G);
     return true;
    default: return false;
   }

  case 0x6:
   switch(low)
   {
    case 0x3: emit_reg_reg(B_MOV,   n, m); return true;
    case 0x7: emit_reg_reg(B_NOT,   n, m); return true;
    case 0x8: emit_reg_reg(B_SWAPB, n, m); return true;
    case 0x9: emit_reg_reg(B_SWAPW, n, m); return true;
    case 0xB: emit_reg_reg(B_NEG,   n, m); return true;
    case 0xC: emit_reg_reg(B_EXTUB, n, m); return true;
    case 0xD: emit_reg_reg(B_EXTUW, n, m); return true;
    case 0xE: emit_reg_reg(B_EXTSB, n, m); return true;
    case 0xF: emit_reg_reg(B_EXTSW, n, m); return true;
    default: return false;
   }

  case 0x7:                                                            /* ADD #imm, Rn */
   emit_wb_check(n);
   x86_alu_mi32(g_cg, X86_ADD, M_Z(O_R(n)), (int32_t)imm8s);
   return true;

  case 0x8:
   if((instr & 0x0F00) == 0x0800)                                      /* CMP/EQ #imm, R0 */
   {
    emit_wb_check(0);
    x86_cmp_mi32(g_cg, M_Z(O_R(0)), (int32_t)imm8s);
    emit_set_t_cc(X86_CC_E);
    emit_fuse_cond_branch(OP_BF_ID, false);
    return true;
   }
   return false;

  case 0xC:
   switch((instr >> 8) & 0xF)
   {
    case 0x8:                                                          /* TST #imm, R0 */
     emit_wb_check(0);
     x86_mov_rm(g_cg, EAX, M_Z(O_R(0)));
     x86_alu_ri(g_cg, X86_AND, EAX, (int32_t)imm8u);
     emit_set_t_cc(X86_CC_E);
     emit_fuse_cond_branch(OP_BT_ID, true);
     return true;
    case 0x9: emit_wb_check(0); x86_alu_mi32(g_cg, X86_AND, M_Z(O_R(0)), (int32_t)imm8u); return true;   /* AND #imm, R0 */
    case 0xA: emit_wb_check(0); x86_alu_mi32(g_cg, X86_XOR, M_Z(O_R(0)), (int32_t)imm8u); return true;   /* XOR #imm, R0 */
    case 0xB: emit_wb_check(0); x86_alu_mi32(g_cg, X86_OR,  M_Z(O_R(0)), (int32_t)imm8u); return true;   /* OR  #imm, R0 */
    default: return false;
   }

  case 0xE:                                                            /* MOV #imm, Rn */
   emit_wb_check(n);
   x86_mov_mi32(g_cg, M_Z(O_R(n)), imm8s);
   return true;

  default:
   return false;
 }
}


/* ===========================================================================
 * Block compiler: master SH-2 with the slave powered off.
 *
 * With the slave off there is no instruction-granular interleave to
 * honour, so a run of instructions can be compiled as one unit.  Each
 * instruction keeps every statement of the interpreter's step -- the
 * pipeline words, the fetch timing, WB stalls, the op body, PC += 2,
 * the DMA kludge and the event deadline -- but the fetch's memory read
 * is folded (the words are known at compile time), the decode-table
 * and handler-table lookups disappear, and there is no dispatch
 * between instructions.  A block is validated against memory at entry
 * (its words are stored), so code loaded by DMA or rewritten by the
 * interpreter path is caught exactly; a store from inside the block
 * into its own address range ends it after that instruction.
 *
 * Exits (all leave interpreter-exact state at a step boundary):
 *   - Pipe_ID's op byte is 0xFF (exception pending) before an instruction
 *   - FRT/WDT update due before an instruction
 *   - event deadline reached after an instruction
 *   - the block wrote into its own range
 *   - a branch (block ends after it; the delay slot runs per-instruction)
 *   - an instruction without a native body / block length limit
 * ======================================================================== */

#define BLK_MAX_INSTR   24
#define BLK_TABLE_BITS  15
#define BLK_TABLE_SIZE  (1u << BLK_TABLE_BITS)

typedef struct
{
 uint32_t addr;            /* address of the first instruction; 0 = empty */
 uint32_t nwords;          /* instructions in the block */
 uint32_t nvalid;          /* words validated at entry: nwords, plus 2 lookahead unless it ends in a branch */
 uint16_t words[BLK_MAX_INSTR + 2];
 void (*code)(struct SH7095*);
} Block;

static Block g_blk[BLK_TABLE_SIZE];
static x86_codegen* g_blk_cg = NULL;     /* blocks' own code segment; flushed wholesale when full */
#define BLK_CODE_BYTES ((size_t)32 << 20)
static unsigned g_blk_max = BLK_MAX_INSTR;   /* SH2JIT_BLKMAX env overrides (diagnostic) */
uint64_t SH2JIT_BlockStats[4];   /* entries, compiles, validations failed, uncompilable */

static uint32_t blk_hash(uint32_t addr)
{
 uint32_t h = addr >> 1;
 h ^= h >> BLK_TABLE_BITS; h ^= h >> (2 * BLK_TABLE_BITS);
 return h & (BLK_TABLE_SIZE - 1);
}

static uint16_t guest_word(uint32_t addr)
{
 return *(const uint16_t*)(SH7095_FastMap[addr >> SH7095_EXT_MAP_GRAN_BITS] + addr);
}

/* Per-instruction folded DoIDIF: Pipe_ID = next | opid << 24 | EPending,
 * Pipe_IF = next-next, then the fetch timing with PC known. */
static void emit_folded_fetch(uint32_t pc_at_body, uint16_t next_w, uint16_t nextnext_w)
{
 const uint32_t id = (uint32_t)next_w | ((uint32_t)g_opid[next_w] << 24);
 x86_mov_rm(g_cg, EAX, M_Z(O(EPending)));
 if(g_fetch_int_prevent)
 {
  x86_label keep; x86_label_init(&keep);
  x86_alu_ri(g_cg, X86_AND, EAX, ~(1 << (SH7095_PEX_INT + SH7095_EPENDING_PEXBITS_SHIFT)));
  x86_test_ri(g_cg, EAX, 0xFF << SH7095_EPENDING_PEXBITS_SHIFT);
  x86_jcc(g_cg, X86_CC_NE, &keep);
  x86_alu_rr(g_cg, X86_XOR, EAX, EAX);
  x86_label_bind(g_cg, &keep);
 }
 x86_alu_ri(g_cg, X86_OR, EAX, (int32_t)id);
 x86_mov_mr(g_cg, M_Z(O(Pipe_ID)), EAX);
 x86_mov_mi32(g_cg, M_Z(O(Pipe_IF)), nextnext_w);
 /* if(timestamp < MA_until - ((PC & 2) << 28)) timestamp = MA_until;  timestamp++ */
 x86_mov_rm(g_cg, EAX, M_Z(O(timestamp)));
 x86_mov_rm(g_cg, ECX, M_Z(O(MA_until)));
 if(pc_at_body & 2)
 {
  /* threshold is MA_until - 0x20000000: only a wildly negative timestamp qualifies */
  x86_mov_rr(g_cg, EDX, ECX);
  x86_alu_ri(g_cg, X86_SUB, EDX, 0x20000000);
  x86_alu_rr(g_cg, X86_CMP, EAX, EDX);
  x86_cmov(g_cg, X86_CC_L, EAX, ECX);
 }
 else
 {
  x86_alu_rr(g_cg, X86_CMP, EAX, ECX);
  x86_cmov(g_cg, X86_CC_L, EAX, ECX);
 }
 x86_alu_ri(g_cg, X86_ADD, EAX, 1);
 x86_mov_mr(g_cg, M_Z(O(timestamp)), EAX);
}

/* The C loop's post-instruction work: DMA kludge, then exit if an event
 * is due.  (Slave check is moot: the slave is off.) */
/* Block variant.  Inside a block, SH7095_mem_timestamp and the DMA
 * penalty accumulator change only on bus accesses, and next_event_ts
 * changes only at exits (it is pinned in R13 at entry), so after a
 * non-memory instruction the event test is one compare of the
 * timestamp.  The C loop's own eff_ts / mem_timestamp update runs at
 * every exit and is idempotent, so the per-instruction store of
 * mem_timestamp is not needed within the block. */
static void emit_post_instr(x86_label* exit, bool touched_memory)
{
 { static int full = -1; if(full < 0) full = getenv("SH2JIT_FULLPOST") != NULL; if(full) touched_memory = true; }
 if(touched_memory)
 {
  x86_mov_rm(g_cg, EAX, M_Z(O(DMA_PenaltyKludgeAccum)));
  x86_alu_mr(g_cg, X86_ADD, M_Z(O(timestamp)), EAX);
  x86_mov_mi32(g_cg, M_Z(O(DMA_PenaltyKludgeAccum)), 0);
 }
 x86_mov_rm(g_cg, EAX, M_Z(O(timestamp)));
 /* The C loop stores mem_timestamp = max(mem_timestamp, timestamp) after
  * every instruction, and peripherals' event handlers catch up to the
  * stored value at exits: it must be exact at every exit, so it is kept
  * exact after every instruction (one load, one compare, one store). */
 emit_env_ptr(S0, E(mem_ts));
 x86_mov_rm(g_cg, ECX, S0, X86_NOIDX, 0, 0);
 x86_alu_rr(g_cg, X86_CMP, ECX, EAX);
 x86_cmov(g_cg, X86_CC_L, ECX, EAX);
 x86_mov_mr(g_cg, S0, X86_NOIDX, 0, 0, ECX);     /* ECX = eff_ts = max(mem_ts, ts) */
#if X86EMIT_64
 if(touched_memory)
 {
  /* a register access may have scheduled an earlier event: refresh the pin */
  emit_env_ptr(S0, E(next_event_ts));
  x86_mov_rm(g_cg, PIN_TABLE, S0, X86_NOIDX, 0, 0);
 }
 x86_alu_rr(g_cg, X86_CMP, ECX, PIN_TABLE);
 x86_jcc(g_cg, X86_CC_GE, exit);
#else
 emit_env_ptr(S0, E(next_event_ts));
 x86_alu_rm(g_cg, X86_CMP, ECX, S0, X86_NOIDX, 0, 0);
 x86_jcc(g_cg, X86_CC_GE, exit);
#endif
}

/* After a branch or a fused pair in a block: if the branch landed on the
 * block's own head (PC == head address + 4) and the next instruction is
 * an ordinary one (no delay flag), loop natively.  The state after a
 * taken branch to the head equals the entry state, and the block's own
 * stores are already detected, so no revalidation is needed. */
static void emit_loop_if_head(x86_label* head, x86_label* exit, uint32_t head_pc)
{
 x86_label no; x86_label_init(&no);
 { static int noloop = -1; if(noloop < 0) noloop = getenv("SH2JIT_NOLOOP") != NULL; if(noloop) { x86_jmp(g_cg, exit); return; } }
 x86_mov_rm(g_cg, EAX, M_Z(O(PC)));
 x86_alu_ri(g_cg, X86_CMP, EAX, (int32_t)head_pc);
 x86_jcc(g_cg, X86_CC_NE, exit);
 x86_mov_rm(g_cg, EAX, M_Z(O(Pipe_ID)));
 x86_shift_ri(g_cg, X86_SHR, EAX, 24);
 x86_alu_ri(g_cg, X86_CMP, EAX, 0x80);
 x86_jcc(g_cg, X86_CC_AE, exit);
 x86_jmp(g_cg, head);
 x86_label_bind(g_cg, &no);
}

/* Pre-instruction checks: exception pending (op byte 0xFF), FRT due. */
static void emit_pre_instr(x86_label* exit)
{
 x86_mov_rm(g_cg, EAX, M_Z(O(Pipe_ID)));
 x86_shift_ri(g_cg, X86_SHR, EAX, 24);
 x86_alu_ri(g_cg, X86_CMP, EAX, 0xFF);
 x86_jcc(g_cg, X86_CC_E, exit);
 x86_mov_rm(g_cg, EAX, M_Z(O(timestamp)));
 x86_alu_rm(g_cg, X86_CMP, EAX, M_Z(O(FRT_WDT_NextTS)));
 x86_jcc(g_cg, X86_CC_GE, exit);
}


/* Fusion sources (TST, CMP/GT, DT, CMP/EQ #imm, TST #imm) followed by
 * BF/BT execute the pair inside one handler; a block must end there. */
static bool is_fusion_pair(uint32_t w, uint32_t next)
{
 const unsigned top = w >> 12, low = w & 0xF;
 bool src = (top == 0x2 && low == 0x8) || (top == 0x3 && low == 0x7) || (top == 0x4 && (w & 0xFF) == 0x10) ||
            (w & 0xFF00) == 0x8800 || (w & 0xFF00) == 0xC800;
 bool br = (next & 0xFF00) == 0x8B00 || (next & 0xFF00) == 0x8900;
 return src && br;
}

static bool is_branch_word(uint32_t w)
{
 const unsigned top = w >> 12;
 if(top == 0xA || top == 0xB) return true;
 if(top == 0x4 && ((w & 0xFF) == 0x2B || (w & 0xFF) == 0x0B)) return true;
 if(w == 0x000B) return true;
 if(top == 0x8) { const unsigned sub = (w >> 8) & 0xF; return sub == 0x9 || sub == 0xB || sub == 0xD || sub == 0xF; }
 return false;
}

static bool compile_block(Block* b)
{
 void* start; x86_label exit, head; uint32_t i; bool ok = true;
 x86_codegen* saved_cg = g_cg;
 if(!g_blk_cg) return false;
 if(x86_codegen_offset(g_blk_cg) + 512 * (BLK_MAX_INSTR + 1) > x86_codegen_capacity(g_blk_cg))
 {
  /* segment full: drop every block and start over (the table entries
   * point into the segment; none may survive the reset) */
  SH2JIT_InvalidateBlocks();
  x86_codegen_set_wptr(g_blk_cg, x86_codegen_base(g_blk_cg));
  SH2JIT_BlockStats[3] += 1000000;   /* visible in the stats as a flush */
 }
 g_cg = g_blk_cg;
 g_in_block = true;
 x86_label_init(&exit); x86_label_init(&head);
 start = x86_codegen_wptr(g_cg);
 emit_entry_frame();
#if X86EMIT_64
 emit_env_ptr(S0, E(next_event_ts));
 x86_mov_rm(g_cg, PIN_TABLE, S0, X86_NOIDX, 0, 0);   /* R13 = next_event_ts for the block's life */
#endif
 x86_label_bind(g_cg, &head);
 g_blk_lo = b->addr; g_blk_hi = b->addr + b->nwords * 2;
 g_blk_self_write_check = true;
 for(i = 0; i < b->nwords; i++)
 {
  const uint32_t w = b->words[i];
  const uint32_t pc_at_body = b->addr + i * 2 + 4;
  bool prefetch;
  void* instr_start = x86_codegen_wptr(g_cg);
  g_blk_allow_movb = !((b->words[i + 1] & 0xFF00) == 0x8800 || (b->words[i + 1] & 0xFF00) == 0xC800);
  { static int nomovb = -1; if(nomovb < 0) nomovb = getenv("SH2JIT_NOMOVB") != NULL; if(nomovb) g_blk_allow_movb = false; }
  /* Probe first: nothing that references the block's exit label may be
   * emitted and then discarded (its patch sites would outlive it). */
  {
   bool supported;
   g_blk_self_write_check = false;
   g_fetch_emitter = emit_folded_fetch_from_globals;
   g_fold_pc = pc_at_body; g_fold_nw = b->words[i + 1]; g_fold_nnw = b->words[i + 2];
   supported = is_branch_word(w) ? compile_branch(w, &prefetch) : compile_body(w);
   g_probe_placement = g_fetch_placement;
   x86_codegen_set_wptr(g_cg, instr_start);
   g_blk_self_write_check = true;
   if(!supported)
   {
    { static int tr = -1; if(tr < 0) tr = getenv("SH2JIT_TRACEEND") != NULL; if(tr) fprintf(stderr, "E %04x\n", w); }
    if(i == 0) ok = false;
    else b->nwords = i;
    break;
   }
  }
  emit_pre_instr(&exit);
  if(is_branch_word(w))
  {
   /* branches fetch through the helpers (their pipeline effects are
    * data-dependent), so use the live fetch when they prefetch */
   x86_mov_mi32(g_cg, M_Z(O(PC)), pc_at_body);
   if(prefetch) emit_fetch_inline();
   compile_branch(w, &prefetch);
   emit_pc_advance();
   emit_post_instr(&exit, true);        /* branch helpers fetch through the bus */
   emit_loop_if_head(&head, &exit, b->addr + 4);
   /* Delayed branch back to the head: PC == head + 2 and Pipe_ID holds
    * the slot (words[i+1], validated as lookahead) with the delay flag.
    * Run the slot here, folding the head's words, then loop. */
   {
    const uint16_t slot = b->words[i + 1];
    bool sp; g_blk_self_write_check = false;
    { void* pr = x86_codegen_wptr(g_cg); sp = !is_branch_word(slot) && compile_body(slot); x86_codegen_set_wptr(g_cg, pr); }
    g_blk_self_write_check = true;
    { static int noloop = -1; if(noloop < 0) noloop = getenv("SH2JIT_NOLOOP") != NULL; if(noloop) sp = false; }
    if(sp && g_fetch_placement == FETCH_BEFORE)
    {
     x86_label not_loop; x86_label_init(&not_loop);
     x86_mov_rm(g_cg, EAX, M_Z(O(PC)));
     x86_alu_ri(g_cg, X86_CMP, EAX, (int32_t)(b->addr + 2));
     x86_jcc(g_cg, X86_CC_NE, &not_loop);
     emit_pre_instr(&exit);
     g_blk_allow_movb = !((b->words[0] & 0xFF00) == 0x8800 || (b->words[0] & 0xFF00) == 0xC800);
     emit_folded_fetch(b->addr + 2, b->words[0], b->words[1]);   /* PC at slot body = head + 2 */
     compile_body(slot);
     emit_pc_advance();
     emit_post_instr(&exit, g_body_touches_memory);
     x86_jmp(g_cg, &head);
     x86_label_bind(g_cg, &not_loop);
    }
   }
   break;                               /* block ends at a branch (or loops to its head) */
  }
  else
  {
   const uint16_t nw  = b->words[i + 1];
   const uint16_t nnw = b->words[i + 2];
   x86_mov_mi32(g_cg, M_Z(O(PC)), pc_at_body);
   {
    const FetchPlacement fp = g_probe_placement;
    g_fold_pc = pc_at_body; g_fold_nw = nw; g_fold_nnw = nnw;
    g_fetch_emitter = emit_folded_fetch_from_globals;
    g_fetch_int_prevent = false;
    if(fp == FETCH_BEFORE) emit_folded_fetch(pc_at_body, nw, nnw);
    compile_body(w);
    if(fp == FETCH_AFTER_INTDIS) { g_fetch_int_prevent = true; emit_folded_fetch(pc_at_body, nw, nnw); g_fetch_int_prevent = false; }
   }
   emit_pc_advance();
   /* a store into this block's own range: exit after this instruction */
   { static int full = -1; if(full < 0) full = getenv("SH2JIT_FULLPOST") != NULL; if(full) g_body_wrote_memory = true; }
   if(g_body_wrote_memory)
   {
    x86_label no; x86_label_init(&no);
    emit_env_base(S0);
    x86_cmp_mi8(g_cg, S0, X86_NOIDX, 0, E(self_write), 0);
    x86_jcc(g_cg, X86_CC_E, &no);
    x86_mov_mi8(g_cg, S0, X86_NOIDX, 0, E(self_write), 0);
    x86_jmp(g_cg, &exit);
    x86_label_bind(g_cg, &no);
   }
   emit_post_instr(&exit, g_body_touches_memory);
   if(is_fusion_pair(w, b->words[i + 1]))
   {
    emit_loop_if_head(&head, &exit, b->addr + 4);
    break;                              /* the handler may have consumed the branch */
   }
  }
 }
 g_blk_self_write_check = false;
 g_blk_allow_movb = false;
 g_in_block = false;
 if(ok && b->nvalid != b->nwords + 2)
 {
  /* truncated: lookahead is now the two words after the new end */
  b->words[b->nwords]     = guest_word(b->addr + b->nwords * 2);
  b->words[b->nwords + 1] = guest_word(b->addr + b->nwords * 2 + 2);
  b->nvalid = b->nwords + 2;
 }
 x86_label_bind(g_cg, &exit);
 x86_jmp_abs(g_cg, g_exit_stub);
 if(!ok || x86_codegen_overflowed(g_cg))
 {
  x86_codegen_set_wptr(g_cg, start);
  g_cg = saved_cg;
  return false;
 }
 b->code = (void (*)(struct SH7095*))start;
 g_cg = saved_cg;
 {
  /* perf: /tmp/perf-<pid>.map lines so JIT samples get names */
  static FILE* pm; static int pm_tried;
  if(!pm_tried) { pm_tried = 1; if(getenv("SH2JIT_PERFMAP")) { char n[64]; snprintf(n, sizeof n, "/tmp/perf-%d.map", (int)getpid()); pm = fopen(n, "a"); } }
  if(pm) { fprintf(pm, "%lx %lx blk_%08x_n%u\n", (unsigned long)(uintptr_t)start, (unsigned long)((uint8_t*)x86_codegen_wptr(g_blk_cg) - (uint8_t*)start), b->addr, b->nwords); fflush(pm); }
 }
 {
  static int dumped; const char* e = getenv("SH2JIT_DUMPBLK");
  if(e && !dumped && b->nwords == 2)
  {
   FILE* f = fopen("/tmp/blk.bin", "wb"); size_t n = (size_t)((uint8_t*)x86_codegen_wptr(g_cg) - (uint8_t*)start);
   if(f) { fwrite(start, 1, n, f); fclose(f); }
   f = fopen("/tmp/blk.txt", "w");
   if(f) { fprintf(f, "addr=%08x words=%04x %04x %04x %04x\n", b->addr, b->words[0], b->words[1], b->words[2], b->words[3]); fclose(f); }
   dumped = 1;
  }
 }
 return true;
}

/* Entry: called when the slave is off and Pipe_ID marks an ordinary
 * instruction.  Runs a block from the current position; returns false
 * when none can be compiled here (caller runs one step otherwise). */
bool SH2JIT_RunBlock(struct SH7095* z)
{
 const uint32_t addr = z->PC - 4;
 const uint16_t w0 = (uint16_t)z->Pipe_ID, w1 = (uint16_t)z->Pipe_IF;
 Block* b;
 uint32_t i;
 if((int32_t)addr < 0) return false;
 b = &g_blk[blk_hash(addr)];
 SH2JIT_BlockStats[0]++;
 if(b->addr == addr && !b->code && b->nvalid == 1 && b->words[0] == w0 && guest_word(addr) == w0)
  return false;                                 /* known uncompilable here */
 if(b->addr == addr && b->code)
 {
  /* validate against memory (code may have been rewritten or reloaded) */
  for(i = 0; i < b->nvalid; i++)
   if(guest_word(addr + i * 2) != b->words[i]) break;
  if(i == b->nvalid && b->words[0] == w0 && (b->nvalid < 2 || b->words[1] == w1))
  {
   static int trb = -1; if(trb < 0) trb = getenv("SH2JIT_TRACEBLK") != NULL;
   if(trb) fprintf(stderr, "B run  %08x n=%u R4=%08x R3=%08x PC=%08x ts=%d ->", addr, b->nwords, z->R[4], z->R[3], z->PC, z->timestamp);
   b->code(z);
   if(trb) fprintf(stderr, " R4=%08x R3=%08x PC=%08x ts=%d\n", z->R[4], z->R[3], z->PC, z->timestamp);
   return true;
  }
  SH2JIT_BlockStats[2]++;
  b->code = NULL;
 }
 /* compile: gather words up to a branch (inclusive) or the limit */
 b->addr = addr; b->nwords = 0; b->code = NULL;
 if(guest_word(addr) != w0) return false;       /* pipeline disagrees with memory: let the interpreter sort it out */
 for(i = 0; i < g_blk_max; i++)
 {
  const uint16_t w = guest_word(addr + i * 2);
  { static int nobr = -1; if(nobr < 0) nobr = getenv("SH2JIT_NOBRANCH") != NULL;
    if(nobr && is_branch_word(w) && b->nwords > 0) break; }
  b->words[b->nwords++] = w;
  if(is_branch_word(w)) break;
  if(is_fusion_pair(w, guest_word(addr + (i + 1) * 2))) break;
  {
   /* diagnostic: SH2JIT_BLKCLASS=1 stops before a memory op in slot >= 1, =2 before a non-memory op */
   static int cls = -1; if(cls < 0) { const char* e = getenv("SH2JIT_BLKCLASS"); cls = e ? atoi(e) : 0; }
   if(cls)
   {
    const uint16_t nw = guest_word(addr + (i + 1) * 2);
    const unsigned t = nw >> 12, l = nw & 0xF, n1 = (nw >> 8) & 0xF;
    bool mem = (t == 0 && (l >= 4 && l <= 6 || l >= 0xC && l <= 0xE)) || t == 1 || (t == 2 && (l <= 2 || (l >= 4 && l <= 6))) || t == 5 ||
               (t == 6 && (l <= 2 || (l >= 4 && l <= 6))) || (t == 8 && (n1 <= 1 || n1 == 4 || n1 == 5)) || t == 9 || (t == 0xC && n1 <= 7) || t == 0xD;
    if((cls == 1 && mem) || (cls == 2 && !mem && !is_branch_word(nw))) break;
   }
  }
 }
 /* Every instruction's folded fetch uses the two words after it, so the
  * two words past the block are always gathered and validated; for a
  * block ending in a branch they are its delay slot and the word after,
  * which the instruction before the branch folds into Pipe_IF. */
 b->words[b->nwords]     = guest_word(addr + b->nwords * 2);
 b->words[b->nwords + 1] = guest_word(addr + b->nwords * 2 + 2);
 b->nvalid = b->nwords + 2;
 SH2JIT_BlockStats[1]++;
 if(!compile_block(b))
 {
  /* remember the failure: entries with code == NULL and a matching
   * first word return false without recompiling (re-tried once the
   * word changes) */
  SH2JIT_BlockStats[3]++;
  b->code = NULL; b->nwords = 1; b->nvalid = 1; b->words[0] = w0;
  return false;
 }
 {
  static int trb = -1; if(trb < 0) trb = getenv("SH2JIT_TRACEBLK") != NULL;
  if(trb) fprintf(stderr, "B new  %08x n=%u words=%04x %04x %04x %04x R4=%08x PC=%08x ts=%d ->", addr, b->nwords, b->words[0], b->words[1], b->words[2], b->words[3], z->R[4], z->PC, z->timestamp);
  b->code(z);
  if(trb) fprintf(stderr, " R4=%08x PC=%08x ts=%d\n", z->R[4], z->PC, z->timestamp);
 }
 return true;
}

void SH2JIT_InvalidateBlocks(void)
{
 memset(g_blk, 0, sizeof g_blk);
}

static bool g_compile_capacity_fail;   /* compile() refused for lack of space, not for lack of a body */

static void (*compile(uint32_t instr))(struct SH7095*)
{
 void* start;
 if(!g_cg) return NULL;
 if(x86_codegen_offset(g_cg) + 512 > x86_codegen_capacity(g_cg)) { g_compile_capacity_fail = true; return NULL; }
 start = x86_codegen_wptr(g_cg);
 g_body_touches_memory = false;
 {
  bool prefetch;
  void* probe = x86_codegen_wptr(g_cg);
  /* Branches choose their own prefetch; compile speculatively, undo if
   * the word is not a branch. */
  x86_codegen_set_wptr(g_cg, probe);
  if(compile_branch(instr, &prefetch))
  {
   /* re-emit in the right order: prefetch (if any), then the body */
   x86_codegen_set_wptr(g_cg, start);
   if(prefetch) emit_fetch_inline();
   compile_branch(instr, &prefetch);
   g_body_touches_memory = false;               /* branches only read memory: the self-check may run them on a copy */
  }
  else
  {
   /* placement is known only after compile_body: probe, then emit */
   x86_codegen_set_wptr(g_cg, start);
   g_fetch_emitter = emit_fetch_inline;
   if(!compile_body(instr)) { x86_codegen_set_wptr(g_cg, start); return NULL; }
   {
    const FetchPlacement fp = g_fetch_placement;
    x86_codegen_set_wptr(g_cg, start);
    g_fetch_int_prevent = false;
    if(fp == FETCH_BEFORE) emit_fetch_inline();
    compile_body(instr);
    if(fp == FETCH_AFTER_INTDIS) { g_fetch_int_prevent = true; emit_fetch_inline(); g_fetch_int_prevent = false; }
   }
  }
 }
 emit_pc_advance();
 x86_jmp_abs(g_cg, g_dispatch_stub);
 if(x86_codegen_overflowed(g_cg))
 {
  x86_codegen_set_wptr(g_cg, start);
  return NULL;
 }
 return (void (*)(struct SH7095*))start;
}

void SH2JIT_Init(struct SH7095* master, struct SH7095* slave, int32_t* mem_ts, const int32_t* next_event_ts, int32_t quantum)
{
 g_env.quantum = quantum;
 { const char* e = getenv("SH2JIT_BLKMAX"); if(e && atoi(e) > 0 && atoi(e) <= BLK_MAX_INSTR) g_blk_max = atoi(e); }
 g_master = master; g_slave = slave; g_slave_ts = &slave->timestamp; g_mem_ts = mem_ts; g_next_event_ts = next_event_ts;
 g_env.master = master; g_env.slave = slave; g_env.mem_ts = mem_ts; g_env.next_event_ts = next_event_ts;
 memset(SH2JIT_Table, 0, sizeof SH2JIT_Table);
 memset(g_status, 0, sizeof g_status);
 memset(SH2JIT_Pure, 0, sizeof SH2JIT_Pure);
 SH7095_JIT_BuildOpIDTable(g_opid);
 if(!g_blk_cg)
  g_blk_cg = x86_codegen_create(BLK_CODE_BYTES);
 if(!g_cg)
 {
  void* p;
  g_cg = x86_codegen_create(SH2JIT_CODE_BYTES);
  if(!g_cg) return;
  /* exit stub, dispatch stub, then the entry stub (frame + dispatch) */
  p = x86_codegen_wptr(g_cg);
  emit_exit_frame();
  g_exit_stub = p;
  p = x86_codegen_wptr(g_cg);
  emit_dispatch_stub();
  g_dispatch_stub = p;
  p = x86_codegen_wptr(g_cg);
  emit_entry_frame();
  x86_jmp_abs(g_cg, g_dispatch_next);   /* first instruction runs unconditionally */
  g_entry_stub = (void (*)(struct SH7095*))p;
  if(getenv("SH2JIT_PERFMAP")) { char n[64]; FILE* f; snprintf(n, sizeof n, "/tmp/perf-%d.map", (int)getpid()); f = fopen(n, "a");
   if(f) { fprintf(f, "%lx %lx sh2jit_stubs\n", (unsigned long)(uintptr_t)g_exit_stub, (unsigned long)((uint8_t*)x86_codegen_wptr(g_cg) - (uint8_t*)g_exit_stub)); fclose(f); } }
 }
}

void SH2JIT_RunChain(struct SH7095* z)
{
 g_entry_stub(z);
}

void SH2JIT_SetSingleStep(bool on)
{
 g_single_step = on;
 g_env.single_step = on;
}

void SH2JIT_SetCounting(bool on)
{
 g_counting = on;   /* must precede SH2JIT_Init: the stub is emitted once */
}

bool SH2JIT_Available(void)
{
 return g_cg != NULL;
}

void (*SH2JIT_Handler(uint32_t instr))(struct SH7095*)
{
 void (*h)(struct SH7095*);
 instr &= 0xFFFF;
 h = SH2JIT_Table[instr];
 if(h) return h;
 if(g_status[instr]) return NULL;               /* known unsupported */
 g_compile_capacity_fail = false;
 h = compile(instr);
 if(h) { SH2JIT_Table[instr] = h; g_status[instr] = 1; SH2JIT_Pure[instr] = !g_body_touches_memory;
  if(getenv("SH2JIT_PERFMAP")) { char n[64]; FILE* f; snprintf(n, sizeof n, "/tmp/perf-%d.map", (int)getpid()); f = fopen(n, "a");
   if(f) { fprintf(f, "%lx %lx hnd_%04x\n", (unsigned long)(uintptr_t)h, (unsigned long)((uint8_t*)x86_codegen_wptr(g_cg) - (uint8_t*)h), instr); fclose(f); } } }
 else if(!g_compile_capacity_fail) g_status[instr] = 2;   /* a capacity failure is not 'unsupported' */
 return h;
}

#else

void SH2JIT_Init(struct SH7095* a, struct SH7095* d, int32_t* b, const int32_t* c, int32_t q) { (void)a; (void)d; (void)b; (void)c; (void)q; }
bool SH2JIT_Available(void) { return false; }
void SH2JIT_RunChain(struct SH7095* z) { (void)z; }
void SH2JIT_SetSingleStep(bool on) { (void)on; }
void SH2JIT_SetCounting(bool on) { (void)on; }
bool SH2JIT_RunBlock(struct SH7095* z) { (void)z; return false; }
void SH2JIT_InvalidateBlocks(void) {}
uint64_t SH2JIT_BlockStats[4];
uint64_t SH2JIT_NativeCount, SH2JIT_ChainCount, SH2JIT_FallbackCount;
void (*SH2JIT_Handler(uint32_t instr))(struct SH7095*) { (void)instr; return NULL; }

#endif
