/******************************************************************************/
/* Mednafen Sega Saturn Emulation Module                                      */
/******************************************************************************/
/* vdp1.h:
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

#ifndef __MDFN_SS_VDP1_H
#define __MDFN_SS_VDP1_H

#include <stdint.h>
#ifndef __cplusplus
#include <stdbool.h>
#endif

#include <mednafen/mednafen-types.h>
#include <retro_inline.h>
#include <mednafen/state.h>

/* Formerly `namespace VDP1`. Converted to C: the namespace is removed
   and every exported symbol gets a VDP1_ prefix. sscpu_timestamp_t
   is defined in the C++-only ss.h as `typedef int32_t
   sscpu_timestamp_t;` -- mirror that here rather than pulling in
   ss.h. */
#ifndef SS_SSCPU_TIMESTAMP_T_DEFINED
#define SS_SSCPU_TIMESTAMP_T_DEFINED
typedef int32_t sscpu_timestamp_t;
#endif

#ifdef __cplusplus
extern "C" {
#endif

void VDP1_Init(void) MDFN_COLD;
void VDP1_Kill(void) MDFN_COLD;
void VDP1_StateAction(StateMem* sm, const unsigned load, const bool data_only) MDFN_COLD;

void VDP1_Reset(bool powering_up) MDFN_COLD;

sscpu_timestamp_t VDP1_Update(sscpu_timestamp_t timestamp);
void VDP1_AdjustTS(const int32_t delta);

MDFN_FASTCALL void VDP1_Write_CheckDrawSlowdown(uint32_t A, sscpu_timestamp_t time_thing) MDFN_HOT;
MDFN_FASTCALL void VDP1_Read_CheckDrawSlowdown(uint32_t A, sscpu_timestamp_t time_thing) MDFN_HOT;
MDFN_FASTCALL void VDP1_Write8_DB(uint32_t A, uint16_t DB) MDFN_HOT;
MDFN_FASTCALL void VDP1_Write16_DB(uint32_t A, uint16_t DB) MDFN_HOT;
MDFN_FASTCALL uint16_t VDP1_Read16_DB(uint32_t A) MDFN_HOT;

void VDP1_SetHBVB(const sscpu_timestamp_t event_timestamp, const bool new_hb_status, const bool new_vb_status);

bool VDP1_GetLine(const int line, uint16_t* buf, uint16_t* mesh_buf, unsigned w, uint32_t rot_x, uint32_t rot_y, uint32_t rot_xinc, uint32_t rot_yinc);

/* Toggle the "improved mesh transparency" mode for VDP1 mesh-bit
   primitives (mode bit 8 / MSH). When false (default), mesh
   primitives use the hardware-accurate (x ^ y) & 1 stipple, which
   looks like a visible checker pattern on a flat-panel display.
   When true, mesh primitives instead get routed to a parallel side-
   buffer (MeshFB) and VDP2's MixIt path blends them 50% over the
   final composited surface -- a CPU port of Kronos's GL "improved
   mesh" mechanism (outMeshSurface side-buffer + composite-time
   blend).

   Setter is called from the libretro option-update path on the
   emulator main thread; the same thread runs SH-2 / VDP1
   rasterisation, so no synchronisation is needed. */
void VDP1_SetMeshImproved(bool improved) MDFN_COLD;

/* "Improved mesh transparency" toggle storage (libretro core
   option). Defined in vdp1.c; read directly by vdp2_render. */
MDFN_HIDE extern bool VDP1_MeshImproved;

/*
**
**
*/

MDFN_HIDE extern uint16_t VDP1_VRAM[0x40000];
MDFN_HIDE extern uint16_t VDP1_FB[2][0x20000];

static INLINE uint8_t VDP1_PeekVRAM(const uint32_t addr)
{
 /* ne16_rbo_be<uint8_t> folded: byte read from uint16_t-array BE bus.
  * MSB_FIRST: natural byte index. LE host: XOR with 1 to swap
  * the byte halves of each uint16_t. */
#ifdef MSB_FIRST
 return ((const uint8_t*)VDP1_VRAM)[addr & 0x7FFFF];
#else
 return ((const uint8_t*)VDP1_VRAM)[(addr & 0x7FFFF) ^ 1];
#endif
}

static INLINE void VDP1_PokeVRAM(const uint32_t addr, const uint8_t val)
{
#ifdef MSB_FIRST
 ((uint8_t*)VDP1_VRAM)[addr & 0x7FFFF] = val;
#else
 ((uint8_t*)VDP1_VRAM)[(addr & 0x7FFFF) ^ 1] = val;
#endif
}

static INLINE uint8_t VDP1_PeekFB(const bool which, const uint32_t addr)
{
#ifdef MSB_FIRST
 return ((const uint8_t*)VDP1_FB[which])[addr & 0x3FFFF];
#else
 return ((const uint8_t*)VDP1_FB[which])[(addr & 0x3FFFF) ^ 1];
#endif
}

static INLINE void VDP1_PokeFB(const bool which, const uint32_t addr, const uint8_t val)
{
#ifdef MSB_FIRST
 ((uint8_t*)VDP1_FB[which])[addr & 0x3FFFF] = val;
#else
 ((uint8_t*)VDP1_FB[which])[(addr & 0x3FFFF) ^ 1] = val;
#endif
}

enum
{
 VDP1_GSREG_SYSCLIPX = 0,
 VDP1_GSREG_SYSCLIPY,
 VDP1_GSREG_USERCLIPX0,
 VDP1_GSREG_USERCLIPY0,
 VDP1_GSREG_USERCLIPX1,
 VDP1_GSREG_USERCLIPY1,
 VDP1_GSREG_LOCALX,
 VDP1_GSREG_LOCALY,

 VDP1_GSREG_TVMR,
 VDP1_GSREG_FBCR,
 VDP1_GSREG_EWDR,
 VDP1_GSREG_EWLR,
 VDP1_GSREG_EWRR
};
uint32_t VDP1_GetRegister(const unsigned id, char* const special, const uint32_t special_len) MDFN_COLD;
void VDP1_SetRegister(const unsigned id, const uint32_t value) MDFN_COLD;

#ifdef __cplusplus
}
#endif

#endif
