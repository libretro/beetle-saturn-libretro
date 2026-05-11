/******************************************************************************/
/* Mednafen Sega Saturn Emulation Module                                      */
/******************************************************************************/
/* stv.cpp - ST-V Cart Emulation
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

#include "common.h"
#include "stv.h"
#include "../db.h"

#include <streams/file_stream.h>
#include <string>
#include <cstring>

static unsigned ECChip;
static uint16* ROM;

static uint8 rsg_thingy;
static uint8 rsg_counter;

static MDFN_HOT void ROM_Read(uint32 A, uint16* DB)
{
 *DB = *(uint16*)((uint8*)ROM + ((A - 0x2000000) & 0x3FFFFFE));

 if(A >= 0x04FFFFFC && rsg_thingy)
 {
  *DB = (((((rsg_counter & 0x7F) << 1) + 0) << 8) | ((((rsg_counter & 0x7F) << 1) + 1) << 0)) & (0xF0F0 >> ((rsg_counter & 0x80) >> 5));
  rsg_counter++;
 }
}

uint8 CART_STV_PeekROM(uint32 A)
{
 assert(A < 0x3000000);
 return ne16_rbo_be<uint8>(ROM, A);
}

template<typename T>
static MDFN_HOT void Write(uint32 A, uint16* DB)
{
 if(A >= 0x04FFFFF0 && ECChip == STV_EC_CHIP_RSG)
 {
  if(sizeof(T) == 2 || (A & 1))
  {
   if((A & ~1) == 0x04FFFFF0)
   {
    rsg_thingy = *DB & 0x1;
    rsg_counter = 0;
   }
  }
 }
}

static void Reset(bool powering_up)
{
 rsg_thingy = 0;
 rsg_counter = 0;
}

static void StateAction(StateMem* sm, const unsigned load, const bool data_only)
{
 SFORMAT StateRegs[] =
 {
  SFVAR(rsg_thingy),
  SFVAR(rsg_counter),

  SFEND
 };

 MDFNSS_StateAction(sm, load, data_only, StateRegs, "STV_CART", false);
}

static void Kill(void)
{
 if(ROM)
 {
  delete[] ROM;
  ROM = nullptr;
 }
}

static std::string JoinPath(const char* dir, const char* fname)
{
 // Simple path concatenation. We expect rom_dir already does NOT have a
 // trailing slash; if the caller fed us one we tolerate it. Doesn't try to
 // handle Windows backslashes -- libretro frontends normalize to forward
 // slashes by the time this layer sees a path.
 std::string s(dir);
 if(!s.empty() && s.back() != '/' && s.back() != '\\')
  s += '/';
 s += fname;
 return s;
}

void CART_STV_Init(CartInfo* c, const char* rom_dir, const char* main_fname, const STVGameInfo* sgi)
{
 assert(rom_dir && main_fname && sgi);

 try
 {
  ECChip = sgi->ec_chip;

  ROM = new uint16[0x3000000 / sizeof(uint16)];
  memset(ROM, 0xFF, 0x3000000);

  for(size_t i = 0; i < sizeof(sgi->rom_layout) / sizeof(sgi->rom_layout[0]) && sgi->rom_layout[i].size; i++)
  {
   const STVROMLayout* rle = &sgi->rom_layout[i];
   const STVROMLayout* prev_rle = i ? &sgi->rom_layout[i - 1] : nullptr;
   const bool prev_match = prev_rle && !strcmp(rle->fname, prev_rle->fname);

   if(prev_match)
   {
    // Same source file mapped twice (mirrored region). Copy from
    // previous slot to avoid re-reading the file.
    assert(rle->size == prev_rle->size);
    assert(rle->map == prev_rle->map);

    if(rle->map == STV_MAP_BYTE)
    {
     for(uint32 j = 0; j < rle->size; j++)
     {
      uint8 tmp = ne16_rbo_be<uint8>(ROM, prev_rle->offset + (j << 1));
      ne16_wbo_be<uint8>(ROM, rle->offset + (j << 1), tmp);
     }
    }
    else
    {
     memmove((uint8*)ROM + rle->offset, (uint8*)ROM + prev_rle->offset, rle->size);
    }
    continue;
   }

   const std::string fpath = JoinPath(rom_dir, rle->fname);
   RFILE* fp = filestream_open(fpath.c_str(),
                               RETRO_VFS_FILE_ACCESS_READ,
                               RETRO_VFS_FILE_ACCESS_HINT_NONE);
   if(!fp)
    throw MDFN_Error(0, _("ST-V: cannot open ROM image file \"%s\""), fpath.c_str());

   try
   {
    if(rle->map == STV_MAP_BYTE)
    {
     // Byte-interleaved into 16-bit ROM: one source byte per 16-bit
     // ROM slot, big-endian high byte.
     for(uint32 j = 0; j < rle->size; j++)
     {
      uint8 tmp;
      if(filestream_read(fp, &tmp, 1) != 1)
       throw MDFN_Error(0, _("ST-V: ROM image \"%s\" shorter than expected (need %u bytes)"), fpath.c_str(), rle->size);
      ne16_wbo_be<uint8>(ROM, rle->offset + (j << 1), tmp);
     }
    }
    else
    {
     assert(!(rle->offset & 1));
     assert(!(rle->size & 1));
     uint8* dest = (uint8*)ROM + rle->offset;
     int64_t dr = filestream_read(fp, dest, rle->size);
     if(dr != (int64_t)rle->size)
      throw MDFN_Error(0, _("ST-V: ROM image \"%s\" shorter than expected (got %lld of %u bytes)"), fpath.c_str(), (long long)dr, rle->size);

     // Re-pack source bytes into native-endian uint16 slots.
     // Host is LSB_FIRST (the only build configuration this fork ships).
     // For 16LE-packed source: data is already little-endian, native order
     // matches, so this is a no-op. For 16BE-packed source: byteswap each
     // 16-bit word to get native LE.
     if(rle->map == STV_MAP_16BE)
      Endian_A16_Swap(dest, rle->size >> 1);
     // STV_MAP_16LE: no-op on LSB host.
    }
   }
   catch(...)
   {
    filestream_close(fp);
    throw;
   }
   filestream_close(fp);
  }

  if(sgi->romtwiddle == STV_ROMTWIDDLE_SANJEON)
  {
   // Bit-permutation used by the "Final Fight Revenge" Korean version
   // ("sanjeon"). Carried over verbatim from upstream Mednafen. Upstream
   // uses MDFN_densb / MDFN_ennsb (native-endian signed-byte) for the
   // 64-bit load/store, which are equivalent to plain byte-level memcpy
   // on either endian -- the bit operations don't care about byte order
   // since they operate per-byte (mask constants are byte-aligned). This
   // fork doesn't ship those helpers, so use memcpy explicitly.
   for(uint32 i = 0; i < 0x3000000 / sizeof(uint64_t); i++)
   {
    uint64_t tmp;
    memcpy(&tmp, (uint8*)ROM + (i << 3), sizeof(tmp));
    tmp = ~tmp;
    tmp = ((tmp & 0x0404040404040404ULL) >> 2) | ((tmp & 0x0101010101010101ULL) << 6)
        |  (tmp & 0x2020202020202020ULL)
        | ((tmp & 0x1010101010101010ULL) >> 3) | ((tmp & 0x4040404040404040ULL) << 1)
        | ((tmp & 0x0808080808080808ULL) >> 1) | ((tmp & 0x8080808080808080ULL) >> 3)
        | ((tmp & 0x0202020202020202ULL) << 2);
    memcpy((uint8*)ROM + (i << 3), &tmp, sizeof(tmp));
   }
  }

  SS_SetPhysMemMap(0x02000000, 0x04FFFFFF, ROM, 0x3000000, false);
  c->CS01_SetRW8W16(0x02000000, 0x04FFFFFF, ROM_Read, Write<uint8>, Write<uint16>);

  c->StateAction = StateAction;
  c->Reset = Reset;
  c->Kill = Kill;

  // Suppress unused-variable noise.
  (void)main_fname;
 }
 catch(...)
 {
  Kill();
  throw;
 }
}
