/******************************************************************************/
/* Mednafen Sega Saturn Emulation Module                                      */
/******************************************************************************/
/* a64emit.c - minimal AArch64 instruction emitter (C, integer-only)
**  Copyright (C) 2026 pstef
*/

/*
 * Targets Linux/aarch64 with GNU C extensions (inline asm for cache
 * maintenance).  Apple/Windows AArch64 are not handled here; on a
 * non-AArch64 host or on a non-Linux AArch64 host every entry point
 * compiles to either a no-op or NULL.
 */

#include "a64emit.h"

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* ====================================================================
 * Section 1 -- non-aarch64 host: stub out every entry point.
 * ==================================================================== */
#if !(defined(__aarch64__) || defined(__arm64__)) || !defined(__linux__)

struct a64_codegen { int dummy; };

a64_codegen* a64_codegen_create(size_t bytes)                  { (void)bytes; return NULL; }
void         a64_codegen_destroy(a64_codegen* cg)              { (void)cg; }
void*        a64_codegen_base    (const a64_codegen* cg)       { (void)cg; return NULL; }
void*        a64_codegen_wptr    (const a64_codegen* cg)       { (void)cg; return NULL; }
size_t       a64_codegen_offset  (const a64_codegen* cg)       { (void)cg; return 0; }
size_t       a64_codegen_capacity(const a64_codegen* cg)       { (void)cg; return 0; }
size_t       a64_codegen_remaining(const a64_codegen* cg)      { (void)cg; return 0; }
void         a64_codegen_set_wptr(a64_codegen* cg, void* p)    { (void)cg; (void)p; }
void*        a64_codegen_save   (const a64_codegen* cg)        { (void)cg; return NULL; }
void         a64_codegen_restore(a64_codegen* cg, void* p)     { (void)cg; (void)p; }
void         a64_codegen_invalidate(a64_codegen* cg, void* p, size_t b) { (void)cg; (void)p; (void)b; }

void a64_label_reset(a64_label* l) { if(l) memset(l, 0, sizeof *l); }
void a64_label_bind (a64_codegen* cg, a64_label* l) { (void)cg; (void)l; }

int  a64_can_encode_logical_imm32(uint32_t imm) { (void)imm; return 0; }
int  a64_can_encode_logical_imm64(uint64_t imm) { (void)imm; return 0; }
int  a64_can_encode_addsub_imm  (uint32_t imm) { (void)imm; return 0; }

void     a64_ldr_x_pool (a64_codegen* cg, unsigned xd, uint64_t v)
{ (void)cg; (void)xd; (void)v; }
void     a64_pool_flush (a64_codegen* cg)            { (void)cg; }
void     a64_pool_reset (a64_codegen* cg)            { (void)cg; }
unsigned a64_pool_pending(const a64_codegen* cg)     { (void)cg; return 0; }

int  a64_patch_b      (void* s, const void* t) { (void)s; (void)t; return 0; }
int  a64_patch_b_cond (void* s, const void* t) { (void)s; (void)t; return 0; }
int  a64_patch_cbz    (void* s, const void* t) { (void)s; (void)t; return 0; }
int  a64_patch_tbz    (void* s, const void* t) { (void)s; (void)t; return 0; }

/* Every emitter is a no-op on the stub path; argument lists kept so
 * callers compile. */
#define A64_STUB1(name, a1)             void name(a64_codegen* cg, a1)             { (void)cg; }
#define A64_STUB2(name, a1, a2)         void name(a64_codegen* cg, a1, a2)         { (void)cg; }
#define A64_STUB3(name, a1, a2, a3)     void name(a64_codegen* cg, a1, a2, a3)     { (void)cg; }
#define A64_STUB4(name, a1, a2, a3, a4) void name(a64_codegen* cg, a1, a2, a3, a4) { (void)cg; }
#define A64_STUB1_INT(name, a1)             int name(a64_codegen* cg, a1)         { (void)cg; return 0; }
#define A64_STUB3_INT(name, a1, a2, a3) int name(a64_codegen* cg, a1, a2, a3) { (void)cg; return 0; }

A64_STUB2(a64_mov_w_imm, unsigned wd, uint32_t imm)
A64_STUB2(a64_mov_x_imm, unsigned xd, uint64_t imm)
A64_STUB2(a64_mov_w_reg, unsigned wd, unsigned wm)
A64_STUB2(a64_mov_x_reg, unsigned xd, unsigned xm)
A64_STUB1(a64_mov_x_sp,  unsigned xd)
A64_STUB2(a64_movp2r,    unsigned xd, const void* ptr)
A64_STUB2(a64_movp2r_pool, unsigned xd, const void* ptr)
A64_STUB3(a64_add_w_imm, unsigned wd, unsigned wn, uint32_t imm)
A64_STUB3(a64_sub_w_imm, unsigned wd, unsigned wn, uint32_t imm)
A64_STUB3(a64_add_x_imm, unsigned xd, unsigned xn, uint32_t imm)
A64_STUB3(a64_sub_x_imm, unsigned xd, unsigned xn, uint32_t imm)
A64_STUB3_INT(a64_try_add_w_imm, unsigned wd, unsigned wn, uint32_t imm)
A64_STUB3_INT(a64_try_sub_w_imm, unsigned wd, unsigned wn, uint32_t imm)
A64_STUB3_INT(a64_try_add_x_imm, unsigned xd, unsigned xn, uint32_t imm)
A64_STUB3_INT(a64_try_sub_x_imm, unsigned xd, unsigned xn, uint32_t imm)
A64_STUB3(a64_add_w_reg, unsigned wd, unsigned wn, unsigned wm)
A64_STUB3(a64_sub_w_reg, unsigned wd, unsigned wn, unsigned wm)
A64_STUB3(a64_add_x_reg, unsigned xd, unsigned xn, unsigned xm)
A64_STUB3(a64_sub_x_reg, unsigned xd, unsigned xn, unsigned xm)
A64_STUB3(a64_add_x_reg_sxtw, unsigned xd, unsigned xn, unsigned wm)
A64_STUB3(a64_adds_w_imm,unsigned wd, unsigned wn, uint32_t imm)
A64_STUB3(a64_subs_w_imm,unsigned wd, unsigned wn, uint32_t imm)
A64_STUB3(a64_adds_w_reg,unsigned wd, unsigned wn, unsigned wm)
A64_STUB3(a64_subs_w_reg,unsigned wd, unsigned wn, unsigned wm)
A64_STUB3(a64_ands_w_reg,unsigned wd, unsigned wn, unsigned wm)
A64_STUB2(a64_cmp_w_imm, unsigned wn, uint32_t imm)
A64_STUB2(a64_cmp_w_reg, unsigned wn, unsigned wm)
A64_STUB2(a64_tst_w_reg, unsigned wn, unsigned wm)
A64_STUB2(a64_tst_x_reg, unsigned xn, unsigned xm)
A64_STUB2(a64_cset_w,    unsigned wd, unsigned cond)
A64_STUB4(a64_csel_w,    unsigned wd, unsigned wn, unsigned wm, unsigned cond)
A64_STUB3_INT(a64_and_w_imm, unsigned wd, unsigned wn, uint32_t imm)
A64_STUB3_INT(a64_orr_w_imm, unsigned wd, unsigned wn, uint32_t imm)
A64_STUB3(a64_and_w_reg, unsigned wd, unsigned wn, unsigned wm)
A64_STUB3(a64_orr_w_reg, unsigned wd, unsigned wn, unsigned wm)
A64_STUB3(a64_eor_w_reg, unsigned wd, unsigned wn, unsigned wm)
A64_STUB3(a64_and_x_reg, unsigned xd, unsigned xn, unsigned xm)
A64_STUB3(a64_eor_x_reg, unsigned xd, unsigned xn, unsigned xm)
A64_STUB3(a64_bic_x_reg, unsigned xd, unsigned xn, unsigned xm)
A64_STUB4(a64_orr_w_reg_lsl, unsigned wd, unsigned wn, unsigned wm, unsigned shift)
A64_STUB3(a64_lsl_w_imm, unsigned wd, unsigned wn, unsigned shift)
A64_STUB3(a64_lsr_w_imm, unsigned wd, unsigned wn, unsigned shift)
A64_STUB3(a64_asr_w_imm, unsigned wd, unsigned wn, unsigned shift)
A64_STUB3(a64_ror_w_imm, unsigned wd, unsigned wn, unsigned shift)
A64_STUB3(a64_asr_w_reg, unsigned wd, unsigned wn, unsigned wm)
A64_STUB3(a64_lsl_x_imm, unsigned xd, unsigned xn, unsigned shift)
A64_STUB3(a64_lsr_x_imm, unsigned xd, unsigned xn, unsigned shift)
A64_STUB3(a64_asr_x_imm, unsigned xd, unsigned xn, unsigned shift)
A64_STUB4(a64_ubfx_w, unsigned wd, unsigned wn, unsigned lsb, unsigned width)
A64_STUB4(a64_sbfx_w, unsigned wd, unsigned wn, unsigned lsb, unsigned width)
A64_STUB4(a64_bfi_w,  unsigned wd, unsigned wn, unsigned lsb, unsigned width)
A64_STUB4(a64_bfi_x,  unsigned xd, unsigned xn, unsigned lsb, unsigned width)
A64_STUB2(a64_sxtw,   unsigned xd, unsigned wn)
A64_STUB2(a64_clz_w,  unsigned wd, unsigned wn)
A64_STUB3(a64_smull,  unsigned xd, unsigned wn, unsigned wm)
A64_STUB2(a64_neg_w,  unsigned wd, unsigned wm)
A64_STUB2(a64_cbz_w,  unsigned wn, a64_label* l)
A64_STUB2(a64_cbnz_w, unsigned wn, a64_label* l)
A64_STUB3(a64_tbnz_w, unsigned wn, unsigned bit, a64_label* l)
A64_STUB1(a64_b,      a64_label* l)
A64_STUB2(a64_b_cond, unsigned cond, a64_label* l)
A64_STUB1(a64_b_addr, const void* addr)
A64_STUB1(a64_br,     unsigned xn)
A64_STUB1(a64_blr,    unsigned xn)
void a64_ret(a64_codegen* cg) { (void)cg; }
A64_STUB3(a64_ldr_w_imm,  unsigned wt, unsigned xn, uint32_t off)
A64_STUB3(a64_str_w_imm,  unsigned wt, unsigned xn, uint32_t off)
A64_STUB3(a64_ldr_x_imm,  unsigned xt, unsigned xn, uint32_t off)
A64_STUB3(a64_str_x_imm,  unsigned xt, unsigned xn, uint32_t off)
A64_STUB3(a64_ldrsw_x_imm,unsigned xt, unsigned xn, uint32_t off)
A64_STUB3(a64_ldrh_w_imm, unsigned wt, unsigned xn, uint32_t off)
A64_STUB3(a64_strh_w_imm, unsigned wt, unsigned xn, uint32_t off)
A64_STUB3(a64_ldrb_w_imm, unsigned wt, unsigned xn, uint32_t off)
A64_STUB3(a64_strb_w_imm, unsigned wt, unsigned xn, uint32_t off)
A64_STUB3(a64_ldur_w, unsigned wt, unsigned xn, int off)
A64_STUB3(a64_stur_w, unsigned wt, unsigned xn, int off)
A64_STUB3(a64_ldr_w_reg,  unsigned wt, unsigned xn, unsigned xm)
A64_STUB3(a64_str_w_reg,  unsigned wt, unsigned xn, unsigned xm)
A64_STUB3(a64_ldrh_w_reg, unsigned wt, unsigned xn, unsigned xm)
A64_STUB3(a64_strh_w_reg, unsigned wt, unsigned xn, unsigned xm)
A64_STUB3(a64_ldrb_w_reg, unsigned wt, unsigned xn, unsigned xm)
A64_STUB3(a64_strb_w_reg, unsigned wt, unsigned xn, unsigned xm)
A64_STUB4(a64_ldr_w_idx_lsl, unsigned wt, unsigned xn, unsigned xm, unsigned shift)
A64_STUB4(a64_str_w_idx_lsl, unsigned wt, unsigned xn, unsigned xm, unsigned shift)
A64_STUB4(a64_ldr_x_idx_lsl, unsigned xt, unsigned xn, unsigned xm, unsigned shift)
A64_STUB4(a64_ldr_w_uxtw,  unsigned wt, unsigned xn, unsigned wm, unsigned shift)
A64_STUB4(a64_str_w_uxtw,  unsigned wt, unsigned xn, unsigned wm, unsigned shift)
A64_STUB4(a64_ldrh_w_uxtw, unsigned wt, unsigned xn, unsigned wm, unsigned shift)
A64_STUB4(a64_strh_w_uxtw, unsigned wt, unsigned xn, unsigned wm, unsigned shift)
A64_STUB3(a64_stp_x_pre,  unsigned xt1, unsigned xt2, int off)
A64_STUB3(a64_ldp_x_post, unsigned xt1, unsigned xt2, int off)
A64_STUB4(a64_stp_x_off,  unsigned xt1, unsigned xt2, unsigned xn, int off)
A64_STUB4(a64_ldp_x_off,  unsigned xt1, unsigned xt2, unsigned xn, int off)
void a64_nop(a64_codegen* cg) { (void)cg; }

#else /* AArch64 + Linux */

#include <sys/mman.h>

/* ====================================================================
 * Section 2 -- CodeGenerator state.
 * ==================================================================== */

struct a64_pool_ref {
 ptrdiff_t wb_off;     /* byte offset of LDR site from base */
 unsigned  entry;      /* index into pool_values[] */
};

struct a64_codegen {
 uint32_t* base;    /* base of the mmap'd region */
 uint32_t* wp;      /* current write pointer (always within [base, end]) */
 size_t    size;    /* bytes in the region */

 /* Embedded 64-bit constant pool. */
 uint64_t  pool_values[A64_POOL_MAX_ENTRIES];
 unsigned  pool_count;
 struct a64_pool_ref pool_refs[A64_POOL_MAX_REFS];
 unsigned  pool_ref_count;
};

a64_codegen* a64_codegen_create(size_t bytes)
{
 a64_codegen* cg;
 void* mem;

 cg = (a64_codegen*)calloc(1, sizeof *cg);
 if(!cg) return NULL;

 mem = mmap(NULL, bytes, PROT_READ | PROT_WRITE | PROT_EXEC,
            MAP_ANON | MAP_PRIVATE, -1, 0);
 if(mem == MAP_FAILED) {
  free(cg);
  return NULL;
 }

 cg->base = (uint32_t*)mem;
 cg->wp   = (uint32_t*)mem;
 cg->size = bytes;
 return cg;
}

void a64_codegen_destroy(a64_codegen* cg)
{
 if(!cg) return;
 if(cg->base) munmap(cg->base, cg->size);
 free(cg);
}

void*  a64_codegen_base    (const a64_codegen* cg) { return cg ? (void*)cg->base : NULL; }
void*  a64_codegen_wptr    (const a64_codegen* cg) { return cg ? (void*)cg->wp   : NULL; }
size_t a64_codegen_capacity(const a64_codegen* cg) { return cg ? cg->size : 0u; }

size_t a64_codegen_offset(const a64_codegen* cg)
{
 if(!cg) return 0;
 return (size_t)((char*)cg->wp - (char*)cg->base);
}

size_t a64_codegen_remaining(const a64_codegen* cg)
{
 size_t used;
 if(!cg) return 0;
 used = (size_t)((char*)cg->wp - (char*)cg->base);
 return (used <= cg->size) ? (cg->size - used) : 0u;
}

void a64_codegen_set_wptr(a64_codegen* cg, void* p)
{
 if(cg) cg->wp = (uint32_t*)p;
}

void* a64_codegen_save(const a64_codegen* cg)
{
 return cg ? (void*)cg->wp : NULL;
}

void a64_codegen_restore(a64_codegen* cg, void* p)
{
 if(cg) cg->wp = (uint32_t*)p;
}

/*
 * Architectural icache invalidation for ARMv8: clean each D-cache line
 * to PoU, then invalidate each I-cache line to PoU.  The CTR_EL0 read
 * gives us per-line size; we cache the floor across calls.
 */
void a64_codegen_invalidate(a64_codegen* cg, void* mem, size_t bytes)
{
 static size_t icache_line_size = 0x10000;
 static size_t dcache_line_size = 0x10000;
 uint64_t ctr;
 size_t isize, dsize;
 uintptr_t addr, end;

 (void)cg;
 if(!mem || !bytes) return;

 __asm__ volatile("mrs %0, ctr_el0" : "=r"(ctr));
 dsize = (size_t)4u << ((ctr >> 16) & 0xfu);
 isize = (size_t)4u << ((ctr >>  0) & 0xfu);
 if(dsize < dcache_line_size) dcache_line_size = dsize;
 if(isize < icache_line_size) icache_line_size = isize;
 dsize = dcache_line_size;
 isize = icache_line_size;

 end = (uintptr_t)mem + bytes;

 for(addr = ((uintptr_t)mem) & ~(dsize - 1); addr < end; addr += dsize)
  __asm__ volatile("dc cvau, %0" :: "r"(addr) : "memory");
 __asm__ volatile("dsb ish" ::: "memory");

 for(addr = ((uintptr_t)mem) & ~(isize - 1); addr < end; addr += isize)
  __asm__ volatile("ic ivau, %0" :: "r"(addr) : "memory");
 __asm__ volatile("dsb ish\nisb" ::: "memory");
}

/* ====================================================================
 * Section 3 -- low-level emit + immediate encoders.
 * ==================================================================== */

#define A64_REG(r)   ((uint32_t)((r) & 0x1Fu))

/* Patch kinds for forward-branch resolution. */
enum {
 A64_PATCH_B     = 0, /* imm26 at bits 25..0  (B / BL)  */
 A64_PATCH_BCOND = 1, /* imm19 at bits 23..5  (B.cond / CBZ / CBNZ) */
 A64_PATCH_TBNZ  = 2  /* imm14 at bits 18..5  (TBZ / TBNZ) */
};

static void emit_w(a64_codegen* cg, uint32_t w)
{
 *cg->wp++ = w;
}

static int sint_fits(int64_t v, unsigned bits)
{
 int64_t lo = -((int64_t)1 << (bits - 1));
 int64_t hi =  ((int64_t)1 << (bits - 1)) - 1;
 return v >= lo && v <= hi;
}

/*
 * Encode `imm` as a 12-bit AddSubImm value with optional shift-by-12.
 * Returns 1 on success and writes (shift22|imm10) into *out, where
 * shift22 is the bit-22 marker (0 or (1<<22)) and imm10 is the 12-bit
 * value left-shifted into bits 21..10.
 */
static int encode_addsub_imm(uint32_t imm, uint32_t* out)
{
 if(imm <= 0xFFFu) {
  *out = imm << 10;
  return 1;
 }
 if((imm & 0xFFFu) == 0u && (imm >> 12) <= 0xFFFu) {
  *out = ((uint32_t)1u << 22) | ((imm >> 12) << 10);
  return 1;
 }
 return 0;
}

/*
 * Encode a 64-bit value as a 13-bit (N|immr|imms) logical-immediate
 * bit-pattern.  Returns 1 on success and writes the field into *out.
 * Algorithm matches the ARMv8-A "DecodeBitMasks" pseudocode inverse.
 */
static int encode_logical_imm64(uint64_t imm, uint32_t* out)
{
 uint64_t imm_low;
 unsigned size, ones;
 unsigned R;
 unsigned int popc;
 uint64_t pattern;
 uint64_t rot, mask;
 unsigned half;
 uint64_t lo, hi;
 uint32_t N, immr, imms;

 if(imm == 0u || imm == ~(uint64_t)0u) return 0;

 imm_low = imm;
 size = 64;
 while(size > 2u) {
  half = size >> 1;
  if(half == 64u) mask = ~(uint64_t)0u;
  else            mask = ((uint64_t)1u << half) - 1u;
  lo = imm_low & mask;
  hi = (imm_low >> half) & mask;
  if(lo != hi) break;
  imm_low = lo;
  size = half;
 }

 popc = 0u;
 {
  uint64_t v = imm_low;
  while(v) {
   popc += (unsigned)(v & 1u);
   v >>= 1;
  }
 }
 ones = popc;
 if(ones == 0u || ones == size) return 0;

 pattern = (ones >= 64u) ? ~(uint64_t)0u : (((uint64_t)1u << ones) - 1u);
 if(size == 64u) mask = ~(uint64_t)0u;
 else            mask = ((uint64_t)1u << size) - 1u;

 /* Find R such that ROL(pattern, R) == imm_low.  immr is the ROR
  * amount applied to the canonical low-aligned pattern, the inverse:
  * immr = (size - R) mod size.  Storing R directly encodes `#0x2000`
  * where the caller asked for `#0x80000`. */
 for(R = 0u; R < size; ++R) {
  if(R == 0u) rot = pattern & mask;
  else        rot = ((pattern << R) | (pattern >> (size - R))) & mask;
  if(rot == imm_low) break;
 }
 if(R == size) return 0;

 immr = (size - R) & (size - 1u);
 if(size == 64u) {
  N = 1u;
  imms = ones - 1u;
 } else {
  /* For element size 2^k (k=1..5), imms[5:k+1] = all-1, imms[k] = 0,
   * imms[k-1:0] = ones-1, i.e. ((~(size-1)) << 1 | (ones-1)) masked to
   * 6 bits.  Without the shift, imms[k] = 1 and DecodeBitMasks()
   * returns UNDEFINED. */
  N = 0u;
  imms = (((~(unsigned)(size - 1u)) << 1) | (ones - 1u)) & 0x3Fu;
 }
 *out = (N << 12) | (immr << 6) | imms;
 return 1;
}

static int encode_logical_imm32(uint32_t imm, uint32_t* out)
{
 /* 32-bit form requires N=0; replicate imm to 64 bits and reuse. */
 uint64_t imm64 = ((uint64_t)imm << 32) | imm;
 uint32_t enc;
 if(!encode_logical_imm64(imm64, &enc)) return 0;
 if((enc >> 12) & 1u) return 0; /* N must be 0 for 32-bit form */
 *out = enc;
 return 1;
}

int a64_can_encode_logical_imm32(uint32_t imm)
{
 uint32_t enc;
 return encode_logical_imm32(imm, &enc);
}

int a64_can_encode_logical_imm64(uint64_t imm)
{
 uint32_t enc;
 return encode_logical_imm64(imm, &enc);
}

int a64_can_encode_addsub_imm(uint32_t imm)
{
 uint32_t fld;
 return encode_addsub_imm(imm, &fld);
}

/*
 * MovImm16 validity: imm fits in 16 bits left-shifted by 0/16 (W) or
 * 0/16/32/48 (X).  Writes the (hw, imm16) pair into *out_hw / *out_imm
 * on success.
 */
static int try_mov_imm16(uint64_t imm, int is_x, unsigned* out_hw, uint32_t* out_imm)
{
 unsigned max_hw = is_x ? 4u : 2u;
 unsigned hw;
 for(hw = 0; hw < max_hw; ++hw) {
  uint64_t mask16 = (uint64_t)0xFFFFu << (hw * 16u);
  if((imm & ~mask16) == 0u) {
   *out_hw  = hw;
   *out_imm = (uint32_t)((imm >> (hw * 16u)) & 0xFFFFu);
   return 1;
  }
 }
 return 0;
}

/* ====================================================================
 * Section 4 -- labels.
 * ==================================================================== */

void a64_label_reset(a64_label* l)
{
 if(l) memset(l, 0, sizeof *l);
}

static void label_add_patch(a64_label* lbl, ptrdiff_t wb_off, unsigned kind)
{
 assert(lbl->patch_count < A64_LABEL_MAX_PATCHES);
 lbl->patches[lbl->patch_count].wb_off = wb_off;
 lbl->patches[lbl->patch_count].kind   = kind;
 lbl->patch_count++;
}

void a64_label_bind(a64_codegen* cg, a64_label* lbl)
{
 unsigned i;
 ptrdiff_t target;

 assert(!lbl->bound);
 target = (char*)cg->wp - (char*)cg->base;
 lbl->target_off = target;
 lbl->bound      = 1;

 for(i = 0; i < lbl->patch_count; ++i) {
  ptrdiff_t wb    = lbl->patches[i].wb_off;
  unsigned  kind  = lbl->patches[i].kind;
  int64_t   delta = (int64_t)((target - wb) >> 2);
  uint32_t* p     = (uint32_t*)((char*)cg->base + wb);

  switch(kind) {
  case A64_PATCH_B:
   assert(sint_fits(delta, 26));
   *p |= (uint32_t)((uint64_t)delta & 0x3FFFFFFu);
   break;
  case A64_PATCH_BCOND:
   assert(sint_fits(delta, 19));
   *p |= (uint32_t)(((uint64_t)delta & 0x7FFFFu) << 5);
   break;
  case A64_PATCH_TBNZ:
   assert(sint_fits(delta, 14));
   *p |= (uint32_t)(((uint64_t)delta & 0x3FFFu) << 5);
   break;
  default:
   assert(0);
  }
 }
 lbl->patch_count = 0;
}

/* ====================================================================
 * Section 5 -- instruction emitters.
 *
 * Each function builds the 32-bit instruction word from a base opcode
 * pattern plus register and immediate fields, then appends it via
 * emit_w().  Encoding references the ARMv8-A Reference Manual.
 * ==================================================================== */

/* --- MOV / pointer materialisation -------------------------------- */

void a64_mov_w_imm(a64_codegen* cg, unsigned wd, uint32_t imm)
{
 unsigned hw;
 uint32_t imm16;
 uint32_t enc;

 if(A64_REG(wd) == 31u) return; /* MOV WZR, imm : no-op */

 if(try_mov_imm16(imm, 0, &hw, &imm16)) {
  /* MOVZ Wd, #imm16, LSL #(hw*16) */
  emit_w(cg, 0x52800000u | ((uint32_t)hw << 21) | (imm16 << 5) | A64_REG(wd));
  return;
 }
 if(try_mov_imm16((uint32_t)~imm, 0, &hw, &imm16)) {
  /* MOVN Wd, #~imm16, LSL #(hw*16) */
  emit_w(cg, 0x12800000u | ((uint32_t)hw << 21) | (imm16 << 5) | A64_REG(wd));
  return;
 }
 if(encode_logical_imm32(imm, &enc)) {
  /* ORR Wd, WZR, #imm */
  emit_w(cg, 0x32000000u | (enc << 10) | (31u << 5) | A64_REG(wd));
  return;
 }
 /* MOVZ low half + MOVK high half. */
 emit_w(cg, 0x52800000u | ((uint32_t)0u << 21) | ((imm & 0xFFFFu) << 5) | A64_REG(wd));
 emit_w(cg, 0x72800000u | ((uint32_t)1u << 21) | (((imm >> 16) & 0xFFFFu) << 5) | A64_REG(wd));
}

void a64_mov_x_imm(a64_codegen* cg, unsigned xd, uint64_t imm)
{
 unsigned hw;
 uint32_t imm16;
 uint32_t enc;
 unsigned hword[4];
 unsigned zero_count;
 unsigned ones_count;
 unsigned filler;
 uint32_t seed_op;
 int      movz_done;
 unsigned i;

 if(A64_REG(xd) == 31u) return;

 if((imm >> 32) == 0u) {
  a64_mov_w_imm(cg, xd, (uint32_t)imm);
  return;
 }
 if(try_mov_imm16(imm, 1, &hw, &imm16)) {
  emit_w(cg, 0xD2800000u | ((uint32_t)hw << 21) | (imm16 << 5) | A64_REG(xd));
  return;
 }
 if(try_mov_imm16(~imm, 1, &hw, &imm16)) {
  emit_w(cg, 0x92800000u | ((uint32_t)hw << 21) | (imm16 << 5) | A64_REG(xd));
  return;
 }
 if(encode_logical_imm64(imm, &enc)) {
  /* ORR Xd, XZR, #imm  (N|immr|imms shifted into bits 22..10) */
  emit_w(cg, 0xB2000000u | (enc << 10) | (31u << 5) | A64_REG(xd));
  return;
 }

 /* Multi-hword fallback.  MOVN seeds every hword to 0xFFFF and MOVZ to
  * zero, so pick the seed matching the more common hword and skip the
  * MOVKs it already covers. */
 zero_count = 0u;
 ones_count = 0u;
 for(i = 0; i < 4u; ++i) {
  hword[i] = (unsigned)((imm >> (i * 16u)) & 0xFFFFu);
  if(hword[i] == 0u)           zero_count++;
  else if(hword[i] == 0xFFFFu) ones_count++;
 }
 if(ones_count > zero_count) {
  filler  = 0xFFFFu;
  seed_op = 0x92800000u; /* MOVN (x) */
 } else {
  filler  = 0u;
  seed_op = 0xD2800000u; /* MOVZ (x) */
 }

 movz_done = 0;
 for(i = 0; i < 4u; ++i) {
  unsigned hw16 = hword[i];
  if(hw16 == filler) continue;
  if(!movz_done) {
   uint32_t v = (filler == 0xFFFFu) ? ((~hw16) & 0xFFFFu) : hw16;
   emit_w(cg, seed_op | ((uint32_t)i << 21) | (v << 5) | A64_REG(xd));
   movz_done = 1;
  } else {
   emit_w(cg, 0xF2800000u | ((uint32_t)i << 21) | (hw16 << 5) | A64_REG(xd));
  }
 }
 if(!movz_done) /* Unreachable: all-zero and all-ones are handled above. */
  emit_w(cg, seed_op | A64_REG(xd));
}

void a64_mov_w_reg(a64_codegen* cg, unsigned wd, unsigned wm)
{
 /* ORR Wd, WZR, Wm */
 emit_w(cg, 0x2A0003E0u | (A64_REG(wm) << 16) | A64_REG(wd));
}

void a64_mov_x_reg(a64_codegen* cg, unsigned xd, unsigned xm)
{
 emit_w(cg, 0xAA0003E0u | (A64_REG(xm) << 16) | A64_REG(xd));
}

void a64_mov_x_sp(a64_codegen* cg, unsigned xd)
{
 /* ADD Xd_sp, SP, #0 */
 emit_w(cg, 0x91000000u | (31u << 5) | A64_REG(xd));
}

void a64_movp2r(a64_codegen* cg, unsigned xd, const void* ptr)
{
 uintptr_t here = (uintptr_t)cg->wp;
 uintptr_t targ = (uintptr_t)ptr;
 int64_t   diff = (int64_t)(targ - here);
 uintptr_t here_page, targ_page;
 int64_t   page_diff;

 /* Try ADR (+/-1 MiB).  imm21 = signed 21-bit byte offset. */
 if(diff >= -((int64_t)1 << 20) && diff < ((int64_t)1 << 20)) {
  uint32_t imm21 = (uint32_t)((uint64_t)diff & 0x1FFFFFu);
  uint32_t immlo = imm21 & 0x3u;
  uint32_t immhi = (imm21 >> 2) & 0x7FFFFu;
  emit_w(cg, 0x10000000u | (immlo << 29) | (immhi << 5) | A64_REG(xd));
  return;
 }

 /* Try ADRP+ADD (+/-4 GiB, 4 KiB-aligned page). */
 here_page = here & ~(uintptr_t)0xFFFu;
 targ_page = targ & ~(uintptr_t)0xFFFu;
 page_diff = ((int64_t)targ_page - (int64_t)here_page) >> 12;
 if(page_diff >= -((int64_t)1 << 20) && page_diff < ((int64_t)1 << 20)) {
  uint32_t imm21 = (uint32_t)((uint64_t)page_diff & 0x1FFFFFu);
  uint32_t immlo = imm21 & 0x3u;
  uint32_t immhi = (imm21 >> 2) & 0x7FFFFu;
  emit_w(cg, 0x90000000u | (immlo << 29) | (immhi << 5) | A64_REG(xd));
  /* ADD Xd, Xd, #(targ & 0xFFF) */
  a64_add_x_imm(cg, xd, xd, (uint32_t)(targ & 0xFFFu));
  return;
 }

 /* Fallback: full 64-bit immediate. */
 a64_mov_x_imm(cg, xd, (uint64_t)targ);
}

/* --- Add/Sub (imm and reg) ---------------------------------------- */

static void emit_addsub_imm(a64_codegen* cg, uint32_t base,
                            unsigned rd, unsigned rn, uint32_t imm)
{
 uint32_t fld;
 int ok = encode_addsub_imm(imm, &fld);
 assert(ok); (void)ok;
 emit_w(cg, base | fld | (A64_REG(rn) << 5) | A64_REG(rd));
}

void a64_add_w_imm(a64_codegen* cg, unsigned wd, unsigned wn, uint32_t imm)
{ emit_addsub_imm(cg, 0x11000000u, wd, wn, imm); }
void a64_sub_w_imm(a64_codegen* cg, unsigned wd, unsigned wn, uint32_t imm)
{ emit_addsub_imm(cg, 0x51000000u, wd, wn, imm); }
void a64_add_x_imm(a64_codegen* cg, unsigned xd, unsigned xn, uint32_t imm)
{ emit_addsub_imm(cg, 0x91000000u, xd, xn, imm); }
void a64_sub_x_imm(a64_codegen* cg, unsigned xd, unsigned xn, uint32_t imm)
{ emit_addsub_imm(cg, 0xD1000000u, xd, xn, imm); }

static int try_emit_addsub_imm(a64_codegen* cg, uint32_t base,
                               unsigned rd, unsigned rn, uint32_t imm)
{
 uint32_t fld;
 if(!encode_addsub_imm(imm, &fld)) return 0;
 emit_w(cg, base | fld | (A64_REG(rn) << 5) | A64_REG(rd));
 return 1;
}

int a64_try_add_w_imm(a64_codegen* cg, unsigned wd, unsigned wn, uint32_t imm)
{ return try_emit_addsub_imm(cg, 0x11000000u, wd, wn, imm); }
int a64_try_sub_w_imm(a64_codegen* cg, unsigned wd, unsigned wn, uint32_t imm)
{ return try_emit_addsub_imm(cg, 0x51000000u, wd, wn, imm); }
int a64_try_add_x_imm(a64_codegen* cg, unsigned xd, unsigned xn, uint32_t imm)
{ return try_emit_addsub_imm(cg, 0x91000000u, xd, xn, imm); }
int a64_try_sub_x_imm(a64_codegen* cg, unsigned xd, unsigned xn, uint32_t imm)
{ return try_emit_addsub_imm(cg, 0xD1000000u, xd, xn, imm); }

void a64_adds_w_imm(a64_codegen* cg, unsigned wd, unsigned wn, uint32_t imm)
{ emit_addsub_imm(cg, 0x31000000u, wd, wn, imm); }
void a64_subs_w_imm(a64_codegen* cg, unsigned wd, unsigned wn, uint32_t imm)
{ emit_addsub_imm(cg, 0x71000000u, wd, wn, imm); }

void a64_add_w_reg(a64_codegen* cg, unsigned wd, unsigned wn, unsigned wm)
{ emit_w(cg, 0x0B000000u | (A64_REG(wm) << 16) | (A64_REG(wn) << 5) | A64_REG(wd)); }
void a64_sub_w_reg(a64_codegen* cg, unsigned wd, unsigned wn, unsigned wm)
{ emit_w(cg, 0x4B000000u | (A64_REG(wm) << 16) | (A64_REG(wn) << 5) | A64_REG(wd)); }
void a64_add_x_reg(a64_codegen* cg, unsigned xd, unsigned xn, unsigned xm)
{ emit_w(cg, 0x8B000000u | (A64_REG(xm) << 16) | (A64_REG(xn) << 5) | A64_REG(xd)); }
void a64_add_x_reg_sxtw(a64_codegen* cg, unsigned xd_sp, unsigned xn_sp, unsigned wm)
{ /* ADD (extended register), 64-bit, option=SXTW (0b110), shift 0. */
 emit_w(cg, 0x8B20C000u | (A64_REG(wm) << 16) | (A64_REG(xn_sp) << 5) | A64_REG(xd_sp)); }
void a64_sub_x_reg(a64_codegen* cg, unsigned xd, unsigned xn, unsigned xm)
{ emit_w(cg, 0xCB000000u | (A64_REG(xm) << 16) | (A64_REG(xn) << 5) | A64_REG(xd)); }

void a64_adds_w_reg(a64_codegen* cg, unsigned wd, unsigned wn, unsigned wm)
{ emit_w(cg, 0x2B000000u | (A64_REG(wm) << 16) | (A64_REG(wn) << 5) | A64_REG(wd)); }
void a64_subs_w_reg(a64_codegen* cg, unsigned wd, unsigned wn, unsigned wm)
{ emit_w(cg, 0x6B000000u | (A64_REG(wm) << 16) | (A64_REG(wn) << 5) | A64_REG(wd)); }
void a64_ands_w_reg(a64_codegen* cg, unsigned wd, unsigned wn, unsigned wm)
{ emit_w(cg, 0x6A000000u | (A64_REG(wm) << 16) | (A64_REG(wn) << 5) | A64_REG(wd)); }

void a64_cmp_w_imm(a64_codegen* cg, unsigned wn, uint32_t imm)
{ a64_subs_w_imm(cg, 31u, wn, imm); }
void a64_cmp_w_reg(a64_codegen* cg, unsigned wn, unsigned wm)
{ a64_subs_w_reg(cg, 31u, wn, wm); }
void a64_tst_w_reg(a64_codegen* cg, unsigned wn, unsigned wm)
{ a64_ands_w_reg(cg, 31u, wn, wm); }
void a64_tst_x_reg(a64_codegen* cg, unsigned xn, unsigned xm)
{ /* ANDS Xd=31, Xn, Xm */
 emit_w(cg, 0xEA000000u | (A64_REG(xm) << 16) | (A64_REG(xn) << 5) | 31u);
}

void a64_cset_w(a64_codegen* cg, unsigned wd, unsigned cond)
{
 /* CSINC Wd, WZR, WZR, !cond */
 uint32_t inv = (cond ^ 1u) & 0xFu;
 emit_w(cg, 0x1A9F07E0u | (inv << 12) | A64_REG(wd));
}

void a64_csel_w(a64_codegen* cg, unsigned wd, unsigned wn, unsigned wm, unsigned cond)
{
 emit_w(cg, 0x1A800000u | (A64_REG(wm) << 16) | ((cond & 0xFu) << 12)
            | (A64_REG(wn) << 5) | A64_REG(wd));
}

/* --- Bitwise -------------------------------------------------------- */

int a64_and_w_imm(a64_codegen* cg, unsigned wd, unsigned wn, uint32_t imm)
{
 uint32_t enc;
 if(!encode_logical_imm32(imm, &enc)) return 0;
 emit_w(cg, 0x12000000u | (enc << 10) | (A64_REG(wn) << 5) | A64_REG(wd));
 return 1;
}

int a64_orr_w_imm(a64_codegen* cg, unsigned wd, unsigned wn, uint32_t imm)
{
 uint32_t enc;
 if(!encode_logical_imm32(imm, &enc)) return 0;
 emit_w(cg, 0x32000000u | (enc << 10) | (A64_REG(wn) << 5) | A64_REG(wd));
 return 1;
}

void a64_and_w_reg(a64_codegen* cg, unsigned wd, unsigned wn, unsigned wm)
{ emit_w(cg, 0x0A000000u | (A64_REG(wm) << 16) | (A64_REG(wn) << 5) | A64_REG(wd)); }
void a64_orr_w_reg(a64_codegen* cg, unsigned wd, unsigned wn, unsigned wm)
{ emit_w(cg, 0x2A000000u | (A64_REG(wm) << 16) | (A64_REG(wn) << 5) | A64_REG(wd)); }
void a64_eor_w_reg(a64_codegen* cg, unsigned wd, unsigned wn, unsigned wm)
{ emit_w(cg, 0x4A000000u | (A64_REG(wm) << 16) | (A64_REG(wn) << 5) | A64_REG(wd)); }
void a64_and_x_reg(a64_codegen* cg, unsigned xd, unsigned xn, unsigned xm)
{ emit_w(cg, 0x8A000000u | (A64_REG(xm) << 16) | (A64_REG(xn) << 5) | A64_REG(xd)); }
void a64_eor_x_reg(a64_codegen* cg, unsigned xd, unsigned xn, unsigned xm)
{ emit_w(cg, 0xCA000000u | (A64_REG(xm) << 16) | (A64_REG(xn) << 5) | A64_REG(xd)); }
void a64_bic_x_reg(a64_codegen* cg, unsigned xd, unsigned xn, unsigned xm)
{ emit_w(cg, 0x8A200000u | (A64_REG(xm) << 16) | (A64_REG(xn) << 5) | A64_REG(xd)); }

void a64_orr_w_reg_lsl(a64_codegen* cg, unsigned wd, unsigned wn, unsigned wm,
                       unsigned shift)
{
 assert(shift < 32u);
 emit_w(cg, 0x2A000000u | (A64_REG(wm) << 16) | (shift << 10)
            | (A64_REG(wn) << 5) | A64_REG(wd));
}

/* --- Shifts (imm and reg) ------------------------------------------ */

void a64_lsl_w_imm(a64_codegen* cg, unsigned wd, unsigned wn, unsigned s)
{
 uint32_t immr = (32u - s) & 0x1Fu;
 uint32_t imms = 31u - s;
 emit_w(cg, 0x53000000u | (immr << 16) | (imms << 10)
            | (A64_REG(wn) << 5) | A64_REG(wd));
}
void a64_lsr_w_imm(a64_codegen* cg, unsigned wd, unsigned wn, unsigned s)
{
 emit_w(cg, 0x53000000u | ((s & 0x1Fu) << 16) | (31u << 10)
            | (A64_REG(wn) << 5) | A64_REG(wd));
}
void a64_asr_w_imm(a64_codegen* cg, unsigned wd, unsigned wn, unsigned s)
{
 emit_w(cg, 0x13000000u | ((s & 0x1Fu) << 16) | (31u << 10)
            | (A64_REG(wn) << 5) | A64_REG(wd));
}
void a64_ror_w_imm(a64_codegen* cg, unsigned wd, unsigned wn, unsigned s)
{
 /* EXTR Wd, Wn, Wn, #s */
 emit_w(cg, 0x13800000u | (A64_REG(wn) << 16) | ((s & 0x1Fu) << 10)
            | (A64_REG(wn) << 5) | A64_REG(wd));
}
void a64_asr_w_reg(a64_codegen* cg, unsigned wd, unsigned wn, unsigned wm)
{
 /* ASRV Wd, Wn, Wm */
 emit_w(cg, 0x1AC02800u | (A64_REG(wm) << 16) | (A64_REG(wn) << 5) | A64_REG(wd));
}
void a64_lsl_x_imm(a64_codegen* cg, unsigned xd, unsigned xn, unsigned s)
{
 uint32_t immr = (64u - s) & 0x3Fu;
 uint32_t imms = 63u - s;
 emit_w(cg, 0xD3400000u | (immr << 16) | (imms << 10)
            | (A64_REG(xn) << 5) | A64_REG(xd));
}
void a64_lsr_x_imm(a64_codegen* cg, unsigned xd, unsigned xn, unsigned s)
{
 emit_w(cg, 0xD3400000u | ((s & 0x3Fu) << 16) | (63u << 10)
            | (A64_REG(xn) << 5) | A64_REG(xd));
}
void a64_asr_x_imm(a64_codegen* cg, unsigned xd, unsigned xn, unsigned s)
{
 emit_w(cg, 0x9340FC00u | ((s & 0x3Fu) << 16)
            | (A64_REG(xn) << 5) | A64_REG(xd));
}

/* --- Bitfield ------------------------------------------------------ */

void a64_ubfx_w(a64_codegen* cg, unsigned wd, unsigned wn, unsigned lsb, unsigned width)
{
 emit_w(cg, 0x53000000u | ((lsb & 0x1Fu) << 16) | (((lsb + width - 1u) & 0x1Fu) << 10)
            | (A64_REG(wn) << 5) | A64_REG(wd));
}
void a64_sbfx_w(a64_codegen* cg, unsigned wd, unsigned wn, unsigned lsb, unsigned width)
{
 emit_w(cg, 0x13000000u | ((lsb & 0x1Fu) << 16) | (((lsb + width - 1u) & 0x1Fu) << 10)
            | (A64_REG(wn) << 5) | A64_REG(wd));
}
void a64_bfi_w(a64_codegen* cg, unsigned wd, unsigned wn, unsigned lsb, unsigned width)
{
 uint32_t immr = (32u - lsb) & 0x1Fu;
 uint32_t imms = (width - 1u) & 0x1Fu;
 emit_w(cg, 0x33000000u | (immr << 16) | (imms << 10)
            | (A64_REG(wn) << 5) | A64_REG(wd));
}
void a64_bfi_x(a64_codegen* cg, unsigned xd, unsigned xn, unsigned lsb, unsigned width)
{
 uint32_t immr = (64u - lsb) & 0x3Fu;
 uint32_t imms = (width - 1u) & 0x3Fu;
 emit_w(cg, 0xB3400000u | (immr << 16) | (imms << 10)
            | (A64_REG(xn) << 5) | A64_REG(xd));
}

void a64_sxtw(a64_codegen* cg, unsigned xd, unsigned wn)
{ /* SBFM Xd, Xn, 0, 31  -> SXTW alias */
 emit_w(cg, 0x93407C00u | (A64_REG(wn) << 5) | A64_REG(xd));
}

void a64_clz_w(a64_codegen* cg, unsigned wd, unsigned wn)
{
 emit_w(cg, 0x5AC01000u | (A64_REG(wn) << 5) | A64_REG(wd));
}

/* --- Multiply / negate -------------------------------------------- */

void a64_smull(a64_codegen* cg, unsigned xd, unsigned wn, unsigned wm)
{
 /* SMADDL Xd, Wn, Wm, XZR */
 emit_w(cg, 0x9B207C00u | (A64_REG(wm) << 16) | (A64_REG(wn) << 5) | A64_REG(xd));
}

void a64_neg_w(a64_codegen* cg, unsigned wd, unsigned wm)
{
 emit_w(cg, 0x4B0003E0u | (A64_REG(wm) << 16) | A64_REG(wd));
}

/* --- Branches ----------------------------------------------------- */

/*
 * Emit a forward / backward branch whose offset is either patched on
 * bind (forward) or computed inline (backward).
 *
 * `inst_base` is the instruction with imm-field cleared.
 */
static void emit_branch(a64_codegen* cg, a64_label* lbl,
                        uint32_t inst_base, unsigned kind)
{
 if(lbl->bound) {
  ptrdiff_t here = (char*)cg->wp - (char*)cg->base;
  int64_t   delta = (int64_t)((lbl->target_off - here) >> 2);
  uint32_t  imm;
  switch(kind) {
  case A64_PATCH_B:
   assert(sint_fits(delta, 26));
   imm = (uint32_t)((uint64_t)delta & 0x3FFFFFFu);
   break;
  case A64_PATCH_BCOND:
   assert(sint_fits(delta, 19));
   imm = (uint32_t)(((uint64_t)delta & 0x7FFFFu) << 5);
   break;
  case A64_PATCH_TBNZ:
   assert(sint_fits(delta, 14));
   imm = (uint32_t)(((uint64_t)delta & 0x3FFFu) << 5);
   break;
  default:
   imm = 0;
   assert(0);
  }
  emit_w(cg, inst_base | imm);
 } else {
  ptrdiff_t wb = (char*)cg->wp - (char*)cg->base;
  emit_w(cg, inst_base);
  label_add_patch(lbl, wb, kind);
 }
}

void a64_b(a64_codegen* cg, a64_label* lbl)
{ emit_branch(cg, lbl, 0x14000000u, A64_PATCH_B); }

void a64_b_cond(a64_codegen* cg, unsigned cond, a64_label* lbl)
{ emit_branch(cg, lbl, 0x54000000u | (cond & 0xFu), A64_PATCH_BCOND); }

void a64_cbz_w(a64_codegen* cg, unsigned wn, a64_label* lbl)
{ emit_branch(cg, lbl, 0x34000000u | A64_REG(wn), A64_PATCH_BCOND); }

void a64_cbnz_w(a64_codegen* cg, unsigned wn, a64_label* lbl)
{ emit_branch(cg, lbl, 0x35000000u | A64_REG(wn), A64_PATCH_BCOND); }

void a64_tbnz_w(a64_codegen* cg, unsigned wn, unsigned bit, a64_label* lbl)
{ emit_branch(cg, lbl, 0x37000000u | ((bit & 0x1Fu) << 19) | A64_REG(wn),
              A64_PATCH_TBNZ); }

void a64_b_addr(a64_codegen* cg, const void* addr)
{
 int64_t delta = (int64_t)(((intptr_t)addr - (intptr_t)cg->wp) >> 2);
 assert(sint_fits(delta, 26));
 emit_w(cg, 0x14000000u | ((uint32_t)((uint64_t)delta & 0x3FFFFFFu)));
}

void a64_br (a64_codegen* cg, unsigned xn) { emit_w(cg, 0xD61F0000u | (A64_REG(xn) << 5)); }
void a64_blr(a64_codegen* cg, unsigned xn) { emit_w(cg, 0xD63F0000u | (A64_REG(xn) << 5)); }
void a64_ret(a64_codegen* cg)              { emit_w(cg, 0xD65F0000u | (30u << 5)); }

/* --- Loads / stores (imm offset) ---------------------------------- */

static void emit_ldst_imm12(a64_codegen* cg, uint32_t base, unsigned size_log2,
                            unsigned rt, unsigned rn, uint32_t off)
{
 uint32_t scaled = off >> size_log2;
 assert((off & ((1u << size_log2) - 1u)) == 0u);
 assert(scaled <= 0xFFFu);
 emit_w(cg, base | (scaled << 10) | (A64_REG(rn) << 5) | A64_REG(rt));
}

void a64_ldr_w_imm  (a64_codegen* cg, unsigned wt, unsigned xn, uint32_t off)
{ emit_ldst_imm12(cg, 0xB9400000u, 2u, wt, xn, off); }
void a64_str_w_imm  (a64_codegen* cg, unsigned wt, unsigned xn, uint32_t off)
{ emit_ldst_imm12(cg, 0xB9000000u, 2u, wt, xn, off); }
void a64_ldr_x_imm  (a64_codegen* cg, unsigned xt, unsigned xn, uint32_t off)
{ emit_ldst_imm12(cg, 0xF9400000u, 3u, xt, xn, off); }
void a64_str_x_imm  (a64_codegen* cg, unsigned xt, unsigned xn, uint32_t off)
{ emit_ldst_imm12(cg, 0xF9000000u, 3u, xt, xn, off); }
void a64_ldrsw_x_imm(a64_codegen* cg, unsigned xt, unsigned xn, uint32_t off)
{ emit_ldst_imm12(cg, 0xB9800000u, 2u, xt, xn, off); }
void a64_ldrh_w_imm (a64_codegen* cg, unsigned wt, unsigned xn, uint32_t off)
{ emit_ldst_imm12(cg, 0x79400000u, 1u, wt, xn, off); }
void a64_strh_w_imm (a64_codegen* cg, unsigned wt, unsigned xn, uint32_t off)
{ emit_ldst_imm12(cg, 0x79000000u, 1u, wt, xn, off); }
void a64_ldrb_w_imm (a64_codegen* cg, unsigned wt, unsigned xn, uint32_t off)
{ emit_ldst_imm12(cg, 0x39400000u, 0u, wt, xn, off); }
void a64_strb_w_imm (a64_codegen* cg, unsigned wt, unsigned xn, uint32_t off)
{ emit_ldst_imm12(cg, 0x39000000u, 0u, wt, xn, off); }

static void emit_ldst_unscaled(a64_codegen* cg, uint32_t base,
                               unsigned rt, unsigned rn, int off)
{
 uint32_t imm9;
 assert(off >= -256 && off <= 255);
 imm9 = (uint32_t)((uint32_t)off & 0x1FFu);
 emit_w(cg, base | (imm9 << 12) | (A64_REG(rn) << 5) | A64_REG(rt));
}

void a64_ldur_w(a64_codegen* cg, unsigned wt, unsigned xn, int off)
{ emit_ldst_unscaled(cg, 0xB8400000u, wt, xn, off); }
void a64_stur_w(a64_codegen* cg, unsigned wt, unsigned xn, int off)
{ emit_ldst_unscaled(cg, 0xB8000000u, wt, xn, off); }

/* --- Loads / stores (register index) ------------------------------ */

static void emit_ldst_reg(a64_codegen* cg, uint32_t base, unsigned size_log2,
                          unsigned rt, unsigned rn, unsigned rm,
                          unsigned option, unsigned shift)
{
 unsigned S;
 if(shift == 0u)              S = 0u;
 else { assert(shift == size_log2); S = 1u; }
 emit_w(cg, base | (A64_REG(rm) << 16) | (option << 13) | (S << 12)
            | (A64_REG(rn) << 5) | A64_REG(rt));
}

/* LSL = option 011, UXTW = option 010 */
void a64_ldr_w_reg(a64_codegen* cg, unsigned wt, unsigned xn, unsigned xm)
{ emit_ldst_reg(cg, 0xB8600800u, 2u, wt, xn, xm, 3u, 0u); }
void a64_str_w_reg(a64_codegen* cg, unsigned wt, unsigned xn, unsigned xm)
{ emit_ldst_reg(cg, 0xB8200800u, 2u, wt, xn, xm, 3u, 0u); }
void a64_ldrh_w_reg(a64_codegen* cg, unsigned wt, unsigned xn, unsigned xm)
{ emit_ldst_reg(cg, 0x78600800u, 1u, wt, xn, xm, 3u, 0u); }
void a64_strh_w_reg(a64_codegen* cg, unsigned wt, unsigned xn, unsigned xm)
{ emit_ldst_reg(cg, 0x78200800u, 1u, wt, xn, xm, 3u, 0u); }
void a64_ldrb_w_reg(a64_codegen* cg, unsigned wt, unsigned xn, unsigned xm)
{ emit_ldst_reg(cg, 0x38600800u, 0u, wt, xn, xm, 3u, 0u); }
void a64_strb_w_reg(a64_codegen* cg, unsigned wt, unsigned xn, unsigned xm)
{ emit_ldst_reg(cg, 0x38200800u, 0u, wt, xn, xm, 3u, 0u); }
void a64_ldr_w_idx_lsl(a64_codegen* cg, unsigned wt, unsigned xn, unsigned xm, unsigned shift)
{ emit_ldst_reg(cg, 0xB8600800u, 2u, wt, xn, xm, 3u, shift); }
void a64_str_w_idx_lsl(a64_codegen* cg, unsigned wt, unsigned xn, unsigned xm, unsigned shift)
{ emit_ldst_reg(cg, 0xB8200800u, 2u, wt, xn, xm, 3u, shift); }
void a64_ldr_x_idx_lsl(a64_codegen* cg, unsigned xt, unsigned xn, unsigned xm, unsigned shift)
{ emit_ldst_reg(cg, 0xF8600800u, 3u, xt, xn, xm, 3u, shift); }
void a64_ldr_w_uxtw(a64_codegen* cg, unsigned wt, unsigned xn, unsigned wm, unsigned shift)
{ emit_ldst_reg(cg, 0xB8600800u, 2u, wt, xn, wm, 2u, shift); }
void a64_str_w_uxtw(a64_codegen* cg, unsigned wt, unsigned xn, unsigned wm, unsigned shift)
{ emit_ldst_reg(cg, 0xB8200800u, 2u, wt, xn, wm, 2u, shift); }
void a64_ldrh_w_uxtw(a64_codegen* cg, unsigned wt, unsigned xn, unsigned wm, unsigned shift)
{ emit_ldst_reg(cg, 0x78600800u, 1u, wt, xn, wm, 2u, shift); }
void a64_strh_w_uxtw(a64_codegen* cg, unsigned wt, unsigned xn, unsigned wm, unsigned shift)
{ emit_ldst_reg(cg, 0x78200800u, 1u, wt, xn, wm, 2u, shift); }

/* --- Pair load/store (X regs) ------------------------------------- */

static void emit_pair(a64_codegen* cg, uint32_t base,
                      unsigned rt, unsigned rt2, unsigned rn, int off)
{
 uint32_t imm7;
 assert((off & 0x7) == 0);
 assert(off >= -512 && off <= 504);
 imm7 = (uint32_t)(((int32_t)off >> 3) & 0x7Fu);
 emit_w(cg, base | (imm7 << 15) | (A64_REG(rt2) << 10)
            | (A64_REG(rn) << 5) | A64_REG(rt));
}

void a64_stp_x_pre(a64_codegen* cg, unsigned xt1, unsigned xt2, int off)
{ emit_pair(cg, 0xA9800000u, xt1, xt2, 31u, off); }    /* pre,  L=0 */
void a64_ldp_x_post(a64_codegen* cg, unsigned xt1, unsigned xt2, int off)
{ emit_pair(cg, 0xA8C00000u, xt1, xt2, 31u, off); }    /* post, L=1 */
void a64_stp_x_off(a64_codegen* cg, unsigned xt1, unsigned xt2, unsigned xn, int off)
{ emit_pair(cg, 0xA9000000u, xt1, xt2, xn, off); }     /* signed off, L=0 */
void a64_ldp_x_off(a64_codegen* cg, unsigned xt1, unsigned xt2, unsigned xn, int off)
{ emit_pair(cg, 0xA9400000u, xt1, xt2, xn, off); }     /* signed off, L=1 */

void a64_nop(a64_codegen* cg) { emit_w(cg, 0xD503201Fu); }

/* ====================================================================
 * Section 6 -- constant pool (deduplicated 64-bit values).
 *
 * Queue an LDR (literal) at emit time with imm19 left zero, remember
 * (site, entry) in pool_refs[].  On flush, align to 8 bytes, lay down
 * each unique value, then patch the refs.
 * ==================================================================== */

static unsigned pool_intern(a64_codegen* cg, uint64_t v)
{
 unsigned i;
 for(i = 0; i < cg->pool_count; ++i)
  if(cg->pool_values[i] == v) return i;
 assert(cg->pool_count < A64_POOL_MAX_ENTRIES);
 cg->pool_values[cg->pool_count] = v;
 return cg->pool_count++;
}

void a64_ldr_x_pool(a64_codegen* cg, unsigned xd, uint64_t value)
{
 unsigned idx;
 ptrdiff_t wb;

 if(A64_REG(xd) == 31u) return;
 idx = pool_intern(cg, value);
 wb  = (char*)cg->wp - (char*)cg->base;

 assert(cg->pool_ref_count < A64_POOL_MAX_REFS);
 cg->pool_refs[cg->pool_ref_count].wb_off = wb;
 cg->pool_refs[cg->pool_ref_count].entry  = idx;
 cg->pool_ref_count++;

 /* LDR Xt, label : imm19 left zero, patched at flush. */
 emit_w(cg, 0x58000000u | A64_REG(xd));
}

void a64_movp2r_pool(a64_codegen* cg, unsigned xd, const void* ptr)
{
 a64_ldr_x_pool(cg, xd, (uint64_t)(uintptr_t)ptr);
}

unsigned a64_pool_pending(const a64_codegen* cg)
{
 return cg ? cg->pool_ref_count : 0u;
}

void a64_pool_reset(a64_codegen* cg)
{
 if(!cg) return;
 cg->pool_count     = 0u;
 cg->pool_ref_count = 0u;
}

void a64_pool_flush(a64_codegen* cg)
{
 ptrdiff_t pool_off;
 unsigned  i;

 if(!cg || cg->pool_ref_count == 0u) {
  if(cg) cg->pool_count = 0u;
  return;
 }

 /* Align pool start to 8 bytes: misaligned is permitted with SCTLR.A=0
  * but costs a split access. */
 if(((uintptr_t)cg->wp & 0x7u) != 0u)
  a64_nop(cg);

 pool_off = (char*)cg->wp - (char*)cg->base;

 /* Two 32-bit words each, little-endian, through emit_w so the
  * bump-allocator invariant (one emit_w per 4 bytes) holds. */
 for(i = 0; i < cg->pool_count; ++i) {
  uint64_t v = cg->pool_values[i];
  emit_w(cg, (uint32_t)(v & 0xFFFFFFFFu));
  emit_w(cg, (uint32_t)(v >> 32));
 }

 /* Patch each LDR-literal site with the imm19 byte-offset/4 to its
  * pool slot. */
 for(i = 0; i < cg->pool_ref_count; ++i) {
  ptrdiff_t site_off = cg->pool_refs[i].wb_off;
  unsigned  entry    = cg->pool_refs[i].entry;
  ptrdiff_t slot_off = pool_off + (ptrdiff_t)entry * 8;
  int64_t   delta    = (int64_t)((slot_off - site_off) >> 2);
  uint32_t* p        = (uint32_t*)((char*)cg->base + site_off);
  assert(sint_fits(delta, 19));
  *p |= (uint32_t)(((uint64_t)delta & 0x7FFFFu) << 5);
 }

 cg->pool_count     = 0u;
 cg->pool_ref_count = 0u;
}

/* ====================================================================
 * Section 7 -- in-place branch patching.  The site is the byte address
 * of the 32-bit instruction word; the imm field is cleared and
 * rewritten.  Out-of-range returns 0.
 * ==================================================================== */

static int patch_imm(void* site, const void* target,
                     unsigned bits, uint32_t field_mask, unsigned shift)
{
 uint32_t* p = (uint32_t*)site;
 int64_t   delta = (int64_t)(((intptr_t)target - (intptr_t)site) >> 2);
 uint64_t  field;
 if(!sint_fits(delta, bits)) return 0;
 field = ((uint64_t)delta & (((uint64_t)1u << bits) - 1u)) << shift;
 *p = (*p & ~field_mask) | (uint32_t)field;
 return 1;
}

int a64_patch_b(void* site, const void* target)
{ return patch_imm(site, target, 26u, 0x03FFFFFFu, 0u); }

int a64_patch_b_cond(void* site, const void* target)
{ return patch_imm(site, target, 19u, 0x00FFFFE0u, 5u); }

int a64_patch_cbz(void* site, const void* target)
{ return patch_imm(site, target, 19u, 0x00FFFFE0u, 5u); }

int a64_patch_tbz(void* site, const void* target)
{ return patch_imm(site, target, 14u, 0x0007FFE0u, 5u); }

#endif /* AArch64 + Linux */
