/******************************************************************************/
/* Mednafen Sega Saturn Emulation Module                                      */
/******************************************************************************/
/* stv.c - ST-V cart emulation
**  Copyright (C) 2022 Mednafen Team
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

/*
 TODO (carried over from upstream):
   - 315-5881 decryption support
   - Decathlete decryption/decompression chip support
   - "sanjeon" ROM twiddle for Final Fight Revenge (upstream has it,
     adapter below preserves the logic, untested in this fork)
*/

/* Converted from cart/stv.cpp. The Write helper was a
   template<typename T>; T is used (sizeof(T) selects byte vs word
   handling), so it monomorphizes to the two concrete instantiations
   the original used: Write<uint8_t> and Write<uint16_t>. The
   STVGameInfo / STVROMLayout structs and STV_* enums now come from
   the C-includable db_stv.h. new[]/delete[] become calloc/free. */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <boolean.h>

#include <libretro.h>
#include <streams/file_stream.h>

#include "../../mednafen-types.h"   /* MDFN_HOT */
#include "../../state.h"            /* SFORMAT, SFVAR, SFEND, MDFNSS_StateAction */
#include "../cart.h"
#include "../db_stv.h"
#include "../stvio.h"               /* STVIO_StateAction -- chained from this driver's StateAction */
#include "stv.h"
#include "stv_5881.h"             /* Sega 315-5881 encryption/compression chip */

/* SS_SetPhysMemMap: defined in ss.c, declared in ss.h.
   Mirror the prototype (trailing is_writeable default arg dropped). */
void SS_SetPhysMemMap(uint32_t Astart, uint32_t Aend, uint16_t *ptr, uint32_t length, bool is_writeable);

extern retro_log_printf_t log_cb;

static unsigned  ECChip;
static uint16_t *ROM;

static uint8_t rsg_thingy;
static uint8_t rsg_counter;

/* --- Sega 315-5881 A-bus protection protocol state (ST-V) ---
   MAME maps four 32-bit registers at 0x4FFFFF0..0x4FFFFFF (offsets 0..3).
   a_bus[] latches written values; abus_protenable bit 0x00010000 gates the
   decryption read path; abus_protkey feeds set_subkey. See common_prot_r/w
   in MAME's stv.cpp. */
static uint32_t a_bus[4];
static uint32_t abus_protenable;
static uint32_t abus_protkey;

/* Endianness verify-knob for the source-data fetch. MAME's crypt_read_callback
   does read_word(0x02000000 + 2*addr) then byteswaps. beetle stores FFR's data
   ROMs as STV_MAP_16LE whereas MAME uses ROM_LOAD16_WORD_SWAP into a BE region;
   if decrypted graphics come out byte-swapped on real hardware, flip this to 0.
   This is the single most likely thing to need adjusting (see patch notes). */
#ifndef STV5881_RAW_READ_BYTESWAP
#define STV5881_RAW_READ_BYTESWAP 1
#endif

static uint16_t stv5881_source_read(uint32_t addr)
{
   uint32_t off = (2u * addr) & 0x3FFFFFE;
   uint16_t dat = *(uint16_t*)((uint8_t*)ROM + off);
#if STV5881_RAW_READ_BYTESWAP
   return (uint16_t)(((dat & 0xff00) >> 8) | ((dat & 0x00ff) << 8));
#else
   return dat;
#endif
}

static MDFN_HOT void ROM_Read(uint32_t A, uint16_t *DB)
{
   *DB = *(uint16_t*)((uint8_t*)ROM + ((A - 0x2000000) & 0x3FFFFFE));

   if(ECChip == STV_EC_CHIP_315_5881 && A >= 0x04FFFFF0)
   {
      unsigned off = (A - 0x04FFFFF0) >> 2;   /* register index 0..3        */
      int      low = (A & 2) != 0;            /* low 16-bit half of the reg */

      if(abus_protenable & 0x00010000)        /* decryption active          */
      {
         if(off == 3)
         {
            /* MAME common_prot_r offset 3 reads do_decrypt() twice, byteswaps
               each, and returns res2 | (res << 16). On this 16-bit bus a SH-2
               longword read is split into two 16-bit reads issued high-word
               (lower address, A&2==0) first, so one decrypt per 16-bit read
               preserves MAME's ordering: 0x4FFFFFC -> res, 0x4FFFFFE -> res2. */
            uint16_t w = STV5881_DoDecrypt();
            *DB = (uint16_t)(((w & 0xff00) >> 8) | ((w & 0x00ff) << 8));
         }
         else
            *DB = low ? (uint16_t)a_bus[off] : (uint16_t)(a_bus[off] >> 16);
      }
      else if(a_bus[off] != 0)
      {
         /* protection off: MAME returns the latch if nonzero, else the ROM
            mirror (already in *DB from the load above, which aliases MAME's
            ROM[(0x02fffff0/4)+offset]). */
         *DB = low ? (uint16_t)a_bus[off] : (uint16_t)(a_bus[off] >> 16);
      }
      return;
   }

   if(A >= 0x04FFFFFC && rsg_thingy)
   {
      *DB = (((((rsg_counter & 0x7F) << 1) + 0) << 8) | ((((rsg_counter & 0x7F) << 1) + 1) << 0)) & (0xF0F0 >> ((rsg_counter & 0x80) >> 5));
      rsg_counter++;
   }
}

uint8_t CART_STV_PeekROM(uint32_t A)
{
   assert(A < 0x3000000);
   /* ne16_rbo_be<uint8_t>(ROM, A) folded. */
#ifdef MSB_FIRST
   return ((const uint8_t*)ROM)[A];
#else
   return ((const uint8_t*)ROM)[A ^ 1];
#endif
}

/* Was Write<uint8_t>: 8-bit access, the (A & 1) path. */
static MDFN_HOT void Write8(uint32_t A, uint16_t *DB)
{
   if(ECChip == STV_EC_CHIP_315_5881 && A >= 0x04FFFFF0)
   {
      /* The 315-5881 protection ports (0x4FFFFF0..0x4FFFFFF) are driven by
         the game with 32-bit writes, which the A-bus splits into 16-bit
         cycles handled by Write16 -- never 8-bit. A byte write here is not a
         real access pattern, so ignore it rather than guess a byte lane. */
      return;
   }

   if(A >= 0x04FFFFF0 && ECChip == STV_EC_CHIP_RSG)
   {
      if(A & 1)
      {
         if((A & ~1) == 0x04FFFFF0)
         {
            rsg_thingy  = *DB & 0x1;
            rsg_counter = 0;
         }
      }
   }
}

/* Was Write<uint16_t>: 16-bit access, the sizeof(T) == 2 path. */
static MDFN_HOT void Write16(uint32_t A, uint16_t *DB)
{
   if(ECChip == STV_EC_CHIP_315_5881 && A >= 0x04FFFFF0)
   {
      unsigned off = (A - 0x04FFFFF0) >> 2;
      int      low = (A & 2) != 0;

      /* COMBINE_DATA into the addressed 16-bit half of the 32-bit latch */
      if(low)
         a_bus[off] = (a_bus[off] & 0xFFFF0000u) | *DB;
      else
         a_bus[off] = (a_bus[off] & 0x0000FFFFu) | ((uint32_t)*DB << 16);

      if(off == 0)
         abus_protenable = a_bus[0];
      else if(off == 2)
      {
         /* MAME: hi word -> set_addr_low(data>>16); lo word -> set_addr_high(data&0xffff) */
         if(!low)
            STV5881_SetAddrLow(*DB);
         else
            STV5881_SetAddrHigh(*DB);
      }
      else if(off == 3)
      {
         abus_protkey = a_bus[3];
         if(!low)
            STV5881_SetSubkey(*DB);           /* MAME: set_subkey(protkey>>16) */
      }
      return;
   }

   if(A >= 0x04FFFFF0 && ECChip == STV_EC_CHIP_RSG)
   {
      if((A & ~1) == 0x04FFFFF0)
      {
         rsg_thingy  = *DB & 0x1;
         rsg_counter = 0;
      }
   }
}

static void Reset(bool powering_up)
{
   (void)powering_up;
   rsg_thingy  = 0;
   rsg_counter = 0;

   a_bus[0] = a_bus[1] = a_bus[2] = a_bus[3] = 0;
   abus_protenable = 0;
   abus_protkey    = 0;
   if(ECChip == STV_EC_CHIP_315_5881)
      STV5881_Reset();
}

static void StateAction(StateMem *sm, const unsigned load, const bool data_only)
{
   SFORMAT StateRegs[] =
   {
      SFVAR(rsg_thingy),
      SFVAR(rsg_counter),

      SFVAR(a_bus[0]),
      SFVAR(a_bus[1]),
      SFVAR(a_bus[2]),
      SFVAR(a_bus[3]),
      SFVAR(abus_protenable),
      SFVAR(abus_protkey),

      SFEND
   };

   MDFNSS_StateAction(sm, load, data_only, StateRegs, "STV_CART", false);

   if(ECChip == STV_EC_CHIP_315_5881)
      STV5881_StateAction(sm, load, data_only);

   /* ST-V's runtime state is split between this cart driver
    * (rsg_thingy / rsg_counter -- the security-chip handshake
    * counters above) and stvio.c (IOGA DataIn/DataOut/DataDir,
    * CoinPending / CoinActiveCounter, HammerX / HammerY for
    * the hammer-control scheme, and the AK93C45 EEPROM).  We
    * own the StateAction entry that CART_StateAction reaches,
    * so chain through to STVIO_StateAction here -- that keeps
    * the cart's full state under a single save-state branch
    * and means LibRetro_StateAction doesn't need an ST-V-only
    * dispatch case. */
   STVIO_StateAction(sm, load, data_only);
}

static void Kill(void)
{
   if(ROM)
   {
      free(ROM);
      ROM = NULL;
   }
}

static bool JoinPath(char *out, size_t out_size, const char *dir, const char *fname)
{
   /* Simple path concatenation. We expect rom_dir already does NOT have a
      trailing slash; if the caller fed us one we tolerate it. Doesn't try to
      handle Windows backslashes -- libretro frontends normalize to forward
      slashes by the time this layer sees a path.
      Pre-fix strncat chain silently truncated on overflow; the caller then
      saw a misleading "ROM image file not found" instead of a path-too-long
      error.  snprintf with return-check surfaces the failure here. */
   size_t dl = strlen(dir);
   bool   has_sep = (dl > 0 && (dir[dl - 1] == '/' || dir[dl - 1] == '\\'));
   int    n;

   if (has_sep)
      n = snprintf(out, out_size, "%s%s", dir, fname);
   else
      n = snprintf(out, out_size, "%s/%s", dir, fname);

   return !(n < 0 || (size_t)n >= out_size);
}

bool CART_STV_Init(struct CartInfo *c, const char *rom_dir, const char *main_fname, const struct STVGameInfo *sgi)
{
   size_t i;

   assert(rom_dir && main_fname && sgi);

   ECChip = sgi->ec_chip;

   ROM = (uint16_t*)calloc(0x3000000 / sizeof(uint16_t), sizeof(uint16_t));
   if (!ROM)
   {
      /* Pre-conversion C++ used `new uint16_t[]` which threw on OOM;
       * the conversion to calloc dropped the check.  Kill() at the
       * fail label NULL-checks ROM, so this path is safe.  ECChip is
       * a file-static that's reassigned on every CART_Init attempt
       * (above this in the function), so leaving the now-failed
       * value there has no observable effect: the cart's read/write
       * handlers are still the CART_Init-installed Dummies. */
      log_cb(RETRO_LOG_ERROR,
            "ST-V: out of memory allocating cart ROM buffer (%u bytes).\n",
            0x3000000u);
      goto fail;
   }
   memset(ROM, 0xFF, 0x3000000);

   for(i = 0; i < sizeof(sgi->rom_layout) / sizeof(sgi->rom_layout[0]) && sgi->rom_layout[i].size; i++)
   {
      const struct STVROMLayout *rle      = &sgi->rom_layout[i];
      const struct STVROMLayout *prev_rle = i ? &sgi->rom_layout[i - 1] : NULL;
      const bool prev_match = prev_rle && !strcmp(rle->fname, prev_rle->fname);
      char fpath[4096];
      RFILE *fp;

      if(prev_match)
      {
         /* Same source file mapped twice (mirrored region). Copy from
            previous slot to avoid re-reading the file. */
         assert(rle->size == prev_rle->size);
         assert(rle->map == prev_rle->map);

         if(rle->map == STV_MAP_BYTE)
         {
            uint32_t j;
            for(j = 0; j < rle->size; j++)
            {
               const uint32_t src_off = prev_rle->offset + (j << 1);
               const uint32_t dst_off = rle->offset + (j << 1);
               uint8_t tmp;
               /* ne16_rbo_be<uint8_t>(ROM, src) / ne16_wbo_be<uint8_t>(ROM, dst, val). */
#ifdef MSB_FIRST
               tmp = ((const uint8_t*)ROM)[src_off];
               ((uint8_t*)ROM)[dst_off] = tmp;
#else
               tmp = ((const uint8_t*)ROM)[src_off ^ 1];
               ((uint8_t*)ROM)[dst_off ^ 1] = tmp;
#endif
            }
         }
         else
         {
            memmove((uint8_t*)ROM + rle->offset, (uint8_t*)ROM + prev_rle->offset, rle->size);
         }
         continue;
      }

      if(!JoinPath(fpath, sizeof(fpath), rom_dir, rle->fname))
      {
         log_cb(RETRO_LOG_ERROR, "ST-V: ROM image path too long (rom_dir + \"%s\")\n", rle->fname);
         goto fail;
      }
      fp = filestream_open(fpath,
                           RETRO_VFS_FILE_ACCESS_READ,
                           RETRO_VFS_FILE_ACCESS_HINT_NONE);
      if(!fp)
      {
         log_cb(RETRO_LOG_ERROR, "ST-V: cannot open ROM image file \"%s\"\n", fpath);
         goto fail;
      }

      if(rle->map == STV_MAP_BYTE)
      {
         /* Byte-interleaved into 16-bit ROM: one source byte per 16-bit
            ROM slot, big-endian high byte. */
         uint32_t j;
         for(j = 0; j < rle->size; j++)
         {
            uint8_t tmp;
            const uint32_t dst_off = rle->offset + (j << 1);
            if(filestream_read(fp, &tmp, 1) != 1)
            {
               log_cb(RETRO_LOG_ERROR, "ST-V: ROM image \"%s\" shorter than expected (need %u bytes)\n", fpath, rle->size);
               filestream_close(fp);
               goto fail;
            }
            /* ne16_wbo_be<uint8_t>(ROM, offs, tmp) folded. */
#ifdef MSB_FIRST
            ((uint8_t*)ROM)[dst_off] = tmp;
#else
            ((uint8_t*)ROM)[dst_off ^ 1] = tmp;
#endif
         }
      }
      else
      {
         uint8_t *dest;
         int64_t  dr;

         assert(!(rle->offset & 1));
         assert(!(rle->size & 1));
         dest = (uint8_t*)ROM + rle->offset;
         dr = filestream_read(fp, dest, rle->size);
         if(dr != (int64_t)rle->size)
         {
            log_cb(RETRO_LOG_ERROR, "ST-V: ROM image \"%s\" shorter than expected (got %lld of %u bytes)\n", fpath, (long long)dr, rle->size);
            filestream_close(fp);
            goto fail;
         }

         /* Re-pack source bytes into native-endian uint16_t slots.
          *
          *   STV_MAP_16LE: on-disk bytes are LE.  On LSB host: native
          *                 is already LE, no-op.  On MSB host: byteswap
          *                 to convert LE -> native BE.
          *
          *   STV_MAP_16BE: on-disk bytes are BE.  On MSB host: native
          *                 is already BE, no-op.  On LSB host: byteswap
          *                 to convert BE -> native LE.
          *
          * Matches upstream's Endian_A16_NE_LE / Endian_A16_NE_BE
          * gating shape (those helpers boil down to the same
          * #ifdef-driven swap).
          *
          * Pre-fix this fork unconditionally byteswapped on
          * STV_MAP_16BE and unconditionally did nothing on
          * STV_MAP_16LE -- only correct on LSB host.  The build
          * matrix's default_BE config would have silently mis-loaded
          * both formats: 16BE byteswapped when native already matched
          * disk, 16LE left as-is when it should have been swapped.
          * No symptoms in practice -- there's no shipping libretro
          * BE host that actually runs ST-V content -- but the bug
          * was clearly wrong. */
         {
#ifdef MSB_FIRST
            const bool need_swap = (rle->map == STV_MAP_16LE);
#else
            const bool need_swap = (rle->map == STV_MAP_16BE);
#endif
            if(need_swap)
            {
               /* Endian_A16_Swap folded: byte-pair swap loop. */
               uint8_t *p__ = (uint8_t*)dest;
               uint32_t n__ = rle->size >> 1;
               uint32_t k__;
               for(k__ = 0; k__ < n__; k__++)
               {
                  uint8_t t = p__[k__ * 2];
                  p__[k__ * 2]     = p__[k__ * 2 + 1];
                  p__[k__ * 2 + 1] = t;
               }
            }
         }
      }
      filestream_close(fp);
   }

   if(sgi->romtwiddle == STV_ROMTWIDDLE_SANJEON)
   {
      /* Bit-permutation used by the "Final Fight Revenge" Korean version
         ("sanjeon"). Carried over verbatim from upstream Mednafen. Upstream
         uses MDFN_densb / MDFN_ennsb (native-endian signed-byte) for the
         64-bit load/store, which are equivalent to plain byte-level memcpy
         on either endian -- the bit operations don't care about byte order
         since they operate per-byte (mask constants are byte-aligned). This
         fork doesn't ship those helpers, so use memcpy explicitly. */
      uint32_t k;
      for(k = 0; k < 0x3000000 / sizeof(uint64_t); k++)
      {
         uint64_t tmp;
         memcpy(&tmp, (uint8_t*)ROM + (k << 3), sizeof(tmp));
         tmp = ~tmp;
         tmp = ((tmp & 0x0404040404040404ULL) >> 2) | ((tmp & 0x0101010101010101ULL) << 6)
             |  (tmp & 0x2020202020202020ULL)
             | ((tmp & 0x1010101010101010ULL) >> 3) | ((tmp & 0x4040404040404040ULL) << 1)
             | ((tmp & 0x0808080808080808ULL) >> 1) | ((tmp & 0x8080808080808080ULL) >> 3)
             | ((tmp & 0x0202020202020202ULL) << 2);
         memcpy((uint8_t*)ROM + (k << 3), &tmp, sizeof(tmp));
      }
   }

   SS_SetPhysMemMap(0x02000000, 0x04FFFFFF, ROM, 0x3000000, false);
   CartInfo_CS01_SetRW8W16(c, 0x02000000, 0x04FFFFFF, ROM_Read, Write8, Write16);

   if(ECChip == STV_EC_CHIP_315_5881)
      STV5881_Init(sgi->crypt_key, stv5881_source_read);

   c->StateAction = StateAction;
   c->Reset       = Reset;
   c->Kill        = Kill;

   /* Suppress unused-variable noise. */
   (void)main_fname;

   return true;

fail:
   Kill();
   return false;
}
