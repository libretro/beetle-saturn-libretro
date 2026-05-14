/******************************************************************************/
/* Mednafen - Multi-system Emulator                                           */
/******************************************************************************/
/* sha256.c:
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

/* Converted from C++ to C: sha256_digest (std::array) and
 * sha256_hasher (class) are now plain structs; the methods are
 * sha256_hasher_* free functions. The template<unsigned n> rotr
 * helper became a plain rotr(x, n) -- it was only ever instantiated
 * with compile-time-constant n, and the compiler still folds a
 * constant rotate. */

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <retro_inline.h>

#include "sha256.h"

#ifndef MDFN_LIKELY
#define MDFN_LIKELY(x) (x)
#endif

static const uint32_t K[64] =
{
 0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
 0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
 0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
 0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
 0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
 0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
 0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
 0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

static INLINE uint32_t rotr(const uint32_t val, const unsigned n)
{
 return (val >> n) | (val << (32 - n));
}

static INLINE uint32_t ch(const uint32_t x, const uint32_t y, const uint32_t z)
{
 return (x & y) ^ ((~x) & z);
}

static INLINE uint32_t maj(const uint32_t x, const uint32_t y, const uint32_t z)
{
 return (x & y) ^ (x & z) ^ (y & z);
}

static INLINE uint32_t bs0(const uint32_t x)
{
 return rotr(x, 2) ^ rotr(x, 13) ^ rotr(x, 22);
}

static INLINE uint32_t bs1(const uint32_t x)
{
 return rotr(x, 6) ^ rotr(x, 11) ^ rotr(x, 25);
}

static INLINE uint32_t ls0(const uint32_t x)
{
 return rotr(x, 7) ^ rotr(x, 18) ^ (x >> 3);
}

static INLINE uint32_t ls1(const uint32_t x)
{
 return rotr(x, 17) ^ rotr(x, 19) ^ (x >> 10);
}

static INLINE void block(uint32_t h[8], const void *blk_data)
{
 uint32_t w[64];
 uint32_t v[8];
 unsigned t, i;

 for(i = 0; i < 8; i++)
  v[i] = h[i];

 for(t = 0; t < 16; t++)
 {
  /* MDFN_de32msb folded: 4 BE bytes -> host uint32_t. */
  const uint8_t *bp__ = (const uint8_t*)blk_data + (t << 2);
  w[t] = ((uint32_t)bp__[0] << 24) | ((uint32_t)bp__[1] << 16) | ((uint32_t)bp__[2] << 8) | (uint32_t)bp__[3];
 }

 for(t = 16; t < 64; t++)
  w[t] = ls1(w[t - 2]) + w[t - 7] + ls0(w[t - 15]) + w[t - 16];

 for(t = 0; t < 64; t++)
 {
  uint32_t T1 = v[7] + bs1(v[4]) + ch(v[4], v[5], v[6]) + K[t] + w[t];
  uint32_t T2 = bs0(v[0]) + maj(v[0], v[1], v[2]);

  v[7] = v[6];
  v[6] = v[5];
  v[5] = v[4];
  v[4] = v[3] + T1;
  v[3] = v[2];
  v[2] = v[1];
  v[1] = v[0];
  v[0] = T1 + T2;
 }

 for(i = 0; i < 8; i++)
  h[i] += v[i];
}

sha256_digest sha256(const void* data, const uint64_t len)
{
 sha256_digest ret;
 uint32_t h[8] = { 0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19 };
 uint8_t* p = (uint8_t*)data;
 uint64_t dc = len;
 unsigned i;

 while(MDFN_LIKELY(dc >= 64))
 {
  block(h, p);

  p += 64;
  dc -= 64;
 }

 {
  uint8_t tmp[128];

  memcpy(tmp, p, dc);
  memset(tmp + dc, 0, 128 - dc);
  tmp[dc] |= 0x80;

  dc = ((dc + 8) &~ 63) + 56;

  /* MDFN_en64msb<true>(ptr, len*8) folded: aligned BE 64-bit store.
   * On MSB_FIRST host: plain memcpy. On LE host: bswap64 then memcpy. */
  {
   uint64_t v__ = len * 8;
#ifndef MSB_FIRST
   v__ =  (v__ << 56) | (v__ >> 56)
       | ((v__ & 0xFF00ULL) << 40) | ((v__ >> 40) & 0xFF00ULL)
       | ((v__ & 0xFF0000ULL) << 24) | ((v__ >> 24) & 0xFF0000ULL)
       | ((v__ & 0xFF000000ULL) <<  8) | ((v__ >>  8) & 0xFF000000ULL);
#endif
   memcpy(&tmp[dc], &v__, 8);
  }

  block(h, tmp);
  if(dc >= 64)
   block(h, tmp + 64);
 }

 for(i = 0; i < 8; i++)
 {
  /* MDFN_en32msb folded: write host uint32_t as 4 BE bytes. */
  const uint32_t v__ = h[i];
  ret.b[i * 4 + 0] = v__ >> 24;
  ret.b[i * 4 + 1] = v__ >> 16;
  ret.b[i * 4 + 2] = v__ >>  8;
  ret.b[i * 4 + 3] = v__;
 }
 return ret;
}

void sha256_hasher_init(sha256_hasher *st)
{
 static const uint32_t h0[8] = { 0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19 };
 unsigned i;

 st->buf_count = 0;
 st->bytes_processed = 0;

 for(i = 0; i < 8; i++)
  st->h[i] = h0[i];
}

void sha256_hasher_process(sha256_hasher *st, const void* data, size_t len)
{
 uint8_t* d8 = (uint8_t*)data;

 st->bytes_processed += len;

 while(len)
 {
  if(st->buf_count || len < 0x40)
  {
   const size_t copy_len = ((size_t)(0x40 - st->buf_count) < (size_t)(len) ? (size_t)(0x40 - st->buf_count) : (size_t)(len));

   memcpy(&st->buf[st->buf_count], d8, copy_len);
   len -= copy_len;
   d8 += copy_len;
   st->buf_count += copy_len;
   if(st->buf_count == 0x40)
   {
    block(st->h, st->buf);
    st->buf_count = 0;
   }
  }
  else
  {
   block(st->h, d8);
   d8 += 0x40;
   len -= 0x40;
  }
 }
}

sha256_digest sha256_hasher_digest(const sha256_hasher *st)
{
 sha256_digest ret;
 sha256_hasher tmp = *st;
 const size_t footer_len = ((st->buf_count <= (0x40 - 9)) ? 0x40 : 0x80) - st->buf_count;
 uint8_t footer[0x80];
 unsigned i;

 memset(footer, 0, sizeof(footer));
 footer[0] |= 0x80;

 /* MDFN_en64msb folded: byte-by-byte BE 64-bit store (no MSB_FIRST
  * needed - explicit byte arithmetic gives BE bytes on any host). */
 {
  uint8_t *fp__ = &footer[footer_len - 8];
  uint64_t v__ = st->bytes_processed * 8;
  fp__[0] = v__ >> 56;
  fp__[1] = v__ >> 48;
  fp__[2] = v__ >> 40;
  fp__[3] = v__ >> 32;
  fp__[4] = v__ >> 24;
  fp__[5] = v__ >> 16;
  fp__[6] = v__ >>  8;
  fp__[7] = v__;
 }

 sha256_hasher_process(&tmp, footer, footer_len);

 for(i = 0; i < 8; i++)
 {
  /* MDFN_en32msb folded: write host uint32_t as 4 BE bytes. */
  const uint32_t v__ = tmp.h[i];
  ret.b[i * 4 + 0] = v__ >> 24;
  ret.b[i * 4 + 1] = v__ >> 16;
  ret.b[i * 4 + 2] = v__ >>  8;
  ret.b[i * 4 + 3] = v__;
 }

 return ret;
}
