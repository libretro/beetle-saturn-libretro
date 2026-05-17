/******************************************************************************/
/* Mednafen Sega Saturn Emulation Module                                      */
/******************************************************************************/
/* db.h:
**  Copyright (C) 2016-2020 Mednafen Team
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

#ifndef __MDFN_SS_DB_STV_H
#define __MDFN_SS_DB_STV_H

#include <stdint.h>
#include <boolean.h>

/* The ST-V game-info structs and enums, factored out of db.h so they
   can be included from plain C. db.h itself is C++ (it pulls in
   <string>, <vector>, git.h), but STVROMLayout / STVGameInfo and the
   STV_* enums are pure POD and several C files now need them --
   cart.c, cart/stv.c, and stvio.c. db.h #includes this header in
   place of its former inline copy, so the C++ side sees the
   identical definitions. */

#ifdef __cplusplus
extern "C" {
#endif

// ST-V (Sega Titan Video) arcade hardware support.
//
// Ported from upstream Mednafen 1.32.1's mednafen/ss/db.h. ST-V games are
// distributed as multi-file MAME-style ROM sets; STVROMLayout describes how
// to map each file's contents into Saturn A-bus CS0/CS1 address space, and
// STVGameInfo wraps the per-game settings (region, control scheme,
// encryption/compression chip, rotation flag, ROM layout table).
//
// The layout `map` field selects byte-vs-16-bit-endian unpacking, and
// `head_crc32` lets DB_LookupSTV disambiguate ROM sets that share a first
// file name across game variants.
//
struct STVROMLayout
{
 uint32_t offset;
 uint32_t size;
 unsigned map;
 const char* fname;
 uint32_t head_crc32;
};

enum
{
 STV_CONTROL_3B = 0,
 STV_CONTROL_6B,
 STV_CONTROL_HAMMER,
 //
 STV_CONTROL_RSG
};

enum
{
 STV_MAP_BYTE = 0,
 STV_MAP_16LE,
 STV_MAP_16BE
};

enum
{
 STV_ROMTWIDDLE_NONE = 0,
 STV_ROMTWIDDLE_SANJEON
};

enum
{
 STV_EC_CHIP_NONE = 0,
 //
 STV_EC_CHIP_315_5881,
 STV_EC_CHIP_315_5838,
 //
 STV_EC_CHIP_RSG
};

struct STVGameInfo
{
 const char* name;
 unsigned area;
 unsigned control;
 unsigned ec_chip;
 unsigned romtwiddle;
 bool rotate;
 struct STVROMLayout rom_layout[16];
};

#ifdef __cplusplus
}
#endif

#endif
