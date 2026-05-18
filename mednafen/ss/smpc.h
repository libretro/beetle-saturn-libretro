/******************************************************************************/
/* Mednafen Sega Saturn Emulation Module                                      */
/******************************************************************************/
/* smpc.h:
**  Copyright (C) 2015-2017 Mednafen Team
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

#include <time.h>

#ifndef __MDFN_SS_SMPC_H
#define __MDFN_SS_SMPC_H

#include "../state.h"
/* MDFN_COLD / MDFN_HOT attribute macros.  Existing TUs got
 * these transitively (via ss.h / mednafen.h); for C consumers the
 * header needs to be self-contained. */
#include "../mednafen-types.h"

#include <stdint.h>
#include <boolean.h>

#include "../cdstream.h"

/* EmulateSpecStruct lives in git.h, which is no longer used (it uses
 * std::vector / std::string).  smpc.h only uses the type as a
 * pointer parameter (SMPC_EndFrame), so a forward declaration is
 * sufficient and lets the header parse from C. */
struct EmulateSpecStruct;

#ifdef __cplusplus
extern "C" {
#endif

enum
{
 SMPC_AREA_JP		= 0x1,
 SMPC_AREA_ASIA_NTSC	= 0x2,
 SMPC_AREA_NA		= 0x4,
 SMPC_AREA_CSA_NTSC	= 0x5,
 SMPC_AREA_KR		= 0x6,

 SMPC_AREA_ASIA_PAL	= 0xA,
 SMPC_AREA_EU_PAL	= 0xC,
 SMPC_AREA_CSA_PAL	= 0xD,
 //
 //
 //
 SMPC_AREA__PAL_MASK	= 0x8
};

enum
{
 SMPC_RTC_LANG_ENGLISH = 0,
 SMPC_RTC_LANG_GERMAN = 1,
 SMPC_RTC_LANG_FRENCH = 2,
 SMPC_RTC_LANG_SPANISH = 3,
 SMPC_RTC_LANG_ITALIAN = 4,
 SMPC_RTC_LANG_JAPANESE = 5,
};

#ifdef __cplusplus
/* C++ form keeps the default arg as a caller-side convenience. */
void SMPC_Init(const uint8_t area_code, const int32_t master_clock, bool block_soundcpu_control = false) MDFN_COLD;
#else
/* C form: default arguments are no longer used syntax; C callers must
 * pass block_soundcpu_control explicitly. */
void SMPC_Init(const uint8_t area_code, const int32_t master_clock, bool block_soundcpu_control) MDFN_COLD;
#endif
void SMPC_Kill(void) MDFN_COLD;
void SMPC_Reset(bool powering_up) MDFN_COLD;
void SMPC_LoadNV(cdstream* s) MDFN_COLD;
void SMPC_SaveNV(cdstream* s) MDFN_COLD;
void SMPC_StateAction(StateMem* sm, const unsigned load, const bool data_only) MDFN_COLD;

void SMPC_SetRTC(const struct tm* ht, const uint8_t lang) MDFN_COLD;

/* int32_t in place of sscpu_timestamp_t (typedef'd to int32_t in
 * ss.h) -- keeps this header self-contained for C consumers and
 * matches the C-ABI convention used by vdp1.c / smpc_iodevice.h /
 * stvio.c / sound.h. */
void SMPC_Write(const int32_t timestamp, uint8_t A, uint8_t V) MDFN_HOT;
uint8_t SMPC_Read(const int32_t timestamp, uint8_t A) MDFN_HOT;

int32_t SMPC_Update(int32_t timestamp);
void SMPC_ResetTS(void);

void SMPC_ProcessSlaveOffOn(void);
int32_t SMPC_StartFrame(void);
void SMPC_EndFrame(struct EmulateSpecStruct* espec, int32_t timestamp);
void SMPC_TransformInput(void);
void SMPC_UpdateInput(const int32_t time_elapsed);
void SMPC_UpdateOutput(void);
void SMPC_SetInput(unsigned port, const char* type, uint8_t* ptr) MDFN_COLD;
void SMPC_SetMultitap(unsigned sport, bool enabled) MDFN_COLD;
void SMPC_SetCrosshairsColor(unsigned port, uint32_t color) MDFN_COLD;

void SMPC_SetVBVS(int32_t event_timestamp, bool vb_status, bool vsync_status);

void SMPC_LineHook(int32_t event_timestamp, int32_t out_line, int32_t div, int32_t coord_adj);

#ifdef __cplusplus
}
#endif

#endif
