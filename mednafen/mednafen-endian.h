#ifndef __MDFN_ENDIAN_H
#define __MDFN_ENDIAN_H

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <retro_inline.h>
#include "mednafen-types.h"

/* MDFN_ASSUME_ALIGNED: alignment hint for memcpy targets in the
 * MDFN_deXsb / ne16_* template chain.  Compiles to nothing on
 * MSVC and to the gcc/clang builtin elsewhere. */
#ifndef _MSC_VER
#define MDFN_ASSUME_ALIGNED(p, align) ((decltype(p))__builtin_assume_aligned((p), (align)))
#else
#define MDFN_ASSUME_ALIGNED(p, align) p
#endif

/* Compile-time host-endian flag, driven entirely by MSB_FIRST from
 * the build flags.  No runtime detection - MSB_FIRST defined means
 * big-endian, undefined means little-endian.  Used by the
 * MDFN_deXsb / ss_endian.h MDFN_enXsb templates to elide byteswap
 * branches at compile time when the requested target endian
 * matches the host. */
#ifdef MSB_FIRST
#define MDFN_ENDIANH_IS_BIGENDIAN 1
#else
#define MDFN_ENDIANH_IS_BIGENDIAN 0
#endif

#if defined(__cplusplus)
extern "C" {
#endif

/* Byte-swap an array of N 16-bit elements in place.  Unconditional -
 * does NOT depend on host endian.  Used to flip raw CD audio
 * samples when a track is flagged RawAudioMSBFirst (cdrdao TOC and
 * .wav-wrapped CHD audio tracks come in MSB order; the Saturn
 * expects native LSB).  Inline in the header so the three call
 * sites (CDAccess_Image, CDAccess_CHD, ss/cart/stv) don't pay a
 * cross-TU call.  Loop auto-vectorizes at -O2. */
static INLINE void Endian_A16_Swap(void *src, uint32_t nelements)
{
   uint32_t i;
   uint8_t *nsrc = (uint8_t *)src;

   for (i = 0; i < nelements; i++)
   {
      uint8_t tmp = nsrc[i * 2];
      nsrc[i * 2]     = nsrc[i * 2 + 1];
      nsrc[i * 2 + 1] = tmp;
   }
}

#if defined(__cplusplus)
}
#endif

/* The following encode/decode to and from unaligned little/big-
 * endian byte sequences.  Pure byte arithmetic; host endian is
 * irrelevant.  Hot in state save/load and TOC handling. */

static INLINE void MDFN_en16lsb(uint8_t *buf, uint16_t morp)
{
   buf[0] = morp;
   buf[1] = morp >> 8;
}

static INLINE void MDFN_en32lsb(uint8_t *buf, uint32_t morp)
{
   buf[0] = morp;
   buf[1] = morp >> 8;
   buf[2] = morp >> 16;
   buf[3] = morp >> 24;
}

static INLINE void MDFN_enlsb(void *buf, void *value, size_t size)
{
   if (size == 2)
      MDFN_en16lsb((uint8_t *)buf, *(uint16_t *)value);
   else if (size == 4)
      MDFN_en32lsb((uint8_t *)buf, *(uint32_t *)value);
}

static INLINE void MDFN_en16msb(uint8_t *buf, uint16_t morp)
{
   buf[0] = morp >> 8;
   buf[1] = morp;
}

static INLINE void MDFN_en32msb(uint8_t *buf, uint32_t morp)
{
   buf[0] = morp >> 24;
   buf[1] = morp >> 16;
   buf[2] = morp >> 8;
   buf[3] = morp;
}

static INLINE void MDFN_en64msb(uint8_t *buf, uint64_t morp)
{
   buf[0] = morp >> 56;
   buf[1] = morp >> 48;
   buf[2] = morp >> 40;
   buf[3] = morp >> 32;
   buf[4] = morp >> 24;
   buf[5] = morp >> 16;
   buf[6] = morp >> 8;
   buf[7] = morp >> 0;
}

static INLINE uint16_t MDFN_de16lsb(const uint8_t *morp)
{
   return morp[0] | (morp[1] << 8);
}

static INLINE uint32_t MDFN_de32lsb(const uint8_t *morp)
{
   return morp[0] | (morp[1] << 8) | (morp[2] << 16) | (morp[3] << 24);
}

static INLINE uint64_t MDFN_de64lsb(const uint8_t *morp)
{
   uint64_t ret  = (uint64_t)morp[0];
   ret |= (uint64_t)morp[1] << 8;
   ret |= (uint64_t)morp[2] << 16;
   ret |= (uint64_t)morp[3] << 24;
   ret |= (uint64_t)morp[4] << 32;
   ret |= (uint64_t)morp[5] << 40;
   ret |= (uint64_t)morp[6] << 48;
   ret |= (uint64_t)morp[7] << 56;
   return ret;
}

static INLINE uint32_t MDFN_de32msb(const uint8_t *morp)
{
   return morp[3] | (morp[2] << 8) | (morp[1] << 16) | (morp[0] << 24);
}

static INLINE uint16 MDFN_bswap16(uint16 v)
{
   return (v << 8) | (v >> 8);
}

static INLINE uint32 MDFN_bswap32(uint32 v)
{
   return (v << 24) | ((v & 0xFF00) << 8) | ((v >> 8) & 0xFF00) | (v >> 24);
}

static INLINE uint64 MDFN_bswap64(uint64 v)
{
   return (v << 56) | (v >> 56) | ((v & 0xFF00) << 40) | ((v >> 40) & 0xFF00)
        | ((uint64)MDFN_bswap32(v >> 16) << 16);
}

#ifdef __cplusplus

/* X-endian decode template.
 * `isbigendian = -1` means "host endian" (always-match branch).
 * Otherwise compared against MDFN_ENDIANH_IS_BIGENDIAN at compile
 * time; the byteswap branch is elided when host matches the
 * requested target endian. */
template<int isbigendian, typename T, bool aligned>
static INLINE T MDFN_deXsb(const void *ptr)
{
   T tmp;

   memcpy(&tmp, MDFN_ASSUME_ALIGNED(ptr, (aligned ? sizeof(T) : 1)), sizeof(T));

   if (isbigendian != -1 && isbigendian != MDFN_ENDIANH_IS_BIGENDIAN)
   {
      static_assert(sizeof(T) == 1 || sizeof(T) == 2 || sizeof(T) == 4 || sizeof(T) == 8,
            "Unsupported scalar size");

      if (sizeof(T) == 8)
         return MDFN_bswap64(tmp);
      else if (sizeof(T) == 4)
         return MDFN_bswap32(tmp);
      else if (sizeof(T) == 2)
         return MDFN_bswap16(tmp);
   }

   return tmp;
}

template<typename T, bool aligned = false>
static INLINE T MDFN_demsb(const void *ptr)
{
   return MDFN_deXsb<1, T, aligned>(ptr);
}

template<bool aligned = false>
static INLINE uint16 MDFN_de16msb(const void *ptr)
{
   return MDFN_demsb<uint16, aligned>(ptr);
}

/* neX_ptr_be: address of the little-endian 16-bit word that the
 * BIG-endian access of size T at byte_offset would land on.
 *
 * Saturn VRAM, BIOS ROM, work RAM are physically wired as 16-bit
 * big-endian, but we store them as a uint16[] array in native
 * (little-endian on x86) memory.  For an MSB-first byte read at
 * offset N, the byte we want is at index `N ^ 1` inside a uint16.
 * For a 32-bit big-endian read at aligned offset N, the two
 * uint16 words are at the natural order (no XOR needed).  This
 * helper centralises that index math; MSB_FIRST host turns it
 * into a no-op. */
template<typename T, typename X>
static INLINE uintptr_t neX_ptr_be(uintptr_t const base, const size_t byte_offset)
{
#ifdef MSB_FIRST
   return base + (byte_offset &~ (sizeof(T) - 1));
#else
   return base + (((byte_offset &~ (sizeof(T) - 1)) ^ (sizeof(X) - std::min<size_t>(sizeof(X), sizeof(T)))));
#endif
}

template<typename T, typename BT>
static INLINE void ne16_wbo_be(BT base, const size_t byte_offset, const T value)
{
   static_assert(sizeof(T) == 1 || sizeof(T) == 2 || sizeof(T) == 4, "Unsupported type size");
   static_assert(std::is_same<BT, uintptr_t>::value || std::is_convertible<BT, uint16*>::value, "Wrong base type");

   uintptr_t const ptr = neX_ptr_be<T, uint16>((uintptr_t)base, byte_offset);

   if (sizeof(T) == 4)
   {
      uint16 *const ptr16 = (uint16 *)ptr;
      ptr16[0] = value >> 16;
      ptr16[1] = value;
   }
   else
      *(T *)ptr = value;
}

template<typename T, typename BT>
static INLINE T ne16_rbo_be(BT base, const size_t byte_offset)
{
   static_assert(sizeof(T) == 1 || sizeof(T) == 2 || sizeof(T) == 4, "Unsupported type size");
   static_assert(std::is_same<BT, uintptr_t>::value || std::is_convertible<BT, const uint16*>::value, "Wrong base type");

   uintptr_t const ptr = neX_ptr_be<T, uint16>((uintptr_t)base, byte_offset);

   if (sizeof(T) == 4)
   {
      uint16 *const ptr16 = (uint16 *)ptr;
      T tmp;
      tmp  = ptr16[0] << 16;
      tmp |= ptr16[1];
      return tmp;
   }
   else
      return *(T *)ptr;
}

template<typename T, bool IsWrite, typename BT>
static INLINE void ne16_rwbo_be(BT base, const size_t byte_offset, T *value)
{
   if (IsWrite)
      ne16_wbo_be<T>(base, byte_offset, *value);
   else
      *value = ne16_rbo_be<T>(base, byte_offset);
}

#endif /* C++ only */

#endif
