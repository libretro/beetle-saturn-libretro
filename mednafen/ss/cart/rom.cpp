/******************************************************************************/
/* Mednafen Sega Saturn Emulation Module                                      */
/******************************************************************************/
/* rom.cpp - ROM cart emulation
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

#include "common.h"
#include "rom.h"

static uint16 ROM[0x100000];

static MDFN_HOT void ROM_Read(uint32 A, uint16* DB)
{
 // TODO: Check mirroring.
 *DB = *(uint16*)((uint8*)ROM + (A & 0x1FFFFE));
}

void CART_ROM_Init(CartInfo* c, RFILE *str)
{
   filestream_read(str, ROM, 0x200000);

   for(unsigned i = 0; i < 0x100000; i++)
   {
      /* MDFN_de16msb<true> folded: aligned BE 16-bit decode.
       * On MSB_FIRST host: ROM[i] = ROM[i] (no-op). On LE host:
       * byteswap each ROM[i] to convert BE-on-disk to host-endian. */
#ifndef MSB_FIRST
      ROM[i] = (uint16)((ROM[i] << 8) | (ROM[i] >> 8));
#endif
   }

   SS_SetPhysMemMap (0x02000000, 0x03FFFFFF, ROM, 0x200000, false);
   c->CS01_SetRW8W16(0x02000000, 0x03FFFFFF, ROM_Read);
}
