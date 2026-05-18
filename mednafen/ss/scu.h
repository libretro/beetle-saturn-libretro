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

#include <stdint.h>
#include <boolean.h>
#include "../mednafen-types.h"     /* MDFN_COLD attribute macro */
#include "../state.h"              /* StateMem typedef for SCU_StateAction */

/* SCU interrupt vectors.  Order matters -- savestate-visible. */
enum
{
 SCU_INT_VBIN = 0x00,
 SCU_INT_VBOUT,
 SCU_INT_HBIN,
 SCU_INT_TIMER0,
 SCU_INT_TIMER1,
 SCU_INT_DSP,
 SCU_INT_SCSP,
 SCU_INT_SMPC,
 SCU_INT_PAD,

 SCU_INT_L2DMA,
 SCU_INT_L1DMA,
 SCU_INT_L0DMA,

 SCU_INT_DMA_ILL,

 SCU_INT_VDP1,

 SCU_INT_EXT0 = 0x10,
 SCU_INT_EXTF = 0x1F
};

#ifdef __cplusplus
extern "C" {
#endif

void SCU_Reset(bool powering_up) MDFN_COLD;

void SCU_SetInt(unsigned which, bool active);
int32_t SCU_SetHBVB(int32_t pclocks, bool hblank_in, bool vblank_in);
bool SCU_CheckVDP1HaltKludge(void);

/* int32_t in place of sscpu_timestamp_t (typedef'd to int32_t in
 * ss.h) -- keeps the header self-contained for C consumers and
 * matches the C-ABI convention used by vdp1.c / sound.h / smpc.h. */
int32_t SCU_UpdateDMA(int32_t timestamp);
int32_t SCU_UpdateDSP(int32_t timestamp);

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

/* Phase-7d: promoted from file-static so ss_state.c's
 * LibRetro_StateAction can call it across the C / C++ boundary. */
void SCU_StateAction(StateMem* sm, const unsigned load, const bool data_only) MDFN_COLD;

/* Phase-7e: promoted from file-static so ss_init.c's Emulate
 * can call it across the C / C++ boundary. */
void SCU_AdjustTS(const int32_t delta);

/* Phase-7f: promoted from file-static so ss_init.c's InitCommon
 * can call it across the C / C++ boundary. */
void SCU_Init(void) MDFN_COLD;

#ifdef __cplusplus
}
#endif

#endif
