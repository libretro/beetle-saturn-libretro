/******************************************************************************/
/* Mednafen Sega Saturn Emulation Module                                      */
/******************************************************************************/
/* debug.cpp - Mednafen debug cart emulation
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

#include "common.h"
#include "debug.h"

template<typename T, bool IsWrite>
static void Debug_RW_DB(uint32_t A, uint16_t* DB)
{
 //
 // printf-related debugging
 //
 if((A &~ 0x3) == 0x02100000)
 {
  if(IsWrite)
  {
   if(A == 0x02100001)
   {
    fputc(*DB, stderr);
    fflush(stderr);
   }
  }
  else
   *DB = 0;

  return;
 }
}


void CART_Debug_Init(CartInfo* c)
{
 c->CS01_SetRW8W16(0x02100000, /*0x02100001*/ 0x021FFFFF,
	Debug_RW_DB<uint16_t, false>,
	Debug_RW_DB<uint8_t, true>,
	Debug_RW_DB<uint16_t, true>);
}
