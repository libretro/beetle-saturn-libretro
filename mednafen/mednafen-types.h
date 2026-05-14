#ifndef __MDFN_TYPES
#define __MDFN_TYPES

#include <assert.h>
#include <stdint.h>

#ifdef __cplusplus
#include <type_traits>
#endif

/* int{8,16,32,64} / uint{8,16,32,64} were typedef'd here to the
 * <stdint.h> fixed-width types.  All call sites now use the
 * <stdint.h> spellings (uint8_t, ...) directly; <stdint.h> above
 * provides them.  The aliases are gone. */

#include <retro_inline.h>

#ifdef __GNUC__
#define MDFN_UNLIKELY(n) __builtin_expect((n) != 0, 0)
#define MDFN_LIKELY(n) __builtin_expect((n) != 0, 1)

  #define NO_INLINE __attribute__((noinline))
  #define FORCE_INLINE inline __attribute__((always_inline))

  #if defined(__386__) || defined(__i386__) || defined(__i386) || defined(_M_IX86) || defined(_M_I386)
    #define MDFN_FASTCALL __attribute__((fastcall))
  #else
    #define MDFN_FASTCALL
  #endif

  #define MDFN_ALIGN(n)	__attribute__ ((aligned (n)))
  #define MDFN_FORMATSTR(a,b,c) __attribute__ ((format (a, b, c)));
  #define MDFN_WARN_UNUSED_RESULT __attribute__ ((warn_unused_result))
  #define MDFN_NOWARN_UNUSED __attribute__((unused))

#elif defined(_MSC_VER)
  #define NO_INLINE
  #define FORCE_INLINE __forceinline

#define MDFN_LIKELY(n) ((n) != 0)
#define MDFN_UNLIKELY(n) ((n) != 0)

  #define MDFN_FASTCALL

  //#define MDFN_ALIGN(n) __declspec(align(n))
#define MDFN_ALIGN(n)

  #define MDFN_FORMATSTR(a,b,c)

  #define MDFN_WARN_UNUSED_RESULT
  #define MDFN_NOWARN_UNUSED

#else
  #error "Not compiling with GCC nor MSVC"
  #define NO_INLINE
  #define FORCE_INLINE inline

  #define MDFN_FASTCALL

  #define MDFN_ALIGN(n)

  #define MDFN_FORMATSTR(a,b,c)

  #define MDFN_WARN_UNUSED_RESULT

#endif


typedef struct
{
 union
 {
  struct
  {
   #ifdef MSB_FIRST
   uint8_t   High;
   uint8_t   Low;
   #else
   uint8_t   Low;
   uint8_t   High;
   #endif
  } Union8;
  uint16_t Val16;
 };
} Uuint16;

typedef struct
{
 union
 {
  struct
  {
   #ifdef MSB_FIRST
   Uuint16   High;
   Uuint16   Low;
   #else
   Uuint16   Low;
   Uuint16   High;
   #endif
  } Union16;
  uint32_t  Val32;
 };
} Uuint32;

// Compiler hint macros previously expanded to nothing. The codebase
// already tags 41 functions with MDFN_HOT and 164 with MDFN_COLD --
// activating the underlying attributes is a free codegen improvement
// on GCC and Clang (changes branch-prediction defaults, hot/cold
// section placement, and inlining decisions). MSVC has no direct
// equivalents; the empty defines are kept for that path. Same idea
// for MDFN_HIDE (symbol visibility) and NO_CLONE (cloning suppression).
//
// MDFN_FORCE_INLINE is new -- there were ad-hoc places in the renderer
// audit that wanted an unconditional inline; this gives a portable
// spelling for it. Maps to __forceinline on MSVC.
//
// MDFN_HIDE specifically needs an extra guard: the
// __attribute__((visibility)) form is only meaningful on ELF / Mach-O
// targets (Linux, macOS, BSD). On Windows PE/COFF -- including MinGW
// builds, which still use GCC -- the attribute is parsed but ignored
// with a warning at every use site (and we have 60+ of them). Disable
// it cleanly for that target so MinGW builds compile quietly.
//
// NO_CLONE expands to __attribute__((noclone)), which is GCC-only --
// clang defines __GNUC__ for compatibility but does not implement
// noclone and emits -Wunknown-attributes at every use site. Keep
// noclone on real GCC and elide it on clang; the other attributes
// (hot, cold, always_inline, visibility) are supported by both.
#if defined(__GNUC__) && !defined(__clang__)
 #define MDFN_HOT          __attribute__((hot))
 #define MDFN_COLD         __attribute__((cold))
 #define NO_CLONE          __attribute__((noclone))
 #define MDFN_FORCE_INLINE __attribute__((always_inline)) inline
 #if defined(_WIN32) || defined(__CYGWIN__)
  #define MDFN_HIDE
 #else
  #define MDFN_HIDE        __attribute__((visibility("hidden")))
 #endif
#elif defined(__clang__)
 #define MDFN_HOT          __attribute__((hot))
 #define MDFN_COLD         __attribute__((cold))
 #define NO_CLONE
 #define MDFN_FORCE_INLINE __attribute__((always_inline)) inline
 #if defined(_WIN32) || defined(__CYGWIN__)
  #define MDFN_HIDE
 #else
  #define MDFN_HIDE        __attribute__((visibility("hidden")))
 #endif
#elif defined(_MSC_VER)
 #define MDFN_HOT
 #define MDFN_COLD
 #define MDFN_HIDE
 #define NO_CLONE
 #define MDFN_FORCE_INLINE __forceinline
#else
 #define MDFN_HOT
 #define MDFN_COLD
 #define MDFN_HIDE
 #define NO_CLONE
 #define MDFN_FORCE_INLINE inline
#endif

#ifdef __cplusplus
template<typename T> typename std::remove_all_extents<T>::type* MDAP(T* v) { return (typename std::remove_all_extents<T>::type*)v; }
#include "error.h"
#endif

#endif
