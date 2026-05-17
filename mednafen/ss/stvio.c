/******************************************************************************/
/* Mednafen Sega Saturn Emulation Module                                      */
/******************************************************************************/
/* stvio.c - ST-V I/O Emulation
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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <boolean.h>

#include "../mednafen-types.h"   /* MDFN_HOT, MDFN_COLD, MDFN_HIDE */
#include "../hash/crc.h"         /* crc16_ccitt */

#include "ak93c45.h"

#include "stvio.h"
#include "smpc_iodevice.h"

#include "cart/stv.h"

/* sound.h (SOUND_Reset68K, SOUND_ResetSCSP, SOUND_Set68KActive)
 * and smpc.h (SMPC_SetInput) became C-includable in their own
 * commits (sound: 72933f7, smpc: 13b748f), so include them
 * directly instead of forward-declaring each symbol inline.
 *
 * ss.h and mednafen.h (which scaffold the whole module set in C++
 * builds) are still not pulled in -- they're either C-incompatible
 * via git.h's std:: usage or carry more declarations than this TU
 * needs.  This matches the principle established in the original
 * vdp1.c conversion: include only the headers whose interface this
 * TU actually consumes. */
#include "sound.h"
#include "smpc.h"

static unsigned ControlScheme;

static uint8_t* DPtr[13];

static uint8_t DataDir;
static uint8_t DataOut[0x8];
static uint8_t DataIn[0x8];

static uint32_t CoinPending;
static int32_t CoinActiveCounter;

static uint8_t HammerX, HammerY;

static uint8_t prev_sctrl;
static uint8_t prev_ectrl;
static AK93C45 eep;

static IODevice *gun = NULL;

void STVIO_SetInput(unsigned port, const char* type, uint8_t* ptr)
{
 assert(port < 13);

 if(port < 12)
 {
  if(ControlScheme == STV_CONTROL_HAMMER)
  {
   if(port != 0 || strcmp(type, "gun"))
    ptr = NULL;
  }
  else if(port < 2 && strcmp(type, "gamepad"))
   ptr = NULL;
 }

 DPtr[port] = ptr;
}

void STVIO_SetCrosshairsColor(unsigned port, uint32_t color)
{
 if(!port)
  IODevice_Gun_SetCrosshairsColor(gun, color);
}

void STVIO_TransformInput(void)
{
 *DPtr[12] &= ~0x01; // Zero SS reset button bit to SMPC.
}

void STVIO_UpdateInput(int32_t elapsed_time)
{
 memset(DataIn, 0xFF, sizeof(DataIn));

 if(0)
 {
  //memset(DataIn, 0x00, sizeof(DataIn));
  //DataIn[0x2] = 0xFF;
 }
 else if(ControlScheme == STV_CONTROL_HAMMER)
 {
  uint8_t tmp_data[2 + 2 + 1];
  int16_t nom_x, nom_y;
  int x, y;

  memset(tmp_data, 0, sizeof(tmp_data));

  if(DPtr[0])
  {
   memcpy(tmp_data, DPtr[0], 4);
   tmp_data[4] = DPtr[0][4] & 0x1;
  }

  nom_x = (int16_t)(uint16_t)(tmp_data[0] | (tmp_data[1] << 8));
  nom_y = (int16_t)(uint16_t)(tmp_data[2] | (tmp_data[3] << 8));
  x = ((nom_x * 193) + 32768) >> 16;
  y = ((nom_y + 7) * 49 + 128) >> 8; //55;

  //   printf("%d %d\n", nom_x, nom_y);

  if((tmp_data[4] & 0x01) && x >= (0 - 3) && x <= (62 + 3) && y >= (0 - 3) && y <= (46 + 3))
  {
   HammerX = ((int32_t)(62) < (int32_t)(((int32_t)(0) > (int32_t)(x) ? (int32_t)(0) : (int32_t)(x))) ? (int32_t)(62) : (int32_t)(((int32_t)(0) > (int32_t)(x) ? (int32_t)(0) : (int32_t)(x))));
   HammerY = ((int32_t)(46) < (int32_t)(((int32_t)(0) > (int32_t)(y) ? (int32_t)(0) : (int32_t)(y))) ? (int32_t)(46) : (int32_t)(((int32_t)(0) > (int32_t)(y) ? (int32_t)(0) : (int32_t)(y))));

   // HAMMERTIME:
   DataIn[0x2] &= ~0x10;
  }

  DataIn[0x0] = ((HammerX >> 4) & 0x3) | ((HammerX & 0x1) << 5) | ((HammerX & 0x2) << 3) | ((HammerX & 0x4) << 5) | ((HammerX & 0x8) << 3);
  DataIn[0x1] = ((HammerY >> 4) & 0x3) | ((HammerY & 0x1) << 5) | ((HammerY & 0x2) << 3) | ((HammerY & 0x4) << 5) | ((HammerY & 0x8) << 3);

  //
  //
  //
  gun->vt->UpdateInput(gun, tmp_data, elapsed_time);
 }
 else
 {
  for(unsigned i = 0; i < 2; i++)
  {
   uint16_t tmp = DPtr[i] ? (uint16_t)(DPtr[i][0] | (DPtr[i][1] << 8)) : 0;
   {
    // SW1, SW2, SW3:
    DataIn[i] ^= (((tmp & 0xA0) >> 1) | ((tmp & 0x50) << 1)) | ((tmp >> 10) & 0x01) | ((tmp >> 7) & 0x06);

    if(ControlScheme == STV_CONTROL_RSG)
    {
     if(tmp & 0x1)
      DataIn[i] &= ~0x3;

     if(tmp & 0x2)
      DataIn[i] &= ~0x5;

     if(tmp & 0x4)
      DataIn[i] &= ~0x6;

     if(tmp & 0x8)
      DataIn[i] &= ~0x7;
    }
    else
    {
     // SW4, SW5, SW6:
     DataIn[0x5] ^= (((tmp >> 2) & 0x01) | (tmp & 0x02) | ((tmp << 2) & 0x04)) << (i << 2);
    }
   }
   //
   if(i < 2)
   {
    // Start:
    DataIn[0x2] ^= (tmp & 0x0800) >> (7 - i);
   }
  }
 }

 // Test, Service:
 DataIn[0x2] ^= DPtr[12][0] & 0xC;

 // Pause
 DataIn[0x2] ^= (DPtr[12][0] & 0x10) << 3;

 CoinActiveCounter = ((int32_t)(-75000) > (int32_t)(CoinActiveCounter - elapsed_time) ? (int32_t)(-75000) : (int32_t)(CoinActiveCounter - elapsed_time));

 if(CoinPending && CoinActiveCounter == -75000)
 {
  CoinActiveCounter = 75000;
  CoinPending--;
 }

 // Coin(1P)
 DataIn[0x2] ^= (CoinActiveCounter > 0);

 // ?
 DataIn[0x3] = 0x00;
}

void STVIO_Reset(bool powering_up)
{
 if(powering_up)
 {
  AK93C45_Power(&eep);
  gun->vt->Power(gun);
 }
 //
 if(powering_up)
 {
  CoinPending = 0;
  CoinActiveCounter = 0;
 }

 DataDir = 0xFF;
 memset(DataOut, 0xFF, sizeof(DataOut));
}

/*
 EEPROM Notes:

 Offs,  Size:
  0x00, 4 bytes: 'SEGA'

  0x08, 2 bytes: CRC-16(poly 0x1021, MSB-first, initial=0x5A81 or prepend data with
		 "SEGA JIM!!!" for silliness), 0x0C ... 0x41, XOR CRC
		 value after calc with 16-bits at 0x42...0x43

  0x0F, 1 byte: EEPROM programming count? initially at 0x01

  0x1A, 2 byte: Settings
		 Bits 0...1: Cabinet type;
			1P=0x0
			2P=0x1 (Default)
			3P=0x2
			4P=0x3

		 Bit 5: Alone/Multi
			Multi=0	(Enables more fields in EEPROM, TODO)
			Alone=1 (Default)

		 Bit 6: Advertise sound
			Off=0
			On=1 (Default)

		 Bit 12: V/H switch
			Normal=0 (Default)
			Vertical=1

 0x1C: 2 byte: Game-ID?  Copied from 0xF40 in cart ROM space(*2 offset for 8-bit)

 0x1E, 8 byte: Game-specific settings?  Copied from 0xF48 in cart ROM space?
*/
static void InitEEPROM(const struct STVGameInfo* sgi)
{
 /* 4 KiB scratch buffer, lifetime entirely within this function --
  * was a std::unique_ptr<uint8_t[]> purely for RAII over new[]. A
  * plain stack array is simpler and allocation-free. */
 uint8_t rom_data[0x1000];
 unsigned cab_players = 2;
 uint16_t crc16, settings;
 uint8_t tmp[0x80];

 for(int i = 1; i >= 0; i--)
 {
  for(uint32_t offs = 0; offs < 0x1000; offs++)
   rom_data[offs] = CART_STV_PeekROM((offs << i) | i | (!i << 21));

  if(!memcmp(rom_data, "SEGA ST-V(TITAN)", 16))
   break;
  else if(!i)
  {
#ifdef MDFN_ENABLE_DEV_BUILD
   assert(0);
#endif
   return;
  }
 }

 // Upstream verifies a SHA-256 of the EEPROM-bound ROM region here
 // (rom_data[0x100 .. 0xDFF]) against a hard-coded digest of the
 // known-good BIOS area, returning silently if it doesn't match. The
 // check is only meaningful in MDFN_ENABLE_DEV_BUILD; this fork doesn't
 // ship the sha256_digest / _sha256 literal operator infrastructure
 // upstream uses, so drop the check. The ROM-magic memcmp above already
 // gates this code path on a recognized ROM header.

 // 0x01: 1P
 // 0x02: 1P or 2P
 // 0x03: 1P
 // 0x04: 2P
 // 0x05: 2P
 // 0x0C: 2P
 // 0x0F: 1P or 2P
 // 0x10: 3P
 // 0xFF: any
 switch(rom_data[0xF46])
 {
  case 0x01:
  case 0x03:
	cab_players = 1;
	break;

  case 0x10:
	cab_players = 3;
	break;
 }

 memset(tmp, 0xFF, sizeof(tmp));
 memcpy(tmp + 0x00, "SEGA", 4);

 assert(cab_players >= 1 && cab_players <= 4);
 settings = (sgi->rotate << 12) | (cab_players - 1) | (1U << 5) | (1U << 6) | 0x089C;

 tmp[0x0C] = 0x00; // ?
 tmp[0x0D] = 0x00; // ?
 tmp[0x0E] = 0x00; // ?
 tmp[0x0F] = 0x01 + (settings != 0x08FD);
 tmp[0x10] = 0x01; // ?
 tmp[0x11] = 0x00; // ?
 tmp[0x12] = 0x01; // ?
 tmp[0x13] = 0x01; // ?
 tmp[0x14] = 0x00; // ?
 tmp[0x15] = 0x00; // ?
 tmp[0x16] = 0x00; // ?
 tmp[0x17] = 0x00; // ?
 tmp[0x18] = 0x00; // ?
 tmp[0x19] = 0x08; // ?

 /* MDFN_en16msb folded: write host uint16_t as 2 BE bytes. */
 tmp[0x1A] = settings >> 8;
 tmp[0x1B] = settings;

 memcpy(tmp + 0x1C, rom_data + 0xF40, 0x2);
 memcpy(tmp + 0x1E, rom_data + 0xF48, 0x8);

 crc16 = crc16_ccitt(0x5A81, tmp + 0x0C, 0x38 - 0x02);
 /* MDFN_de16msb folded: 2 BE bytes -> host uint16_t.  This is the
  * one MSB_FIRST-aware fold in the file: result is the SAME on
  * either endian since the byte arithmetic is explicit. */
 crc16 ^= (uint16_t)((tmp[0x42] << 8) | tmp[0x43]);
 /* MDFN_en16msb folded again. */
 tmp[0x8] = crc16 >> 8;
 tmp[0x9] = crc16;

 memcpy(tmp + 0x44, tmp + 0x08, 0x3C);
 //
 for(unsigned addr = 0; addr < 0x40; addr++)
 {
  const uint8_t *bp__ = tmp + (addr << 1);
  AK93C45_PokeMem(&eep, addr, (uint16_t)((bp__[0] << 8) | bp__[1]));
 }
}

void STVIO_Init(const struct STVGameInfo* sgi)
{
 ControlScheme = sgi->control;

 AK93C45_Init(&eep);

 IODevice_Free(gun);
 gun = IODevice_Gun_Create();

 InitEEPROM(sgi);
 //
 prev_sctrl = 0xFF;	// Don't change
 prev_ectrl = 0x1C;
}

void STVIO_Kill(void)
{
 /* gun is heap-allocated in STVIO_Init via IODevice_Gun_Create.
  * Pre-conversion this had no Kill function, so gun was only ever
  * released by the next STVIO_Init's preceding IODevice_Free(gun) --
  * which left the buffer leaked on the final game close (no next
  * Init) and across any non-STV game load that followed an STV one.
  *
  * IODevice_Free is NULL-safe, so this is also safe to call from
  * ss.cpp's Cleanup() for non-STV games where STVIO_Init never ran
  * (gun is still NULL). */
 IODevice_Free(gun);
 gun = NULL;
}

void STVIO_LoadNV(cdstream* s)
{
 uint8_t tmp[0x80];

 cdstream_read(s, tmp, sizeof(tmp));

 for(unsigned addr = 0; addr < 0x40; addr++)
 {
  const uint8_t *bp__ = tmp + (addr << 1);
  AK93C45_PokeMem(&eep, addr, (uint16_t)((bp__[0] << 8) | bp__[1]));
 }
}

void STVIO_SaveNV(cdstream* s)
{
 uint8_t tmp[0x80];

 for(unsigned addr = 0; addr < 0x40; addr++)
 {
  uint8_t *bp__ = tmp + (addr << 1);
  uint16_t v__ = AK93C45_PeekMem(&eep, addr);
  bp__[0] = v__ >> 8;
  bp__[1] = v__;
 }

 cdstream_write(s, tmp, sizeof(tmp));
}

void STVIO_WriteIOGA(const int32_t timestamp, uint8_t A, uint8_t V)
{
 //printf("[IOGA] Write: %02x %02x\n", A, V);

 if(A < 0x8)
 {
  DataOut[A & 0x7] = V;
 }
 else if(A == 0x8)
  DataDir = V;

 // 0x03: outputs, D0->D7: 1p coin counter, 2p coin counter, 1p coin lock, 2p coin lock, reserved 1, reserved 2, reserved 3, reserved 4
 // 0x04: port E
 // 0x05: port F

 // 0x08: port input/output control?
 //	Port E output, Port F input: 0xEF
 //	Port E output, Port G input: 0xEF
 //	Port F output, Port E input: 0xDF
}

uint8_t STVIO_ReadIOGA(const int32_t timestamp, uint8_t A)
{
 uint8_t ret = 0xFF;

 //printf("[IOGA] Read: %02x\n", A);
 //assert(A <= 0x8);

 if(A == 0x8)
  ret = DataDir;
 else
 {
  const size_t offs = A & 0x7;

  ret = DataIn[offs];

  if(!(DataDir & (1U << (offs))))
   ret &= DataOut[offs];
 }

 return ret;
}

void STVIO_InsertCoin(void)
{
 CoinPending++;
}

void STVIO_StateAction(StateMem* sm, const unsigned load, const bool data_only)
{
 SFORMAT StateRegs[] =
 {
  SFVAR(DataDir),
  SFPTR8N(&(DataOut)[0], (sizeof(DataOut) / sizeof(uint8_t)), "DataOut"),
  SFPTR8N(&(DataIn)[0], (sizeof(DataIn) / sizeof(uint8_t)), "DataIn"),
  SFVAR(CoinPending),
  SFVAR(CoinActiveCounter),
  //
  SFVAR(HammerX),
  SFVAR(HammerY),
  //
  SFVAR(prev_sctrl),
  SFVAR(prev_ectrl),
  //
  SFEND
 };

 MDFNSS_StateAction(sm, load, data_only, StateRegs, "STV_IO", false);

 AK93C45_StateAction(&eep, sm, load, data_only, "STV_EEPROM");
}


/* The ST-V SMPC port shim. Was a template<bool sport> class deriving
   from IODevice; with IODevice now a C vtable struct, this becomes a
   plain struct embedding IODevice, with `sport` as a field rather
   than a template parameter. Two instances are constructed (sport 0
   and 1) and handed to the SMPC via SMPC_SetInput(..., "extern", ...).
   Only TransformInput / SetTSFreq / ResetTS / UpdateBus / Draw are
   overridden; the remaining vtable slots reuse IODevice_base_vtable's
   implementations. */

typedef struct
{
   IODevice base;
   bool     sport;
} IODevice_STVSMPC;

static void IODevice_STVSMPC_SetTSFreq(IODevice *self_, const int32_t rate)
{
   IODevice_STVSMPC *self = (IODevice_STVSMPC *)self_;
   if(!self->sport)
      AK93C45_SetTSFreq(&eep, rate);
}

static void IODevice_STVSMPC_ResetTS(IODevice *self_)
{
   IODevice_STVSMPC *self = (IODevice_STVSMPC *)self_;
   if(!self->sport)
      AK93C45_ResetTS(&eep);
}

static void IODevice_STVSMPC_TransformInput(IODevice *self_, uint8_t *data, float gun_x_scale, float gun_x_offs)
{
   IODevice_STVSMPC *self = (IODevice_STVSMPC *)self_;
   (void)data;
   if((ControlScheme == STV_CONTROL_HAMMER) && !self->sport && DPtr[0])
      gun->vt->TransformInput(gun, DPtr[0], gun_x_scale, gun_x_offs);
}

static void IODevice_STVSMPC_Draw(IODevice *self_, MDFN_Surface *surface, const MDFN_Rect *drect, const int32_t *lw, int ifield, float gun_x_scale, float gun_x_offs)
{
   IODevice_STVSMPC *self = (IODevice_STVSMPC *)self_;
   if((ControlScheme == STV_CONTROL_HAMMER) && !self->sport && DPtr[0])
      gun->vt->Draw(gun, surface, drect, lw, ifield, gun_x_scale, gun_x_offs);
}

static uint8_t IODevice_STVSMPC_UpdateBus(IODevice *self_, const int32_t timestamp, const uint8_t smpc_out, const uint8_t smpc_out_asserted)
{
   IODevice_STVSMPC *self = (IODevice_STVSMPC *)self_;
   uint8_t tmp = 0x7F;

   if(!self->sport)
   {
      const uint8_t cur_ectrl = smpc_out & 0x1C;

      AK93C45_UpdateBus(&eep, timestamp, (bool)(cur_ectrl & 0x04), (bool)(cur_ectrl & 0x08), (bool)(cur_ectrl & 0x10));

      prev_ectrl = cur_ectrl;

      tmp &= ~0x23;
   }
   else
   {
      const uint8_t cur_sctrl = smpc_out & 0x18;

      tmp = (tmp &~ 1) | AK93C45_UpdateBus(&eep, timestamp, (bool)(prev_ectrl & 0x04), (bool)(prev_ectrl & 0x08), (bool)(prev_ectrl & 0x10));

      if(prev_sctrl != cur_sctrl)	// Be careful with prev_sctrl init value if changing this code.
      {
         if((prev_sctrl ^ cur_sctrl) & 0x10)
            SOUND_Reset68K();

         if((prev_sctrl ^ cur_sctrl) & 0x08)
            SOUND_ResetSCSP();

         SOUND_Set68KActive(cur_sctrl == 0x00); // FIXME: probably not totally correct.
      }

      prev_sctrl = cur_sctrl;
   }

   if((ControlScheme == STV_CONTROL_HAMMER) && !self->sport)
   {
      tmp &= gun->vt->UpdateBus(gun, timestamp, smpc_out, smpc_out_asserted) | ~0x40;
   }

   return (smpc_out & smpc_out_asserted) | (tmp &~ smpc_out_asserted);
}

/* Built once, lazily, on first STVIO_GetSMPCDevice call: copy the
   base vtable, override the five ST-V slots. */
static IODevice_VTable IODevice_STVSMPC_vtable;
static bool            IODevice_STVSMPC_vtable_built = false;

IODevice* STVIO_GetSMPCDevice(bool sport)
{
   static IODevice_STVSMPC p0;
   static IODevice_STVSMPC p1;

   if(!IODevice_STVSMPC_vtable_built)
   {
      IODevice_STVSMPC_vtable                = IODevice_base_vtable;
      IODevice_STVSMPC_vtable.TransformInput = IODevice_STVSMPC_TransformInput;
      IODevice_STVSMPC_vtable.SetTSFreq      = IODevice_STVSMPC_SetTSFreq;
      IODevice_STVSMPC_vtable.ResetTS        = IODevice_STVSMPC_ResetTS;
      IODevice_STVSMPC_vtable.UpdateBus      = IODevice_STVSMPC_UpdateBus;
      IODevice_STVSMPC_vtable.Draw           = IODevice_STVSMPC_Draw;

      IODevice_Ctor(&p0.base);
      p0.base.vt = &IODevice_STVSMPC_vtable;
      p0.sport   = false;

      IODevice_Ctor(&p1.base);
      p1.base.vt = &IODevice_STVSMPC_vtable;
      p1.sport   = true;

      IODevice_STVSMPC_vtable_built = true;
   }

   return sport ? &p1.base : &p0.base;
}

