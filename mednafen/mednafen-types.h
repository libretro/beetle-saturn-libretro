#ifndef __MDFN_TYPES
#define __MDFN_TYPES

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

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
  /* MDFN_FORMATSTR (function-call format-string checks) and
   * MDFN_WARN_UNUSED_RESULT used to live here.  Zero call sites
   * in this fork; dropped.  Reintroduce per call site rather
   * than as a tree-wide macro if a future user wants either. */
  #define MDFN_NOWARN_UNUSED __attribute__((unused))

#elif defined(_MSC_VER)
  #define NO_INLINE
  #define FORCE_INLINE __forceinline

#define MDFN_LIKELY(n) ((n) != 0)
#define MDFN_UNLIKELY(n) ((n) != 0)

  #define MDFN_FASTCALL

  //#define MDFN_ALIGN(n) __declspec(align(n))
#define MDFN_ALIGN(n)

  #define MDFN_NOWARN_UNUSED

#else
  #error "Not compiling with GCC nor MSVC"
  #define NO_INLINE
  #define FORCE_INLINE inline

  #define MDFN_FASTCALL

  #define MDFN_ALIGN(n)

#endif

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
//
// MDFN_UNREACHABLE tells the optimizer that a code path is dead
// (e.g. the `default:` arm of a switch over a known-enumerated
// integer).  GCC and clang expose `__builtin_unreachable()`; MSVC
// has had the equivalent `__assume(0)` since VS 2005, which long
// predates the MSVC C89 target this codebase still wants to
// compile under.  The fallback path expands to nothing, which is
// always safe -- the worst case is a bounds-check the optimizer
// could otherwise have elided.  Useful for dense switch dispatches
// where every value is accounted for, so the compiler can drop the
// jump-table bounds check (one indirect jump beats a compare +
// branch + indirect jump).
#if defined(__GNUC__) && !defined(__clang__)
 #define MDFN_HOT          __attribute__((hot))
 #define MDFN_COLD         __attribute__((cold))
 #define NO_CLONE          __attribute__((noclone))
 #define MDFN_FORCE_INLINE __attribute__((always_inline)) inline
 #define MDFN_UNREACHABLE  __builtin_unreachable()
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
 #define MDFN_UNREACHABLE  __builtin_unreachable()
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
 #define MDFN_UNREACHABLE  __assume(0)
#else
 #define MDFN_HOT
 #define MDFN_COLD
 #define MDFN_HIDE
 #define NO_CLONE
 #define MDFN_FORCE_INLINE inline
 #define MDFN_UNREACHABLE  /* nothing -- compiler keeps any bounds checks */
#endif

/* MDFN_STATIC_ASSERT(condition, message) -- portable compile-time
 * assertion.  Uses the language-native form where available (C++11
 * keyword or C11 _Static_assert) and falls back to the negative
 * array-bound trick for older compilers.  The fallback consumes a
 * unique tag per call via __COUNTER__ where supported, otherwise
 * __LINE__ (which means two MDFN_STATIC_ASSERTs on the same source
 * line would collide on those compilers -- don't do that).
 *
 * C++ pre-C++11 and C pre-C11 are both supported; the negative-
 * array-bound trick is portable to C89 and to C++98.  GCC accepts
 * the trick at file scope and at block scope. */
#if defined(__cplusplus) && __cplusplus >= 201103L
 #define MDFN_STATIC_ASSERT(c_, msg_) static_assert((c_), msg_)
#elif !defined(__cplusplus) && defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
 #define MDFN_STATIC_ASSERT(c_, msg_) _Static_assert((c_), msg_)
#else
 /* C89 / MSVC-89 / pre-C++11 fallback.
  *
  * Uniqueness via __LINE__ -- specifically NOT __COUNTER__.  Several
  * sites in this codebase encode counter checkpoints in their
  * assertion conditions (e.g.
  *   `MDFN_STATIC_ASSERT(__COUNTER__ == 5000, "...")`
  *   `MDFN_STATIC_ASSERT(__COUNTER__ == 5000 + 393 + 512 + 1, "...")`
  * in sh7095_ops.inc and sh7095.inc).  These rely on __COUNTER__
  * being incremented exactly once per textual occurrence of
  * __COUNTER__ in the source.
  *
  * If MDFN_STATIC_ASSERT itself expanded __COUNTER__ for its typedef
  * name, every assertion would silently consume one extra counter
  * value past what its own condition expanded -- so checkpoints
  * placed N asserts after the previous checkpoint would drift by N.
  *
  * __LINE__ does not have this problem (it's a property of the
  * source location, not a side-effect-bearing macro), and there
  * is no instance of two MDFN_STATIC_ASSERT() invocations on the
  * same source line anywhere in the codebase. */
 #define MDFN_STATIC_ASSERT_CAT2_(a_, b_) a_##b_
 #define MDFN_STATIC_ASSERT_CAT_(a_, b_)  MDFN_STATIC_ASSERT_CAT2_(a_, b_)
 #define MDFN_STATIC_ASSERT(c_, msg_) \
   typedef char MDFN_STATIC_ASSERT_CAT_(_mdfn_static_assert_, __LINE__) \
        [(c_) ? 1 : -1] MDFN_NOWARN_UNUSED
#endif

#endif
