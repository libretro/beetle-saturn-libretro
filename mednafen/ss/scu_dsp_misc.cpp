/******************************************************************************/
/* Mednafen Sega Saturn Emulation Module                                      */
/******************************************************************************/
/* scu_dsp_misc.cpp - SCU DSP Miscellaneous Instructions Emulation
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

template<bool looped, unsigned op>
static NO_INLINE NO_CLONE void MiscInstr(DSPS* dsp)
{
 DSP_InstrPre<looped>(dsp);

 //
 // END/ENDI
 //
 if(op == 2 || op == 3)
 {
  if(op & 0x1)
  {
   dsp->FlagEnd = true;
   SCU_SetInt(SCU_INT_DSP, true);
  }

  if(dsp->PRAMDMABufCount)
   DSP_FinishPRAMDMA();
  else
  {
   dsp->State &= ~DSPS::STATE_MASK_EXECUTE;
   dsp->CycleCounter -= DSP_EndCCSubVal;	// Break out of execution loop(also remember to handle this case for manual stepping via port writes).
  }
 }
 else if(op == 0)	// BTM
 {
  if(dsp->LOP)
   dsp->PC = dsp->TOP;

  dsp->LOP = (dsp->LOP - 1) & 0x0FFF;
 }
 else if(op == 1)	// LPS
 {
  dsp->NextInstr = DSP_DecodeInstruction<true>(dsp->NextInstr >> 32);
 }
}

MDFN_HIDE extern void (*const DSP_MiscFuncTable[2][4])(DSPS*) =
{
 #include "scu_dsp_misctab.inc"
};

