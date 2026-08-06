/******************************************************************************/
/* Mednafen Sega Saturn Emulation Module                                      */
/******************************************************************************/
/* mpeg.c - Video CD Card (MPEG card) emulation
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
   STAGE 1 (this file): host-visible control plane.  The CD block
   command surface, the card's mode/connection/stream/display state, the
   interrupt register and mask, authentication, savestates, and the
   elementary-stream FIFOs.  Enough for software to probe for the card,
   authenticate it, configure it and drive it without hanging.

   STAGE 2 (not implemented): MPEG-1 video and MPEG-1 Layer II audio
   elementary stream decoders, fed by MPEG_FeedSector() and clocked by
   MPEG_RunFrame().

   STAGE 3 (not implemented): VDP2 external-background compositing of
   the decoded picture, and SCSP-side mixing of the decoded audio.

   Deliberately NOT modelled as a CartInfo -- see the header comment.
*/

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <boolean.h>

#include <streams/file_stream.h>

#include "../state.h"
#include "../mednafen-types.h"

#include "mpeg.h"

/* The Sega/Victor cards carry a 512 KiB mask ROM holding the Video CD
   player program.  Accept anything up to that; short images are zero
   padded, longer ones are truncated with the tail ignored. */
#define MPEG_ROM_SIZE 0x80000

/* Elementary stream FIFOs.  The card has 2 MiB of DRAM split between
   the two streams and the frame buffers; the split is programmable via
   the buffer-number fields of SET_CONNECTION, which stage 1 records but
   does not act on.  Until the decoders land these are fixed-size ring
   buffers sized generously enough that a stalled decoder drops sectors
   instead of stalling the CD block. */
#define MPEG_VFIFO_SIZE (256 * 1024)
#define MPEG_AFIFO_SIZE (64  * 1024)

typedef struct
{
 uint8_t  audcon;
 uint8_t  audlay;
 uint8_t  audbufnum;
 uint8_t  vidcon;
 uint8_t  vidlay;
 uint8_t  vidbufnum;
} MPEG_Connection;

typedef struct
{
 uint8_t  audstm;
 uint8_t  audstmid;
 uint8_t  audchannum;
 uint8_t  vidstm;
 uint8_t  vidstmid;
 uint8_t  vidchannum;
} MPEG_Stream;

typedef struct
{
 uint8_t  vidplaymode;
 uint8_t  dectimingmode;
 uint8_t  outmode;
 uint8_t  slmode;
} MPEG_Mode;

typedef struct
{
 uint8_t *data;
 uint32_t size;
 uint32_t rp;
 uint32_t wp;
 uint32_t count;
} MPEG_FIFO;

/*
 *
 * State
 *
 */

static bool     Present;
static uint8_t *CardROM;
static uint32_t CardROMSize;

static uint8_t  AuthState;      /* 0 = unauthenticated, 2 = authenticated */

/* MPEG interrupt register.  24 bits wide: CR1's low byte carries bits
   23..16 and CR2 carries bits 15..0.  The individual bit assignments
   are not documented in any source I can verify against, so nothing
   sets them yet -- GET_INTERRUPT returns a masked zero, which is what
   idle hardware would report.  Filling this in is a stage 2 job, driven
   by the decoders. */
static uint32_t IntFlags;
static uint32_t IntMask;

static MPEG_Mode        Mode;
static MPEG_Connection  Con[2];   /* [0] = current, [1] = next */
static MPEG_Stream      Stm[2];   /* [0] = current, [1] = next */

/* Fields the card echoes back in every MPEG status report. */
static uint8_t  ActionStatus;
static uint16_t VCounter;
static uint8_t  PictureInfo;
static uint8_t  AudioStatus;
static uint16_t VideoStatus;

static uint8_t  DecodeMethod;
static uint8_t  VideoEffects[4];
static uint32_t LSIRegs[2];

static MPEG_DisplayState Display;

static MPEG_FIFO VFIFO;
static MPEG_FIFO AFIFO;

/* Decoded picture, RGB555.  Untouched by stage 1. */
static uint16_t *FrameBuf;
static uint32_t  FrameW, FrameH;
static bool      FrameValid;

/*
 *
 * FIFO helpers
 *
 */

static MDFN_COLD bool FIFO_Alloc(MPEG_FIFO *f, uint32_t size)
{
   f->data = (uint8_t*)malloc(size);
   if(!f->data)
      return false;

   f->size  = size;
   f->rp    = 0;
   f->wp    = 0;
   f->count = 0;
   memset(f->data, 0, size);

   return true;
}

static MDFN_COLD void FIFO_Free(MPEG_FIFO *f)
{
   if(f->data)
      free(f->data);
   f->data  = NULL;
   f->size  = 0;
   f->rp    = 0;
   f->wp    = 0;
   f->count = 0;
}

static MDFN_COLD void FIFO_Clear(MPEG_FIFO *f)
{
   f->rp    = 0;
   f->wp    = 0;
   f->count = 0;
}

/* Writes as much as will fit and drops the remainder.  A real card
   would apply backpressure through the CD block's buffer-full path;
   until the decoders exist there is nothing draining these, so dropping
   is the only option that does not deadlock. */
static MDFN_HOT void FIFO_Write(MPEG_FIFO *f, const uint8_t *src, uint32_t len)
{
   uint32_t space;

   if(!f->data)
      return;

   space = f->size - f->count;
   if(len > space)
      len = space;

   while(len)
   {
      uint32_t chunk = f->size - f->wp;

      if(chunk > len)
         chunk = len;

      memcpy(f->data + f->wp, src, chunk);

      f->wp     = (f->wp + chunk) % f->size;
      f->count += chunk;
      src      += chunk;
      len      -= chunk;
   }
}

/*
 *
 * Lifecycle
 *
 */

bool MPEG_Init(RFILE *rom_fp)
{
   /* Defensive: CDB_Init() can run again on a second content load
      without an intervening CDB_Kill().  Without this the ROM and both
      FIFOs would be silently leaked. */
   MPEG_Kill();

   Present     = true;
   CardROM     = NULL;
   CardROMSize = 0;
   FrameBuf    = NULL;

   memset(&VFIFO, 0, sizeof(VFIFO));
   memset(&AFIFO, 0, sizeof(AFIFO));

   if(rom_fp)
   {
      int64_t len;

      filestream_seek(rom_fp, 0, RETRO_VFS_SEEK_POSITION_END);
      len = filestream_tell(rom_fp);
      filestream_seek(rom_fp, 0, RETRO_VFS_SEEK_POSITION_START);

      if(len > 0)
      {
         if(len > MPEG_ROM_SIZE)
            len = MPEG_ROM_SIZE;

         CardROM = (uint8_t*)malloc(MPEG_ROM_SIZE);
         if(!CardROM)
            goto fail;

         memset(CardROM, 0, MPEG_ROM_SIZE);

         if(filestream_read(rom_fp, CardROM, (int64_t)len) != len)
         {
            free(CardROM);
            CardROM = NULL;
         }
         else
            CardROMSize = MPEG_ROM_SIZE;
      }
   }

   if(!FIFO_Alloc(&VFIFO, MPEG_VFIFO_SIZE))
      goto fail;

   if(!FIFO_Alloc(&AFIFO, MPEG_AFIFO_SIZE))
      goto fail;

   FrameBuf = (uint16_t*)malloc(MPEG_MAX_WIDTH * MPEG_MAX_HEIGHT * sizeof(uint16_t));
   if(!FrameBuf)
      goto fail;

   memset(FrameBuf, 0, MPEG_MAX_WIDTH * MPEG_MAX_HEIGHT * sizeof(uint16_t));

   MPEG_Reset(true);

   return true;

fail:
   MPEG_Kill();
   return false;
}

void MPEG_Kill(void)
{
   FIFO_Free(&VFIFO);
   FIFO_Free(&AFIFO);

   if(CardROM)
      free(CardROM);
   CardROM     = NULL;
   CardROMSize = 0;

   if(FrameBuf)
      free(FrameBuf);
   FrameBuf = NULL;

   Present = false;
}

void MPEG_Reset(bool powering_up)
{
   unsigned i;

   AuthState    = 0;

   IntFlags     = 0;
   IntMask      = 0;

   Mode.vidplaymode   = 0;
   Mode.dectimingmode = 0;
   Mode.outmode       = 0;
   Mode.slmode        = 0;

   for(i = 0; i < 2; i++)
   {
      Con[i].audcon    = 0x00;
      Con[i].audlay    = 0x00;
      Con[i].audbufnum = 0xFF;
      Con[i].vidcon    = 0x00;
      Con[i].vidlay    = 0x00;
      Con[i].vidbufnum = 0xFF;

      Stm[i].audstm     = 0x00;
      Stm[i].audstmid   = 0x00;
      Stm[i].audchannum = 0x00;
      Stm[i].vidstm     = 0x00;
      Stm[i].vidstmid   = 0x00;
      Stm[i].vidchannum = 0x00;
   }

   ActionStatus = 0;
   VCounter     = 0;
   PictureInfo  = 0;
   AudioStatus  = 0;
   VideoStatus  = 0;

   DecodeMethod = 0;

   memset(VideoEffects, 0, sizeof(VideoEffects));
   memset(LSIRegs, 0, sizeof(LSIRegs));

   memset(&Display, 0, sizeof(Display));
   Display.w = MPEG_MAX_WIDTH;
   Display.h = 240;

   FIFO_Clear(&VFIFO);
   FIFO_Clear(&AFIFO);

   FrameW     = 0;
   FrameH     = 0;
   FrameValid = false;

   if(FrameBuf && powering_up)
      memset(FrameBuf, 0, MPEG_MAX_WIDTH * MPEG_MAX_HEIGHT * sizeof(uint16_t));
}

bool MPEG_IsPresent(void)
{
   return Present;
}

const uint8_t *MPEG_GetROM(uint32_t *size)
{
   if(size)
      *size = CardROMSize;
   return CardROM;
}

/*
 *
 * Authentication
 *
 */

void MPEG_Auth(void)
{
   /* The card authenticates only if its firmware ROM is present.  A
      card with no ROM is exactly the case a user hits when they enable
      the option without supplying the firmware, and reporting success
      there would send the BIOS off to run a player that is not
      there. */
   AuthState = CardROM ? 2 : 0;
}

uint16_t MPEG_GetAuth(void)
{
   return AuthState;
}

/*
 *
 * Command dispatch
 *
 */

static void MPEGReport(uint8_t base_status, uint16_t out[4])
{
   out[0] = ((uint16_t)base_status << 8) | ActionStatus;
   out[1] = VCounter;
   out[2] = ((uint16_t)PictureInfo << 8) | AudioStatus;
   out[3] = VideoStatus;
}

bool MPEG_Command(uint8_t cmd, const uint16_t cd[4],
                  uint8_t base_status, uint16_t out[4], uint16_t *hirq)
{
   unsigned which;

   if(!MPEG_CMD_IS_MPEG(cmd))
      return false;

   /* With no card installed every opcode in this range is rejected,
      which is how software probes for it. */
   if(!Present)
   {
      out[0] = 0xFF00;
      out[1] = 0xFFFF;
      out[2] = 0xFFFF;
      out[3] = 0xFFFF;
      *hirq |= MPEG_HIRQ_MPCM;
      return true;
   }

   switch(cmd)
   {
      case MPEG_CMD_GET_STATUS:
         MPEGReport(base_status, out);
         *hirq |= MPEG_HIRQ_MPCM;
         break;

      case MPEG_CMD_GET_INTERRUPT:
      {
         const uint32_t pending = IntFlags & IntMask;

         out[0] = ((uint16_t)base_status << 8) | ((pending >> 16) & 0xFF);
         out[1] = (uint16_t)pending;
         out[2] = 0;
         out[3] = 0;

         /* Read-to-clear: the host has consumed these. */
         IntFlags &= ~pending;

         *hirq |= MPEG_HIRQ_MPCM;
      }
      break;

      case MPEG_CMD_SET_INT_MASK:
         IntMask = ((uint32_t)(cd[0] & 0xFF) << 16) | cd[1];
         MPEGReport(base_status, out);
         *hirq |= MPEG_HIRQ_MPCM;
         break;

      case MPEG_CMD_INIT:
         if(AuthState)
            out[0] = (uint16_t)base_status << 8;
         else
            out[0] = 0xFF00;

         out[1] = 0;
         out[2] = 0;
         out[3] = 0;

         IntFlags = 0;

         FIFO_Clear(&VFIFO);
         FIFO_Clear(&AFIFO);

         ActionStatus = 0;
         PictureInfo  = 0;
         AudioStatus  = 0;
         VideoStatus  = 0;
         VCounter     = 0;
         FrameValid   = false;

         /* CR2 bit 0 selects the soft-reset variant, which additionally
            completes the command handshake immediately. */
         *hirq |= MPEG_HIRQ_MPED | MPEG_HIRQ_MPST;
         if(cd[1] & 0x0001)
            *hirq |= MPEG_HIRQ_MPCM;
         break;

      case MPEG_CMD_SET_MODE:
      {
         const uint8_t vidplaymode   = cd[0] & 0xFF;
         const uint8_t dectimingmode = cd[1] >> 8;
         const uint8_t outmode       = cd[1] & 0xFF;
         const uint8_t slmode        = cd[2] >> 8;

         /* 0xFF in a field means "leave alone". */
         if(vidplaymode   != 0xFF) Mode.vidplaymode   = vidplaymode;
         if(dectimingmode != 0xFF) Mode.dectimingmode = dectimingmode;
         if(outmode       != 0xFF) Mode.outmode       = outmode;
         if(slmode        != 0xFF) Mode.slmode        = slmode;

         MPEGReport(base_status, out);
         *hirq |= MPEG_HIRQ_MPCM;
      }
      break;

      case MPEG_CMD_PLAY:
         /* STAGE 2: kick the decoders from the play mode in CR1/CR2 and
            the start offset in CR3/CR4. */
         ActionStatus = 0;
         MPEGReport(base_status, out);
         *hirq |= MPEG_HIRQ_MPCM;
         break;

      case MPEG_CMD_SET_DECMETHOD:
         DecodeMethod = cd[0] & 0xFF;
         MPEGReport(base_status, out);
         *hirq |= MPEG_HIRQ_MPCM;
         break;

      case MPEG_CMD_SET_CONNECTION:
         which = (cd[2] >> 8) ? 1 : 0;

         Con[which].audcon    = cd[0] & 0xFF;
         Con[which].audlay    = cd[1] >> 8;
         Con[which].audbufnum = cd[1] & 0xFF;
         Con[which].vidcon    = cd[2] & 0xFF;
         Con[which].vidlay    = cd[3] >> 8;
         Con[which].vidbufnum = cd[3] & 0xFF;

         MPEGReport(base_status, out);
         *hirq |= MPEG_HIRQ_MPCM;
         break;

      case MPEG_CMD_GET_CONNECTION:
         which = (cd[2] >> 8) ? 1 : 0;

         out[0] = ((uint16_t)base_status << 8) | Con[which].audcon;
         out[1] = ((uint16_t)Con[which].audlay << 8) | Con[which].audbufnum;
         out[2] = Con[which].vidcon;
         out[3] = ((uint16_t)Con[which].vidlay << 8) | Con[which].vidbufnum;

         *hirq |= MPEG_HIRQ_MPCM;
         break;

      case MPEG_CMD_SET_STREAM:
         which = (cd[2] >> 8) ? 1 : 0;

         Stm[which].audstm     = cd[0] & 0xFF;
         Stm[which].audstmid   = cd[1] >> 8;
         Stm[which].audchannum = cd[1] & 0xFF;
         Stm[which].vidstm     = cd[2] & 0xFF;
         Stm[which].vidstmid   = cd[3] >> 8;
         Stm[which].vidchannum = cd[3] & 0xFF;

         MPEGReport(base_status, out);
         *hirq |= MPEG_HIRQ_MPCM;
         break;

      case MPEG_CMD_GET_STREAM:
         which = (cd[2] >> 8) ? 1 : 0;

         out[0] = ((uint16_t)base_status << 8) | Stm[which].audstm;
         out[1] = ((uint16_t)Stm[which].audstmid << 8) | Stm[which].audchannum;
         out[2] = Stm[which].vidstm;
         out[3] = ((uint16_t)Stm[which].vidstmid << 8) | Stm[which].vidchannum;

         *hirq |= MPEG_HIRQ_MPCM;
         break;

      case MPEG_CMD_DISPLAY:
         Display.enabled = (cd[0] & 0x0001) != 0;
         MPEGReport(base_status, out);
         *hirq |= MPEG_HIRQ_MPCM;
         break;

      case MPEG_CMD_SET_WINDOW:
         Display.src_x = cd[0] & 0x03FF;
         Display.src_y = cd[1] & 0x03FF;
         Display.x     = cd[2] & 0x03FF;
         Display.y     = cd[3] & 0x03FF;

         MPEGReport(base_status, out);
         *hirq |= MPEG_HIRQ_MPCM;
         break;

      case MPEG_CMD_SET_BORDERCOL:
         Display.border_color = cd[1];
         MPEGReport(base_status, out);
         *hirq |= MPEG_HIRQ_MPCM;
         break;

      case MPEG_CMD_SET_FADE:
         Display.fade = cd[1] & 0xFF;
         MPEGReport(base_status, out);
         *hirq |= MPEG_HIRQ_MPCM;
         break;

      case MPEG_CMD_SET_VIDEOEFF:
         VideoEffects[0] = cd[0] & 0xFF;
         VideoEffects[1] = cd[1] & 0xFF;
         VideoEffects[2] = cd[2] & 0xFF;
         VideoEffects[3] = cd[3] & 0xFF;

         MPEGReport(base_status, out);
         *hirq |= MPEG_HIRQ_MPCM;
         break;

      case MPEG_CMD_SET_LSI:
         LSIRegs[0] = ((uint32_t)cd[0] << 16) | cd[1];
         LSIRegs[1] = ((uint32_t)cd[2] << 16) | cd[3];

         MPEGReport(base_status, out);
         *hirq |= MPEG_HIRQ_MPCM;
         break;

      default:
         /* Opcode in the MPEG range that this model does not implement.
            Answer with a plain status report instead of rejecting: the
            caller is far more likely to survive a benign no-op than a
            0xFF status it did not expect. */
         MPEGReport(base_status, out);
         *hirq |= MPEG_HIRQ_MPCM;
         break;
   }

   return true;
}

/*
 *
 * Data path
 *
 */

void MPEG_FeedSector(const uint8_t *data, uint32_t len, bool is_video)
{
   if(!Present || !data || !len)
      return;

   FIFO_Write(is_video ? &VFIFO : &AFIFO, data, len);
}

bool MPEG_RunFrame(void)
{
   /* STAGE 2: parse the video elementary stream out of VFIFO, decode
      one picture into FrameBuf, decode the matching Layer II audio out
      of AFIFO, update PictureInfo/VideoStatus/AudioStatus/VCounter and
      raise the corresponding IntFlags bits. */
   return false;
}

const uint16_t *MPEG_GetFrame(uint32_t *width, uint32_t *height)
{
   if(!FrameValid)
   {
      if(width)  *width  = 0;
      if(height) *height = 0;
      return NULL;
   }

   if(width)  *width  = FrameW;
   if(height) *height = FrameH;

   return FrameBuf;
}

const MPEG_DisplayState *MPEG_GetDisplayState(void)
{
   return &Display;
}

/*
 *
 * Savestates
 *
 */

void MPEG_StateAction(StateMem *sm, const unsigned load, const bool data_only)
{
   SFORMAT StateRegs[] =
   {
      SFVAR(Present),
      SFVAR(AuthState),

      SFVAR(IntFlags),
      SFVAR(IntMask),

      SFVAR(Mode.vidplaymode),
      SFVAR(Mode.dectimingmode),
      SFVAR(Mode.outmode),
      SFVAR(Mode.slmode),

      SFVAR(Con->audcon,    2, sizeof(*Con), Con),
      SFVAR(Con->audlay,    2, sizeof(*Con), Con),
      SFVAR(Con->audbufnum, 2, sizeof(*Con), Con),
      SFVAR(Con->vidcon,    2, sizeof(*Con), Con),
      SFVAR(Con->vidlay,    2, sizeof(*Con), Con),
      SFVAR(Con->vidbufnum, 2, sizeof(*Con), Con),

      SFVAR(Stm->audstm,     2, sizeof(*Stm), Stm),
      SFVAR(Stm->audstmid,   2, sizeof(*Stm), Stm),
      SFVAR(Stm->audchannum, 2, sizeof(*Stm), Stm),
      SFVAR(Stm->vidstm,     2, sizeof(*Stm), Stm),
      SFVAR(Stm->vidstmid,   2, sizeof(*Stm), Stm),
      SFVAR(Stm->vidchannum, 2, sizeof(*Stm), Stm),

      SFVAR(ActionStatus),
      SFVAR(VCounter),
      SFVAR(PictureInfo),
      SFVAR(AudioStatus),
      SFVAR(VideoStatus),

      SFVAR(DecodeMethod),
      SFPTR8N(VideoEffects, sizeof(VideoEffects), "VideoEffects"),
      SFVAR(LSIRegs[0]),
      SFVAR(LSIRegs[1]),

      SFVAR(Display.enabled),
      SFVAR(Display.x),
      SFVAR(Display.y),
      SFVAR(Display.w),
      SFVAR(Display.h),
      SFVAR(Display.src_x),
      SFVAR(Display.src_y),
      SFVAR(Display.border_color),
      SFVAR(Display.fade),

      SFVAR(VFIFO.rp),
      SFVAR(VFIFO.wp),
      SFVAR(VFIFO.count),
      SFVAR(AFIFO.rp),
      SFVAR(AFIFO.wp),
      SFVAR(AFIFO.count),

      SFEND
   };

   MDFNSS_StateAction(sm, load, data_only, StateRegs, "MPEG", true);

   /* FIFO contents are intentionally not serialised: they are pure
      decoder input, sized in the hundreds of KiB, and stage 1 never
      drains them.  Clamp the indices on load so a state written by a
      future build with different FIFO sizes cannot walk off the end. */
   if(load)
   {
      if(VFIFO.size)
      {
         VFIFO.rp    %= VFIFO.size;
         VFIFO.wp    %= VFIFO.size;
         if(VFIFO.count > VFIFO.size)
            VFIFO.count = VFIFO.size;
      }
      else
         FIFO_Clear(&VFIFO);

      if(AFIFO.size)
      {
         AFIFO.rp    %= AFIFO.size;
         AFIFO.wp    %= AFIFO.size;
         if(AFIFO.count > AFIFO.size)
            AFIFO.count = AFIFO.size;
      }
      else
         FIFO_Clear(&AFIFO);

      FrameValid = false;
   }
}
