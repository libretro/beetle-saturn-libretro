/******************************************************************************/
/* Mednafen Sega Saturn Emulation Module                                      */
/******************************************************************************/
/* rom.c - ROM cart emulation
**  Copyright (C) 2016-2017 Mednafen Team
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

/* Converted from cart/rom.cpp. The original had no C++ constructs
   beyond its includes; the conversion is just the include trim and
   the c->CS01_SetRW8W16 member call becoming a free-function call. */

#include <stdint.h>
#include <stdbool.h>

#include <streams/file_stream.h>

#include <mednafen/mednafen-types.h>   /* MDFN_HOT */
#include "../cart.h"
#include "rom.h"

/* SS_SetPhysMemMap is defined in ss.cpp; its declaration lives in the
   C++ ss.h (class SH7095, default args), so it cannot be included
   here. Mirror the prototype -- the trailing is_writeable argument
   had a C++ default of false, dropped here, so callers pass it
   explicitly. */
void SS_SetPhysMemMap(uint32_t Astart, uint32_t Aend, uint16_t *ptr, uint32_t length, bool is_writeable);

static uint16_t ROM[0x100000];

static MDFN_HOT void ROM_Read(uint32_t A, uint16_t *DB)
{
   /* TODO: Check mirroring. */
   *DB = *(uint16_t*)((uint8_t*)ROM + (A & 0x1FFFFE));
}

void CART_ROM_Init(struct CartInfo *c, RFILE *str)
{
   unsigned i;

   filestream_read(str, ROM, 0x200000);

   for(i = 0; i < 0x100000; i++)
   {
      /* MDFN_de16msb<true> folded: aligned BE 16-bit decode.
       * On MSB_FIRST host: ROM[i] = ROM[i] (no-op). On LE host:
       * byteswap each ROM[i] to convert BE-on-disk to host-endian. */
#ifndef MSB_FIRST
      ROM[i] = (uint16_t)((ROM[i] << 8) | (ROM[i] >> 8));
#endif
   }

   SS_SetPhysMemMap(0x02000000, 0x03FFFFFF, ROM, 0x200000, false);
   CartInfo_CS01_SetRW8W16(c, 0x02000000, 0x03FFFFFF, ROM_Read, NULL, NULL);
}
