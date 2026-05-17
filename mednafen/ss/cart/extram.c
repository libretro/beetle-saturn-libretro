/******************************************************************************/
/* Mednafen Sega Saturn Emulation Module                                      */
/******************************************************************************/
/* extram.c - Extended RAM(1MiB/4MiB) cart emulation
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

/* Converted from cart/extram.cpp. The memory-access function was a
   template<typename T, bool IsWrite>; T is used (sizeof(T) in the
   mask), so it monomorphizes to read16 / write8 / write16. */

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <boolean.h>

#include "../../mednafen-types.h"   /* MDFN_HOT, MDFN_COLD */
#include "../../state.h"            /* SFORMAT, SFPTR16N, SFEND, MDFNSS_StateAction */
#include "../cart.h"
#include "extram.h"

/* SS_SetPhysMemMap: defined in ss.cpp, declared in the C++ ss.h.
   Mirror the prototype (trailing is_writeable default arg dropped). */
void SS_SetPhysMemMap(uint32_t Astart, uint32_t Aend, uint16_t *ptr, uint32_t length, bool is_writeable);

static uint16_t ExtRAM[0x200000];
static size_t   ExtRAM_Mask;
static uint8_t  Cart_ID;

/* Was ExtRAM_RW_DB<uint16_t, false>. */
static MDFN_HOT void ExtRAM_Read16_DB(uint32_t A, uint16_t *DB)
{
   uint16_t* const ptr = (uint16_t*)((uint8_t*)ExtRAM + (A & ExtRAM_Mask));
   *DB = *ptr;
}

/* Was ExtRAM_RW_DB<uint8_t, true>. */
static MDFN_HOT void ExtRAM_Write8_DB(uint32_t A, uint16_t *DB)
{
   const uint32_t mask = (0xFF << (((A & 1) ^ 1) << 3));
   uint16_t* const ptr = (uint16_t*)((uint8_t*)ExtRAM + (A & ExtRAM_Mask));
   *ptr = (*ptr & ~mask) | (*DB & mask);
}

/* Was ExtRAM_RW_DB<uint16_t, true>. */
static MDFN_HOT void ExtRAM_Write16_DB(uint32_t A, uint16_t *DB)
{
   uint16_t* const ptr = (uint16_t*)((uint8_t*)ExtRAM + (A & ExtRAM_Mask));
   *ptr = *DB;
}

static MDFN_HOT void CartID_Read_DB(uint32_t A, uint16_t *DB)
{
   if((A & ~1) == 0x04FFFFFE)
      *DB = Cart_ID;
}

static MDFN_COLD void Reset(bool powering_up)
{
   if(powering_up)
      memset(ExtRAM, 0, sizeof(ExtRAM));	/* TODO: Test. */
}

static MDFN_COLD void StateAction(StateMem *sm, const unsigned load, const bool data_only)
{
   const size_t pcount = (Cart_ID == 0x5C) ? 0x100000 : 0x040000;

   SFORMAT StateRegs[] =
   {
      SFPTR16N(&ExtRAM[0x000000], pcount, "LO"),
      SFPTR16N(&ExtRAM[0x100000], pcount, "HI"),
      SFEND
   };

   MDFNSS_StateAction(sm, load, data_only, StateRegs, "CART_EXTRAM", false);
}

void CART_ExtRAM_Init(struct CartInfo *c, bool R4MiB)
{
   if(R4MiB)
   {
      Cart_ID     = 0x5C;
      ExtRAM_Mask = 0x3FFFFE;
   }
   else
   {
      Cart_ID     = 0x5A;
      ExtRAM_Mask = 0x27FFFE;
   }

   SS_SetPhysMemMap(0x02400000, 0x025FFFFF, ExtRAM + (0x000000 / sizeof(uint16_t)), (R4MiB ? 0x200000 : 0x080000), true);
   SS_SetPhysMemMap(0x02600000, 0x027FFFFF, ExtRAM + (0x200000 / sizeof(uint16_t)), (R4MiB ? 0x200000 : 0x080000), true);

   CartInfo_CS01_SetRW8W16(c, 0x02400000, 0x027FFFFF,
         ExtRAM_Read16_DB,
         ExtRAM_Write8_DB,
         ExtRAM_Write16_DB);

   CartInfo_CS01_SetRW8W16(c, /*0x04FFFFFE*/0x04F00000, 0x04FFFFFF, CartID_Read_DB, NULL, NULL);

   c->Reset       = Reset;
   c->StateAction = StateAction;
}
