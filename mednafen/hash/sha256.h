/******************************************************************************/
/* Mednafen - Multi-system Emulator                                           */
/******************************************************************************/
/* sha256.h:
**  Copyright (C) 2014-2017 Mednafen Team
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

#ifndef __MDFN_SHA256_H
#define __MDFN_SHA256_H

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <retro_inline.h>

/* Formerly C++: sha256_digest was std::array<uint8_t,32> and
   sha256_hasher was a class. Converted to a plain struct + free
   functions.

   std::array gave digests value-semantics (copy, ==/!=) plus
   .data()/.size(). The struct keeps copy-assignment for free; in
   place of the rest, callers use sha256_digest_eq() for comparison
   and the .b member (with sizeof) for the raw bytes.

   Dropped as part of the conversion, all of it dead or C++-only:
     - constexpr operator""_sha256 (user-defined literal)
     - sha256_test() -- its only consumer was the _sha256 literal
     - process_scalar / process_cstr -- unused template helpers */

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
   uint8_t b[32];
} sha256_digest;

typedef struct
{
   uint32_t h[8];
   uint8_t  buf[64];
   size_t   buf_count;
   uint64_t bytes_processed;
} sha256_hasher;

/* One-shot hash of a buffer. */
sha256_digest sha256(const void *data, uint64_t len);

/* Incremental hasher. */
void          sha256_hasher_init(sha256_hasher *st);
void          sha256_hasher_process(sha256_hasher *st, const void *data, size_t len);
sha256_digest sha256_hasher_digest(const sha256_hasher *st);

/* 1 if the two digests are equal, else 0. */
static INLINE int sha256_digest_eq(const sha256_digest *a, const sha256_digest *b)
{
   return !memcmp(a->b, b->b, sizeof(a->b));
}

#ifdef __cplusplus
}
#endif

#endif
