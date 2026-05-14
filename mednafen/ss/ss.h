/******************************************************************************/
/* Mednafen Sega Saturn Emulation Module                                      */
/******************************************************************************/
/* ss.h:
**  Copyright (C) 2015-2020 Mednafen Team
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

#ifndef __MDFN_SS_SS_H
#define __MDFN_SS_SS_H

#include "../mednafen-types.h"
#include "../math_ops.h"
#include <stdint.h>
#include <stdarg.h>

 enum
 {
  SS_DBG_ERROR     = (1U <<  0),
  SS_DBG_WARNING   = (1U <<  1),

  SS_DBG_M68K 	   = (1U <<  2),

  SS_DBG_SH2  	   = (1U <<  3),
  SS_DBG_SH2_REGW  = (1U <<  4),
  SS_DBG_SH2_CACHE = (1U <<  5),
  SS_DBG_SH2_CACHE_NOISY = (1U << 6),
  SS_DBG_SH2_EXCEPT= (1U <<  7),
  SS_DBG_SH2_DMARACE=(1U <<  8),

  SS_DBG_SCU  	   = (1U <<  9),
  SS_DBG_SCU_REGW  = (1U << 10),
  SS_DBG_SCU_INT   = (1U << 11),
  //SS_DBG_SCU_TIMER = (1U << 12),
  SS_DBG_SCU_DSP   = (1U << 13),

  SS_DBG_SMPC 	   = (1U << 14),
  SS_DBG_SMPC_REGW = (1U << 15),

  SS_DBG_BIOS	   = (1U << 16),

  SS_DBG_CDB  	   = (1U << 17),
  SS_DBG_CDB_REGW  = (1U << 18),

  SS_DBG_VDP1 	   = (1U << 19),
  SS_DBG_VDP1_REGW = (1U << 20),
  SS_DBG_VDP1_VRAMW= (1U << 21),
  SS_DBG_VDP1_FBW  = (1U << 22),
  SS_DBG_VDP1_RACE = (1U << 23),

  SS_DBG_VDP2 	   = (1U << 24),
  SS_DBG_VDP2_REGW = (1U << 25),

  SS_DBG_SCSP 	   = (1U << 26),
  SS_DBG_SCSP_REGW = (1U << 27),
  SS_DBG_SCSP_MOBUF= (1U << 28),
 };
 enum { ss_dbg_mask = 0 };

#if 1
 enum
 {
  HORRIBLEHACK_NOSH2DMALINE106	 = (1U << 0),
  HORRIBLEHACK_NOSH2DMAPENALTY   = (1U << 1),
  HORRIBLEHACK_VDP1VRAM5000FIX	 = (1U << 2),
  HORRIBLEHACK_VDP1RWDRAWSLOWDOWN= (1U << 3),
  HORRIBLEHACK_VDP1INSTANT	 = (1U << 4),
  /*HORRIBLEHACK_SCUINTDELAY = (1U << 5),*/
 };
 MDFN_HIDE extern uint32_t ss_horrible_hacks;
#endif

 #define WORKRAM_BANK_SIZE_BYTES (1024*1024)

  extern uint8_t WorkRAM[2*WORKRAM_BANK_SIZE_BYTES]; // unified 2MB work ram for linear access.

 // Backup RAM is exposed so libretro.cpp can hand it to the frontend via
 // RETRO_MEMORY_SAVE_RAM. The dirty flags are maintained by the emulation
 // (the BackupRAM_Dirty bit is set on every write to the BRAM region,
 // CartNV_Dirty is set at end-of-frame when CART_GetClearNVDirty returns
 // true), and consumed by libretro.cpp after Emulate() returns. See the
 // long comment in Emulate() in ss.cpp.
 extern uint8_t BackupRAM[32768];
 extern bool BackupRAM_Dirty;
 extern bool CartNV_Dirty;

 // Persist BackupRAM / cart NV to disk. Safe to call from any thread
 // (including outside Emulate()). Return false if the write failed so
 // the caller can schedule a retry.
 bool SS_FlushBackupRAM(void);
 bool SS_FlushCartNV(void);


 typedef int32_t sscpu_timestamp_t;

 class SH7095;

 MDFN_HIDE extern SH7095 CPU[2];	// for smpc.cpp

 MDFN_HIDE extern int32_t SH7095_mem_timestamp;

 void SS_RequestMLExit(void);
 void SS_RequestEHLExit(void);

 enum
 {
  SS_EVENT__SYNFIRST = 0,

  SS_EVENT_SH2_M_DMA,
  SS_EVENT_SH2_S_DMA,

  SS_EVENT_SCU_DMA,
  SS_EVENT_SCU_DSP,

  SS_EVENT_SMPC,

  SS_EVENT_VDP1,
  SS_EVENT_VDP2,

  SS_EVENT_CDB,

  SS_EVENT_SOUND,

  SS_EVENT_CART,

  SS_EVENT_MIDSYNC,
  //
  //
  //
  /* SS_EVENT_SCU_INT, */

  SS_EVENT__SYNLAST,
  SS_EVENT__COUNT,
 };

 // events[] is padded so FindNextEventTS() can min-reduce over whole vectors;
 // padding slots stay at SS_EVENT_DISABLED_TS.
 enum { SS_EVENT__SIMD_COUNT = (SS_EVENT__COUNT + 3) & ~3 };

 typedef sscpu_timestamp_t (*ss_event_handler)(const sscpu_timestamp_t timestamp);

 struct event_list_entry
 {
  sscpu_timestamp_t event_time;
 };

 MDFN_HIDE extern event_list_entry events[SS_EVENT__SIMD_COUNT];

 enum : sscpu_timestamp_t { SS_EVENT_DISABLED_TS = 0x7FFFFFFF };

 void SS_SetEventNT(event_list_entry* e, const sscpu_timestamp_t next_timestamp);

 // Call from init code, or power/reset code, as appropriate.
 // (length is in units of bytes, not 16-bit units)
 //
 // is_writeable is mostly for cheat stuff.
 //
 // Declared with C linkage: the cart subsystem (cart.c and the
 // cart/*.c device files) is C now and calls this across the C/C++
 // boundary. The default argument is a caller-side C++ convenience
 // and is fine on an extern "C" function; the C declarations in the
 // cart files just pass is_writeable explicitly.
 extern "C" void SS_SetPhysMemMap(uint32_t Astart, uint32_t Aend, uint16_t* ptr, uint32_t length, bool is_writeable = false) MDFN_COLD;

 void SS_Reset(bool powering_up) MDFN_COLD;

#endif
