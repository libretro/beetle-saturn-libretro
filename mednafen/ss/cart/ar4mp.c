/******************************************************************************/
/* Mednafen Sega Saturn Emulation Module                                      */
/******************************************************************************/
/* ar4mp.c - Action Replay 4M Plus cart emulation
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

/*
 Unfinished, and looks like the firmware needs CPU UBC emulation.
*/

/* Converted from cart/ar4mp.cpp. The ExtRAM access function was a
   template<typename T, bool IsWrite>; T is used (sizeof(T) in the
   mask), so it monomorphizes to read16 / write8 / write16. The
   buffers were already raw malloc/free here (a previous pass removed
   the std::unique_ptr ownership guard). */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <boolean.h>

#include <streams/file_stream.h>

#include "../../mednafen-types.h"   /* MDFN_HOT, MDFN_COLD, MDFN_UNLIKELY */
#include "../../state.h"            /* SFORMAT, SFPTR16, SFEND, MDFNSS_StateAction */
#include "../cart.h"
#include "ar4mp.h"

/* SS_SetPhysMemMap: defined in ss.c, declared in ss.h.
   Mirror the prototype (trailing is_writeable default arg dropped). */
void SS_SetPhysMemMap(uint32_t Astart, uint32_t Aend, uint16_t *ptr, uint32_t length, bool is_writeable);

static bool      FLASH_Dirty;

static uint16_t *FLASH  = NULL; /* [0x20000]  */
static uint16_t *ExtRAM = NULL; /* [0x200000] */

/* Was ExtRAM_RW_DB<uint16_t, false>. */
static MDFN_HOT void ExtRAM_Read16_DB(uint32_t A, uint16_t *DB)
{
   uint16_t* const ptr = (uint16_t*)((uint8_t*)ExtRAM + (A & 0x3FFFFE));
   *DB = *ptr;
}

/* Was ExtRAM_RW_DB<uint8_t, true>. */
static MDFN_HOT void ExtRAM_Write8_DB(uint32_t A, uint16_t *DB)
{
   const uint32_t mask = (0xFF << (((A & 1) ^ 1) << 3));
   uint16_t* const ptr = (uint16_t*)((uint8_t*)ExtRAM + (A & 0x3FFFFE));
   *ptr = (*ptr & ~mask) | (*DB & mask);
}

/* Was ExtRAM_RW_DB<uint16_t, true>. */
static MDFN_HOT void ExtRAM_Write16_DB(uint32_t A, uint16_t *DB)
{
   uint16_t* const ptr = (uint16_t*)((uint8_t*)ExtRAM + (A & 0x3FFFFE));
   *ptr = *DB;
}

static MDFN_HOT void FLASH_Read(uint32_t A, uint16_t *DB)
{
   if(MDFN_UNLIKELY(A & 0x080000))
      *DB = 0xFFFF;
   else
      *DB = *(uint16_t*)((uint8_t*)FLASH + (A & 0x3FFFE));
}

static MDFN_HOT void CV_Read(uint32_t A, uint16_t *DB)
{
   *DB = 0xFFFF ^ ((A >> 20) & ((A >> 18) | (A >> 19) | ((A >> 21) ^ (A >> 22))) & 0x2);
}

static MDFN_HOT void RAMID_Read(uint32_t A, uint16_t *DB)
{
   (void)A;
   *DB = 0xFF5C;
}

static MDFN_COLD void Reset(bool powering_up)
{
   if(powering_up)
      memset(ExtRAM, 0, 0x400000);	/* TODO: Test. */
}

static MDFN_COLD bool GetClearNVDirty(void)
{
   bool ret = FLASH_Dirty;

   FLASH_Dirty = false;

   return ret;
}

static MDFN_COLD void GetNVInfo(const char **ext, void **nv_ptr, bool *nv16, uint64_t *nv_size)
{
   *ext     = "arp";
   *nv_ptr  = FLASH;
   *nv16    = true;
   *nv_size = 0x40000;
}

static MDFN_COLD void StateAction(StateMem *sm, const unsigned load, const bool data_only)
{
   SFORMAT StateRegs[] =
   {
      SFPTR16(FLASH, 0x20000),
      SFPTR16(ExtRAM, 0x200000),

      SFEND
   };

   MDFNSS_StateAction(sm, load, data_only, StateRegs, "CART_AR4MP", false);

   if(load)
   {
      FLASH_Dirty = true;
   }
}

static MDFN_COLD void Kill(void)
{
   if(FLASH)
   {
      free(FLASH);
      FLASH = NULL;
   }

   if(ExtRAM)
   {
      free(ExtRAM);
      ExtRAM = NULL;
   }
}

void CART_AR4MP_Init(struct CartInfo *c, RFILE *str)
{
   unsigned i;

   /* Was a pair of std::unique_ptr<uint16_t[]> that owned the buffers
    * until setup finished and then .release()d ownership to the raw
    * FLASH / ExtRAM globals -- RAII guarding the path between
    * allocation and hand-off. There is no early return on that path,
    * and with exceptions gone from the tree the guard is unnecessary:
    * allocate straight into the globals. Kill() frees them. */
   FLASH  = (uint16_t*)malloc(0x20000 * sizeof(uint16_t));
   ExtRAM = (uint16_t*)malloc(0x200000 * sizeof(uint16_t));

   if(!FLASH || !ExtRAM)
   {
      free(FLASH);
      free(ExtRAM);
      FLASH  = NULL;
      ExtRAM = NULL;
      return;
   }
   /* Short read leaves uninitialised malloc'd bytes in the tail of
    * FLASH, which then become "ROM" after the byte-swap loop below
    * and get installed into the Saturn address map by
    * SS_SetPhysMemMap.  Tear down the same way as the OOM path:
    * free both buffers, NULL the globals, and return without
    * installing cart callbacks -- the cart slot stays at the
    * dummies CART_Init configured at the top.  filestream_read
    * returns the byte count actually read (or a value < expected
    * on EOF / short file / read error), so a single `!=` check
    * catches every short-read variant. */
   if(filestream_read(str, FLASH, 0x40000) != 0x40000)
   {
      free(FLASH);
      free(ExtRAM);
      FLASH  = NULL;
      ExtRAM = NULL;
      return;
   }

   for(i = 0; i < 0x20000; i++)
   {
      /* MDFN_de16msb<true> folded: BE-on-disk to host-endian. */
#ifndef MSB_FIRST
      FLASH[i] = (uint16_t)((FLASH[i] << 8) | (FLASH[i] >> 8));
#endif
   }

   SS_SetPhysMemMap(0x02000000, 0x020FFFFF, FLASH, 0x40000, false);
   CartInfo_CS01_SetRW8W16(c, 0x02000000, 0x020FFFFF, FLASH_Read, NULL, NULL);
   CartInfo_CS01_SetRW8W16(c, 0x03000000, 0x03FFFFFF, CV_Read,    NULL, NULL);
   CartInfo_CS01_SetRW8W16(c, 0x04000000, 0x04FFFFFF, RAMID_Read, NULL, NULL);

   SS_SetPhysMemMap(0x02400000, 0x027FFFFF, ExtRAM, 0x400000, true);
   CartInfo_CS01_SetRW8W16(c, 0x02400000, 0x027FFFFF,
         ExtRAM_Read16_DB,
         ExtRAM_Write8_DB,
         ExtRAM_Write16_DB);

   FLASH_Dirty        = false;
   c->GetClearNVDirty = GetClearNVDirty;
   c->GetNVInfo       = GetNVInfo;

   c->StateAction = StateAction;
   c->Reset       = Reset;
   c->Kill        = Kill;
}
