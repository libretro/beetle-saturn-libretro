/******************************************************************************/
/* Mednafen Sega Saturn Emulation Module                                      */
/******************************************************************************/
/* cart.c - Expansion cart emulation
**  Copyright (C) 2016-2017 Mednafen Team
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

/* Converted from cart.c. CartInfo was already a plain struct of
   function pointers -- C-style polymorphism -- so there is no class
   hierarchy here; the conversion is mechanical. The two CartInfo
   member functions (CS01_SetRW8W16 / CS2M_SetRW8W16) become free
   functions taking a CartInfo*; the DummyRead / DummyWrite
   template<typename T> stubs, whose T was unused, collapse to single
   functions. */

#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <boolean.h>

#include <streams/file_stream.h>

/* mednafen.h is C++ (it pulls in git.h -> <algorithm>, <vector>, ...).
   cart.c needs only MDFN_GetSettingS and MDFN_MakeFName / the
   MDFNMKF_FIRMWARE enum, which live in the C-clean settings.h and
   general.h; include those directly. */
#include "../settings.h"
#include "../general.h"

#include "cart.h"
#include "cart/backup.h"
#include "cart/cs1ram.h"
#include "cart/bootrom.h"
#include "cart/extram.h"
/* #include "cart/nlmodem.h" */
#include "cart/rom.h"
#include "cart/ar4mp.h"
#include "cart/stv.h"

#include "ss.h"   /* SS_EVENT_DISABLED_TS */

struct CartInfo Cart;

/* Were template<typename T> ... with T unused in the body; now a
   single function each. */
static MDFN_HOT void DummyRead(uint32_t A, uint16_t *DB)
{
   (void)A;
   (void)DB;
}

static MDFN_HOT void DummyWrite(uint32_t A, uint16_t *DB)
{
   (void)A;
   (void)DB;
}

static int32_t DummyUpdate(int32_t timestamp)
{
   (void)timestamp;
   return SS_EVENT_DISABLED_TS;
}

static void DummyAdjustTS(const int32_t delta)
{
   (void)delta;
}

static void DummySetCPUClock(const int32_t master_clock, const int32_t divider)
{
   (void)master_clock;
   (void)divider;
}

static MDFN_COLD void DummyReset(bool powering_up)
{
   (void)powering_up;
}

static MDFN_COLD void DummyStateAction(StateMem *sm, const unsigned load, const bool data_only)
{
   (void)sm;
   (void)load;
   (void)data_only;
}

static MDFN_COLD bool DummyGetClearNVDirty(void)
{
   return false;
}

static MDFN_COLD void DummyGetNVInfo(const char **ext, void **nv_ptr, bool *nv16, uint64_t *nv_size)
{
   *ext     = NULL;
   *nv_ptr  = NULL;
   *nv16    = false;
   *nv_size = 0;
}

static MDFN_COLD void DummyKill(void)
{
}

/* Was CartInfo::CS01_SetRW8W16. The body referenced the file-scope
   global Cart directly (not `this`), so it is preserved verbatim --
   the passed-in CartInfo* `c` is currently unused, kept for the
   free-function signature and for the device files that call this
   with &Cart. */
void CartInfo_CS01_SetRW8W16(struct CartInfo *c, uint32_t Astart, uint32_t Aend,
                             void (*r16)(uint32_t A, uint16_t *DB),
                             void (*w8)(uint32_t A, uint16_t *DB),
                             void (*w16)(uint32_t A, uint16_t *DB))
{
   unsigned i;
   (void)c;

   assert(Astart >= 0x02000000 && Aend <= 0x04FFFFFF);

   assert(!(Astart & ((1U << 20) - 1)));
   assert(!((Aend + 1) & ((1U << 20) - 1)));

   for(i = (Astart - 0x02000000) >> 20; i <= (Aend - 0x02000000) >> 20; i++)
   {
      if(r16) Cart.CS01_RW[i].Read16  = r16;
      if(w8)  Cart.CS01_RW[i].Write8  = w8;
      if(w16) Cart.CS01_RW[i].Write16 = w16;
   }
}

/* Was CartInfo::CS2M_SetRW8W16. Same note as above. */
static void CartInfo_CS2M_SetRW8W16(struct CartInfo *c, uint8_t Ostart, uint8_t Oend,
                             void (*r16)(uint32_t A, uint16_t *DB),
                             void (*w8)(uint32_t A, uint16_t *DB),
                             void (*w16)(uint32_t A, uint16_t *DB))
{
   int i;
   (void)c;

   assert(!(Ostart & 0x1));
   assert(Oend & 0x1);
   assert(Ostart < 0x40);
   assert(Oend < 0x40);

   for(i = Ostart >> 1; i <= Oend >> 1; i++)
   {
      if(r16) Cart.CS2M_RW[i].Read16  = r16;
      if(w8)  Cart.CS2M_RW[i].Write8  = w8;
      if(w16) Cart.CS2M_RW[i].Write16 = w16;
   }
}

bool CART_Init(const int cart_type, const char *rom_dir, const char *main_fname, const struct STVGameInfo *sgi)
{
   unsigned i;

   CartInfo_CS01_SetRW8W16(&Cart, 0x02000000, 0x04FFFFFF, DummyRead, DummyWrite, DummyWrite);
   CartInfo_CS2M_SetRW8W16(&Cart, 0x00, 0x3F, DummyRead, DummyWrite, DummyWrite);

   Cart.Reset          = DummyReset;
   Cart.Kill           = DummyKill;
   Cart.GetNVInfo      = DummyGetNVInfo;
   Cart.GetClearNVDirty = DummyGetClearNVDirty;
   Cart.StateAction    = DummyStateAction;
   Cart.EventHandler   = DummyUpdate;
   Cart.AdjustTS       = DummyAdjustTS;
   Cart.SetCPUClock    = DummySetCPUClock;

   switch(cart_type)
   {
      default:
      case CART_NONE:
         break;

      case CART_BACKUP_MEM:
         CART_Backup_Init(&Cart);
         break;

      case CART_EXTRAM_1M:
      case CART_EXTRAM_4M:
         CART_ExtRAM_Init(&Cart, cart_type == CART_EXTRAM_4M);
         break;

      case CART_KOF95:
      case CART_ULTRAMAN:
      {
         char fpath[4096];
         const char *path_cxx = MDFN_GetSettingS((cart_type == CART_KOF95) ? "ss.cart.kof95_path" : "ss.cart.ultraman_path");
         if(path_cxx)
         {
            const char *path = MDFN_MakeFName(fpath, sizeof(fpath), MDFNMKF_FIRMWARE, 0, path_cxx);
            RFILE      *fp   = filestream_open(path,
                  RETRO_VFS_FILE_ACCESS_READ,
                  RETRO_VFS_FILE_ACCESS_HINT_NONE);

            if(fp)
            {
               CART_ROM_Init(&Cart, fp);
               filestream_close(fp);
            }
         }
      }
      break;

      case CART_AR4MP:
      {
         char fpath[4096];
         const char *path_cxx = MDFN_GetSettingS("ss.cart.satar4mp_path");
         if(path_cxx)
         {
            const char *path = MDFN_MakeFName(fpath, sizeof(fpath), MDFNMKF_FIRMWARE, 0, path_cxx);
            RFILE      *fp   = filestream_open(path,
                  RETRO_VFS_FILE_ACCESS_READ,
                  RETRO_VFS_FILE_ACCESS_HINT_NONE);

            if(fp)
            {
               CART_AR4MP_Init(&Cart, fp);
               filestream_close(fp);
            }
         }
      }
      break;

      case CART_CS1RAM_16M:
         CART_CS1RAM_Init(&Cart);
         break;

      case CART_BOOTROM:
      {
         char fpath[4096];
         const char *path_cxx = MDFN_GetSettingS("ss.cart.bootrom_path");
         if(path_cxx)
         {
            const char *path = MDFN_MakeFName(fpath, sizeof(fpath), MDFNMKF_FIRMWARE, 0, path_cxx);
            RFILE      *fp   = filestream_open(path,
                  RETRO_VFS_FILE_ACCESS_READ,
                  RETRO_VFS_FILE_ACCESS_HINT_NONE);

            if(fp)
            {
               bool ok = CART_BootROM_Init(&Cart, fp);
               filestream_close(fp);
               if(!ok)
                  return false;
            }
         }
      }
      break;

      case CART_STV:
         /* ST-V loads a multi-file MAME-style ROM set out of rom_dir.
            The STVGameInfo entry (sgi) tells us which files to open
            and how each one maps into the Saturn A-bus address space.
            Caller must have populated all three of rom_dir /
            main_fname / sgi -- enforce here and fall through to the
            dummy default if anything's missing rather than crash on a
            null deref inside CART_STV_Init. */
         if(rom_dir && main_fname && sgi)
         {
            if(!CART_STV_Init(&Cart, rom_dir, main_fname, sgi))
               return false;
         }
         break;

      /* case CART_NLMODEM:
            CART_NLModem_Init(&Cart);
            break; */
   }

   for(i = 0; i < (sizeof(Cart.CS01_RW) / sizeof(Cart.CS01_RW[0])); i++)
      assert(Cart.CS01_RW[i].Read16 != NULL && Cart.CS01_RW[i].Write8 != NULL && Cart.CS01_RW[i].Write16 != NULL);

   for(i = 0; i < (sizeof(Cart.CS2M_RW) / sizeof(Cart.CS2M_RW[0])); i++)
      assert(Cart.CS2M_RW[i].Read16 != NULL && Cart.CS2M_RW[i].Write8 != NULL && Cart.CS2M_RW[i].Write16 != NULL);

   return true;
}
