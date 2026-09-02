/******************************************************************************/
/* Mednafen Sega Saturn Emulation Module                                      */
/******************************************************************************/
/* scsp_dsp_jit_x86.c - SCSP MPROG DSP JIT, x86 / x86-64 backend
**
** Compiles the 128-step MPROG into one straight-line native function,
** void entry(SS_SCSP*), with the same contract as the aarch64 backend in
** scsp_dsp_jit.c: it replaces SS_SCSP_RunDSPInterpreter for a pass,
** including the MDEC_CT update at the end.  Dead steps emit nothing;
** every decoded field of a live step (IRA, TRA/TWA, CRA, MASA, IWA, EWA,
** the flag word) plus RBL/RBP is folded into the code, so a recompile
** is required whenever any of those change -- the same MPROG_Dirty
** trigger the interpreter already services.
**
** Register plan (identical on both widths):
**
**   EBX  SS_SCSP* z              callee-saved everywhere
**   ESI  MDEC_CT                 per-pass constant; TEMP indices are
**                                (TRA|TWA + ESI) & 0x7F
**   EAX  INPUTS, then the multiplier X side, then the product
**   ECX  Y multiplicand / RAM and TEMP index
**   EDX  ShifterOutput (SO); spilled to [ESP] across the multiply
**   EBP  TEMP[TRA+MDEC_CT] / scratch
**   EDI  scratch
**
** Everything else (INPUTS, SFT_REG, FRC_REG, Y_REG, ADRS_REG, the RAM
** pipeline's RWAddr / ReadPending / WritePending / ReadValue / WriteValue)
** stays memory-resident in the SS_SCSP block and is reached as
** [EBX + disp32].  That is what lets one emitter serve x86-32's six
** usable registers and x86-64 alike: the cost is a store-to-load forward
** per pipeline access, which is invisible next to the unrolling and
** field folding that constitute the JIT's actual win.  The 1 MiB ring
** RAM is addressed as [EBX + ECX*2 + offsetof(RAM)], so no base register
** is spent on it either.
**
** Semantics are a line-for-line transcription of SS_SCSP_RunDSPStep in
** scsp.inc; the comments name the interpreter statement each block
** implements.  scsp_dsp_difftest.c is the oracle.
*/

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "ss.h"
#include "scsp.h"
#include "scsp_dsp_jit.h"
#include "x86emit.h"
#include "jitdump.h"

#if defined(WANT_JIT) && X86EMIT_HOST

#define SCSP_X86_CODE_BYTES (128u * 1024u)   /* ~24 KiB typical; ample headroom */

static x86_codegen* g_cg = NULL;

#define O(field) ((int32_t)offsetof(SS_SCSP, field))

/* Memory-operand shorthands. */
#define M_Z(disp)               X86_EBX, X86_NOIDX, 0, (disp)
#define M_ZIDX(idx, scale, disp) X86_EBX, (idx), (scale), (disp)
#define M_SPILL                 X86_ESP, X86_NOIDX, 0, 0

enum { EAX = X86_EAX, ECX = X86_ECX, EDX = X86_EDX, EBX = X86_EBX,
       ESP = X86_ESP, EBP = X86_EBP, ESI = X86_ESI, EDI = X86_EDI };

/* --- label pool (per compile) ----------------------------------------- */

#define LABEL_POOL 1024
static x86_label g_labels[LABEL_POOL];
static unsigned  g_label_count;

static x86_label* label_new(void)
{
 x86_label* l = &g_labels[g_label_count < LABEL_POOL ? g_label_count : LABEL_POOL - 1];
 if(g_label_count < LABEL_POOL) g_label_count++;
 x86_label_init(l);
 return l;
}

/* --- dspfloat helpers ------------------------------------------------- */

/*
 * dspfloat_to_int(inv) with inv in ECX; result in EBP.  Clobbers
 * EAX, ECX, EDX, EDI.
 *
 *   sign_xor = (int32)((inv & 0x8000) << 16) >> 1
 *   exp      = (inv >> 11) & 0xF
 *   ret      = inv & 0x7FF;  if(exp < 12) ret |= 0x800
 *   ret    <<= 19;  ret ^= sign_xor
 *   ret      = (int32)ret >> (8 + min(11, exp))
 */
static void emit_dspfloat_to_int(void)
{
 x86_label* no_hidden = label_new();
 x86_label* clamp_done = label_new();

 x86_mov_rr(g_cg, EBP, ECX);
 x86_alu_ri(g_cg, X86_AND, EBP, 0x7FF);
 x86_mov_rr(g_cg, EAX, ECX);
 x86_shift_ri(g_cg, X86_SHR, EAX, 11);
 x86_alu_ri(g_cg, X86_AND, EAX, 0xF);            /* EAX = exp */
 x86_alu_ri(g_cg, X86_CMP, EAX, 12);
 x86_jcc(g_cg, X86_CC_AE, no_hidden);
 x86_alu_ri(g_cg, X86_OR, EBP, 0x800);
 x86_label_bind(g_cg, no_hidden);
 x86_shift_ri(g_cg, X86_SHL, EBP, 19);
 /* sign_xor: (inv << 16) & 0x80000000, then >> 1 arithmetic. */
 x86_mov_rr(g_cg, EDX, ECX);
 x86_shift_ri(g_cg, X86_SHL, EDX, 16);
 x86_alu_ri(g_cg, X86_AND, EDX, (int32_t)0x80000000);
 x86_shift_ri(g_cg, X86_SAR, EDX, 1);
 x86_alu_rr(g_cg, X86_XOR, EBP, EDX);
 /* shift = 8 + min(11, exp) */
 x86_alu_ri(g_cg, X86_CMP, EAX, 11);
 x86_jcc(g_cg, X86_CC_BE, clamp_done);
 x86_mov_ri(g_cg, EAX, 11);
 x86_label_bind(g_cg, clamp_done);
 x86_alu_ri(g_cg, X86_ADD, EAX, 8);
 x86_mov_rr(g_cg, ECX, EAX);
 x86_sar_cl(g_cg, EBP);
}

/*
 * int_to_dspfloat(inv) with inv in EDX; result (16-bit) in EDI.
 * Clobbers EAX, ECX, EDX, EBP.
 *
 *   invsl8   = inv << 8
 *   sign_xor = (int32)invsl8 >> 31
 *   exp      = lzcount32(((invsl8 ^ sign_xor) << 1) | (1 << 19))   [0..12]
 *   shift    = exp - (exp == 12)
 *   ret      = (int32)invsl8 >> (19 - shift);  ret &= 0x87FF;  ret |= exp << 11
 *
 * The OR with 1<<19 guarantees a set bit, so BSR is defined and
 * lzcount = 31 - BSR.
 */
static void emit_int_to_dspfloat(void)
{
 x86_label* not12 = label_new();

 x86_shift_ri(g_cg, X86_SHL, EDX, 8);            /* EDX = invsl8 */
 x86_mov_rr(g_cg, EBP, EDX);
 x86_shift_ri(g_cg, X86_SAR, EBP, 31);           /* EBP = sign_xor */
 x86_alu_rr(g_cg, X86_XOR, EBP, EDX);
 x86_shift_ri(g_cg, X86_SHL, EBP, 1);
 x86_alu_ri(g_cg, X86_OR, EBP, 1 << 19);
 x86_bsr(g_cg, EAX, EBP);
 x86_mov_ri(g_cg, ECX, 31);
 x86_alu_rr(g_cg, X86_SUB, ECX, EAX);            /* ECX = exp */
 x86_mov_rr(g_cg, EDI, ECX);                     /* EDI = exp (kept) */
 x86_alu_ri(g_cg, X86_CMP, ECX, 12);
 x86_jcc(g_cg, X86_CC_NE, not12);
 x86_alu_ri(g_cg, X86_SUB, ECX, 1);
 x86_label_bind(g_cg, not12);                    /* ECX = shift */
 x86_mov_ri(g_cg, EAX, 19);
 x86_alu_rr(g_cg, X86_SUB, EAX, ECX);
 x86_mov_rr(g_cg, ECX, EAX);                     /* CL = 19 - shift */
 x86_sar_cl(g_cg, EDX);
 x86_alu_ri(g_cg, X86_AND, EDX, 0x87FF);
 x86_shift_ri(g_cg, X86_SHL, EDI, 11);
 x86_alu_rr(g_cg, X86_OR, EDI, EDX);
}

/* --- one live step ------------------------------------------------------ */

static void emit_step(const SS_SCSP_DSPStep* s, unsigned rbl, unsigned rbp)
{
 const uint32_t f     = s->flags;
 const unsigned IRA   = s->IRA;
 const unsigned shft0 = (f >> 7) & 1;
 const unsigned shft1 = (f >> 8) & 1;
 const bool need_temp = !(f & DSPF_XSEL) || !(f & DSPF_BSEL);

 /* --- INPUTS -------------------------------------------------------- */
 if(IRA & 0x20)
 {
  if(IRA & 0x10)
  {
   if(!(IRA & 0xE))
   {
    /* INPUTS = (int16)EXTS[IRA & 1] << 8 */
    x86_movsx_rm16(g_cg, EAX, M_Z(O(EXTS) + (int32_t)((IRA & 1) * 2)));
    x86_shift_ri(g_cg, X86_SHL, EAX, 8);
    x86_mov_mr(g_cg, M_Z(O(DSP.INPUTS)), EAX);
   }
   else
    x86_mov_rm(g_cg, EAX, M_Z(O(DSP.INPUTS)));  /* unchanged */
  }
  else
  {
   /* INPUTS = sign_x_to_s32(20, MIXS[IRA & 0xF]) << 4  ==  (MIXS << 12) >> 8 */
   x86_mov_rm(g_cg, EAX, M_Z(O(DSP.MIXS) + (int32_t)((IRA & 0xF) * 4)));
   x86_shift_ri(g_cg, X86_SHL, EAX, 12);
   x86_shift_ri(g_cg, X86_SAR, EAX, 8);
   x86_mov_mr(g_cg, M_Z(O(DSP.INPUTS)), EAX);
  }
 }
 else
 {
  x86_mov_rm(g_cg, EAX, M_Z(O(DSP.MEMS) + (int32_t)((IRA & 0x1F) * 4)));
  x86_mov_mr(g_cg, M_Z(O(DSP.INPUTS)), EAX);
 }
 /* EAX = INPUTS */

 /* --- Y multiplicand -> ECX ------------------------------------------ */
 switch(s->YSEL & 3)
 {
  case 0: x86_movzx_rm16(g_cg, ECX, M_Z(O(DSP.FRC_REG))); break;
  case 1: x86_movzx_rm16(g_cg, ECX, M_Z(O(DSP.COEF) + (int32_t)(s->CRA * 2))); break;
  case 2: x86_mov_rm(g_cg, ECX, M_Z(O(DSP.Y_REG)));
          x86_shift_ri(g_cg, X86_SHR, ECX, 11);
          x86_alu_ri(g_cg, X86_AND, ECX, 0x1FFF); break;
  default:x86_mov_rm(g_cg, ECX, M_Z(O(DSP.Y_REG)));
          x86_shift_ri(g_cg, X86_SHR, ECX, 4);
          x86_alu_ri(g_cg, X86_AND, ECX, 0xFFF); break;
 }

 /* --- YRL: Y_REG = INPUTS & 0xFFFFFF ---------------------------------- */
 if(f & DSPF_YRL)
 {
  x86_mov_rr(g_cg, EDX, EAX);
  x86_alu_ri(g_cg, X86_AND, EDX, 0xFFFFFF);
  x86_mov_mr(g_cg, M_Z(O(DSP.Y_REG)), EDX);
 }

 /* --- ShifterOutput -> EDX -------------------------------------------- */
 /* SO = sign_x_to_s32(26, SFT_REG) << (shft0 ^ shft1) */
 x86_mov_rm(g_cg, EDX, M_Z(O(DSP.SFT_REG)));
 x86_shift_ri(g_cg, X86_SHL, EDX, 6);
 x86_shift_ri(g_cg, X86_SAR, EDX, 6 - (shft0 ^ shft1));
 if(!shft1)
 {
  /* saturate to [-0x800000, 0x7FFFFF] */
  x86_mov_ri(g_cg, EDI, 0x7FFFFF);
  x86_alu_rr(g_cg, X86_CMP, EDX, EDI);
  x86_cmov(g_cg, X86_CC_G, EDX, EDI);
  x86_mov_ri(g_cg, EDI, (uint32_t)-0x800000);
  x86_alu_rr(g_cg, X86_CMP, EDX, EDI);
  x86_cmov(g_cg, X86_CC_L, EDX, EDI);
 }
 else
 {
  /* sign_x_to_s32(24, SO) */
  x86_shift_ri(g_cg, X86_SHL, EDX, 8);
  x86_shift_ri(g_cg, X86_SAR, EDX, 8);
 }

 /* --- FRCL ---------------------------------------------------------- */
 if(f & DSPF_FRCL)
 {
  x86_mov_rr(g_cg, EDI, EDX);
  if(shft0 & shft1)
   x86_alu_ri(g_cg, X86_AND, EDI, 0xFFF);
  else
  {
   x86_shift_ri(g_cg, X86_SAR, EDI, 11);
   x86_alu_ri(g_cg, X86_AND, EDI, 0x1FFF);
  }
  x86_mov_mr16(g_cg, M_Z(O(DSP.FRC_REG)), EDI);
 }

 /* --- multiply / SGA / SFT_REG ---------------------------------------- */
 /* SO lives in EDX and the one-operand IMUL needs EDX:EAX; spill it. */
 x86_mov_mr(g_cg, M_SPILL, EDX);

 if(need_temp)
 {
  /* EBP = TEMP[(TRA + MDEC_CT) & 0x7F] */
  x86_lea(g_cg, EBP, X86_ESI, X86_NOIDX, 0, (int32_t)s->TRA);
  x86_alu_ri(g_cg, X86_AND, EBP, 0x7F);
  x86_mov_rm(g_cg, EBP, M_ZIDX(EBP, 2, O(DSP.TEMP)));
 }
 /* x_input: XSEL ? INPUTS : TEMP */
 if(!(f & DSPF_XSEL))
  x86_mov_rr(g_cg, EAX, EBP);
 /* y: sign_x_to_s32(13, y_input) */
 x86_shift_ri(g_cg, X86_SHL, ECX, 19);
 x86_shift_ri(g_cg, X86_SAR, ECX, 19);
 /* Product = ((int64)y * x) >> 12, low 32 bits */
 x86_imul_r(g_cg, ECX);
 x86_shrd_rri(g_cg, EAX, EDX, 12);
 /* SGAOutput */
 if(!(f & DSPF_ZERO))
 {
  if(f & DSPF_BSEL)
   x86_mov_rm(g_cg, EDX, M_Z(O(DSP.SFT_REG)));   /* old SFT_REG */
  else
   x86_mov_rr(g_cg, EDX, EBP);
  if(f & DSPF_NEGB)
   x86_neg(g_cg, EDX);
  x86_alu_rr(g_cg, X86_ADD, EAX, EDX);
 }
 x86_alu_ri(g_cg, X86_AND, EAX, 0x3FFFFFF);
 x86_mov_mr(g_cg, M_Z(O(DSP.SFT_REG)), EAX);

 /* SO back in EDX */
 x86_mov_rm(g_cg, EDX, M_SPILL);

 /* --- EWT: EFREG[EWA] = SO >> 8 ---------------------------------------- */
 if(f & DSPF_EWT)
 {
  x86_mov_rr(g_cg, EDI, EDX);
  x86_shift_ri(g_cg, X86_SAR, EDI, 8);
  x86_mov_mr16(g_cg, M_Z(O(DSP.EFREG) + (int32_t)(s->EWA * 2)), EDI);
 }

 /* --- TWT: TEMP[(TWA + MDEC_CT) & 0x7F] = SO ---------------------------- */
 if(f & DSPF_TWT)
 {
  x86_lea(g_cg, ECX, X86_ESI, X86_NOIDX, 0, (int32_t)s->TWA);
  x86_alu_ri(g_cg, X86_AND, ECX, 0x7F);
  x86_mov_mr(g_cg, M_ZIDX(ECX, 2, O(DSP.TEMP)), EDX);
 }

 /* --- IWT: MEMS[IWA] = ReadValue ---------------------------------------- */
 if(f & DSPF_IWT)
 {
  x86_mov_rm(g_cg, ECX, M_Z(O(DSP.ReadValue)));
  x86_mov_mr(g_cg, M_Z(O(DSP.MEMS) + (int32_t)(s->IWA * 4)), ECX);
 }

 /* --- RAM pipeline -------------------------------------------------------- */
 {
  x86_label* do_read  = label_new();
  x86_label* done     = label_new();
  x86_label* skip_w   = label_new();
  x86_label* nofl     = label_new();
  x86_label* store_rv = label_new();

  x86_cmp_mi8(g_cg, M_Z(O(DSP.ReadPending)), 0);
  x86_jcc(g_cg, X86_CC_NE, do_read);
  x86_cmp_mi8(g_cg, M_Z(O(DSP.WritePending)), 0);
  x86_jcc(g_cg, X86_CC_E, done);
  /* write: if(!(RWAddr & 0x40000)) RAM[RWAddr] = WriteValue; WritePending = 0 */
  x86_mov_rm(g_cg, ECX, M_Z(O(DSP.RWAddr)));
  x86_test_ri(g_cg, ECX, 0x40000);
  x86_jcc(g_cg, X86_CC_NE, skip_w);
  x86_movzx_rm16(g_cg, EDI, M_Z(O(DSP.WriteValue)));
  x86_mov_mr16(g_cg, M_ZIDX(ECX, 1, O(RAM)), EDI);
  x86_label_bind(g_cg, skip_w);
  x86_mov_mi8(g_cg, M_Z(O(DSP.WritePending)), 0);
  x86_jmp(g_cg, done);
  /* read: tmp = RAM[RWAddr]; ReadValue = RP==2 ? (int16)tmp<<8 : dspfloat_to_int(tmp); RP = 0 */
  x86_label_bind(g_cg, do_read);
  x86_mov_rm(g_cg, ECX, M_Z(O(DSP.RWAddr)));
  x86_movzx_rm16(g_cg, ECX, M_ZIDX(ECX, 1, O(RAM)));
  x86_cmp_mi8(g_cg, M_Z(O(DSP.ReadPending)), 2);
  x86_jcc(g_cg, X86_CC_E, nofl);
  emit_dspfloat_to_int();                       /* EBP = result */
  x86_jmp(g_cg, store_rv);
  x86_label_bind(g_cg, nofl);
  x86_movsx_rr16(g_cg, EBP, ECX);
  x86_shift_ri(g_cg, X86_SHL, EBP, 8);
  x86_label_bind(g_cg, store_rv);
  x86_mov_mr(g_cg, M_Z(O(DSP.ReadValue)), EBP);
  x86_mov_mi8(g_cg, M_Z(O(DSP.ReadPending)), 0);
  x86_label_bind(g_cg, done);
 }

 /* SO again (the read path clobbered EDX) */
 x86_mov_rm(g_cg, EDX, M_SPILL);

 /* --- MADRS / RWAddr -------------------------------------------------- */
 /* uint16 addr = MADRS[MASA]; addr += NXADDR; if(ADRGB) addr += sxt12(ADRS_REG);
  * if(!TABLE) { addr += MDEC_CT; addr &= (0x2000 << RBL) - 1; }
  * RWAddr = (addr + (RBP << 12)) & 0x7FFFF  -- every add wraps at 16 bits. */
 x86_movzx_rm16(g_cg, ECX, M_Z(O(DSP.MADRS) + (int32_t)(s->MASA * 2)));
 if(f & DSPF_NXADDR)
  x86_alu_ri(g_cg, X86_ADD, ECX, 1);
 if(f & DSPF_ADRGB)
 {
  x86_movzx_rm16(g_cg, EDI, M_Z(O(DSP.ADRS_REG)));
  x86_shift_ri(g_cg, X86_SHL, EDI, 20);
  x86_shift_ri(g_cg, X86_SAR, EDI, 20);
  x86_alu_rr(g_cg, X86_ADD, ECX, EDI);
 }
 if(!(f & DSPF_TABLE))
 {
  x86_alu_rr(g_cg, X86_ADD, ECX, ESI);
  x86_alu_ri(g_cg, X86_AND, ECX, (int32_t)((0x2000u << (rbl & 3)) - 1u));
 }
 else
  x86_alu_ri(g_cg, X86_AND, ECX, 0xFFFF);
 x86_alu_ri(g_cg, X86_ADD, ECX, (int32_t)((uint32_t)rbp << 12));
 x86_alu_ri(g_cg, X86_AND, ECX, 0x7FFFF);
 x86_mov_mr(g_cg, M_Z(O(DSP.RWAddr)), ECX);

 if(f & DSPF_MRT)
  x86_mov_mi8(g_cg, M_Z(O(DSP.ReadPending)), (f & DSPF_NOFL) ? 2 : 1);

 if(f & DSPF_MWT)
 {
  x86_mov_mi8(g_cg, M_Z(O(DSP.WritePending)), 1);
  if(f & DSPF_NOFL)
  {
   x86_mov_rr(g_cg, EDI, EDX);
   x86_shift_ri(g_cg, X86_SAR, EDI, 8);
  }
  else
   emit_int_to_dspfloat();                      /* EDI = result; clobbers EDX */
  x86_mov_mr16(g_cg, M_Z(O(DSP.WriteValue)), EDI);
 }

 /* --- ADRL ----------------------------------------------------------- */
 if(f & DSPF_ADRL)
 {
  if(shft0 & shft1)
  {
   x86_mov_rm(g_cg, EDI, M_SPILL);              /* SO */
   x86_shift_ri(g_cg, X86_SAR, EDI, 12);
  }
  else
  {
   x86_mov_rm(g_cg, EDI, M_Z(O(DSP.INPUTS)));
   x86_shift_ri(g_cg, X86_SAR, EDI, 16);
  }
  x86_alu_ri(g_cg, X86_AND, EDI, 0xFFF);
  x86_mov_mr16(g_cg, M_Z(O(DSP.ADRS_REG)), EDI);
 }
}

/* --- prologue / epilogue ------------------------------------------------ */

/*
 * Frame: push the callee-saved registers every ABI we run under wants
 * preserved among the ones we touch (EBX, EBP, ESI, EDI), then reserve
 * 16 bytes for the SO spill slot.  No calls are made from the body, so
 * neither stack alignment nor Win64 shadow space matters.
 *
 *   x86-32 cdecl : z at [ESP + 4*4 pushes + 16 + 4 return] = [ESP+36]
 *   x86-64 SysV  : z in RDI
 *   x86-64 Win64 : z in RCX
 */
static void emit_prologue(void)
{
 x86_push(g_cg, EBX);
 x86_push(g_cg, EBP);
 x86_push(g_cg, ESI);
 x86_push(g_cg, EDI);
#if X86EMIT_64
 x86_alu_ri64(g_cg, X86_SUB, ESP, 16);
 #if defined(_WIN32) || defined(SCSP_JIT_X86_MSABI)
 /* SCSP_JIT_X86_MSABI: test hook so the Win64 prologue can be exercised
  * on a SysV host through an __attribute__((ms_abi)) call. */
 x86_mov_rr64(g_cg, EBX, ECX);
 #else
 x86_mov_rr64(g_cg, EBX, EDI);
 #endif
#else
 x86_alu_ri(g_cg, X86_SUB, ESP, 16);
 x86_mov_rm(g_cg, EBX, X86_ESP, X86_NOIDX, 0, 36);
#endif
 x86_movzx_rm16(g_cg, ESI, M_Z(O(DSP.MDEC_CT)));
}

static void emit_epilogue(unsigned rbl)
{
 x86_label* nz = label_new();

 /* if(!MDEC_CT) MDEC_CT = 0x2000 << RBL;  MDEC_CT--; */
 x86_test_rr(g_cg, ESI, ESI);
 x86_jcc(g_cg, X86_CC_NE, nz);
 x86_mov_ri(g_cg, ESI, 0x2000u << (rbl & 3));
 x86_label_bind(g_cg, nz);
 x86_alu_ri(g_cg, X86_SUB, ESI, 1);
 x86_mov_mr16(g_cg, M_Z(O(DSP.MDEC_CT)), ESI);

#if X86EMIT_64
 x86_alu_ri64(g_cg, X86_ADD, ESP, 16);
#else
 x86_alu_ri(g_cg, X86_ADD, ESP, 16);
#endif
 x86_pop(g_cg, EDI);
 x86_pop(g_cg, ESI);
 x86_pop(g_cg, EBP);
 x86_pop(g_cg, EBX);
 x86_ret(g_cg);
}

/* --- public API ----------------------------------------------------------- */

void SCSP_DSP_JIT_Init(struct SS_SCSP* scsp)
{
 (void)scsp;
 if(!g_cg)
  g_cg = x86_codegen_create(SCSP_X86_CODE_BYTES);
 SCSP_DSP_JIT_Entry = NULL;
}

void SCSP_DSP_JIT_Reset(struct SS_SCSP* scsp)
{
 (void)scsp;
 SCSP_DSP_JIT_Entry = NULL;
}

void SCSP_DSP_JIT_Compile(struct SS_SCSP* scsp)
{
 unsigned i;
 void* entry;
 const unsigned rbl = scsp->RBL & 3;
 const unsigned rbp = scsp->RBP;

 SCSP_DSP_JIT_Entry = NULL;
 if(!g_cg) return;

 x86_codegen_set_wptr(g_cg, x86_codegen_base(g_cg));
 g_label_count = 0;
 entry = x86_codegen_wptr(g_cg);

 emit_prologue();
 for(i = 0; i < 128u; i++)
 {
  const SS_SCSP_DSPStep* s = &scsp->DSP.MPROG_Decoded[i];
  if(s->live)
   emit_step(s, rbl, rbp);
 }
 emit_epilogue(rbl);

 if(x86_codegen_overflowed(g_cg))
  return;                                       /* interpreter keeps the pass */

 SS_JitDump_Emit("scsp_mprog", entry, (size_t)((char*)x86_codegen_wptr(g_cg) - (char*)entry));
 SCSP_DSP_JIT_Entry = (void (*)(struct SS_SCSP*))entry;
}

#endif /* WANT_JIT && X86EMIT_HOST */
