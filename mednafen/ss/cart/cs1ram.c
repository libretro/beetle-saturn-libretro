/******************************************************************************/
/* Mednafen Sega Saturn Emulation Module                                      */
/******************************************************************************/
/* cs1ram.c - CS1 16MiB RAM cart emulation
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

/* Converted from cart/cs1ram.cpp. The memory-access function was a
   template<typename T, bool IsWrite>; here T *is* used (sizeof(T) in
   the byte/word mask), so it monomorphizes to the three concrete
   instantiations the original used: read16, write8, write16.
   new[]/delete[] become calloc/free. */

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <mednafen/mednafen-types.h>   /* MDFN_HOT, MDFN_COLD */
#include <mednafen/state.h>            /* SFORMAT, SFPTR16N, SFEND, MDFNSS_StateAction */
#include "../cart.h"
#include "cs1ram.h"

/* SS_SetPhysMemMap: defined in ss.cpp, declared in the C++ ss.h.
   Mirror the prototype (trailing is_writeable default arg dropped). */
void SS_SetPhysMemMap(uint32_t Astart, uint32_t Aend, uint16_t *ptr, uint32_t length, bool is_writeable);

static uint16_t *CS1RAM = NULL;

/* Was CS1RAM_RW_DB<uint16_t, false>. */
static MDFN_HOT void CS1RAM_Read16_DB(uint32_t A, uint16_t *DB)
{
   uint16_t* const ptr = (uint16_t*)((uint8_t*)CS1RAM + (A & 0x00FFFFFE));
   *DB = *ptr;
}

/* Was CS1RAM_RW_DB<uint8_t, true>. */
static MDFN_HOT void CS1RAM_Write8_DB(uint32_t A, uint16_t *DB)
{
   const uint32_t mask = (0xFF << (((A & 1) ^ 1) << 3));
   uint16_t* const ptr = (uint16_t*)((uint8_t*)CS1RAM + (A & 0x00FFFFFE));
   *ptr = (*ptr & ~mask) | (*DB & mask);
}

/* Was CS1RAM_RW_DB<uint16_t, true>. */
static MDFN_HOT void CS1RAM_Write16_DB(uint32_t A, uint16_t *DB)
{
   uint16_t* const ptr = (uint16_t*)((uint8_t*)CS1RAM + (A & 0x00FFFFFE));
   *ptr = *DB;
}

static MDFN_COLD void Reset(bool powering_up)
{
   if(powering_up)
      memset(CS1RAM, 0, 0x1000000);
}

static MDFN_COLD void Kill(void)
{
   if(CS1RAM)
   {
      free(CS1RAM);
      CS1RAM = NULL;
   }
}

static MDFN_COLD void StateAction(StateMem *sm, const unsigned load, const bool data_only)
{
   SFORMAT StateRegs[] =
   {
      SFPTR16N(CS1RAM, 0x800000, "RAM"),
      SFEND
   };

   MDFNSS_StateAction(sm, load, data_only, StateRegs, "CART_CS1RAM", false);
}

void CART_CS1RAM_Init(struct CartInfo *c)
{
   CS1RAM = (uint16_t*)calloc(0x1000000 / sizeof(uint16_t), sizeof(uint16_t));
   if (!CS1RAM)
   {
      /* OOM allocating 16MB.  Pre-conversion C++ used `new uint16_t[]`
       * which threw std::bad_alloc; the conversion to calloc dropped
       * the check.  Silent return matches ar4mp.c's convention --
       * Cart.Read/Write/Kill stay at CART_Init's installed dummies so
       * any further cart access is a safe no-op. */
      return;
   }

   SS_SetPhysMemMap(0x04000000, 0x04FFFFFF, CS1RAM, 0x1000000, true);
   CartInfo_CS01_SetRW8W16(c, 0x04000000, 0x04FFFFFF,
         CS1RAM_Read16_DB,
         CS1RAM_Write8_DB,
         CS1RAM_Write16_DB);

   c->Reset       = Reset;
   c->Kill        = Kill;
   c->StateAction = StateAction;
}
