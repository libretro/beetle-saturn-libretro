/******************************************************************************/
/* Mednafen Sega Saturn Emulation Module                                      */
/******************************************************************************/
/* bootrom.c - Bootable cart ROM emulation
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

/* Converted from cart/bootrom.cpp. No templates here; the conversion
   is the include trim, new[]/delete[] -> calloc/free, nullptr ->
   NULL, and the c->CS01_SetRW8W16 member calls becoming free-function
   calls. */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <boolean.h>

#include <libretro.h>
#include <streams/file_stream.h>

#include "../../mednafen-types.h"   /* MDFN_HOT, MDFN_COLD */
#include "../../math_ops.h"         /* round_up_pow2 */
#include "../../mdfn_gameinfo.h"    /* MDFNGameInfo */
#include "../../hash/sha256.h"
#include "../cart.h"
#include "bootrom.h"
#include "backup.h"

/* SS_SetPhysMemMap: defined in ss.c, declared in ss.h.
   Mirror the prototype (trailing is_writeable default arg dropped). */
void SS_SetPhysMemMap(uint32_t Astart, uint32_t Aend, uint16_t *ptr, uint32_t length, bool is_writeable);

extern retro_log_printf_t log_cb;

static uint16_t *ROM = NULL;
static uint32_t  ROM_Mask[2];

static MDFN_HOT void CS0_ROM_Read(uint32_t A, uint16_t *DB)
{
   const uint32_t offs = (A - 0x02000000) & ROM_Mask[0];

   /* ne16_rbo_be<uint16_t>(ROM, byte_off) folded: aligned uint16_t
    * read from uint16_t array.  Same on both BE and LE hosts since
    * each uint16_t slot is stored host-endian. */
   *DB = ROM[offs >> 1];
}

static MDFN_HOT void CS1_ROM_Read(uint32_t A, uint16_t *DB)
{
   const uint32_t offs = 0x02000000 + ((A - 0x04000000) & ROM_Mask[1]);

   *DB = ROM[offs >> 1];
}

static MDFN_COLD void Kill(void)
{
   if(ROM)
   {
      free(ROM);
      ROM = NULL;
   }
}

bool CART_BootROM_Init(struct CartInfo *c, RFILE *str)
{
   const uint64_t ss = filestream_get_size(str);
   const uint64_t min_size = 1;
   const uint64_t max_size = 0x3000000;
   uint32_t ROM_Size;
   sha256_hasher h;
   sha256_digest dig;
   unsigned i;

   sha256_hasher_init(&h);

   if(ss < min_size)
   {
      log_cb(RETRO_LOG_ERROR, "Bootable Saturn cart ROM image is smaller than the minimum of %llu bytes.\n", (unsigned long long)min_size);
      return false;
   }

   if(ss > max_size)
   {
      log_cb(RETRO_LOG_ERROR, "Bootable Saturn cart ROM image is larger than the maximum of %llu bytes.\n", (unsigned long long)max_size);
      return false;
   }
   /* */
   if(ss > 0x2000000)
      ROM_Size = 0x2000000 + round_up_pow2((ss - 0x2000000 + 0xFFFF) &~ 0xFFFF);
   else
      ROM_Size = round_up_pow2((ss + 0xFFFF) &~ 0xFFFF);

   assert(ROM_Size >= ss);
   /* */
   ROM = (uint16_t*)calloc(ROM_Size / sizeof(uint16_t), sizeof(uint16_t));
   if (!ROM)
   {
      /* Pre-conversion C++ code used `new uint16_t[]` which threw
       * std::bad_alloc on OOM; the conversion to calloc dropped the
       * check.  Cart.Kill is still DummyKill at this point (CART_Init
       * installs the real Kill on success only), so an early bail
       * leaves the Cart struct in a safe state for the caller. */
      log_cb(RETRO_LOG_ERROR,
            "Bootable Saturn cart ROM: out of memory allocating %u-byte buffer.\n",
            (unsigned)ROM_Size);
      return false;
   }
   memset(ROM, 0x00, ROM_Size);
   filestream_read(str, ROM, ss);
   sha256_hasher_process(&h, ROM, ss);
   dig = sha256_hasher_digest(&h);
   memcpy(MDFNGameInfo->MD5, dig.b, 16);

   for(i = 0; i < ROM_Size / sizeof(uint16_t); i++)
   {
      /* MDFN_de16msb<true> folded: BE-on-disk to host-endian. */
#ifndef MSB_FIRST
      ROM[i] = (uint16_t)((ROM[i] << 8) | (ROM[i] >> 8));
#endif
   }

   SS_SetPhysMemMap(0x02000000, 0x03FFFFFF, ROM, ((uint32_t)(0x02000000) < (uint32_t)(ROM_Size) ? (uint32_t)(0x02000000) : (uint32_t)(ROM_Size)), false);
   CartInfo_CS01_SetRW8W16(c, 0x02000000, 0x03FFFFFF, CS0_ROM_Read, NULL, NULL);

   c->Kill = Kill;

   ROM_Mask[0] = (round_up_pow2(ROM_Size) - 1) & 0x01FFFFFE;

   if(ROM_Size > 0x2000000)
   {
      ROM_Mask[1] = (round_up_pow2(ROM_Size - 0x2000000) - 1) & 0x00FFFFFE;

      SS_SetPhysMemMap(0x04000000, 0x04FFFFFF, ROM + (0x02000000 / sizeof(uint16_t)), ROM_Size - 0x02000000, false);
      CartInfo_CS01_SetRW8W16(c, 0x04000000, 0x04FFFFFF, CS1_ROM_Read, NULL, NULL);
   }
   else
   {
      CART_Backup_Init(c);
      assert(c->Kill == Kill);
   }

   return true;
}
