/******************************************************************************/
/* Mednafen Sega Saturn Emulation Module                                      */
/******************************************************************************/
/* scu.h:
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

#ifndef __MDFN_SS_SCU_H
#define __MDFN_SS_SCU_H

/* SCU_INT_* enum lives in ss_c_abi.h (shared verbatim with the C-converted
   modules). Single source of truth -- see that header. */
#include "ss_c_abi.h"

void SCU_Reset(bool powering_up) MDFN_COLD;

/* extern "C": called from vdp1.c (converted to C). */
#ifdef __cplusplus
extern "C" {
#endif
void SCU_SetInt(unsigned which, bool active);
#ifdef __cplusplus
}
#endif
int32_t SCU_SetHBVB(int32_t pclocks, bool hblank_in, bool vblank_in);

#ifdef __cplusplus
extern "C" {
#endif
bool SCU_CheckVDP1HaltKludge(void);
#ifdef __cplusplus
}
#endif

sscpu_timestamp_t SCU_UpdateDMA(sscpu_timestamp_t timestamp);
sscpu_timestamp_t SCU_UpdateDSP(sscpu_timestamp_t timestamp);

enum
{
 SCU_GSREG_ILEVEL = 0,
 SCU_GSREG_IVEC,
 SCU_GSREG_ICLEARMASK,

 SCU_GSREG_IASSERTED,
 SCU_GSREG_IPENDING,
 SCU_GSREG_IMASK,

 SCU_GSREG_D0MD,
 SCU_GSREG_D1MD,
 SCU_GSREG_D2MD,

 SCU_GSREG_ASR0_CS0,
 SCU_GSREG_ASR0_CS1,
 SCU_GSREG_ASR1_CS2,
 SCU_GSREG_ASR1_CSD,

 SCU_GSREG_AREF,

 SCU_GSREG_RSEL,

 SCU_GSREG_T0CNT,
 SCU_GSREG_T0CMP,
 SCU_GSREG_T0MET,

 SCU_GSREG_T1RLV,
 SCU_GSREG_T1CNT,
 SCU_GSREG_T1MOD,
 SCU_GSREG_T1MET,

 SCU_GSREG_TENBL,
 //
 //
 //
 SCU_GSREG_DSP_EXEC,
 SCU_GSREG_DSP_PAUSE,
 SCU_GSREG_DSP_PC,
 SCU_GSREG_DSP_END,
};

uint32_t SCU_GetRegister(const unsigned id, char* const special, const uint32_t special_len) MDFN_COLD;
void SCU_SetRegister(const unsigned id, const uint32_t value) MDFN_COLD;

#endif
