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
 *   x16, x17 = MOVP2R staging and BR/BLR targets.
 *   w20  = pinned dsp->LOP (12-bit loop counter).
 *   w21  = pinned dsp->CycleCounter.
 *   w22  = pinned dsp->State (read-only cache).
 *   w23  = pinned dsp->CT32 (4 packed 6-bit CT counters).
 *   w24  = pinned packed flag bytes (FlagZ/S/V/C at byte 0..3).
 *   w25  = pinned dsp->NextInstr.low32 (threaded-dispatch offset).
 *   x26  = pinned dsp->AC.T.
 *   w27  = pinned dsp->PC.
 *   x28  = pinned dsp->P.T.
 *
 * Entry-stub frame: 96 bytes, preserving x19/x20, x21/x22, x25/x26,
 * x23/x24 and x27/x28.  Slot bodies push no frame; their tail dispatch
 * B's to the exit stub, which RETs through the entry stub's after-BLR.
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
static size_t      g_post_stub_byte_offset = 0;

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

static void rewind_locked(void)
{
 unsigned i;
 if(!g_cg) return;
 a64_codegen_set_wptr(g_cg, (char*)g_seg_start + g_post_stub_byte_offset);
 labels_reset();
 for(i = 0; i < 256; ++i) g_looped_cache[i].entry = NULL;

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
 {
  const uint32_t instr = (uint32_t)(DSP.NextInstr >> 32);
  const bool looped = DSP.NextInstrLooped;
  /* The looped JIT cache was cleared above, so this dispatch goes to the
   * C looped handler; the next lps_helper or InstrPre re-establishes
   * JIT. */
  DSP.NextInstr = looped
   ? DSP_DecodeInstruction(instr, true)
   : DSP_DecodeSlotInstruction(0, instr, false);
 }
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

static void emit_instr_pre(bool looped)
{
 if(!looped)
 {
  /* W27 pin = dsp->PC byte (zero-extended). */
  emit_add_x_imm_safe(X4, X0, O_PRAM);
  a64_ldr_x_idx_lsl(g_cg, X5, X4, X27, 3u);
  a64_str_x_imm(g_cg, X5, X0, O_NI);
  a64_strb_w_imm(g_cg, WZR, X0, O_NILooped);
  a64_mov_w_reg(g_cg, W25, W5);
  a64_add_w_imm(g_cg, W27, W27, 1u);
  a64_and_w_imm(g_cg, W27, W27, 0xFFu);
  a64_strb_w_imm(g_cg, W27, X0, O_PC);
 }
 else
 {
  a64_label* skip_load = label_new();
  a64_cbnz_w(g_cg, W20, skip_load);
  emit_add_x_imm_safe(X5, X0, O_PRAM);
  a64_ldr_x_idx_lsl(g_cg, X6, X5, X27, 3u);
  a64_str_x_imm(g_cg, X6, X0, O_NI);
  a64_strb_w_imm(g_cg, WZR, X0, O_NILooped);
  a64_mov_w_reg(g_cg, W25, W6);
  a64_add_w_imm(g_cg, W27, W27, 1u);
  a64_and_w_imm(g_cg, W27, W27, 0xFFu);
  a64_strb_w_imm(g_cg, W27, X0, O_PC);
  label_bind(skip_load);
  a64_sub_w_imm(g_cg, W20, W20, 1u);
  a64_and_w_imm(g_cg, W20, W20, 0xFFFu);
  a64_strh_w_imm(g_cg, W20, X0, O_LOP);
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
  a64_smull(g_cg, X9, W7, W8);
  a64_str_x_imm(g_cg, X9, X0, O_P);
  a64_mov_x_reg(g_cg, X28, X9);
 }

 if(x_op >= 0x3)
 {
  const unsigned drw = ((instr >> 20) & 0x7) & 0x3;
  a64_ubfx_w(g_cg, W4, W23, 8u * drw, 6u);
  emit_add_x_imm_safe(X5, X0, O_DRAM + drw * 256u);
  a64_ldr_w_idx_lsl(g_cg, W12, X5, X4, 2u);

  if((x_op & 0x3) == 0x3)
  {
   a64_sxtw(g_cg, X9, W12);
   a64_str_x_imm(g_cg, X9, X0, O_P);
   a64_mov_x_reg(g_cg, X28, X9);
  }
  if(x_op & 0x4)
  {
   a64_str_w_imm(g_cg, W12, X0, O_RX);
  }
 }
}

static void emit_y_op(unsigned y_op, uint32_t instr)
{
 if((y_op & 0x3) == 0x1)
 {
  a64_str_x_imm(g_cg, XZR, X0, O_AC);
  a64_mov_x_reg(g_cg, X26, XZR);
 }
 else if((y_op & 0x3) == 0x2)
 {
  a64_str_x_imm(g_cg, X3, X0, O_AC);
  a64_mov_x_reg(g_cg, X26, X3);
 }

 if(y_op >= 0x3)
 {
  const unsigned drw = ((instr >> 14) & 0x7) & 0x3;
  a64_ubfx_w(g_cg, W4, W23, 8u * drw, 6u);
  emit_add_x_imm_safe(X5, X0, O_DRAM + drw * 256u);
  a64_ldr_w_idx_lsl(g_cg, W12, X5, X4, 2u);

  if((y_op & 0x3) == 0x3)
  {
   a64_sxtw(g_cg, X9, W12);
   a64_str_x_imm(g_cg, X9, X0, O_AC);
   a64_mov_x_reg(g_cg, X26, X9);
  }
  if(y_op & 0x4)
  {
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
    a64_ubfx_w(g_cg, W4, W23, 8u * drw, 6u);
    emit_add_x_imm_safe(X5, X0, O_DRAM + drw * 256u);
    a64_ldr_w_idx_lsl(g_cg, W12, X5, X4, 2u);
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
   a64_sxtw(g_cg, X7, W12);
   a64_str_x_imm(g_cg, X7, X0, O_P);
   a64_mov_x_reg(g_cg, X28, X7);
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
    a64_strh_w_imm(g_cg, W7, X0, O_LOP);
    a64_mov_w_reg(g_cg, W20, W7);
   }
   else
   {
    a64_label* skip = label_new();
    a64_cmp_w_imm(g_cg, W20, 0xFFFu);
    a64_b_cond(g_cg, A64_COND_NE, skip);
    a64_strh_w_imm(g_cg, W7, X0, O_LOP);
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
 a64_str_w_imm(g_cg, W23, X0, O_CT32);
}

static void emit_tail_dispatch(void)
{
 a64_label* exit_lbl = label_new();

 /* Flush packed-flags pin to memory.  O_FZ is byte-aligned. */
 a64_stur_w(g_cg, W24, X0, (int)O_FZ);

 /* Decrement pinned CC (2 cycles per slot), flush, branch on <= 0. */
 a64_subs_w_imm(g_cg, W21, W21, 2u);
 a64_str_w_imm(g_cg, W21, X0, O_CC);
 a64_b_cond(g_cg, A64_COND_LE, exit_lbl);

 /* DSPS_IsRunning() = State > 0 (signed). */
 a64_cmp_w_imm(g_cg, W22, 0u);
 a64_b_cond(g_cg, A64_COND_LE, exit_lbl);

 /* W25 = pinned NI.low32; SXTW to recover the signed offset. */
 a64_sxtw(g_cg, X16, W25);
 a64_movp2r_pool(g_cg, X17, (const void*)&DSP_Init);
 a64_add_x_reg(g_cg, X16, X17, X16);
 /* Skip the next slot's 7-LDR prelude (28 bytes).  AC.T/P.T are pinned
  * in X26/X28 across the JIT-to-JIT chain; only cold entries (entry stub
  * BLR, DSP_TailDispatch tail-call from a C handler) land at offset 0
  * and execute the AC/P reloads. */
 a64_add_x_imm(g_cg, X16, X16, 28u);
 a64_br(g_cg, X16);

 label_bind(exit_lbl);
 a64_b_addr(g_cg, g_exit_stub_addr);
}

/* --- Slot preludes ------------------------------------------------ */
static void emit_load_cc_pin   (void) { a64_ldr_w_imm (g_cg, W21, X0, O_CC); }
static void emit_load_ct32_pin (void) { a64_ldr_w_imm (g_cg, W23, X0, O_CT32); }
static void emit_load_flags_pin(void) { a64_ldur_w    (g_cg, W24, X0, (int)O_FZ); }
static void emit_load_pc_pin   (void) { a64_ldrb_w_imm(g_cg, W27, X0, O_PC); }
static void emit_load_lop_pin  (void) { a64_ldrh_w_imm(g_cg, W20, X0, O_LOP); }
/* AC.T (X26) and P.T (X28) are pinned across blocks: tail-dispatched
 * entries skip these loads (emit_tail_dispatch's +28 byte skip), while
 * cold entries via SCU_DSP_JIT_Entry's BLR or DSP_TailDispatch's
 * tail-call from a C handler land at offset 0 and need the reload. */
static void emit_load_ac_pin   (void) { a64_ldr_x_imm (g_cg, X26, X0, O_AC); }
static void emit_load_p_pin    (void) { a64_ldr_x_imm (g_cg, X28, X0, O_P); }

static void emit_gen(bool looped, uint32_t instr)
{
 const unsigned alu_op = (instr >> 26) & 0xF;
 const unsigned x_op   = (instr >> 23) & 0x7;
 const unsigned y_op   = (instr >> 17) & 0x7;
 const unsigned d1_op  = (instr >> 12) & 0x3;
 const GenMeta  meta   = compute_meta(x_op, y_op, d1_op, instr);

 emit_load_cc_pin();
 emit_load_ct32_pin();
 emit_load_flags_pin();
 emit_load_pc_pin();
 emit_load_lop_pin();
 emit_load_ac_pin();
 emit_load_p_pin();
 emit_instr_pre(looped);

 /* X3 = ALU.T (mutated in place by alu_op). */
 a64_mov_x_reg(g_cg, X3, X26);
 emit_alu_op(alu_op);

 emit_x_op(x_op, instr);
 emit_y_op(y_op, instr);
 emit_d1_op(looped, d1_op, instr, &meta);
 emit_ct32_update(x_op, y_op, d1_op, meta.ct_inc);
 emit_tail_dispatch();
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

/* --- Helpers BLR'd from JIT slots --------------------------------- */
static void lps_helper(struct DSPS* dsp)
{
 const uint32_t instr = dsp->NextInstr >> 32;
 const uint8_t  pc    = (dsp->PC - 1) & 0xFFu;
 LoopedSlot* cached = &g_looped_cache[pc];

 if(MDFN_UNLIKELY(!cached->entry || cached->instr != instr))
 {
  void (*entry)(struct DSPS*);

  /* Called from a live JIT slot: LR points into the segment the compile
   * path may rewind, so when it is too tight for a fresh slot fall back
   * to the C handler for this LPS instance.  A later PRAM write triggers
   * a safe rewind and the next LPS dispatch can JIT again. */
  if(a64_codegen_offset(g_cg) + SCU_JIT_SLOT_MAX_BYTES > SCU_JIT_CODE_SEGMENT_SIZE)
  {
   dsp->NextInstr = DSP_DecodeInstruction(instr, true);
   dsp->NextInstrLooped = true;
   return;
  }

  entry = SCU_DSP_JIT_CompileSlot(pc, true, instr);
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
 * via the stack.  The caller sets up the arg registers.
 */
static void emit_call_helper_addr(const void* helper_addr)
{
 a64_stp_x_pre(g_cg, X0, X30, -16);
 a64_movp2r_pool(g_cg, X16, helper_addr);
 a64_blr(g_cg, X16);
 a64_ldp_x_post(g_cg, X0, X30, 16);
 /* Re-sync pinned regs that helpers may have mutated in memory. */
 a64_ldr_w_imm(g_cg, W22, X0, O_State);
 a64_ldr_w_imm(g_cg, W21, X0, O_CC);
 a64_ldr_w_imm(g_cg, W23, X0, O_CT32);
 a64_ldr_w_imm(g_cg, W25, X0, O_NI);
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

 emit_load_cc_pin();
 emit_load_ct32_pin();
 emit_load_flags_pin();
 emit_load_pc_pin();
 emit_load_lop_pin();
 emit_load_ac_pin();
 emit_load_p_pin();
 emit_instr_pre(looped);

 skip = label_new();
 emit_test_cond(cond, skip);

 if(dest == 0x6 || dest == 0x7)
 {
  a64_label* nodma = label_new();
  a64_ldr_w_imm(g_cg, W8, X0, O_PRAMDMACt);
  a64_cbz_w(g_cg, W8, nodma);
  a64_sub_w_imm(g_cg, W9, W27, 1u);
  a64_strb_w_imm(g_cg, W9, X0, O_PC);
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
   a64_str_w_imm(g_cg, W23, X0, O_CT32);
   break;

  case 0x4:
   a64_mov_w_imm(g_cg, W12, (uint32_t)imm);
   a64_str_w_imm(g_cg, W12, X0, O_RX);
   break;

  case 0x5:
   /* P.T = (int64)(int32)imm -- sign-extended into 64-bit slot. */
   a64_mov_x_imm(g_cg, X12, (uint64_t)(int64_t)imm);
   a64_str_x_imm(g_cg, X12, X0, O_P);
   a64_mov_x_reg(g_cg, X28, X12);
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
    a64_strh_w_imm(g_cg, W12, X0, O_LOP);
    a64_mov_w_reg(g_cg, W20, W12);
   }
   else
   {
    a64_label* sk = label_new();
    a64_cmp_w_imm(g_cg, W20, 0xFFFu);
    a64_b_cond(g_cg, A64_COND_NE, sk);
    a64_mov_w_imm(g_cg, W12, lop_val);
    a64_strh_w_imm(g_cg, W12, X0, O_LOP);
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
   a64_mov_w_imm(g_cg, W9, (uint32_t)imm & 0xFFu);
   a64_strb_w_imm(g_cg, W9, X0, O_PC);
   a64_mov_w_reg(g_cg, W27, W9);
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

 emit_load_cc_pin();
 emit_load_ct32_pin();
 emit_load_flags_pin();
 emit_load_pc_pin();
 emit_load_lop_pin();
 emit_load_ac_pin();
 emit_load_p_pin();
 emit_instr_pre(looped);

 skip = label_new();
 emit_test_cond(cond, skip);

 a64_mov_w_imm(g_cg, W12, (uint32_t)target);
 a64_strb_w_imm(g_cg, W12, X0, O_PC);
 a64_mov_w_reg(g_cg, W27, W12);

 label_bind(skip);
 emit_tail_dispatch();
}

static void emit_misc(bool looped, uint32_t instr)
{
 const unsigned op = (instr >> 27) & 0x3;

 emit_load_cc_pin();
 emit_load_ct32_pin();
 emit_load_flags_pin();
 emit_load_pc_pin();
 emit_load_lop_pin();
 emit_load_ac_pin();
 emit_load_p_pin();
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
  a64_ldrb_w_imm(g_cg, W5, X0, O_TOP);
  a64_strb_w_imm(g_cg, W5, X0, O_PC);
  a64_mov_w_reg(g_cg, W27, W5);
  label_bind(skip);
  a64_sub_w_imm(g_cg, W20, W20, 1u);
  a64_and_w_imm(g_cg, W20, W20, 0xFFFu);
  a64_strh_w_imm(g_cg, W20, X0, O_LOP);
 }
 else if(op == 1)              /* LPS */
 {
  emit_call_helper_addr((const void*)&lps_helper);
 }

 emit_tail_dispatch();
}

/* --- Entry / exit stubs ------------------------------------------ */

/*
 * Entry stub: called from SCU_UpdateDSP with x0 = DSPS*.  Sets up an
 * AAPCS-conformant frame, loads pinned State/AC.T/P.T/NI.low32, then
 * BLR's to the first handler.
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

 a64_ldrsw_x_imm(g_cg, X16, X0, O_NI);
 a64_movp2r_pool(g_cg, X17, (const void*)&DSP_Init);
 a64_add_x_reg(g_cg, X16, X17, X16);
 a64_ldr_w_imm(g_cg, W25, X0, O_NI);
 a64_blr(g_cg, X16);

 a64_ldp_x_off(g_cg, X27, X28, SP_REG, 80);
 a64_ldp_x_off(g_cg, X23, X24, SP_REG, 64);
 a64_ldp_x_off(g_cg, X25, X26, SP_REG, 48);
 a64_ldp_x_off(g_cg, X21, X22, SP_REG, 32);
 a64_ldp_x_off(g_cg, X19, X20, SP_REG, 16);
 a64_ldp_x_post(g_cg, X29, X30, 96);
 a64_ret(g_cg);
}

static void emit_exit_stub(void)
{
 a64_ret(g_cg);
}

/* --- Public API --------------------------------------------------- */

void SCU_DSP_JIT_Init(void)
{
 void* stubs_start;
 void* entry_addr;
 void* exit_addr;
 void* post_stub_ptr;

 if(!g_cg)
 {
  g_cg = a64_codegen_create(SCU_JIT_CODE_SEGMENT_SIZE);
  if(!g_cg) return;
  g_seg_start = a64_codegen_wptr(g_cg);

  stubs_start = a64_codegen_wptr(g_cg);

  entry_addr = a64_codegen_wptr(g_cg);
  emit_entry_stub();
  SCU_DSP_JIT_Entry = (void (*)(struct DSPS*))entry_addr;

  exit_addr = a64_codegen_wptr(g_cg);
  emit_exit_stub();
  g_exit_stub_addr = exit_addr;

  /* Resolve the entry stub's pooled DSP_Init pointer (and any other
   * stub-time pool refs).  The pool data lives past the exit stub's
   * RET, so it's unreachable -- but it sits below g_post_stub_byte_offset
   * so rewind_locked() won't trample it. */
  a64_pool_flush(g_cg);

  post_stub_ptr = a64_codegen_wptr(g_cg);
  g_post_stub_byte_offset = (size_t)((uintptr_t)post_stub_ptr - (uintptr_t)stubs_start);
  a64_codegen_invalidate(g_cg, stubs_start,
                         (size_t)((uintptr_t)post_stub_ptr - (uintptr_t)stubs_start));

  SS_JitDump_Open();
  SS_JitDump_Emit("dsp_entry_stub", entry_addr,
                  (size_t)((uintptr_t)exit_addr - (uintptr_t)entry_addr));
  SS_JitDump_Emit("dsp_exit_stub", exit_addr,
                  (size_t)((uintptr_t)post_stub_ptr - (uintptr_t)exit_addr));
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

void (*SCU_DSP_JIT_CompileSlot(uint8_t pc, bool looped, uint32_t instr))(struct DSPS*)
{
 void* start;
 void* end;
 typedef void (*EmitFn)(bool, uint32_t);
 EmitFn emit_inline = NULL;

#ifndef WANT_DSP_JIT_PERF_DUMP
 (void)pc;
#endif

 if(!g_cg)
  SCU_DSP_JIT_Init();
 if(!g_cg)
  return NULL;

 if(a64_codegen_offset(g_cg) + SCU_JIT_SLOT_MAX_BYTES > SCU_JIT_CODE_SEGMENT_SIZE)
  rewind_locked();

 start = a64_codegen_wptr(g_cg);

 if(is_general_instr(instr))   emit_inline = &emit_gen;
 else if(is_mvi_instr(instr))  emit_inline = &emit_mvi;
 else if(is_jmp_instr(instr))  emit_inline = &emit_jmp;
 else if(is_misc_instr(instr)) emit_inline = &emit_misc;

 if(emit_inline)
 {
  emit_inline(looped, instr);
 }
 else
 {
  /* DMA: tail-jump straight to the templated C handler.  7 leading
   * NOPs are the skip-safe prelude (matches the 7-LDR slot prelude;
   * JIT tail_dispatch BR's to target+28). */
  void (* const c_handler)(struct DSPS*) = pick_c_handler(looped, instr);
  a64_nop(g_cg);
  a64_nop(g_cg);
  a64_nop(g_cg);
  a64_nop(g_cg);
  a64_nop(g_cg);
  a64_nop(g_cg);
  a64_nop(g_cg);
  a64_movp2r_pool(g_cg, X16, (const void*)c_handler);
  a64_br(g_cg, X16);
 }

 /* Drain pool refs queued by this slot.  Every code path above ends in
  * an unconditional terminator (B/BR/RET via emit_tail_dispatch's exit
  * branch, or the DMA fallback's BR X16), so the pool data emitted
  * here is unreachable. */
 a64_pool_flush(g_cg);

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

 return (void (*)(struct DSPS*))start;
}

#else /* non-aarch64: stub everything */

void SCU_DSP_JIT_Init(void)  {}
void SCU_DSP_JIT_Reset(void) {}

void (*SCU_DSP_JIT_CompileSlot(uint8_t pc, bool looped, uint32_t instr))(struct DSPS*)
{
 (void)pc; (void)looped; (void)instr;
 return NULL;
}

#endif
