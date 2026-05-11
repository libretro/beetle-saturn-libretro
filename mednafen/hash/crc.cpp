/******************************************************************************/
/* Mednafen - Multi-system Emulator                                           */
/******************************************************************************/
/* crc.cpp:
**  Copyright (C) 2018-2023 Mednafen Team
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

#include "../mednafen.h"
#include "crc.h"

#include <zlib.h>

uint16 crc16_ccitt(uint16 initial, const void* data, size_t len)
{
 /* CRC-16-CCITT, polynomial 0x1021, MSB-first, no reflection, no final XOR.
  * Matches Mednafen's crc16_ccitt(initial, data, len). Slow bitwise form;
  * the only callers in this codebase invoke it once per ST-V boot to seed
  * EEPROM contents with ~0x36 bytes, so a table-driven version isn't worth
  * the binary bloat. */
 const uint8_t* p = (const uint8_t*)data;
 uint16 crc = initial;
 for(size_t i = 0; i < len; i++)
 {
  crc ^= ((uint16)p[i]) << 8;
  for(int b = 0; b < 8; b++)
   crc = (crc & 0x8000) ? ((crc << 1) ^ 0x1021) : (crc << 1);
 }
 return crc;
}

uint32 crc32_zip(uint32 initial, const void* data, size_t len)
{
 /* The "zip" CRC32 is the standard Ethernet/PNG/zip CRC32, identical to
  * what zlib's crc32() computes, so forward to it. zlib's crc32(0, ...) is
  * exactly initial=0, the seed Mednafen documents. */
 return (uint32)crc32(initial, (const unsigned char*)data, (unsigned int)len);
}
