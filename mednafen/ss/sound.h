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

#include <mednafen/state.h>
/* MDFN_COLD / MDFN_HOT attribute macros.  Existing C++ TUs got
 * these transitively (via ss.h / mednafen.h); for C consumers
 * include them explicitly so this header is self-contained. */
#include <mednafen/mednafen-types.h>

#include <stdint.h>
/* C++ has 'bool' built in; C inclusion (future C-converted modules)
 * needs the stdbool keyword macros. */
#ifndef __cplusplus
#include <stdbool.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

extern int16_t IBuffer[1024][2];

void SOUND_Init(void) MDFN_COLD;
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

uint8_t SOUND_PeekRAM(uint32_t A);
void SOUND_PokeRAM(uint32_t A, uint8_t V);

uint32_t SOUND_GetSCSPRegister(const unsigned id, char* const special, const uint32_t special_len) MDFN_COLD;
void SOUND_SetSCSPRegister(const unsigned id, const uint32_t value) MDFN_COLD;
uint32_t SOUND_GetM68KRegister(const unsigned id, char* const special, const uint32_t special_len) MDFN_COLD;
void SOUND_SetM68KRegister(const unsigned id, const uint32_t value) MDFN_COLD;

#ifdef __cplusplus
}
#endif

#endif
