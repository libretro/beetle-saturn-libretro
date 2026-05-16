/******************************************************************************/
/* Mednafen Sega Saturn Emulation Module                                      */
/******************************************************************************/
/* ss_init.c - FastMemMap + SH-2 event-system orchestration.
**             Phase-7c extraction from ss.cpp.  Holds the page-mapped
**             SH7095_FastMap table that every SH-2 memory access goes
**             through, the FMIsWriteable bitmap, the event ring, the
**             event-handler dispatch loop, and the public-ABI entry
**             points that fire events from outside ss.cpp.
**
**             Nothing here touches the SH-2 / SCU / SCSP / M68K class
**             interiors -- the few cross-language entry points needed
**             (CART_GetEventHandler, the various subsystem _Update
**             handlers stuffed into event_handlers[] at boot) are
**             already extern "C" function pointers.
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

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <assert.h>

#include <mednafen/mednafen-types.h>
#include "ss.h"
#include "ss_init.h"

#include "scu.h"     /* SCU_UpdateDMA, SCU_UpdateDSP, SS_EVENT_SCU_* */
#include "smpc.h"    /* SMPC_Update */
#include "vdp1.h"    /* VDP1_Update */
#include "vdp2.h"    /* VDP2_Update */
#include "cdb.h"     /* CDB_Update */
#include "sound.h"   /* SOUND_Update */
#include "cart.h"    /* CART_GetEventHandler */

#include "../mempatcher.h"

/* ===================================================================
 * FastMemMap
 * =================================================================== */

uintptr_t SH7095_FastMap[1U << (32 - SH7095_EXT_MAP_GRAN_BITS)];
uint32_t  FMIsWriteable[FMISWRITEABLE_BITS / 32];

static uint16_t fmap_dummy[(1U << SH7095_EXT_MAP_GRAN_BITS) / sizeof(uint16_t)];

static void SetFastMemMap(uint32_t Astart, uint32_t Aend, uint16_t* ptr, uint32_t length, bool is_writeable)
{
 const uint64_t Abound = (uint64_t)Aend + 1;
 uint64_t A;

 assert((Astart & ((1U << SH7095_EXT_MAP_GRAN_BITS) - 1)) == 0);
 assert((Abound & ((1U << SH7095_EXT_MAP_GRAN_BITS) - 1)) == 0);
 assert((length & ((1U << SH7095_EXT_MAP_GRAN_BITS) - 1)) == 0);
 assert(length > 0);
 assert(length <= (Abound - Astart));

 for(A = Astart; A < Abound; A += (1U << SH7095_EXT_MAP_GRAN_BITS))
 {
  uintptr_t tmp = (uintptr_t)ptr + ((A - Astart) % length);

  if(A < (1U << 27))
   FMIsWriteable_set(A >> SH7095_EXT_MAP_GRAN_BITS, is_writeable);

  SH7095_FastMap[A >> SH7095_EXT_MAP_GRAN_BITS] = tmp - A;
 }
}

bool InitFastMemMap(void)
{
 unsigned i;
 uint64_t A;

 for(i = 0; i < sizeof(fmap_dummy) / sizeof(fmap_dummy[0]); i++)
 {
  fmap_dummy[i] = 0;
 }

 FMIsWriteable_reset();

 /* MDFNMP_Init returns false on RAMPtrs calloc failure; the rest of
  * InitFastMemMap and InitCommon downstream (MDFNMP_RegSearchable,
  * MDFNMP_AddRAM, the cheat search machinery) assume RAMPtrs is a
  * live array, so a NULL there would crash on the first patch /
  * cheat install.  Propagate the failure instead. */
 if(!MDFNMP_Init(1ULL << SH7095_EXT_MAP_GRAN_BITS, (1ULL << 27) / (1ULL << SH7095_EXT_MAP_GRAN_BITS)))
  return false;

 for(A = 0; A < 1ULL << 32; A += (1U << SH7095_EXT_MAP_GRAN_BITS))
 {
  SH7095_FastMap[A >> SH7095_EXT_MAP_GRAN_BITS] = (uintptr_t)fmap_dummy - A;
 }

 return true;
}

void SS_SetPhysMemMap(uint32_t Astart, uint32_t Aend, uint16_t* ptr, uint32_t length, bool is_writeable)
{
 uint32_t Abase;

 assert(Astart < 0x20000000);
 assert(Aend < 0x20000000);

 if(!ptr)
 {
  ptr = fmap_dummy;
  length = sizeof(fmap_dummy);
 }

 for(Abase = 0; Abase < 0x40000000; Abase += 0x20000000)
  SetFastMemMap(Astart + Abase, Aend + Abase, ptr, length, is_writeable);
}

uint8_t CheatMemRead(uint32_t A)
{
 A &= (1U << 27) - 1;

 /* ne16_rbo_be<uint8_t>(base, A) folded - byte read from BE bus
  * over uint16_t fast-map slot. */
#ifdef MSB_FIRST
 return ((const uint8_t*)SH7095_FastMap[A >> SH7095_EXT_MAP_GRAN_BITS])[A];
#else
 return ((const uint8_t*)SH7095_FastMap[A >> SH7095_EXT_MAP_GRAN_BITS])[A ^ 1];
#endif
}

/* ===================================================================
 * Event system
 * =================================================================== */

int Running;
__attribute__((aligned(16))) event_list_entry events[SS_EVENT__SIMD_COUNT];
ss_event_handler event_handlers[SS_EVENT__COUNT];
sscpu_timestamp_t next_event_ts;

/* NO_INLINE keeps the body out of any caller's no-unroll pragma scope
 * so -O2 auto-vectorizes the reduction (smin/sminv on aarch64). */
NO_INLINE sscpu_timestamp_t FindNextEventTS(void)
{
 sscpu_timestamp_t m = SS_EVENT_DISABLED_TS;
 unsigned i;
 for(i = 0; i < SS_EVENT__SIMD_COUNT; i++)
  m = ((m) < (events[i].event_time) ? (m) : (events[i].event_time));
 return m;
}

/* Phase-7a: was `template<unsigned c> static sscpu_timestamp_t
 * SH_DMA_EventHandler(sscpu_timestamp_t et)` with c instantiated
 * to 0 (master SH-2) and 1 (slave SH-2).  The C++ body called
 * CPU[c].DMA_Update(et); the C-side rewrite reaches the same
 * dispatch through the extern "C" wrappers SH7095_M_DMA_Update /
 * SH7095_S_DMA_Update (defined in ss.cpp, where the SH7095 class
 * type still lives -- this gets retired once the SH7095 class is
 * fully converted to a C struct in a later phase). */
extern int32_t SH7095_M_DMA_Update(int32_t et);
extern int32_t SH7095_S_DMA_Update(int32_t et);
extern uint32_t SH7095_BusLock;

#define SH_DMA_EVENT_HANDLER_BODY(UPDATE_FN)                                                       \
{                                                                                                  \
 if(et < SH7095_mem_timestamp)                                                                     \
  return SH7095_mem_timestamp;                                                                     \
                                                                                                   \
 /* Must come after the (et < SH7095_mem_timestamp) check. */                                      \
 if(MDFN_UNLIKELY(SH7095_BusLock))                                                                 \
  return et + 1;                                                                                   \
                                                                                                   \
 return UPDATE_FN(et);                                                                             \
}

static sscpu_timestamp_t SH_DMA_EventHandler_M(sscpu_timestamp_t et) SH_DMA_EVENT_HANDLER_BODY(SH7095_M_DMA_Update)
static sscpu_timestamp_t SH_DMA_EventHandler_S(sscpu_timestamp_t et) SH_DMA_EVENT_HANDLER_BODY(SH7095_S_DMA_Update)

#undef SH_DMA_EVENT_HANDLER_BODY

void InitEvents(void)
{
 unsigned i;

 /* SYNFIRST/SYNLAST and padding slots stay disabled so the min-reduction
  * ignores them; only [SYNFIRST+1, SYNLAST) hold real events. */
 for(i = 0; i < SS_EVENT__SIMD_COUNT; i++)
 {
  if(i == SS_EVENT__SYNFIRST || i == SS_EVENT__SYNLAST || i >= SS_EVENT__COUNT)
   events[i].event_time = SS_EVENT_DISABLED_TS;
  else
   events[i].event_time = 0;
 }

 for(i = 0; i < SS_EVENT__COUNT; i++)
  event_handlers[i] = NULL;

 event_handlers[SS_EVENT_SH2_M_DMA] = &SH_DMA_EventHandler_M;
 event_handlers[SS_EVENT_SH2_S_DMA] = &SH_DMA_EventHandler_S;

 event_handlers[SS_EVENT_SCU_DMA] = SCU_UpdateDMA;
 event_handlers[SS_EVENT_SCU_DSP] = SCU_UpdateDSP;
 /*event_handlers[SS_EVENT_SCU_INT] = SCU_UpdateInt;*/

 event_handlers[SS_EVENT_SMPC] = SMPC_Update;

 event_handlers[SS_EVENT_VDP1] = VDP1_Update;
 event_handlers[SS_EVENT_VDP2] = VDP2_Update;

 event_handlers[SS_EVENT_CDB] = CDB_Update;

 event_handlers[SS_EVENT_SOUND] = SOUND_Update;

 event_handlers[SS_EVENT_CART] = CART_GetEventHandler();

 event_handlers[SS_EVENT_MIDSYNC] = MidSync;
 /*  */
 SS_SetEventNT(&events[SS_EVENT_MIDSYNC], SS_EVENT_DISABLED_TS);
}

void RebaseTS(const sscpu_timestamp_t timestamp)
{
 unsigned i;
 for(i = SS_EVENT__SYNFIRST + 1; i < SS_EVENT__SYNLAST; i++)
 {
  assert(events[i].event_time > timestamp);

  if(events[i].event_time != SS_EVENT_DISABLED_TS)
   events[i].event_time -= timestamp;
 }

 next_event_ts = FindNextEventTS();
}

void SS_SetEventNT(event_list_entry* e, const sscpu_timestamp_t next_timestamp)
{
 const sscpu_timestamp_t old_t = e->event_time;
 e->event_time = next_timestamp;

 if(MDFN_UNLIKELY(Running <= 0))
  next_event_ts = 0;
 else if(old_t == next_event_ts)
  next_event_ts = FindNextEventTS();
 else if(next_timestamp < next_event_ts)
  next_event_ts = next_timestamp;
}

/* EventHandler was static INLINE; promoted to TU-external in phase 7c
 * because ss.cpp's RunLoop template body calls it.  Keeping it INLINE
 * (declared as such in ss_init.h via the prototype) lets gcc/LTO
 * fold it back into the hot loop at link time. */
bool EventHandler(const sscpu_timestamp_t timestamp)
{
 sscpu_timestamp_t best_t;
 /* next_event_ts is forced to 0 (sentinel) when Running <= 0 to make
  * CheckEventsByMemTS trip and unwind RunLoop. Don't enter the dispatch
  * loop in that state -- best_t = 0 wouldn't match any
  * events[i].event_time and the inner scan would walk off the end. */
 if(MDFN_UNLIKELY(Running <= 0))
  return false;
 best_t = next_event_ts;
 while(best_t <= timestamp)
 {
  unsigned best_i = SS_EVENT__SYNFIRST + 1;
  while(events[best_i].event_time != best_t)
   best_i++;
  events[best_i].event_time = event_handlers[best_i](best_t);
  best_t = FindNextEventTS();
 }

 next_event_ts = (Running > 0) ? best_t : 0;
 return Running > 0;
}

static void CheckEventsByMemTS_Sub(void)
{
 EventHandler(SH7095_mem_timestamp);
}

void CheckEventsByMemTS(void)
{
 if(MDFN_UNLIKELY(SH7095_mem_timestamp >= next_event_ts))
  CheckEventsByMemTS_Sub();
}

void SS_RequestEHLExit(void)
{
 if(Running)
 {
  Running = -1;
  next_event_ts = 0;
 }
}

void SS_RequestMLExit(void)
{
 Running = 0;
 next_event_ts = 0;
}
