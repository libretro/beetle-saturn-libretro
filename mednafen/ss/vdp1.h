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

#include <mednafen/state.h>


namespace VDP1
{

void Init(void) MDFN_COLD;
void Kill(void) MDFN_COLD;
void StateAction(StateMem* sm, const unsigned load, const bool data_only) MDFN_COLD;

void Reset(bool powering_up) MDFN_COLD;

sscpu_timestamp_t Update(sscpu_timestamp_t timestamp);
void AdjustTS(const int32_t delta);

MDFN_FASTCALL void Write_CheckDrawSlowdown(uint32_t A, sscpu_timestamp_t time_thing) MDFN_HOT;
MDFN_FASTCALL void Read_CheckDrawSlowdown(uint32_t A, sscpu_timestamp_t time_thing) MDFN_HOT;
MDFN_FASTCALL void Write8_DB(uint32_t A, uint16_t DB) MDFN_HOT;
MDFN_FASTCALL void Write16_DB(uint32_t A, uint16_t DB) MDFN_HOT;
MDFN_FASTCALL uint16_t Read16_DB(uint32_t A) MDFN_HOT;

void SetHBVB(const sscpu_timestamp_t event_timestamp, const bool new_hb_status, const bool new_vb_status);

bool GetLine(const int line, uint16_t* buf, uint16_t* mesh_buf, unsigned w, uint32_t rot_x, uint32_t rot_y, uint32_t rot_xinc, uint32_t rot_yinc);

// Toggle the "improved mesh transparency" mode for VDP1 mesh-bit
// primitives (mode bit 8 / MSH). When false (default), mesh
// primitives use the hardware-accurate (x ^ y) & 1 stipple, which
// looks like a visible checker pattern on a flat-panel display.
// When true, mesh primitives instead get routed to a parallel side-
// buffer (MeshFB) and VDP2's MixIt path blends them 50% over the
// final composited surface -- a CPU port of Kronos's GL "improved
// mesh" mechanism (outMeshSurface side-buffer + composite-time
// blend).
//
// Setter is called from the libretro option-update path on the
// emulator main thread; the same thread runs SH-2 / VDP1
// rasterisation, so no synchronisation is needed.
void SetMeshImproved(bool improved) MDFN_COLD;

//
//
//

INLINE uint8_t PeekVRAM(const uint32_t addr)
{
 MDFN_HIDE extern uint16_t VRAM[0x40000];
 /* ne16_rbo_be<uint8_t> folded: byte read from uint16_t-array BE bus.
  * MSB_FIRST: natural byte index. LE host: XOR with 1 to swap
  * the byte halves of each uint16_t. */
#ifdef MSB_FIRST
 return ((const uint8_t*)VRAM)[addr & 0x7FFFF];
#else
 return ((const uint8_t*)VRAM)[(addr & 0x7FFFF) ^ 1];
#endif
}

INLINE void PokeVRAM(const uint32_t addr, const uint8_t val)
{
 MDFN_HIDE extern uint16_t VRAM[0x40000];
#ifdef MSB_FIRST
 ((uint8_t*)VRAM)[addr & 0x7FFFF] = val;
#else
 ((uint8_t*)VRAM)[(addr & 0x7FFFF) ^ 1] = val;
#endif
}

INLINE uint8_t PeekFB(const bool which, const uint32_t addr)
{
 MDFN_HIDE extern uint16_t FB[2][0x20000];
#ifdef MSB_FIRST
 return ((const uint8_t*)FB[which])[addr & 0x3FFFF];
#else
 return ((const uint8_t*)FB[which])[(addr & 0x3FFFF) ^ 1];
#endif
}

INLINE void PokeFB(const bool which, const uint32_t addr, const uint8_t val)
{
 MDFN_HIDE extern uint16_t FB[2][0x20000];
#ifdef MSB_FIRST
 ((uint8_t*)FB[which])[addr & 0x3FFFF] = val;
#else
 ((uint8_t*)FB[which])[(addr & 0x3FFFF) ^ 1] = val;
#endif
}

enum
{
 GSREG_SYSCLIPX = 0,
 GSREG_SYSCLIPY,
 GSREG_USERCLIPX0,
 GSREG_USERCLIPY0,
 GSREG_USERCLIPX1,
 GSREG_USERCLIPY1,
 GSREG_LOCALX,
 GSREG_LOCALY,

 GSREG_TVMR,
 GSREG_FBCR,
 GSREG_EWDR,
 GSREG_EWLR,
 GSREG_EWRR
};
uint32_t GetRegister(const unsigned id, char* const special, const uint32_t special_len) MDFN_COLD;
void SetRegister(const unsigned id, const uint32_t value) MDFN_COLD;

}


#endif
