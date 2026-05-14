/******************************************************************************/
/* Mednafen - Multi-system Emulator                                           */
/******************************************************************************/
/* memory.h:
**  Copyright (C) 2010-2016 Mednafen Team
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

#ifndef __MDFN_SS_MEMORY_H
#define __MDFN_SS_MEMORY_H


#include <new>
#include <stdlib.h>

// "Array" is a bit of a misnomer, but it helps avoid confusion with memset() semantics hopefully.
static INLINE void MDFN_FastArraySet(uint64_t* const dst, const uint64_t value, const size_t count)
{
 #if defined(__x86_64__) && !defined(_MSC_VER)
 {
  uint32_t dummy_output0, dummy_output1;

  asm volatile(
        "cld\n\t"
        "rep stosq\n\t"
        : "=D" (dummy_output0), "=c" (dummy_output1)
        : "a" (value), "D" (dst), "c" (count)
        : "cc", "memory");
 }
 #else

 for(uint64_t *ai = dst; MDFN_LIKELY(ai != (dst + count)); ai++)
  *ai = value;

 #endif
}

static INLINE void MDFN_FastArraySet(uint32_t* const dst, const uint32_t value, const size_t count)
{
 #if defined(__x86_64__) && !defined(_MSC_VER)
 {
  uint32_t dummy_output0, dummy_output1;

  asm volatile(
        "cld\n\t"
        "rep stosl\n\t"
        : "=D" (dummy_output0), "=c" (dummy_output1)
        : "a" (value), "D" (dst), "c"(count)
        : "cc", "memory");

  return;
 }
 #else
 if(0 == ((uintptr_t)dst & (sizeof(uint64_t) - 1)) && !(count & 1))
  MDFN_FastArraySet((uint64_t*)dst, value | ((uint64_t)value << 32), count >> 1);
 else
 {
  for(uint32_t *ai = dst; MDFN_LIKELY(ai != (dst + count)); ai++)
   *ai = value;
 }
 #endif
}

static INLINE void MDFN_FastArraySet(uint16_t* const dst, const uint16_t value, const size_t count)
{
 if(0 == ((uintptr_t)dst & (sizeof(uint32_t) - 1)) && !(count & 1))
  MDFN_FastArraySet((uint32_t*)dst, value | (value << 16), count >> 1);
 else
 {
  for(uint16_t *ai = dst; MDFN_LIKELY(ai != (dst + count)); ai++)
   *ai = value;
 }
}

static INLINE void MDFN_FastArraySet(uint8_t* const dst, const uint16_t value, const size_t count)
{
 if(0 == ((uintptr_t)dst & (sizeof(uint16_t) - 1)) && !(count & 1))
  MDFN_FastArraySet((uint16_t*)dst, value | (value << 8), count >> 1);
 else
 {
  for(uint8_t *ai = dst; MDFN_LIKELY(ai != (dst + count)); ai++)
   *ai = value;
 }
}

#endif
