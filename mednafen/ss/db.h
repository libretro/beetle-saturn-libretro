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

#include "db_stv.h"

const STVGameInfo* DB_LookupSTV(const char* fname, cdstream* s);

void DB_Lookup(const char* path, const char* sgid, const char* sgname, const char* sgarea, const uint8_t* fd_id, unsigned* const region, int* const cart_type, unsigned* const cpucache_emumode);
uint32_t DB_LookupHH(const char* sgid, const uint8_t* fd_id);
void DB_GetInternalDB(std::vector<GameDB_Database>* databases) MDFN_COLD;
std::string DB_GetHHDescriptions(const uint32_t hhv) MDFN_COLD;


#endif
