/******************************************************************************/
/* Mednafen Sega Saturn Emulation Module                                      */
/******************************************************************************/
/* ss_state.c - BackupRAM / Cart NV / RTC file I/O for the Saturn core.
**              Phase-7b extraction from ss.cpp: these eight functions
**              are pure-C orchestration (FILE I/O through libretro-common
**              streams + the cdstream wrapper) with no SH-2 / SCU / class
**              dependencies, so they migrated cleanly to a standalone C
**              TU while ss.cpp keeps the actual emulation engine.
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
#include <stddef.h>
#include <string.h>
#include <boolean.h>

#include "../mednafen-types.h"
#include "../state.h"
#include "../hash/sha256.h"
#include "ss.h"
#include "ss_state.h"
#include "ss_init.h"     /* events[], next_event_ts, InitEvents */
#include "smpc.h"
#include "cart.h"
#include "scu.h"
#include "cdb.h"
#include "vdp1.h"
#include "vdp2.h"
#include "sound.h"

#include "../general.h"
#include "../cdstream.h"
#include "../../input.h"   /* input_StateAction */

#include <streams/file_stream.h>
#include <libretro.h>

/* log_cb is the libretro front-end's log printf, declared globally
 * in mednafen/git.h (a C++ header) but defined as a plain C function
 * pointer.  Pulling the C++ git.h chain into this pure-C TU would
 * drag <algorithm> / <vector> with it, so just declare what we use. */
extern retro_log_printf_t log_cb;

/* "BackUpRam Format" ASCII -- the magic prefix every Saturn backup
 * memory region starts with.  Used both by InitCommon in ss.cpp (to
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
 if(!cdstream_open_write(&brs, MDFN_MakeFName(fpath, sizeof(fpath), MDFNMKF_SAV, 0, "bkr")))
  return;

 cdstream_write(&brs, BackupRAM, sizeof(BackupRAM));

 cdstream_close(&brs);
}

void SS_LoadBackupRAM(void)
{
 char fpath[4096];
 RFILE *brs = filestream_open(MDFN_MakeFName(fpath, sizeof(fpath), MDFNMKF_SAV, 0, "bkr"),
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

void SS_BackupBackupRAM(void)
{
 MDFN_BackupSavFile(10, "bkr");
}

void SS_BackupCartNV(void)
{
 const char* ext = NULL;
 void* nv_ptr = NULL;
 bool nv16 = false;
 uint64_t nv_size = 0;

 CART_GetNVInfo(&ext, &nv_ptr, &nv16, &nv_size);

 if(ext)
  MDFN_BackupSavFile(10, ext);
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

void SS_SaveRTC(void)
{
   char fpath[4096];
   cdstream sds;
   if(!cdstream_open_write(&sds, MDFN_MakeFName(fpath, sizeof(fpath), MDFNMKF_SAV, 0, "smpc")))
      return;

   SMPC_SaveNV(&sds);

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
 * Public flush entry points for libretro.cpp.
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
 * Phase-7d: emulator-state save/load
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
 * point.  Reaches into ss.cpp's globals (NeedEmuICache, BIOS_SHA256,
 * ActiveCartType, BackupRAM_StateHelper, WorkRAML/H, SH7095_DB,
 * UpdateInputLastBigTS -- all promoted to TU-external in this
 * phase) and dispatches into the SH7095 cores through the four
 * extern "C" SH7095_{M,S}_{StateAction,PostStateLoad} wrappers
 * added to ss.cpp.  Those wrappers retire once the SH7095 class
 * itself becomes a C struct (later phase).
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

/* Externs into ss.cpp -- the ss.cpp globals these touch are
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
 * SH7095 C++-class state-action / post-state-load entry points are
 * extern "C" wrappers defined in ss.cpp. */
extern int32_t       SH7095_mem_timestamp;
extern uint32_t      SH7095_BusLock;
void SH7095_M_StateAction(StateMem* sm, const unsigned load, const bool data_only, const char* sname) MDFN_COLD;
void SH7095_S_StateAction(StateMem* sm, const unsigned load, const bool data_only, const char* sname) MDFN_COLD;
void SH7095_M_PostStateLoad(const unsigned load, bool prev_NeedEmuICache, bool current_NeedEmuICache);
void SH7095_S_PostStateLoad(const unsigned load, bool prev_NeedEmuICache, bool current_NeedEmuICache);

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

   SH7095_M_StateAction(sm, load, false, "SH2-M");
   SH7095_S_StateAction(sm, load, false, "SH2-S");
   SCU_StateAction(sm, load, false);
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

      SH7095_M_PostStateLoad(load, RecordedNeedEmuICache, NeedEmuICache);
      SH7095_S_PostStateLoad(load, RecordedNeedEmuICache, NeedEmuICache);
   }

   return 1;
}
