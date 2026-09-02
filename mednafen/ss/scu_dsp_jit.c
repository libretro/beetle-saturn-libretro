/******************************************************************************/
/* Mednafen Sega Saturn Emulation Module                                      */
/******************************************************************************/
/* scu_dsp_jit.c - SCU DSP JIT (aarch64 backend) implementation
**  Copyright (C) 2026 pstef
*/

/*
 * Register allocation map (AArch64, AAPCS).  Anything not listed here
 * is either standard caller-save scratch (x0-x18, used freely for
 * intermediate values) or untouched.
 *
 *   x0   = DSPS* throughout the chain.
 *   x3   = ALU.T scratch inside emit_gen.
 *   x4-x9, x12 = generic per-emitter scratches.
 *   x13-x15 = cross-slot DataRAM preload carriers, loaded sign-extended
 *          (see DRReadSet / emit_dram_preloads).  Only gen slot bodies,
 *          which never BLR to C, run between a preload and its use.
 *   x16, x17 = MOVP2R staging and BR/BLR targets.
 *   x19  = pinned dispatch anchor: &DSP_Init + SCU_JIT_SLOT_PRELUDE_BYTES.
 *          Re-established by the shared prelude stub on every cold entry,
 *          so tail dispatch is one ADD (extended register) of the signed
 *          NI.low32 offset.
 *   w20  = pinned dsp->LOP (12-bit loop counter).
 *   w21  = pinned dsp->CycleCounter.
 *   w22  = pinned dsp->State (read-only cache).
 *   w23  = pinned dsp->CT32 (4 packed 6-bit CT counters).
 *   w24  = pinned packed flag bytes (FlagZ/S/V/C at byte 0..3).
 *   x25  = pinned dsp->NextInstr, full 64 bits (high32 = raw instr,
 *          low32 = threaded-dispatch offset; tail dispatch reads W25).
 *   x26  = pinned dsp->AC.T.
 *   w27  = pinned dsp->PC.
 *   x28  = pinned dsp->P.T.
 *
 * Memory-coherence contract: slot bodies do not flush NI/PC/flags/CC/
 * LOP/CT32/AC.T/P.T per slot.  emit_flush_pins() writes them back
 * wherever C code can observe DSPS: the exit stub, the top of
 * emit_call_helper_addr, and the hot paths of the DMA fallback and the
 * fallback thunks before their BR to C.  TOP and RX/RY have no pins and
 * store straight to memory; NextInstrLooped is set only by lps_helper
 * and cleared in place by the looped refetch path.
 *
 * Dispatch-offset invariant: tail dispatch enters its target at
 * +SCU_JIT_SLOT_PRELUDE_BYTES, so every NextInstr/ProgRAM[].low32 value
 * reaching it must point into the JIT segment, never at a raw C handler.
 * While the JIT is active, DSP_DecodeInstruction returns
 * SCU_DSP_JIT_FallbackNI's thunk for every C-handler decode.
 *
 * Entry-stub frame: 96 bytes, preserving x19/x20, x21/x22, x25/x26,
 * x23/x24 and x27/x28.  Slot bodies push no frame; their tail dispatch
 * B's to the exit stub, which flushes the pins and RETs through the
 * entry stub's after-BLR.
 */

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "ss.h"
#include "scu.h"
#include "scu_dsp_jit.h"
#include "a64emit.h"
#include "jitdump.h"

void (*SCU_DSP_JIT_Entry)(struct DSPS*) = NULL;

#if defined(WANT_JIT) && (defined(__aarch64__) || defined(__arm64__))

#include "scu_dsp_common.inc"

#ifdef WANT_DSP_JIT_PERF_DUMP
#include <stdio.h>
#endif

/*
 * Single 1 MB code segment, bump-allocated per slot.  On overflow the
 * write pointer rewinds to the post-stubs offset and rewind_locked()
 * recompiles every DSP.ProgRAM[] entry (plus DSP.NextInstr), so no JIT
 * pointer cached in the DSP state outlives the bytes it points to.
 */
#define SCU_JIT_CODE_SEGMENT_SIZE  ((size_t)0x100000)
#define SCU_JIT_SLOT_MAX_BYTES     ((size_t)1024)

/* Cold-entry size of every slot, in bytes: ADR X16,body + B to the
 * shared prelude stub (emit_cold_entry).  Tail dispatch skips it via the
 * X19 anchor, pre-biased by this amount; the DMA fallback pads with this
 * many NOP bytes to stay skip-safe. */
#define SCU_JIT_SLOT_PRELUDE_BYTES 8u

/* AArch64 register-index conventions.  WZR/XZR/SP all encode as 31.
 * Numeric in source so a64emit accepts them as plain `unsigned`s. */
#define W0  0u
#define W1  1u
#define W2  2u
#define W3  3u
#define W4  4u
#define W5  5u
#define W6  6u
#define W7  7u
#define W8  8u
#define W9  9u
#define W10 10u
#define W11 11u
#define W12 12u
#define W13 13u
#define W14 14u
#define W15 15u
#define W16 16u
#define W17 17u
#define W18 18u
#define W19 19u
#define W20 20u
#define W21 21u
#define W22 22u
#define W23 23u
#define W24 24u
#define W25 25u
#define W26 26u
#define W27 27u
#define W28 28u
#define W29 29u
#define W30 30u
#define WZR 31u

#define X0  0u
#define X1  1u
#define X2  2u
#define X3  3u
#define X4  4u
#define X5  5u
#define X6  6u
#define X7  7u
#define X8  8u
#define X9  9u
#define X10 10u
#define X11 11u
#define X12 12u
#define X13 13u
#define X14 14u
#define X15 15u
#define X16 16u
#define X17 17u
#define X19 19u
#define X20 20u
#define X21 21u
#define X22 22u
#define X23 23u
#define X24 24u
#define X25 25u
#define X26 26u
#define X27 27u
#define X28 28u
#define X29 29u
#define X30 30u
#define XZR 31u
#define SP_REG 31u

/* --- DSPS field byte offsets ------------------------------------- */
#define O_CC         ((uint32_t)offsetof(struct DSPS, CycleCounter))
#define O_T0_Until   ((uint32_t)offsetof(struct DSPS, T0_Until))
#define O_State      ((uint32_t)offsetof(struct DSPS, State))
#define O_NI         ((uint32_t)offsetof(struct DSPS, NextInstr))
#define O_NILooped   ((uint32_t)offsetof(struct DSPS, NextInstrLooped))
#define O_PC         ((uint32_t)offsetof(struct DSPS, PC))
#define O_FZ         ((uint32_t)offsetof(struct DSPS, FlagZ))
#define O_FS         ((uint32_t)offsetof(struct DSPS, FlagS))
#define O_FV         ((uint32_t)offsetof(struct DSPS, FlagV))
#define O_FC         ((uint32_t)offsetof(struct DSPS, FlagC))
#define O_FlagEnd    ((uint32_t)offsetof(struct DSPS, FlagEnd))
#define O_LOP        ((uint32_t)offsetof(struct DSPS, LOP))
#define O_TOP        ((uint32_t)offsetof(struct DSPS, TOP))
#define O_AC         ((uint32_t)offsetof(struct DSPS, AC))
#define O_P          ((uint32_t)offsetof(struct DSPS, P))
#define O_P_L        (O_P + 0u)
#define O_CT32       ((uint32_t)offsetof(struct DSPS, CT32))
#define O_RX         ((uint32_t)offsetof(struct DSPS, RX))
#define O_RY         ((uint32_t)offsetof(struct DSPS, RY))
#define O_RAO        ((uint32_t)offsetof(struct DSPS, RAO))
#define O_WAO        ((uint32_t)offsetof(struct DSPS, WAO))
#define O_DRAM       ((uint32_t)offsetof(struct DSPS, DataRAM))
#define O_PRAM       ((uint32_t)offsetof(struct DSPS, ProgRAM))
#define O_PRAMDMACt  ((uint32_t)offsetof(struct DSPS, PRAMDMABufCount))

/* --- Codegen + label pool ---------------------------------------- */

/* Per-Compile pool; emit_* sites use at most ~3 live labels per slot. */
#define LABEL_POOL_SIZE 16u

static a64_codegen* g_cg          = NULL;
static void*        g_seg_start   = NULL;
static a64_label    g_label_pool[LABEL_POOL_SIZE];
static size_t       g_label_count = 0;

static a64_label* label_new(void)
{
 a64_label* p;
 if(g_label_count >= LABEL_POOL_SIZE) return NULL;
 p = &g_label_pool[g_label_count++];
 a64_label_reset(p);
 return p;
}
static void label_bind(a64_label* lbl) { a64_label_bind(g_cg, lbl); }
static void labels_reset(void)
{
 memset(g_label_pool, 0, sizeof(g_label_pool));
 g_label_count = 0;
}

/* AND Wd, Wn, #imm with a MOV+AND reg fallback when imm isn't encodable
 * as an AArch64 logical immediate. */
static void emit_and_w_imm_safe(unsigned wd, unsigned wn, uint32_t imm, unsigned scratch)
{
 if(!a64_and_w_imm(g_cg, wd, wn, imm))
 {
  a64_mov_w_imm(g_cg, scratch, imm);
  a64_and_w_reg(g_cg, wd, wn, scratch);
 }
}

/* ADD Xd, Xn, #imm with a MOV+ADD reg fallback when `imm` doesn't fit
 * the AddSubImm encoding, covering DSPS offsets past the direct/shifted
 * range.  Requires xd != xn: the staging MOV writes xd's low 32 bits,
 * then add_x_reg reads xn. */
static void emit_add_x_imm_safe(unsigned xd, unsigned xn, uint32_t imm)
{
 if(!a64_try_add_x_imm(g_cg, xd, xn, imm))
 {
  a64_mov_w_imm(g_cg, xd, imm);
  a64_add_x_reg(g_cg, xd, xn, xd);
 }
}

/* --- Stubs / globals --------------------------------------------- */

static const void* g_exit_stub_addr = NULL;
/* Shared CC-decrement/checks/indirect-dispatch stub (emit_dispatch_stub),
 * emitted once below g_post_stub_byte_offset, falling through into the exit
 * stub.  Every unlinked slot tail is a single B here. */
static const void* g_dispatch_stub_addr = NULL;
/* Shared no-op-run coalescing stub (emit_coalesce_stub), emitted once below
 * g_post_stub_byte_offset so no rewind invalidates it.  Every coalesced gen
 * slot loads its run's constants into w4/w5/w6 and branches here. */
static const void* g_coalesce_stub_addr = NULL;
/* Shared cold-reconstruction prelude (emit_prelude_stub), emitted once below
 * g_post_stub_byte_offset.  Every slot's 2-instruction cold entry branches
 * here to reload the cross-block pins, then the stub BR X16's back into the
 * slot body.  The hot JIT-to-JIT chain skips it entirely (X19 anchor bias). */
static const void* g_prelude_stub_addr = NULL;
static size_t      g_post_stub_byte_offset = 0;

/* Permanent fallback thunks (indexed by `looped`), emitted once below
 * g_post_stub_byte_offset so no rewind ever invalidates them.  See
 * SCU_DSP_JIT_FallbackNI. */
static const void* g_fallback_thunk[2] = { NULL, NULL };

/* Chain-live flag + deferred-rewind latch; see jit_entry() and the
 * segment-full check in SCU_DSP_JIT_CompileSlot. */
static bool g_chain_live     = false;
static bool g_rewind_pending = false;
/* True only while rewind_locked() recompiles the whole segment.  No-op-run
 * coalescing keys off it: only there is the entire DSP.ProgRAM[] populated,
 * so the forward look-ahead that measures a run is valid. */
static bool g_in_rewind      = false;
/* True only across rewind_locked()'s 256-slot recompile loop (pass 1).
 * Block linking keys off this, not g_in_rewind: the trailing NextInstr
 * re-decode also compiles under g_in_rewind but runs after the pass-2
 * patch loop, so a link site recorded there is never patched. */
static bool g_in_rewind_slots = false;
static void (*g_entry_stub)(struct DSPS*) = NULL;

/* --- Block linking (aarch64 direct-B chaining) -------------------------
 * When the predecessor cannot perturb PC and the slot keeps X25=NI intact,
 * the indirect tail dispatch (add x19,w25 ; br) becomes a direct B to
 * slot-(pc+1)'s hot entry.  The successor's address is known only once
 * every slot is compiled and moves on each rewind, so the tail emits a
 * placeholder B recorded here for rewind_locked's second pass.  Populated
 * only during the rewind slot pass (g_in_rewind_slots), so every recorded
 * site is reached by the patch loop. */
static void*   g_link_site[256];        /* placeholder-B address per pc, NULL = unlinked */
static bool    g_link_this_slot = false;/* set by CompileSlot for a linkable slot */
static uint8_t g_link_this_pc   = 0;    /* pc whose tail records into g_link_site */

/* NextInstr-as-immediate: a slot with a linear entry fetches
 * NI = ProgRAM[pc+1], known at compile time, so emit_instr_pre
 * materializes it as a fixed-length MOVZ+MOVK chain instead of the
 * indexed ProgRAM load.  Successor ProgRAM entries are still stale during
 * the rewind slot pass, so the emitted value is a placeholder recorded
 * here for the second pass.  A PRAM rewrite between rewinds leaves it
 * stale for the rest of the live chain; the per-slot recompile latches
 * g_rewind_pending, so the next chain entry repairs it. */
static void*   g_nic_site[256];         /* mov_x_imm4 site per pc, NULL = load kept */
static bool    g_nic_this_slot = false; /* set by CompileSlot for a linear-entry slot */
static uint8_t g_nic_this_pc   = 0;     /* pc whose instr-pre records into g_nic_site */

/* Cross-slot DataRAM read pipelining.  Every read in a slot uses the
 * slot's entry CT (increments are aggregated into one post-body update),
 * so at any dispatch site that statically knows the next slot, W23
 * already holds that slot's entry CT and its reads can issue a whole
 * slot early:
 *
 *  - a linked predecessor's tail (emit_tail_dispatch) preloads the
 *    carriers, and the pass-2 patch aims its direct B past the
 *    successor's refill block (g_pipe_delta below);
 *  - a self-pipelined looped gen slot preloads at its own loop-back
 *    tail (emit_looped_pipe_tail);
 *  - every other entry (indirect hot dispatch at
 *    +SCU_JIT_SLOT_PRELUDE_BYTES, cold entry via the shared prelude)
 *    lands on the refill block, which loads the same carriers in-body.
 *
 * Preloads sit after every DataRAM write of the emitting slot, so program
 * order keeps them alias-correct.  Carriers are X13..X15, loaded by LDRSW
 * so a MOV covers the X26/X28 (AC/P) consumers and the low word covers
 * the 32-bit ones.  Predecessor and successor compiles derive the same
 * positional bank->carrier map from the same instr (dr_read_set). */
typedef struct {
 unsigned nbanks;
 uint8_t  bank[3];
} DRReadSet;

/* Distinct DataRAM banks a gen instr reads (x_op, y_op, d1 alt-source),
 * in first-read order; empty for anything else.  bank[i] loads into
 * X13+i. */
static DRReadSet dr_read_set(uint32_t instr)
{
 DRReadSet rs;
 unsigned x_op, y_op, d1_op, k, nwant;
 uint8_t want[3];

 rs.nbanks = 0;
 if(((instr >> 28) & 0xFu) > 0x3u)
  return rs;

 x_op  = (instr >> 23) & 0x7;
 y_op  = (instr >> 17) & 0x7;
 d1_op = (instr >> 12) & 0x3;
 nwant = 0;

 if(x_op >= 0x3)
  want[nwant++] = ((instr >> 20) & 0x7) & 0x3;
 if(y_op >= 0x3)
  want[nwant++] = ((instr >> 14) & 0x7) & 0x3;
 if((d1_op & 0x1) && (d1_op & 0x2) && (instr & 0xF) <= 0x7)
  want[nwant++] = instr & 0x3;

 for(k = 0; k < nwant; ++k)
 {
  unsigned i;
  for(i = 0; i < rs.nbanks; ++i)
   if(rs.bank[i] == want[k])
    break;
  if(i == rs.nbanks)
   rs.bank[rs.nbanks++] = want[k];
 }
 return rs;
}

static bool      g_pipe_this_slot = false; /* set by CompileSlot: the gen body consumes carriers */
static DRReadSet g_pipe_rs;                /* this slot's read set when g_pipe_this_slot */
static uint8_t   g_pipe_this_pc   = 0;     /* pc whose refill size records into g_pipe_delta */
static uint16_t  g_pipe_delta[256];        /* refill-block bytes past the hot entry, 0 = none */

/*
 * Looped-slot JIT cache.  LPS dispatches the same instruction up to 4096
 * times.  Keyed by pc, validated by the cached instr, so a PRAM write
 * that swaps the body forces a lazy recompile on the next LPS.  Cleared
 * by rewind_locked(), which invalidates every prior pointer in the
 * segment.
 */
typedef struct {
 void (*entry)(struct DSPS*);
 uint32_t instr;
} LoopedSlot;
static LoopedSlot g_looped_cache[256];

/* --- Perf jitdump symbol-kind decoder ---------------------------- */

#ifdef WANT_DSP_JIT_PERF_DUMP
/*
 * Decodes the opcode top-nibble into a short string so each slot shows
 * up in `perf report` as dsp_<l|n>_pc<XX>_<gen|mvi|dma|jmp|msc>.
 */
static const char* jitdump_kind_str(uint32_t instr)
{
 const unsigned top = (instr >> 28) & 0xFu;
 if(top <= 0x3u)                 return "gen";
 if(top >= 0x8u && top <= 0xBu)  return "mvi";
 if(top == 0xCu)                 return "dma";
 if(top == 0xDu)                 return "jmp";
 if(top >= 0xEu)                 return "msc";
 return "unk";
}
#else /* !WANT_DSP_JIT_PERF_DUMP */
static inline const char* jitdump_kind_str(uint32_t i) { (void)i; return ""; }
#endif

/* --- Local helpers ----------------------------------------------- */

/*
 * Pick the templated C handler that DSP_DecodeInstruction would have
 * returned, mirroring the opcode-kind switch in scu_dsp_common.inc.
 */
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

/*
 * C re-dispatch targets of the fallback thunks: pick the templated C
 * handler for the instr sitting in NextInstr.high32 and tail-call it.
 * The looped flavor is baked per thunk so the handler choice mirrors
 * DSP_DecodeInstruction's decode-time `looped` argument exactly.
 */
static void fallback_shim_normal(struct DSPS* dsp)
{
 pick_c_handler(false, (uint32_t)(dsp->NextInstr >> 32))(dsp);
}

static void fallback_shim_looped(struct DSPS* dsp)
{
 pick_c_handler(true, (uint32_t)(dsp->NextInstr >> 32))(dsp);
}

static void rewind_locked(void)
{
 unsigned i;
 if(!g_cg) return;
 g_rewind_pending = false;
 g_in_rewind = true;
 g_in_rewind_slots = true;
 a64_codegen_set_wptr(g_cg, (char*)g_seg_start + g_post_stub_byte_offset);
 labels_reset();
 for(i = 0; i < 256; ++i)
 {
  g_looped_cache[i].entry = NULL;
  g_link_site[i] = NULL;
  g_nic_site[i] = NULL;
  g_pipe_delta[i] = 0;
 }

 /* The rewind invalidated every JIT pointer cached in
  * DSP.ProgRAM[].low32 / DSP.NextInstr.low32, so re-decode every slot
  * here from the raw instr in its own high32.  256 slots *
  * SCU_JIT_SLOT_MAX_BYTES (1 KB) fits the 1 MB segment, so no inner
  * overflow can re-trigger rewind_locked. */
 for(i = 0; i < 256; ++i)
 {
  const uint32_t instr = (uint32_t)(DSP.ProgRAM[i] >> 32);
  DSP.ProgRAM[i] = DSP_DecodeSlotInstruction((uint8_t)i, instr, false);
 }
 g_in_rewind_slots = false;

 /* Second pass: every slot is compiled, so patch each linkable slot's
  * placeholder tail B to slot-(pc+1)'s +SCU_JIT_SLOT_PRELUDE_BYTES hot
  * entry.  a64_patch_b fails only on an out-of-range offset, which the
  * 1 MB segment cannot produce; the placeholder's branch to the exit
  * stub stays as the correct slower fallback. */
 for(i = 0; i < 256; ++i)
 {
  const unsigned succ     = (i + 1u) & 0xFFu;
  const int32_t  succ_off = (int32_t)(uint32_t)(DSP.ProgRAM[succ] & 0xFFFFFFFFu);
  const void*    succ_hot;
  /* NI-immediate sites: the value emitted during the slot pass used the
   * successor's then-stale ProgRAM entry; rewrite with the final one. */
  if(g_nic_site[i])
  {
   a64_patch_mov_x_imm4(g_nic_site[i], DSP.ProgRAM[succ]);
   a64_codegen_invalidate(g_cg, g_nic_site[i], 4u * sizeof(uint32_t));
  }
  if(!g_link_site[i]) continue;
  /* The tail preloaded the successor's DataRAM reads exactly when the
   * successor compiled a refill block (both from the same ProgRAM instr
   * via dr_read_set), so the direct B skips it; g_pipe_delta[succ] is 0
   * for a non-pipelined slot. */
  succ_hot = (const void*)((const char*)DSP_INSTR_BASE_UIPT
                           + succ_off + (int)SCU_JIT_SLOT_PRELUDE_BYTES
                           + (int)g_pipe_delta[succ]);
  if(a64_patch_b(g_link_site[i], succ_hot))
   a64_codegen_invalidate(g_cg, g_link_site[i], sizeof(uint32_t));
 }

 {
  const uint32_t instr = (uint32_t)(DSP.NextInstr >> 32);
  const bool looped = DSP.NextInstrLooped;
  /* The looped JIT cache was cleared above, so this dispatch goes to the
   * C looped handler via the fallback thunk; the next lps_helper or
   * InstrPre re-establishes JIT. */
  DSP.NextInstr = looped
   ? DSP_DecodeInstruction(instr, true)
   : DSP_DecodeSlotInstruction(0, instr, false);
 }
 g_in_rewind = false;
}

/* --- Instruction emitters ---------------------------------------- */

/* Compile-time aggregation: x_op, y_op and the d1 alt-source switch
 * each contribute to dr_read/ct_inc.  The final d switch consults the
 * accumulated dr_read to skip duplicate DataRAM writes; d in C..F
 * folds a byte-clear into ct_inc. */
typedef struct {
 uint32_t dr_read;
 uint32_t ct_inc;
} GenMeta;

static GenMeta compute_meta(unsigned x_op, unsigned y_op, unsigned d1_op, uint32_t instr)
{
 GenMeta m;
 m.dr_read = 0u;
 m.ct_inc  = 0u;

 if(x_op >= 0x3)
 {
  const unsigned s = (instr >> 20) & 0x7;
  const unsigned drw = s & 0x3;
  m.dr_read |= 1u << drw;
  if(s & 0x4) m.ct_inc |= 1u << (drw * 8);
 }
 if(y_op >= 0x3)
 {
  const unsigned s = (instr >> 14) & 0x7;
  const unsigned drw = s & 0x3;
  m.dr_read |= 1u << drw;
  if(s & 0x4) m.ct_inc |= 1u << (drw * 8);
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

/* X register carrying DataRAM[bank][CT] for this slot's pipelined body,
 * or 0 when the slot is not pipelined. */
static unsigned dr_carrier(unsigned bank)
{
 unsigned i;
 if(!g_pipe_this_slot)
  return 0;
 for(i = 0; i < g_pipe_rs.nbanks; ++i)
  if(g_pipe_rs.bank[i] == bank)
   return X13 + i;
 return 0;
}

/* One sign-extending carrier load per distinct bank in `rs`.  The CT
 * index comes out of W23, so this must run where W23 holds the consuming
 * slot's entry CT: its own refill block, a linked predecessor's tail
 * (post CT-update), or a looped slot's loop-back tail. */
static void emit_dram_preloads(const DRReadSet* rs)
{
 unsigned i;
 for(i = 0; i < rs->nbanks; ++i)
 {
  const unsigned b = rs->bank[i];
  a64_ubfx_w(g_cg, W4, W23, 8u * b, 6u);
  emit_add_x_imm_safe(X5, X0, O_DRAM + b * 256u);
  a64_ldrsw_x_idx_lsl(g_cg, X13 + i, X5, X4, 2u);
 }
}

static void emit_instr_pre(bool looped)
{
 if(!looped)
 {
  /* W27 pin = dsp->PC byte (zero-extended).  Fetch straight into the
   * X25 NI pin; NI/PC reach memory via emit_flush_pins() only.  No
   * NextInstrLooped clear: a normal slot only executes with the memory
   * byte already 0. */
  if(g_nic_this_slot)
  {
   /* Linear entry: when runtime PC == pc+1 the fetch result is the
    * compile-time constant ProgRAM[pc+1] (see g_nic_site).  The static
    * predicate (predecessor can't perturb PC) is necessary but not
    * sufficient: a branch executing in another branch's delay slot
    * lands its own delay slot -- this very slot -- with PC equal to
    * the second target, and BTM's TOP makes that unpredictable.  Guard
    * at runtime: on a mismatch, bail to the fallback thunk's hot entry,
    * which flushes the live pins (X25 still holds this slot's NI) and
    * re-dispatches this instruction through the C handler, whose
    * InstrPre reads the real PC.  Cost on the hot path is one CMP and
    * one never-taken B.NE. */
   a64_cmp_w_imm(g_cg, W27, (uint32_t)((g_nic_this_pc + 1u) & 0xFFu));
   a64_b_cond_addr(g_cg, A64_COND_NE,
                   (const char*)g_fallback_thunk[0] + SCU_JIT_SLOT_PRELUDE_BYTES);
   g_nic_site[g_nic_this_pc] = a64_codegen_wptr(g_cg);
   a64_mov_x_imm4(g_cg, X25, DSP.ProgRAM[(uint8_t)(g_nic_this_pc + 1u)]);
  }
  else
  {
   emit_add_x_imm_safe(X4, X0, O_PRAM);
   a64_ldr_x_idx_lsl(g_cg, X25, X4, X27, 3u);
  }
  a64_add_w_imm(g_cg, W27, W27, 1u);
  a64_and_w_imm(g_cg, W27, W27, 0xFFu);
 }
 else
 {
  a64_label* skip_load = label_new();
  a64_cbnz_w(g_cg, W20, skip_load);
  emit_add_x_imm_safe(X5, X0, O_PRAM);
  a64_ldr_x_idx_lsl(g_cg, X25, X5, X27, 3u);
  /* The one 1->0 NextInstrLooped transition: leaving the loop.  Kept
   * as an in-place store so the memory byte never needs a flush. */
  a64_strb_w_imm(g_cg, WZR, X0, O_NILooped);
  a64_add_w_imm(g_cg, W27, W27, 1u);
  a64_and_w_imm(g_cg, W27, W27, 0xFFu);
  label_bind(skip_load);
  a64_sub_w_imm(g_cg, W20, W20, 1u);
  a64_and_w_imm(g_cg, W20, W20, 0xFFFu);
 }
}

/*
 * Set FlagS/FlagZ from the most recent flag-setting op.  W24 is the
 * pinned packed-flags register; byte 0 = FlagZ, byte 1 = FlagS.
 */
static void emit_store_sz(void)
{
 a64_cset_w(g_cg, W7, A64_COND_MI); a64_bfi_w(g_cg, W24, W7, 8, 1);
 a64_cset_w(g_cg, W7, A64_COND_EQ); a64_bfi_w(g_cg, W24, W7, 0, 1);
}

/*
 * FlagV |= V from the most recent flag-setting op (byte 2 of W24).
 */
static void emit_or_flagv(void)
{
 a64_cset_w(g_cg, W7, A64_COND_VS);
 a64_orr_w_reg_lsl(g_cg, W24, W24, W7, 16u);
}

/*
 * Emit the alu sub-block.  X3 holds ALU.T on entry (loaded by caller).
 */
static void emit_alu_op(unsigned alu_op)
{
 switch(alu_op)
 {
  case 0x01: /* AND */
   a64_ands_w_reg(g_cg, W6, W3, W28);
   a64_bfi_w(g_cg, W24, WZR, 24, 1);
   emit_store_sz();
   a64_bfi_x(g_cg, X3, X6, 0, 32);
   break;

  case 0x02: /* OR */
   a64_orr_w_reg(g_cg, W6, W3, W28);
   a64_bfi_w(g_cg, W24, WZR, 24, 1);
   a64_tst_w_reg(g_cg, W6, W6);
   emit_store_sz();
   a64_bfi_x(g_cg, X3, X6, 0, 32);
   break;

  case 0x03: /* XOR */
   a64_eor_w_reg(g_cg, W6, W3, W28);
   a64_bfi_w(g_cg, W24, WZR, 24, 1);
   a64_tst_w_reg(g_cg, W6, W6);
   emit_store_sz();
   a64_bfi_x(g_cg, X3, X6, 0, 32);
   break;

  case 0x04: /* ADD */
   a64_adds_w_reg(g_cg, W6, W3, W28);
   a64_cset_w(g_cg, W7, A64_COND_CS); a64_bfi_w(g_cg, W24, W7, 24, 1);
   emit_store_sz();
   emit_or_flagv();
   a64_bfi_x(g_cg, X3, X6, 0, 32);
   break;

  case 0x05: /* SUB */
   a64_subs_w_reg(g_cg, W6, W3, W28);
   a64_cset_w(g_cg, W7, A64_COND_CC); a64_bfi_w(g_cg, W24, W7, 24, 1);
   emit_store_sz();
   emit_or_flagv();
   a64_bfi_x(g_cg, X3, X6, 0, 32);
   break;

  case 0x06: /* AD2 (48-bit add of ALU.T low 48 + P.T low 48) */
  {
   const uint64_t mask48 = 0x0000FFFFFFFFFFFFULL;
   a64_mov_x_imm(g_cg, X10, mask48);
   a64_and_x_reg(g_cg, X11, X3, X10);
   a64_and_x_reg(g_cg, X12, X28, X10);
   a64_add_x_reg(g_cg, X6, X11, X12);

   /* FlagV |= ((~(a^b)) & (a^tmp)) >> 47 & 1 */
   a64_eor_x_reg(g_cg, X7, X3, X28);
   a64_eor_x_reg(g_cg, X8, X3, X6);
   a64_bic_x_reg(g_cg, X9, X8, X7);
   a64_lsr_x_imm(g_cg, X9, X9, 47);
   a64_and_w_imm(g_cg, W9, W9, 0x1u);
   a64_orr_w_reg_lsl(g_cg, W24, W24, W9, 16u);

   /* C = (tmp >> 48) & 1 */
   a64_lsr_x_imm(g_cg, X9, X6, 48);
   a64_and_w_imm(g_cg, W9, W9, 0x1u);
   a64_bfi_w(g_cg, W24, W9, 24, 1);

   /* CalcZS48: val = tmp << 16; FlagS = (int64)val < 0; FlagZ = !val */
   a64_lsl_x_imm(g_cg, X10, X6, 16);
   a64_tst_x_reg(g_cg, X10, X10);
   emit_store_sz();

   /* ALU.T = tmp */
   a64_mov_x_reg(g_cg, X3, X6);
   break;
  }

  case 0x08: /* SR */
   a64_and_w_imm(g_cg, W7, W3, 0x1u);
   a64_bfi_w(g_cg, W24, W7, 24, 1);
   a64_asr_w_imm(g_cg, W6, W3, 1);
   a64_tst_w_reg(g_cg, W6, W6);
   emit_store_sz();
   a64_bfi_x(g_cg, X3, X6, 0, 32);
   break;

  case 0x09: /* RR */
   a64_and_w_imm(g_cg, W7, W3, 0x1u);
   a64_bfi_w(g_cg, W24, W7, 24, 1);
   a64_ror_w_imm(g_cg, W6, W3, 1);
   a64_tst_w_reg(g_cg, W6, W6);
   emit_store_sz();
   a64_bfi_x(g_cg, X3, X6, 0, 32);
   break;

  case 0x0A: /* SL */
   a64_lsr_w_imm(g_cg, W7, W3, 31);
   a64_bfi_w(g_cg, W24, W7, 24, 1);
   a64_lsl_w_imm(g_cg, W6, W3, 1);
   a64_tst_w_reg(g_cg, W6, W6);
   emit_store_sz();
   a64_bfi_x(g_cg, X3, X6, 0, 32);
   break;

  case 0x0B: /* RL */
   a64_lsr_w_imm(g_cg, W7, W3, 31);
   a64_bfi_w(g_cg, W24, W7, 24, 1);
   a64_ror_w_imm(g_cg, W6, W3, 31); /* ROR by 31 == ROL by 1 */
   a64_tst_w_reg(g_cg, W6, W6);
   emit_store_sz();
   a64_bfi_x(g_cg, X3, X6, 0, 32);
   break;

  case 0x0F: /* RL8 */
   a64_ubfx_w(g_cg, W7, W3, 24, 1);
   a64_bfi_w(g_cg, W24, W7, 24, 1);
   a64_ror_w_imm(g_cg, W6, W3, 24); /* ROR by 24 == ROL by 8 */
   a64_tst_w_reg(g_cg, W6, W6);
   emit_store_sz();
   a64_bfi_x(g_cg, X3, X6, 0, 32);
   break;

  default: /* 0x00, 0x07, 0x0C..0x0E -> NOP */
   break;
 }
}

static void emit_x_op(unsigned x_op, uint32_t instr)
{
 if((x_op & 0x3) == 0x2)
 {
  /* MAC: P = (int64)(int32)RX * (int32)RY */
  a64_ldr_w_imm(g_cg, W7, X0, O_RX);
  a64_ldr_w_imm(g_cg, W8, X0, O_RY);
  a64_smull(g_cg, X28, W7, W8);
 }

 if(x_op >= 0x3)
 {
  const unsigned drw = ((instr >> 20) & 0x7) & 0x3;
  const unsigned car = dr_carrier(drw);

  if(car)
  {
   if((x_op & 0x3) == 0x3)
    a64_mov_x_reg(g_cg, X28, car);
   if(x_op & 0x4)
    a64_str_w_imm(g_cg, car, X0, O_RX);
  }
  else if((x_op & 0x3) == 0x3)
  {
   a64_ubfx_w(g_cg, W4, W23, 8u * drw, 6u);
   emit_add_x_imm_safe(X5, X0, O_DRAM + drw * 256u);
   a64_ldrsw_x_idx_lsl(g_cg, X28, X5, X4, 2u);
   if(x_op & 0x4)
    a64_str_w_imm(g_cg, W28, X0, O_RX);
  }
  else
  {
   a64_ubfx_w(g_cg, W4, W23, 8u * drw, 6u);
   emit_add_x_imm_safe(X5, X0, O_DRAM + drw * 256u);
   a64_ldr_w_idx_lsl(g_cg, W12, X5, X4, 2u);
   if(x_op & 0x4)
    a64_str_w_imm(g_cg, W12, X0, O_RX);
  }
 }
}

static void emit_y_op(unsigned y_op, uint32_t instr)
{
 if((y_op & 0x3) == 0x1)
 {
  a64_mov_x_reg(g_cg, X26, XZR);
 }
 else if((y_op & 0x3) == 0x2)
 {
  a64_mov_x_reg(g_cg, X26, X3);
 }

 if(y_op >= 0x3)
 {
  const unsigned drw = ((instr >> 14) & 0x7) & 0x3;
  const unsigned car = dr_carrier(drw);

  if(car)
  {
   if((y_op & 0x3) == 0x3)
    a64_mov_x_reg(g_cg, X26, car);
   if(y_op & 0x4)
    a64_str_w_imm(g_cg, car, X0, O_RY);
  }
  else if((y_op & 0x3) == 0x3)
  {
   a64_ubfx_w(g_cg, W4, W23, 8u * drw, 6u);
   emit_add_x_imm_safe(X5, X0, O_DRAM + drw * 256u);
   a64_ldrsw_x_idx_lsl(g_cg, X26, X5, X4, 2u);
   if(y_op & 0x4)
    a64_str_w_imm(g_cg, W26, X0, O_RY);
  }
  else
  {
   a64_ubfx_w(g_cg, W4, W23, 8u * drw, 6u);
   emit_add_x_imm_safe(X5, X0, O_DRAM + drw * 256u);
   a64_ldr_w_idx_lsl(g_cg, W12, X5, X4, 2u);
   if(y_op & 0x4)
    a64_str_w_imm(g_cg, W12, X0, O_RY);
  }
 }
}

static void emit_d1_op(bool looped, unsigned d1_op, uint32_t instr, const GenMeta* meta)
{
 unsigned     d;
 int32_t      imm;
 unsigned     alt_src;

 if(!(d1_op & 0x1)) return;

 d       = (instr >> 8) & 0xF;
 imm     = (int32_t)(int8_t)(uint8_t)instr;
 alt_src = instr & 0xF;

 /* Resolve src_data into W12. */
 if(d1_op & 0x2)
 {
  switch(alt_src)
  {
   case 0x0: case 0x1: case 0x2: case 0x3:
   case 0x4: case 0x5: case 0x6: case 0x7:
   {
    const unsigned drw = alt_src & 0x3;
    const unsigned car = dr_carrier(drw);
    if(car)
     a64_mov_w_reg(g_cg, W12, car);
    else
    {
     a64_ubfx_w(g_cg, W4, W23, 8u * drw, 6u);
     emit_add_x_imm_safe(X5, X0, O_DRAM + drw * 256u);
     a64_ldr_w_idx_lsl(g_cg, W12, X5, X4, 2u);
    }
    break;
   }
   case 0x9:
    a64_mov_w_reg(g_cg, W12, W3);
    break;
   case 0xA:
    a64_lsr_x_imm(g_cg, X12, X3, 16);
    break;
   default: /* 0x8, 0xB..0xF -> 0xFFFFFFFF */
    a64_mov_w_imm(g_cg, W12, 0xFFFFFFFFu);
    break;
  }
 }
 else
 {
  /* src = sign-extended (int8)imm into a 32-bit register. */
  a64_mov_w_imm(g_cg, W12, (uint32_t)imm);
 }

 /* Apply src_data to destination. */
 switch(d)
 {
  case 0x0: case 0x1: case 0x2: case 0x3:
   if(!(meta->dr_read & (1u << d)))
   {
    a64_ubfx_w(g_cg, W4, W23, 8u * d, 6u);
    emit_add_x_imm_safe(X5, X0, O_DRAM + d * 256u);
    a64_str_w_idx_lsl(g_cg, W12, X5, X4, 2u);
   }
   break;

  case 0x4:
   a64_str_w_imm(g_cg, W12, X0, O_RX);
   break;

  case 0x5:
   a64_sxtw(g_cg, X28, W12);
   break;

  case 0x6:
   a64_str_w_imm(g_cg, W12, X0, O_RAO);
   break;

  case 0x7:
   a64_str_w_imm(g_cg, W12, X0, O_WAO);
   break;

  case 0x8:
  case 0x9:
   break;

  case 0xA:
  {
   a64_and_w_imm(g_cg, W7, W12, 0xFFFu);
   if(!looped)
   {
    a64_mov_w_reg(g_cg, W20, W7);
   }
   else
   {
    a64_label* skip = label_new();
    a64_cmp_w_imm(g_cg, W20, 0xFFFu);
    a64_b_cond(g_cg, A64_COND_NE, skip);
    a64_mov_w_reg(g_cg, W20, W7);
    label_bind(skip);
   }
   break;
  }

  case 0xB:
   a64_and_w_imm(g_cg, W7, W12, 0xFFu);
   a64_strb_w_imm(g_cg, W7, X0, O_TOP);
   break;

  case 0xC: case 0xD: case 0xE: case 0xF:
  {
   const int byte_idx = (int)d - 0xC;
   a64_bfi_w(g_cg, W23, W12, 8u * (unsigned)byte_idx, 8u);
   break;
  }

  default:
   break;
 }
}

static void emit_ct32_update(unsigned x_op, unsigned y_op, unsigned d1_op, uint32_t ct_inc)
{
 if(!(x_op >= 0x3 || y_op >= 0x3 || (d1_op & 0x1))) return;

 if(ct_inc != 0u)
 {
  a64_mov_w_imm(g_cg, W9, ct_inc);
  a64_add_w_reg(g_cg, W23, W23, W9);
 }
 emit_and_w_imm_safe(W23, W23, 0x3F3F3F3Fu, W9);
}

/* Write every sunk pin back to DSPS; see the coherence contract at the
 * top of the file for where this is emitted. */
static void emit_flush_pins(void)
{
 a64_str_x_imm (g_cg, X25, X0, O_NI);
 a64_strb_w_imm(g_cg, W27, X0, O_PC);
 a64_stur_w    (g_cg, W24, X0, (int)O_FZ);   /* O_FZ is byte-aligned */
 a64_str_w_imm (g_cg, W21, X0, O_CC);
 a64_strh_w_imm(g_cg, W20, X0, O_LOP);
 a64_str_w_imm (g_cg, W23, X0, O_CT32);
 a64_str_x_imm (g_cg, X26, X0, O_AC);
 a64_str_x_imm (g_cg, X28, X0, O_P);
}

static void emit_tail_dispatch(void)
{
 /* Unlinked tail: the CC decrement, the run checks and the indirect
  * dispatch are slot-invariant, so the whole tail is one B into the
  * shared dispatch stub.  No flags cross the branch; the SUBS is in the
  * stub. */
 if(!g_link_this_slot)
 {
  a64_b_addr(g_cg, g_dispatch_stub_addr);
  return;
 }

 /* Linked tail: the target slot is static, so its DataRAM reads issue
  * here, a whole slot ahead of their consumers -- W23 already holds its
  * entry CT.  The pass-2 patch aims the B past the successor's refill
  * block.  On the exit paths below the loads are dead but harmless;
  * DataRAM is always-mapped DSPS memory. */
 {
  const uint32_t succ_instr =
   (uint32_t)(DSP.ProgRAM[(uint8_t)(g_link_this_pc + 1u)] >> 32);
  const DRReadSet rs = dr_read_set(succ_instr);
  if(rs.nbanks)
   emit_dram_preloads(&rs);
 }

 /* Decrement pinned CC (2 cycles per slot), branch on <= 0 straight to
  * the exit stub (imm19 reaches anywhere in the 1 MB segment). */
 a64_subs_w_imm(g_cg, W21, W21, 2u);
 a64_b_cond_addr(g_cg, A64_COND_LE, g_exit_stub_addr);

 /* DSPS_IsRunning() = State > 0 (signed). */
 a64_cmp_w_imm(g_cg, W22, 0u);
 a64_b_cond_addr(g_cg, A64_COND_LE, g_exit_stub_addr);

 /* Direct B to the linear successor slot-(pc+1), patched by
  * rewind_locked's second pass; the placeholder targets the exit stub,
  * so an unpatched word still exits safely.  X25=NI is intact, so a
  * successor whose hot entry flushes it (a DMA fallback) sees the right
  * NextInstr. */
 g_link_site[g_link_this_pc] = a64_codegen_wptr(g_cg);
 a64_b_addr(g_cg, g_exit_stub_addr);
}

/*
 * Tail for a self-pipelined looped gen slot.  While the loop stays live
 * the dispatch target is this slot itself, so the tail preloads the next
 * iteration's DataRAM reads (W23 holds its entry CT) and branches back to
 * the post-refill body.
 *
 * Post-decrement LOP == 0xFFF is the leave-the-loop signal: it arises
 * only from the refetch iteration's 0 -> 0xFFF wrap (mid-loop values are
 * <= 0xFFE, and CompileSlot excludes slots whose d1 writes LOP), and
 * after a refetch X25 points at the next block.  Post-decrement 0 stays
 * on the fast path: the refetch iteration still executes this slot's body
 * once before leaving.
 */
static void emit_looped_pipe_tail(const void* pipe_body)
{
 a64_label* indir_lbl = label_new();

 a64_subs_w_imm(g_cg, W21, W21, 2u);
 a64_b_cond_addr(g_cg, A64_COND_LE, g_exit_stub_addr);
 a64_cmp_w_imm(g_cg, W22, 0u);
 a64_b_cond_addr(g_cg, A64_COND_LE, g_exit_stub_addr);
 a64_cmp_w_imm(g_cg, W20, 0xFFFu);
 a64_b_cond(g_cg, A64_COND_EQ, indir_lbl);

 emit_dram_preloads(&g_pipe_rs);
 a64_b_addr(g_cg, pipe_body);

 label_bind(indir_lbl);
 a64_add_x_reg_sxtw(g_cg, X16, X19, W25);
 a64_br(g_cg, X16);
}

/* --- Slot preludes ------------------------------------------------ */
static void emit_load_cc_pin   (void) { a64_ldr_w_imm (g_cg, W21, X0, O_CC); }
static void emit_load_ct32_pin (void) { a64_ldr_w_imm (g_cg, W23, X0, O_CT32); }
static void emit_load_flags_pin(void) { a64_ldur_w    (g_cg, W24, X0, (int)O_FZ); }
static void emit_load_pc_pin   (void) { a64_ldrb_w_imm(g_cg, W27, X0, O_PC); }
static void emit_load_lop_pin  (void) { a64_ldrh_w_imm(g_cg, W20, X0, O_LOP); }
/* AC.T (X26), P.T (X28) and the X19 anchor are pinned across blocks:
 * tail-dispatched entries skip these loads, while cold entries land at
 * offset 0 with arbitrary X19 and route through the shared prelude, which
 * runs them. */
static void emit_load_ac_pin   (void) { a64_ldr_x_imm (g_cg, X26, X0, O_AC); }
static void emit_load_p_pin    (void) { a64_ldr_x_imm (g_cg, X28, X0, O_P); }
static void emit_load_anchor_pin(void)
{ a64_movp2r(g_cg, X19, (const void*)(DSP_INSTR_BASE_UIPT + SCU_JIT_SLOT_PRELUDE_BYTES)); }

/*
 * Shared cold-reconstruction prelude (see g_prelude_stub_addr).  Reloads
 * every pin from DSPS, then BR X16 back into the slot body.  A cold
 * entry arrives from C (the entry stub's BLR, or DSP_TailDispatch in a
 * C handler) with memory current and every register stale, so the body
 * must be able to rely on the whole pin set -- including NI (X25) and
 * State (W22), which the linear-entry guard in emit_instr_pre reads
 * before the body's own NI fetch replaces it.
 */
static void emit_prelude_stub(void)
{
 a64_ldr_x_imm(g_cg, X25, X0, O_NI);
 a64_ldr_w_imm(g_cg, W22, X0, O_State);
 emit_load_cc_pin();
 emit_load_ct32_pin();
 emit_load_flags_pin();
 emit_load_pc_pin();
 emit_load_lop_pin();
 emit_load_ac_pin();
 emit_load_p_pin();
 emit_load_anchor_pin();
 a64_br(g_cg, X16);
}

/*
 * Per-slot cold entry at every real-body slot's offset 0, exactly
 * SCU_JIT_SLOT_PRELUDE_BYTES: stash the body address in X16 and branch to
 * the shared prelude.  The hot chain skips it, entering at
 * +SCU_JIT_SLOT_PRELUDE_BYTES via the X19 anchor bias.
 */
static void emit_cold_entry(void)
{
 void* body = (char*)a64_codegen_wptr(g_cg) + SCU_JIT_SLOT_PRELUDE_BYTES;
 a64_adr(g_cg, X16, body);
 a64_b_addr(g_cg, g_prelude_stub_addr);
}

static void emit_gen(bool looped, uint32_t instr)
{
 const unsigned alu_op = (instr >> 26) & 0xF;
 const unsigned x_op   = (instr >> 23) & 0x7;
 const unsigned y_op   = (instr >> 17) & 0x7;
 const unsigned d1_op  = (instr >> 12) & 0x3;
 const GenMeta  meta   = compute_meta(x_op, y_op, d1_op, instr);
 const void*    pipe_body = NULL;

 emit_cold_entry();
 if(g_pipe_this_slot)
 {
  /* Dual entry: indirect/cold entries land on the refill block at the
   * hot entry and load the carriers in-body; a preloading dispatcher (a
   * linked predecessor's tail, this slot's own loop-back tail) enters at
   * pipe_body, past it. */
  const void* hot_entry = a64_codegen_wptr(g_cg);
  emit_dram_preloads(&g_pipe_rs);
  pipe_body = a64_codegen_wptr(g_cg);
  if(!looped)
   g_pipe_delta[g_pipe_this_pc] =
    (uint16_t)((uintptr_t)pipe_body - (uintptr_t)hot_entry);
 }
 emit_instr_pre(looped);

 /* X3 = ALU.T (mutated in place by alu_op). */
 a64_mov_x_reg(g_cg, X3, X26);
 emit_alu_op(alu_op);

 emit_x_op(x_op, instr);
 emit_y_op(y_op, instr);
 emit_d1_op(looped, d1_op, instr, &meta);
 emit_ct32_update(x_op, y_op, d1_op, meta.ct_inc);
 if(looped && g_pipe_this_slot)
  emit_looped_pipe_tail(pipe_body);
 else
  emit_tail_dispatch();
}

/*
 * True iff `instr` is a general instruction whose gen body is a no-op:
 * its only effect is the dispatch step emit_instr_pre +
 * emit_tail_dispatch reproduce (NI = ProgRAM[PC], PC++, CC -= 2).
 * Mirrors the empty-work arms of emit_alu_op / emit_x_op / emit_y_op /
 * emit_d1_op.
 */
static bool is_nop_gen(uint32_t instr)
{
 unsigned alu_op, x_op, y_op, d1_op;

 if(((instr >> 28) & 0xF) > 0x3) return false;   /* not a general instr */

 alu_op = (instr >> 26) & 0xF;
 x_op   = (instr >> 23) & 0x7;
 y_op   = (instr >> 17) & 0x7;
 d1_op  = (instr >> 12) & 0x3;

 /* alu_op no-op set = emit_alu_op's default arm {0,7,C,D,E}; every other
  * value writes the packed flags (W24) and/or ALU.T. */
 if(!(alu_op == 0x0 || alu_op == 0x7 ||
      alu_op == 0xC || alu_op == 0xD || alu_op == 0xE)) return false;
 if(x_op > 0x1)     return false;  /* x_op 2 = MAC, >=3 = DataRAM read */
 if(y_op != 0x0)    return false;  /* y_op 1/2 = AC write, >=3 = DataRAM read */
 if(d1_op & 0x1)    return false;  /* d1 store */
 return true;
}

/*
 * Length of the maximal run of consecutive no-op gen instructions in
 * DSP.ProgRAM[] starting at `pc`, capped so pc + M <= 0xFF -- the run
 * never crosses the 8-bit PRAM wrap, and the coalesced case-A successor
 * ProgRAM[pc + M] is always in range.  Reads high32 directly, so it is
 * only valid once the whole program is populated (i.e. from a rewind).
 */
static unsigned nop_run_length(unsigned pc)
{
 unsigned M = 0, j = pc;
 while(j <= 0xFFu && is_nop_gen((uint32_t)(DSP.ProgRAM[j] >> 32)))
 {
  M++;
  j++;
 }
 if(pc + M > 0xFFu)
  M = 0xFFu - pc;
 return M;
}

/*
 * Shared no-op-run coalescing stub, emitted once at init.  Inputs are the
 * three constants the per-slot trampoline loads:
 *   W4 = pc1      expected linear entry PC ((pc+1) & 0xFF)
 *   W5 = two_m    2*M, M = run length
 *   W6 = succ_idx ProgRAM index of the first non-no-op ((pc+M) & 0xFF)
 * plus the live pins (W21=CC, W22=State, W27=PC, X25=NI, X0=DSPS, X19=anchor).
 *
 * On a linear entry (runtime PC == pc1) the whole run fast-forwards:
 * case B peels where the cycle budget crosses zero, case A advances past
 * the run and dispatches to the first non-no-op.  A non-linear entry (a
 * delay slot, runtime PC != pc1), a paused DSP (State <= 0) or an
 * exhausted budget (CC <= 0) take `slow`, one plain no-op step.  Runs are
 * capped (nop_run_length) so pc+M <= 0xFF: no PRAM wrap, succ_idx in
 * range.
 */
static void emit_coalesce_stub(void)
{
 a64_label* caseA = label_new();
 a64_label* slow  = label_new();

 a64_cmp_w_reg(g_cg, W27, W4);          /* linear entry? */
 a64_b_cond(g_cg, A64_COND_NE, slow);
 a64_cmp_w_imm(g_cg, W22, 0u);
 a64_b_cond(g_cg, A64_COND_LE, slow);   /* State <= 0 -> not running */
 a64_cmp_w_imm(g_cg, W21, 0u);
 a64_b_cond(g_cg, A64_COND_LE, slow);   /* CC <= 0 */

 a64_cmp_w_reg(g_cg, W21, W5);          /* CC vs 2M */
 a64_b_cond(g_cg, A64_COND_GT, caseA);  /* CC > 2M -> whole run survives */

 /* Case B: budget crosses zero inside the run.  n = ceil(CC/2) steps, each
  * NI=ProgRAM[PC], PC++.  End: PC = pc1+n, NI = ProgRAM[PC-1],
  * CC = CC-2n (0 if CC even, -1 if odd), then exit.  W27 == pc1 here. */
 a64_add_w_imm(g_cg, W9, W21, 1u);
 a64_lsr_w_imm(g_cg, W9, W9, 1u);         /* n = (CC+1) >> 1 */
 a64_add_w_reg(g_cg, W27, W27, W9);
 a64_and_w_imm(g_cg, W27, W27, 0xFFu);    /* PC = (pc1+n) & 0xFF */
 a64_and_w_imm(g_cg, W21, W21, 0x1u);
 a64_neg_w    (g_cg, W21, W21);           /* CC = -(CC & 1) -> {0,-1} */
 a64_sub_w_imm(g_cg, W8, W27, 1u);
 a64_and_w_imm(g_cg, W8, W8, 0xFFu);      /* (PC-1) & 0xFF */
 emit_add_x_imm_safe(X7, X0, O_PRAM);
 a64_ldr_x_idx_lsl(g_cg, X25, X7, X8, 3u); /* NI = ProgRAM[PC-1] */
 a64_b_addr(g_cg, g_exit_stub_addr);       /* flush pins + ret */

 /* Case A: whole run fits the remaining budget; advance past it and dispatch
  * onward to the first non-no-op slot (succ_pc = succ_idx+1). */
 label_bind(caseA);
 a64_sub_w_reg(g_cg, W21, W21, W5);        /* CC -= 2M */
 a64_add_w_imm(g_cg, W27, W6, 1u);
 a64_and_w_imm(g_cg, W27, W27, 0xFFu);     /* PC = (succ_idx+1) & 0xFF */
 emit_add_x_imm_safe(X7, X0, O_PRAM);
 a64_ldr_x_idx_lsl(g_cg, X25, X7, X6, 3u); /* NI = ProgRAM[succ_idx] */
 a64_add_x_reg_sxtw(g_cg, X16, X19, W25);
 a64_br(g_cg, X16);

 /* Slow path: one faithful no-op step (== a plain no-op gen slot body). */
 label_bind(slow);
 emit_instr_pre(false);
 emit_tail_dispatch();
}

/*
 * A coalesced gen slot for a no-op run of length M >= 2 starting at `pc`:
 * the cold entry, then a trampoline that loads the run's three constants
 * and branches to the shared stub above -- ~24 bytes against ~192
 * inlined.  Emitted only from rewind_locked(), where the look-ahead that
 * measured M saw the whole program.
 */
static void emit_gen_coalesced(unsigned pc, unsigned M)
{
 const unsigned pc1      = (pc + 1u) & 0xFFu;   /* expected linear entry PC */
 const unsigned succ_idx = (pc + M) & 0xFFu;    /* first non-no-op ProgRAM index */
 const uint32_t two_m    = 2u * M;

 /* Cold entry (exactly SCU_JIT_SLOT_PRELUDE_BYTES): tail dispatch skips it
  * on the hot chain; cold entries route through the shared prelude to reload
  * the pins the coalesce stub reads. */
 emit_cold_entry();

 /* Trampoline: run constants -> shared stub. */
 a64_mov_w_imm(g_cg, W4, pc1);
 a64_mov_w_imm(g_cg, W5, two_m);
 a64_mov_w_imm(g_cg, W6, succ_idx);
 a64_b_addr(g_cg, g_coalesce_stub_addr);
}

static bool is_general_instr(uint32_t instr) { return ((instr >> 28) & 0xF) <= 0x3; }
static bool is_mvi_instr(uint32_t instr)
{
 const unsigned top = (instr >> 28) & 0xF;
 return top >= 0x8 && top <= 0xB;
}
static bool is_jmp_instr(uint32_t instr) { return ((instr >> 28) & 0xF) == 0xD; }
static bool is_misc_instr(uint32_t instr)
{
 const unsigned top = (instr >> 28) & 0xF;
 return top == 0xE || top == 0xF;
}

/*
 * Block-linking predicates (see g_link_site).  A slot-pc's indirect tail
 * dispatch becomes a direct B to slot-(pc+1) when both hold:
 *
 *  - is_linkable_slot(instr): the slot ends in emit_tail_dispatch with X25
 *    still holding NI = ProgRAM[pc+1].  True for gen; jmp (its PC write is
 *    delayed to the delay slot); misc BTM (op 0, the one misc form with no
 *    helper); and mvi whose dest calls no helper.  dest 6/7/C run
 *    DSP_FinishPRAMDMA and misc END/ENDI/LPS call helpers, all of which
 *    reshape PC/NI; DMA has no tail dispatch at all.
 *
 *  - !may_perturb_pc(ProgRAM[pc-1]): the predecessor leaves PC linear, so
 *    slot-pc is never a delay slot and is always entered with runtime
 *    PC == pc+1.  Only a plain gen or a non-helper mvi advances PC purely
 *    linearly; jmp, dma, misc, DMA-finishing/PC-writing mvi and reserved
 *    opcodes are treated as perturbing.
 */
static bool is_linkable_slot(uint32_t instr)
{
 const unsigned top = (instr >> 28) & 0xF;
 if(top <= 0x3)               return true;    /* gen */
 if(top == 0xD)               return true;    /* JMP (delayed branch) */
 if(top == 0xE || top == 0xF)                 /* MISC: only BTM (op 0) has no helper */
  return ((instr >> 27) & 0x3) == 0x0;
 if(top >= 0x8 && top <= 0xB)                 /* MVI: exclude helper dests 6/7/C */
 {
  const unsigned dest = (instr >> 26) & 0xF;
  return !(dest == 0x6 || dest == 0x7 || dest == 0xC);
 }
 return false;                                /* DMA (0xC) / reserved: no tail dispatch */
}

static bool may_perturb_pc(uint32_t instr)
{
 const unsigned top = (instr >> 28) & 0xF;
 if(top <= 0x3) return false;                 /* gen: linear PC advance only */
 if(top >= 0x8 && top <= 0xB)                 /* MVI: only dest 6/7/C move PC */
 {
  const unsigned dest = (instr >> 26) & 0xF;
  return dest == 0x6 || dest == 0x7 || dest == 0xC;
 }
 return true;                                 /* JMP / DMA / MISC / reserved 4-7 */
}

/* --- Helpers BLR'd from JIT slots --------------------------------- */
static void lps_helper(struct DSPS* dsp)
{
 const uint32_t instr = dsp->NextInstr >> 32;
 const uint8_t  pc    = (dsp->PC - 1) & 0xFFu;
 LoopedSlot* cached = &g_looped_cache[pc];

 if(MDFN_UNLIKELY(!cached->entry || cached->instr != instr))
 {
  /* Called from a live JIT slot, so the chain-live flag is set and
   * CompileSlot returns NULL rather than rewinding over this return
   * address.  DSP_DecodeInstruction then resolves to the fallback thunk,
   * and this LPS instance loops through the C handler until the latched
   * rewind re-enables JIT. */
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

/*
 * Emit the DSP_TestCond chain.  Branches to skip_label when the test
 * fails.
 */
static void emit_test_cond(unsigned cond, a64_label* skip_label)
{
 if(!(cond & 0x40)) return;

 a64_mov_w_imm(g_cg, W7, 0u);
 if(cond & 0x1) { a64_ubfx_w(g_cg, W8, W24,  0, 1); a64_orr_w_reg(g_cg, W7, W7, W8); }
 if(cond & 0x2) { a64_ubfx_w(g_cg, W8, W24,  8, 1); a64_orr_w_reg(g_cg, W7, W7, W8); }
 if(cond & 0x4) { a64_ubfx_w(g_cg, W8, W24, 24, 1); a64_orr_w_reg(g_cg, W7, W7, W8); }
 if(cond & 0x8)
 {
  a64_ldr_w_imm(g_cg, W8, X0, O_T0_Until);
  a64_cmp_w_reg(g_cg, W8, W21);
  a64_cset_w(g_cg, W8, A64_COND_LT);
  a64_orr_w_reg(g_cg, W7, W7, W8);
 }
 if(cond & 0x20)
  a64_cbz_w(g_cg, W7, skip_label);
 else
  a64_cbnz_w(g_cg, W7, skip_label);
}

/*
 * BLR to a C helper, preserving x0 and the link register across the call
 * via the stack.  The caller sets up the arg registers; the sunk pins are
 * flushed first so the helper sees a current DSPS, and a caller wanting an
 * adjusted value (mvi dest 6/7's PC-1) adjusts the pin beforehand.
 */
static void emit_call_helper_addr(const void* helper_addr)
{
 emit_flush_pins();
 a64_stp_x_pre(g_cg, X0, X30, -16);
 a64_movp2r_pool(g_cg, X16, helper_addr);
 a64_blr(g_cg, X16);
 a64_ldp_x_post(g_cg, X0, X30, 16);
 /* Re-sync pinned regs that helpers may have mutated in memory.
  * NI is the full 64-bit pin (lps_helper / rewind rewrite it). */
 a64_ldr_w_imm(g_cg, W22, X0, O_State);
 a64_ldr_w_imm(g_cg, W21, X0, O_CC);
 a64_ldr_w_imm(g_cg, W23, X0, O_CT32);
 a64_ldr_x_imm(g_cg, X25, X0, O_NI);
 a64_ldrb_w_imm(g_cg, W27, X0, O_PC);
}

static void emit_mvi(bool looped, uint32_t instr)
{
 const unsigned dest = (instr >> 26) & 0xF;
 const unsigned cond = (instr >> 19) & 0x7F;
 const int32_t  imm  = (cond & 0x40)
                       ? sign_x_to_s32(19, instr)
                       : sign_x_to_s32(25, instr);
 a64_label* skip;

 emit_cold_entry();
 emit_instr_pre(looped);

 skip = label_new();
 emit_test_cond(cond, skip);

 if(dest == 0x6 || dest == 0x7)
 {
  a64_label* nodma = label_new();
  a64_ldr_w_imm(g_cg, W8, X0, O_PRAMDMACt);
  a64_cbz_w(g_cg, W8, nodma);
  /* Helper must see PC-1: adjust the pin, the helper's flush stores it
   * (STRB truncates the un-masked wrap), and the post-BLR reload keeps
   * W27 = PC-1 -- same end state as the interpreter. */
  a64_sub_w_imm(g_cg, W27, W27, 1u);
  emit_call_helper_addr((const void*)&DSP_FinishPRAMDMA);
  label_bind(nodma);
 }

 switch(dest)
 {
  case 0x0: case 0x1: case 0x2: case 0x3:
   a64_ubfx_w(g_cg, W4, W23, 8u * dest, 6u);
   a64_mov_w_imm(g_cg, W12, (uint32_t)imm);
   emit_add_x_imm_safe(X5, X0, O_DRAM + dest * 256u);
   a64_str_w_idx_lsl(g_cg, W12, X5, X4, 2u);
   a64_add_w_imm(g_cg, W4, W4, 1u);
   a64_and_w_imm(g_cg, W4, W4, 0x3Fu);
   a64_bfi_w(g_cg, W23, W4, 8u * dest, 8u);
   break;

  case 0x4:
   a64_mov_w_imm(g_cg, W12, (uint32_t)imm);
   a64_str_w_imm(g_cg, W12, X0, O_RX);
   break;

  case 0x5:
   /* P.T = (int64)(int32)imm -- sign-extended into 64-bit slot. */
   a64_mov_x_imm(g_cg, X28, (uint64_t)(int64_t)imm);
   break;

  case 0x6:
   a64_mov_w_imm(g_cg, W12, (uint32_t)imm);
   a64_str_w_imm(g_cg, W12, X0, O_RAO);
   break;

  case 0x7:
   a64_mov_w_imm(g_cg, W12, (uint32_t)imm);
   a64_str_w_imm(g_cg, W12, X0, O_WAO);
   break;

  case 0xA:
  {
   const uint32_t lop_val = (uint32_t)imm & 0xFFFu;
   if(!looped)
   {
    a64_mov_w_imm(g_cg, W12, lop_val);
    a64_mov_w_reg(g_cg, W20, W12);
   }
   else
   {
    a64_label* sk = label_new();
    a64_cmp_w_imm(g_cg, W20, 0xFFFu);
    a64_b_cond(g_cg, A64_COND_NE, sk);
    a64_mov_w_imm(g_cg, W12, lop_val);
    a64_mov_w_reg(g_cg, W20, W12);
    label_bind(sk);
   }
   break;
  }

  case 0xC:
  {
   a64_label* nodma2 = label_new();
   a64_sub_w_imm(g_cg, W8, W27, 1u);
   a64_strb_w_imm(g_cg, W8, X0, O_TOP);
   a64_mov_w_imm(g_cg, W27, (uint32_t)imm & 0xFFu);
   a64_ldr_w_imm(g_cg, W10, X0, O_PRAMDMACt);
   a64_cbz_w(g_cg, W10, nodma2);
   emit_call_helper_addr((const void*)&DSP_FinishPRAMDMA);
   label_bind(nodma2);
   break;
  }

  default:
   /* dest = 0x8, 0x9, 0xB, 0xD, 0xE, 0xF -> no commit */
   break;
 }

 label_bind(skip);
 emit_tail_dispatch();
}

static void emit_jmp(bool looped, uint32_t instr)
{
 const unsigned cond   = (instr >> 19) & 0x7F;
 const uint8_t  target = (uint8_t)instr;
 a64_label* skip;

 emit_cold_entry();
 emit_instr_pre(looped);

 skip = label_new();
 emit_test_cond(cond, skip);

 a64_mov_w_imm(g_cg, W27, (uint32_t)target);

 label_bind(skip);
 emit_tail_dispatch();
}

static void emit_misc(bool looped, uint32_t instr)
{
 const unsigned op = (instr >> 27) & 0x3;

 emit_cold_entry();
 emit_instr_pre(looped);

 if(op == 2 || op == 3)        /* END / ENDI */
 {
  a64_mov_w_imm(g_cg, W1, (uint32_t)(op & 0x1));
  emit_call_helper_addr((const void*)&misc_end_helper);
 }
 else if(op == 0)              /* BTM */
 {
  a64_label* skip = label_new();
  a64_cbz_w(g_cg, W20, skip);
  a64_ldrb_w_imm(g_cg, W27, X0, O_TOP);
  label_bind(skip);
  a64_sub_w_imm(g_cg, W20, W20, 1u);
  a64_and_w_imm(g_cg, W20, W20, 0xFFFu);
 }
 else if(op == 1)              /* LPS */
 {
  emit_call_helper_addr((const void*)&lps_helper);
 }

 emit_tail_dispatch();
}

/* --- Entry / exit stubs ------------------------------------------ */

/*
 * Entry stub: called from jit_entry with x0 = DSPS*.  Sets up an
 * AAPCS-conformant frame, loads pinned State/AC.T/P.T/NI (full 64-bit),
 * then BLR's to the first handler.
 */
static void emit_entry_stub(void)
{
 a64_stp_x_pre(g_cg, X29, X30, -96);
 a64_stp_x_off(g_cg, X19, X20, SP_REG, 16);
 a64_stp_x_off(g_cg, X21, X22, SP_REG, 32);
 a64_stp_x_off(g_cg, X25, X26, SP_REG, 48);
 a64_stp_x_off(g_cg, X23, X24, SP_REG, 64);
 a64_stp_x_off(g_cg, X27, X28, SP_REG, 80);

 a64_ldr_w_imm(g_cg, W22, X0, O_State);
 a64_ldr_x_imm(g_cg, X26, X0, O_AC);
 a64_ldr_x_imm(g_cg, X28, X0, O_P);

 /* One NI load serves both the X25 pin and the dispatch target: the
  * low32 offset comes out of the pin via the extending ADD.  X17 is the
  * unbiased base, so the BLR lands at slot offset 0 -- the cold entry,
  * which routes through the shared prelude for the remaining pins. */
 a64_ldr_x_imm(g_cg, X25, X0, O_NI);
 a64_movp2r(g_cg, X17, (const void*)&DSP_Init);
 a64_add_x_reg_sxtw(g_cg, X16, X17, W25);
 a64_blr(g_cg, X16);

 a64_ldp_x_off(g_cg, X27, X28, SP_REG, 80);
 a64_ldp_x_off(g_cg, X23, X24, SP_REG, 64);
 a64_ldp_x_off(g_cg, X25, X26, SP_REG, 48);
 a64_ldp_x_off(g_cg, X21, X22, SP_REG, 32);
 a64_ldp_x_off(g_cg, X19, X20, SP_REG, 16);
 a64_ldp_x_post(g_cg, X29, X30, 96);
 a64_ret(g_cg);
}

/*
 * Shared dispatch stub: the tail of every slot without a linked
 * successor.  Decrement pinned CC (2 cycles per slot), leave the chain on
 * CC <= 0 or State <= 0 (DSPS_IsRunning() = State > 0, signed), else
 * dispatch indirect through the X19 anchor and W25.
 *
 * Must be emitted immediately before the exit stub: both B.LEs bind to
 * its pin flush.  Linked tails keep their own copy of the sequence (the
 * fallthrough is the per-slot patched direct B), as does
 * emit_looped_pipe_tail, whose LOP test sits between the checks and the
 * dispatch.
 */
static void emit_dispatch_stub(void)
{
 a64_label* exit_lbl = label_new();

 a64_subs_w_imm(g_cg, W21, W21, 2u);
 a64_b_cond(g_cg, A64_COND_LE, exit_lbl);
 a64_cmp_w_imm(g_cg, W22, 0u);
 a64_b_cond(g_cg, A64_COND_LE, exit_lbl);
 a64_add_x_reg_sxtw(g_cg, X16, X19, W25);
 a64_br(g_cg, X16);

 label_bind(exit_lbl);   /* == the exit stub, emitted next */
}

static void emit_exit_stub(void)
{
 /* Pins reach memory here and at C-helper call sites, not per slot. */
 emit_flush_pins();
 a64_ret(g_cg);
}

/*
 * Fallback thunk: same shape as the DMA fallback in CompileSlot -- a
 * skip-safe prelude-sized pad whose first word branches around the
 * flush, so cold entries (offset 0, from C, memory already current)
 * and hot entries (tail dispatch at +SCU_JIT_SLOT_PRELUDE_BYTES, live
 * pins) both end up at the BR to the C shim with DSPS current.
 */
static void emit_fallback_thunk(void (*shim)(struct DSPS*))
{
 a64_label* cold = label_new();
 unsigned pad;
 a64_b(g_cg, cold);
 for(pad = 1; pad < SCU_JIT_SLOT_PRELUDE_BYTES / 4u; ++pad)
  a64_nop(g_cg);
 emit_flush_pins();
 label_bind(cold);
 a64_movp2r_pool(g_cg, X16, (const void*)shim);
 a64_br(g_cg, X16);
}

/*
 * Installed as SCU_DSP_JIT_Entry.  Brackets the JIT chain with a
 * chain-live flag: helpers BLR'd from slot code (lps_helper,
 * misc_end_helper, DSP_FinishPRAMDMA) can reach CompileSlot while the
 * stack still holds return addresses into live slot bytes
 * (emit_call_helper_addr's saved LR), so a segment rewind there would
 * recompile over code the chain returns into.  CompileSlot refuses and
 * latches the rewind instead; it runs here on the next entry, before
 * any slot code is live.
 */
static void jit_entry(struct DSPS* dsp)
{
 if(MDFN_UNLIKELY(g_rewind_pending))
  rewind_locked();
 g_chain_live = true;
 g_entry_stub(dsp);
 g_chain_live = false;
}

/* --- Public API --------------------------------------------------- */

void SCU_DSP_JIT_Init(void)
{
 void* stubs_start;
 void* entry_addr;
 void* dispatch_addr;
 void* exit_addr;
 void* coalesce_addr;
 void* prelude_addr;
 void* thunk_n_addr;
 void* thunk_l_addr;
 void* post_stub_ptr;

 if(!g_cg)
 {
  /* Slot entries live in ProgRAM[].low32 / NextInstr.low32 as int32
   * offsets from DSP_Init (DSP_INSTR_BASE_UIPT), so the segment must
   * sit within +/-2 GiB of it; a64_codegen_create_near refuses any
   * other placement and the JIT then stays off (Entry == NULL). */
  g_cg = a64_codegen_create_near(SCU_JIT_CODE_SEGMENT_SIZE, (const void*)DSP_INSTR_BASE_UIPT);
  if(!g_cg) return;
  g_seg_start = a64_codegen_wptr(g_cg);

  stubs_start = a64_codegen_wptr(g_cg);

  entry_addr = a64_codegen_wptr(g_cg);
  emit_entry_stub();
  g_entry_stub = (void (*)(struct DSPS*))entry_addr;
  SCU_DSP_JIT_Entry = &jit_entry;

  dispatch_addr = a64_codegen_wptr(g_cg);
  emit_dispatch_stub();
  g_dispatch_stub_addr = dispatch_addr;

  exit_addr = a64_codegen_wptr(g_cg);
  emit_exit_stub();
  g_exit_stub_addr = exit_addr;

  coalesce_addr = a64_codegen_wptr(g_cg);
  emit_coalesce_stub();
  g_coalesce_stub_addr = coalesce_addr;

  prelude_addr = a64_codegen_wptr(g_cg);
  emit_prelude_stub();
  g_prelude_stub_addr = prelude_addr;

  thunk_n_addr = a64_codegen_wptr(g_cg);
  emit_fallback_thunk(&fallback_shim_normal);
  thunk_l_addr = a64_codegen_wptr(g_cg);
  emit_fallback_thunk(&fallback_shim_looped);
  g_fallback_thunk[0] = thunk_n_addr;
  g_fallback_thunk[1] = thunk_l_addr;

  /* Resolve the entry stub's pooled DSP_Init pointer, the thunks'
   * shim pointers and any other stub-time pool refs.  The pool data
   * lives past unconditional terminators, so it's unreachable -- but
   * it sits below g_post_stub_byte_offset so rewind_locked() won't
   * trample it. */
  a64_pool_flush(g_cg);

  post_stub_ptr = a64_codegen_wptr(g_cg);
  g_post_stub_byte_offset = (size_t)((uintptr_t)post_stub_ptr - (uintptr_t)stubs_start);
  a64_codegen_invalidate(g_cg, stubs_start,
                         (size_t)((uintptr_t)post_stub_ptr - (uintptr_t)stubs_start));

  SS_JitDump_Open();
  SS_JitDump_Emit("dsp_entry_stub", entry_addr,
                  (size_t)((uintptr_t)dispatch_addr - (uintptr_t)entry_addr));
  SS_JitDump_Emit("dsp_dispatch_stub", dispatch_addr,
                  (size_t)((uintptr_t)exit_addr - (uintptr_t)dispatch_addr));
  SS_JitDump_Emit("dsp_exit_stub", exit_addr,
                  (size_t)((uintptr_t)coalesce_addr - (uintptr_t)exit_addr));
  SS_JitDump_Emit("dsp_coalesce_stub", coalesce_addr,
                  (size_t)((uintptr_t)prelude_addr - (uintptr_t)coalesce_addr));
  SS_JitDump_Emit("dsp_prelude_stub", prelude_addr,
                  (size_t)((uintptr_t)thunk_n_addr - (uintptr_t)prelude_addr));
  SS_JitDump_Emit("dsp_fallback_thunk_n", thunk_n_addr,
                  (size_t)((uintptr_t)thunk_l_addr - (uintptr_t)thunk_n_addr));
  SS_JitDump_Emit("dsp_fallback_thunk_l", thunk_l_addr,
                  (size_t)((uintptr_t)post_stub_ptr - (uintptr_t)thunk_l_addr));
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
 void* end;
 typedef void (*EmitFn)(bool, uint32_t);
 EmitFn emit_inline = NULL;
 bool   coalesced   = false;

 if(!g_cg)
  SCU_DSP_JIT_Init();
 if(!g_cg)
  return NULL;

 if(a64_codegen_offset(g_cg) + SCU_JIT_SLOT_MAX_BYTES > SCU_JIT_CODE_SEGMENT_SIZE)
 {
  /* A rewind recompiles every slot in place, so while a chain is live
   * (see jit_entry) it would overwrite bytes the chain still returns
   * into.  Refuse instead: DSP_DecodeSlotInstruction then falls back
   * to DSP_DecodeInstruction, whose WANT_JIT redirect yields the
   * fallback thunk -- correct, just C-speed until the latched rewind
   * runs at the next chain entry. */
  if(MDFN_UNLIKELY(g_chain_live))
  {
   g_rewind_pending = true;
   return NULL;
  }
  rewind_locked();
 }

 start = a64_codegen_wptr(g_cg);

 /* No-op-run coalescing.  Only during a whole-program rewind is the
  * forward look-ahead valid, and only for a non-looped gen slot that
  * genuinely sits at ProgRAM[pc] (the NextInstr re-decode reuses pc 0 for
  * a possibly-unrelated instr, so guard on the match).  A run of >= 2
  * no-ops collapses into one coalesced slot; see emit_gen_coalesced. */
 if(g_in_rewind && !looped && is_nop_gen(instr)
    && instr == (uint32_t)(DSP.ProgRAM[pc] >> 32))
 {
  const unsigned M = nop_run_length(pc);
  if(M >= 2u)
  {
   emit_gen_coalesced(pc, M);
   coalesced = true;
  }
 }

 /* Linear-entry specializations, valid only during the rewind slot pass.
  * A non-coalesced slot whose predecessor can't perturb PC is always
  * entered with runtime PC == pc+1, so its NI fetch becomes a patchable
  * immediate (emit_instr_pre records the site).  If the slot additionally
  * keeps X25=NI and dispatches linearly (calls no helper), its indirect
  * tail becomes a direct B (emit_tail_dispatch records the patch site);
  * rewind_locked's second pass fills both in once every slot is compiled. */
 g_link_this_slot = false;
 g_nic_this_slot  = false;
 if(g_in_rewind_slots && !looped && !coalesced
    && !may_perturb_pc((uint32_t)(DSP.ProgRAM[(uint8_t)(pc - 1u)] >> 32)))
 {
  g_nic_this_slot = true;
  g_nic_this_pc   = pc;
  if(is_linkable_slot(instr))
  {
   g_link_this_slot = true;
   g_link_this_pc   = pc;
  }
 }

 /* Cross-slot DataRAM read pipelining (see DRReadSet): a rewind-pass gen
  * slot with DataRAM reads compiles the dual refill/pipelined entry --
  * pipe validity needs no linear-entry proof (the reads depend only on
  * W23/X0, never on PC), so delay slots qualify too, and a linked
  * predecessor's tail preloads for it.  A looped gen slot self-pipelines
  * its own next iteration regardless of the rewind, except when its d1
  * writes LOP: emit_looped_pipe_tail's loop-back test relies on
  * post-decrement LOP == 0xFFF being unique to the refetch iteration. */
 g_pipe_this_slot = false;
 if(!coalesced
    && (looped ? !(((instr >> 12) & 0x1) && ((instr >> 8) & 0xF) == 0xA)
               : g_in_rewind_slots))
 {
  g_pipe_rs = dr_read_set(instr);
  g_pipe_this_slot = (g_pipe_rs.nbanks != 0);
  g_pipe_this_pc   = pc;
 }

 if(coalesced)
 {
  /* emit_gen_coalesced emitted the whole slot; nothing further. */
 }
 else if((emit_inline =
           is_general_instr(instr) ? &emit_gen  :
           is_mvi_instr(instr)     ? &emit_mvi  :
           is_jmp_instr(instr)     ? &emit_jmp  :
           is_misc_instr(instr)    ? &emit_misc : NULL))
 {
  emit_inline(looped, instr);
 }
 else
 {
  /* DMA: tail-jump straight to the templated C handler.  The pad is the
   * skip-safe cold entry (SCU_JIT_SLOT_PRELUDE_BYTES).  Hot entries (tail
   * dispatch, +SCU_JIT_SLOT_PRELUDE_BYTES) carry live pins and must flush
   * them for the C handler; cold entries (offset 0: entry stub BLR,
   * DSP_TailDispatch) arrive from C with memory already current but
   * garbage registers, so the pad's first word branches around the flush.
   * Going straight to C, it needs no pin reloads and does not route
   * through the shared prelude. */
  void (* const c_handler)(struct DSPS*) = pick_c_handler(looped, instr);
  a64_label* cold = label_new();
  unsigned pad;
  a64_b(g_cg, cold);
  for(pad = 1; pad < SCU_JIT_SLOT_PRELUDE_BYTES / 4u; ++pad)
   a64_nop(g_cg);
  emit_flush_pins();
  label_bind(cold);
  a64_movp2r_pool(g_cg, X16, (const void*)c_handler);
  a64_br(g_cg, X16);
 }

 /* Drain pool refs queued by this slot.  Every code path above ends in
  * an unconditional terminator (B/BR/RET via emit_tail_dispatch's exit
  * branch, or the DMA fallback's BR X16), so the pool data emitted
  * here is unreachable. */
 a64_pool_flush(g_cg);

 /* A slot that outgrew SCU_JIT_SLOT_MAX_BYTES could only have been
  * caught by the segment-end guard in emit_w; never publish such code.
  * Rewinding wp discards it, and the C-decode fallback in
  * DSP_DecodeSlotInstruction routes the slot through the thunk. */
 if(MDFN_UNLIKELY(a64_codegen_overflowed(g_cg)))
 {
  a64_codegen_set_wptr(g_cg, start);
  labels_reset();
  return NULL;
 }

 end = a64_codegen_wptr(g_cg);
 a64_codegen_invalidate(g_cg, start,
                        (size_t)((uintptr_t)end - (uintptr_t)start));

#ifdef WANT_DSP_JIT_PERF_DUMP
 {
  char nm[40];
  snprintf(nm, sizeof(nm), "dsp_%c_pc%02x_%s",
           looped ? 'l' : 'n', (unsigned)pc,
           jitdump_kind_str(instr));
  SS_JitDump_Emit(nm, start, (size_t)((uintptr_t)end - (uintptr_t)start));
 }
#endif

 /* Labels are scoped to one Compile -- reset for the next call. */
 labels_reset();

 /* This compile ran outside a rewind (a PRAM write / DMA / reset / state
  * load, never a looped lps recompile), so ProgRAM changed: latch a full
  * rewind for the next chain entry.  It recompiles every slot with the
  * whole program visible, replacing the plain slot emitted here before it
  * can execute a stale span. */
 if(!g_in_rewind && !looped)
  g_rewind_pending = true;

 return (void (*)(struct DSPS*))start;
}

#elif !(defined(WANT_JIT) && (defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)))
/* No backend for this target (x86 / x86-64 live in scu_dsp_jit_x86.c): stub everything. */

void SCU_DSP_JIT_Init(void)  {}
void SCU_DSP_JIT_Reset(void) {}

void (*SCU_DSP_JIT_CompileSlot(uint8_t pc, bool looped, uint32_t instr))(struct DSPS*)
{
 (void)pc; (void)looped; (void)instr;
 return NULL;
}

uint64_t SCU_DSP_JIT_FallbackNI(uint32_t instr, bool looped)
{
 (void)instr; (void)looped;
 return 0;
}

#endif
