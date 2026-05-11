/******************************************************************************/
/* Mednafen - Multi-system Emulator                                           */
/******************************************************************************/
/* crc.h:
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

#ifndef __MDFN_HASH_CRC_H
#define __MDFN_HASH_CRC_H

/* Minimal subset of upstream Mednafen's hash/crc.h sufficient for the ST-V port.
 * crc32_zip wraps zlib's standard crc32(); crc16_ccitt is the same polynomial 0x1021
 * left-shift form Mednafen uses (no input/output reflection, no final XOR). */

uint16 crc16_ccitt(uint16 initial, const void* data, size_t len);
uint32 crc32_zip(uint32 initial, const void* data, size_t len);

#endif
