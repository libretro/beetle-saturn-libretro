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

#include "../mednafen-types.h"
#include "../cdstream.h"

/* db.c -> db.c (this conversion sequence): DB_Lookup* are defined
 * in C now, but libretro.cpp (C++) calls them.  Force C linkage on
 * both sides via the extern "C" wrap so the C++ caller doesn't
 * name-mangle the references and the linker sees the same unmangled
 * symbols db.c emits.  Same pattern as ss.h / smpc.h / sound.h /
 * input.h after the SS-core C-compat sequence.  Without this, an
 * LTO+mingw build fails with undefined-reference errors -- the
 * a7fa45b class of bug. */
#ifdef __cplusplus
extern "C" {
#endif

enum
{
 CPUCACHE_EMUMODE_DATA_CB = 0,
 CPUCACHE_EMUMODE_DATA = 1,
 CPUCACHE_EMUMODE_FULL = 2,
 CPUCACHE_EMUMODE__COUNT = 3,
};

#include "db_stv.h"

const struct STVGameInfo* DB_LookupSTV(const char* fname, cdstream* s);

void DB_Lookup(const char* path, const char* sgid, const char* sgname, const char* sgarea, const uint8_t* fd_id, unsigned* const region, int* const cart_type, unsigned* const cpucache_emumode);
uint32_t DB_LookupHH(const char* sgid, const uint8_t* fd_id);

#ifdef __cplusplus
}
#endif

#endif
