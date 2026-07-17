/******************************************************************************/
/* Mednafen Sega Saturn Emulation Module                                      */
/******************************************************************************/
/* scsp_dsp_jit.c - SCSP DSP JIT (aarch64 backend) implementation
**  Copyright (C) 2026 pstef
*/

/*
 * Two compile modes:
 *
 *   Mode A (any live step <= MAX_NATIVE_STEPS): full callee-save
 *     prologue, load pinned regs (W19..W28), emit per-step body --
 *     natively for supported indices, helper-BL with pin flush/reload
 *     for the rest -- then MDEC_CT update on the pin, frame restore.
 *
 *   Mode B (MAX_NATIVE_STEPS=0 or no live step in range): minimal
 *     fp/lr frame, BL helper per live step, MDEC_CT memory update, RET.
 *
 * The folded ring-mask constant (0x2000 << RBL) and the RAM-base add
 * (RBP << 12) are captured at Compile time; the RBL/RBP write path
 * marks MPROG_Dirty so those constants stay valid.
 *
 * Pinned-register layout (Mode A):
 *
 *   x0  = SS_SCSP* throughout the body
 *   w19 = MDEC_CT (16-bit) -- final decrement at exit
 *   w20 = SFT_REG (26-bit) -- every step writes
 *   w21 = FRC_REG (13-bit) -- FRCL steps write
 *   w22 = Y_REG   (24-bit) -- YRL steps write
 *   w23 = ADRS_REG (12-bit) -- ADRL steps write
 *   w24 = INPUTS  (24-bit) -- most steps write
 *   w25 = RWAddr  (19-bit) -- every step writes
 *   w26 = ReadPending (uint8)  -- gates RAM-read branch
 *   w27 = WritePending (bool)  -- gates RAM-write branch
 *   w28 = ReadValue (uint32)
 *   x29 = &DSP.TEMP[0] -- multiplier read every step, TWT write
 *   x30 = &RAM[0]      -- RAM-pipeline read/write blocks
 *
 * x29/x30 carry no frame state in the body: the prologue's STP saves
 * the caller's values and the body is a leaf, so both are free as
 * table-base pins (TEMP and RAM need register-offset addressing for
 * their dynamic indices, which [x0,#imm] can't express).  The
 * helper-BL fallback clobbers x30, so that path re-pins after the BL.
 *
 * WriteValue (uint16) is set by an MWT step and consumed by the next
 * step's RAM-write block; lives in memory between steps rather than
 * being pinned.
 */

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "ss.h"
#include "scsp.h"
#include "scsp_dsp_jit.h"
#include "a64emit.h"
#include "jitdump.h"

void (*SCSP_DSP_JIT_Entry)(struct SS_SCSP*) = NULL;

#if defined(WANT_JIT) && (defined(__aarch64__) || defined(__arm64__))

/* 0 forces all-helper Mode B; 128 is fully native.  Knob is useful
 * for bisecting native/interpreter divergence. */
#ifndef SCSP_DSP_JIT_MAX_NATIVE_STEPS
#define SCSP_DSP_JIT_MAX_NATIVE_STEPS 128
#endif

/* 128 native steps ~ 25 KB; 64 KB has headroom for the helper-mixed case. */
#define SCSP_JIT_CODE_BYTES (64u * 1024u)

/* Byte offset of an SS_SCSP field, compile-time. */
#define O(field) ((uint32_t)offsetof(SS_SCSP, field))

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
#define X8  8u
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

/* Table-base pins (Mode A body; see the layout comment above). */
#define X_TEMP_BASE X29
#define X_RAM_BASE  X30

extern void SCSP_DSP_run_step(struct SS_SCSP* scsp, unsigned step);
extern void SCSP_DSP_run_interpreter(struct SS_SCSP* scsp);

/* --- Codegen + label pool ---------------------------------------- */

/* MPROG has 128 slots; largest single step (RAM-pipeline block) uses
 * 3 labels and is followed by labels_reset(), so 64 is plenty. */
#define LABEL_POOL_SIZE 64u

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

/* --- Memory accessors with offset-range fallback ------------------ */

/* LDR/STR with [x0, #off] when the size-scaled 12-bit immediate fits,
 * falling back to MOV W16, off + register-offset form otherwise.  DSP
 * field offsets fit the direct form; only RAM (~1 MB) needs the
 * fallback. */
static void emit_ldr_w(unsigned dst, uint32_t off)
{
 if((off & 3) == 0 && off <= 16380) a64_ldr_w_imm(g_cg, dst, X0, off);
 else { a64_mov_w_imm(g_cg, W16, off); a64_ldr_w_reg(g_cg, dst, X0, X16); }
}
static void emit_str_w(unsigned src, uint32_t off)
{
 if((off & 3) == 0 && off <= 16380) a64_str_w_imm(g_cg, src, X0, off);
 else { a64_mov_w_imm(g_cg, W16, off); a64_str_w_reg(g_cg, src, X0, X16); }
}
static void emit_ldrh_w(unsigned dst, uint32_t off)
{
 if((off & 1) == 0 && off <= 8190) a64_ldrh_w_imm(g_cg, dst, X0, off);
 else { a64_mov_w_imm(g_cg, W16, off); a64_ldrh_w_reg(g_cg, dst, X0, X16); }
}
static void emit_strh_w(unsigned src, uint32_t off)
{
 if((off & 1) == 0 && off <= 8190) a64_strh_w_imm(g_cg, src, X0, off);
 else { a64_mov_w_imm(g_cg, W16, off); a64_strh_w_reg(g_cg, src, X0, X16); }
}
static void emit_ldrb_w(unsigned dst, uint32_t off)
{
 if(off <= 4095) a64_ldrb_w_imm(g_cg, dst, X0, off);
 else { a64_mov_w_imm(g_cg, W16, off); a64_ldrb_w_reg(g_cg, dst, X0, X16); }
}
static void emit_strb_w(unsigned src, uint32_t off)
{
 if(off <= 4095) a64_strb_w_imm(g_cg, src, X0, off);
 else { a64_mov_w_imm(g_cg, W16, off); a64_strb_w_reg(g_cg, src, X0, X16); }
}

/* Pin the TEMP and RAM bases.  Both offsets are larger than the
 * ADD-imm direct range, hence the MOV staging through W17.  Emitted in
 * the Mode A prologue and again after each helper BL (BL trashes x30). */
static void emit_base_pins(void)
{
 a64_mov_w_imm(g_cg, W17, O(DSP.TEMP));
 a64_add_x_reg(g_cg, X_TEMP_BASE, X0, X17);
 a64_mov_w_imm(g_cg, W17, O(RAM));
 a64_add_x_reg(g_cg, X_RAM_BASE, X0, X17);
}

/* ADD Wd, Wn, #imm with a MOV+ADD reg fallback when `imm` doesn't fit
 * the AddSubImm encoding.  W16 is the canonical transient register
 * (also used by the LDR/STR fallbacks above and by emit_step_helper_bl
 * for MOVP2R staging), so wd/wn must not alias W16. */
static void emit_add_w_imm_safe(unsigned wd, unsigned wn, uint32_t imm)
{
 if(!a64_try_add_w_imm(g_cg, wd, wn, imm))
 {
  a64_mov_w_imm(g_cg, W16, imm);
  a64_add_w_reg(g_cg, wd, wn, W16);
 }
}

/* --- Frames ------------------------------------------------------- */

static void emit_min_prologue(void)
{
 a64_stp_x_pre(g_cg, X29, X30, -16);
 a64_mov_x_sp(g_cg, X29);
}
static void emit_min_epilogue(void)
{
 a64_ldp_x_post(g_cg, X29, X30, 16);
 a64_ret(g_cg);
}

static void emit_full_prologue(void)
{
 /* No `mov x29, sp` frame link: x29 becomes a table-base pin right
  * below, so the register can't double as a frame pointer.  The
  * caller's x29/x30 still sit in the frame record for the epilogue. */
 a64_stp_x_pre(g_cg, X29, X30, -96);
 a64_stp_x_off(g_cg, X19, X20, SP_REG, 16);
 a64_stp_x_off(g_cg, X21, X22, SP_REG, 32);
 a64_stp_x_off(g_cg, X23, X24, SP_REG, 48);
 a64_stp_x_off(g_cg, X25, X26, SP_REG, 64);
 a64_stp_x_off(g_cg, X27, X28, SP_REG, 80);

 emit_ldrh_w(W19, O(DSP.MDEC_CT));
 emit_ldr_w (W20, O(DSP.SFT_REG));
 emit_ldrh_w(W21, O(DSP.FRC_REG));
 emit_ldr_w (W22, O(DSP.Y_REG));
 emit_ldrh_w(W23, O(DSP.ADRS_REG));
 emit_ldr_w (W24, O(DSP.INPUTS));
 emit_ldr_w (W25, O(DSP.RWAddr));
 emit_ldrb_w(W26, O(DSP.ReadPending));
 emit_ldrb_w(W27, O(DSP.WritePending));
 emit_ldr_w (W28, O(DSP.ReadValue));
 emit_base_pins();
}
static void emit_full_epilogue(void)
{
 emit_strh_w(W19, O(DSP.MDEC_CT));
 emit_str_w (W20, O(DSP.SFT_REG));
 emit_strh_w(W21, O(DSP.FRC_REG));
 emit_str_w (W22, O(DSP.Y_REG));
 emit_strh_w(W23, O(DSP.ADRS_REG));
 emit_str_w (W24, O(DSP.INPUTS));
 emit_str_w (W25, O(DSP.RWAddr));
 emit_strb_w(W26, O(DSP.ReadPending));
 emit_strb_w(W27, O(DSP.WritePending));
 emit_str_w (W28, O(DSP.ReadValue));

 a64_ldp_x_off(g_cg, X27, X28, SP_REG, 80);
 a64_ldp_x_off(g_cg, X25, X26, SP_REG, 64);
 a64_ldp_x_off(g_cg, X23, X24, SP_REG, 48);
 a64_ldp_x_off(g_cg, X21, X22, SP_REG, 32);
 a64_ldp_x_off(g_cg, X19, X20, SP_REG, 16);
 a64_ldp_x_post(g_cg, X29, X30, 96);
 a64_ret(g_cg);
}

/* Around helper BLs: trampoline sees the live state via memory; the
 * subsequent emit_pin_reload picks up its updates. */
static void emit_pin_flush(void)
{
 emit_strh_w(W19, O(DSP.MDEC_CT));
 emit_str_w (W20, O(DSP.SFT_REG));
 emit_strh_w(W21, O(DSP.FRC_REG));
 emit_str_w (W22, O(DSP.Y_REG));
 emit_strh_w(W23, O(DSP.ADRS_REG));
 emit_str_w (W24, O(DSP.INPUTS));
 emit_str_w (W25, O(DSP.RWAddr));
 emit_strb_w(W26, O(DSP.ReadPending));
 emit_strb_w(W27, O(DSP.WritePending));
 emit_str_w (W28, O(DSP.ReadValue));
}
static void emit_pin_reload(void)
{
 emit_ldrh_w(W19, O(DSP.MDEC_CT));
 emit_ldr_w (W20, O(DSP.SFT_REG));
 emit_ldrh_w(W21, O(DSP.FRC_REG));
 emit_ldr_w (W22, O(DSP.Y_REG));
 emit_ldrh_w(W23, O(DSP.ADRS_REG));
 emit_ldr_w (W24, O(DSP.INPUTS));
 emit_ldr_w (W25, O(DSP.RWAddr));
 emit_ldrb_w(W26, O(DSP.ReadPending));
 emit_ldrb_w(W27, O(DSP.WritePending));
 emit_ldr_w (W28, O(DSP.ReadValue));
}

/* --- Helper-BL fallback ------------------------------------------- */

static void emit_step_helper_bl(unsigned step)
{
 a64_mov_w_imm(g_cg, W1, step);
 a64_movp2r_pool(g_cg, X16, (const void*)&SCSP_DSP_run_step);
 a64_blr(g_cg, X16);
}

/* --- DSP-float helpers (inline in emitted code) ------------------- */

/* Mirrors scsp.inc::dspfloat_to_int.  w_tmp_a/w_tmp_b must not alias
 * w_out prematurely. */
static void emit_dspfloat_to_int(unsigned w_out, unsigned w_in,
                                 unsigned w_tmp_a, unsigned w_tmp_b)
{
 /* sign_xor = (inv & 0x8000) ? 0xC0000000 : 0
  * SBFX sign-broadcast -> 0xFFFFFFFF or 0, then LSL 30 gives 0xC0000000. */
 a64_sbfx_w(g_cg, w_tmp_a, w_in, 15, 1);
 a64_lsl_w_imm(g_cg, w_tmp_a, w_tmp_a, 30);
 /* exp = (inv >> 11) & 0xF */
 a64_ubfx_w(g_cg, w_tmp_b, w_in, 11, 4);
 /* ret = inv & 0x7FF */
 a64_and_w_imm(g_cg, w_out, w_in, 0x7FFu);
 /* if (exp < 12) ret |= 0x800 */
 a64_mov_w_imm(g_cg, W15, 0x800u);
 a64_cmp_w_imm(g_cg, w_tmp_b, 12);
 a64_csel_w(g_cg, W15, WZR, W15, A64_COND_GE);
 a64_orr_w_reg(g_cg, w_out, w_out, W15);
 /* ret <<= 19 */
 a64_lsl_w_imm(g_cg, w_out, w_out, 19);
 /* ret ^= sign_xor */
 a64_eor_w_reg(g_cg, w_out, w_out, w_tmp_a);
 /* shift = 8 + min(11, exp) */
 a64_mov_w_imm(g_cg, W15, 11u);
 a64_cmp_w_imm(g_cg, w_tmp_b, 11);
 a64_csel_w(g_cg, W15, w_tmp_b, W15, A64_COND_LE);
 a64_add_w_imm(g_cg, W15, W15, 8);
 /* ret = (int32)ret >> shift */
 a64_asr_w_reg(g_cg, w_out, w_out, W15);
 /* return ret & 0xFFFFFF */
 a64_and_w_imm(g_cg, w_out, w_out, 0xFFFFFFu);
}

/* int_to_dspfloat(W_in32) -> W_out16. */
static void emit_int_to_dspfloat(unsigned w_out, unsigned w_in,
                                 unsigned w_tmp_a, unsigned w_tmp_b)
{
 /* invsl8 = inv << 8 */
 a64_lsl_w_imm(g_cg, w_tmp_a, w_in, 8);
 /* sign_xor = (int32)invsl8 >> 31 */
 a64_asr_w_imm(g_cg, w_tmp_b, w_tmp_a, 31);
 /* base = ((invsl8 ^ sign_xor) << 1) | (1 << 19) */
 a64_eor_w_reg(g_cg, W15, w_tmp_a, w_tmp_b);
 a64_lsl_w_imm(g_cg, W15, W15, 1);
 a64_orr_w_imm(g_cg, W15, W15, 0x80000u);
 /* exp = clz32(base) */
 a64_clz_w(g_cg, W15, W15);
 /* shift = exp - (exp == 12 ? 1 : 0) */
 a64_sub_w_imm(g_cg, w_tmp_b, W15, 1);
 a64_cmp_w_imm(g_cg, W15, 12);
 a64_csel_w(g_cg, w_tmp_b, w_tmp_b, W15, A64_COND_EQ);
 /* shift_amt = 19 - shift */
 a64_mov_w_imm(g_cg, w_out, 19u);
 a64_sub_w_reg(g_cg, w_tmp_b, w_out, w_tmp_b);
 /* ret = (int32)invsl8 >> shift_amt */
 a64_asr_w_reg(g_cg, w_tmp_a, w_tmp_a, w_tmp_b);
 /* ret = (ret & 0x87FF) | (exp << 11)
  * 0x87FF is two disjoint bit-runs (not a valid logical-imm), so
  * materialise it explicitly. */
 a64_mov_w_imm(g_cg, w_out, 0x87FFu);
 a64_and_w_reg(g_cg, w_tmp_a, w_tmp_a, w_out);
 a64_lsl_w_imm(g_cg, W15, W15, 11);
 a64_orr_w_reg(g_cg, w_out, w_tmp_a, W15);
}

/* --- MDEC_CT update ----------------------------------------------- */

static void emit_mdec_ct_update_pin(uint8_t rbl)
{
 const uint32_t reload = 0x2000u << (rbl & 3);
 a64_label* skip = label_new();
 a64_cbnz_w(g_cg, W19, skip);
 a64_mov_w_imm(g_cg, W19, reload);
 label_bind(skip);
 a64_sub_w_imm(g_cg, W19, W19, 1);
 labels_reset();
}
static void emit_mdec_ct_update_mem(uint8_t rbl)
{
 const uint32_t reload = 0x2000u << (rbl & 3);
 a64_label* skip = label_new();
 emit_ldrh_w(W3, O(DSP.MDEC_CT));
 a64_cbnz_w(g_cg, W3, skip);
 a64_mov_w_imm(g_cg, W3, reload);
 label_bind(skip);
 a64_sub_w_imm(g_cg, W3, W3, 1);
 emit_strh_w(W3, O(DSP.MDEC_CT));
 labels_reset();
}

/* --- Native per-step body ----------------------------------------- */

/* Mirrors scsp.inc::RunDSPStep verbatim -- each `if(f & DSPF_X)`
 * becomes a compile-time decision to emit that branch's body.
 *
 * Per-step scratch registers (not preserved across steps):
 *   W1   = INPUTS_sxt (sxt24 of W24)
 *   W2   = y_input
 *   W3   = ShifterOutput (24-bit unsigned)
 *   W4   = TEMP read address
 *   W5   = TEMP_sxt
 *   W7   = y_sxt13
 *   X8   = Product
 *   W9   = SGAOutput
 *   W10  = TEMP write address / staging
 *   W11  = MADRS accumulator
 *   W12,W13,W14,W15 = RAM-pipeline + dspfloat scratches
 *   W16,X16,W17,X17 = MOVP2R + offset-fallback staging */
static void emit_step_native(const SS_SCSP_DSPStep* s,
                             uint8_t rbl, uint8_t rbp)
{
 const uint32_t f   = s->flags;
 const unsigned IRA = s->IRA;

 /* IRA decode -- compile-time pick. */
 if(IRA & 0x20) {
  if(IRA & 0x10) {
   if(!(IRA & 0xE)) {
    emit_ldrh_w(W24, O(EXTS) + (IRA & 0x1) * 2);
    a64_lsl_w_imm(g_cg, W24, W24, 8);
   }
   /* else: INPUTS unchanged */
  } else {
   emit_ldr_w(W24, O(DSP.MIXS) + (IRA & 0xF) * 4);
   a64_lsl_w_imm(g_cg, W24, W24, 4);
  }
 } else {
  emit_ldr_w(W24, O(DSP.MEMS) + (IRA & 0x1F) * 4);
 }

 /* Always emitted: X_SEL, ADRL, and the interpreter's INPUTS_sxt all
  * read this even when DSP.INPUTS wasn't updated this step. */
 a64_sbfx_w(g_cg, W1, W24, 0, 24);

 /* Y selector -- compile-time pick on YSEL. */
 switch(s->YSEL & 3) {
  case 0:
   a64_mov_w_reg(g_cg, W2, W21);
   break;
  case 1:
   emit_ldrh_w(W2, O(DSP.COEF) + s->CRA * 2);
   break;
  case 2:
   a64_ubfx_w(g_cg, W2, W22, 11, 13);
   break;
  case 3:
   a64_ubfx_w(g_cg, W2, W22, 4, 12);
   break;
 }

 /* YRL: Y_REG <- INPUTS & 0xFFFFFF.  W24 holds the raw DSP.INPUTS
  * (already <= 0xFFFFFF), so AND-mask is enough. */
 if(f & DSPF_YRL)
  a64_and_w_imm(g_cg, W22, W24, 0xFFFFFFu);

 /* Shifter:
  *   shft0 = (f >> 7) & 1
  *   shft1 = (f >> 8) & 1
  *   ShifterOutput = ((int32)sxt26(SFT_REG)) << (shft0 ^ shft1)
  *   if (!shft1) saturate to [-0x800000, 0x7FFFFF]
  *   ShifterOutput &= 0xFFFFFF
  *
  * shft0/shft1 are compile-time, so the shift amount, the saturate
  * check, and the FRCL/ADRL branch-select downstream all collapse. */
 const unsigned shft0     = (f >> 7) & 1;
 const unsigned shft1     = (f >> 8) & 1;
 const unsigned shift_amt = shft0 ^ shft1;
 a64_sbfx_w(g_cg, W3, W20, 0, 26);
 if(shift_amt)
  a64_lsl_w_imm(g_cg, W3, W3, shift_amt);
 if(!shft1) {
  /* Clamp signed-32 to [-0x800000, 0x7FFFFF]. */
  a64_mov_w_imm(g_cg, W10, 0x7FFFFFu);
  a64_cmp_w_reg(g_cg, W3, W10);
  a64_csel_w(g_cg, W3, W10, W3, A64_COND_GT);
  a64_mov_w_imm(g_cg, W10, 0xFF800000u);  /* int32 -0x800000 as bit pattern */
  a64_cmp_w_reg(g_cg, W3, W10);
  a64_csel_w(g_cg, W3, W10, W3, A64_COND_LT);
 }
 a64_and_w_imm(g_cg, W3, W3, 0xFFFFFFu);

 /* FRCL: FRC_REG <- (shft0&shft1) ? (Shifter & 0xFFF) : (Shifter >> 11) */
 if(f & DSPF_FRCL) {
  if(shft0 && shft1)
   a64_and_w_imm(g_cg, W21, W3, 0xFFFu);
  else
   a64_lsr_w_imm(g_cg, W21, W3, 11);
 }

 /* Multiplier-adder:
  *   TEMP[TEMPReadAddr] read, sxt24 -> TEMP_sxt
  *   x_input = XSEL ? INPUTS_sxt : TEMP_sxt
  *   Product = (sxt13(y_input) * x_input) >> 12
  *   SGAOutput = ZERO ? 0 : NEGB ? -B : B  (B = BSEL ? SFT_REG : TEMP_sxt)
  *   SFT_REG  = (Product + SGAOutput) & 0x3FFFFFF
  *
  * TEMPReadAddr = (TRA + MDEC_CT) & 0x7F -- TRA compile-time. */
 emit_add_w_imm_safe(W4, W19, s->TRA);
 a64_and_w_imm(g_cg, W4, W4, 0x7Fu);
 a64_ldr_w_uxtw(g_cg, W5, X_TEMP_BASE, W4, 2);
 a64_sbfx_w(g_cg, W5, W5, 0, 24);
 const unsigned w_x_input = (f & DSPF_XSEL) ? W1 : W5;
 a64_sbfx_w(g_cg, W7, W2, 0, 13);
 a64_smull(g_cg, X8, W7, w_x_input);
 a64_asr_x_imm(g_cg, X8, X8, 12);
 /* The AND 0x3FFFFFF below truncates, so we read W8 (low 32). */
 if(f & DSPF_ZERO) {
  a64_mov_w_imm(g_cg, W9, 0u);
 } else {
  const unsigned w_b = (f & DSPF_BSEL) ? W20 : W5;
  if(f & DSPF_NEGB)
   a64_neg_w(g_cg, W9, w_b);
  else
   a64_mov_w_reg(g_cg, W9, w_b);
 }
 a64_add_w_reg(g_cg, W20, W8, W9);
 a64_and_w_imm(g_cg, W20, W20, 0x3FFFFFFu);

 /* EWT: EFREG[EWA] <- ShifterOutput >> 8 */
 if(f & DSPF_EWT) {
  a64_lsr_w_imm(g_cg, W10, W3, 8);
  emit_strh_w(W10, O(DSP.EFREG) + s->EWA * 2);
 }

 /* TWT: TEMP[(TWA + MDEC_CT) & 0x7F] <- ShifterOutput */
 if(f & DSPF_TWT) {
  emit_add_w_imm_safe(W10, W19, s->TWA);
  a64_and_w_imm(g_cg, W10, W10, 0x7Fu);
  a64_str_w_uxtw(g_cg, W3, X_TEMP_BASE, W10, 2);
 }

 /* IWT: MEMS[IWA] <- ReadValue (pin W28) */
 if(f & DSPF_IWT)
  emit_str_w(W28, O(DSP.MEMS) + s->IWA * 4);

 /* RAM pipeline (data-dependent):
  *   if (ReadPending)  { tmp=RAM[RWAddr]; ReadValue = ...; RP=0; }
  *   elif(WritePending){ if(!(RWAddr&0x40000)) RAM[RWAddr]=WV; WP=0;}
  *
  * The branchiness can't be folded -- both ReadPending and
  * WritePending depend on flags set in earlier steps. */
 {
  a64_label* ram_done       = label_new();
  a64_label* ram_read       = label_new();
  a64_label* ram_write_skip = label_new();

  a64_cbnz_w(g_cg, W26, ram_read);
  a64_cbz_w (g_cg, W27, ram_done);

  /* Write path: skip if RWAddr & 0x40000 (bit 18). */
  a64_tbnz_w(g_cg, W25, 18, ram_write_skip);
  emit_ldrh_w(W12, O(DSP.WriteValue));
  a64_strh_w_uxtw(g_cg, W12, X_RAM_BASE, W25, 1);
  label_bind(ram_write_skip);
  a64_mov_w_imm(g_cg, W27, 0u);
  a64_b(g_cg, ram_done);

  /* Read path:
   *   tmp = RAM[RWAddr]
   *   ReadValue = (ReadPending == 2) ? (tmp << 8) : dspfloat_to_int(tmp)
   *   ReadPending = 0 */
  label_bind(ram_read);
  a64_ldrh_w_uxtw(g_cg, W14, X_RAM_BASE, W25, 1);
  /* Inline dspfloat path into W12, NOFL path into W13, CSEL into W28. */
  emit_dspfloat_to_int(W12, W14, W13, W11);
  a64_lsl_w_imm(g_cg, W13, W14, 8);
  a64_cmp_w_imm(g_cg, W26, 2);
  a64_csel_w(g_cg, W28, W13, W12, A64_COND_EQ);
  a64_mov_w_imm(g_cg, W26, 0u);
  label_bind(ram_done);
 }

 /* MADRS / RWAddr update:
  *   addr = MADRS[MASA]
  *   if (NXADDR) addr += 1
  *   if (ADRGB)  addr += sxt12(ADRS_REG)
  *   if (!TABLE) addr += MDEC_CT; addr &= (0x2000<<RBL) - 1
  *   RWAddr = (addr + (RBP<<12)) & 0x7FFFF */
 emit_ldrh_w(W11, O(DSP.MADRS) + s->MASA * 2);
 if(f & DSPF_NXADDR)
  a64_add_w_imm(g_cg, W11, W11, 1u);
 if(f & DSPF_ADRGB) {
  a64_sbfx_w(g_cg, W12, W23, 0, 12);
  a64_add_w_reg(g_cg, W11, W11, W12);
 }
 if(!(f & DSPF_TABLE)) {
  a64_add_w_reg(g_cg, W11, W11, W19);
  a64_and_w_imm(g_cg, W11, W11, (0x2000u << (rbl & 3)) - 1u);
 } else {
  /* Interpreter holds addr as uint16_t; the non-TABLE mask above
   * incidentally wraps to 0xFFFF, so TABLE must do it explicitly. */
  a64_and_w_imm(g_cg, W11, W11, 0xFFFFu);
 }
 /* (RBP << 12) is 0..0x7F000 -- fits AddSubImm shifted-by-12. */
 {
  const uint32_t rbp_off = (uint32_t)(rbp & 0x7F) << 12;
  if(rbp_off)
   emit_add_w_imm_safe(W11, W11, rbp_off);
 }
 a64_and_w_imm(g_cg, W25, W11, 0x7FFFFu);

 /* MRT: ReadPending <- NOFL ? 2 : 1 */
 if(f & DSPF_MRT)
  a64_mov_w_imm(g_cg, W26, (f & DSPF_NOFL) ? 2u : 1u);

 /* MWT: WritePending <- 1; WriteValue <- NOFL ? (Shifter>>8) : int_to_dspfloat(Shifter) */
 if(f & DSPF_MWT) {
  a64_mov_w_imm(g_cg, W27, 1u);
  if(f & DSPF_NOFL) {
   a64_lsr_w_imm(g_cg, W12, W3, 8);
   emit_strh_w(W12, O(DSP.WriteValue));
  } else {
   emit_int_to_dspfloat(W12, W3, W13, W11);
   emit_strh_w(W12, O(DSP.WriteValue));
  }
 }

 /* ADRL: ADRS_REG <- (shft0&shft1) ? (Shifter>>12) : (INPUTS_sxt>>16) & 0xFFF */
 if(f & DSPF_ADRL) {
  if(shft0 && shft1) {
   a64_lsr_w_imm(g_cg, W23, W3, 12);
  } else {
   a64_lsr_w_imm(g_cg, W23, W1, 16);
   a64_and_w_imm(g_cg, W23, W23, 0xFFFu);
  }
 }

 /* Reclaim label-pool slots; without this the pool overflows after
  * ~22 live steps. */
 labels_reset();
}

/* --- Public API --------------------------------------------------- */

void SCSP_DSP_JIT_Init(struct SS_SCSP* scsp)
{
 (void)scsp;
 if(!g_cg) {
  g_cg = a64_codegen_create(SCSP_JIT_CODE_BYTES);
  if(g_cg) g_seg_start = a64_codegen_wptr(g_cg);
 }
 SCSP_DSP_JIT_Entry = NULL;
}

void SCSP_DSP_JIT_Reset(struct SS_SCSP* scsp)
{
 if(!g_cg) SCSP_DSP_JIT_Init(scsp);
 SCSP_DSP_JIT_Entry = NULL;
}

void SCSP_DSP_JIT_Compile(struct SS_SCSP* scsp)
{
 if(!g_cg)
  return;

 labels_reset();
 a64_codegen_set_wptr(g_cg, g_seg_start);
 void* const entry_addr = a64_codegen_wptr(g_cg);

 const uint8_t rbl = scsp->RBL;
 const uint8_t rbp = scsp->RBP;

 const unsigned max_native =
  (SCSP_DSP_JIT_MAX_NATIVE_STEPS < 128u)
  ? (unsigned)SCSP_DSP_JIT_MAX_NATIVE_STEPS : 128u;

 /* Any live step within max_native picks pin-based Mode A;
  * otherwise fall through to all-helper Mode B. */
 bool mode_a = false;
 for(unsigned i = 0; i < max_native; ++i) {
  if(scsp->DSP.MPROG_Decoded[i].live) { mode_a = true; break; }
 }

 if(mode_a) {
  emit_full_prologue();
  for(unsigned step = 0; step < 128u; ++step) {
   const SS_SCSP_DSPStep* s = &scsp->DSP.MPROG_Decoded[step];
   if(!s->live) continue;
   if(step < max_native) {
    emit_step_native(s, rbl, rbp);
   } else {
    emit_pin_flush();
    emit_step_helper_bl(step);
    emit_pin_reload();
    /* The BL trashed the x30 base pin (x29 is callee-saved). */
    emit_base_pins();
   }
  }
  emit_mdec_ct_update_pin(rbl);
  emit_full_epilogue();
 } else {
  emit_min_prologue();
  for(unsigned step = 0; step < 128u; ++step) {
   if(scsp->DSP.MPROG_Decoded[step].live)
    emit_step_helper_bl(step);
  }
  emit_mdec_ct_update_mem(rbl);
  emit_min_epilogue();
 }

 /* Resolve every queued movp2r_pool site.  Both epilogue paths above
  * end in RET, so the pool data emitted here is unreachable code. */
 a64_pool_flush(g_cg);

 void* const end_addr = a64_codegen_wptr(g_cg);
 const size_t code_bytes = (size_t)((char*)end_addr - (char*)entry_addr);
 a64_codegen_invalidate(g_cg, entry_addr, code_bytes);

 /* Publish to perf jitdump.  perf inject --jit will resolve samples
  * landing anywhere in [entry_addr, end_addr) to this symbol.  The
  * code_index counter in the shared writer disambiguates successive
  * MPROG_Dirty recompiles that reuse the same address. */
 SS_JitDump_Open();
 SS_JitDump_Emit("scsp_mprog", entry_addr, code_bytes);

 SCSP_DSP_JIT_Entry = (void(*)(struct SS_SCSP*))entry_addr;
}

#else /* non-aarch64 or JIT not requested: stub everything */

void SCSP_DSP_JIT_Init   (struct SS_SCSP* z) { (void)z; }
void SCSP_DSP_JIT_Reset  (struct SS_SCSP* z) { (void)z; }
void SCSP_DSP_JIT_Compile(struct SS_SCSP* z) { (void)z; }

#endif
