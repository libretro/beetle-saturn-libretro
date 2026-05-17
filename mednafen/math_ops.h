/******************************************************************************/
/* Mednafen - Multi-system Emulator                                           */
/******************************************************************************/
/* math_ops.h:
**  Copyright (C) 2007-2016 Mednafen Team
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
** Some ideas from:
**  blargg
**  http://graphics.stanford.edu/~seander/bithacks.html
*/

#ifndef __MDFN_MATH_OPS_H
#define __MDFN_MATH_OPS_H

#if defined(_MSC_VER)
 #include <intrin.h>
#endif

static INLINE unsigned MDFN_lzcount32_0UD(uint32_t v)
{
#if defined(__GNUC__) || defined(__clang__) || defined(__ICC) || defined(__INTEL_COMPILER)
 return __builtin_clz(v);
 #elif defined(_MSC_VER)
 unsigned long idx;

 _BitScanReverse(&idx, v);

 return 31 ^ idx;
#else
 unsigned ret = 0;
 unsigned tmp = !(v & 0xFFFF0000) << 4; v <<= tmp; ret += tmp;
 tmp = !(v & 0xFF000000) << 3; v <<= tmp; ret += tmp;
 tmp = !(v & 0xF0000000) << 2; v <<= tmp; ret += tmp;
 tmp = !(v & 0xC0000000) << 1; v <<= tmp; ret += tmp;
 tmp = !(v & 0x80000000) << 0;            ret += tmp;

 return(ret);
#endif
}

static INLINE unsigned MDFN_lzcount64_0UD(uint64_t v)
{
#if defined(__GNUC__) || defined(__clang__) || defined(__ICC) || defined(__INTEL_COMPILER)
 return __builtin_clzll(v);
#elif defined(_MSC_VER)
#if defined(_WIN64)
   unsigned long idx;
   _BitScanReverse64(&idx, v);
   return 63 ^ idx;
#else
   unsigned long idx0;
   unsigned long idx1;

   _BitScanReverse(&idx1, v >> 0);
   idx1 -= 32;
   if(!_BitScanReverse(&idx0, v >> 32))
    idx0 = idx1;

   idx0 += 32;

   return 63 ^ idx0;
#endif
#else
 unsigned ret = 0;
 unsigned tmp = !(v & 0xFFFFFFFF00000000ULL) << 5; v <<= tmp; ret += tmp;
 tmp = !(v & 0xFFFF000000000000ULL) << 4; v <<= tmp; ret += tmp;
 tmp = !(v & 0xFF00000000000000ULL) << 3; v <<= tmp; ret += tmp;
 tmp = !(v & 0xF000000000000000ULL) << 2; v <<= tmp; ret += tmp;
 tmp = !(v & 0xC000000000000000ULL) << 1; v <<= tmp; ret += tmp;
 tmp = !(v & 0x8000000000000000ULL) << 0;            ret += tmp;
 return(ret);
#endif
}

static INLINE unsigned MDFN_tzcount16_0UD(uint16_t v)
{
 #if defined(__GNUC__) || defined(__clang__) || defined(__ICC) || defined(__INTEL_COMPILER)
 return __builtin_ctz(v);
 #elif defined(_MSC_VER)
 unsigned long idx;

 _BitScanForward(&idx, v);

 return idx;
 #else
 unsigned ret = 0;
 unsigned tmp;

 tmp = !( (uint8_t)v)  << 3; v >>= tmp; ret += tmp;
 tmp = !(v & 0x000F) << 2; v >>= tmp; ret += tmp;
 tmp = !(v & 0x0003) << 1; v >>= tmp; ret += tmp;
 tmp = !(v & 0x0001) << 0;            ret += tmp;

 return ret;
 #endif
}

//
// Result is defined for all possible inputs(including 0).
//
static INLINE unsigned MDFN_lzcount32(uint32_t v) { return !v ? 32 : MDFN_lzcount32_0UD(v); }
static INLINE unsigned MDFN_lzcount64(uint64_t v) { return !v ? 64 : MDFN_lzcount64_0UD(v); }

static INLINE unsigned MDFN_tzcount16(uint16_t v) { return !v ? 16 : MDFN_tzcount16_0UD(v); }

// 0-undefined-input log2. Single 64-bit form; 32-bit callers promote
// cleanly. (Was a set of C++ overloads; only round_up_pow2 ever called
// it, always with values that fit the unsigned path.)
static INLINE unsigned MDFN_log2(uint64_t v) { return 63 ^ MDFN_lzcount64_0UD(v | 1); }

// Rounds up to the nearest power of 2(treats input as unsigned to a degree, but be aware of integer promotion rules).
// Returns 0 on overflow.
// Single 64-bit form; the only caller (cart/bootrom.cpp) passes
// uint32_t / uint64_t values, which promote without surprise.
static INLINE uint64_t round_up_pow2(uint64_t v) { uint64_t tmp = (uint64_t)1 << MDFN_log2(v); return tmp << (tmp < v); }

// Some compilers' optimizers and some platforms might fubar the generated code from these macros,
// so some tests are run in...tests.cpp
#define sign_8_to_s16(_value) ((int16_t)(int8_t)(_value))
#define sign_9_to_s16(_value)  (((int16_t)((unsigned int)(_value) << 7)) >> 7)
#define sign_10_to_s16(_value)  (((int16_t)((uint32_t)(_value) << 6)) >> 6)
#define sign_11_to_s16(_value)  (((int16_t)((uint32_t)(_value) << 5)) >> 5)
#define sign_12_to_s16(_value)  (((int16_t)((uint32_t)(_value) << 4)) >> 4)
#define sign_13_to_s16(_value)  (((int16_t)((uint32_t)(_value) << 3)) >> 3)
#define sign_14_to_s16(_value)  (((int16_t)((uint32_t)(_value) << 2)) >> 2)
#define sign_15_to_s16(_value)  (((int16_t)((uint32_t)(_value) << 1)) >> 1)

// This obviously won't convert higher-than-32 bit numbers to signed 32-bit ;)
// Also, this shouldn't be used for 8-bit and 16-bit signed numbers, since you can
// convert those faster with typecasts...
#define sign_x_to_s32(_bits, _value) (((int32_t)((uint32_t)(_value) << (32 - _bits))) >> (32 - _bits))

#endif
