/******************************************************************************/
/* Mednafen Sega Saturn Emulation Module                                      */
/******************************************************************************/
/* ss_init.h:  Phase-7c cross-TU declarations between ss.cpp and ss_init.c.
**             Hosts the FastMemMap state (SH-2 page-mapped uintptr_t array
**             accessed by every memory op in sh7095.inc) and the SH-2 event
**             system (events[], event_handlers[], next_event_ts) plus their
**             public-ABI entry points.
**
**  Copyright (C) 2015-2023 Mednafen Team
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

#ifndef __MDFN_SS_INIT_H
#define __MDFN_SS_INIT_H

#include <stdint.h>
#include <boolean.h>

#include "../mednafen-types.h"
#include <retro_inline.h>
#include <string.h>

#include "ss.h"     /* event_list_entry, ss_event_handler, sscpu_timestamp_t,
                     * SS_EVENT__* enum, SS_EVENT__SIMD_COUNT */

/* Variable externs sit outside the extern "C" block: ss.h already declares
 * `events` and `SH7095_mem_timestamp` with default (C++) linkage, and a
 * mismatch with C linkage trips a "conflicting declaration" diagnostic.
 * For top-level variables the symbol name is the same either way at the
 * link layer; the linkage attribute only matters for the C++ frontend's
 * consistency check. */

/* SH-2 page-mapped fast memory map.  Indexed by
 * (A >> SH7095_EXT_MAP_GRAN_BITS); each slot stores an offset such
 * that (uint8_t*)slot[A>>GRAN] + A reaches the physical-RAM byte.
 * sh7095.inc reads this on every external memory access. */
#define SH7095_EXT_MAP_GRAN_BITS 16
extern uintptr_t SH7095_FastMap[1U << (32 - SH7095_EXT_MAP_GRAN_BITS)];

/* Per-FastMap-slot writeable bit (packed 1-bit-per-slot bitmap).
 * sh7095.inc reads the array directly for the CacheBypassHack
 * path; CheatMemWrite gates direct cheat writes on it; the
 * SetFastMemMap setup path writes it.  Was std::bitset; replaced
 * by uint32_t[] packed bitfield. */
#define FMISWRITEABLE_BITS (1U << (27 - SH7095_EXT_MAP_GRAN_BITS))
extern uint32_t FMIsWriteable[FMISWRITEABLE_BITS / 32];

/* Event system globals.
 *
 * Running:
 *   0 at end of (emulation) frame
 *   1 during normal execution
 *  -1 when we need to temporarily break out of the execution loop to
 *     e.g. handle turning the slave CPU on or off, which can't safely
 *     happen from an event handler due to it potentially being called
 *     from deep within the memory read/write functions.
 *
 * events:        the timestamp-tagged event ring; min-reduced by
 *                FindNextEventTS to find the next due event.
 *                (Already declared in ss.h with default linkage.)
 *
 * event_handlers: indexed by SS_EVENT_*; each slot is the callback
 *                 the event ring invokes when the matching event
 *                 fires.  Populated by InitEvents at boot.
 *
 * next_event_ts: cached next-due timestamp (the min-reduction's
 *                most recent result).
 */
extern int Running;
extern ss_event_handler event_handlers[SS_EVENT__COUNT];
extern sscpu_timestamp_t next_event_ts;

#ifdef __cplusplus
extern "C" {
#endif

static INLINE bool FMIsWriteable_get(uint32_t i)
{
 return (FMIsWriteable[i >> 5] >> (i & 31)) & 1;
}

static INLINE void FMIsWriteable_set(uint32_t i, bool v)
{
 const uint32_t mask = (uint32_t)1 << (i & 31);
 if(v)
  FMIsWriteable[i >> 5] |= mask;
 else
  FMIsWriteable[i >> 5] &= ~mask;
}

static INLINE void FMIsWriteable_reset(void)
{
 memset(FMIsWriteable, 0, sizeof(FMIsWriteable));
}

/* Event-handler dispatch (called from scu.inc's memory write paths
 * to drain due events before continuing). */
void CheckEventsByMemTS(void);

/* Min-reduction over events[].event_time to find the next due
 * timestamp.  Kept NO_INLINE so callers' optimisation pragmas
 * (e.g. RunLoop's no-unroll-loops scope) don't suppress the
 * SIMD min/min-across-lanes idiom gcc emits for the reduction. */
sscpu_timestamp_t FindNextEventTS(void) MDFN_HOT;

/* Drain due events up to `timestamp`.  Called from RunLoop's
 * inner-inner loop; returns whether the outer loop should
 * continue running (Running > 0). */
bool EventHandler(const sscpu_timestamp_t timestamp);

/* Event-system maintenance entry points. */
void   InitEvents(void)                          MDFN_COLD;
void   RebaseTS(const sscpu_timestamp_t timestamp);

/* Fast-mem-map maintenance entry points. */
bool    InitFastMemMap(void)                                            MDFN_COLD;
uint8_t CheatMemRead(uint32_t A)                                        MDFN_COLD;

/* MidSync lives in ss.cpp (uses statics shared with Emulate); InitEvents
 * needs its address to install in the event-handler table.  The phase-7c
 * extraction promoted it from file-static to TU-external so the table
 * setup can take &MidSync from a different TU. */
sscpu_timestamp_t MidSync(const sscpu_timestamp_t timestamp);

#ifdef __cplusplus
}
#endif

#endif
