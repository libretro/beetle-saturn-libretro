/******************************************************************************/
/* Mednafen Sega Saturn Emulation Module                                      */
/******************************************************************************/
/* a64emit.h - minimal AArch64 instruction emitter (C, integer-only)
**  Copyright (C) 2026 pstef
*/

/*
 * Covers what the two DSP JITs need: integer arithmetic / logic /
 * bitfield / shift / compare-and-branch / load-store / pair, plus a
 * mmap'd RWX code block with icache invalidation.  No FP/SIMD, no
 * atomics.
 *
 * Register parameters are plain `unsigned`s -- 0..30 for general
 * registers, 31 for WZR/XZR/WSP/SP.  Conditions use the AArch64 4-bit
 * encoding (also available as the A64_COND_* names below).
 */

#ifndef MDFN_SS_A64EMIT_H
#define MDFN_SS_A64EMIT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* SP/WZR/XZR all encode as register index 31. */
#define A64_SP_INDEX 31u

/* Condition codes (AArch64 encoding). */
enum {
 A64_COND_EQ = 0,  A64_COND_NE = 1,
 A64_COND_CS = 2,  A64_COND_CC = 3,
 A64_COND_MI = 4,  A64_COND_PL = 5,
 A64_COND_VS = 6,  A64_COND_VC = 7,
 A64_COND_HI = 8,  A64_COND_LS = 9,
 A64_COND_GE = 10, A64_COND_LT = 11,
 A64_COND_GT = 12, A64_COND_LE = 13,
 A64_COND_AL = 14, A64_COND_NV = 15
};

/* Maximum forward-reference patches per label; the two DSP JITs use at
 * most 3. */
#define A64_LABEL_MAX_PATCHES 8u

/*
 * Forward-branch label.  POD: zero-init before first use, reuse by
 * calling a64_label_reset().
 */
typedef struct a64_label {
 int          bound;
 ptrdiff_t    target_off;
 unsigned int patch_count;
 struct {
  ptrdiff_t    wb_off;
  unsigned int kind;
 } patches[A64_LABEL_MAX_PATCHES];
} a64_label;

/* Opaque codegen handle: one mmap'd RWX region + a write pointer. */
typedef struct a64_codegen a64_codegen;

/* --- Code-segment lifecycle --------------------------------------- */

/* Allocate `bytes` of RWX memory and a code-generator pointing at its
 * base.  Returns NULL on failure (mmap returned MAP_FAILED). */
a64_codegen* a64_codegen_create(size_t bytes);

/* Like a64_codegen_create, but tries to place the segment within
 * +/-2 GiB of `near` (a code address in the host binary).  Returns
 * NULL if no such placement can be found, so a caller that encodes
 * segment addresses as 32-bit offsets from `near` never gets a
 * mapping it cannot reach. */
a64_codegen* a64_codegen_create_near(size_t bytes, const void* near);
void         a64_codegen_destroy(a64_codegen*);

void*  a64_codegen_base    (const a64_codegen*);
void*  a64_codegen_wptr    (const a64_codegen*);
size_t a64_codegen_offset  (const a64_codegen*);
size_t a64_codegen_capacity(const a64_codegen*);

/* Bytes between the current write pointer and the end of the segment. */
size_t a64_codegen_remaining(const a64_codegen*);

/* Move the write pointer to `p` (which must point inside the segment).
 * Outstanding labels are not reset. */
void a64_codegen_set_wptr(a64_codegen*, void* p);
/* Non-zero once an emit ran past the segment end (sticky until the
 * next a64_codegen_set_wptr); callers must not publish such code. */
int  a64_codegen_overflowed(const a64_codegen*);

/* Save / restore the write pointer around an excursion to another
 * region of the segment. */
void* a64_codegen_save   (const a64_codegen*);
void  a64_codegen_restore(a64_codegen*, void*);

/* Call after emitting a region so the new code is safe to execute. */
void a64_codegen_invalidate(a64_codegen*, void* p, size_t bytes);

/* --- Labels ------------------------------------------------------- */

/* Discard any pending patches and clear the bound flag; equivalent to
 * `memset(lbl, 0, sizeof *lbl)`. */
void a64_label_reset(a64_label*);

/* Bind the label at the current write pointer and resolve every patch
 * site that was queued before this point.  Subsequent uses of the same
 * label require a64_label_reset() first. */
void a64_label_bind(a64_codegen*, a64_label*);

/* --- Encodability predicates (no emit) ---------------------------- */

/* True iff `imm` is a valid 32/64-bit AArch64 logical immediate.
 * The bitwise *_imm emitters below return the same value (0/1). */
int a64_can_encode_logical_imm32(uint32_t imm);
int a64_can_encode_logical_imm64(uint64_t imm);

/* True iff `imm` is a valid 12-bit AddSubImm value, optionally with
 * shift-by-12. */
int a64_can_encode_addsub_imm(uint32_t imm);

/* --- Instruction emitters ----------------------------------------- */

/* MOV / pointer materialisation. */
void a64_mov_w_imm(a64_codegen*, unsigned wd,   uint32_t imm);
void a64_mov_x_imm(a64_codegen*, unsigned xd,   uint64_t imm);
/* Fixed-length MOVZ + 3 MOVK (always 16 bytes), rewritable in place
 * via a64_patch_mov_x_imm4. */
void a64_mov_x_imm4(a64_codegen*, unsigned xd,  uint64_t imm);
void a64_mov_w_reg(a64_codegen*, unsigned wd,   unsigned wm);
void a64_mov_x_reg(a64_codegen*, unsigned xd,   unsigned xm);
/* MOV Xd_sp, SP : alias ADD Xd_sp, SP, #0.  Needed for `MOV Xd, SP`
 * since the register-form MOV alias only accepts ZR, not SP. */
void a64_mov_x_sp (a64_codegen*, unsigned xd);
void a64_movp2r   (a64_codegen*, unsigned xd, const void* ptr);

/* Pool-backed pointer materialisation: one LDR (literal) plus a queued
 * pool entry, in place of the 1..4 MOVZ/MOVK of a64_movp2r.  The caller
 * must invoke a64_pool_flush before the LDR can fall out of imm19 range
 * (+/-1 MiB). */
void a64_movp2r_pool(a64_codegen*, unsigned xd, const void* ptr);

/* ADR Xd, target : PC-relative code address in one instruction.
 * `target` must be within +/-1 MiB of the emit point (imm21 is a signed
 * byte offset, unshifted). */
void a64_adr(a64_codegen*, unsigned xd, const void* target);

/* Add/Sub.  imm form: 12-bit unsigned + optional shift-by-12 (the
 * encoder picks the shift automatically).  reg form: shifted-reg with
 * LSL #0. */
void a64_add_w_imm(a64_codegen*, unsigned wd_sp, unsigned wn_sp, uint32_t imm);
void a64_sub_w_imm(a64_codegen*, unsigned wd_sp, unsigned wn_sp, uint32_t imm);
void a64_add_x_imm(a64_codegen*, unsigned xd_sp, unsigned xn_sp, uint32_t imm);
void a64_sub_x_imm(a64_codegen*, unsigned xd_sp, unsigned xn_sp, uint32_t imm);

/* Return 1 and emit on success, 0 and emit nothing if `imm` doesn't
 * fit the AddSubImm encoding. */
int  a64_try_add_w_imm(a64_codegen*, unsigned wd_sp, unsigned wn_sp, uint32_t imm);
int  a64_try_sub_w_imm(a64_codegen*, unsigned wd_sp, unsigned wn_sp, uint32_t imm);
int  a64_try_add_x_imm(a64_codegen*, unsigned xd_sp, unsigned xn_sp, uint32_t imm);
int  a64_try_sub_x_imm(a64_codegen*, unsigned xd_sp, unsigned xn_sp, uint32_t imm);
void a64_add_w_reg(a64_codegen*, unsigned wd, unsigned wn, unsigned wm);
void a64_sub_w_reg(a64_codegen*, unsigned wd, unsigned wn, unsigned wm);
void a64_add_x_reg(a64_codegen*, unsigned xd, unsigned xn, unsigned xm);
void a64_sub_x_reg(a64_codegen*, unsigned xd, unsigned xn, unsigned xm);
/* ADD (extended register): Xd_sp = Xn_sp + SXTW(Wm).  Base + signed
 * 32-bit offset in one instruction; Rd/Rn are SP-capable in this form,
 * Rm index 31 = WZR. */
void a64_add_x_reg_sxtw(a64_codegen*, unsigned xd_sp, unsigned xn_sp, unsigned wm);

/* Flag-setting; pair with a64_cset_w to materialise the flag. */
void a64_adds_w_imm(a64_codegen*, unsigned wd, unsigned wn_sp, uint32_t imm);
void a64_subs_w_imm(a64_codegen*, unsigned wd, unsigned wn_sp, uint32_t imm);
void a64_adds_w_reg(a64_codegen*, unsigned wd, unsigned wn, unsigned wm);
void a64_subs_w_reg(a64_codegen*, unsigned wd, unsigned wn, unsigned wm);
void a64_ands_w_reg(a64_codegen*, unsigned wd, unsigned wn, unsigned wm);

void a64_cmp_w_imm(a64_codegen*, unsigned wn_sp, uint32_t imm);
void a64_cmp_w_reg(a64_codegen*, unsigned wn, unsigned wm);
void a64_tst_w_reg(a64_codegen*, unsigned wn, unsigned wm);
void a64_tst_x_reg(a64_codegen*, unsigned xn, unsigned xm);

void a64_cset_w(a64_codegen*, unsigned wd, unsigned cond);
void a64_csel_w(a64_codegen*, unsigned wd, unsigned wn, unsigned wm, unsigned cond);

/* Bitwise immediate.  Returns 1 on success, 0 if `imm` is not a valid
 * logical immediate, in which case nothing is emitted. */
int a64_and_w_imm(a64_codegen*, unsigned wd_sp, unsigned wn, uint32_t imm);
int a64_orr_w_imm(a64_codegen*, unsigned wd_sp, unsigned wn, uint32_t imm);
int a64_eor_w_imm(a64_codegen*, unsigned wd_sp, unsigned wn, uint32_t imm);

void a64_and_w_reg(a64_codegen*, unsigned wd, unsigned wn, unsigned wm);
void a64_orr_w_reg(a64_codegen*, unsigned wd, unsigned wn, unsigned wm);
void a64_eor_w_reg(a64_codegen*, unsigned wd, unsigned wn, unsigned wm);
void a64_and_x_reg(a64_codegen*, unsigned xd, unsigned xn, unsigned xm);
void a64_eor_x_reg(a64_codegen*, unsigned xd, unsigned xn, unsigned xm);
void a64_bic_x_reg(a64_codegen*, unsigned xd, unsigned xn, unsigned xm);

void a64_orr_w_reg_lsl(a64_codegen*, unsigned wd, unsigned wn, unsigned wm,
                       unsigned shift);

/* Shifts. */
void a64_lsl_w_imm(a64_codegen*, unsigned wd, unsigned wn, unsigned shift);
void a64_lsr_w_imm(a64_codegen*, unsigned wd, unsigned wn, unsigned shift);
void a64_asr_w_imm(a64_codegen*, unsigned wd, unsigned wn, unsigned shift);
void a64_ror_w_imm(a64_codegen*, unsigned wd, unsigned wn, unsigned shift);
void a64_asr_w_reg(a64_codegen*, unsigned wd, unsigned wn, unsigned wm);
void a64_lsl_x_imm(a64_codegen*, unsigned xd, unsigned xn, unsigned shift);
void a64_lsr_x_imm(a64_codegen*, unsigned xd, unsigned xn, unsigned shift);
void a64_asr_x_imm(a64_codegen*, unsigned xd, unsigned xn, unsigned shift);

/* Bitfield. */
void a64_ubfx_w(a64_codegen*, unsigned wd, unsigned wn, unsigned lsb, unsigned width);
void a64_sbfx_w(a64_codegen*, unsigned wd, unsigned wn, unsigned lsb, unsigned width);
void a64_sbfiz_w(a64_codegen*, unsigned wd, unsigned wn, unsigned lsb, unsigned width);
void a64_bfi_w (a64_codegen*, unsigned wd, unsigned wn, unsigned lsb, unsigned width);
void a64_bfi_x (a64_codegen*, unsigned xd, unsigned xn, unsigned lsb, unsigned width);

void a64_sxtw  (a64_codegen*, unsigned xd, unsigned wn);
void a64_clz_w (a64_codegen*, unsigned wd, unsigned wn);

void a64_smull(a64_codegen*, unsigned xd, unsigned wn, unsigned wm);
void a64_neg_w(a64_codegen*, unsigned wd, unsigned wm);

/* Branches.  Labels may be either unbound (forward branch, patched on
 * bind) or already bound (backward branch, fully encoded here). */
void a64_cbz_w (a64_codegen*, unsigned wn, a64_label*);
void a64_cbnz_w(a64_codegen*, unsigned wn, a64_label*);
void a64_tbnz_w(a64_codegen*, unsigned wn, unsigned bit, a64_label*);
void a64_b     (a64_codegen*, a64_label*);
void a64_b_cond(a64_codegen*, unsigned cond, a64_label*);

/* B / BL to an absolute address (must be within +/-128 MiB / +/-128 MiB
 * respectively of the current write pointer). */
void a64_b_addr (a64_codegen*, const void* addr);

/* B.cond to an absolute address (imm19: must be within +/-1 MiB of the
 * current write pointer). */
void a64_b_cond_addr(a64_codegen*, unsigned cond, const void* addr);

void a64_br (a64_codegen*, unsigned xn);
void a64_blr(a64_codegen*, unsigned xn);
void a64_ret(a64_codegen*);

/* Loads/stores -- imm offset (unsigned scaled). */
void a64_ldr_w_imm  (a64_codegen*, unsigned wt, unsigned xn_sp, uint32_t off);
void a64_str_w_imm  (a64_codegen*, unsigned wt, unsigned xn_sp, uint32_t off);
void a64_ldr_x_imm  (a64_codegen*, unsigned xt, unsigned xn_sp, uint32_t off);
void a64_str_x_imm  (a64_codegen*, unsigned xt, unsigned xn_sp, uint32_t off);
void a64_ldrsw_x_imm(a64_codegen*, unsigned xt, unsigned xn_sp, uint32_t off);
void a64_ldrh_w_imm (a64_codegen*, unsigned wt, unsigned xn_sp, uint32_t off);
void a64_strh_w_imm (a64_codegen*, unsigned wt, unsigned xn_sp, uint32_t off);
void a64_ldrb_w_imm (a64_codegen*, unsigned wt, unsigned xn_sp, uint32_t off);
void a64_strb_w_imm (a64_codegen*, unsigned wt, unsigned xn_sp, uint32_t off);

/* Loads/stores -- signed-offset unscaled (LDUR/STUR). */
void a64_ldur_w(a64_codegen*, unsigned wt, unsigned xn_sp, int off);
void a64_stur_w(a64_codegen*, unsigned wt, unsigned xn_sp, int off);

/* Loads/stores -- register offset.  `shift` is the LSL amount (0 or 2
 * for word, 0 or 3 for dword).  *_reg variants emit LSL #0. */
void a64_ldr_w_reg    (a64_codegen*, unsigned wt, unsigned xn, unsigned xm);
void a64_str_w_reg    (a64_codegen*, unsigned wt, unsigned xn, unsigned xm);
void a64_ldrh_w_reg   (a64_codegen*, unsigned wt, unsigned xn, unsigned xm);
void a64_strh_w_reg   (a64_codegen*, unsigned wt, unsigned xn, unsigned xm);
void a64_ldrb_w_reg   (a64_codegen*, unsigned wt, unsigned xn, unsigned xm);
void a64_strb_w_reg   (a64_codegen*, unsigned wt, unsigned xn, unsigned xm);
void a64_ldr_w_idx_lsl(a64_codegen*, unsigned wt, unsigned xn, unsigned xm, unsigned shift);
void a64_str_w_idx_lsl(a64_codegen*, unsigned wt, unsigned xn, unsigned xm, unsigned shift);
void a64_ldr_x_idx_lsl(a64_codegen*, unsigned xt, unsigned xn, unsigned xm, unsigned shift);
void a64_ldrsw_x_idx_lsl(a64_codegen*, unsigned xt, unsigned xn, unsigned xm, unsigned shift);

/* PRFM (immediate) -- off must be 8-byte aligned (imm12 scaled by 8).
 * prfop: use the A64_PRFOP_* constants. */
#define A64_PRFOP_PLDL1KEEP 0x00u
#define A64_PRFOP_PSTL1KEEP 0x10u
void a64_prfm_imm(a64_codegen*, unsigned prfop, unsigned xn_sp, uint32_t off);

/* Loads/stores -- 32-bit-extended index (Wm with UXTW). */
void a64_ldr_w_uxtw (a64_codegen*, unsigned wt, unsigned xn, unsigned wm, unsigned shift);
void a64_str_w_uxtw (a64_codegen*, unsigned wt, unsigned xn, unsigned wm, unsigned shift);
void a64_ldrh_w_uxtw(a64_codegen*, unsigned wt, unsigned xn, unsigned wm, unsigned shift);
void a64_strh_w_uxtw(a64_codegen*, unsigned wt, unsigned xn, unsigned wm, unsigned shift);

/* Pair load/store (X regs). */
void a64_stp_x_pre (a64_codegen*, unsigned xt1, unsigned xt2, int off);
void a64_ldp_x_post(a64_codegen*, unsigned xt1, unsigned xt2, int off);
void a64_stp_x_off (a64_codegen*, unsigned xt1, unsigned xt2, unsigned xn_sp, int off);
void a64_ldp_x_off (a64_codegen*, unsigned xt1, unsigned xt2, unsigned xn_sp, int off);

void a64_nop(a64_codegen*);

/* --- Constant pool ------------------------------------------------ */

/*
 * Embedded 64-bit constant pool.  Values are deduplicated; identical
 * a64_ldr_x_pool calls share one pool slot.
 *
 *   a64_ldr_x_pool(cg, xd, value)
 *     -> LDR Xd, =value as a single LDR (literal) whose imm19 is
 *        patched at flush time.
 *
 *   a64_pool_flush(cg)
 *     -> aligns to 8 bytes (one NOP if needed), emits every queued
 *        value, rewrites each LDR site's imm19, clears pool state.
 *        The caller must branch over the pool region.
 *
 *   a64_pool_reset(cg)
 *     -> drops queued entries without emitting, leaving their LDR
 *        sites unresolved.
 *
 * Capacity is fixed at compile time (A64_POOL_MAX_*); overflow asserts.
 */
#define A64_POOL_MAX_ENTRIES 64u
#define A64_POOL_MAX_REFS    256u

void     a64_ldr_x_pool (a64_codegen*, unsigned xd, uint64_t value);
void     a64_pool_flush (a64_codegen*);
void     a64_pool_reset (a64_codegen*);
unsigned a64_pool_pending(const a64_codegen*); /* number of queued LDR sites */

/* --- In-place branch patching ------------------------------------- */

/*
 * Rewrite the imm field of an already-emitted branch at `site` so it
 * targets `target` (both absolute addresses).  Returns 1 on success, 0
 * if the offset overflows the encoding.  `site` is the byte address of
 * the instruction word, recorded by the caller at emit time; no label
 * state is tracked, and the caller invalidates I-cache afterwards.
 *
 *   a64_patch_b      : B / BL   (imm26, +/-128 MiB)
 *   a64_patch_b_cond : B.cond   (imm19, +/-1 MiB)
 *   a64_patch_cbz    : CBZ/CBNZ (imm19, +/-1 MiB)
 *   a64_patch_tbz    : TBZ/TBNZ (imm14, +/-32 KiB)
 */
int a64_patch_b      (void* site, const void* target);
int a64_patch_b_cond (void* site, const void* target);
int a64_patch_cbz    (void* site, const void* target);
int a64_patch_tbz    (void* site, const void* target);
/* Rewrite an a64_mov_x_imm4 site (4 words) with a new immediate; the
 * destination register is read back from the first word.  Cannot fail. */
void a64_patch_mov_x_imm4(void* site, uint64_t imm);

#ifdef __cplusplus
}
#endif

#endif /* MDFN_SS_A64EMIT_H */
