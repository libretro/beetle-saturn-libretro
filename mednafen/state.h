/******************************************************************************/
/* Mednafen - Multi-system Emulator                                           */
/******************************************************************************/
/* state.h:
**  Copyright (C) 2005-2017 Mednafen Team
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

#ifndef __MDFN_STATE_H
#define __MDFN_STATE_H

#include <stdint.h>
#include <stddef.h>
#include <retro_inline.h>
#include <boolean.h>

typedef struct
{
   uint8_t *data;
   uint8_t *data_frontend; // never realloc'd
   uint32_t loc;
   uint32_t len;
   uint32_t malloced;
   uint32_t initial_malloc; // A setting!
   /* Sticky 'a previous smem_write hit OOM' flag.  Latched true by
    * smem_write on grow-allocation failure; checked by MDFNSS_SaveSM
    * at the end of a save pass to fail loudly instead of returning
    * a half-written state buffer.  MDFNSS_SaveSM clears this at
    * entry; the libretro.c callers (retro_serialize* /
    * retro_get_memory_*) don't need to initialise it -- their
    * StateMem instances are stack-locals reused only by SaveSM. */
   bool write_failed;
} StateMem;

/* The external functions declared in this header keep C linkage so the
   SS core can be a mix of C and C++ translation units while it is
   converted. The static INLINE helpers further down are intentionally
   NOT inside this extern "C" block: SF_FORCE_A* is an overload set in
   C++, and overloading is incompatible with C linkage. */
#ifdef __cplusplus
extern "C" {
#endif

int MDFNSS_SaveSM(void *st, uint32_t ver);
int MDFNSS_LoadSM(void *st, uint32_t ver);

#ifdef __cplusplus
}
#endif

// Flag for a single, >= 1 byte native-endian variable
#define MDFNSTATE_RLSB            0x80000000

// 32-bit native-endian elements
#define MDFNSTATE_RLSB32          0x40000000

// 16-bit native-endian elements
#define MDFNSTATE_RLSB16          0x20000000

// 64-bit native-endian elements
#define MDFNSTATE_RLSB64          0x10000000

#define MDFNSTATE_BOOL		  0x08000000

typedef struct
{
	const char* name;	// Name;
	void* data;		// Pointer to the variable/array
	uint32_t size;		// Length, in bytes, of the data to be saved EXCEPT:
				//  In the case of 'bool' it is the number of bool elements to save(bool is not always 1-byte).
				// If 0, the subchunk isn't saved.
	uint32_t type;		// Type/element size; 0(bool), 1, 2, 4, 8
	uint32_t repcount;
	uint32_t repstride;
} SFORMAT;

/* SF_FORCE_A{8,16,32,64}: identity functions whose only purpose is to
   pin the argument to an exact pointer width at the call site, so a
   mismatched pointer fails to compile. They generate no code.

   In C++ they are overload sets (signed + unsigned per width). In C
   they are _Generic selections that accept only the matching
   signed/unsigned pointer pair and otherwise produce a compile error,
   giving the C side the same width-checking the C++ overloads gave. */
#ifdef __cplusplus
static INLINE int8_t* SF_FORCE_A8(int8_t* p) { return p; }
static INLINE uint8_t* SF_FORCE_A8(uint8_t* p) { return p; }

static INLINE int16_t* SF_FORCE_A16(int16_t* p) { return p; }
static INLINE uint16_t* SF_FORCE_A16(uint16_t* p) { return p; }

static INLINE int32_t* SF_FORCE_A32(int32_t* p) { return p; }
static INLINE uint32_t* SF_FORCE_A32(uint32_t* p) { return p; }

static INLINE int64_t* SF_FORCE_A64(int64_t* p) { return p; }
static INLINE uint64_t* SF_FORCE_A64(uint64_t* p) { return p; }
#else
#define SF_FORCE_A8(p)  _Generic((p), int8_t*:  (p), uint8_t*:  (p))
#define SF_FORCE_A16(p) _Generic((p), int16_t*: (p), uint16_t*: (p))
#define SF_FORCE_A32(p) _Generic((p), int32_t*: (p), uint32_t*: (p))
#define SF_FORCE_A64(p) _Generic((p), int64_t*: (p), uint64_t*: (p))
#endif

/* SFORMAT_BUILD_: the single, plain-C implementation that builds one
   SFORMAT entry. Both the C and C++ macro front-ends forward to this.

   elem_size and is_bool are the only pieces of per-type information the
   old SFBASE_ template obtained from its template parameter; the macros
   below now compute them lexically at the call site (sizeof, plus a
   bool test), so this function needs no type information of its own.

   Output layout is identical to the previous template SFBASE_:
     bool   -> size = element count,        type = 0
     other  -> size = sizeof(elem) * count, type = sizeof(elem)
   and ret.data carries the same repbase-relative offset. */
static INLINE SFORMAT SFORMAT_BUILD_(void* iv, uint32_t icount,
      uint32_t totalcount, size_t repstride, void* repbase,
      const char* name, size_t elem_size, int is_bool)
{
   SFORMAT ret;

   ret.data      = iv ? (char*)repbase + ((char*)iv - (char*)repbase) : (void*)0;
   ret.name      = name;
   ret.repcount  = totalcount - 1;
   ret.repstride = (uint32_t)repstride;

   if(is_bool)
   {
      ret.size = icount;
      ret.type = 0;
   }
   else
   {
      ret.size = (uint32_t)(elem_size * icount);
      ret.type = (uint32_t)elem_size;
   }

   return ret;
}

/* SF_IS_BOOL_(x): 1 if the lvalue x has type bool, else 0. The
   bool-ness of the element type is the one piece of information that
   genuinely needs language-specific machinery; everything else is
   plain C. In C++ it is std::is_same; in C it is _Generic. */
#ifdef __cplusplus
#include <type_traits>
#define SF_IS_BOOL_(x) ((int)std::is_same<__typeof__(x), bool>::value)
#else
#define SF_IS_BOOL_(x) _Generic((x), bool: 1, default: 0)
#endif

/* SFEXP_*: helpers expand to the explicit-element-size SFORMAT_BUILD_
   call. _S forms take a pointer whose pointee is the element (used by
   SFVAR for scalars and by SFPTR* for arrays); the element size and
   bool-ness come from *(p). */
#define SFEXP_S6_(p, ic, tc, rs, rb, nm) \
   SFORMAT_BUILD_((void*)(p), (ic), (tc), (rs), (void*)(rb), (nm), \
                  sizeof(*(p)), SF_IS_BOOL_(*(p)))

#define SFEXP_S3_(p, ct, nm) \
   SFORMAT_BUILD_((void*)(p), (ct), 1, 0, (void*)(p), (nm), \
                  sizeof(*(p)), SF_IS_BOOL_(*(p)))

/* SFVARN(x, ...): x is a scalar lvalue. After the array-typed call
   sites were migrated to the SFPTR*N forms, SFVAR/SFVARN only ever
   receive scalars, so &(x) points at a single element and the count
   is 1. The variadic tail is either just the name (1-arg SFVAR) or
   totalcount/repstride/repbase + name (4-arg repeat form). */
#define SFVARN(x, ...)	SFEXP_S6_DISPATCH_(&(x), __VA_ARGS__)

/* dispatch on the SFVARN tail length:
     SFVARN(x, "nm")               -> 1 trailing arg
     SFVARN(x, tc, rs, rb, "nm")   -> 4 trailing args                  */
#define SFEXP_S6_DISPATCH_(p, ...) \
   SFEXP_S6_PICK_(__VA_ARGS__, SFEXP_S6_T4_, x, x, SFEXP_S6_T1_)(p, __VA_ARGS__)
#define SFEXP_S6_PICK_(a, b, c, d, NAME, ...) NAME
#define SFEXP_S6_T1_(p, nm)               SFEXP_S6_(p, 1, 1, 0, p, nm)
#define SFEXP_S6_T4_(p, tc, rs, rb, nm)   SFEXP_S6_(p, 1, tc, rs, rb, nm)

#define SFVAR1_(x)	   SFVARN((x), #x)
#define SFVAR4_(x, tc, rs, rb) SFVARN((x), tc, rs, rb, #x)
#define SFVAR_(a, b, c, d, e, ...)	e
#define SFVAR(...) 	SFVAR_(__VA_ARGS__, SFVAR4_, SFVAR3_, SFVAR2_, SFVAR1_, SFVAR0_)(__VA_ARGS__)

/* SFPTR*N / SFPTR*: x is a pointer to the (innermost) element. The
   SF_FORCE_A* wrapper pins the pointer width; SFEXP_S3_/S6_ take the
   element size and bool-ness from *(x). The N forms take an explicit
   name; the non-N forms stringize x. The variadic tail is either
   (count) / (count,"nm") or (count,tc,rs,rb) / (count,tc,rs,rb,"nm"). */
#define SFPTRXN_(p, ...) \
   SFEXP_SX_PICK_(__VA_ARGS__, SFEXP_S6_T5_, SFEXP_S6_T4N_, x, SFEXP_S3_2_, SFEXP_S3_1_)(p, __VA_ARGS__)
#define SFEXP_SX_PICK_(a, b, c, d, e, NAME, ...) NAME
#define SFEXP_S3_1_(p, ct)                  SFEXP_S3_(p, ct, #p)
#define SFEXP_S3_2_(p, ct, nm)              SFEXP_S3_(p, ct, nm)
#define SFEXP_S6_T4N_(p, ct, tc, rs, rb)    SFEXP_S6_(p, ct, tc, rs, rb, #p)
#define SFEXP_S6_T5_(p, ct, tc, rs, rb, nm) SFEXP_S6_(p, ct, tc, rs, rb, nm)

#define SFPTR8N(x, ...)		SFPTRXN_(SF_FORCE_A8(x), __VA_ARGS__)
#define SFPTR8(x, ...)		SFPTRXN_(SF_FORCE_A8(x), __VA_ARGS__, #x)

#define SFPTRBN(x, ...)		SFPTRXN_((x), __VA_ARGS__)

#define SFPTR16N(x, ...)	SFPTRXN_(SF_FORCE_A16(x), __VA_ARGS__)
#define SFPTR16(x, ...)		SFPTRXN_(SF_FORCE_A16(x), __VA_ARGS__, #x)

#define SFPTR32N(x, ...)	SFPTRXN_(SF_FORCE_A32(x), __VA_ARGS__)
#define SFPTR32(x, ...)		SFPTRXN_(SF_FORCE_A32(x), __VA_ARGS__, #x)

#define SFPTR64N(x, ...)	SFPTRXN_(SF_FORCE_A64(x), __VA_ARGS__)

#define SFLINK(x) { (const char*)0, (x), ~0U, 0, 0, 0 }

#define SFEND { (const char*)0, (void*)0, 0, 0, 0, 0 }

// State-Section Descriptor
typedef struct
{
   SFORMAT *sf;
   const char *name;
   bool optional;
} SSDescriptor;

#ifdef __cplusplus
extern "C" {
#endif

int MDFNSS_StateAction(void *st, int load, int data_only, SFORMAT *sf, const char *name, bool optional);

#ifdef __cplusplus
}
#endif

#endif
