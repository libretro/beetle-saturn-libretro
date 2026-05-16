/******************************************************************************/
/* Mednafen Sega Saturn Emulation Module                                      */
/******************************************************************************/
/* scu_dsp_gen.cpp - SCU DSP General Instructions Emulation
**  Copyright (C) 2015-2016 Mednafen Team
**
** This program is free software; you can redistribute it and/or
** modify it under the terms of the GNU General Public License
** as published by the Free Software Foundation; either version 2
** of the License, or (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software Foundation, Inc.,
** 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include "ss.h"
#include "scu.h"

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC optimize("Os")
#endif

  // Is first DSP instruction cached on PC load, or when execution starts?

  // MOV [s],[d] s=0x8 = 0xFFFFFFFF?

  // MOV [s],[d] when moving from/to the same data RAM bank seems to be treated like a NOP...

#include "scu_dsp_common.inc"

static FORCE_INLINE void SetC(struct DSPS* dsp, bool value)
{
 dsp->FlagC = value;
}

static FORCE_INLINE void CalcZS32(struct DSPS* dsp, uint32_t val)
{
 dsp->FlagS = (int32_t)val < 0;
 dsp->FlagZ = !val;
}

static FORCE_INLINE void CalcZS48(struct DSPS* dsp, uint64_t val)
{
 val <<= 16;

 dsp->FlagS = (int64_t)val < 0;
 dsp->FlagZ = !val;
}

/* On big-endian hosts the per-byte CT-increment mask is in the
 * reversed byte order; swap it so the `+ ct_inc` addition lands
 * on the right CT[] byte.  Hoisted out of the body macro because
 * `#ifdef` inside a function-like macro is not portable (the `#`
 * is taken as the stringizing operator). */
#ifdef MSB_FIRST
 #define DGI_CT_INC_HOST_FIX(ct_inc) \
  ((ct_inc) = ((ct_inc) << 24) | (((ct_inc) & 0xFF00) << 8) | (((ct_inc) >> 8) & 0xFF00) | ((ct_inc) >> 24))
#else
 #define DGI_CT_INC_HOST_FIX(ct_inc) ((void)0)
#endif

/* Phase-5d: was `template<const bool looped, const unsigned alu_op,
 * const unsigned x_op, const unsigned y_op, const unsigned d1_op>
 * static NO_INLINE NO_CLONE void GeneralInstr(DSPS* dsp)` -- 5-axis
 * template (2 x 16 x 8 x 8 x 4 = 8192 table slots, 2 x 12 x 7 x 8 x
 * 4 = 5376 unique instantiations after the gentab.inc hand-dedup of
 * the NOP-equivalent alu values (0x00, 0x07, 0x0C..0x0E all hit the
 * `default` arm of the switch, so they collapse to 0x00) and the
 * x_op=0x00/0x01 collapse (both produce the same code -- neither
 * triggers `(x_op & 3) == 2` nor `x_op >= 3`).
 *
 * Monomorphized via X-macro into exactly 5376 named functions
 * (GeneralInstr_<L>_<ALU>_<X>_<Y>_<D1>) over the same instantiation
 * set the C++ template emitted; the regenerated scu_dsp_gentab.inc
 * keeps the 8192-slot layout with the same hand-dedup pattern,
 * so 2816 slots reference an already-emitted function name.  No
 * codegen change. */

#define GeneralInstr_BODY(LOOPED, ALU_OP, X_OP, Y_OP, D1_OP)                                       \
{                                                                                                  \
 const uint32_t instr = DSP_InstrPre(dsp, LOOPED);                                                 \
 /* */                                                                                             \
 union DSPR48 ALU = dsp->AC;                                                                       \
 unsigned dr_read = 0;                                                                             \
 unsigned ct_inc = 0;                                                                              \
                                                                                                   \
 switch(ALU_OP)                                                                                    \
 {                                                                                                 \
  /* NOP */                                                                                        \
  default: { } break;                                                                              \
                                                                                                   \
  /* AND */                                                                                        \
  case 0x01: { ALU.L &= dsp->P.L; SetC(dsp, false); CalcZS32(dsp, ALU.L); } break;                 \
                                                                                                   \
  /* OR */                                                                                         \
  case 0x02: { ALU.L |= dsp->P.L; SetC(dsp, false); CalcZS32(dsp, ALU.L); } break;                 \
                                                                                                   \
  /* XOR */                                                                                        \
  case 0x03: { ALU.L ^= dsp->P.L; SetC(dsp, false); CalcZS32(dsp, ALU.L); } break;                 \
                                                                                                   \
  /* ADD */                                                                                        \
  case 0x04: {                                                                                     \
   const uint64_t tmp = (uint64_t)ALU.L + dsp->P.L;                                                \
   dsp->FlagV |= (((~(ALU.L ^ dsp->P.L)) & (ALU.L ^ tmp)) >> 31) & 1;                              \
   SetC(dsp, (tmp >> 32) & 0x1);                                                                   \
   CalcZS32(dsp, tmp);                                                                             \
   ALU.L = tmp;                                                                                    \
  } break;                                                                                         \
                                                                                                   \
  /* SUB */                                                                                        \
  case 0x05: {                                                                                     \
   const uint64_t tmp = (uint64_t)ALU.L - dsp->P.L;                                                \
   dsp->FlagV |= ((((ALU.L ^ dsp->P.L)) & (ALU.L ^ tmp)) >> 31) & 1;                               \
   SetC(dsp, (tmp >> 32) & 0x1);                                                                   \
   CalcZS32(dsp, tmp);                                                                             \
   ALU.L = tmp;                                                                                    \
  } break;                                                                                         \
                                                                                                   \
  /* AD2 */                                                                                        \
  case 0x06: {                                                                                     \
   const uint64_t tmp = (ALU.T & 0xFFFFFFFFFFFFULL) + (dsp->P.T & 0xFFFFFFFFFFFFULL);              \
   dsp->FlagV |= (((~(ALU.T ^ dsp->P.T)) & (ALU.T ^ tmp)) >> 47) & 1;                              \
   SetC(dsp, (tmp >> 48) & 0x1);                                                                   \
   CalcZS48(dsp, tmp);                                                                             \
   ALU.T = tmp;                                                                                    \
  } break;                                                                                         \
                                                                                                   \
  /* SR */                                                                                         \
  case 0x08: {                                                                                     \
   const bool new_C = ALU.L & 0x1;                                                                 \
   SetC(dsp, new_C);                                                                               \
   ALU.L = (int32_t)ALU.L >> 1;                                                                    \
   CalcZS32(dsp, ALU.L);                                                                           \
  } break;                                                                                         \
                                                                                                   \
  /* RR */                                                                                         \
  case 0x09: {                                                                                     \
   const bool new_C = ALU.L & 0x1;                                                                 \
   SetC(dsp, new_C);                                                                               \
   ALU.L = (ALU.L >> 1) | (new_C << 31);                                                           \
   CalcZS32(dsp, ALU.L);                                                                           \
  } break;                                                                                         \
                                                                                                   \
  /* SL */                                                                                         \
  case 0x0A: {                                                                                     \
   const bool new_C = ALU.L >> 31;                                                                 \
   SetC(dsp, new_C);                                                                               \
   ALU.L <<= 1;                                                                                    \
   CalcZS32(dsp, ALU.L);                                                                           \
  } break;                                                                                         \
                                                                                                   \
  /* RL */                                                                                         \
  case 0x0B: {                                                                                     \
   const bool new_C = ALU.L >> 31;                                                                 \
   SetC(dsp, new_C);                                                                               \
   ALU.L = (ALU.L << 1) | new_C;                                                                   \
   CalcZS32(dsp, ALU.L);                                                                           \
  } break;                                                                                         \
                                                                                                   \
  /* RL8 */                                                                                        \
  case 0x0F: {                                                                                     \
   const bool new_C = (ALU.L >> 24) & 1;                                                           \
   SetC(dsp, new_C);                                                                               \
   ALU.L = (ALU.L << 8) | (ALU.L >> 24);                                                           \
   CalcZS32(dsp, ALU.L);                                                                           \
  } break;                                                                                         \
 }                                                                                                 \
                                                                                                   \
 /* X Op */                                                                                        \
 if(((X_OP) & 0x3) == 0x2)                                                                         \
  dsp->P.T = (int64_t)(int32_t)dsp->RX * (int32_t)dsp->RY;                                         \
                                                                                                   \
 if((X_OP) >= 0x3)                                                                                 \
 {                                                                                                 \
  const unsigned s = (instr >> 20) & 0x7;                                                          \
  const size_t drw = s & 0x3;                                                                      \
  uint32_t src_data;                                                                               \
                                                                                                   \
  src_data = dsp->DataRAM[drw][dsp->CT[drw]];                                                      \
  dr_read |= 1U << drw;                                                                            \
  ct_inc |= (bool)(s & 0x4) << (drw << 3);                                                         \
                                                                                                   \
  if(((X_OP) & 0x3) == 0x3)                                                                        \
   dsp->P.T = (int32_t)src_data;                                                                   \
                                                                                                   \
  if((X_OP) & 0x4)                                                                                 \
   dsp->RX = src_data;                                                                             \
 }                                                                                                 \
                                                                                                   \
 /* Y Op */                                                                                        \
 if(((Y_OP) & 0x3) == 0x1)                                                                         \
  dsp->AC.T = 0;                                                                                   \
 else if(((Y_OP) & 0x3) == 0x2)                                                                    \
  dsp->AC.T = ALU.T;                                                                               \
                                                                                                   \
 if((Y_OP) >= 0x3)                                                                                 \
 {                                                                                                 \
  const unsigned s = (instr >> 14) & 0x7;                                                          \
  const size_t drw = s & 0x3;                                                                      \
  uint32_t src_data;                                                                               \
                                                                                                   \
  src_data = dsp->DataRAM[drw][dsp->CT[drw]];                                                      \
  dr_read |= 1U << drw;                                                                            \
  ct_inc |= (bool)(s & 0x4) << (drw << 3);                                                         \
                                                                                                   \
  if(((Y_OP) & 0x3) == 0x3)                                                                        \
   dsp->AC.T = (int32_t)src_data;                                                                  \
                                                                                                   \
  if((Y_OP) & 0x4)                                                                                 \
   dsp->RY = src_data;                                                                             \
 }                                                                                                 \
                                                                                                   \
 /* D1 Op (TODO: Test illegal bit patterns) */                                                     \
 if((D1_OP) & 0x1)                                                                                 \
 {                                                                                                 \
  const unsigned d = (instr >> 8) & 0xF;                                                           \
  uint32_t src_data = (int8_t)instr;                                                               \
                                                                                                   \
  if((D1_OP) & 0x2)                                                                                \
  {                                                                                                \
   switch(instr & 0xF)                                                                             \
   {                                                                                               \
    case 0x8: case 0xB: case 0xC: case 0xD: case 0xE: case 0xF: src_data = 0xFFFFFFFF; break;      \
                                                                                                   \
    case 0x0: src_data = dsp->DataRAM[0][dsp->CT[0]]; dr_read |= 0x01; break;                      \
    case 0x1: src_data = dsp->DataRAM[1][dsp->CT[1]]; dr_read |= 0x02; break;                      \
    case 0x2: src_data = dsp->DataRAM[2][dsp->CT[2]]; dr_read |= 0x04; break;                      \
    case 0x3: src_data = dsp->DataRAM[3][dsp->CT[3]]; dr_read |= 0x08; break;                      \
                                                                                                   \
    case 0x4: src_data = dsp->DataRAM[0][dsp->CT[0]]; if(d != 0) { ct_inc |= 1 <<  0; } dr_read |= 0x01; break;\
    case 0x5: src_data = dsp->DataRAM[1][dsp->CT[1]]; if(d != 1) { ct_inc |= 1 <<  8; } dr_read |= 0x02; break;\
    case 0x6: src_data = dsp->DataRAM[2][dsp->CT[2]]; if(d != 2) { ct_inc |= 1 << 16; } dr_read |= 0x04; break;\
    case 0x7: src_data = dsp->DataRAM[3][dsp->CT[3]]; if(d != 3) { ct_inc |= 1 << 24; } dr_read |= 0x08; break;\
                                                                                                   \
    case 0x9: src_data = ALU.T; break;                                                             \
    case 0xA: src_data = ALU.T >> 16; break;                                                       \
   }                                                                                               \
  }                                                                                                \
                                                                                                   \
  switch(d)                                                                                        \
  {                                                                                                \
   case 0x0: if(!(dr_read & 0x01)) { dsp->DataRAM[0][dsp->CT[0]] = src_data; ct_inc |= 1 <<  0; } break;\
   case 0x1: if(!(dr_read & 0x02)) { dsp->DataRAM[1][dsp->CT[1]] = src_data; ct_inc |= 1 <<  8; } break;\
   case 0x2: if(!(dr_read & 0x04)) { dsp->DataRAM[2][dsp->CT[2]] = src_data; ct_inc |= 1 << 16; } break;\
   case 0x3: if(!(dr_read & 0x08)) { dsp->DataRAM[3][dsp->CT[3]] = src_data; ct_inc |= 1 << 24; } break;\
   case 0x4: dsp->RX = src_data; break;                                                            \
   case 0x5: dsp->P.T = (int32_t)src_data; break;                                                  \
   case 0x6: dsp->RAO = src_data; break;                                                           \
   case 0x7: dsp->WAO = src_data; break;                                                           \
   case 0x8: case 0x9: break;                                                                      \
   case 0xA: if(!(LOOPED) || dsp->LOP == 0x0FFF) { dsp->LOP = src_data & 0x0FFF; } break;          \
   case 0xB: dsp->TOP = src_data & 0xFF; break;                                                    \
                                                                                                   \
   /* Don't bother masking with 0x3F here, since the & 0x3F3F3F3F mask down below will cover it    \
    * (and no chance of overflowing into an adjacent byte since we're masking out the              \
    * corresponding byte in ct_inc, too). */                                                       \
   case 0xC: dsp->CT[0] = src_data; ct_inc &= ~0x000000FF; break;                                  \
   case 0xD: dsp->CT[1] = src_data; ct_inc &= ~0x0000FF00; break;                                  \
   case 0xE: dsp->CT[2] = src_data; ct_inc &= ~0x00FF0000; break;                                  \
   case 0xF: dsp->CT[3] = src_data; ct_inc &= ~0xFF000000; break;                                  \
  }                                                                                                \
 }                                                                                                 \
                                                                                                   \
 /* */                                                                                             \
 DGI_CT_INC_HOST_FIX(ct_inc);                                                                      \
                                                                                                   \
 if((X_OP) >= 0x3 || (Y_OP) >= 0x3 || ((D1_OP) & 0x1))                                             \
  dsp->CT32 = (dsp->CT32 + ct_inc) & 0x3F3F3F3F;                                                   \
                                                                                                   \
 DSP_TailDispatch(dsp);                                                                            \
}

#define GeneralInstr_NAME(LOOPED, ALU_OP, X_OP, Y_OP, D1_OP) \
 GeneralInstr_##LOOPED##_##ALU_OP##_##X_OP##_##Y_OP##_##D1_OP

#define DEFINE_GeneralInstr(LOOPED, ALU_OP, X_OP, Y_OP, D1_OP)                                     \
 static NO_INLINE NO_CLONE void GeneralInstr_NAME(LOOPED, ALU_OP, X_OP, Y_OP, D1_OP)(struct DSPS* dsp) \
 GeneralInstr_BODY(LOOPED, ALU_OP, X_OP, Y_OP, D1_OP)

#define DGI_FOR_EACH_D1(M, LOOPED, ALU, X, Y) \
M(LOOPED, ALU, X, Y, 0x0) \
 M(LOOPED, ALU, X, Y, 0x1) \
 M(LOOPED, ALU, X, Y, 0x2) \
 M(LOOPED, ALU, X, Y, 0x3)

#define DGI_FOR_EACH_Y(M, LOOPED, ALU, X) \
DGI_FOR_EACH_D1(M, LOOPED, ALU, X, 0x0) \
 DGI_FOR_EACH_D1(M, LOOPED, ALU, X, 0x1) \
 DGI_FOR_EACH_D1(M, LOOPED, ALU, X, 0x2) \
 DGI_FOR_EACH_D1(M, LOOPED, ALU, X, 0x3) \
 DGI_FOR_EACH_D1(M, LOOPED, ALU, X, 0x4) \
 DGI_FOR_EACH_D1(M, LOOPED, ALU, X, 0x5) \
 DGI_FOR_EACH_D1(M, LOOPED, ALU, X, 0x6) \
 DGI_FOR_EACH_D1(M, LOOPED, ALU, X, 0x7)

#define DGI_FOR_EACH_X(M, LOOPED, ALU) \
DGI_FOR_EACH_Y(M, LOOPED, ALU, 0x0) \
 DGI_FOR_EACH_Y(M, LOOPED, ALU, 0x2) \
 DGI_FOR_EACH_Y(M, LOOPED, ALU, 0x3) \
 DGI_FOR_EACH_Y(M, LOOPED, ALU, 0x4) \
 DGI_FOR_EACH_Y(M, LOOPED, ALU, 0x5) \
 DGI_FOR_EACH_Y(M, LOOPED, ALU, 0x6) \
 DGI_FOR_EACH_Y(M, LOOPED, ALU, 0x7)

#define DGI_FOR_EACH_ALU(M, LOOPED) \
DGI_FOR_EACH_X(M, LOOPED, 0x0) \
 DGI_FOR_EACH_X(M, LOOPED, 0x1) \
 DGI_FOR_EACH_X(M, LOOPED, 0x2) \
 DGI_FOR_EACH_X(M, LOOPED, 0x3) \
 DGI_FOR_EACH_X(M, LOOPED, 0x4) \
 DGI_FOR_EACH_X(M, LOOPED, 0x5) \
 DGI_FOR_EACH_X(M, LOOPED, 0x6) \
 DGI_FOR_EACH_X(M, LOOPED, 0x8) \
 DGI_FOR_EACH_X(M, LOOPED, 0x9) \
 DGI_FOR_EACH_X(M, LOOPED, 0xa) \
 DGI_FOR_EACH_X(M, LOOPED, 0xb) \
 DGI_FOR_EACH_X(M, LOOPED, 0xf)

DGI_FOR_EACH_ALU(DEFINE_GeneralInstr, 0)
DGI_FOR_EACH_ALU(DEFINE_GeneralInstr, 1)

#undef DEFINE_GeneralInstr
#undef DGI_FOR_EACH_ALU
#undef DGI_FOR_EACH_X
#undef DGI_FOR_EACH_Y
#undef DGI_FOR_EACH_D1

MDFN_HIDE extern void (*const DSP_GenFuncTable[2][16][8][8][4])(struct DSPS*) =
{
 #include "scu_dsp_gentab.inc"
};

#undef GeneralInstr_NAME
#undef GeneralInstr_BODY
