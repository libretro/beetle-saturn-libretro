/******************************************************************************/
/* Mednafen Sega Saturn Emulation Module                                      */
/******************************************************************************/
/* x86emit.c - minimal x86 / x86-64 machine-code emitter for the DSP JITs
**
** See x86emit.h for the model.  Only the encodings the SCSP MPROG body
** needs are implemented; each one is the canonical form from the SDM,
** with the REX prefix derived from the operands so that x86-32 callers
** (who can only name registers 0..7) never see one emitted.
*/

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "x86emit.h"

#if X86EMIT_HOST

#if defined(_WIN32)
 #include <windows.h>
#else
 #include <sys/mman.h>
 #ifndef MAP_ANON
  #define MAP_ANON MAP_ANONYMOUS
 #endif
#endif

struct x86_codegen
{
 uint8_t* base;
 uint8_t* wp;
 size_t   size;
 int      overflow;
};

/* --- segment ------------------------------------------------------ */

x86_codegen* x86_codegen_create(size_t bytes)
{
 x86_codegen* cg = (x86_codegen*)calloc(1, sizeof *cg);
 void* mem;
 if(!cg) return NULL;
#if defined(_WIN32)
 mem = VirtualAlloc(NULL, bytes, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
 if(!mem) { free(cg); return NULL; }
#else
 mem = mmap(NULL, bytes, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANON | MAP_PRIVATE, -1, 0);
 if(mem == MAP_FAILED) { free(cg); return NULL; }
#endif
 cg->base = (uint8_t*)mem;
 cg->wp   = (uint8_t*)mem;
 cg->size = bytes;
 return cg;
}

void x86_codegen_destroy(x86_codegen* cg)
{
 if(!cg) return;
#if defined(_WIN32)
 VirtualFree(cg->base, 0, MEM_RELEASE);
#else
 munmap(cg->base, cg->size);
#endif
 free(cg);
}

void*  x86_codegen_base    (const x86_codegen* cg) { return cg ? cg->base : NULL; }
void*  x86_codegen_wptr    (const x86_codegen* cg) { return cg ? cg->wp : NULL; }
size_t x86_codegen_offset  (const x86_codegen* cg) { return cg ? (size_t)(cg->wp - cg->base) : 0; }
size_t x86_codegen_capacity(const x86_codegen* cg) { return cg ? cg->size : 0; }
int    x86_codegen_overflowed(const x86_codegen* cg) { return cg ? cg->overflow : 1; }

void x86_codegen_set_wptr(x86_codegen* cg, void* p)
{
 if(!cg) return;
 cg->wp = (uint8_t*)p;
 cg->overflow = 0;
}

/* --- raw emission -------------------------------------------------- */

static void emit_b(x86_codegen* cg, uint8_t b)
{
 if(cg->wp >= cg->base + cg->size)
 {
  cg->overflow = 1;
  return;
 }
 *cg->wp++ = b;
}

static void emit_d(x86_codegen* cg, uint32_t d)
{
 emit_b(cg, (uint8_t)d);
 emit_b(cg, (uint8_t)(d >> 8));
 emit_b(cg, (uint8_t)(d >> 16));
 emit_b(cg, (uint8_t)(d >> 24));
}

static int fits_s8(int32_t v) { return v >= -128 && v <= 127; }

/*
 * REX: 0100WRXB.  `reg` feeds R, `index` feeds X, `base`/`rm` feeds B.
 * On x86-32 every register is < 8 and w is never set, so nothing is
 * emitted -- 0x40 there would decode as INC EAX.
 */
static void emit_rex(x86_codegen* cg, int w, unsigned reg, int index, unsigned rm)
{
 unsigned rex = 0x40u | ((unsigned)(w != 0) << 3) | ((reg >> 3) << 2)
              | (((index < 0 ? 0u : (unsigned)index) >> 3) << 1) | (rm >> 3);
#if !X86EMIT_64
 assert(reg < 8 && rm < 8 && index < 8 && !w);
 (void)cg; (void)rex;
 return;
#else
 if(rex != 0x40u)
  emit_b(cg, (uint8_t)rex);
#endif
}

/* ModRM (+SIB, +disp) for a memory operand.  `reg` is the /r field. */
static void emit_mem(x86_codegen* cg, unsigned reg, unsigned base, int index, unsigned scale, int32_t disp)
{
 const unsigned rm = base & 7u;
 const int need_sib = (index >= 0) || (rm == 4u);
 unsigned mod;

 if(disp == 0 && rm != 5u)      mod = 0u;
 else if(fits_s8(disp))         mod = 1u;
 else                           mod = 2u;

 emit_b(cg, (uint8_t)((mod << 6) | ((reg & 7u) << 3) | (need_sib ? 4u : rm)));
 if(need_sib)
  emit_b(cg, (uint8_t)((scale << 6) | (((index < 0) ? 4u : ((unsigned)index & 7u)) << 3) | rm));
 if(mod == 1u)      emit_b(cg, (uint8_t)disp);
 else if(mod == 2u) emit_d(cg, (uint32_t)disp);
}

static void emit_modrm_rr(x86_codegen* cg, unsigned reg, unsigned rm)
{
 emit_b(cg, (uint8_t)(0xC0u | ((reg & 7u) << 3) | (rm & 7u)));
}

/* --- labels ---------------------------------------------------------- */

void x86_label_init(x86_label* l)
{
 memset(l, 0, sizeof *l);
}

static void emit_rel32_to(x86_codegen* cg, x86_label* l)
{
 if(l->bound)
 {
  const intptr_t delta = (intptr_t)(l->bound - (cg->wp + 4));
  emit_d(cg, (uint32_t)delta);
 }
 else
 {
  assert(l->patch_count < X86_LABEL_MAX_PATCHES);
  if(l->patch_count < X86_LABEL_MAX_PATCHES)
   l->patches[l->patch_count++] = cg->wp;
  emit_d(cg, 0);
 }
}

void x86_label_bind(x86_codegen* cg, x86_label* l)
{
 unsigned i;
 assert(!l->bound);
 l->bound = cg->wp;
 for(i = 0; i < l->patch_count; i++)
 {
  uint8_t* site = l->patches[i];
  const int32_t delta = (int32_t)(l->bound - (site + 4));
  site[0] = (uint8_t)delta;
  site[1] = (uint8_t)(delta >> 8);
  site[2] = (uint8_t)(delta >> 16);
  site[3] = (uint8_t)(delta >> 24);
 }
}

/* --- moves ---------------------------------------------------------- */

void x86_mov_rr(x86_codegen* cg, unsigned dst, unsigned src)
{
 emit_rex(cg, 0, src, -1, dst);
 emit_b(cg, 0x89);
 emit_modrm_rr(cg, src, dst);
}

void x86_mov_rr64(x86_codegen* cg, unsigned dst, unsigned src)
{
 emit_rex(cg, 1, src, -1, dst);
 emit_b(cg, 0x89);
 emit_modrm_rr(cg, src, dst);
}

void x86_mov_ri(x86_codegen* cg, unsigned dst, uint32_t imm)
{
 emit_rex(cg, 0, 0, -1, dst);
 emit_b(cg, (uint8_t)(0xB8u | (dst & 7u)));
 emit_d(cg, imm);
}

void x86_mov_rm(x86_codegen* cg, unsigned dst, unsigned base, int index, unsigned scale, int32_t disp)
{
 emit_rex(cg, 0, dst, index, base);
 emit_b(cg, 0x8B);
 emit_mem(cg, dst, base, index, scale, disp);
}

void x86_mov_rm64(x86_codegen* cg, unsigned dst, unsigned base, int index, unsigned scale, int32_t disp)
{
 emit_rex(cg, 1, dst, index, base);
 emit_b(cg, 0x8B);
 emit_mem(cg, dst, base, index, scale, disp);
}

void x86_mov_mr(x86_codegen* cg, unsigned base, int index, unsigned scale, int32_t disp, unsigned src)
{
 emit_rex(cg, 0, src, index, base);
 emit_b(cg, 0x89);
 emit_mem(cg, src, base, index, scale, disp);
}

void x86_mov_mr16(x86_codegen* cg, unsigned base, int index, unsigned scale, int32_t disp, unsigned src)
{
 emit_b(cg, 0x66);
 emit_rex(cg, 0, src, index, base);
 emit_b(cg, 0x89);
 emit_mem(cg, src, base, index, scale, disp);
}

void x86_mov_mi8(x86_codegen* cg, unsigned base, int index, unsigned scale, int32_t disp, uint8_t imm)
{
 emit_rex(cg, 0, 0, index, base);
 emit_b(cg, 0xC6);
 emit_mem(cg, 0, base, index, scale, disp);
 emit_b(cg, imm);
}

void x86_mov_mi32(x86_codegen* cg, unsigned base, int index, unsigned scale, int32_t disp, uint32_t imm)
{
 emit_rex(cg, 0, 0, index, base);
 emit_b(cg, 0xC7);
 emit_mem(cg, 0, base, index, scale, disp);
 emit_d(cg, imm);
}

void x86_movzx_rm16(x86_codegen* cg, unsigned dst, unsigned base, int index, unsigned scale, int32_t disp)
{
 emit_rex(cg, 0, dst, index, base);
 emit_b(cg, 0x0F); emit_b(cg, 0xB7);
 emit_mem(cg, dst, base, index, scale, disp);
}

void x86_movsx_rm16(x86_codegen* cg, unsigned dst, unsigned base, int index, unsigned scale, int32_t disp)
{
 emit_rex(cg, 0, dst, index, base);
 emit_b(cg, 0x0F); emit_b(cg, 0xBF);
 emit_mem(cg, dst, base, index, scale, disp);
}

void x86_movzx_rm8(x86_codegen* cg, unsigned dst, unsigned base, int index, unsigned scale, int32_t disp)
{
 emit_rex(cg, 0, dst, index, base);
 emit_b(cg, 0x0F); emit_b(cg, 0xB6);
 emit_mem(cg, dst, base, index, scale, disp);
}

void x86_movsx_rr16(x86_codegen* cg, unsigned dst, unsigned src)
{
 emit_rex(cg, 0, dst, -1, src);
 emit_b(cg, 0x0F); emit_b(cg, 0xBF);
 emit_modrm_rr(cg, dst, src);
}

void x86_lea(x86_codegen* cg, unsigned dst, unsigned base, int index, unsigned scale, int32_t disp)
{
 emit_rex(cg, 0, dst, index, base);
 emit_b(cg, 0x8D);
 emit_mem(cg, dst, base, index, scale, disp);
}

/* --- ALU -------------------------------------------------------------- */

void x86_alu_rr(x86_codegen* cg, unsigned op, unsigned dst, unsigned src)
{
 emit_rex(cg, 0, src, -1, dst);
 emit_b(cg, (uint8_t)((op << 3) | 0x01u));
 emit_modrm_rr(cg, src, dst);
}

static void alu_ri_w(x86_codegen* cg, unsigned op, unsigned dst, int32_t imm, int w)
{
 emit_rex(cg, w, 0, -1, dst);
 if(fits_s8(imm))
 {
  emit_b(cg, 0x83);
  emit_modrm_rr(cg, op, dst);
  emit_b(cg, (uint8_t)imm);
 }
 else
 {
  emit_b(cg, 0x81);
  emit_modrm_rr(cg, op, dst);
  emit_d(cg, (uint32_t)imm);
 }
}

void x86_alu_ri  (x86_codegen* cg, unsigned op, unsigned dst, int32_t imm) { alu_ri_w(cg, op, dst, imm, 0); }
void x86_alu_ri64(x86_codegen* cg, unsigned op, unsigned dst, int32_t imm) { alu_ri_w(cg, op, dst, imm, 1); }

void x86_alu_rm(x86_codegen* cg, unsigned op, unsigned dst, unsigned base, int index, unsigned scale, int32_t disp)
{
 emit_rex(cg, 0, dst, index, base);
 emit_b(cg, (uint8_t)((op << 3) | 0x03u));
 emit_mem(cg, dst, base, index, scale, disp);
}

void x86_cmp_mi8(x86_codegen* cg, unsigned base, int index, unsigned scale, int32_t disp, uint8_t imm)
{
 emit_rex(cg, 0, 0, index, base);
 emit_b(cg, 0x80);
 emit_mem(cg, X86_CMP, base, index, scale, disp);
 emit_b(cg, imm);
}

void x86_shift_ri(x86_codegen* cg, unsigned kind, unsigned r, unsigned imm)
{
 if(!imm) return;
 emit_rex(cg, 0, 0, -1, r);
 if(imm == 1)
 {
  emit_b(cg, 0xD1);
  emit_modrm_rr(cg, kind, r);
 }
 else
 {
  emit_b(cg, 0xC1);
  emit_modrm_rr(cg, kind, r);
  emit_b(cg, (uint8_t)imm);
 }
}

void x86_sar_cl(x86_codegen* cg, unsigned r)
{
 emit_rex(cg, 0, 0, -1, r);
 emit_b(cg, 0xD3);
 emit_modrm_rr(cg, X86_SAR, r);
}

void x86_shrd_rri(x86_codegen* cg, unsigned dst, unsigned src, unsigned imm)
{
 emit_rex(cg, 0, src, -1, dst);
 emit_b(cg, 0x0F); emit_b(cg, 0xAC);
 emit_modrm_rr(cg, src, dst);
 emit_b(cg, (uint8_t)imm);
}

void x86_imul_r(x86_codegen* cg, unsigned r)
{
 emit_rex(cg, 0, 0, -1, r);
 emit_b(cg, 0xF7);
 emit_modrm_rr(cg, 5, r);
}

void x86_neg(x86_codegen* cg, unsigned r)
{
 emit_rex(cg, 0, 0, -1, r);
 emit_b(cg, 0xF7);
 emit_modrm_rr(cg, 3, r);
}

void x86_test_rr(x86_codegen* cg, unsigned a, unsigned b)
{
 emit_rex(cg, 0, b, -1, a);
 emit_b(cg, 0x85);
 emit_modrm_rr(cg, b, a);
}

void x86_test_ri(x86_codegen* cg, unsigned r, uint32_t imm)
{
 emit_rex(cg, 0, 0, -1, r);
 emit_b(cg, 0xF7);
 emit_modrm_rr(cg, 0, r);
 emit_d(cg, imm);
}

void x86_cmov(x86_codegen* cg, unsigned cc, unsigned dst, unsigned src)
{
 emit_rex(cg, 0, dst, -1, src);
 emit_b(cg, 0x0F); emit_b(cg, (uint8_t)(0x40u | cc));
 emit_modrm_rr(cg, dst, src);
}

void x86_bsr(x86_codegen* cg, unsigned dst, unsigned src)
{
 emit_rex(cg, 0, dst, -1, src);
 emit_b(cg, 0x0F); emit_b(cg, 0xBD);
 emit_modrm_rr(cg, dst, src);
}

/* --- control ---------------------------------------------------------- */

void x86_jcc(x86_codegen* cg, unsigned cc, x86_label* l)
{
 emit_b(cg, 0x0F); emit_b(cg, (uint8_t)(0x80u | cc));
 emit_rel32_to(cg, l);
}

void x86_jmp(x86_codegen* cg, x86_label* l)
{
 emit_b(cg, 0xE9);
 emit_rel32_to(cg, l);
}

void x86_push(x86_codegen* cg, unsigned r)
{
 emit_rex(cg, 0, 0, -1, r);
 emit_b(cg, (uint8_t)(0x50u | (r & 7u)));
}

void x86_pop(x86_codegen* cg, unsigned r)
{
 emit_rex(cg, 0, 0, -1, r);
 emit_b(cg, (uint8_t)(0x58u | (r & 7u)));
}

void x86_ret(x86_codegen* cg)
{
 emit_b(cg, 0xC3);
}

#else /* !X86EMIT_HOST: keep the TU non-empty for pedantic toolchains */

typedef int x86emit_not_host;

#endif
