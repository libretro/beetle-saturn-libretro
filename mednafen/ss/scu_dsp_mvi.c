/******************************************************************************/
/* Mednafen Sega Saturn Emulation Module                                      */
/******************************************************************************/
/* scu_dsp_mvi.c - SCU DSP MVI Instructions Emulation
**  Copyright (C) 2015-2018 Mednafen Team
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

#include "scu_dsp_common.inc"

/* was `template<bool looped, unsigned dest, unsigned cond>
 * static NO_INLINE NO_CLONE void MVIInstr(DSPS* dsp)` -- 2x16x128 in
 * the function-pointer table, 2x16x65 = 2080 unique instantiations
 * (the cond&0x40==0 short-circuit collapses cond 0x00..0x3F to the
 * 0x00 representative per (looped, dest) like the JMP table).
 *
 * Monomorphized here via X-macro into 2080 named functions
 * (MVIInstr_<LOOPED>_<DEST>_<COND>), same X-macro pattern as the
 * phase 5b JMPInstr conversion -- extended by one dimension (DEST in
 * 0x0..0xf).  Body macro is identical to the template body.
 *
 * LOOPED, DEST, COND become literal constants at expansion; the
 * `if(cond & 0x40)`, `if(DSP_TestCond(...))`, and `switch(dest)`
 * branches fold per-spec and the helper calls inline from the
 * phase-5a FORCE_INLINE forms -- codegen byte-matches the pre-
 * template form. */

#define MVIInstr_BODY(LOOPED, DEST, COND)                                                    \
{                                                                                            \
 const uint32_t instr = DSP_InstrPre(dsp, LOOPED);                                           \
 uint32_t imm;                                                                               \
                                                                                             \
 if((COND) & 0x40)                                                                           \
  imm = sign_x_to_s32(19, instr);                                                            \
 else                                                                                        \
  imm = sign_x_to_s32(25, instr);                                                            \
                                                                                             \
 if(DSP_TestCond(dsp, COND))                                                                 \
 {                                                                                           \
  if(dsp->PRAMDMABufCount && ((DEST) == 0x6 || (DEST) == 0x7))                               \
  {                                                                                          \
   dsp->PC--;                                                                                \
   /* */                                                                                     \
   DSP_FinishPRAMDMA();                                                                      \
  }                                                                                          \
                                                                                             \
  switch(DEST)                                                                               \
  {                                                                                          \
   default:                                                                                  \
	break;                                                                                     \
                                                                                             \
   case 0x0:                                                                                 \
   case 0x1:                                                                                 \
   case 0x2:                                                                                 \
   case 0x3:                                                                                 \
	dsp->DataRAM[DEST][dsp->CT[DEST]] = imm;                                                   \
	dsp->CT[DEST] = (dsp->CT[DEST] + 1) & 0x3F;                                                \
	break;                                                                                     \
                                                                                             \
   case 0x4: dsp->RX = imm; break;                                                           \
   case 0x5: dsp->P.T = (int32_t)imm; break;                                                 \
                                                                                             \
   case 0x6: dsp->RAO = imm; break;                                                          \
                                                                                             \
   case 0x7: dsp->WAO = imm; break;                                                          \
                                                                                             \
   case 0xA: if(!(LOOPED) || dsp->LOP == 0x0FFF) { dsp->LOP = imm & 0x0FFF; } break;         \
                                                                                             \
   case 0xC:                                                                                 \
	dsp->TOP = dsp->PC - 1;                                                                    \
	dsp->PC = imm & 0xFF;                                                                      \
        /* */                                                                                \
	if(dsp->PRAMDMABufCount)                                                                   \
	 DSP_FinishPRAMDMA();                                                                      \
	break;                                                                                     \
  }                                                                                          \
 }                                                                                           \
                                                                                             \
 DSP_TailDispatch(dsp);                                                                      \
}

#define MVIInstr_NAME(LOOPED, DEST, COND) MVIInstr_##LOOPED##_##DEST##_##COND

#define DEFINE_MVIInstr(LOOPED, DEST, COND)                                                  \
 static NO_INLINE NO_CLONE void MVIInstr_NAME(LOOPED, DEST, COND)(struct DSPS* dsp)          \
 MVIInstr_BODY(LOOPED, DEST, COND)

#define DMVI_FOR_EACH_COND(M, LOOPED, DEST) \
 M(LOOPED, DEST, 0x00) \
 M(LOOPED, DEST, 0x40) \
 M(LOOPED, DEST, 0x41) \
 M(LOOPED, DEST, 0x42) \
 M(LOOPED, DEST, 0x43) \
 M(LOOPED, DEST, 0x44) \
 M(LOOPED, DEST, 0x45) \
 M(LOOPED, DEST, 0x46) \
 M(LOOPED, DEST, 0x47) \
 M(LOOPED, DEST, 0x48) \
 M(LOOPED, DEST, 0x49) \
 M(LOOPED, DEST, 0x4a) \
 M(LOOPED, DEST, 0x4b) \
 M(LOOPED, DEST, 0x4c) \
 M(LOOPED, DEST, 0x4d) \
 M(LOOPED, DEST, 0x4e) \
 M(LOOPED, DEST, 0x4f) \
 M(LOOPED, DEST, 0x50) \
 M(LOOPED, DEST, 0x51) \
 M(LOOPED, DEST, 0x52) \
 M(LOOPED, DEST, 0x53) \
 M(LOOPED, DEST, 0x54) \
 M(LOOPED, DEST, 0x55) \
 M(LOOPED, DEST, 0x56) \
 M(LOOPED, DEST, 0x57) \
 M(LOOPED, DEST, 0x58) \
 M(LOOPED, DEST, 0x59) \
 M(LOOPED, DEST, 0x5a) \
 M(LOOPED, DEST, 0x5b) \
 M(LOOPED, DEST, 0x5c) \
 M(LOOPED, DEST, 0x5d) \
 M(LOOPED, DEST, 0x5e) \
 M(LOOPED, DEST, 0x5f) \
 M(LOOPED, DEST, 0x60) \
 M(LOOPED, DEST, 0x61) \
 M(LOOPED, DEST, 0x62) \
 M(LOOPED, DEST, 0x63) \
 M(LOOPED, DEST, 0x64) \
 M(LOOPED, DEST, 0x65) \
 M(LOOPED, DEST, 0x66) \
 M(LOOPED, DEST, 0x67) \
 M(LOOPED, DEST, 0x68) \
 M(LOOPED, DEST, 0x69) \
 M(LOOPED, DEST, 0x6a) \
 M(LOOPED, DEST, 0x6b) \
 M(LOOPED, DEST, 0x6c) \
 M(LOOPED, DEST, 0x6d) \
 M(LOOPED, DEST, 0x6e) \
 M(LOOPED, DEST, 0x6f) \
 M(LOOPED, DEST, 0x70) \
 M(LOOPED, DEST, 0x71) \
 M(LOOPED, DEST, 0x72) \
 M(LOOPED, DEST, 0x73) \
 M(LOOPED, DEST, 0x74) \
 M(LOOPED, DEST, 0x75) \
 M(LOOPED, DEST, 0x76) \
 M(LOOPED, DEST, 0x77) \
 M(LOOPED, DEST, 0x78) \
 M(LOOPED, DEST, 0x79) \
 M(LOOPED, DEST, 0x7a) \
 M(LOOPED, DEST, 0x7b) \
 M(LOOPED, DEST, 0x7c) \
 M(LOOPED, DEST, 0x7d) \
 M(LOOPED, DEST, 0x7e) \
 M(LOOPED, DEST, 0x7f)

#define DMVI_FOR_EACH_DEST(M, LOOPED) \
 DMVI_FOR_EACH_COND(M, LOOPED, 0x0) \
 DMVI_FOR_EACH_COND(M, LOOPED, 0x1) \
 DMVI_FOR_EACH_COND(M, LOOPED, 0x2) \
 DMVI_FOR_EACH_COND(M, LOOPED, 0x3) \
 DMVI_FOR_EACH_COND(M, LOOPED, 0x4) \
 DMVI_FOR_EACH_COND(M, LOOPED, 0x5) \
 DMVI_FOR_EACH_COND(M, LOOPED, 0x6) \
 DMVI_FOR_EACH_COND(M, LOOPED, 0x7) \
 DMVI_FOR_EACH_COND(M, LOOPED, 0x8) \
 DMVI_FOR_EACH_COND(M, LOOPED, 0x9) \
 DMVI_FOR_EACH_COND(M, LOOPED, 0xa) \
 DMVI_FOR_EACH_COND(M, LOOPED, 0xb) \
 DMVI_FOR_EACH_COND(M, LOOPED, 0xc) \
 DMVI_FOR_EACH_COND(M, LOOPED, 0xd) \
 DMVI_FOR_EACH_COND(M, LOOPED, 0xe) \
 DMVI_FOR_EACH_COND(M, LOOPED, 0xf)

DMVI_FOR_EACH_DEST(DEFINE_MVIInstr, 0)
DMVI_FOR_EACH_DEST(DEFINE_MVIInstr, 1)

#undef DEFINE_MVIInstr
#undef DMVI_FOR_EACH_DEST
#undef DMVI_FOR_EACH_COND

MDFN_HIDE void (*const DSP_MVIFuncTable[2][16][128])(struct DSPS*) =
{
 #include "scu_dsp_mvitab.inc"
};

#undef MVIInstr_NAME
#undef MVIInstr_BODY
