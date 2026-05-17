/******************************************************************************/
/* Mednafen Sega Saturn Emulation Module                                      */
/******************************************************************************/
/* backup.c - Backup memory(512KiB) cart emulation
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

/* Converted from cart/backup.cpp. The memory-access function was a
   template<typename T, bool IsWrite>; T was unused in the body (byte
   vs word is decided by A & 1, not by T), so it collapses to two
   functions, one read and one write. */

#include <stdint.h>
#include <string.h>
#include <boolean.h>

#include "../../mednafen-types.h"   /* MDFN_HOT, MDFN_COLD */
#include "../../state.h"            /* SFORMAT, SFPTR8N, SFEND, MDFNSS_StateAction */
#include "../cart.h"
#include "backup.h"

static uint8_t ExtBackupRAM[0x80000];
static bool    ExtBackupRAM_Dirty;

/* TODO: Check mirroring. */
static MDFN_HOT void ExtBackupRAM_Read_DB(uint32_t A, uint16_t *DB)
{
   uint8_t* const ptr = ExtBackupRAM + ((A >> 1) & 0x7FFFF);

   *DB = (*ptr << 0) | 0xFF00;

   if((A & ~1) == 0x04FFFFFE)
      *DB = 0x21;
}

static MDFN_HOT void ExtBackupRAM_Write_DB(uint32_t A, uint16_t *DB)
{
   uint8_t* const ptr = ExtBackupRAM + ((A >> 1) & 0x7FFFF);

   if(A & 1)
   {
      ExtBackupRAM_Dirty = true;
      *ptr = *DB;
   }
}

static MDFN_COLD bool GetClearNVDirty(void)
{
   bool ret = ExtBackupRAM_Dirty;

   ExtBackupRAM_Dirty = false;

   return ret;
}

static MDFN_COLD void GetNVInfo(const char **ext, void **nv_ptr, bool *nv16, uint64_t *nv_size)
{
   *ext     = "bcr";
   *nv_ptr  = ExtBackupRAM;
   *nv16    = false;
   *nv_size = sizeof(ExtBackupRAM);
}

static MDFN_COLD void StateAction(StateMem *sm, const unsigned load, const bool data_only)
{
   SFORMAT StateRegs[] =
   {
      SFPTR8N(ExtBackupRAM, 0x80000, "ExtBackupRAM"),
      SFEND
   };

   MDFNSS_StateAction(sm, load, data_only, StateRegs, "CART_BACKUP", false);

   if(load)
   {
      ExtBackupRAM_Dirty = true;
   }
}

void CART_Backup_Init(struct CartInfo *c)
{
   static const uint8_t init[0x10] = { 0x42, 0x61, 0x63, 0x6B, 0x55, 0x70, 0x52, 0x61, 0x6D, 0x20, 0x46, 0x6F, 0x72, 0x6D, 0x61, 0x74 };
   unsigned i;

   memset(ExtBackupRAM, 0x00, sizeof(ExtBackupRAM));
   for(i = 0; i < 0x200; i += 0x10)
      memcpy(ExtBackupRAM + i, init, 0x10);

   ExtBackupRAM_Dirty = false;

   CartInfo_CS01_SetRW8W16(c, 0x04000000, 0x04FFFFFF,
         ExtBackupRAM_Read_DB,
         ExtBackupRAM_Write_DB,
         ExtBackupRAM_Write_DB);

   c->GetClearNVDirty = GetClearNVDirty;
   c->GetNVInfo       = GetNVInfo;
   c->StateAction     = StateAction;
}
