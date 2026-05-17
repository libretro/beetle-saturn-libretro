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
/* Phase-9 step 6: mednafen.h / git.h dropped to avoid pulling
 * <algorithm>, <string>, <vector>, <map> into a TU whose body is
 * already pure C in everything except the #include surface.  The
 * MDFNGI typedef ss.cpp needs for `extern MDFNGI EmulatedSS;` lives
 * in mdfn_gameinfo.h which is C-clean (factored out of git.h
 * specifically so C TUs can include it). */
#include "../mdfn_gameinfo.h"
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
extern "C" MDFN_COLD void SH7095_ConstructAll(void)
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
extern "C" void SH7095_S_SetActive(bool active)
{
 SH7095_SetActive(&CPU[1], active);
}

extern "C" void SH7095_M_SetNMI(bool level)
{
 SH7095_SetNMI(&CPU[0], level);
}

/* Used by vdp2.c (converted from C++) for the HORRIBLEHACK_NOSH2DMA-
 * LINE106 path -- vdp2's CPU loop iterates CPU[0..1] once per scanline
 * advance and sets the kludge flag.  Matches the SetActive / SetNMI
 * proxies above; cpu index picks master (0) / slave (1). */
extern "C" void SH7095_SetExtHaltDMAKludge(int cpu, bool state)
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

/* Phase-7c: the SH-2 event ring + dispatch machinery (Running,
 * events[], event_handlers[], next_event_ts, FindNextEventTS,
 * SH_DMA_EventHandler_M/_S, InitEvents, RebaseTS, SS_SetEventNT,
 * EventHandler, CheckEventsByMemTS{_Sub}, SS_RequestEHLExit,
 * SS_RequestMLExit) moved to ss_init.c.  ss.cpp drives the loop
 * via the externs declared in ss_init.h and provides the two
 * extern "C" SH7095_{M,S}_DMA_Update helpers below that wrap the
 * C++-only SH7095_DMA_Update(&CPU[c], et) method dispatch. */

extern "C" int32_t SH7095_M_DMA_Update(int32_t et) { return SH7095_DMA_Update(&CPU[0], et); }
extern "C" int32_t SH7095_S_DMA_Update(int32_t et) { return SH7095_DMA_Update(&CPU[1], et); }

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
extern "C" int32_t SS_RunLoop_ICache(EmulateSpecStruct* espec)                                   { return RunLoop_ICache(espec); }
extern "C" int32_t SS_RunLoop_NoICache(EmulateSpecStruct* espec)                                 { return RunLoop_NoICache(espec); }
extern "C" void    SS_ForceEventUpdates(int32_t timestamp)                                       { ForceEventUpdates(timestamp); }
extern "C" void    SH7095_M_AdjustTS(int32_t delta)                                              { SH7095_AdjustTS(&CPU[0], delta); }
extern "C" void    SH7095_S_AdjustTS(int32_t delta)                                              { SH7095_AdjustTS(&CPU[1], delta); }

/* Phase-7f: SH7095 wrappers used by InitCommon (Init / SetMD5 /
 * TruePowerOn) and SS_Reset (TruePowerOn / Reset).  Retires when
 * SH7095 becomes a C struct. */
extern "C" MDFN_COLD void SH7095_M_Init(const bool emumode_full, const bool emumode_cb_only)     { SH7095_Init(&CPU[0], emumode_full, emumode_cb_only); }
extern "C" MDFN_COLD void SH7095_S_Init(const bool emumode_full, const bool emumode_cb_only)     { SH7095_Init(&CPU[1], emumode_full, emumode_cb_only); }
extern "C" void           SH7095_M_SetMD5(bool level)                                            { SH7095_SetMD5(&CPU[0], level); }
extern "C" void           SH7095_S_SetMD5(bool level)                                            { SH7095_SetMD5(&CPU[1], level); }
extern "C" MDFN_COLD void SH7095_M_TruePowerOn(void)                                             { SH7095_TruePowerOn(&CPU[0]); }
extern "C" MDFN_COLD void SH7095_S_TruePowerOn(void)                                             { SH7095_TruePowerOn(&CPU[1]); }
extern "C" MDFN_COLD void SH7095_M_Reset(bool power_on_reset)                                    { SH7095_Reset(&CPU[0], power_on_reset, false); }


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

extern "C" void SH7095_M_StateAction(StateMem* sm, const unsigned load, const bool data_only, const char* sname)
{
 SH7095_StateAction(&CPU[0], sm, load, data_only, sname);
}

extern "C" void SH7095_S_StateAction(StateMem* sm, const unsigned load, const bool data_only, const char* sname)
{
 SH7095_StateAction(&CPU[1], sm, load, data_only, sname);
}

extern "C" void SH7095_M_PostStateLoad(const unsigned load, bool prev_NeedEmuICache, bool current_NeedEmuICache)
{
 SH7095_PostStateLoad(&CPU[0], load, prev_NeedEmuICache, current_NeedEmuICache);
}

extern "C" void SH7095_S_PostStateLoad(const unsigned load, bool prev_NeedEmuICache, bool current_NeedEmuICache)
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
