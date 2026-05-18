/******************************************************************************/
/* Mednafen Sega Saturn Emulation Module                                      */
/******************************************************************************/
/* scu_dsp_misc.c - SCU DSP Miscellaneous Instructions Emulation
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

/* was `template<bool looped, unsigned op> static NO_INLINE
 * NO_CLONE void MiscInstr(DSPS* dsp)` -- 2 x 4 = 8 instantiations
 * (BTM / LPS / END / ENDI per looped value).  Monomorphized via
 * X-macro into 8 named functions following the same pattern as
 * phases 5b / 5c.  STATE_MASK_EXECUTE accessed bare per the
 * phase-5d hoist out of struct DSPS. */

#define MiscInstr_BODY(LOOPED, OP)                                                                 \
{                                                                                                  \
 DSP_InstrPre(dsp, LOOPED);                                                                        \
                                                                                                   \
 /* END/ENDI */                                                                                    \
 if((OP) == 2 || (OP) == 3)                                                                        \
 {                                                                                                 \
  if((OP) & 0x1)                                                                                   \
  {                                                                                                \
   dsp->FlagEnd = true;                                                                            \
   SCU_SetInt(SCU_INT_DSP, true);                                                                  \
  }                                                                                                \
                                                                                                   \
  if(dsp->PRAMDMABufCount)                                                                         \
   DSP_FinishPRAMDMA();                                                                            \
  else                                                                                             \
  {                                                                                                \
   dsp->State &= ~STATE_MASK_EXECUTE;                                                              \
   /* Break out of execution loop(also remember to handle this case for manual stepping            \
    * via port writes). */                                                                         \
   dsp->CycleCounter -= DSP_EndCCSubVal;                                                           \
  }                                                                                                \
 }                                                                                                 \
 else if((OP) == 0)	/* BTM */                                                                  \
 {                                                                                                 \
  if(dsp->LOP)                                                                                     \
   dsp->PC = dsp->TOP;                                                                             \
                                                                                                   \
  dsp->LOP = (dsp->LOP - 1) & 0x0FFF;                                                              \
 }                                                                                                 \
 else if((OP) == 1)	/* LPS */                                                                  \
 {                                                                                                 \
  dsp->NextInstr = DSP_DecodeInstruction(dsp->NextInstr >> 32, true);                              \
 }                                                                                                 \
                                                                                                   \
 DSP_TailDispatch(dsp);                                                                            \
}

#define MiscInstr_NAME(LOOPED, OP) MiscInstr_##LOOPED##_##OP

#define DEFINE_MiscInstr(LOOPED, OP)                                                               \
 static NO_INLINE NO_CLONE void MiscInstr_NAME(LOOPED, OP)(struct DSPS* dsp)                       \
 MiscInstr_BODY(LOOPED, OP)

#define DMI_FOR_EACH_OP(M, LOOPED) \
 M(LOOPED, 0) M(LOOPED, 1) M(LOOPED, 2) M(LOOPED, 3)

DMI_FOR_EACH_OP(DEFINE_MiscInstr, 0)
DMI_FOR_EACH_OP(DEFINE_MiscInstr, 1)

#undef DEFINE_MiscInstr
#undef DMI_FOR_EACH_OP

MDFN_HIDE void (*const DSP_MiscFuncTable[2][4])(struct DSPS*) =
{
 #include "scu_dsp_misctab.inc"
};

#undef MiscInstr_NAME
#undef MiscInstr_BODY
