/******************************************************************************/
/* Mednafen Sega Saturn Emulation Module                                      */
/******************************************************************************/
/* ar4mp.cpp - Action Replay 4M Plus cart emulation
**  Copyright (C) 2017 Mednafen Team
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

#include "common.h"
#include "ar4mp.h"
#include <stdlib.h>

static bool FLASH_Dirty;

static uint16_t* FLASH = nullptr; //[0x20000];
static uint16_t* ExtRAM = nullptr; //[0x200000];

template<typename T, bool IsWrite>
static MDFN_HOT void ExtRAM_RW_DB(uint32_t A, uint16_t* DB)
{
 const uint32_t mask = (sizeof(T) == 2) ? 0xFFFF : (0xFF << (((A & 1) ^ 1) << 3));
 uint16_t* const ptr = (uint16_t*)((uint8_t*)ExtRAM + (A & 0x3FFFFE));

 if(IsWrite)
  *ptr = (*ptr & ~mask) | (*DB & mask);
 else
  *DB = *ptr;
}

static MDFN_HOT void FLASH_Read(uint32_t A, uint16_t* DB)
{
 if(MDFN_UNLIKELY(A & 0x080000))
  *DB = 0xFFFF;
 else
  *DB = *(uint16_t*)((uint8_t*)FLASH + (A & 0x3FFFE));
}

static MDFN_HOT void CV_Read(uint32_t A, uint16_t* DB)
{
 *DB = 0xFFFF ^ ((A >> 20) & ((A >> 18) | (A >> 19) | ((A >> 21) ^ (A >> 22))) & 0x2);
}

static MDFN_HOT void RAMID_Read(uint32_t A, uint16_t* DB)
{
 *DB = 0xFF5C;
}

static MDFN_COLD void Reset(bool powering_up)
{
 if(powering_up)
  memset(ExtRAM, 0, 0x400000);	// TODO: Test.
}

static MDFN_COLD bool GetClearNVDirty(void)
{
 bool ret = FLASH_Dirty;

 FLASH_Dirty = false;

 return ret;
}

static MDFN_COLD void GetNVInfo(const char** ext, void** nv_ptr, bool* nv16, uint64_t* nv_size)
{
 *ext = "arp";
 *nv_ptr = FLASH;
 *nv16 = true;
 *nv_size = 0x40000;
}

static MDFN_COLD void StateAction(StateMem* sm, const unsigned load, const bool data_only)
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

void CART_AR4MP_Init(CartInfo* c, RFILE* str)
{
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
 //
 //
 filestream_read(str, FLASH, 0x40000);

 for(unsigned i = 0; i < 0x20000; i++)
 {
  /* MDFN_de16msb<true> folded: BE-on-disk to host-endian. */
#ifndef MSB_FIRST
  FLASH[i] = (uint16_t)((FLASH[i] << 8) | (FLASH[i] >> 8));
#endif
 }

 SS_SetPhysMemMap (0x02000000, 0x020FFFFF, FLASH, 0x40000, false);
 c->CS01_SetRW8W16(0x02000000, 0x020FFFFF, FLASH_Read);
 c->CS01_SetRW8W16(0x03000000, 0x03FFFFFF, CV_Read);
 c->CS01_SetRW8W16(0x04000000, 0x04FFFFFF, RAMID_Read);

 SS_SetPhysMemMap (0x02400000, 0x027FFFFF, ExtRAM, 0x400000, true);
 c->CS01_SetRW8W16(0x02400000, 0x027FFFFF,
	ExtRAM_RW_DB<uint16_t, false>,
	ExtRAM_RW_DB<uint8_t, true>,
	ExtRAM_RW_DB<uint16_t, true>);


 FLASH_Dirty = false;
 c->GetClearNVDirty = GetClearNVDirty;
 c->GetNVInfo = GetNVInfo;

 c->StateAction = StateAction;
 c->Reset = Reset;
 c->Kill = Kill;
}
