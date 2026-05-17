/******************************************************************************/
/* Mednafen Sega Saturn Emulation Module                                      */
/******************************************************************************/
/* vdp2.h:
**  Copyright (C) 2015-2019 Mednafen Team
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

#ifndef __MDFN_SS_VDP2_H
#define __MDFN_SS_VDP2_H

#include "../state.h"
#include "ss.h"				/* sscpu_timestamp_t, events[], SS_SetEventNT, SS_EVENT_VDP2 */

/* MDFNGI / EmulateSpecStruct: forward-declared rather than pulling
 * in the C++-only git.h.  This header is now C-compat (the
 * VDP2 subsystem was converted from C++ to C; the `namespace VDP2
 * { ... }` wrap was replaced with VDP2_-prefixed free functions
 * under an extern "C" wrap).  vdp2_render.cpp / ss.cpp stay C++
 * for now; smpc.c / vdp1.c / libretro.c are C and now consume
 * vdp2.h directly instead of going through extern "C" proxies. */
struct MDFNGI;
struct EmulateSpecStruct;

#ifdef __cplusplus
extern "C" {
#endif

uint32_t VDP2_Write8_DB(uint32_t A, uint16_t DB) MDFN_HOT;
uint32_t VDP2_Write16_DB(uint32_t A, uint16_t DB) MDFN_HOT;
/* DSP-DMA burst of n16 16-bit writes into the VDP2 window:
 * words[i] -> (base + i*((1<<add_mode)&~1)).  Performs the SH2-side
 * array writes here and queues a single renderer burst; returns the
 * summed VRAM access penalty. */
uint32_t VDP2_Write16Burst_DB(uint32_t base, uint32_t n16, uint32_t add_mode, const uint16_t* words) MDFN_HOT;
uint16_t VDP2_Read16_DB(uint32_t A) MDFN_HOT;

void VDP2_Init(const bool IsPAL, const uint64_t affinity) MDFN_COLD;
void VDP2_SetGetVideoParams(struct MDFNGI* gi, const bool caspect, const int sls, const int sle, const bool show_h_overscan, const bool dohblend) MDFN_COLD;
void VDP2_Kill(void) MDFN_COLD;
void VDP2_StateAction(StateMem* sm, const unsigned load, const bool data_only) MDFN_COLD;

void VDP2_Reset(bool powering_up) MDFN_COLD;
void VDP2_SetLayerEnableMask(uint64_t mask) MDFN_COLD;
void VDP2_SetDeinterlaceOff(bool off) MDFN_COLD;

sscpu_timestamp_t VDP2_Update(sscpu_timestamp_t timestamp);
void VDP2_AdjustTS(const int32_t delta);

void VDP2_GetGunXTranslation(const bool clock28m, float* scale, float* offs);
void VDP2_StartFrame(struct EmulateSpecStruct* espec, const bool clock28m);

/* MDFN_HIDE extern globals defined in vdp2.c, accessed via the
 * INLINE accessors below from C++ callers (ss.cpp's CRT-line and
 * VBlank handling).  Kept inline so the optimizer can fold the
 * cross-TU read into the caller. */
MDFN_HIDE extern bool VBOut;
MDFN_HIDE extern bool HBOut;
MDFN_HIDE extern bool ExLatchIn;
MDFN_HIDE extern bool ExLatchEnable;
MDFN_HIDE extern bool ExLatchPending;
MDFN_HIDE extern int32_t VCounter;
MDFN_HIDE extern int32_t HCounter;

static INLINE bool VDP2_GetVBOut(void) { return VBOut; }
static INLINE bool VDP2_GetHBOut(void) { return HBOut; }

static INLINE void VDP2_SetExtLatch(sscpu_timestamp_t event_timestamp, bool status)
{
 if(MDFN_UNLIKELY(ExLatchIn != status))
 {
  ExLatchIn = status;
  //
  //
  if(ExLatchEnable & ExLatchIn)
  {
   /*
    * Should be safer (avoid unintended reentrant and recursive
    * calls to *Update() functions, now and in the future) to just
    * schedule a call to VDP2_Update() than calling it directly
    * from here, though it's possible a scheduled DMA could
    * rewrite ExLatchEnable and VDP2 timing registers and cause
    * weird results (latching correct values or not latching at
    * all, versus also latching wrong values), but that shouldn't
    * be a problem in practice...
    */
   ExLatchPending = true;
   SS_SetEventNT(&events[SS_EVENT_VDP2], event_timestamp);
  }
 }
}

/*
 *
 */
enum
{
 GSREG_LINE = 0,
 GSREG_DON,
 GSREG_BM,
 GSREG_IM,
 GSREG_VRES,
 GSREG_HRES,

 GSREG_RAMCTL,

 GSREG_BGON,
 GSREG_MZCTL,
 GSREG_SFSEL,
 GSREG_SFCODE,
 GSREG_CHCTLA,
 GSREG_CHCTLB,

 GSREG_SCXIN0,
 GSREG_SCXDN0,
 GSREG_SCYIN0,
 GSREG_SCYDN0,
 GSREG_ZMXIN0,
 GSREG_ZMXDN0,
 GSREG_ZMYIN0,
 GSREG_ZMYDN0,

 GSREG_SCXIN1,
 GSREG_SCXDN1,
 GSREG_SCYIN1,
 GSREG_SCYDN1,
 GSREG_ZMXIN1,
 GSREG_ZMXDN1,
 GSREG_ZMYIN1,
 GSREG_ZMYDN1,

 GSREG_SCXN2,
 GSREG_SCYN2,
 GSREG_SCXN3,
 GSREG_SCYN3,

 GSREG_ZMCTL,
 GSREG_SCRCTL,

 GSREG_CYCA0,
 GSREG_CYCA1 = GSREG_CYCA0 + 1,
 GSREG_CYCB0 = GSREG_CYCA0 + 2,
 GSREG_CYCB1 = GSREG_CYCA0 + 3,
 GSREG_RPMD,
 GSREG_RPRCTL,
 GSREG_KTCTL,
 GSREG_KTAOF,
 GSREG_OVPNRA,
 GSREG_OVPNRB,
 GSREG_RPTA,

 GSREG_PRINA,
 GSREG_PRINB,
 GSREG_PRIR
};

uint32_t VDP2_GetRegister(const unsigned id, char* const special, const uint32_t special_len) MDFN_COLD;
void VDP2_SetRegister(const unsigned id, const uint32_t value) MDFN_COLD;
uint8_t VDP2_PeekVRAM(uint32_t addr) MDFN_COLD;
void VDP2_PokeVRAM(uint32_t addr, const uint8_t val) MDFN_COLD;

static INLINE uint32_t VDP2_PeekLine(void) { return VCounter; }
static INLINE uint32_t VDP2_PeekHPos(void) { return HCounter; }

#ifdef __cplusplus
}
#endif

#endif
