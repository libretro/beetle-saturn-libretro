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
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#include <mednafen/mednafen-types.h>
#include "ss.h"
#include "ss_state.h"
#include "smpc.h"
#include "cart.h"

#include "../general.h"
#include "../cdstream.h"

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
