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

#ifndef __MDFN_SS_DB_H
#define __MDFN_SS_DB_H

#include <string>
#include <vector>

#include "../mednafen-types.h"
#include "../git.h"
#include "../cdstream.h"

enum
{
 CPUCACHE_EMUMODE_DATA_CB = 0,
 CPUCACHE_EMUMODE_DATA = 1,
 CPUCACHE_EMUMODE_FULL = 2,
 CPUCACHE_EMUMODE__COUNT = 3,
};

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
 STVROMLayout rom_layout[16];
};

const STVGameInfo* DB_LookupSTV(const std::string& fname, cdstream* s);

void DB_Lookup(const char* path, const char* sgid, const char* sgname, const char* sgarea, const uint8_t* fd_id, unsigned* const region, int* const cart_type, unsigned* const cpucache_emumode);
uint32_t DB_LookupHH(const char* sgid, const uint8_t* fd_id);
void DB_GetInternalDB(std::vector<GameDB_Database>* databases) MDFN_COLD;
std::string DB_GetHHDescriptions(const uint32_t hhv) MDFN_COLD;


#endif
