/******************************************************************************/
/* Mednafen Sega Saturn Emulation Module                                      */
/******************************************************************************/
/* ss.c - Saturn Core Emulation and Support Functions
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
 * <algorithm>, <string>, <vector>, <map> (no longer used) into a TU whose
 * body is otherwise pure C.  The MDFNGI typedef this file needs for
 * `extern MDFNGI EmulatedSS;` lives in mdfn_gameinfo.h which is
 * C-clean (factored out of git.h specifically so C TUs can include
 * it).  EmulateSpecStruct lives in emuspec.h for the same reason --
 * typedef'd at file scope so both C and TUs can name the type
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
#include "../../zip_reader.h"

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

#include "ss_state.h"

#include "sh7095.h"

static uint8_t SCU_MSH2VectorFetch(void);
static uint8_t SCU_SSH2VectorFetch(void);

/* CheckEventsByMemTS forward decl removed (phase 7c): canonical
 * decl in ss_init.h; definition lives in ss.c. */

SH7095 CPU[2];

MDFN_COLD void SH7095_ConstructAll(void)
{
 SH7095_Construct(&CPU[0], "SH2-M", SS_EVENT_SH2_M_DMA, SCU_MSH2VectorFetch);
 SH7095_Construct(&CPU[1], "SH2-S", SS_EVENT_SH2_S_DMA, SCU_SSH2VectorFetch);
}

/* C-linkage proxies bridging the SH7095 class to C consumers.  Used
 * by smpc.c (the converted SMPC TU) to drive slave-CPU enable/disable
 * (SMPC SH2_RESET / SH2_GET / SH2_SET commands) and to assert NMI
 * (SMPC SYSRES / SNDRES / CDON / CDOFF paths).  The proxies hide
 * `&CPU[0]` / `&CPU[1]` from external TUs so they don't need to know
 * about the master/slave SH7095 array layout in ss.c.
 *
 * The proxies stay here in ss.c rather than in sh7095.inc because
 * the CPU[2] global lives in this TU.  Both were always called with
 * hard-coded CPU indices (SetActive only ever with 1 = slave,
 * SetNMI only ever with 0 = master), so the per-side `SH7095_S_*`
 * / `SH7095_M_*` shape -- drop the int parameter, encode the CPU
 * in the function name -- is the right one.  An earlier set of
 * SH7095_{M,S}_* wrappers for ss.c-internal lifecycle (Init,
 * SetMD5, TruePowerOn, Reset, AdjustTS, StateAction, PostStateLoad)
 * was retired once SH7095 became a C struct -- those call sites
 * now invoke sh7095.h's SH7095_* exports directly with &CPU[0] /
 * &CPU[1].  Only externally-called proxies remain. */
void SH7095_S_SetActive(bool active)
{
 SH7095_SetActive(&CPU[1], active);
}

void SH7095_M_SetNMI(bool level)
{
 SH7095_SetNMI(&CPU[0], level);
}

/* Used by vdp2.c  for the HORRIBLEHACK_NOSH2DMA-
 * LINE106 path -- vdp2's CPU loop iterates CPU[0..1] once per scanline
 * advance and sets the kludge flag.  Matches the SetActive / SetNMI
 * proxies above; cpu index picks master (0) / slave (1). */
void SH7095_SetExtHaltDMAKludge(int cpu, bool state)
{
 SH7095_SetExtHaltDMAKludgeFromVDP2(&CPU[cpu], state);
}

/* promoted from file-static -- ss.c's InitCommon
 * loads it from disk and assigns BIOS_SHA256.  Definition stays
 * here so the rest of ss.c's globals layout is undisturbed. */
uint16_t BIOSROM[524288 / sizeof(uint16_t)];
uint8_t WorkRAM[2*WORKRAM_BANK_SIZE_BYTES]; // unified 2MB work ram for linear access.
// Effectively 32-bit in reality, but 16-bit here because of CPU interpreter design(regarding fastmap).
uint16_t* WorkRAML = (uint16_t*)(WorkRAM + (WORKRAM_BANK_SIZE_BYTES*0));
uint16_t* WorkRAMH = (uint16_t*)(WorkRAM + (WORKRAM_BANK_SIZE_BYTES*1));
// BackupRAM is exposed (no longer file-static) so libretro.c can hand a
// pointer to the frontend via retro_get_memory_data(RETRO_MEMORY_SAVE_RAM).
// BackupRAM_Dirty and CartNV_Dirty are sticky flags maintained here and
// drained by libretro.c from outside Emulate() -- see comment in
// Emulate() above. The old master-cycle delay variables are gone.
uint8_t BackupRAM[32768];
uint8_t BackupRAM_StateHelper[32768];
bool BackupRAM_Dirty;
bool CartNV_Dirty;

/* ss.c's InitCommon zero-initialises this on game load
 * (line ~867); ss.c's MidSync helper UpdateSMPCInput / Emulate
 * loop read and update it.  Define lives here, extern declared in
 * ss.c for the C-side accessors. */
int64_t UpdateInputLastBigTS;

int32_t SH7095_mem_timestamp;
/* SH7095_BusLock is read from ss.c's SH_DMA_EventHandler -- promoted
 * from file-static to TU-external in phase 7c. */
uint32_t SH7095_BusLock;
uint32_t SH7095_DB;

#include "scu.inc"

sha256_digest BIOS_SHA256;   // SHA-256 hash of the currently-loaded BIOS; used for save state sanity checks.
int ActiveCartType;		// Used in save states.

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
// (BurstHax-driven timing in high work RAM is handled in
//  SH7095_BSC_BusWrite / SH7095_BSC_BusRead in sh7095.inc.  The
//  BusRW_DB_CS* functions below do only data movement; the BurstHax
//  parameter they used to take was unused and has been dropped.)
//

static INLINE void BusRW_DB_CS0_u8_W1(const uint32_t A, uint32_t* DB, int32_t* SH2DMAHax)
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
    * macro-monomorphized form. */
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

static INLINE void BusRW_DB_CS0_u16_W1(const uint32_t A, uint32_t* DB, int32_t* SH2DMAHax)
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
    * macro-monomorphized form. */
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

static INLINE void BusRW_DB_CS0_u8_W0(const uint32_t A, uint32_t* DB, int32_t* SH2DMAHax)
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

static INLINE void BusRW_DB_CS0_u16_W0(const uint32_t A, uint32_t* DB, int32_t* SH2DMAHax)
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

static INLINE void BusRW_DB_CS12_u8_W0(const uint32_t A, uint32_t* DB, int32_t* SH2DMAHax)
{

 //
 // CS1 and CS2: SCU
 //
 *DB = 0;

 /* sizeof(T) + IsWrite fold at BusRW_DB_CS12
  * macro-monomorphized form. */
 {
  SCU_FromSH2_BusRW_DB_u8_W0 (A, DB, SH2DMAHax);
 }
}

static INLINE void BusRW_DB_CS12_u8_W1(const uint32_t A, uint32_t* DB, int32_t* SH2DMAHax)
{

 //
 // CS1 and CS2: SCU
 //

 /* sizeof(T) + IsWrite fold at BusRW_DB_CS12
  * macro-monomorphized form. */
 {
  SCU_FromSH2_BusRW_DB_u8_W1 (A, DB, SH2DMAHax);
 }
}

static INLINE void BusRW_DB_CS12_u16_W0(const uint32_t A, uint32_t* DB, int32_t* SH2DMAHax)
{

 //
 // CS1 and CS2: SCU
 //
 *DB = 0;

 /* sizeof(T) + IsWrite fold at BusRW_DB_CS12
  * macro-monomorphized form. */
 {
  SCU_FromSH2_BusRW_DB_u16_W0(A, DB, SH2DMAHax);
 }
}

static INLINE void BusRW_DB_CS12_u16_W1(const uint32_t A, uint32_t* DB, int32_t* SH2DMAHax)
{

 //
 // CS1 and CS2: SCU
 //

 /* sizeof(T) + IsWrite fold at BusRW_DB_CS12
  * macro-monomorphized form. */
 {
  SCU_FromSH2_BusRW_DB_u16_W1(A, DB, SH2DMAHax);
 }
}

static INLINE void BusRW_DB_CS12_u32_W0(const uint32_t A, uint32_t* DB, int32_t* SH2DMAHax)
{

 //
 // CS1 and CS2: SCU
 //
 *DB = 0;

 /* sizeof(T) + IsWrite fold at BusRW_DB_CS12
  * macro-monomorphized form. */
 {
  SCU_FromSH2_BusRW_DB_u32_W0(A, DB, SH2DMAHax);
 }
}

static INLINE void BusRW_DB_CS12_u32_W1(const uint32_t A, uint32_t* DB, int32_t* SH2DMAHax)
{

 //
 // CS1 and CS2: SCU
 //

 /* sizeof(T) + IsWrite fold at BusRW_DB_CS12
  * macro-monomorphized form. */
 {
  SCU_FromSH2_BusRW_DB_u32_W1(A, DB, SH2DMAHax);
 }
}

static INLINE void BusRW_DB_CS3_u8_W0(const uint32_t A, uint32_t* DB)
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

static INLINE void BusRW_DB_CS3_u8_W1(const uint32_t A, uint32_t* DB)
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

static INLINE void BusRW_DB_CS3_u16_W0(const uint32_t A, uint32_t* DB)
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

static INLINE void BusRW_DB_CS3_u16_W1(const uint32_t A, uint32_t* DB)
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

static INLINE void BusRW_DB_CS3_u32_W0(const uint32_t A, uint32_t* DB)
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

static INLINE void BusRW_DB_CS3_u32_W1(const uint32_t A, uint32_t* DB)
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

//
//
//

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
#include "../state.h"
#include "ss_init.h"     /* events[], next_event_ts, InitEvents */
#include "../../input.h"   /* input_StateAction */
#include <streams/file_stream.h>
#include <libretro.h>

int32_t SH7095_M_DMA_Update(int32_t et) { return SH7095_DMA_Update(&CPU[0], et); }
int32_t SH7095_S_DMA_Update(int32_t et) { return SH7095_DMA_Update(&CPU[1], et); }

/* ForceEventUpdates stays in ss.c -- the first loop dispatches into
 * SH7095_ForceInternalEventUpdates(&CPU[c]), which is an SH7095 class method
 * and not yet a C-callable wrapper.  After the SH7095 class -> struct
 * conversion the function will move into ss.c proper.
 *
 * Called from RunLoop.  Touches the
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
    /* master Step dispatch.  RunLoop is templated on
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
    /* master Step dispatch.  RunLoop is templated on
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

/* SH7095_M_DMA_Update / SH7095_S_DMA_Update are kept as 1-arg
 * forwarders because the SH_DMA_EVENT_HANDLER_BODY macro at line
 * ~1335 substitutes them as UPDATE_FN(et) and requires a 1-arg
 * (sscpu_timestamp_t) signature.  All the other SH7095_{M,S}_*
 * 1-line wrappers (Init/SetMD5/TruePowerOn/Reset/AdjustTS/State
 * Action/PostStateLoad) that used to live here were inlined into
 * their call sites in this TU when the cross-TU SH7095 conversion
 * to C struct retired the need for C-linkage shims. */

//
//
//

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
   0,    /* MasterClock     -- set by InitCommon when region is known */
   320,  /* nominal_width   -- overwritten by VDP2REND_Init on first geometry update */
   0.0f, /* mouse_scale_x   -- ditto */
   0.0f, /* mouse_offs_x    -- ditto */
   0.0f, /* mouse_offs_y    -- ditto */
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
 * because ss.c's RunLoop template body calls it.  Keeping it INLINE
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
 * per-frame Emulate() loop + MidSync helper
 * =================================================================== */

/* Externs into ss.c -- promoted to TU-external in phase 7d. */
extern bool          NeedEmuICache;
extern int           ActiveCartType;
extern int64_t       UpdateInputLastBigTS;

/* Externs for the libretro front-end / game-info structs. */
extern MDFNGI        EmulatedSS;
extern uint32_t      IBufferCount;

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

 /* ST-V games don't have a Saturn reset button -- they have their own
  * service/test/coin switches via the IOGA.  STVIO_TransformInput
  * masks out the reset-button bit from DPtr[12] before SMPC sees it,
  * so an accidental "Reset" press on the frontend doesn't actually
  * issue a soft reset in ST-V mode.  Runs ahead of SMPC's transform. */
 if(ActiveCartType == CART_STV)
   STVIO_TransformInput();

 SMPC_TransformInput();

 elapsed_time = (((int64_t)timestamp * cur_clock_div * 1000 * 1000) - UpdateInputLastBigTS) / (EmulatedSS.MasterClock / ((int64_t)1 << 32));

 UpdateInputLastBigTS += (int64_t)elapsed_time * (EmulatedSS.MasterClock / ((int64_t)1 << 32));

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
 CART_SetCPUClock(EmulatedSS.MasterClock / ((int64_t)1 << 32), cur_clock_div);
 espec->SoundBufSize = 0;
 espec->MasterCycles = 0;

 if (NeedEmuICache)
  end_ts = RunLoop_ICache(espec);
 else
  end_ts = RunLoop_NoICache(espec);
 assert(end_ts >= 0);

 ForceEventUpdates(end_ts);

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

 /* AdjustTS(-end_ts) for both SH7095 CPU instances. */
 SH7095_AdjustTS(&CPU[0], -end_ts);
 SH7095_AdjustTS(&CPU[1], -end_ts);
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
  * pure flags here, and let libretro.c flush them from retro_run --
  * outside Emulate, with awareness of RETRO_ENVIRONMENT_GET_AUDIO_VIDEO_ENABLE
  * so run-ahead simulation frames don't trigger writes. The frontend
  * can also manage Backup RAM directly via RETRO_MEMORY_SAVE_RAM. */
 if(CART_GetClearNVDirty())
  CartNV_Dirty = true;
}

/* ===================================================================
 * InitCommon / SS_Reset / Cleanup / CloseGame
 *
 * These are the boot-time orchestration entry points the libretro
 * front-end calls (InitCommon from retro_load_game, SS_Reset from
 * retro_reset, CloseGame from retro_unload_game).  Each reaches
 * into the SH7095 CPU[2] instances via the SH7095_{M,S}_*
 * dispatch wrappers (Init/SetMD5/TruePowerOn/Reset).
 * =================================================================== */

/* Externs into ss.c (TU-external definitions live there). */
extern uint8_t       WorkRAM[];
extern uint16_t*     WorkRAML;
extern uint16_t*     WorkRAMH;
extern uint16_t      BIOSROM[];   /* 524288 / sizeof(uint16_t) entries */
extern uint32_t      SH7095_DB;
extern uint32_t      ss_horrible_hacks;
extern bool          is_pal;
extern sha256_digest BIOS_SHA256;

/* Defined in libretro.c (no header to include for this -- ss.c and
 * settings.c each redeclare it locally; match that pattern). */
extern char retro_base_directory[4096];

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
      SH7095_ConstructAll();
   SH7095_Init(&CPU[0], (cpucache_emumode == CPUCACHE_EMUMODE_FULL), (cpucache_emumode == CPUCACHE_EMUMODE_DATA_CB));
   SH7095_Init(&CPU[1], (cpucache_emumode == CPUCACHE_EMUMODE_FULL), (cpucache_emumode == CPUCACHE_EMUMODE_DATA_CB));
   SH7095_SetMD5(&CPU[0], false);
   SH7095_SetMD5(&CPU[1], true);

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

   /* Wire WorkRAM into the mempatcher's RAMPtrs table so the periodic-
    * cheat-apply path (MDFNMP_ApplyPeriodicCheats, called from scu.inc
    * on VBlank-In) can actually write the user's cheats.  We pass the
    * uint16_t-aliased pointers cast back to uint8_t* -- the cheat code
    * is aware that WorkRAM is stored in host byte order on LE builds
    * and applies the matching A^1 byte-address swizzle there.
    *
    * Both regions cover BANK_SIZE_BYTES (1MB) at their canonical
    * addresses; the SH-2 mirroring across 0x00200000-0x003FFFFF and
    * 0x06000000-0x07FFFFFF is not replicated in the cheat table.
    * Cheat authors use the canonical addresses by convention. */
   MDFNMP_AddRAM(WORKRAM_BANK_SIZE_BYTES, 0x00200000, (uint8_t *)WorkRAML);
   MDFNMP_AddRAM(WORKRAM_BANK_SIZE_BYTES, 0x06000000, (uint8_t *)WorkRAMH);

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
         /* bios_loaded gates the file-read path below: when set by the
          * stvbios.zip fallback, BIOSROM has already been populated and
          * the filestream_read / filestream_close block must be
          * skipped.  Both paths converge on the same SHA256 +
          * byte-swap finalisation at the bottom. */
         bool bios_loaded = false;

         snprintf(bios_path, sizeof(bios_path), "%s" RETRO_SLASH "%s", retro_base_directory, bios_filename);

         BIOSFile = filestream_open(bios_path,
               RETRO_VFS_FILE_ACCESS_READ,
               RETRO_VFS_FILE_ACCESS_HINT_NONE);

         if(!BIOSFile)
         {
            /* ST-V parent-set fallback: when the bare BIOS file isn't
             * present, try retro_base_directory/stvbios.zip and extract
             * the region-specific entry directly into BIOSROM[].  This
             * matches the MAME-style distribution convention -- ST-V
             * BIOS dumps ship as a parent set (stvbios.zip) containing
             * all region variants, and users typically drop the zip
             * into their system dir as-is.
             *
             * Restricted to is_stv -- consumer Saturn BIOSes don't have
             * an equivalent parent-set convention; the user expects
             * sega_101.bin / mpr-17933.bin as bare files. */
            if(is_stv)
            {
               char zip_path[4096];
               zip_archive za;
               snprintf(zip_path, sizeof(zip_path),
                     "%s" RETRO_SLASH "stvbios.zip", retro_base_directory);
               if(zip_open(&za, zip_path))
               {
                  const struct zip_entry *ze = zip_find(&za, bios_filename);
                  /* Accept either size the file-read path accepts:
                   *   131072 -- bare ST-V BIOS chip dump
                   *   524288 -- MAME parent-set / Saturn-style dump
                   * (the MAME stvbios.zip distributed in the wild
                   * uses the 524288 form; verified against a real
                   * set with all 13 entries at 524288 bytes).
                   * The byte-swap loop's `bios_size/2` upper bound
                   * handles both: ST-V reads 64 KiB of words and
                   * leaves the high 393216 bytes at 0xFF; Saturn-
                   * size reads all 262144 words. */
                  if (ze && (   ze->uncompressed_size == 131072
                             || ze->uncompressed_size == 524288))
                  {
                     /* memset before extract so high half stays 0xFF
                      * when the source is 131072 (ST-V chip dump).
                      * For 524288 the extract fills the whole buffer
                      * and the memset becomes a no-op tail. */
                     memset(BIOSROM, 0xFF, 512 * 1024);
                     if(zip_extract(&za, ze, (uint8_t*)BIOSROM))
                     {
                        bios_size   = ze->uncompressed_size;
                        bios_loaded = true;
                        log_cb(RETRO_LOG_INFO,
                              "ST-V BIOS loaded from \"%s\" entry \"%s\" (%u bytes).\n",
                              zip_path, bios_filename, ze->uncompressed_size);
                     }
                     else
                     {
                        log_cb(RETRO_LOG_ERROR,
                              "ST-V BIOS: extract of \"%s\" from \"%s\" failed (CRC mismatch?).\n",
                              bios_filename, zip_path);
                     }
                  }
                  else if(ze)
                  {
                     log_cb(RETRO_LOG_ERROR,
                           "ST-V BIOS: \"%s\" inside \"%s\" is %u bytes, expected 131072 or 524288.\n",
                           bios_filename, zip_path, ze->uncompressed_size);
                  }
                  zip_close(&za);
               }
            }
            if(!bios_loaded)
            {
               log_cb(RETRO_LOG_ERROR, "Cannot open BIOS file \"%s\".\n", bios_path);
               Cleanup();
               return false;
            }
         }

         if(!bios_loaded)
         {
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
         }

         BIOS_SHA256 = sha256(BIOSROM, 512 * 1024);

         /* swap endian-ness.
          *
          * Two BIOS storage formats:
          *  - Saturn:  BE-stored 16-bit words      (MDFN_de16msb in upstream)
          *  - ST-V:    LE-stored 16-bit words      (MDFN_de16lsb in upstream),
          *             which is equivalent to MAME's ROM_LOAD16_WORD_SWAP --
          *             bytes within each 16-bit word are in the OPPOSITE
          *             byte-order from Saturn.
          *
          * Therefore on a little-endian host we byte-swap Saturn BIOS but
          * leave ST-V BIOS as-is; on a big-endian host the polarity flips. */
         {
            const bool stv = (ActiveCartType == CART_STV);
            for(bw = 0; bw < (unsigned)(bios_size / 2); bw++)
            {
#ifdef MSB_FIRST
               /* BE host: swap iff ST-V (LE-on-disk -> BE host) */
               if(stv)
                  BIOSROM[bw] = (uint16_t)((BIOSROM[bw] << 8) | (BIOSROM[bw] >> 8));
#else
               /* LE host: swap iff Saturn (BE-on-disk -> LE host) */
               if(!stv)
                  BIOSROM[bw] = (uint16_t)((BIOSROM[bw] << 8) | (BIOSROM[bw] >> 8));
#endif
            }
         }
      }

      /* Q32.32 master clock.  MDFN_MASTERCLOCK_FIXED() routes through a
       * host double ((double)n * 2^32); for the Saturn's integer MasterClock
       * (n < 2^31) that product is exact -- scaling by a power of two costs
       * no mantissa bits -- so this integer shift is bit-identical while
       * keeping the emulation timing path free of host floating point. */
      EmulatedSS.MasterClock = (int64_t)MasterClock << 32;

      SCU_Init();
      SMPC_Init(smpc_area, MasterClock, is_stv);
      VDP1_Init();
      VDP2_Init(PAL, vdp2_affinity);
      VDP2_SetGetVideoParams(&EmulatedSS, true, sls, sle, true, DoHBlend);
      CDB_Init();
      SOUND_Init(is_stv);

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
 * retro_reset and from InitCommon's final step.  TruePowerOn fires
 * on both CPUs only when powering_up; the warm-reset path runs
 * SH7095_Reset on the master only, matching the Saturn hardware
 * behaviour where SMPC re-enables the slave on demand rather than
 * the slave being reset alongside the master. */
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
  SH7095_TruePowerOn(&CPU[0]);
  SH7095_TruePowerOn(&CPU[1]);
 }

 SCU_Reset(powering_up);
 SH7095_Reset(&CPU[0], powering_up, false);

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
 SS_SaveBackupRAM();
 SS_SaveCartNV();
 SS_SaveRTC();

 Cleanup();
}

/* log_cb is declared in mednafen/git.h (formerly a header) and
 * defined as a plain C function pointer; redeclare here so we don't
 * need to drag git.h's <algorithm> / <vector> chain into this TU. */
extern retro_log_printf_t log_cb;

/* "BackUpRam Format" ASCII -- the magic prefix every Saturn backup
 * memory region starts with.  Used both by InitCommon in ss.c (to
 * stamp a freshly-zeroed BackupRAM) and by SS_LoadBackupRAM below
 * (to restore the stamp after a short/failed read). */
const uint8_t BRAM_Init_Data[0x10] = {
 0x42, 0x61, 0x63, 0x6b, 0x55, 0x70, 0x52, 0x61,
 0x6d, 0x20, 0x46, 0x6f, 0x72, 0x6d, 0x61, 0x74
};

void SS_SaveBackupRAM(void)
{
 char fpath[4096];
 cdstream brs;

 /* Libretro save mode: the frontend persists BackupRAM as .srm, so the
  * core must not also write .bkr. This single guard covers every writer
  * -- the periodic flush (via SS_FlushBackupRAM) and the CloseGame
  * save-on-exit -- keeping .srm the sole source of truth in this mode. */
 if (!use_mednafen_save_method)
  return;

 if(!cdstream_open_write(&brs, MDFN_MakeFName(fpath, sizeof(fpath), MDFNMKF_SAV, 0, "bkr")))
  return;

 /* Pre-fix the write was unchecked; a partial 32 KiB write
  * (disk full, quota exceeded, network share flake) would leave
  * a truncated .bkr file the user thinks succeeded.  Subsequent
  * SS_LoadBackupRAM detects this via the size mismatch and
  * falls back to BRAM_Init_Data (the unformatted-cart pattern),
  * but at that point the user's saves are already gone -- the
  * truncated file overwrote the previous good one.  Log so the
  * user sees there's a problem before they trust the new save. */
 if(cdstream_write(&brs, BackupRAM, sizeof(BackupRAM)) != sizeof(BackupRAM))
  log_cb(RETRO_LOG_ERROR, "BackupRAM save (\"%s\"): short write (disk full or I/O error).\n", fpath);

 cdstream_close(&brs);
}

/* Input-routing wrapper -- mirrors upstream Mednafen's TU-static
 * SetInput() in ss.cpp (the one the frontend calls instead of
 * SMPC_SetInput directly).  Required because for ST-V games the
 * input flow is fundamentally different:
 *
 *   - SMPC ports 0/1 are owned by the STVIO SMPC-port shim (installed
 *     by InitCommon via SMPC_SetInput(sp, "extern", STVIO_GetSMPCDevice(sp))).
 *     The shim handles AK93C45 EEPROM access and 68K sound-CPU control
 *     forwarding.  Overwriting them with SMPC_SetInput(sp, "gamepad",
 *     ...) -- as the libretro fork's input.c does in input_init --
 *     breaks both: EEPROM reads return garbage (game can't load its
 *     settings and may loop at boot), and the sound CPU stays held in
 *     reset (no audio output at all).
 *
 *   - Player input bytes for ST-V games must flow into STVIO via DPtr[]
 *     (set by STVIO_SetInput), not into SMPC's gamepad IODevice via
 *     VirtualPortsDPtr.  STVIO_UpdateInput reads DPtr to build the
 *     ST-V IOGA register state; the gamepad IODevice's "buttons"
 *     field is never read by ST-V games (SMPC's gamepad output goes
 *     to the (non-existent) Saturn-side SMPC ports, not the ST-V cab).
 *
 *   - Misc-input (port 12, the reset/test/service/pause byte) must
 *     be wired in BOTH places: STVIO reads test/service/pause via
 *     DPtr[12]; SMPC reads reset-button via MiscInputPtr.
 *
 * Upstream's wrapper encodes all of this; copy it verbatim with the
 * extern int ActiveCartType picked up from this TU.  Callers replace
 * direct SMPC_SetInput() calls with SS_SetInput() to get the right
 * routing automatically. */
void SS_SetInput(unsigned port, const char* type, uint8_t* ptr)
{
 if(ActiveCartType == CART_STV)
 {
  STVIO_SetInput(port, type, ptr);
  if(port < 12)
   return;
 }
 SMPC_SetInput(port, type, ptr);
}

void SS_LoadBackupRAM(void)
{
 char fpath[4096];
 RFILE *brs;

 /* Libretro save mode: the frontend owns the save file (.srm) and
  * fills BackupRAM via RETRO_MEMORY_SAVE_RAM *after* retro_load_game
  * returns. Reading .bkr here is pointless when an .srm exists (the
  * frontend's copy lands on top of it) and actively wrong when no
  * .srm exists (the stale .bkr would silently win, making the two
  * modes inconsistent). Skip the read and leave the freshly-formatted
  * BRAM_Init_Data pattern InitCommon already stamped. Migrating a
  * legacy save is a one-time manual rename of .bkr -> .srm. */
 if (!use_mednafen_save_method)
    return;

 brs = filestream_open(MDFN_MakeFName(fpath, sizeof(fpath), MDFNMKF_SAV, 0, "bkr"),
       RETRO_VFS_FILE_ACCESS_READ,
       RETRO_VFS_FILE_ACCESS_HINT_NONE);

 if (!brs)
    return;

 /* Short / failed read would leave BackupRAM holding a mix of
  * save-file bytes in the head and the BRAM_Init_Data pattern
  * (installed by InitCommon a few lines back) in the tail.  The
  * game may then write that hybrid back to disk as a "save",
  * propagating the corruption forward.  On a short read, restore
  * the fresh-format BRAM pattern so the game sees an unformatted
  * BackupRAM and either reformats it cleanly or treats it as a
  * missing save -- both well-defined behaviours. */
 if(filestream_read(brs, BackupRAM, sizeof(BackupRAM)) != (int64_t)sizeof(BackupRAM))
 {
    unsigned i;
    log_cb(RETRO_LOG_WARN, "Backup RAM save file at \"%s\" is short or unreadable; reverting to unformatted BRAM.\n", fpath);
    memset(BackupRAM, 0x00, sizeof(BackupRAM));
    for(i = 0; i < 0x40; i++)
       BackupRAM[i] = BRAM_Init_Data[i & 0x0F];
 }
 filestream_close(brs);
}

void SS_LoadCartNV(void)
{
   uint64_t i;
   RFILE *nvs      = NULL;
   const char* ext = NULL;
   void* nv_ptr    = NULL;
   bool nv16       = false;
   uint64_t nv_size  = 0;
   char fpath[4096];

   /* ST-V's NV storage isn't a contiguous buffer reachable through
    * CART_GetNVInfo (the data lives behind the AK93C45 EEPROM
    * abstraction in stvio.c, accessed via Peek/Poke).  Dispatch
    * to STVIO_LoadNV directly; it walks the EEPROM at the right
    * granularity using a cdstream as transport. */
   if(ActiveCartType == CART_STV)
   {
      cdstream s;
      if(cdstream_open(&s,
            MDFN_MakeFName(fpath, sizeof(fpath), MDFNMKF_CART, 0, "stveep")))
      {
         STVIO_LoadNV(&s);
         cdstream_close(&s);
      }
      return;
   }

   CART_GetNVInfo(&ext, &nv_ptr, &nv16, &nv_size);

   if (!ext)
      return;

   nvs = filestream_open(
         MDFN_MakeFName(fpath, sizeof(fpath), MDFNMKF_CART, 0, ext),
         RETRO_VFS_FILE_ACCESS_READ,
         RETRO_VFS_FILE_ACCESS_HINT_NONE);

   if (!nvs)
      return;

   /* Short / failed read would leave nv_ptr holding a mix of
    * save-file bytes in the head and the cart-specific
    * post-CART_Init state in the tail (different for each cart
    * type).  Same hazard as SS_LoadBackupRAM: the game may write
    * that hybrid back as a save.  Zero the buffer on short read
    * so the game sees a blank cart and rebuilds save state from
    * scratch.  CART_Reset on the next emulation reset would
    * restore any cart-specific magic the cart needs, but a
    * zeroed buffer is already a defined "fresh" state that
    * every cart driver handles. */
   if(filestream_read(nvs, nv_ptr, nv_size) != (int64_t)nv_size)
   {
      log_cb(RETRO_LOG_WARN, "Cart NV save file at \"%s\" is short or unreadable; reverting to blank cart NV.\n", fpath);
      memset(nv_ptr, 0, nv_size);
      filestream_close(nvs);
      return;
   }
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

void SS_SaveCartNV(void)
{
   const char* ext = NULL;
   void* nv_ptr = NULL;
   bool nv16 = false;
   uint64_t nv_size = 0;
   char fpath[4096];

   /* See SS_LoadCartNV: ST-V's EEPROM lives behind the AK93C45
    * abstraction and isn't reachable as a contiguous CART_GetNVInfo
    * buffer.  STVIO_SaveNV walks the EEPROM in the right
    * granularity using a cdstream as transport. */
   if(ActiveCartType == CART_STV)
   {
      cdstream s;
      if(cdstream_open_write(&s,
            MDFN_MakeFName(fpath, sizeof(fpath), MDFNMKF_CART, 0, "stveep")))
      {
         /* STVIO_SaveNV returns false on short write. */
         if(!STVIO_SaveNV(&s))
            log_cb(RETRO_LOG_ERROR, "ST-V EEPROM save (\"%s\"): short write (disk full or I/O error).\n", fpath);
         cdstream_close(&s);
      }
      return;
   }

   CART_GetNVInfo(&ext, &nv_ptr, &nv16, &nv_size);

   if(ext)
   {
      cdstream nvs;
      bool     ok = true;
      if(!cdstream_open_write(&nvs, MDFN_MakeFName(fpath, sizeof(fpath), MDFNMKF_CART, 0, ext)))
         return;

      if(nv16)
      {
         uint64_t i;
         /* nv_ptr is host-endian uint16s; cdstream_write_be_u16
          * takes host-endian and emits big-endian bytes.  Just a
          * misalignment-safe 2-byte load.  Break out on the first
          * short write -- continuing would just produce more 0s
          * in the truncated file. */
         for(i = 0; i < nv_size; i += 2)
         {
            uint16_t v;
            memcpy(&v, (uint8_t*)nv_ptr + i, 2);
            if(cdstream_write_be_u16(&nvs, v) != 2)
            {
               ok = false;
               break;
            }
         }
      }
      else
         ok = (cdstream_write(&nvs, nv_ptr, nv_size) == nv_size);

      cdstream_close(&nvs);

      if(!ok)
         log_cb(RETRO_LOG_ERROR, "Cart NV save (\"%s\"): short write (disk full or I/O error).\n", fpath);
   }
}

void SS_SaveRTC(void)
{
   char fpath[4096];
   cdstream sds;
   if(!cdstream_open_write(&sds, MDFN_MakeFName(fpath, sizeof(fpath), MDFNMKF_SAV, 0, "smpc")))
      return;

   /* SMPC_SaveNV returns false on short write of RTC.Valid / .raw / SaveMem. */
   if(!SMPC_SaveNV(&sds))
      log_cb(RETRO_LOG_ERROR, "SMPC RTC save (\"%s\"): short write (disk full or I/O error).\n", fpath);

   cdstream_close(&sds);
}

void SS_LoadRTC(void)
{
   char fpath[4096];
   cdstream sds;
   if(!cdstream_open(&sds, MDFN_MakeFName(fpath, sizeof(fpath), MDFNMKF_SAV, 0, "smpc")))
      return;

   SMPC_LoadNV(&sds);

   cdstream_close(&sds);
}

/*
 * Public flush entry points for libretro.c.
 *
 * These wrap the (TU-local) SS_SaveBackupRAM / SS_SaveCartNV functions so
 * the file I/O can happen from outside Emulate(), after retro_run() has
 * emulated a frame and before it returns. Calling these from outside
 * Emulate() is what makes run-ahead / rewind / netplay friendly: those
 * features re-run Emulate() multiple times per real frame, and the
 * previous design issued a disk write on every re-run when the BRAM/cart
 * dirty timer expired.
 *
 * Both return true on success and false on failure (so the caller can
 * schedule a retry); they're declared in ss.h as part of the public ABI.
 */
bool SS_FlushBackupRAM(void)
{
 SS_SaveBackupRAM();
 return true;
}

bool SS_FlushCartNV(void)
{
 SS_SaveCartNV();
 return true;
}

/* ===================================================================
 * emulator-state save/load
 * ===================================================================
 *
 * EventsPacker is the SH-2 event ring's save-state packer.  Was a
 * C++ struct with Save / Restore member methods + a size_t-scoped
 * enum-as-named-constant pair; rewritten as a plain C struct with
 * two free functions taking EventsPacker*.  The enum's two named
 * constants become file-scope #defines so they're usable in the
 * sizing array bounds inside the struct.
 *
 * LibRetro_StateAction is the libretro-level state-action entry
 * point.  Reaches into ss.c's globals (NeedEmuICache, BIOS_SHA256,
 * ActiveCartType, BackupRAM_StateHelper, WorkRAML/H, SH7095_DB,
 * UpdateInputLastBigTS) and dispatches into the SH7095 cores
 * through the SH7095_{M,S}_{StateAction,PostStateLoad} wrappers.
 */

#define EVENTCOPY_FIRST  (SS_EVENT__SYNFIRST + 1)
#define EVENTCOPY_BOUND  SS_EVENT__SYNLAST

typedef struct {
 int32_t event_times[EVENTCOPY_BOUND - EVENTCOPY_FIRST];
 uint8_t event_order[EVENTCOPY_BOUND - EVENTCOPY_FIRST];
} EventsPacker;

static INLINE void EventsPacker_Save(EventsPacker* ep)
{
 size_t i, j;
 const size_t n = EVENTCOPY_BOUND - EVENTCOPY_FIRST;

 for(i = 0; i < n; i++)
 {
  ep->event_times[i] = events[EVENTCOPY_FIRST + i].event_time;
  ep->event_order[i] = (uint8_t)(EVENTCOPY_FIRST + i);
 }

 /* event_order is the schedule order Restore() validates; equal
  * times tie-break by index.  Was std::stable_sort with a lambda
  * comparator.  Insertion sort is stable, so equal event_times
  * keep their original index order -- the same tie-break
  * std::stable_sort gave. */
 for(i = 1; i < n; i++)
 {
  const uint8_t key = ep->event_order[i];
  j = i;
  while(j > 0 && events[ep->event_order[j - 1]].event_time > events[key].event_time)
  {
   ep->event_order[j] = ep->event_order[j - 1];
   j--;
  }
  ep->event_order[j] = key;
 }
}

static INLINE bool EventsPacker_Restore(EventsPacker* ep, const unsigned state_version)
{
 bool used[SS_EVENT__COUNT] = { 0 };
 size_t i;

 for(i = 0; i < (size_t)(EVENTCOPY_BOUND - EVENTCOPY_FIRST); i++)
 {
  int32_t et = ep->event_times[i];
  uint8_t eo = ep->event_order[i];

  if(state_version < 0x00102600 && et >= 0x40000000)
  {
   et = SS_EVENT_DISABLED_TS;
  }

  if(eo < EVENTCOPY_FIRST || eo >= EVENTCOPY_BOUND)
   return false;

  if(used[eo])
   return false;

  used[eo] = true;

  if(et < 0)
   return false;

  events[EVENTCOPY_FIRST + i].event_time = et;
 }

 /* Reject malformed save states whose event_order isn't non-decreasing. */
 for(i = 1; i < (size_t)(EVENTCOPY_BOUND - EVENTCOPY_FIRST); i++)
 {
  if(events[ep->event_order[i]].event_time < events[ep->event_order[i - 1]].event_time)
   return false;
 }

 return true;
}

/* Externs into ss.c -- the ss.c globals these touch are
 * promoted to TU-external in phase 7d.  Declared here rather
 * than in ss_state.h because they're internal to the save-state
 * machinery; nobody else needs them. */
extern bool          NeedEmuICache;
extern sha256_digest BIOS_SHA256;
extern int           ActiveCartType;
extern uint8_t       BackupRAM_StateHelper[32768];
extern uint16_t*     WorkRAML;
extern uint16_t*     WorkRAMH;
extern uint32_t      SH7095_DB;
extern int64_t       UpdateInputLastBigTS;
/* SH7095_mem_timestamp + SH7095_BusLock come via ss_init.h.  The
 * SH7095 state-action / post-state-load is invoked directly via
 * sh7095.h's exports (SH7095_StateAction / SH7095_PostStateLoad)
 * with the appropriate CPU[] entry. */
extern int32_t       SH7095_mem_timestamp;
extern uint32_t      SH7095_BusLock;

int LibRetro_StateAction(StateMem* sm, const unsigned load)
{
   bool RecordedNeedEmuICache;
   EventsPacker ep;
   SFORMAT StateRegs[14];   /* sized below; using a fixed buffer keeps
                             * the C version's table layout matching
                             * the C++ original's brace-initialiser. */
   unsigned sridx = 0;

   {
      sha256_digest sr_dig = BIOS_SHA256;
      int cart_type = ActiveCartType;

      SFORMAT SRDStateRegs[] =
      {
         SFPTR8( sr_dig.b, sizeof(sr_dig.b) ),
         SFVAR(cart_type),
         SFEND
      };

      if (MDFNSS_StateAction( sm, load, false, SRDStateRegs, "BIOS_HASH", true ) == 0)
         return 0;

      if ( load )
      {
         if ( !sha256_digest_eq(&sr_dig, &BIOS_SHA256) ) {
           log_cb( RETRO_LOG_WARN, "BIOS hash mismatch(save state created under a different BIOS)!\n" );
           return 0;
         }
         if ( cart_type != ActiveCartType ) {
           log_cb( RETRO_LOG_WARN, "Cart type mismatch(save state created with a different cart)!\n" );
           return 0;
         }
      }
   }

   RecordedNeedEmuICache = load ? false : NeedEmuICache;
   EventsPacker_Save(&ep);

   /* SFORMAT brace-initialiser table -- one slot per save-state
    * variable.  Indices laid out so the resulting table content is
    * the same shape and order as the C++ original's. */
   {
      SFORMAT t0  = SFVAR(UpdateInputLastBigTS);            StateRegs[sridx++] = t0;
      SFORMAT t1  = SFVAR(next_event_ts);                   StateRegs[sridx++] = t1;
      {
         SFORMAT t2 = SFPTR32N(ep.event_times, sizeof(ep.event_times) / sizeof(ep.event_times[0]), "event_times");
         StateRegs[sridx++] = t2;
      }
      {
         SFORMAT t3 = SFPTR8N(ep.event_order, sizeof(ep.event_order) / sizeof(ep.event_order[0]), "event_order");
         StateRegs[sridx++] = t3;
      }
      {
         SFORMAT t4 = SFVAR(SH7095_mem_timestamp); StateRegs[sridx++] = t4;
      }
      {
         SFORMAT t5 = SFVAR(SH7095_BusLock); StateRegs[sridx++] = t5;
      }
      {
         SFORMAT t6 = SFVAR(SH7095_DB);  StateRegs[sridx++] = t6;
      }
      {
         SFORMAT t7 = SFPTR16(WorkRAML, WORKRAM_BANK_SIZE_BYTES / sizeof(uint16_t)); StateRegs[sridx++] = t7;
      }
      {
         SFORMAT t8 = SFPTR16(WorkRAMH, WORKRAM_BANK_SIZE_BYTES / sizeof(uint16_t)); StateRegs[sridx++] = t8;
      }
      {
         SFORMAT t9 = SFPTR8(BackupRAM, sizeof(BackupRAM) / sizeof(BackupRAM[0])); StateRegs[sridx++] = t9;
      }
      {
         SFORMAT t10 = SFVAR(RecordedNeedEmuICache); StateRegs[sridx++] = t10;
      }
      {
         SFORMAT te = SFEND; StateRegs[sridx++] = te;
      }
   }

   SH7095_StateAction(&CPU[0], sm, load, false, "SH2-M");
   SH7095_StateAction(&CPU[1], sm, load, false, "SH2-S");
   SCU_StateAction(sm, load, false);

   /* Restore the per-port libretro device type *before*
    * SMPC_StateAction so the upcoming IODevice_*_StateAction
    * calls find their named sections in the state buffer for the
    * device the state was actually saved under, rather than the
    * (possibly different) device the live core currently has on
    * each port.  This is the residual cross-device piece of
    * issue #21 that the CORE_VARIABLE_SIZE quirk alone couldn't
    * address.  Optional section: states written by core versions
    * pre-dating this fix are loaded unchanged. */
   input_StateActionDevices( sm, load, false );

   SMPC_StateAction(sm, load, false);

   CDB_StateAction(sm, load, false);
   VDP1_StateAction(sm, load, false);
   VDP2_StateAction(sm, load, false);

   SOUND_StateAction(sm, load, false);
   CART_StateAction(sm, load, false);

   if(load)
      memcpy(BackupRAM_StateHelper, BackupRAM, sizeof(BackupRAM));

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

      if ( !EventsPacker_Restore(&ep, load) )
      {
         log_cb( RETRO_LOG_WARN, "Bad state events data.\n" );
         InitEvents();
      }

      SH7095_PostStateLoad(&CPU[0], load, RecordedNeedEmuICache, NeedEmuICache);
      SH7095_PostStateLoad(&CPU[1], load, RecordedNeedEmuICache, NeedEmuICache);
   }

   return 1;
}
