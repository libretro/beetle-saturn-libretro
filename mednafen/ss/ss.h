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
/* C inclusion (for future C-converted SS modules) needs the boolean
 * keyword macros; C++ has `bool` as a built-in. */
#include <boolean.h>

/* (The Mednafen-standalone build wraps each SS_DBG()/SS_DBGTI()
 * logging callsite in an `ss_dbg_mask & SS_DBG_*`-style gate fed
 * by the debugger UI's "active log categories" bitmask.  This
 * libretro core does not expose that interface and never produced
 * any callsite for it -- the entire SS_DBG_* enum and the
 * always-zero `ss_dbg_mask` constant have been retired together
 * with the rest of the standalone debugger machinery.) */

#if 1
 enum
 {
  HORRIBLEHACK_NOSH2DMALINE106    = (1U << 0),
  HORRIBLEHACK_NOSH2DMAPENALTY    = (1U << 1),
  HORRIBLEHACK_VDP1VRAM5000FIX    = (1U << 2),
  HORRIBLEHACK_VDP1RWDRAWSLOWDOWN = (1U << 3),
  HORRIBLEHACK_VDP1INSTANT        = (1U << 4)
  /* HORRIBLEHACK_SCUINTDELAY     = (1U << 5), */
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
 // long comment in Emulate() in ss.c.
 extern uint8_t BackupRAM[32768];
 extern bool BackupRAM_Dirty;
 extern bool CartNV_Dirty;

 // Persist BackupRAM / cart NV to disk. Safe to call from any thread
 // (including outside Emulate()). Return false if the write failed so
 // the caller can schedule a retry.
 //
 // These and the other SS-core entry points below (SS_Reset / Emulate /
 // CloseGame / InitCommon / SS_RequestMLExit / SS_RequestEHLExit) are
 // wrapped in extern "C" because the libretro entry-point TU
 // (libretro.c, converted from C++) calls them across the C/C++
 // boundary.  ss.c is still C++; the wrap propagates C linkage
 // from these declarations to the matching definitions in ss.c.
#ifdef __cplusplus
extern "C" {
#endif

 bool SS_FlushBackupRAM(void);
 bool SS_FlushCartNV(void);

#ifdef __cplusplus
}
#endif

 typedef int32_t sscpu_timestamp_t;

#ifdef __cplusplus
 /* SH7095 is a struct; C consumers of this header can't see the
  * type but also have no use for it (the CPU[] array is accessed only
  * from smpc.c and ss.c internals).  Guard so the header parses
  * as C. */
 class SH7095;

 MDFN_HIDE extern SH7095 CPU[2];	// for smpc.c
#endif

 MDFN_HIDE extern int32_t SH7095_mem_timestamp;

#ifdef __cplusplus
extern "C" {
#endif

 void SS_RequestMLExit(void);
 void SS_RequestEHLExit(void);

#ifdef __cplusplus
}
#endif

 /* Saturn event scheduler.  Order is load-bearing -- indexes events[]
  * in ss.c, savestate-visible. */
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
  /* SS_EVENT_SCU_INT, */

  SS_EVENT__SYNLAST,
  SS_EVENT__COUNT
 };

 // events[] is padded so FindNextEventTS() can min-reduce over whole vectors;
 // padding slots stay at SS_EVENT_DISABLED_TS.
 enum { SS_EVENT__SIMD_COUNT = (SS_EVENT__COUNT + 3) & ~3 };

 typedef sscpu_timestamp_t (*ss_event_handler)(const sscpu_timestamp_t timestamp);

 /* event_list_entry: layout-compatible POD shared with ss.c's events[]. */
 typedef struct event_list_entry
 {
  sscpu_timestamp_t event_time;
 } event_list_entry;

 MDFN_HIDE extern event_list_entry events[SS_EVENT__SIMD_COUNT];

 /* Sentinel "no event scheduled" timestamp.  Savestate-visible, so the
  * value is fixed. */
 enum { SS_EVENT_DISABLED_TS = 0x7FFFFFFF };

#ifdef __cplusplus
extern "C" {
#endif

 void SS_SetEventNT(event_list_entry* e, const sscpu_timestamp_t next_timestamp);

#ifdef __cplusplus
}
#endif

 // Call from init code, or power/reset code, as appropriate.
 // (length is in units of bytes, not 16-bit units)
 //
 // is_writeable is mostly for cheat stuff.
 //
 // Declared with C linkage: the cart subsystem (cart.c and the
 // cart/*.c device files) is C now and calls this across the C/C++
 // boundary.  C++ callers get the convenience default argument;
 // C callers must pass is_writeable explicitly (default args are
 // no longer used syntax, so the declaration is split).
#ifdef __cplusplus
extern "C" void SS_SetPhysMemMap(uint32_t Astart, uint32_t Aend, uint16_t* ptr, uint32_t length, bool is_writeable = false) MDFN_COLD;
#else
void SS_SetPhysMemMap(uint32_t Astart, uint32_t Aend, uint16_t* ptr, uint32_t length, bool is_writeable) MDFN_COLD;
#endif

#ifdef __cplusplus
extern "C" {
#endif

 /* Forward-declare struct types used in the InitCommon/Emulate
  * prototypes below.  Both are defined in C-includable headers
  * (db_stv.h for STVGameInfo, emuspec.h for EmulateSpecStruct via
  * git.h's transitive include); the forward decls here let ss.h
  * stay independent of those includes -- TUs that need the full
  * struct layouts pull the appropriate header themselves. */
 struct STVGameInfo;
 struct EmulateSpecStruct;

 void SS_Reset(bool powering_up) MDFN_COLD;

 /* Top-level SS-core entry points called from libretro.c.
  * Defined in ss.c (still C++).  See the longer comment near
  * SS_FlushBackupRAM above for the cross-language ABI rationale.
  * InitCommon's caller-side default args (rom_dir, main_fname, sgi
  * all default to NULL) are dropped here so the declaration is C-
  * parseable; the few libretro.c callsites that relied on defaults
  * pass NULL explicitly. */
 bool InitCommon(const unsigned cpucache_emumode, const unsigned horrible_hacks, const unsigned cart_type, const unsigned smpc_area, const char* rom_dir, const char* main_fname, const struct STVGameInfo* sgi) MDFN_COLD;
 void Emulate(struct EmulateSpecStruct* espec_arg);
 void CloseGame(void) MDFN_COLD;

 /* Defined in ss.c; called from libretro.c's save-state /
  * end-of-game path via the same TU boundary. */
 void MDFN_BackupSavFile(const uint8_t max_backup_count, const char* sav_ext);

#ifdef __cplusplus
}
#endif

#endif
