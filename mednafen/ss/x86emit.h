/******************************************************************************/
/* Mednafen Sega Saturn Emulation Module                                      */
/******************************************************************************/
/* x86emit.h - minimal x86 / x86-64 machine-code emitter for the DSP JITs
**
** One emitter serves both widths.  Register numbers are 0..7 on x86-32
** and 0..15 on x86-64 (EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI, R8..R15);
** every data operation is 32-bit, and the only 64-bit forms are the few
** pointer moves the prologue needs.  A REX prefix is emitted exactly when
** an operand requires it, so the same call sequence encodes identically
** on both targets whenever it stays within the low eight registers.
**
** Memory operands are always [base + index*scale + disp32]; pass
** X86_NOIDX for no index.  The encoder handles the RSP/R12 (SIB) and
** RBP/R13 (mandatory displacement) cases, so callers never need to.
**
** Every emit goes through emit_b(); running past the segment end drops
** the byte and latches a sticky overflow flag that callers must check
** before publishing code (x86_codegen_overflowed).
*/

#ifndef __MDFN_SS_X86EMIT_H
#define __MDFN_SS_X86EMIT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(__x86_64__) || defined(_M_X64)
 #define X86EMIT_64 1
 #define X86EMIT_HOST 1
#elif defined(__i386__) || defined(_M_IX86)
 #define X86EMIT_64 0
 #define X86EMIT_HOST 1
#else
 #define X86EMIT_64 0
 #define X86EMIT_HOST 0
#endif

enum
{
 X86_EAX = 0, X86_ECX = 1, X86_EDX = 2, X86_EBX = 3,
 X86_ESP = 4, X86_EBP = 5, X86_ESI = 6, X86_EDI = 7,
 X86_R8 = 8, X86_R9 = 9, X86_R10 = 10, X86_R11 = 11,
 X86_R12 = 12, X86_R13 = 13, X86_R14 = 14, X86_R15 = 15,
 X86_NOIDX = -1
};

/* ALU opcode group (the /r field of 0x81/0x83 and the base of 0x01..0x3B). */
enum { X86_ADD = 0, X86_OR = 1, X86_ADC = 2, X86_SBB = 3, X86_AND = 4, X86_SUB = 5, X86_XOR = 6, X86_CMP = 7 };
/* Shift group (the /r field of 0xC1/0xD3). */
enum { X86_SHL = 4, X86_SHR = 5, X86_SAR = 7 };
/* Condition codes (Jcc / CMOVcc / SETcc low nibble). */
enum
{
 X86_CC_O = 0x0, X86_CC_NO = 0x1, X86_CC_B = 0x2, X86_CC_AE = 0x3,
 X86_CC_E = 0x4, X86_CC_NE = 0x5, X86_CC_BE = 0x6, X86_CC_A = 0x7,
 X86_CC_S = 0x8, X86_CC_NS = 0x9, X86_CC_P = 0xA, X86_CC_NP = 0xB,
 X86_CC_L = 0xC, X86_CC_GE = 0xD, X86_CC_LE = 0xE, X86_CC_G = 0xF
};

typedef struct x86_codegen x86_codegen;

#define X86_LABEL_MAX_PATCHES 128
typedef struct
{
 uint8_t* bound;                          /* NULL until x86_label_bind */
 uint8_t* patches[X86_LABEL_MAX_PATCHES]; /* rel32 slots awaiting the target */
 unsigned patch_count;
} x86_label;

/* --- Code-segment lifecycle --------------------------------------- */

/* Allocate `bytes` of RWX memory (VirtualAlloc on Windows, mmap
 * elsewhere).  Returns NULL when the platform refuses. */
x86_codegen* x86_codegen_create(size_t bytes);
void         x86_codegen_destroy(x86_codegen*);

void*  x86_codegen_base    (const x86_codegen*);
void*  x86_codegen_wptr    (const x86_codegen*);
size_t x86_codegen_offset  (const x86_codegen*);
size_t x86_codegen_capacity(const x86_codegen*);
void   x86_codegen_set_wptr(x86_codegen*, void* p);   /* also clears overflow */
int    x86_codegen_overflowed(const x86_codegen*);

/* --- Labels ------------------------------------------------------- */

void x86_label_init(x86_label*);
void x86_label_bind(x86_codegen*, x86_label*);

/* --- Instructions (32-bit operand size unless noted) ---------------- */

void x86_mov_rr   (x86_codegen*, unsigned dst, unsigned src);
void x86_mov_rr64 (x86_codegen*, unsigned dst, unsigned src);
void x86_mov_ri   (x86_codegen*, unsigned dst, uint32_t imm);
void x86_mov_rm   (x86_codegen*, unsigned dst, unsigned base, int index, unsigned scale_log2, int32_t disp);
void x86_mov_rm64 (x86_codegen*, unsigned dst, unsigned base, int index, unsigned scale_log2, int32_t disp);
void x86_mov_mr   (x86_codegen*, unsigned base, int index, unsigned scale_log2, int32_t disp, unsigned src);
void x86_mov_mr16 (x86_codegen*, unsigned base, int index, unsigned scale_log2, int32_t disp, unsigned src);
void x86_mov_mi8  (x86_codegen*, unsigned base, int index, unsigned scale_log2, int32_t disp, uint8_t imm);
void x86_mov_mi32 (x86_codegen*, unsigned base, int index, unsigned scale_log2, int32_t disp, uint32_t imm);
void x86_movzx_rm16(x86_codegen*, unsigned dst, unsigned base, int index, unsigned scale_log2, int32_t disp);
void x86_movsx_rm16(x86_codegen*, unsigned dst, unsigned base, int index, unsigned scale_log2, int32_t disp);
void x86_movzx_rm8 (x86_codegen*, unsigned dst, unsigned base, int index, unsigned scale_log2, int32_t disp);
void x86_movsx_rm8 (x86_codegen*, unsigned dst, unsigned base, int index, unsigned scale_log2, int32_t disp);
void x86_movsx_rr16(x86_codegen*, unsigned dst, unsigned src);
void x86_lea      (x86_codegen*, unsigned dst, unsigned base, int index, unsigned scale_log2, int32_t disp);

void x86_alu_rr   (x86_codegen*, unsigned op, unsigned dst, unsigned src);
void x86_alu_rr16 (x86_codegen*, unsigned op, unsigned dst, unsigned src);   /* 16-bit operand size */
void x86_movzx_rr16(x86_codegen*, unsigned dst, unsigned src);
void x86_alu_ri   (x86_codegen*, unsigned op, unsigned dst, int32_t imm);
void x86_alu_ri64 (x86_codegen*, unsigned op, unsigned dst, int32_t imm);
void x86_alu_rm   (x86_codegen*, unsigned op, unsigned dst, unsigned base, int index, unsigned scale_log2, int32_t disp);
void x86_cmp_mi8  (x86_codegen*, unsigned base, int index, unsigned scale_log2, int32_t disp, uint8_t imm);
void x86_cmp_mi16 (x86_codegen*, unsigned base, int index, unsigned scale_log2, int32_t disp, uint16_t imm);

void x86_shift_ri (x86_codegen*, unsigned kind, unsigned r, unsigned imm);
void x86_sar_cl   (x86_codegen*, unsigned r);
void x86_shrd_rri (x86_codegen*, unsigned dst, unsigned src, unsigned imm);
void x86_imul_r   (x86_codegen*, unsigned r);          /* EDX:EAX = EAX * r */
void x86_neg      (x86_codegen*, unsigned r);
void x86_test_rr  (x86_codegen*, unsigned a, unsigned b);
void x86_test_ri  (x86_codegen*, unsigned r, uint32_t imm);
void x86_cmov     (x86_codegen*, unsigned cc, unsigned dst, unsigned src);
void x86_bsr      (x86_codegen*, unsigned dst, unsigned src);

/* --- additions for the SCU DSP backend ---------------------------------- */

/* 64-bit (REX.W) forms; on x86-32 these assert. */
void x86_mov_mr64 (x86_codegen*, unsigned base, int index, unsigned scale_log2, int32_t disp, unsigned src);
void x86_mov_ri64 (x86_codegen*, unsigned dst, uint64_t imm);
void x86_movabs   (x86_codegen*, unsigned dst, uint64_t imm);   /* always the 10-byte REX.W B8 form */
void x86_alu_rr64 (x86_codegen*, unsigned op, unsigned dst, unsigned src);
void x86_alu_rm64 (x86_codegen*, unsigned op, unsigned dst, unsigned base, int index, unsigned scale_log2, int32_t disp);
void x86_shift_ri64(x86_codegen*, unsigned kind, unsigned r, unsigned imm);
void x86_imul_rr64(x86_codegen*, unsigned dst, unsigned src);
void x86_imul_rr  (x86_codegen*, unsigned dst, unsigned src);                    /* imul r32, r32 */
void x86_imul_ri  (x86_codegen*, unsigned dst, unsigned src, int32_t imm);       /* imul r32, r32, imm32 */
void x86_imul_rm  (x86_codegen*, unsigned dst, unsigned base, int index, unsigned scale_log2, int32_t disp);
void x86_movsxd_rr(x86_codegen*, unsigned dst, unsigned src);                    /* movsxd r64, r32 */
void x86_movsxd   (x86_codegen*, unsigned dst, unsigned src);         /* dst64 = sext(src32) */
void x86_movsxd_rm(x86_codegen*, unsigned dst, unsigned base, int index, unsigned scale_log2, int32_t disp);
void x86_lea64    (x86_codegen*, unsigned dst, unsigned base, int index, unsigned scale_log2, int32_t disp);
void x86_lea_rip  (x86_codegen*, unsigned dst, const void* target);     /* dst = target (RIP-relative, +/-2 GiB) */

/* Byte / flag forms.  Byte registers are AL/CL/DL only (0..2) plus
 * R8B..R15B on x86-64; the encoder asserts on anything else. */
void x86_setcc_r8 (x86_codegen*, unsigned cc, unsigned r);
void x86_setcc_m8 (x86_codegen*, unsigned cc, unsigned base, int index, unsigned scale_log2, int32_t disp);
void x86_mov_m8r8 (x86_codegen*, unsigned base, int index, unsigned scale_log2, int32_t disp, unsigned src);
void x86_or_m8r8  (x86_codegen*, unsigned base, int index, unsigned scale_log2, int32_t disp, unsigned src);
void x86_movzx_rr8(x86_codegen*, unsigned dst, unsigned src);
void x86_movsx_rr8(x86_codegen*, unsigned dst, unsigned src);
void x86_inc_m8   (x86_codegen*, unsigned base, int index, unsigned scale_log2, int32_t disp);
void x86_dec_m8   (x86_codegen*, unsigned base, int index, unsigned scale_log2, int32_t disp);
void x86_cmp_mi32 (x86_codegen*, unsigned base, int index, unsigned scale_log2, int32_t disp, int32_t imm);
void x86_cmp_mr   (x86_codegen*, unsigned base, int index, unsigned scale_log2, int32_t disp, unsigned r);
void x86_alu_mi32 (x86_codegen*, unsigned op, unsigned base, int index, unsigned scale_log2, int32_t disp, int32_t imm);
void x86_alu_mr   (x86_codegen*, unsigned op, unsigned base, int index, unsigned scale_log2, int32_t disp, unsigned src);
void x86_imul_m   (x86_codegen*, unsigned base, int index, unsigned scale_log2, int32_t disp);  /* EDX:EAX = EAX * [m] */
void x86_rol_ri   (x86_codegen*, unsigned r, unsigned imm);
void x86_ror_ri   (x86_codegen*, unsigned r, unsigned imm);
void x86_mov_mi16 (x86_codegen*, unsigned base, int index, unsigned scale_log2, int32_t disp, uint16_t imm);

/* Control transfer to absolute addresses. */
void x86_jmp_r    (x86_codegen*, unsigned r);
void x86_call_r   (x86_codegen*, unsigned r);
void x86_jmp_abs  (x86_codegen*, const void* target);    /* E9 rel32; asserts if out of range on x86-64 */
void x86_call_abs (x86_codegen*, const void* target);    /* E8 rel32 (x86-32) / MOV RAX,imm64; CALL RAX (x86-64) */
void x86_jcc_abs  (x86_codegen*, unsigned cc, const void* target);
void x86_int3     (x86_codegen*);
/* Re-target an E9 rel32 JMP whose opcode byte is at `site`. */
void x86_patch_jmp_abs(void* site, const void* target);

/* Like x86_codegen_create, but places the segment within +/-2 GiB of
 * `nearp` (probing hint addresses on both mmap and VirtualAlloc); NULL if
 * no such placement exists.  On x86-32 every address qualifies. */
x86_codegen* x86_codegen_create_near(size_t bytes, const void* nearp);

void x86_jcc      (x86_codegen*, unsigned cc, x86_label*);
void x86_jmp      (x86_codegen*, x86_label*);
void x86_push     (x86_codegen*, unsigned r);
void x86_pop      (x86_codegen*, unsigned r);
void x86_ret      (x86_codegen*);

#ifdef __cplusplus
}
#endif

#endif
