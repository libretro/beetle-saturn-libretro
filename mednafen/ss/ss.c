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

#include "../mempatcher.h"
/* mednafen.h / git.h are intentionally NOT included -- they pull in
 * <algorithm>, <string>, <vector>, <map> (C++-only) into a TU whose
 * body is otherwise pure C.  The MDFNGI typedef this file needs for
 * `extern MDFNGI EmulatedSS;` lives in mdfn_gameinfo.h which is
 * C-clean (factored out of git.h specifically so C TUs can include
 * it).  EmulateSpecStruct lives in emuspec.h for the same reason --
 * typedef'd at file scope so both C and C++ TUs can name the type
 * without the `struct` keyword.
 *
 * The _() translation-string identity macro normally lives in
 * mednafen.h; redefine here. */
#ifndef _
#define _(String) (String)
#endif
#include "../mdfn_gameinfo.h"
#include "../emuspec.h"
#include "../general.h"
#include "../cdrom/cdromif.h"
#include "../cdstream.h"
#include "../hash/sha256.h"
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

/* Phase-7c: FastMemMap + SH-2 event system moved to ss_init.c.
 * The cross-TU plumbing (externs for SH7095_FastMap, FMIsWriteable,
 * events, event_handlers, next_event_ts, Running plus the
 * function prototypes ss.cpp's ForceEventUpdates / CheatMemWrite
 * still need from C) lives in this header. */
#include "ss_init.h"


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
#include "../general.h"		/* MDFN_MidSync (defined in libretro.c) */
extern bool is_pal;
extern char retro_base_directory[4096];

/* MidSync forward decl removed (phase 7c): the canonical extern decl
 * lives in ss_init.h now, and MidSync's definition below is non-static. */

uint32_t ss_horrible_hacks;

bool NeedEmuICache;
/* BRAM_Init_Data moved to ss_state.c (phase 7b). */

/* Phase-7b: the eight file-I/O functions and BRAM_Init_Data
 * moved to ss_state.c.  Pulled in here for the call sites in
 * Cleanup() / Emulate() / InitCommon() / SS_FlushBackupRAM() /
 * SS_FlushCartNV(). */
#include "ss_state.h"

#include "sh7095.h"

static uint8_t SCU_MSH2VectorFetch(void);
static uint8_t SCU_SSH2VectorFetch(void);

/* CheckEventsByMemTS forward decl removed (phase 7c): canonical
 * decl in ss_init.h; definition lives in ss_init.c. */

SH7095 CPU[2];

/* Phase-9 step 5: SH7095 ctor dropped; CPU[2] is now zero-initialized
 * (static storage duration) and the once-only per-CPU init that the
 * ctor used to do moves into SH7095_ConstructAll below.  Called from
 * InitCommon() before either CPU is touched. */
MDFN_COLD void SH7095_ConstructAll(void)
{
 SH7095_Construct(&CPU[0], "SH2-M", SS_EVENT_SH2_M_DMA, SCU_MSH2VectorFetch);
 SH7095_Construct(&CPU[1], "SH2-S", SS_EVENT_SH2_S_DMA, SCU_SSH2VectorFetch);
}

/* C-linkage proxies bridging the SH7095 class to C consumers.  Used
 * by smpc.c (the converted SMPC TU) to drive slave-CPU enable/disable
 * (SMPC SH2_RESET / SH2_GET / SH2_SET commands) and to assert NMI
 * (SMPC SYSRES / SNDRES / CDON / CDOFF paths).  The cpu index picks
 * between SH-2 master (0) and slave (1); callers in smpc.c pass
 * literal 0/1 to match the historical CPU[0]/CPU[1] indexing.
 *
 * The proxies stay here in ss.cpp rather than in sh7095.cpp because
 * the CPU[2] global lives in this TU.  sh7095.h itself remains
 * C++-only (it exposes the class, and there is no current C TU that
 * needs anything beyond these two methods); when more SH7095
 * operations need C-callable proxies they should be added here. */
/* Phase-9 follow-up: these C-callable proxies used to shadow the
 * SH7095*-primary `SH7095_SetActive` / `SH7095_SetNMI` decls in
 * sh7095.h via C++ overloading (same name, different signature,
 * different linkage namespace).  Once sh7095.h became C-parseable
 * the overload collapsed to a redefinition: C has no overloading.
 *
 * Both wrappers were always called with hard-coded CPU indices
 * (SetActive only ever with 1 = slave, SetNMI only ever with
 * 0 = master), so the right shape is `SH7095_M_*` / `SH7095_S_*`
 * matching the existing SH7095_M_Init / SH7095_M_Reset naming
 * convention -- drop the int parameter, encode the CPU in the
 * function name. */
void SH7095_S_SetActive(bool active)
{
 SH7095_SetActive(&CPU[1], active);
}

void SH7095_M_SetNMI(bool level)
{
 SH7095_SetNMI(&CPU[0], level);
}

/* Used by vdp2.c (converted from C++) for the HORRIBLEHACK_NOSH2DMA-
 * LINE106 path -- vdp2's CPU loop iterates CPU[0..1] once per scanline
 * advance and sets the kludge flag.  Matches the SetActive / SetNMI
 * proxies above; cpu index picks master (0) / slave (1). */
void SH7095_SetExtHaltDMAKludge(int cpu, bool state)
{
 SH7095_SetExtHaltDMAKludgeFromVDP2(&CPU[cpu], state);
}

/* Phase-7f: promoted from file-static -- ss_init.c's InitCommon
 * loads it from disk and assigns BIOS_SHA256.  Definition stays
 * here so the rest of ss.cpp's globals layout is undisturbed. */
uint16_t BIOSROM[524288 / sizeof(uint16_t)];
uint8_t WorkRAM[2*WORKRAM_BANK_SIZE_BYTES]; // unified 2MB work ram for linear access.
// Effectively 32-bit in reality, but 16-bit here because of CPU interpreter design(regarding fastmap).
uint16_t* WorkRAML = (uint16_t*)(WorkRAM + (WORKRAM_BANK_SIZE_BYTES*0));
uint16_t* WorkRAMH = (uint16_t*)(WorkRAM + (WORKRAM_BANK_SIZE_BYTES*1));
// BackupRAM is exposed (no longer file-static) so libretro.cpp can hand a
// pointer to the frontend via retro_get_memory_data(RETRO_MEMORY_SAVE_RAM).
// BackupRAM_Dirty and CartNV_Dirty are sticky flags maintained here and
// drained by libretro.cpp from outside Emulate() -- see comment in
// Emulate() above. The old master-cycle delay variables are gone.
uint8_t BackupRAM[32768];
uint8_t BackupRAM_StateHelper[32768];
bool BackupRAM_Dirty;
bool CartNV_Dirty;

/* Phase-7e: ss.cpp's InitCommon zero-initialises this on game load
 * (line ~867); ss_init.c's MidSync helper UpdateSMPCInput / Emulate
 * loop read and update it.  Define lives here, extern declared in
 * ss_init.c for the C-side accessors. */
int64_t UpdateInputLastBigTS;

/* SH7095_FastMap definition moved to ss_init.c (phase 7c).
 * SH7095_EXT_MAP_GRAN_BITS lives in ss_init.h now. */

int32_t SH7095_mem_timestamp;
/* SH7095_BusLock is read from ss_init.c's SH_DMA_EventHandler -- promoted
 * from file-static to TU-external in phase 7c. */
uint32_t SH7095_BusLock;
uint32_t SH7095_DB;

#include "scu.inc"

sha256_digest BIOS_SHA256;   // SHA-256 hash of the currently-loaded BIOS; used for save state sanity checks.
int ActiveCartType;		// Used in save states.
/* FMIsWriteable + accessors + fmap_dummy moved to ss_init.c (phase 7c).
 * FMIsWriteable_get/set/reset are inline functions in ss_init.h
 * so ss.cpp callers (CheatMemWrite still here) keep their codegen. */

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
/* Phase-8r2: BusRW_DB_CS0 retired into 4 named variants
 * via source-fold.  Only (u8/u16) x (W0/W1) tuples are
 * invoked by callers in sh7095.inc; no u32 CS0 access. */

static INLINE void BusRW_DB_CS0_u8_W1(const uint32_t A, uint32_t* DB, const bool BurstHax, int32_t* SH2DMAHax)
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

   return;
  }

  {
   /* ne16_wbo_be<T>(WorkRAML, byte_off, val) folded.  T can be
    * uint8_t, uint16_t, or uint32_t; sizeof(T) dispatches the right
    * write width.  Compiler folds away the dead branches per
    * template instantiation. */
   const uint32_t boff_ = A & 0xFFFFF;
   const uint8_t val_ = *DB >> (((A & 1) ^ (2 - 1)) << 3);
   {
#ifdef MSB_FIRST
    ((uint8_t*)WorkRAML)[boff_] = val_;
#else
    ((uint8_t*)WorkRAML)[boff_ ^ 1] = val_;
#endif
   }
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

  {
   if(false || (A & 1))
    SMPC_Write(SH7095_mem_timestamp, SMPC_A, *DB);
  }

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

  {
   if(false || (A & 1))
   {
    uint8_t* const brp = &BackupRAM[(A >> 1) & 0x7FFF];

    if(*brp != (uint8_t)*DB)
    {
     *brp = (uint8_t)*DB;
     BackupRAM_Dirty = true;
    }
   }
  }

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

  {
   
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

  {
   if(false || (A & 1))
    STVIO_WriteIOGA(SH7095_mem_timestamp, IOGA_A, (uint8_t)*DB);
  }

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

static INLINE void BusRW_DB_CS0_u16_W1(const uint32_t A, uint32_t* DB, const bool BurstHax, int32_t* SH2DMAHax)
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

   return;
  }

  {
   /* ne16_wbo_be<T>(WorkRAML, byte_off, val) folded.  T can be
    * uint8_t, uint16_t, or uint32_t; sizeof(T) dispatches the right
    * write width.  Compiler folds away the dead branches per
    * template instantiation. */
   const uint32_t boff_ = A & 0xFFFFF;
   const uint16_t val_ = *DB >> (((A & 1) ^ (2 - 2)) << 3);
   WorkRAML[boff_ >> 1] = val_;
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

  {
   if(true || (A & 1))
    SMPC_Write(SH7095_mem_timestamp, SMPC_A, *DB);
  }

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

  {
   if(true || (A & 1))
   {
    uint8_t* const brp = &BackupRAM[(A >> 1) & 0x7FFF];

    if(*brp != (uint8_t)*DB)
    {
     *brp = (uint8_t)*DB;
     BackupRAM_Dirty = true;
    }
   }
  }

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

  {
   {
    const unsigned c = ((A >> 23) & 1) ^ 1;

    SH7095_SetFTI(&CPU[c], true);
    SH7095_SetFTI(&CPU[c], false);
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

  {
   if(true || (A & 1))
    STVIO_WriteIOGA(SH7095_mem_timestamp, IOGA_A, (uint8_t)*DB);
  }

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

static INLINE void BusRW_DB_CS0_u8_W0(const uint32_t A, uint32_t* DB, const bool BurstHax, int32_t* SH2DMAHax)
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
   *DB = *DB | 0xFFFF;

   return;
  }

  {
   /* ne16_rbo_be<uint16_t>(WorkRAML, byte_off): aligned u16 read
    * — host-endian-stored slot, direct index. */
   *DB = (*DB & 0xFFFF0000) | WorkRAML[(A & 0xFFFFE) >> 1];
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

  *DB = (*DB & 0xFFFF0000) | BIOSROM[(A & 0x7FFFE) >> 1];

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

  *DB = (*DB & 0xFFFF0000) | 0xFF00 | SMPC_Read(SH7095_mem_timestamp, SMPC_A);

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

  *DB = (*DB & 0xFFFF0000) | 0xFF00 | BackupRAM[(A >> 1) & 0x7FFF];

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

  *DB = (*DB & 0xFFFF0000) | 0xFF00 | STVIO_ReadIOGA(SH7095_mem_timestamp, IOGA_A);

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

static INLINE void BusRW_DB_CS0_u16_W0(const uint32_t A, uint32_t* DB, const bool BurstHax, int32_t* SH2DMAHax)
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
   *DB = *DB | 0xFFFF;

   return;
  }

  {
   /* ne16_rbo_be<uint16_t>(WorkRAML, byte_off): aligned u16 read
    * — host-endian-stored slot, direct index. */
   *DB = (*DB & 0xFFFF0000) | WorkRAML[(A & 0xFFFFE) >> 1];
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

  *DB = (*DB & 0xFFFF0000) | BIOSROM[(A & 0x7FFFE) >> 1];

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

  *DB = (*DB & 0xFFFF0000) | 0xFF00 | SMPC_Read(SH7095_mem_timestamp, SMPC_A);

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

  *DB = (*DB & 0xFFFF0000) | 0xFF00 | BackupRAM[(A >> 1) & 0x7FFF];

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

  *DB = (*DB & 0xFFFF0000) | 0xFF00 | STVIO_ReadIOGA(SH7095_mem_timestamp, IOGA_A);

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


/* Phase-8r2: BusRW_DB_CS12 retired into 6 named variants
 * via source-fold.  Phase-8q3 sizeof(T)+IsWrite dispatch
 * ladder to SCU_FromSH2_BusRW_DB_* collapses to one
 * direct named call per variant. */

static INLINE void BusRW_DB_CS12_u8_W0(const uint32_t A, uint32_t* DB, const bool BurstHax, int32_t* SH2DMAHax)
{

 //
 // CS1 and CS2: SCU
 //
 *DB = 0;

 /* Phase-8q3: sizeof(T) + IsWrite fold at BusRW_DB_CS12
  * template instantiation. */
 {
  SCU_FromSH2_BusRW_DB_u8_W0 (A, DB, SH2DMAHax);
 }
}

static INLINE void BusRW_DB_CS12_u8_W1(const uint32_t A, uint32_t* DB, const bool BurstHax, int32_t* SH2DMAHax)
{

 //
 // CS1 and CS2: SCU
 //

 /* Phase-8q3: sizeof(T) + IsWrite fold at BusRW_DB_CS12
  * template instantiation. */
 {
  SCU_FromSH2_BusRW_DB_u8_W1 (A, DB, SH2DMAHax);
 }
}

static INLINE void BusRW_DB_CS12_u16_W0(const uint32_t A, uint32_t* DB, const bool BurstHax, int32_t* SH2DMAHax)
{

 //
 // CS1 and CS2: SCU
 //
 *DB = 0;

 /* Phase-8q3: sizeof(T) + IsWrite fold at BusRW_DB_CS12
  * template instantiation. */
 {
  SCU_FromSH2_BusRW_DB_u16_W0(A, DB, SH2DMAHax);
 }
}

static INLINE void BusRW_DB_CS12_u16_W1(const uint32_t A, uint32_t* DB, const bool BurstHax, int32_t* SH2DMAHax)
{

 //
 // CS1 and CS2: SCU
 //

 /* Phase-8q3: sizeof(T) + IsWrite fold at BusRW_DB_CS12
  * template instantiation. */
 {
  SCU_FromSH2_BusRW_DB_u16_W1(A, DB, SH2DMAHax);
 }
}

static INLINE void BusRW_DB_CS12_u32_W0(const uint32_t A, uint32_t* DB, const bool BurstHax, int32_t* SH2DMAHax)
{

 //
 // CS1 and CS2: SCU
 //
 *DB = 0;

 /* Phase-8q3: sizeof(T) + IsWrite fold at BusRW_DB_CS12
  * template instantiation. */
 {
  SCU_FromSH2_BusRW_DB_u32_W0(A, DB, SH2DMAHax);
 }
}

static INLINE void BusRW_DB_CS12_u32_W1(const uint32_t A, uint32_t* DB, const bool BurstHax, int32_t* SH2DMAHax)
{

 //
 // CS1 and CS2: SCU
 //

 /* Phase-8q3: sizeof(T) + IsWrite fold at BusRW_DB_CS12
  * template instantiation. */
 {
  SCU_FromSH2_BusRW_DB_u32_W1(A, DB, SH2DMAHax);
 }
}


/* Phase-8r2: BusRW_DB_CS3 retired into 6 named variants
 * via source-fold. */

static INLINE void BusRW_DB_CS3_u8_W0(const uint32_t A, uint32_t* DB, const bool BurstHax, int32_t* SH2DMAHax)
{

 //
 // CS3: High work RAM/SDRAM, 0x06000000 ... 0x07FFFFFF
 //
 //  Timing is handled in BSC_BusWrite() and BSC_BusRead() in sh7095.inc
 //
 {
  /* ne16_rwbo_be<uint32_t, IsWrite>(WorkRAMH, byte_off, DB) folded:
   * aligned uint32_t BE bus read or write over uint16_t array.  Two
   * uint16_t halves: upper at index, lower at index+1.  Same on
   * BE and LE hosts (host-endian uint16s combined in MSB-first
   * order). */
  const uint32_t idx_ = (A & 0xFFFFC) >> 1;
  *DB = ((uint32_t)WorkRAMH[idx_] << 16) | WorkRAMH[idx_ + 1];
 }
}

static INLINE void BusRW_DB_CS3_u8_W1(const uint32_t A, uint32_t* DB, const bool BurstHax, int32_t* SH2DMAHax)
{

 //
 // CS3: High work RAM/SDRAM, 0x06000000 ... 0x07FFFFFF
 //
 //  Timing is handled in BSC_BusWrite() and BSC_BusRead() in sh7095.inc
 //
 {
  /* ne16_wbo_be<T>(WorkRAMH, byte_off, val) folded.  T is uint8_t
   * or uint16_t here (uint32_t caught above). */
  const uint32_t boff_ = A & 0xFFFFF;
  const uint8_t val_ = *DB >> (((A & 3) ^ (4 - 1)) << 3);
  {
#ifdef MSB_FIRST
   ((uint8_t*)WorkRAMH)[boff_] = val_;
#else
   ((uint8_t*)WorkRAMH)[boff_ ^ 1] = val_;
#endif
  }
 }
}

static INLINE void BusRW_DB_CS3_u16_W0(const uint32_t A, uint32_t* DB, const bool BurstHax, int32_t* SH2DMAHax)
{

 //
 // CS3: High work RAM/SDRAM, 0x06000000 ... 0x07FFFFFF
 //
 //  Timing is handled in BSC_BusWrite() and BSC_BusRead() in sh7095.inc
 //
 {
  /* ne16_rwbo_be<uint32_t, IsWrite>(WorkRAMH, byte_off, DB) folded:
   * aligned uint32_t BE bus read or write over uint16_t array.  Two
   * uint16_t halves: upper at index, lower at index+1.  Same on
   * BE and LE hosts (host-endian uint16s combined in MSB-first
   * order). */
  const uint32_t idx_ = (A & 0xFFFFC) >> 1;
  *DB = ((uint32_t)WorkRAMH[idx_] << 16) | WorkRAMH[idx_ + 1];
 }
}

static INLINE void BusRW_DB_CS3_u16_W1(const uint32_t A, uint32_t* DB, const bool BurstHax, int32_t* SH2DMAHax)
{

 //
 // CS3: High work RAM/SDRAM, 0x06000000 ... 0x07FFFFFF
 //
 //  Timing is handled in BSC_BusWrite() and BSC_BusRead() in sh7095.inc
 //
 {
  /* ne16_wbo_be<T>(WorkRAMH, byte_off, val) folded.  T is uint8_t
   * or uint16_t here (uint32_t caught above). */
  const uint32_t boff_ = A & 0xFFFFF;
  const uint16_t val_ = *DB >> (((A & 3) ^ (4 - 2)) << 3);
  WorkRAMH[boff_ >> 1] = val_;
 }
}

static INLINE void BusRW_DB_CS3_u32_W0(const uint32_t A, uint32_t* DB, const bool BurstHax, int32_t* SH2DMAHax)
{

 //
 // CS3: High work RAM/SDRAM, 0x06000000 ... 0x07FFFFFF
 //
 //  Timing is handled in BSC_BusWrite() and BSC_BusRead() in sh7095.inc
 //
 {
  /* ne16_rwbo_be<uint32_t, IsWrite>(WorkRAMH, byte_off, DB) folded:
   * aligned uint32_t BE bus read or write over uint16_t array.  Two
   * uint16_t halves: upper at index, lower at index+1.  Same on
   * BE and LE hosts (host-endian uint16s combined in MSB-first
   * order). */
  const uint32_t idx_ = (A & 0xFFFFC) >> 1;
  *DB = ((uint32_t)WorkRAMH[idx_] << 16) | WorkRAMH[idx_ + 1];
 }
}

static INLINE void BusRW_DB_CS3_u32_W1(const uint32_t A, uint32_t* DB, const bool BurstHax, int32_t* SH2DMAHax)
{

 //
 // CS3: High work RAM/SDRAM, 0x06000000 ... 0x07FFFFFF
 //
 //  Timing is handled in BSC_BusWrite() and BSC_BusRead() in sh7095.inc
 //
 {
  /* ne16_rwbo_be<uint32_t, IsWrite>(WorkRAMH, byte_off, DB) folded:
   * aligned uint32_t BE bus read or write over uint16_t array.  Two
   * uint16_t halves: upper at index, lower at index+1.  Same on
   * BE and LE hosts (host-endian uint16s combined in MSB-first
   * order). */
  const uint32_t idx_ = (A & 0xFFFFC) >> 1;
  {
   WorkRAMH[idx_ + 0] = *DB >> 16;
   WorkRAMH[idx_ + 1] = *DB;
  }
 }
}


//
//
//
/* CheatMemRead moved to ss_init.c (phase 7c).  Mednafen patches
 * (MDFNMP_RegSearchable) take its address by name; the extern decl is
 * in ss_init.h. */

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
   if(CPU[c].CCR & SH7095_CCR_CE)
   {
    for(uint32_t Abase = 0x00000000; Abase < 0x20000000; Abase += 0x08000000)
    {
     SH7095_Cache_WriteUpdate_u8(&CPU[c], Abase + A, V);
    }
   }
  }
 }
}
//
//
//
/* SetFastMemMap (file-static), InitFastMemMap, and SS_SetPhysMemMap
 * moved to ss_init.c (phase 7c). */

#include "sh7095.inc"
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <assert.h>
#include <boolean.h>
#include "../mednafen-types.h"
#include "scu.h"     /* SCU_UpdateDMA, SCU_UpdateDSP, SS_EVENT_SCU_* */
#include "smpc.h"    /* SMPC_Update */
#include "vdp1.h"    /* VDP1_Update */
#include "vdp2.h"    /* VDP2_Update */
#include "cdb.h"     /* CDB_Update */
#include "sound.h"   /* SOUND_Update */
#include "cart.h"    /* CART_GetEventHandler */
#include "stvio.h"   /* STVIO_UpdateInput */
#include "db.h"      /* CPUCACHE_EMUMODE_* */
#include "ss_state.h" /* BRAM_Init_Data, SS_Load/Backup RTC/BackupRAM/CartNV */
#include "../settings.h"
#include "../general.h"           /* MDFN_MidSync, log_cb (via cdstream.h) */
#include "../../libretro_settings.h" /* setting_midsync, setting_multitap_port*, retro_base_directory */
#include "../../disc.h"           /* disc_cleanup */
#include <streams/file_stream.h>  /* filestream_open/read/close/get_size */
#include <libretro.h>             /* RFILE, RETRO_VFS_*, RETRO_LOG_* */
#include <retro_miscellaneous.h>  /* ARRAY_SIZE */
#include <time.h>                 /* time, localtime, struct tm */

/* Phase-7c: the SH-2 event ring + dispatch machinery (Running,
 * events[], event_handlers[], next_event_ts, FindNextEventTS,
 * SH_DMA_EventHandler_M/_S, InitEvents, RebaseTS, SS_SetEventNT,
 * EventHandler, CheckEventsByMemTS{_Sub}, SS_RequestEHLExit,
 * SS_RequestMLExit) moved to ss_init.c.  ss.cpp drives the loop
 * via the externs declared in ss_init.h and provides the two
 * extern "C" SH7095_{M,S}_DMA_Update helpers below that wrap the
 * C++-only SH7095_DMA_Update(&CPU[c], et) method dispatch. */

int32_t SH7095_M_DMA_Update(int32_t et) { return SH7095_DMA_Update(&CPU[0], et); }
int32_t SH7095_S_DMA_Update(int32_t et) { return SH7095_DMA_Update(&CPU[1], et); }

/* ForceEventUpdates stays in ss.cpp -- the first loop dispatches into
 * SH7095_ForceInternalEventUpdates(&CPU[c]), which is an SH7095 class method
 * and not yet a C-callable wrapper.  After the SH7095 class -> struct
 * conversion (later phase) this function migrates to ss_init.c too.
 *
 * Called from RunLoop and (commented out) debug.cpp.  Touches the
 * event ring via the externs declared in ss_init.h. */
static void ForceEventUpdates(const sscpu_timestamp_t timestamp)
{
 unsigned c;
 unsigned evnum;
 for(c = 0; c < 2; c++)
  SH7095_ForceInternalEventUpdates(&CPU[c]);

 for(evnum = SS_EVENT__SYNFIRST + 1; evnum < SS_EVENT__SYNLAST; evnum++)
 {
  if(events[evnum].event_time != SS_EVENT_DISABLED_TS)
   events[evnum].event_time = event_handlers[evnum](timestamp);
 }

 next_event_ts = (Running > 0) ? FindNextEventTS() : 0;
}


#if defined(__GNUC__) && !defined(__clang__)
 #pragma GCC push_options
 #pragma GCC optimize("O2,no-unroll-loops,no-peel-loops,no-crossjumping")
#endif
/* Phase-8r2: RunLoop<bool EmulateICache> retired into 2
 * named variants via source-fold.  The extern "C"
 * wrappers call them directly. */

static NO_INLINE MDFN_HOT int32_t RunLoop_ICache(EmulateSpecStruct* espec)
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
    /* Phase-8p2: master Step dispatch.  RunLoop is templated on
     * EmulateICache so this folds to one direct call per
     * instantiation. */
    SH7095_Step_w0_C1(&CPU[0]);
    SH7095_DMA_BusTimingKludge(&CPU[0]);

    {
      SH7095_RunSlaveUntil(&CPU[1], CPU[0].timestamp);
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

static NO_INLINE MDFN_HOT int32_t RunLoop_NoICache(EmulateSpecStruct* espec)
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
    /* Phase-8p2: master Step dispatch.  RunLoop is templated on
     * EmulateICache so this folds to one direct call per
     * instantiation. */
    SH7095_Step_w0_C0(&CPU[0]);
    SH7095_DMA_BusTimingKludge(&CPU[0]);

    {
     while(MDFN_LIKELY(CPU[0].timestamp > CPU[1].timestamp))
     {
      SH7095_Step_w1_C0(&CPU[1]);
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

/* Phase-7f: SS_Reset moved to ss_init.c.  The three CPU class-method
 * calls it dispatches (CPU[c].TruePowerOn, CPU[0].Reset) are exposed
 * through the SH7095_{M,S}_{TruePowerOn,Reset} extern "C" wrappers
 * added below in this phase.  Retires when SH7095 becomes a C struct. */

/* Phase-7e: MidSync, UpdateSMPCInput, Emulate (and the file-statics
 * espec / AllowMidSync / cur_clock_div they share) moved to ss_init.c.
 * The C-side Emulate reaches the C++-only bits below through extern "C"
 * wrappers: SS_RunLoop_ICache / SS_RunLoop_NoICache wrap the
 * RunLoop<bool EmulateICache> template; SS_ForceEventUpdates wraps the
 * static ForceEventUpdates which keeps living in ss.cpp because its
 * first loop calls CPU[c].ForceInternalEventUpdates (an SH7095 class
 * method); SH7095_{M,S}_AdjustTS wraps CPU[0/1].AdjustTS.  All four
 * retire once the SH7095 class becomes a C struct. */
int32_t SS_RunLoop_ICache(EmulateSpecStruct* espec)                                   { return RunLoop_ICache(espec); }
int32_t SS_RunLoop_NoICache(EmulateSpecStruct* espec)                                 { return RunLoop_NoICache(espec); }
void    SS_ForceEventUpdates(int32_t timestamp)                                       { ForceEventUpdates(timestamp); }
void    SH7095_M_AdjustTS(int32_t delta)                                              { SH7095_AdjustTS(&CPU[0], delta); }
void    SH7095_S_AdjustTS(int32_t delta)                                              { SH7095_AdjustTS(&CPU[1], delta); }

/* Phase-7f: SH7095 wrappers used by InitCommon (Init / SetMD5 /
 * TruePowerOn) and SS_Reset (TruePowerOn / Reset).  Retires when
 * SH7095 becomes a C struct. */
MDFN_COLD void SH7095_M_Init(const bool emumode_full, const bool emumode_cb_only)     { SH7095_Init(&CPU[0], emumode_full, emumode_cb_only); }
MDFN_COLD void SH7095_S_Init(const bool emumode_full, const bool emumode_cb_only)     { SH7095_Init(&CPU[1], emumode_full, emumode_cb_only); }
void           SH7095_M_SetMD5(bool level)                                            { SH7095_SetMD5(&CPU[0], level); }
void           SH7095_S_SetMD5(bool level)                                            { SH7095_SetMD5(&CPU[1], level); }
MDFN_COLD void SH7095_M_TruePowerOn(void)                                             { SH7095_TruePowerOn(&CPU[0]); }
MDFN_COLD void SH7095_S_TruePowerOn(void)                                             { SH7095_TruePowerOn(&CPU[1]); }
MDFN_COLD void SH7095_M_Reset(bool power_on_reset)                                    { SH7095_Reset(&CPU[0], power_on_reset, false); }


//
//
//

/* Phase-7f: Cleanup, CartName typedef, InitCommon, CloseGame, and
 * MDFN_BackupSavFile moved to ss_init.c.  The SH7095 method calls
 * they used (CPU[c].Init/SetMD5/TruePowerOn from InitCommon, and
 * CPU[c].Reset/TruePowerOn from SS_Reset which also moved) are
 * reached through the extern "C" SH7095_{M,S}_{Init,SetMD5,
 * TruePowerOn,Reset} wrappers above.  Retires together with those
 * wrappers when the SH7095 class becomes a C struct. */

/* Phase-7b: BackupRAM / Cart NV / RTC file I/O moved to
 * ss_state.c.  The SS_Flush* public wrappers above still live
 * here so the file holds the public-ABI surface; their thin
 * bodies forward into the new C TU via the extern "C" decls
 * in ss_state.h. */

/* Phase-7d: EventsPacker struct + Save/Restore method bodies
 * moved to ss_state.c as a plain C struct (EventsPacker) + two
 * free functions (EventsPacker_Save / EventsPacker_Restore). */

/* Phase-7d: LibRetro_StateAction moved to ss_state.c.  The
 * four CPU class-method dispatch sites it used are exposed
 * through extern "C" wrappers below; those wrappers retire
 * when the SH7095 class becomes a C struct. */

void SH7095_M_StateAction(StateMem* sm, const unsigned load, const bool data_only, const char* sname)
{
 SH7095_StateAction(&CPU[0], sm, load, data_only, sname);
}

void SH7095_S_StateAction(StateMem* sm, const unsigned load, const bool data_only, const char* sname)
{
 SH7095_StateAction(&CPU[1], sm, load, data_only, sname);
}

void SH7095_M_PostStateLoad(const unsigned load, bool prev_NeedEmuICache, bool current_NeedEmuICache)
{
 SH7095_PostStateLoad(&CPU[0], load, prev_NeedEmuICache, current_NeedEmuICache);
}

void SH7095_S_PostStateLoad(const unsigned load, bool prev_NeedEmuICache, bool current_NeedEmuICache)
{
 SH7095_PostStateLoad(&CPU[1], load, prev_NeedEmuICache, current_NeedEmuICache);
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

extern retro_log_printf_t log_cb;

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

/* ===================================================================
 * Phase-7e: per-frame Emulate() loop + MidSync helper
 * =================================================================== */

/* Externs into ss.cpp -- promoted to TU-external in phase 7d. */
extern bool          NeedEmuICache;
extern int           ActiveCartType;
extern int64_t       UpdateInputLastBigTS;

/* Externs for the libretro front-end / game-info structs. */
extern MDFNGI        EmulatedSS;
extern uint32_t      IBufferCount;

/* C wrappers that ss.cpp publishes for our use (extern "C" defined
 * there); these forward into the SH7095 class instances and the
 * template-parameterised RunLoop body that still live in C++.
 * Each retires once the SH7095 class becomes a C struct. */
int32_t SS_RunLoop_ICache(struct EmulateSpecStruct* espec);
int32_t SS_RunLoop_NoICache(struct EmulateSpecStruct* espec);
void    SS_ForceEventUpdates(int32_t timestamp);
void    SH7095_M_AdjustTS(int32_t delta);
void    SH7095_S_AdjustTS(int32_t delta);

/* Frame-scoped state.  espec is the active EmulateSpecStruct
 * pointer Emulate received this frame -- shared with MidSync via
 * file-static visibility. AllowMidSync gates whether MidSync's
 * one-shot mid-frame callback fires (front-end flag); reset on
 * each frame, cleared on first fire. cur_clock_div is SMPC's
 * frame-start clock divisor (carried for the per-tick input
 * elapsed-time conversion). */
static struct EmulateSpecStruct* espec;
static bool                      AllowMidSync;
static int32_t                   cur_clock_div;

static INLINE void UpdateSMPCInput(const sscpu_timestamp_t timestamp)
{
 int32_t elapsed_time;

 SMPC_TransformInput();

 elapsed_time = (((int64_t)timestamp * cur_clock_div * 1000 * 1000) - UpdateInputLastBigTS) / (EmulatedSS.MasterClock / MDFN_MASTERCLOCK_FIXED(1));

 UpdateInputLastBigTS += (int64_t)elapsed_time * (EmulatedSS.MasterClock / MDFN_MASTERCLOCK_FIXED(1));

 /* ST-V samples gamepad/gun/coin state into its own DataIn buffer on
  * the same cadence SMPC samples virtual ports. */
 if(ActiveCartType == CART_STV)
   STVIO_UpdateInput(elapsed_time);

 SMPC_UpdateInput(elapsed_time);
}

sscpu_timestamp_t MidSync(const sscpu_timestamp_t timestamp)
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

void Emulate(struct EmulateSpecStruct* espec_arg)
{
 int32_t end_ts;
 unsigned c;

 espec = espec_arg;
 AllowMidSync = setting_midsync;

 cur_clock_div = SMPC_StartFrame();
 UpdateSMPCInput(0);
 VDP2_StartFrame(espec, cur_clock_div == 61);
 CART_SetCPUClock(EmulatedSS.MasterClock / MDFN_MASTERCLOCK_FIXED(1), cur_clock_div);
 espec->SoundBufSize = 0;
 espec->MasterCycles = 0;

 if (NeedEmuICache)
  end_ts = SS_RunLoop_ICache(espec);
 else
  end_ts = SS_RunLoop_NoICache(espec);
 assert(end_ts >= 0);

 SS_ForceEventUpdates(end_ts);

 SMPC_EndFrame(espec, end_ts);

 RebaseTS(end_ts);

 CDB_ResetTS();
 SOUND_AdjustTS(-end_ts);
 VDP1_AdjustTS(-end_ts);
 VDP2_AdjustTS(-end_ts);
 SMPC_ResetTS();
 SCU_AdjustTS(-end_ts);
 CART_AdjustTS(-end_ts);

 UpdateInputLastBigTS -= (int64_t)end_ts * cur_clock_div * 1000 * 1000;

 SH7095_mem_timestamp -= end_ts; /* Update before SH7095 AdjustTS calls. */

 /* CPU[c].AdjustTS(-end_ts) for c in {0,1} via extern "C" wrappers. */
 SH7095_M_AdjustTS(-end_ts);
 SH7095_S_AdjustTS(-end_ts);
 (void)c;

 espec->MasterCycles  = (int64_t)end_ts * cur_clock_div;
 espec->SoundBufSize += IBufferCount;
 IBufferCount         = 0;

 SMPC_UpdateOutput();

 /* Backup-RAM and cart-NV dirty tracking.
  *
  * Previously this block performed synchronous file I/O from inside
  * Emulate() (SaveBackupRAM/SaveCartNV under a master-cycle countdown).
  * That had two big problems for a libretro core:
  *   1. Run-ahead / rewind / netplay re-emulate frames repeatedly. The
  *      cycle-counted delay fires identically on each pass, so a single
  *      real frame could produce two or three full SaveBackupRAM disk
  *      writes -- visible as stutter and unstable frame pacing.
  *   2. The save is fully synchronous (FileStream::write+close) on the
  *      emulation thread, so the duration is unpredictable under load.
  *
  * The fix is to keep BackupRAM_Dirty (and the cart-NV dirty bit) as
  * pure flags here, and let libretro.cpp flush them from retro_run --
  * outside Emulate, with awareness of RETRO_ENVIRONMENT_GET_AUDIO_VIDEO_ENABLE
  * so run-ahead simulation frames don't trigger writes. The frontend
  * can also manage Backup RAM directly via RETRO_MEMORY_SAVE_RAM. */
 if(CART_GetClearNVDirty())
  CartNV_Dirty = true;
}

/* ===================================================================
 * Phase-7f: InitCommon / SS_Reset / Cleanup / CloseGame /
 *           MDFN_BackupSavFile
 *
 * These are the boot-time orchestration entry points the libretro
 * front-end calls (InitCommon from retro_load_game, SS_Reset from
 * retro_reset, CloseGame from retro_unload_game).  Each reaches
 * into the SH7095 CPU[2] instances that still live in ss.cpp via
 * extern "C" wrappers added there in this phase
 * (SH7095_{M,S}_{Init,SetMD5,TruePowerOn,Reset}).  The wrappers
 * retire once the SH7095 class becomes a C struct.
 * =================================================================== */

/* Externs into ss.cpp (TU-external definitions live there). */
extern uint8_t       WorkRAM[];
extern uint16_t*     WorkRAML;
extern uint16_t*     WorkRAMH;
extern uint16_t      BIOSROM[];   /* 524288 / sizeof(uint16_t) entries */
extern uint32_t      SH7095_DB;
extern uint32_t      ss_horrible_hacks;
extern bool          is_pal;
extern sha256_digest BIOS_SHA256;

/* Defined in libretro.c (no header to include for this -- ss.cpp and
 * settings.c each redeclare it locally; match that pattern). */
extern char retro_base_directory[4096];

/* SH7095 method dispatch wrappers (extern "C" defined in ss.cpp). */
void SH7095_ConstructAll(void) MDFN_COLD;
void SH7095_M_Init(const bool emumode_full, const bool emumode_cb_only) MDFN_COLD;
void SH7095_S_Init(const bool emumode_full, const bool emumode_cb_only) MDFN_COLD;
void SH7095_M_SetMD5(bool level);
void SH7095_S_SetMD5(bool level);
void SH7095_M_TruePowerOn(void) MDFN_COLD;
void SH7095_S_TruePowerOn(void) MDFN_COLD;
void SH7095_M_Reset(bool power_on_reset)    MDFN_COLD;

typedef struct
{
   const unsigned type;
   const char *name;
} CartName;

static MDFN_COLD void Cleanup(void)
{
 CART_Kill();

 VDP1_Kill();
 VDP2_Kill();
 SOUND_Kill();
 CDB_Kill();
 STVIO_Kill();
 SMPC_Kill();

 disc_cleanup();
}

bool MDFN_COLD InitCommon(const unsigned cpucache_emumode, const unsigned horrible_hacks, const unsigned cart_type, const unsigned smpc_area, const char* rom_dir, const char* main_fname, const struct STVGameInfo* sgi)
{
   unsigned i;
   /* C99 hoists block-scoped declarations; in C99 they can live inside
    * the if-bodies they belong to, matching the original C++ style. */
   {
      typedef struct {
         unsigned mode;
         const char* name;
      } CPUCacheEmuMode;
      static const CPUCacheEmuMode CPUCacheEmuModes[] =
      {
         { CPUCACHE_EMUMODE_DATA_CB, _("Data only, with high-level bypass") },
         { CPUCACHE_EMUMODE_DATA,    _("Data only") },
         { CPUCACHE_EMUMODE_FULL,    _("Full") },
      };
      const char* cem = _("Unknown");
      unsigned k;

      for(k = 0; k < ARRAY_SIZE(CPUCacheEmuModes); k++)
      {
         if(CPUCacheEmuModes[k].mode == cpucache_emumode)
         {
            cem = CPUCacheEmuModes[k].name;
            break;
         }
      }
      log_cb(RETRO_LOG_INFO, "CPU Cache Emulation Mode: %s\n", cem);
   }

   if(horrible_hacks)
      log_cb(RETRO_LOG_INFO, "Horrible hacks: 0x%08x\n", horrible_hacks);

   {
      const CartName CartNames[] =
      {
         { CART_NONE,       "None" },
         { CART_BACKUP_MEM, "Backup Memory" },
         { CART_EXTRAM_1M,  "1MiB Extended RAM" },
         { CART_EXTRAM_4M,  "4MiB Extended RAM" },
         { CART_KOF95,      "King of Fighters '95 ROM" },
         { CART_ULTRAMAN,   "Ultraman ROM" },
         { CART_CS1RAM_16M, _("16MiB CS1 RAM") },
         { CART_NLMODEM,    _("Netlink Modem") },
         { CART_STV,        _("Sega Titan Video (ST-V)") },
         { CART_BOOTROM,    _("Bootable ROM") }
      };
      const char* cn = NULL;

      log_cb(RETRO_LOG_INFO, "Region: 0x%01x.\n", smpc_area);

      for(i = 0; i < ARRAY_SIZE(CartNames); i++)
      {
         if(CartNames[i].type != cart_type)
            continue;
         cn = CartNames[i].name;
         break;
      }
      if(cn)
         log_cb(RETRO_LOG_INFO, "Cart: %s.\n", cn);
      else
         log_cb(RETRO_LOG_INFO, "Cart: Unknown (%d).\n", cart_type);
   }

   NeedEmuICache = (cpucache_emumode == CPUCACHE_EMUMODE_FULL);
   /* Phase-9 step 5: SH7095 ctor dropped, so the once-only per-CPU
    * member init that the ctor used to do has to run BEFORE the first
    * SH7095_*_Init call.  SH7095_ConstructAll is idempotent in the
    * sense that calling it twice would just re-init the same fields,
    * but call sites should be single. */
   SH7095_ConstructAll();
   SH7095_M_Init((cpucache_emumode == CPUCACHE_EMUMODE_FULL), (cpucache_emumode == CPUCACHE_EMUMODE_DATA_CB));
   SH7095_S_Init((cpucache_emumode == CPUCACHE_EMUMODE_FULL), (cpucache_emumode == CPUCACHE_EMUMODE_DATA_CB));
   SH7095_M_SetMD5(false);
   SH7095_S_SetMD5(true);

   SH7095_mem_timestamp = 0;
   SH7095_DB = 0;

   ss_horrible_hacks = horrible_hacks;

   /* Initialise backup memory. */
   memset(BackupRAM, 0x00, sizeof(BackupRAM));
   for(i = 0; i < 0x40; i++)
      BackupRAM[i] = BRAM_Init_Data[i & 0x0F];

   /* Call InitFastMemMap() before functions like SOUND_Init(). */
   if(!InitFastMemMap())
   {
      Cleanup();
      return false;
   }
   SS_SetPhysMemMap(0x00000000, 0x000FFFFF, BIOSROM, 512 * 1024, false);
   SS_SetPhysMemMap(0x00200000, 0x003FFFFF, WorkRAML, WORKRAM_BANK_SIZE_BYTES, true);
   SS_SetPhysMemMap(0x06000000, 0x07FFFFFF, WorkRAMH, WORKRAM_BANK_SIZE_BYTES, true);
   MDFNMP_RegSearchable(0x00200000, WORKRAM_BANK_SIZE_BYTES);
   MDFNMP_RegSearchable(0x06000000, WORKRAM_BANK_SIZE_BYTES);

   if(!CART_Init(cart_type, rom_dir, main_fname, sgi))
   {
      Cleanup();
      return false;
   }
   ActiveCartType = cart_type;

   {
      const bool is_stv = (cart_type == CART_STV);
      /* ST-V runs on a monitor, not on a TV; force 60 Hz timing
       * regardless of region. PAL ST-V cabinets exist but the video
       * signal is still 60 Hz. */
      const bool PAL = is_pal = (!is_stv) && (smpc_area & SMPC_AREA__PAL_MASK);
      const int32_t MasterClock = PAL ? 1734687500 : 1746818182; /* NTSC: 1746818181.818..., PAL: 1734687500-ish */
      const char* bios_filename;
      int sls = MDFN_GetSettingI(PAL ? "ss.slstartp" : "ss.slstart");
      int sle = MDFN_GetSettingI(PAL ? "ss.slendp"   : "ss.slend");
      const uint64_t vdp2_affinity = 0; /* LibRetro: unused */

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
         /* ST-V BIOSes are 128 KiB (vs Saturn's 512 KiB) and live in their
          * own filename namespace. Hardcoded to match the fork's existing
          * BIOS-filename convention for sega_101.bin / mpr-17933.bin.
          * Users supply the actual files in retro_base_directory.
          *   JP / Asia-NTSC:                              epr-20091.ic8
          *   NA / CSA-NTSC / CSA-PAL:                     epr-17952a.ic8
          *   EU / Asia-PAL / Korea / everything else:     epr-17954a.ic8 */
         if(smpc_area == SMPC_AREA_JP || smpc_area == SMPC_AREA_ASIA_NTSC)
            bios_filename = "epr-20091.ic8";
         else if(smpc_area == SMPC_AREA_NA || smpc_area == SMPC_AREA_CSA_NTSC || smpc_area == SMPC_AREA_CSA_PAL)
            bios_filename = "epr-17952a.ic8";
         else
            bios_filename = "epr-17954a.ic8";
      }
      else if(smpc_area == SMPC_AREA_JP || smpc_area == SMPC_AREA_ASIA_NTSC)
         bios_filename = "sega_101.bin"; /* Japan */
      else
         bios_filename = "mpr-17933.bin"; /* North America and Europe */

      {
         char bios_path[4096];
         RFILE *BIOSFile;
         int64_t bios_size;
         unsigned bw;

         snprintf(bios_path, sizeof(bios_path), "%s" RETRO_SLASH "%s", retro_base_directory, bios_filename);

         BIOSFile = filestream_open(bios_path,
               RETRO_VFS_FILE_ACCESS_READ,
               RETRO_VFS_FILE_ACCESS_HINT_NONE);

         if(!BIOSFile)
         {
            log_cb(RETRO_LOG_ERROR, "Cannot open BIOS file \"%s\".\n", bios_path);
            Cleanup();
            return false;
         }

         /* Saturn BIOSes are 512 KiB; ST-V BIOSes are 128 KiB
          * (mapped into the upper half of the 512 KiB BIOSROM[] array
          * and read by the SH-2 from the same 0x00000000-0x000FFFFF
          * window). Accept either size.
          *
          * filestream_get_size must come AFTER the BIOSFile NULL check
          * -- on filestream_open failure BIOSFile is NULL and passing
          * it to get_size would be undefined. */
         bios_size = filestream_get_size(BIOSFile);
         if(bios_size != 524288 && !(is_stv && bios_size == 131072))
         {
            log_cb(RETRO_LOG_ERROR, "BIOS file \"%s\" is of an incorrect size.\n", bios_path);
            filestream_close(BIOSFile);
            Cleanup();
            return false;
         }

         memset(BIOSROM, 0xFF, 512 * 1024);
         /* Short read between get_size and the actual read would
          * leave BIOSROM half-loaded (head: partial BIOS bytes,
          * tail: 0xFF from the memset above), BIOS_SHA256 would
          * hash the corrupted data, and the byte-swap loop below
          * would scramble it further.  Fail init with a clear
          * error rather than silently emulating with a corrupted
          * BIOS image. */
         if(filestream_read(BIOSFile, BIOSROM, bios_size) != bios_size)
         {
            log_cb(RETRO_LOG_ERROR, "BIOS file \"%s\" could not be fully read (short or failed read).\n", bios_path);
            filestream_close(BIOSFile);
            Cleanup();
            return false;
         }
         filestream_close(BIOSFile);
         BIOS_SHA256 = sha256(BIOSROM, 512 * 1024);

         /* swap endian-ness */
         for(bw = 0; bw < (unsigned)(bios_size / 2); bw++)
         {
            /* MDFN_de16msb folded: BE-on-disk to host-endian. */
#ifndef MSB_FIRST
            BIOSROM[bw] = (uint16_t)((BIOSROM[bw] << 8) | (BIOSROM[bw] >> 8));
#endif
         }
      }

      EmulatedSS.MasterClock = MDFN_MASTERCLOCK_FIXED(MasterClock);

      SCU_Init();
      SMPC_Init(smpc_area, MasterClock, is_stv);
      VDP1_Init();
      VDP2_Init(PAL, vdp2_affinity);
      VDP2_SetGetVideoParams(&EmulatedSS, true, sls, sle, true, DoHBlend);
      CDB_Init();
      SOUND_Init();

      InitEvents();
      UpdateInputLastBigTS = 0;

      /* Apply multi-tap state to SMPC. */
      SMPC_SetMultitap(0, setting_multitap_port1);
      SMPC_SetMultitap(1, setting_multitap_port2);

      if(is_stv)
      {
         /* ST-V provides its own SMPC port shim (handles AK93C45 EEPROM
          * and 68K sound-CPU control), and routes player input through
          * STVIO_SetInput / STVIO_UpdateInput rather than the standard
          * per-virtual-port path. Init the I/O board first, then inject
          * the port shims into SMPC super-ports 0 and 1. */
         unsigned sp;
         STVIO_Init(sgi);
         for(sp = 0; sp < 2; sp++)
            SMPC_SetInput(sp, "extern", (uint8_t*)STVIO_GetSMPCDevice(sp));
      }
   }

   SS_LoadRTC();
   SS_LoadBackupRAM();
   SS_LoadCartNV();

   SS_BackupBackupRAM();
   SS_BackupCartNV();

   /* Just-loaded state is by definition clean. The cycle-counted
    * SaveDelay variables are gone -- see comment in Emulate(). */
   BackupRAM_Dirty = false;
   CART_GetClearNVDirty();
   CartNV_Dirty = false;

   if(MDFN_GetSettingB("ss.smpc.autortc"))
   {
      time_t ut;
      struct tm* ht;

      if((ut = time(NULL)) == (time_t)-1)
      {
         log_cb(RETRO_LOG_ERROR, "AutoRTC error #1\n");
         /* Previously this just returned false, leaving the VDP2
          * render thread, semaphore, queue, SCU, SMPC, etc. fully
          * initialised. A subsequent load would then double-init
          * and race with the orphaned render thread. */
         Cleanup();
         return false;
      }

      if((ht = localtime(&ut)) == NULL)
      {
         log_cb(RETRO_LOG_ERROR, "AutoRTC error #2\n");
         Cleanup();
         return false;
      }

      SMPC_SetRTC(ht, MDFN_GetSettingUI("ss.smpc.autortc.lang"));
   }

   SS_Reset(true);

   return true;
}

/* SS_Reset is the public-ABI reset entry called from libretro.c's
 * retro_reset and from InitCommon's final step.  Reaches into both
 * SH7095 CPU instances via the extern "C" wrappers. */
void SS_Reset(bool powering_up)
{
 SH7095_BusLock = 0;

 if(powering_up)
 {
   memset(WorkRAM, 0x00, sizeof(WorkRAM[0]) * (2 * WORKRAM_BANK_SIZE_BYTES));
   /* TODO: Check real hardware. */
 }

 if(powering_up)
 {
  SH7095_M_TruePowerOn();
  SH7095_S_TruePowerOn();
 }

 SCU_Reset(powering_up);
 SH7095_M_Reset(powering_up);

 /* ST-V's I/O board must reset before SMPC -- SMPC's port shim
  * (IODevice_STVSMPC) consults state that STVIO_Reset re-initialises. */
 if(ActiveCartType == CART_STV)
   STVIO_Reset(powering_up);

 SMPC_Reset(powering_up);

 VDP1_Reset(powering_up);
 VDP2_Reset(powering_up);

 CDB_Reset(powering_up);

 SOUND_Reset(powering_up);

 CART_Reset(powering_up);
}

void MDFN_COLD CloseGame(void)
{
#ifdef SH7095_OP_PAIR_PROFILE
 SS_DumpOpPairProfile();
#endif

 SS_SaveBackupRAM();
 SS_SaveCartNV();
 SS_SaveRTC();

 Cleanup();
}

void MDFN_BackupSavFile(const uint8_t max_backup_count, const char* sav_ext)
{
   /* stub for libretro port */
   (void)max_backup_count;
   (void)sav_ext;
}
