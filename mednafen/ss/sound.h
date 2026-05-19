/******************************************************************************/
/* Mednafen Sega Saturn Emulation Module                                      */
/******************************************************************************/
/* sound.h:
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

#ifndef __MDFN_SS_SOUND_H
#define __MDFN_SS_SOUND_H

#include "../state.h"
/* MDFN_COLD / MDFN_HOT attribute macros.  Existing TUs got
 * these transitively (via ss.h / mednafen.h); for C consumers
 * include them explicitly so this header is self-contained. */
#include "../mednafen-types.h"

#include <stdint.h>
#include <boolean.h>

#ifdef __cplusplus
extern "C" {
#endif

extern int16_t IBuffer[1024][2];

void SOUND_Init(bool stv_mapping) MDFN_COLD;
void SOUND_Reset(bool powering_up) MDFN_COLD;
void SOUND_Kill(void) MDFN_COLD;

void SOUND_Set68KActive(bool active);
void SOUND_Reset68K(void);
void SOUND_ResetSCSP(void);

void SOUND_SetClockRatio(uint32_t ratio); // Ratio between SH-2 clock and 68K clock (sound clock / 2)
/* int32_t in place of sscpu_timestamp_t (which typedefs to int32_t
 * in ss.h) -- keeps this header self-contained for C consumers and
 * matches the C-ABI convention used by vdp1.c and smpc_iodevice.h. */
int32_t SOUND_Update(int32_t timestamp);
void SOUND_AdjustTS(const int32_t delta);
void SOUND_StateAction(StateMem *sm, const unsigned load, const bool data_only);

uint16_t SOUND_Read16(uint32_t A);
void SOUND_Write8(uint32_t A, uint8_t V);
void SOUND_Write16(uint32_t A, uint16_t V);

#ifdef __cplusplus
}
#endif

#endif
