/******************************************************************************/
/* Mednafen Sega Saturn Emulation Module                                      */
/******************************************************************************/
/* ss.cpp - Saturn Core Emulation and Support Functions
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

// WARNING: Be careful with 32-bit access to 16-bit space, bus locking, etc. in respect to DMA and event updates(and where they can occur).

#include "../mednafen.h"
#include "../mempatcher.h"
#include "../git.h"
#include "../general.h"
#include "../cdrom/cdromif.h"
#include "../cdstream.h"
#include "../hash/sha256.h"
#include "../hash/md5.h"
#include "ss.h"

#include "../../disc.h"

#include <retro_miscellaneous.h>

extern MDFNGI EmulatedSS;
extern uint32_t IBufferCount;

#include "ss.h"
#include "sound.h"
#include "smpc.h"
#include "cdb.h"
#include "vdp1.h"
#include "vdp2.h"
#include "scu.h"
#include "cart.h"
#include "db.h"
#include "stvio.h"


// Libretro-specific
#ifndef RETRO_SLASH
#ifdef _WIN32
#define RETRO_SLASH "\\"
#else
#define RETRO_SLASH "/"
#endif
#endif
#include "../../libretro_settings.h"
#include "../../input.h"
extern void MDFN_MidSync(void);
extern bool is_pal;
extern char retro_base_directory[4096];

static sscpu_timestamp_t MidSync(const sscpu_timestamp_t timestamp);

uint32_t ss_horrible_hacks;

static bool NeedEmuICache;
static const uint8_t BRAM_Init_Data[0x10] = { 0x42, 0x61, 0x63, 0x6b, 0x55, 0x70, 0x52, 0x61, 0x6d, 0x20, 0x46, 0x6f, 0x72, 0x6d, 0x61, 0x74 };

static void SaveBackupRAM(void);
static void LoadBackupRAM(void);
static void SaveCartNV(void);
static void LoadCartNV(void);
static void SaveRTC(void);
static void LoadRTC(void);

static MDFN_COLD void BackupBackupRAM(void);
static MDFN_COLD void BackupCartNV(void);

#include "sh7095.h"

static uint8_t SCU_MSH2VectorFetch(void);
static uint8_t SCU_SSH2VectorFetch(void);

static void CheckEventsByMemTS(void);

SH7095 CPU[2]{ {"SH2-M", SS_EVENT_SH2_M_DMA, SCU_MSH2VectorFetch}, {"SH2-S", SS_EVENT_SH2_S_DMA, SCU_SSH2VectorFetch}};

static uint16_t BIOSROM[524288 / sizeof(uint16_t)];
uint8_t WorkRAM[2*WORKRAM_BANK_SIZE_BYTES]; // unified 2MB work ram for linear access.
// Effectively 32-bit in reality, but 16-bit here because of CPU interpreter design(regarding fastmap).
static uint16_t* WorkRAML = (uint16_t*)(WorkRAM + (WORKRAM_BANK_SIZE_BYTES*0));
static uint16_t* WorkRAMH = (uint16_t*)(WorkRAM + (WORKRAM_BANK_SIZE_BYTES*1));
// BackupRAM is exposed (no longer file-static) so libretro.cpp can hand a
// pointer to the frontend via retro_get_memory_data(RETRO_MEMORY_SAVE_RAM).
// BackupRAM_Dirty and CartNV_Dirty are sticky flags maintained here and
// drained by libretro.cpp from outside Emulate() -- see comment in
// Emulate() above. The old master-cycle delay variables are gone.
uint8_t BackupRAM[32768];
static uint8_t BackupRAM_StateHelper[32768];
bool BackupRAM_Dirty;
bool CartNV_Dirty;

#define SH7095_EXT_MAP_GRAN_BITS 16
static uintptr_t SH7095_FastMap[1U << (32 - SH7095_EXT_MAP_GRAN_BITS)];

int32_t SH7095_mem_timestamp;
static uint32_t SH7095_BusLock;
static uint32_t SH7095_DB;

#include "scu.inc"

static sha256_digest BIOS_SHA256;   // SHA-256 hash of the currently-loaded BIOS; used for save state sanity checks.
static int ActiveCartType;		// Used in save states.
/* Was std::bitset<1U << (27 - SH7095_EXT_MAP_GRAN_BITS)>. With
 * SH7095_EXT_MAP_GRAN_BITS == 16 that is 1 << 11 == 2048 bits; a
 * plain uint32_t[64] packed bitfield with three inline accessors
 * replaces it. Only ever indexed one bit at a time, set, or fully
 * cleared. */
#define FMISWRITEABLE_BITS (1U << (27 - SH7095_EXT_MAP_GRAN_BITS))
static uint32_t FMIsWriteable[FMISWRITEABLE_BITS / 32];

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
static uint16_t fmap_dummy[(1U << SH7095_EXT_MAP_GRAN_BITS) / sizeof(uint16_t)];

/*
 SH-2 external bus address map:
  CS0: 0x00000000...0x01FFFFFF (16-bit)
   0x00000000...0x000FFFFF: BIOS ROM (R)
   0x00100000...0x0017FFFF: SMPC (R/W; 8-bit mapped as 16-bit)
   0x00180000...0x001FFFFF: Backup RAM(32KiB) (R/W; 8-bit mapped as 16-bit)
   0x00200000...0x003FFFFF: Low RAM(1MiB) (R/W)
   0x01000000...0x017FFFFF: Slave FRT Input Capture Trigger (W)
   0x01800000...0x01FFFFFF: Master FRT Input Capture Trigger (W)

  CS1: 0x02000000...0x03FFFFFF (SCU managed)
   0x02000000...0x03FFFFFF: A-bus CS0 (R/W)

  CS2: 0x04000000...0x05FFFFFF (SCU managed)
   0x04000000...0x04FFFFFF: A-bus CS1 (R/W)
   0x05000000...0x057FFFFF: A-bus Dummy
   0x05800000...0x058FFFFF: A-bus CS2 (R/W)
   0x05A00000...0x05AFFFFF: SCSP RAM (R/W)
   0x05B00000...0x05BFFFFF: SCSP Registers (R/W)
   0x05C00000...0x05C7FFFF: VDP1 VRAM (R/W)
   0x05C80000...0x05CFFFFF: VDP1 FB RAM (R/W; swappable between two framebuffers, but may be temporarily unreadable at swap time)
   0x05D00000...0x05D7FFFF: VDP1 Registers (R/W)
   0x05E00000...0x05EFFFFF: VDP2 VRAM (R/W)
   0x05F00000...0x05F7FFFF: VDP2 CRAM (R/W; 8-bit writes are illegal)
   0x05F80000...0x05FBFFFF: VDP2 Registers (R/W; 8-bit writes are illegal)
   0x05FE0000...0x05FEFFFF: SCU Registers (R/W)
   0x05FF0000...0x05FFFFFF: SCU Debug/Test Registers (R/W)

  CS3: 0x06000000...0x07FFFFFF
   0x06000000...0x07FFFFFF: High RAM/SDRAM(1MiB) (R/W)
*/
//
// Never add anything to SH7095_mem_timestamp when DMAHax is true.
//
// When BurstHax is true and we're accessing high work RAM, don't add anything.
//
template<typename T, bool IsWrite>
static INLINE void BusRW_DB_CS0(const uint32_t A, uint32_t& DB, const bool BurstHax, int32_t* SH2DMAHax)
{
 //
 // Low(and kinda slow) work RAM 
 //
 if(A >= 0x00200000 && A <= 0x003FFFFF)
 {
  if(!SH2DMAHax)
   SH7095_mem_timestamp += 7;
  else
   *SH2DMAHax += 7;

  //
  // VA0 and VA1 don't map DRAM in the upper 1MiB of the 2MiB region, and return 0xFFFF(~0) on reads.
  // VA2 mirrors DRAM into the upper 1MiB for both reads and writes, which incidentally breaks "Myst" in the generator room due to
  //	it trying to load a file that's too large to fit, wrapping around and corrupting essential data in the process.
  // VA3+ behavior is untested.
  //
  // VA0/VA1 behavior is emulated here.
  //
  if(MDFN_UNLIKELY(A & 0x100000))
  {
   if(!IsWrite)
    DB = DB | 0xFFFF;

   return;
  }

  if(IsWrite)
  {
   /* ne16_wbo_be<T>(WorkRAML, byte_off, val) folded.  T can be
    * uint8_t, uint16_t, or uint32_t; sizeof(T) dispatches the right
    * write width.  Compiler folds away the dead branches per
    * template instantiation. */
   const uint32_t boff_ = A & 0xFFFFF;
   const T val_ = DB >> (((A & 1) ^ (2 - sizeof(T))) << 3);
   if(sizeof(T) == 1)
   {
#ifdef MSB_FIRST
    ((uint8_t*)WorkRAML)[boff_] = val_;
#else
    ((uint8_t*)WorkRAML)[boff_ ^ 1] = val_;
#endif
   }
   else if(sizeof(T) == 2)
    WorkRAML[boff_ >> 1] = val_;
   else /* sizeof(T) == 4 */
   {
    WorkRAML[(boff_ >> 1) + 0] = (uint32_t)val_ >> 16;
    WorkRAML[(boff_ >> 1) + 1] = val_;
   }
  }
  else
  {
   /* ne16_rbo_be<uint16_t>(WorkRAML, byte_off): aligned u16 read
    * — host-endian-stored slot, direct index. */
   DB = (DB & 0xFFFF0000) | WorkRAML[(A & 0xFFFFE) >> 1];
  }

  return;
 }

 //
 // BIOS ROM
 //
 if(A >= 0x00000000 && A <= 0x000FFFFF)
 {
  if(!SH2DMAHax)
   SH7095_mem_timestamp += 8;
  else
   *SH2DMAHax += 8;

  if(!IsWrite) 
   DB = (DB & 0xFFFF0000) | BIOSROM[(A & 0x7FFFE) >> 1];

  return;
 }

 //
 // SMPC
 //
 if(A >= 0x00100000 && A <= 0x0017FFFF)
 {
  const uint32_t SMPC_A = (A & 0x7F) >> 1;

  if(!SH2DMAHax)
  {
   // SH7095_mem_timestamp += 2;
   CheckEventsByMemTS();
  }

  if(IsWrite)
  {
   if(sizeof(T) == 2 || (A & 1))
    SMPC_Write(SH7095_mem_timestamp, SMPC_A, DB);
  }
  else
   DB = (DB & 0xFFFF0000) | 0xFF00 | SMPC_Read(SH7095_mem_timestamp, SMPC_A);

  return;
 }

 //
 // Backup RAM
 //
 if(A >= 0x00180000 && A <= 0x001FFFFF)
 {
  if(!SH2DMAHax)
   SH7095_mem_timestamp += 8;
  else
   *SH2DMAHax += 8;

  if(IsWrite)
  {
   if(sizeof(T) != 1 || (A & 1))
   {
    uint8_t* const brp = &BackupRAM[(A >> 1) & 0x7FFF];

    if(*brp != (uint8_t)DB)
    {
     *brp = (uint8_t)DB;
     BackupRAM_Dirty = true;
    }
   }
  }
  else
   DB = (DB & 0xFFFF0000) | 0xFF00 | BackupRAM[(A >> 1) & 0x7FFF];

  return;
 }

 //
 // FRT trigger region
 //
 if(A >= 0x01000000 && A <= 0x01FFFFFF)
 {
  if(!SH2DMAHax)
   SH7095_mem_timestamp += 8;
  else
   *SH2DMAHax += 8;

  if(IsWrite)
  {
   if(sizeof(T) != 1)
   {
    const unsigned c = ((A >> 23) & 1) ^ 1;

    CPU[c].SetFTI(true);
    CPU[c].SetFTI(false);
   }
  }
  return;
 }

 //
 // ST-V IOGA (gamepad / coin / service / test / EEPROM mux)
 //
 // ST-V wires the IOGA chip into the Saturn A-bus CS0 region at
 // 0x00400000-0x0040007F as 64 byte-wide registers on every other
 // address (the low bit selects within a 16-bit word). Only meaningful
 // when an ST-V cart is loaded -- on Saturn this region returns the
 // default open-bus byte.
 //
 if(A >= 0x00400000 && A <= 0x0040007F && ActiveCartType == CART_STV)
 {
  if(!SH2DMAHax)
   SH7095_mem_timestamp += 8;
  else
   *SH2DMAHax += 8;

  const uint8_t IOGA_A = (A >> 1) & 0x3F;

  if(IsWrite)
  {
   if(sizeof(T) == 2 || (A & 1))
    STVIO_WriteIOGA(SH7095_mem_timestamp, IOGA_A, (uint8_t)DB);
  }
  else
   DB = (DB & 0xFFFF0000) | 0xFF00 | STVIO_ReadIOGA(SH7095_mem_timestamp, IOGA_A);

  return;
 }

 //
 //
 //
 if(!SH2DMAHax)
  SH7095_mem_timestamp += 4;
 else
  *SH2DMAHax += 4;
}

template<typename T, bool IsWrite>
static INLINE void BusRW_DB_CS12(const uint32_t A, uint32_t& DB, const bool BurstHax, int32_t* SH2DMAHax)
{
 //
 // CS1 and CS2: SCU
 //
 if(!IsWrite)
  DB = 0;

 SCU_FromSH2_BusRW_DB<T, IsWrite>(A, &DB, SH2DMAHax);
}

template<typename T, bool IsWrite>
static INLINE void BusRW_DB_CS3(const uint32_t A, uint32_t& DB, const bool BurstHax, int32_t* SH2DMAHax)
{
 //
 // CS3: High work RAM/SDRAM, 0x06000000 ... 0x07FFFFFF
 //
 //  Timing is handled in BSC_BusWrite() and BSC_BusRead() in sh7095.inc
 //
 if(!IsWrite || sizeof(T) == 4)
 {
  /* ne16_rwbo_be<uint32_t, IsWrite>(WorkRAMH, byte_off, &DB) folded:
   * aligned uint32_t BE bus read or write over uint16_t array.  Two
   * uint16_t halves: upper at index, lower at index+1.  Same on
   * BE and LE hosts (host-endian uint16s combined in MSB-first
   * order). */
  const uint32_t idx_ = (A & 0xFFFFC) >> 1;
  if(IsWrite)
  {
   WorkRAMH[idx_ + 0] = DB >> 16;
   WorkRAMH[idx_ + 1] = DB;
  }
  else
   DB = ((uint32_t)WorkRAMH[idx_] << 16) | WorkRAMH[idx_ + 1];
 }
 else
 {
  /* ne16_wbo_be<T>(WorkRAMH, byte_off, val) folded.  T is uint8_t
   * or uint16_t here (uint32_t caught above). */
  const uint32_t boff_ = A & 0xFFFFF;
  const T val_ = DB >> (((A & 3) ^ (4 - sizeof(T))) << 3);
  if(sizeof(T) == 1)
  {
#ifdef MSB_FIRST
   ((uint8_t*)WorkRAMH)[boff_] = val_;
#else
   ((uint8_t*)WorkRAMH)[boff_ ^ 1] = val_;
#endif
  }
  else /* sizeof(T) == 2 */
   WorkRAMH[boff_ >> 1] = val_;
 }
}

//
//
//
static MDFN_COLD uint8_t CheatMemRead(uint32_t A)
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

static MDFN_COLD void CheatMemWrite(uint32_t A, uint8_t V)
{
 A &= (1U << 27) - 1;

 if(FMIsWriteable_get(A >> SH7095_EXT_MAP_GRAN_BITS))
 {
  /* ne16_wbo_be<uint8_t>(base, A, V) folded. */
#ifdef MSB_FIRST
  ((uint8_t*)SH7095_FastMap[A >> SH7095_EXT_MAP_GRAN_BITS])[A] = V;
#else
  ((uint8_t*)SH7095_FastMap[A >> SH7095_EXT_MAP_GRAN_BITS])[A ^ 1] = V;
#endif

  for(unsigned c = 0; c < 2; c++)
  {
   if(CPU[c].CCR & SH7095::CCR_CE)
   {
    for(uint32_t Abase = 0x00000000; Abase < 0x20000000; Abase += 0x08000000)
    {
     CPU[c].Cache_WriteUpdate<uint8_t>(Abase + A, V);
    }
   }
  }
 }
}
//
//
//
static void SetFastMemMap(uint32_t Astart, uint32_t Aend, uint16_t* ptr, uint32_t length, bool is_writeable)
{
 const uint64_t Abound = (uint64_t)Aend + 1;
 assert((Astart & ((1U << SH7095_EXT_MAP_GRAN_BITS) - 1)) == 0);
 assert((Abound & ((1U << SH7095_EXT_MAP_GRAN_BITS) - 1)) == 0);
 assert((length & ((1U << SH7095_EXT_MAP_GRAN_BITS) - 1)) == 0);
 assert(length > 0);
 assert(length <= (Abound - Astart));

 for(uint64_t A = Astart; A < Abound; A += (1U << SH7095_EXT_MAP_GRAN_BITS))
 {
  uintptr_t tmp = (uintptr_t)ptr + ((A - Astart) % length);

  if(A < (1U << 27))
   FMIsWriteable_set(A >> SH7095_EXT_MAP_GRAN_BITS, is_writeable);

  SH7095_FastMap[A >> SH7095_EXT_MAP_GRAN_BITS] = tmp - A;
 }
}

static MDFN_COLD void InitFastMemMap(void)
{
 for(unsigned i = 0; i < sizeof(fmap_dummy) / sizeof(fmap_dummy[0]); i++)
 {
  fmap_dummy[i] = 0;
 }

 FMIsWriteable_reset();
 MDFNMP_Init(1ULL << SH7095_EXT_MAP_GRAN_BITS, (1ULL << 27) / (1ULL << SH7095_EXT_MAP_GRAN_BITS));

 for(uint64_t A = 0; A < 1ULL << 32; A += (1U << SH7095_EXT_MAP_GRAN_BITS))
 {
  SH7095_FastMap[A >> SH7095_EXT_MAP_GRAN_BITS] = (uintptr_t)fmap_dummy - A;
 }
}

void SS_SetPhysMemMap(uint32_t Astart, uint32_t Aend, uint16_t* ptr, uint32_t length, bool is_writeable)
{
 assert(Astart < 0x20000000);
 assert(Aend < 0x20000000);

 if(!ptr)
 {
  ptr = fmap_dummy;
  length = sizeof(fmap_dummy);
 }

 for(uint32_t Abase = 0; Abase < 0x40000000; Abase += 0x20000000)
  SetFastMemMap(Astart + Abase, Aend + Abase, ptr, length, is_writeable);
}

#include "sh7095.inc"

//
// Running is:
//   0 at end of (emulation) frame
//
//   1 during normal execution
//
//  -1 when we need to temporarily break out of the execution loop to
//     e.g. handle turning the slave CPU on or off, which we can't
//     safely do from an event handler due to the event handler
//     potentially being called from deep within the memory read/write
//     functions.
//
int Running;
alignas(16) event_list_entry events[SS_EVENT__SIMD_COUNT];
static ss_event_handler event_handlers[SS_EVENT__COUNT];

static sscpu_timestamp_t next_event_ts;

// NO_INLINE keeps the body out of RunLoop's no-unroll pragma scope so -O2
// auto-vectorizes the reduction (smin/sminv on aarch64).
static NO_INLINE sscpu_timestamp_t FindNextEventTS(void)
{
 sscpu_timestamp_t m = SS_EVENT_DISABLED_TS;
 for(unsigned i = 0; i < SS_EVENT__SIMD_COUNT; i++)
  m = ((m) < (events[i].event_time) ? (m) : (events[i].event_time));
 return m;
}

template<unsigned c>
static sscpu_timestamp_t SH_DMA_EventHandler(sscpu_timestamp_t et)
{
 if(et < SH7095_mem_timestamp)
  return SH7095_mem_timestamp;

 // Must come after the (et < SH7095_mem_timestamp) check.
 if(MDFN_UNLIKELY(SH7095_BusLock))
  return et + 1;

 return CPU[c].DMA_Update(et);
}

//
//
//

static MDFN_COLD void InitEvents(void)
{
 // SYNFIRST/SYNLAST and padding slots stay disabled so the min-reduction
 // ignores them; only [SYNFIRST+1, SYNLAST) hold real events.
 for(unsigned i = 0; i < SS_EVENT__SIMD_COUNT; i++)
 {
  if(i == SS_EVENT__SYNFIRST || i == SS_EVENT__SYNLAST || i >= SS_EVENT__COUNT)
   events[i].event_time = SS_EVENT_DISABLED_TS;
  else
   events[i].event_time = 0;
 }

 for(unsigned i = 0; i < SS_EVENT__COUNT; i++)
  event_handlers[i] = NULL;

 event_handlers[SS_EVENT_SH2_M_DMA] = &SH_DMA_EventHandler<0>;
 event_handlers[SS_EVENT_SH2_S_DMA] = &SH_DMA_EventHandler<1>;

 event_handlers[SS_EVENT_SCU_DMA] = SCU_UpdateDMA;
 event_handlers[SS_EVENT_SCU_DSP] = SCU_UpdateDSP;
/*event_handlers[SS_EVENT_SCU_INT] = SCU_UpdateInt;*/

 event_handlers[SS_EVENT_SMPC] = SMPC_Update;

 event_handlers[SS_EVENT_VDP1] = VDP1::Update;
 event_handlers[SS_EVENT_VDP2] = VDP2::Update;

 event_handlers[SS_EVENT_CDB] = CDB_Update;

 event_handlers[SS_EVENT_SOUND] = SOUND_Update;

 event_handlers[SS_EVENT_CART] = CART_GetEventHandler();

 event_handlers[SS_EVENT_MIDSYNC] = MidSync;
 //
 //
 SS_SetEventNT(&events[SS_EVENT_MIDSYNC], SS_EVENT_DISABLED_TS);
}

static void RebaseTS(const sscpu_timestamp_t timestamp)
{
 for(unsigned i = SS_EVENT__SYNFIRST + 1; i < SS_EVENT__SYNLAST; i++)
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

// Called from debug.cpp too.
static void ForceEventUpdates(const sscpu_timestamp_t timestamp)
{
 for(unsigned c = 0; c < 2; c++)
  CPU[c].ForceInternalEventUpdates();

 for(unsigned evnum = SS_EVENT__SYNFIRST + 1; evnum < SS_EVENT__SYNLAST; evnum++)
 {
  if(events[evnum].event_time != SS_EVENT_DISABLED_TS)
   events[evnum].event_time = event_handlers[evnum](timestamp);
 }

 next_event_ts = (Running > 0) ? FindNextEventTS() : 0;
}

static INLINE bool EventHandler(const sscpu_timestamp_t timestamp)
{
 // next_event_ts is forced to 0 (sentinel) when Running <= 0 to make
 // CheckEventsByMemTS trip and unwind RunLoop. Don't enter the dispatch loop
 // in that state best_t = 0 wouldn't match any events[i].event_time and
 // the inner scan would walk off the end.
 if(MDFN_UNLIKELY(Running <= 0))
  return false;
 sscpu_timestamp_t best_t = next_event_ts;
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

static void CheckEventsByMemTS(void)
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

#if defined(__GNUC__) && !defined(__clang__)
 #pragma GCC push_options
 #pragma GCC optimize("O2,no-unroll-loops,no-peel-loops,no-crossjumping")
#endif
template<bool EmulateICache>
static NO_INLINE MDFN_HOT int32_t RunLoop(EmulateSpecStruct* espec)
{
 sscpu_timestamp_t eff_ts = 0;

 do
 {
  SMPC_ProcessSlaveOffOn();
  //
  //
  Running = true;
  ForceEventUpdates(eff_ts);
  do
  {
   do
   {
    CPU[0].Step<0, EmulateICache>();
    CPU[0].DMA_BusTimingKludge();

    if(EmulateICache)
    {
      CPU[1].RunSlaveUntil(CPU[0].timestamp);
    }
    else
    {
     while(MDFN_LIKELY(CPU[0].timestamp > CPU[1].timestamp))
     {
      CPU[1].Step<1, false>();
     }
    }

    eff_ts = CPU[0].timestamp;
    if(SH7095_mem_timestamp > eff_ts)
     eff_ts = SH7095_mem_timestamp;
    else
     SH7095_mem_timestamp = eff_ts;
   } while(MDFN_LIKELY(eff_ts < next_event_ts));
  } while(MDFN_LIKELY(EventHandler(eff_ts)));
 } while(MDFN_LIKELY(Running != 0));

 return eff_ts;
}

#if defined(__GNUC__) && !defined(__clang__)
 #pragma GCC pop_options
#endif

// Must not be called within an event or read/write handler.
void SS_Reset(bool powering_up)
{
 SH7095_BusLock = 0;

 if(powering_up)
 {
   memset(WorkRAM, 0x00, sizeof(WorkRAM));   // TODO: Check real hardware
 }

 if(powering_up)
 {
  CPU[0].TruePowerOn();
  CPU[1].TruePowerOn();
 }

 SCU_Reset(powering_up);
 CPU[0].Reset(powering_up);

 // ST-V's I/O board must reset before SMPC -- SMPC's port shim
 // (IODevice_STVSMPC) consults state that STVIO_Reset re-initialises.
 if(ActiveCartType == CART_STV)
   STVIO_Reset(powering_up);

 SMPC_Reset(powering_up);

 VDP1::Reset(powering_up);
 VDP2::Reset(powering_up);

 CDB_Reset(powering_up);

 SOUND_Reset(powering_up);

 CART_Reset(powering_up);
}

static EmulateSpecStruct* espec;
static bool AllowMidSync;
static int32_t cur_clock_div;

static int64_t UpdateInputLastBigTS;
static INLINE void UpdateSMPCInput(const sscpu_timestamp_t timestamp)
{
 SMPC_TransformInput();

 int32_t elapsed_time = (((int64_t)timestamp * cur_clock_div * 1000 * 1000) - UpdateInputLastBigTS) / (EmulatedSS.MasterClock / MDFN_MASTERCLOCK_FIXED(1));

 UpdateInputLastBigTS += (int64_t)elapsed_time * (EmulatedSS.MasterClock / MDFN_MASTERCLOCK_FIXED(1));

 // ST-V samples gamepad/gun/coin state into its own DataIn buffer on
 // the same cadence SMPC samples virtual ports.
 if(ActiveCartType == CART_STV)
   STVIO_UpdateInput(elapsed_time);

 SMPC_UpdateInput(elapsed_time);
}

static sscpu_timestamp_t MidSync(const sscpu_timestamp_t timestamp)
{
 if(AllowMidSync)
 {
    SMPC_UpdateOutput();

    MDFN_MidSync();

    UpdateSMPCInput(timestamp);

    AllowMidSync = false;
 }

 return SS_EVENT_DISABLED_TS;
}

void Emulate(EmulateSpecStruct* espec_arg)
{
 int32_t end_ts;

 espec = espec_arg;
 AllowMidSync = setting_midsync;

 cur_clock_div = SMPC_StartFrame();
 UpdateSMPCInput(0);
 VDP2::StartFrame(espec, cur_clock_div == 61);
 CART_SetCPUClock(EmulatedSS.MasterClock / MDFN_MASTERCLOCK_FIXED(1), cur_clock_div);
 espec->SoundBufSize = 0;
 espec->MasterCycles = 0;
 //
 //
 //
 if (NeedEmuICache)
  end_ts = RunLoop<true>(espec);
 else
  end_ts = RunLoop<false>(espec);
 assert(end_ts >= 0);

 ForceEventUpdates(end_ts);
 //
 SMPC_EndFrame(espec, end_ts);
 //
 //
 //
 RebaseTS(end_ts);

 CDB_ResetTS();
 SOUND_AdjustTS(-end_ts);
 VDP1::AdjustTS(-end_ts);
 VDP2::AdjustTS(-end_ts);
 SMPC_ResetTS();
 SCU_AdjustTS(-end_ts);
 CART_AdjustTS(-end_ts);

 UpdateInputLastBigTS -= (int64_t)end_ts * cur_clock_div * 1000 * 1000;
 //
 SH7095_mem_timestamp -= end_ts; // Update before CPU[n].AdjustTS()
 //
 for(unsigned c = 0; c < 2; c++)
  CPU[c].AdjustTS(-end_ts);

 //
 //
 //
 espec->MasterCycles  = end_ts * cur_clock_div;
 espec->SoundBufSize += IBufferCount;
 IBufferCount         = 0;

 SMPC_UpdateOutput();
 //
 // Backup-RAM and cart-NV dirty tracking.
 //
 // Previously this block performed synchronous file I/O from inside
 // Emulate() (SaveBackupRAM/SaveCartNV under a master-cycle countdown).
 // That had two big problems for a libretro core:
 //   1. Run-ahead / rewind / netplay re-emulate frames repeatedly. The
 //      cycle-counted delay fires identically on each pass, so a single
 //      real frame could produce two or three full SaveBackupRAM disk
 //      writes -- visible as stutter and unstable frame pacing.
 //   2. The save is fully synchronous (FileStream::write+close) on the
 //      emulation thread, so the duration is unpredictable under load.
 //
 // The fix is to keep BackupRAM_Dirty (and the cart-NV dirty bit) as
 // pure flags here, and let libretro.cpp flush them from retro_run --
 // outside Emulate, with awareness of RETRO_ENVIRONMENT_GET_AUDIO_VIDEO_ENABLE
 // so run-ahead simulation frames don't trigger writes. The frontend
 // can also manage Backup RAM directly via RETRO_MEMORY_SAVE_RAM.
 //
 if(CART_GetClearNVDirty())
  CartNV_Dirty = true;
}

//
//
//

static MDFN_COLD void Cleanup(void)
{
 CART_Kill();

 VDP1::Kill();
 VDP2::Kill();
 SOUND_Kill();
 CDB_Kill();

 disc_cleanup();
}

typedef struct
{
   const unsigned type;
   const char *name;
} CartName;

bool MDFN_COLD InitCommon(const unsigned cpucache_emumode, const unsigned horrible_hacks, const unsigned cart_type, const unsigned smpc_area, const char* rom_dir, const char* main_fname, const STVGameInfo* sgi)
{
 //

   unsigned i;
 //
 {
  const struct
  {
   unsigned mode;
   const char* name;
  } CPUCacheEmuModes[] =
  {
   { CPUCACHE_EMUMODE_DATA_CB,   _("Data only, with high-level bypass") },
   { CPUCACHE_EMUMODE_DATA,   _("Data only") },
   { CPUCACHE_EMUMODE_FULL,   _("Full") },
  };
  const char* cem = _("Unknown");

  for(auto const& ceme : CPUCacheEmuModes)
  {
   if(ceme.mode == cpucache_emumode)
   {
    cem = ceme.name;
    break;
   }
  }
  log_cb(RETRO_LOG_INFO, "CPU Cache Emulation Mode: %s\n", cem);
 }
 //
 if(horrible_hacks)
  log_cb(RETRO_LOG_INFO, "Horrible hacks: 0x%08x\n", horrible_hacks);
 //
   {
      log_cb(RETRO_LOG_INFO, "Region: 0x%01x.\n", smpc_area);
      const CartName CartNames[] =
      {
         { CART_NONE, "None" },
         { CART_BACKUP_MEM, "Backup Memory" },
         { CART_EXTRAM_1M, "1MiB Extended RAM" },
         { CART_EXTRAM_4M, "4MiB Extended RAM" },
         { CART_KOF95, "King of Fighters '95 ROM" },
         { CART_ULTRAMAN, "Ultraman ROM" },
         { CART_CS1RAM_16M, _("16MiB CS1 RAM") },
         { CART_NLMODEM, _("Netlink Modem") },
         { CART_STV, _("Sega Titan Video (ST-V)") },
         { CART_BOOTROM, _("Bootable ROM") } 
      };
      const char* cn = nullptr;

      for(i = 0; i < ARRAY_SIZE(CartNames); i++)
      {
         CartName cne = CartNames[i];
         if(cne.type != cart_type)
            continue;
         cn = cne.name;
         break;
      }
      if ( cn ) {
         log_cb(RETRO_LOG_INFO, "Cart: %s.\n", cn);
     } else {
         log_cb(RETRO_LOG_INFO, "Cart: Unknown (%d).\n", cart_type );
     }
   }
   //

   NeedEmuICache = (cpucache_emumode == CPUCACHE_EMUMODE_FULL);
   for(i = 0; i < 2; i++)
   {
      CPU[i].Init((cpucache_emumode == CPUCACHE_EMUMODE_FULL), (cpucache_emumode == CPUCACHE_EMUMODE_DATA_CB));
      CPU[i].SetMD5((bool)i);
   }
 SH7095_mem_timestamp = 0;
 SH7095_DB = 0;

 ss_horrible_hacks = horrible_hacks;

   //
   // Initialize backup memory.
   //
   memset(BackupRAM, 0x00, sizeof(BackupRAM));
   for(i = 0; i < 0x40; i++)
      BackupRAM[i] = BRAM_Init_Data[i & 0x0F];

   // Call InitFastMemMap() before functions like SOUND_Init()
   InitFastMemMap();
   SS_SetPhysMemMap(0x00000000, 0x000FFFFF, BIOSROM, sizeof(BIOSROM));
   SS_SetPhysMemMap(0x00200000, 0x003FFFFF, WorkRAML, WORKRAM_BANK_SIZE_BYTES, true);
   SS_SetPhysMemMap(0x06000000, 0x07FFFFFF, WorkRAMH, WORKRAM_BANK_SIZE_BYTES, true);
   MDFNMP_RegSearchable(0x00200000, WORKRAM_BANK_SIZE_BYTES);
   MDFNMP_RegSearchable(0x06000000, WORKRAM_BANK_SIZE_BYTES);

   if(!CART_Init(cart_type, rom_dir, main_fname, sgi))
      return false;
   ActiveCartType = cart_type;

   //
   //
   //
   const bool is_stv = (cart_type == CART_STV);
   // ST-V runs on a monitor, not on a TV; force 60 Hz timing
   // regardless of region. PAL ST-V cabinets exist but the video
   // signal is still 60 Hz.
   const bool PAL = is_pal = (!is_stv) && (smpc_area & SMPC_AREA__PAL_MASK);
   const int32_t MasterClock = PAL ? 1734687500 : 1746818182; // NTSC: 1746818181.8181818181, PAL: 1734687500-ish
   const char* bios_filename;
   int sls = MDFN_GetSettingI(PAL ? "ss.slstartp" : "ss.slstart");
   int sle = MDFN_GetSettingI(PAL ? "ss.slendp" : "ss.slend");
 const uint64_t vdp2_affinity = 0; /*LibRetro: unused*/

   if(sls > sle)
   {
      /* std::swap(sls, sle) folded; braced because this is an
       * unbraced if body. */
      int tmp_sl = sls;
      sls = sle;
      sle = tmp_sl;
   }

   if(is_stv)
   {
      // ST-V BIOSes are 128 KiB (vs Saturn's 512 KiB) and live in their
      // own filename namespace. Hardcoded to match the fork's existing
      // BIOS-filename convention for sega_101.bin / mpr-17933.bin.
      // Users supply the actual files in retro_base_directory.
      //   JP / Asia-NTSC:                       epr-20091.ic8
      //   NA / CSA-NTSC / CSA-PAL:              epr-17952a.ic8
      //   EU / Asia-PAL / Korea / everything else: epr-17954a.ic8
      if(smpc_area == SMPC_AREA_JP || smpc_area == SMPC_AREA_ASIA_NTSC)
         bios_filename = "epr-20091.ic8";
      else if(smpc_area == SMPC_AREA_NA || smpc_area == SMPC_AREA_CSA_NTSC || smpc_area == SMPC_AREA_CSA_PAL)
         bios_filename = "epr-17952a.ic8";
      else
         bios_filename = "epr-17954a.ic8";
   }
   else if(smpc_area == SMPC_AREA_JP || smpc_area == SMPC_AREA_ASIA_NTSC)
      bios_filename = "sega_101.bin"; // Japan
   else
      bios_filename = "mpr-17933.bin"; // North America and Europe

   {
      char bios_path[4096];
      snprintf(bios_path, sizeof(bios_path), "%s" RETRO_SLASH "%s", retro_base_directory, bios_filename);

      RFILE *BIOSFile = filestream_open(bios_path,
            RETRO_VFS_FILE_ACCESS_READ,
            RETRO_VFS_FILE_ACCESS_HINT_NONE);

      // Saturn BIOSes are 512 KiB; ST-V BIOSes are 128 KiB
      // (mapped into the upper half of the 512 KiB BIOSROM[] array
      // and read by the SH-2 from the same 0x00000000-0x000FFFFF
      // window). Accept either size.
      const int64_t bios_size = filestream_get_size(BIOSFile);
      if(!BIOSFile)
      {
         log_cb(RETRO_LOG_ERROR, "Cannot open BIOS file \"%s\".\n", bios_path);
         return false;
      }
      else if(bios_size != 524288 && !(is_stv && bios_size == 131072))
      {
         log_cb(RETRO_LOG_ERROR, "BIOS file \"%s\" is of an incorrect size.\n", bios_path);
         filestream_close(BIOSFile);
         return false;
      }
      else
      {
         memset(BIOSROM, 0xFF, sizeof(BIOSROM));
         filestream_read(BIOSFile, BIOSROM, bios_size);
         filestream_close(BIOSFile);
         BIOS_SHA256 = sha256(BIOSROM, 512 * 1024);

         // swap endian-ness
         for(unsigned i = 0; i < (bios_size / 2); i++)
         {
            /* MDFN_de16msb folded: BE-on-disk to host-endian. */
#ifndef MSB_FIRST
            BIOSROM[i] = (uint16_t)((BIOSROM[i] << 8) | (BIOSROM[i] >> 8));
#endif
         }
      }
   }

   EmulatedSS.MasterClock = MDFN_MASTERCLOCK_FIXED(MasterClock);

   SCU_Init();
   SMPC_Init(smpc_area, MasterClock, is_stv);
   VDP1::Init();
   VDP2::Init(PAL,vdp2_affinity);
   VDP2::SetGetVideoParams(&EmulatedSS, true, sls, sle, true, DoHBlend);
   CDB_Init();
   SOUND_Init();

   InitEvents();
   UpdateInputLastBigTS = 0;

   // Apply multi-tap state to SMPC
   SMPC_SetMultitap( 0, setting_multitap_port1 );
   SMPC_SetMultitap( 1, setting_multitap_port2 );

   if(is_stv)
   {
      // ST-V provides its own SMPC port shim (handles AK93C45 EEPROM
      // and 68K sound-CPU control), and routes player input through
      // STVIO_SetInput / STVIO_UpdateInput rather than the standard
      // per-virtual-port path. Init the I/O board first, then inject
      // the port shims into SMPC super-ports 0 and 1.
      STVIO_Init(sgi);
      for(unsigned sp = 0; sp < 2; sp++)
         SMPC_SetInput(sp, "extern", (uint8_t*)STVIO_GetSMPCDevice(sp));
   }

   LoadRTC();
   LoadBackupRAM();
   LoadCartNV();

   BackupBackupRAM();
   BackupCartNV();

   // Just-loaded state is by definition clean. The cycle-counted
   // SaveDelay variables are gone -- see comment in Emulate().
   BackupRAM_Dirty = false;
   CART_GetClearNVDirty();
   CartNV_Dirty = false;
   //
   if(MDFN_GetSettingB("ss.smpc.autortc"))
   {
      time_t ut;
      struct tm* ht;

      if((ut = time(NULL)) == (time_t)-1)
      {
         log_cb(RETRO_LOG_ERROR,
               "AutoRTC error #1\n");
         // Previously this just returned false, leaving the VDP2
         // render thread, semaphore, queue, SCU, SMPC, etc. fully
         // initialised. A subsequent load would then double-init
         // and race with the orphaned render thread.
         Cleanup();
         return false;
      }

      if((ht = localtime(&ut)) == NULL)
      {
         log_cb(RETRO_LOG_ERROR,
               "AutoRTC error #2\n");
         Cleanup();
         return false;
      }

      SMPC_SetRTC(ht, MDFN_GetSettingUI("ss.smpc.autortc.lang"));
   }
   //
   SS_Reset(true);

   return true;
}

MDFN_COLD void CloseGame(void)
{
 //
 //
#ifdef SH7095_OP_PAIR_PROFILE
 SS_DumpOpPairProfile();
#endif

 SaveBackupRAM();
 SaveCartNV();
 SaveRTC();

 Cleanup();
}

void MDFN_BackupSavFile(const uint8_t max_backup_count, const char* sav_ext)
{
   // stub for libretro port
}

static MDFN_COLD void SaveBackupRAM(void)
{
 char fpath[4096];
 cdstream brs;
 if(!cdstream_open_write(&brs, MDFN_MakeFName(fpath, sizeof(fpath), MDFNMKF_SAV, 0, "bkr")))
  return;

 cdstream_write(&brs, BackupRAM, sizeof(BackupRAM));

 cdstream_close(&brs);
}

static MDFN_COLD void LoadBackupRAM(void)
{
 char fpath[4096];
 RFILE *brs = filestream_open(MDFN_MakeFName(fpath, sizeof(fpath), MDFNMKF_SAV, 0, "bkr"),
       RETRO_VFS_FILE_ACCESS_READ,
       RETRO_VFS_FILE_ACCESS_HINT_NONE);

 if (!brs)
    return;

 filestream_read(brs, BackupRAM, sizeof(BackupRAM));
 filestream_close(brs);
}

static MDFN_COLD void BackupBackupRAM(void)
{
 MDFN_BackupSavFile(10, "bkr");
}

static MDFN_COLD void BackupCartNV(void)
{
 const char* ext = nullptr;
 void* nv_ptr = nullptr;
 bool nv16 = false;
 uint64_t nv_size = 0;

 CART_GetNVInfo(&ext, &nv_ptr, &nv16, &nv_size);

 if(ext)
  MDFN_BackupSavFile(10, ext);
}

static MDFN_COLD void LoadCartNV(void)
{
   uint64_t i;
   RFILE *nvs      = NULL;
   const char* ext = nullptr;
   void* nv_ptr    = nullptr;
   bool nv16       = false;
   uint64_t nv_size  = 0;
   char fpath[4096];

   CART_GetNVInfo(&ext, &nv_ptr, &nv16, &nv_size);

   if (!ext)
      return;

   nvs = filestream_open(
         MDFN_MakeFName(fpath, sizeof(fpath), MDFNMKF_CART, 0, ext),
         RETRO_VFS_FILE_ACCESS_READ,
         RETRO_VFS_FILE_ACCESS_HINT_NONE);

   if (!nvs)
      return;

   filestream_read(nvs, nv_ptr, nv_size);
   filestream_close(nvs);

   if (!nv16)
      return;

   /* nv16-flagged carts store NVRAM as big-endian uint16s.  On
    * MSB_FIRST host the on-disk layout already matches host
    * order, nothing to do.  On LE host we byteswap the buffer
    * in place. */
#ifndef MSB_FIRST
   for(i = 0; i < nv_size; i += 2)
   {
      uint8_t* p = (uint8_t*)nv_ptr + i;
      uint16_t v;
      memcpy(&v, p, 2);
      v = (uint16_t)((v << 8) | (v >> 8));
      memcpy(p, &v, 2);
   }
#else
   (void)i;
#endif
}

static MDFN_COLD void SaveCartNV(void)
{
   const char* ext = nullptr;
   void* nv_ptr = nullptr;
   bool nv16 = false;
   uint64_t nv_size = 0;
   char fpath[4096];

   CART_GetNVInfo(&ext, &nv_ptr, &nv16, &nv_size);

   if(ext)
   {
      cdstream nvs;
      if(!cdstream_open_write(&nvs, MDFN_MakeFName(fpath, sizeof(fpath), MDFNMKF_CART, 0, ext)))
         return;

      if(nv16)
      {
         uint64_t i;
         /* nv_ptr is host-endian uint16s; cdstream_write_be_u16
          * takes host-endian and emits big-endian bytes.  Just a
          * misalignment-safe 2-byte load. */
         for(i = 0; i < nv_size; i += 2)
         {
            uint16_t v;
            memcpy(&v, (uint8_t*)nv_ptr + i, 2);
            cdstream_write_be_u16(&nvs, v);
         }
      }
      else
         cdstream_write(&nvs, nv_ptr, nv_size);

      cdstream_close(&nvs);
   }
}

//
// Public flush entry points for libretro.cpp.
//
// These wrap the (still static) SaveBackupRAM / SaveCartNV functions so
// the file I/O can happen from outside Emulate(), after retro_run() has
// emulated a frame and before it returns. Calling these from outside
// Emulate() is what makes run-ahead / rewind / netplay friendly: those
// features re-run Emulate() multiple times per real frame, and the
// previous design issued a disk write on every re-run when the BRAM/cart
// dirty timer expired.
//
// Both return true on success and false on failure (so the caller can
// schedule a retry); they swallow exceptions because they're called from
// the libretro retro_run boundary where letting an exception escape
// would unwind through the frontend.
//
bool SS_FlushBackupRAM(void)
{
 SaveBackupRAM();
 return true;
}

bool SS_FlushCartNV(void)
{
 SaveCartNV();
 return true;
}

static MDFN_COLD void SaveRTC(void)
{
   char fpath[4096];
   cdstream sds;
   if(!cdstream_open_write(&sds, MDFN_MakeFName(fpath, sizeof(fpath), MDFNMKF_SAV, 0, "smpc")))
      return;

   SMPC_SaveNV(&sds);

   cdstream_close(&sds);
}

static MDFN_COLD void LoadRTC(void)
{
   char fpath[4096];
   cdstream sds;
   if(!cdstream_open(&sds, MDFN_MakeFName(fpath, sizeof(fpath), MDFNMKF_SAV, 0, "smpc")))
      return;

   SMPC_LoadNV(&sds);

   cdstream_close(&sds);
}

struct EventsPacker
{
 enum : size_t { eventcopy_first = SS_EVENT__SYNFIRST + 1 };
 enum : size_t { eventcopy_bound = SS_EVENT__SYNLAST };

 bool Restore(const unsigned state_version);
 void Save(void);

 int32_t event_times[eventcopy_bound - eventcopy_first];
 uint8_t event_order[eventcopy_bound - eventcopy_first];
};

INLINE void EventsPacker::Save(void)
{
 for(size_t i = 0; i < eventcopy_bound - eventcopy_first; i++)
 {
  event_times[i] = events[eventcopy_first + i].event_time;
  event_order[i] = (uint8_t)(eventcopy_first + i);
 }

 // event_order is the schedule order Restore() validates; equal times tie-break by index.
 // Was std::stable_sort with a lambda comparator. The array is tiny
 // (SS_EVENT__SYNLAST - SS_EVENT__SYNFIRST - 1 entries) and insertion
 // sort is stable, so equal event_times keep their original index
 // order -- the same tie-break std::stable_sort gave.
 {
  size_t i, j;
  const size_t n = eventcopy_bound - eventcopy_first;
  for(i = 1; i < n; i++)
  {
   const uint8_t key = event_order[i];
   j = i;
   while(j > 0 && events[event_order[j - 1]].event_time > events[key].event_time)
   {
    event_order[j] = event_order[j - 1];
    j--;
   }
   event_order[j] = key;
  }
 }
}

INLINE bool EventsPacker::Restore(const unsigned state_version)
{
 bool used[SS_EVENT__COUNT] = { 0 };

 for(size_t i = 0; i < eventcopy_bound - eventcopy_first; i++)
 {
  int32_t et = event_times[i];
  uint8_t eo = event_order[i];

  if(state_version < 0x00102600 && et >= 0x40000000)
  {
   et = SS_EVENT_DISABLED_TS;
  }

  /*
     if(state_version < 0x00102800 && i == SS_EVENT_SCU_INT)
     {
     eo = i;
     et = SS_EVENT_DISABLED_TS;
     }
   */

  if(eo < eventcopy_first || eo >= eventcopy_bound)
   return false;

  if(used[eo])
   return false;

  used[eo] = true;

  if(et < 0)
   return false;

  events[eventcopy_first + i].event_time = et;
 }

 // Reject malformed save states whose event_order isn't non-decreasing.
 for(size_t i = 1; i < eventcopy_bound - eventcopy_first; i++)
 {
  if(events[event_order[i]].event_time < events[event_order[i - 1]].event_time)
   return false;
 }

 return true;
}

extern "C" MDFN_COLD int LibRetro_StateAction( StateMem* sm, const unsigned load)
{
   {
      sha256_digest sr_dig = BIOS_SHA256;
      int cart_type = ActiveCartType;

      SFORMAT SRDStateRegs[] =
      {
         SFPTR8( sr_dig.data(), sr_dig.size() ),
         SFVAR(cart_type),
         SFEND
      };

      if (MDFNSS_StateAction( sm, load, false, SRDStateRegs, "BIOS_HASH", true ) == 0)
         return 0;

      if ( load )
      {
		 if ( sr_dig != BIOS_SHA256 ) {
           log_cb( RETRO_LOG_WARN, "BIOS hash mismatch(save state created under a different BIOS)!\n" );
           return 0;
         }
         if ( cart_type != ActiveCartType ) {
           log_cb( RETRO_LOG_WARN, "Cart type mismatch(save state created with a different cart)!\n" );
           return 0;
         }
      }
   }
 //
 //
 //
 bool RecordedNeedEmuICache = load ? false : NeedEmuICache;
 EventsPacker ep;
 ep.Save();

 SFORMAT StateRegs[] =
 {
  // cur_clock_div
  SFVAR(UpdateInputLastBigTS),

  SFVAR(next_event_ts),
  SFPTR32N(ep.event_times, sizeof(ep.event_times) / sizeof(ep.event_times[0]), "event_times"),
  SFPTR8N(ep.event_order, sizeof(ep.event_order) / sizeof(ep.event_order[0]), "event_order"),
/*
  SFPTR32N(ep.event_times, 11, "event_times"),
  SFPTR8N(ep.event_order, 11, "event_order"),
  SFVARN(ep.event_times[11], "event_times[11]"),
  SFVARN(ep.event_order[11], "event_order[11]"),
*/

  SFVAR(SH7095_mem_timestamp),
  SFVAR(SH7095_BusLock),
  SFVAR(SH7095_DB),

  SFPTR16(WorkRAML, WORKRAM_BANK_SIZE_BYTES / sizeof(uint16_t)),
  SFPTR16(WorkRAMH, WORKRAM_BANK_SIZE_BYTES / sizeof(uint16_t)),
  SFPTR8(BackupRAM, sizeof(BackupRAM) / sizeof(BackupRAM[0])),

  SFVAR(RecordedNeedEmuICache),

  SFEND
 };

 CPU[0].StateAction(sm, load, false, "SH2-M");
 CPU[1].StateAction(sm, load, false, "SH2-S");
 SCU_StateAction(sm, load, false);
 SMPC_StateAction(sm, load, false);

 CDB_StateAction(sm, load, false);
 VDP1::StateAction(sm, load, false);
 VDP2::StateAction(sm, load, false);

 SOUND_StateAction(sm, load, false);
 CART_StateAction(sm, load, false);


 if(load)
  memcpy(BackupRAM_StateHelper, BackupRAM, sizeof(BackupRAM));
 //
 //
   if (MDFNSS_StateAction(sm, load, false, StateRegs, "MAIN", false) == 0)
   {
      log_cb( RETRO_LOG_ERROR, "Failed to load MAIN state objects.\n" );
      return 0;
   }

   if (input_StateAction( sm, load, false ) == 0)
      log_cb( RETRO_LOG_WARN, "Input state failed.\n" );

   if ( load )
   {
      if(memcmp(BackupRAM_StateHelper, BackupRAM, sizeof(BackupRAM)))
         BackupRAM_Dirty = true;

      if ( !ep.Restore(load) )
      {
         log_cb( RETRO_LOG_WARN, "Bad state events data.\n" );
         InitEvents();
      }

         
      CPU[0].PostStateLoad(load, RecordedNeedEmuICache, NeedEmuICache);
      CPU[1].PostStateLoad(load, RecordedNeedEmuICache, NeedEmuICache);
   }

   // Success!
   return 1;
}

static const MDFNSetting_EnumList RTCLang_List[] =
{
 { "english", SMPC_RTC_LANG_ENGLISH, "English" },
 { "german", SMPC_RTC_LANG_GERMAN, "Deutsch" },
 { "french", SMPC_RTC_LANG_FRENCH, "Français" },
 { "spanish", SMPC_RTC_LANG_SPANISH, "Español" },
 { "italian", SMPC_RTC_LANG_ITALIAN, "Italiano" },
 { "japanese", SMPC_RTC_LANG_JAPANESE, "日本語" },

 { "deutsch", SMPC_RTC_LANG_GERMAN, NULL },
 { "français", SMPC_RTC_LANG_FRENCH, NULL },
 { "español", SMPC_RTC_LANG_SPANISH, NULL },
 { "italiano", SMPC_RTC_LANG_ITALIAN, NULL },
 { "日本語", SMPC_RTC_LANG_JAPANESE, NULL},

 { NULL, 0 },
};

MDFNGI EmulatedSS =
{
   0,	// MasterClock

   //
   // Note: Following video settings will be overwritten during game load.
   //
   320,  // lcm_width
   240,  // lcm_height
   NULL,  // Dummy

   320,   // Nominal width
   240,   // Nominal height

   0,   // Framebuffer width
   0,   // Framebuffer height
};
