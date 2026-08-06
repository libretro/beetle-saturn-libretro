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

   STAGE 2 (this file): the decode path, entirely on libretro-common
   codecs.  rmpeg1_ps demultiplexes the Program Stream that
   MPEG_FeedSector() receives; the selected video substream goes to
   rmpeg1_video and the selected audio substream to rmp3, both clocked
   by MPEG_RunFrame().  What lives here is glue: substream selection,
   4:2:0 to RGB555 conversion for the Saturn's display path, an audio
   ring, and the status fields the CD block command surface reports.

   STAGE 3 (not implemented): VDP2 external-background compositing of
   the decoded picture, and SCSP-side mixing of the decoded audio.

   Deliberately NOT modelled as a CartInfo -- see the header comment.
*/

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <boolean.h>
#include <retro_inline.h>

#include <streams/file_stream.h>
#include <formats/rmpeg1_ps.h>
#include <formats/rmpeg1_video.h>
#include <formats/rmp3.h>

#include "../state.h"
#include "../mednafen-types.h"

#include "mpeg.h"

/* The Sega/Victor cards carry a 512 KiB mask ROM holding the Video CD
   player program.  Accept anything up to that; short images are zero
   padded, longer ones are truncated with the tail ignored. */
#define MPEG_ROM_SIZE 0x80000

/* Elementary stream FIFOs, downstream of the demuxer.  The card has
   2 MiB of DRAM split between the two streams and the frame buffers;
   the split is programmable via the buffer-number fields of
   SET_CONNECTION, which is recorded but not yet acted on.  Until the
   decoders land these are fixed-size ring buffers sized generously
   enough that a stalled decoder drops elementary stream instead of
   stalling the CD block. */
#define MPEG_VFIFO_SIZE (256 * 1024)
#define MPEG_AFIFO_SIZE (64  * 1024)

/* Demuxer input buffer.  rmpeg1_ps requires at least the 65541-byte
   worst-case packet; give it room for a few sectors beyond that so a
   single MPEG_FeedSector() call never has to be split. */
#define MPEG_DEMUX_CAPACITY (96 * 1024)

/* Decoded audio ring, in interleaved stereo sample pairs.  rmp3 emits
   1152 frames at a time and the SCSP drains at its own cadence, so this
   only has to absorb the mismatch: half a second is generous. */
#define MPEG_ARING_FRAMES 22050

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
   23..16 and CR2 carries bits 15..0.  Bit assignments are SBL 6.01's
   CDC_MPINT_*, mirrored into mpeg.h. */
static uint32_t IntFlags;
static uint32_t IntMask;

/* HIRQ bits owed to the CD block, drained by MPEG_TakePendingHIRQ(). */
static uint16_t PendingHIRQ;

/* Edge detectors, so a level condition raises its factor once rather
   than on every decode. */
static bool SeqSeen;
static bool VideoStarted;
static bool AudioStarted;
static bool VideoReady;
static bool AudioReady;
static uint32_t LastVideoErrors;

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

/* temporal_reference of the last decoded picture, reported by
   GET_TIMECODE alongside the picture type. */
static uint8_t  TemporalRef;
static uint8_t  VideoEffects[6];
static uint32_t LSIRegs[2];

static MPEG_DisplayState Display;

static MPEG_FIFO VFIFO;
static MPEG_FIFO AFIFO;

/* System layer.  Owned here rather than by the CD block: on real
   hardware the CD block hands the card whole sectors and the card does
   its own demultiplexing. */
static rmpeg1_ps_t   *Demux;
static rmpeg1_video_t *Video;
static rmp3_stream_t  *Audio;

/* Decoded audio ring.  Stereo s16 pairs; mono streams are duplicated on
   the way in so the drain side never has to care. */
static int16_t  *ARing;
static uint32_t  ARing_RP, ARing_WP, ARing_Count;
static uint32_t  AudioRate, AudioChannels;

/* Scratch for one rmp3 frame before it is folded into the ring. */
static int16_t   APCM[RMP3_MAX_SAMPLES_PER_FRAME];

/* SCSP-side pull state.  ARatePhase is 16.16; one output sample
   consumes ARateStep input samples.  At the Video CD's 44.1 kHz that is
   exactly 1.0 and the accumulator degenerates to a straight pull. */
#define MPEG_SCSP_RATE 44100

static uint32_t ARatePhase;
static uint32_t ARateStep = 1U << 16;
static int16_t  ACur[2];
static bool     ACurValid;

static uint64_t VideoPTS;
static uint64_t AudioPTS;
static uint64_t LastSCR;

/* Dropped-payload counters.  With the decoders connected and clocked
   these should stay at zero: measured over a full 1.5s NTSC Video CD
   fed at the real 75-sectors-per-second cadence, nothing is dropped on
   either stream, so a nonzero value here means something is wedged
   rather than merely busy.  That is why there is no backpressure path
   into the CD block -- the condition it would handle does not arise. */
static uint32_t VDropped;
static uint32_t ADropped;

/* Set once SET_WINDOW supplies an explicit size, after which the
   decoded picture no longer drives the window geometry. */
static bool WindowSizeSet;

/* Decode clock.  Accumulates the 32.32 fixed-point CD block clocks
   MPEG_Update() is handed and fires one decode per frame period.  The
   period follows the sequence header once one has been parsed; until
   then it runs at NTSC rate, which is what a Video CD without a
   readable header would have been anyway. */
#define MPEG_CDB_CLOCK 11289600
#define MPEG_CATCHUP_FRAMES 4

static int64_t  FrameAccum;
static uint32_t FrameRateNum = 30000;
static uint32_t FrameRateDen = 1001;

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

/* Copy up to len bytes from the front without consuming them.  The
   decoders take a contiguous window, and a ring's front may wrap, so
   the caller stages into a flat buffer and reports back how much was
   actually consumed via FIFO_Drop(). */
static MDFN_HOT uint32_t FIFO_Peek(const MPEG_FIFO *f, uint8_t *dst, uint32_t len)
{
   uint32_t rp  = f->rp;
   uint32_t got = 0;

   if(!f->data)
      return 0;

   if(len > f->count)
      len = f->count;

   while(len)
   {
      uint32_t chunk = f->size - rp;

      if(chunk > len)
         chunk = len;

      memcpy(dst + got, f->data + rp, chunk);

      rp   = (rp + chunk) % f->size;
      got += chunk;
      len -= chunk;
   }

   return got;
}

static MDFN_HOT void FIFO_Drop(MPEG_FIFO *f, uint32_t len)
{
   if(!f->data)
      return;

   if(len > f->count)
      len = f->count;

   f->rp     = (f->rp + len) % f->size;
   f->count -= len;
}

/* Writes as much as will fit and drops the remainder.  A real card
   would apply backpressure through the CD block's buffer-full path;
   until the decoders exist there is nothing draining these, so dropping
   is the only option that does not deadlock. */
static MDFN_HOT uint32_t FIFO_Write(MPEG_FIFO *f, const uint8_t *src, uint32_t len)
{
   uint32_t space;
   uint32_t dropped = 0;

   if(!f->data)
      return len;

   space = f->size - f->count;
   if(len > space)
   {
      dropped = len - space;
      len     = space;
   }

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

   return dropped;
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
   Demux       = NULL;
   Video       = NULL;
   Audio       = NULL;
   ARing       = NULL;

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

   Demux = rmpeg1_ps_init(MPEG_DEMUX_CAPACITY);
   if(!Demux)
      goto fail;

   Video = rmpeg1_video_init();
   if(!Video)
      goto fail;

   Audio = rmp3_stream_new();
   if(!Audio)
      goto fail;

   ARing = (int16_t*)malloc(MPEG_ARING_FRAMES * 2 * sizeof(int16_t));
   if(!ARing)
      goto fail;

   memset(ARing, 0, MPEG_ARING_FRAMES * 2 * sizeof(int16_t));

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
   if(Demux)
      rmpeg1_ps_free(Demux);
   Demux = NULL;

   if(Video)
      rmpeg1_video_free(Video);
   Video = NULL;

   if(Audio)
      rmp3_stream_free(Audio);
   Audio = NULL;

   if(ARing)
      free(ARing);
   ARing = NULL;

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
   PendingHIRQ  = 0;

   SeqSeen         = false;
   VideoStarted    = false;
   AudioStarted    = false;
   VideoReady      = false;
   AudioReady      = false;
   LastVideoErrors = 0;

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

   ActionStatus = MPEG_ASTV_STOP | MPEG_ASTD_STOP | MPEG_ASTA_STOP;
   VCounter     = 0;
   PictureInfo  = 0;
   AudioStatus  = MPEG_STA_BEMPTY;
   VideoStatus  = MPEG_STV_BEMPTY;

   DecodeMethod = 0;
   TemporalRef  = 0;

   memset(VideoEffects, 0, sizeof(VideoEffects));
   memset(LSIRegs, 0, sizeof(LSIRegs));

   memset(&Display, 0, sizeof(Display));
   Display.w = MPEG_MAX_WIDTH;
   Display.h = 240;

   FIFO_Clear(&VFIFO);
   FIFO_Clear(&AFIFO);

   if(Demux)
      rmpeg1_ps_reset(Demux);

   if(Video)
      rmpeg1_video_reset(Video);

   /* rmp3's streaming context has no reset entry point, so recycle it.
      Failure leaves Audio NULL, which RunAudio() already tolerates --
      video keeps working without sound rather than the card dying. */
   if(Audio)
   {
      rmp3_stream_free(Audio);
      Audio = rmp3_stream_new();
   }

   ARing_RP      = 0;
   ARing_WP      = 0;
   ARing_Count   = 0;
   AudioRate     = 0;
   AudioChannels = 0;
   ARatePhase    = 0;
   ARateStep     = 1U << 16;
   ACur[0]       = 0;
   ACur[1]       = 0;
   ACurValid     = false;

   WindowSizeSet = false;
   FrameAccum    = 0;
   FrameRateNum = 30000;
   FrameRateDen = 1001;

   VideoPTS = RMPEG1_PS_NO_PTS;
   AudioPTS = RMPEG1_PS_NO_PTS;
   LastSCR  = RMPEG1_PS_NO_PTS;
   VDropped = 0;
   ADropped = 0;

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

/*
   Raise interrupt factors.

   HIRQ_MPST is what tells the host to go and read GET_INTERRUPT -- SBL
   names it "notification of MPEG interrupt status" -- so it follows the
   masked pending set, not the raw one.  Without this a title that waits
   on a decode-complete interrupt hangs at exactly the point where
   everything else is working, which is why it is raised here rather
   than anywhere convenient.
*/
static void RaiseInt(uint32_t bits)
{
   const uint32_t before = IntFlags & IntMask;

   IntFlags |= bits;

   if((IntFlags & IntMask) != before)
      PendingHIRQ |= MPEG_HIRQ_MPST;
}

uint16_t MPEG_TakePendingHIRQ(void)
{
   const uint16_t ret = PendingHIRQ;

   PendingHIRQ = 0;

   return ret;
}

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

         IntFlags    = 0;
         PendingHIRQ = 0;

         FIFO_Clear(&VFIFO);
         FIFO_Clear(&AFIFO);

         ActionStatus = MPEG_ASTV_STOP | MPEG_ASTD_STOP | MPEG_ASTA_STOP;
         PictureInfo  = 0;
         AudioStatus  = MPEG_STA_BEMPTY;
         VideoStatus  = MPEG_STV_BEMPTY;
         VCounter     = 0;
         FrameValid   = false;

         SeqSeen         = false;
         VideoStarted    = false;
         AudioStarted    = false;
         VideoReady      = false;
         AudioReady      = false;
         LastVideoErrors = 0;

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
         /* Video and audio both move to the transferring state.  The
            play mode in CR1/CR2 and the start offset in CR3/CR4 are not
            acted on: the decoders are already fed by whatever the CD
            block routes, and nothing here seeks. */
         ActionStatus = MPEG_ASTV_TRNS | MPEG_ASTA_TRNS;
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
         /* CDC_MpDisp(dspsw, fbn) puts the display switch in CR2's high
            byte and the frame buffer number in its low byte.  This was
            previously read as CR1 bit 0, which meant the display switch
            never came on and nothing ever composited. */
         Display.enabled = (cd[1] >> 8) != 0;
         Display.fbn     = cd[1] & 0xFF;

         if(Display.enabled)
            VideoStatus |= MPEG_STV_DISP;
         else
            VideoStatus &= ~(uint16_t)MPEG_STV_DISP;

         MPEGReport(base_status, out);
         *hirq |= MPEG_HIRQ_MPCM;
         break;

      case MPEG_CMD_SET_WINDOW:
      {
         /* One opcode, five sub-functions.  SBL's cmdRspWin emits the
            selector in CR1's low byte, the change flag in CR2's low
            byte, x in CR3 and y in CR4.  The previous reading of this
            command ignored the selector entirely and so applied every
            window write to the same two fields. */
         const unsigned sel = cd[0] & 0xFF;
         const uint16_t x   = cd[2];
         const uint16_t y   = cd[3];

         switch(sel)
         {
            case MPEG_WIN_FPOS:
               Display.src_x = x;
               Display.src_y = y;
               break;

            case MPEG_WIN_FRAT:
               Display.rat_x = x;
               Display.rat_y = y;
               break;

            case MPEG_WIN_DPOS:
               Display.x = x;
               Display.y = y;
               break;

            case MPEG_WIN_DSIZ:
               Display.w     = x;
               Display.h     = y;
               WindowSizeSet = true;
               break;

            case MPEG_WIN_DOFS:
               Display.ofs_x = x;
               Display.ofs_y = y;
               break;

            default:
               break;
         }

         /* CR2's low byte is SBL's chgflg.  Its meaning is not
            documented in anything I have -- most likely apply-now
            versus apply-at-next-VSYNC -- so the write is always applied
            rather than sometimes dropped, which is the failure mode
            that would be hardest to spot. */

         MPEGReport(base_status, out);
         *hirq |= MPEG_HIRQ_MPCM;
      }
      break;

      case MPEG_CMD_SET_BORDERCOL:
         Display.border_color = cd[1];
         MPEGReport(base_status, out);
         *hirq |= MPEG_HIRQ_MPCM;
         break;

      case MPEG_CMD_SET_FADE:
         /* CDC_MpSetFade(gain_y, gain_c): luma gain in CR2's high byte,
            chroma gain in its low byte.  Only the luma gain scales the
            converted RGB; the chroma gain is recorded but unused until
            the conversion works in YCbCr. */
         Display.fade   = cd[1] >> 8;
         Display.fade_c = cd[1] & 0xFF;
         MPEGReport(base_status, out);
         *hirq |= MPEG_HIRQ_MPCM;
         break;

      case MPEG_CMD_SET_VIDEOEFF:
         /* CDC_MpSetVeff(itp, trp, moz_h, moz_v, soft_h, soft_v) packs
            six bytes across CR2..CR4.  Recorded but not acted on. */
         VideoEffects[0] = cd[1] >> 8;   /* interpolation mode  */
         VideoEffects[1] = cd[1] & 0xFF; /* transparent bit     */
         VideoEffects[2] = cd[2] >> 8;   /* horizontal mosaic   */
         VideoEffects[3] = cd[2] & 0xFF; /* vertical mosaic     */
         VideoEffects[4] = cd[3] >> 8;   /* horizontal soften   */
         VideoEffects[5] = cd[3] & 0xFF; /* vertical soften     */

         MPEGReport(base_status, out);
         *hirq |= MPEG_HIRQ_MPCM;
         break;

      case MPEG_CMD_GET_PTS:
         /* CDC_MpGetPts(pts_a) reads a single 32-bit value out of the
            response at offset 4, i.e. CR3:CR4.  The 90 kHz system clock
            timestamp is 33 bits on the wire; the low 32 are what fits
            here and what the library hands back. */
      {
         const uint64_t pts = (AudioPTS != RMPEG1_PS_NO_PTS) ? AudioPTS : 0;

         out[0] = (uint16_t)base_status << 8;
         out[1] = 0;
         out[2] = (uint16_t)(pts >> 16);
         out[3] = (uint16_t)pts;

         *hirq |= MPEG_HIRQ_MPCM;
      }
      break;

      case MPEG_CMD_GET_TIMECODE:
      {
         /* CDC_MpGetTc(bnk, pictyp, tr, mptc) unpacks:
              CR1 low byte  bit7 clear, bank number
              CR2           picture type, temporal reference
              CR3           hour, minute
              CR4           second, picture

            The hour/minute/second/picture group is a GOP timecode,
            which rmpeg1_video does not expose, so it is derived from
            the video PTS instead.  For Video CD the two agree; an
            authored stream whose GOP timecode starts at a non-zero
            offset would not be reproduced faithfully here.  Deriving is
            still better than reporting zeros, which a title using this
            to drive a seek would act on. */
         const uint64_t pts   = (VideoPTS != RMPEG1_PS_NO_PTS) ? VideoPTS : 0;
         const uint32_t total = (uint32_t)(pts / 90000);
         const uint32_t rem   = (uint32_t)(pts % 90000);
         uint32_t pic = 0;

         if(FrameRateDen)
            pic = (uint32_t)(((uint64_t)rem * FrameRateNum)
                             / ((uint64_t)FrameRateDen * 90000));

         out[0] = ((uint16_t)base_status << 8) | 0x00;   /* bank 0 */
         out[1] = ((uint16_t)PictureInfo << 8) | TemporalRef;
         out[2] = (uint16_t)((((total / 3600) % 24) << 8) | ((total / 60) % 60));
         out[3] = (uint16_t)(((total % 60) << 8) | (pic & 0xFF));

         *hirq |= MPEG_HIRQ_MPCM;
      }
      break;

      case MPEG_CMD_GET_PICT_SIZE:
         /* CDC_MpGetPictSiz reads the horizontal size from CR3 and the
            vertical from CR4. */
         out[0] = (uint16_t)base_status << 8;
         out[1] = 0;
         out[2] = (uint16_t)FrameW;
         out[3] = (uint16_t)FrameH;

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

/*
   Substream selection.  SET_STREAM's audstmid/vidstmid fields carry the
   stream ID the card should decode.  Software writes them either as a
   raw MPEG stream_id (0xE0.. for video, 0xC0.. for audio) or as a bare
   substream index, so accept both: mask off the type nibble when the
   value looks like a full stream_id.  0xFF is the SBL "don't care"
   convention and matches whatever arrives first.
*/
static INLINE bool StreamSelected(uint8_t want, uint8_t got_index, uint8_t base)
{
   if(want == 0xFF)
      return true;

   if((want & 0xE0) == base)
      return (want & 0x1F) == got_index;

   return want == got_index;
}

void MPEG_FeedSector(const uint8_t *data, uint32_t len, uint8_t submode)
{
   rmpeg1_ps_packet_t pkt;
   bool saw_video = false;
   bool saw_audio = false;

   if(!Present || !Demux || !data || !len)
      return;

   /*
      CdcMpCon.lay says what the connection carries.  The system layer
      is the whole Program Stream and the card demultiplexes it, which
      is the Video CD case and everything below.  A connection on the
      audio or video layer carries an already-separated elementary
      stream, so the demuxer must be bypassed entirely -- running a raw
      elementary stream through a Program Stream parser would find no
      pack headers and discard all of it.
   */
   if((Con[0].vidlay & MPEG_LAY_MASK) == MPEG_LAY_ES
      && Con[0].vidbufnum != MPEG_NUL_SEL)
   {
      VDropped += FIFO_Write(&VFIFO, data, len);

      if(!VideoReady)
      {
         VideoReady = true;
         RaiseInt(MPEG_INT_VSRDY);
      }

      if(submode & 0x10) RaiseInt(MPEG_INT_VTRG);
      if(submode & 0x01) RaiseInt(MPEG_INT_VEOR);
      return;
   }

   if((Con[0].audlay & MPEG_LAY_MASK) == MPEG_LAY_ES
      && Con[0].audbufnum != MPEG_NUL_SEL)
   {
      ADropped += FIFO_Write(&AFIFO, data, len);

      if(!AudioReady)
      {
         AudioReady = true;
         RaiseInt(MPEG_INT_ASRDY);
      }

      if(submode & 0x10) RaiseInt(MPEG_INT_ATRG);
      if(submode & 0x01) RaiseInt(MPEG_INT_AEOR);
      return;
   }

   /* rmpeg1_ps_write() consumes less than len only when its buffer is
      full, which means packets are waiting; drain and retry rather than
      losing the tail. */
   while(len)
   {
      size_t took = rmpeg1_ps_write(Demux, data, len);

      data += took;
      len  -= (uint32_t)took;

      while(rmpeg1_ps_next(Demux, &pkt))
      {
         switch(pkt.type)
         {
            case RMPEG1_PS_VIDEO:
               if(StreamSelected(Stm[0].vidstmid, pkt.index, 0xE0))
               {
                  VDropped += FIFO_Write(&VFIFO, pkt.data, (uint32_t)pkt.size);

                  if(pkt.pts != RMPEG1_PS_NO_PTS)
                     VideoPTS = pkt.pts;

                  saw_video = true;

                  if(!VideoReady)
                  {
                     VideoReady = true;
                     RaiseInt(MPEG_INT_VSRDY);
                  }
               }
               break;

            case RMPEG1_PS_AUDIO:
               if(StreamSelected(Stm[0].audstmid, pkt.index, 0xC0))
               {
                  ADropped += FIFO_Write(&AFIFO, pkt.data, (uint32_t)pkt.size);

                  if(pkt.pts != RMPEG1_PS_NO_PTS)
                     AudioPTS = pkt.pts;

                  saw_audio = true;

                  if(!AudioReady)
                  {
                     AudioReady = true;
                     RaiseInt(MPEG_INT_ASRDY);
                  }
               }
               break;

            default:
               /* Private and padding streams are not used by Video CD
                  video tracks; ignore rather than buffer them. */
               break;
         }
      }

      /* No forward progress and nothing drained: the packet is larger
         than the buffer, which rmpeg1_ps_init() already refused to make
         possible.  Bail rather than spin. */
      if(!took)
         break;
   }

   LastSCR = rmpeg1_ps_scr(Demux);

   /* CD-ROM XA submode carries per-sector trigger and end-of-record
      flags, and the card reports them as interrupt factors so software
      can synchronise to authored marks in the stream.  The sector's
      own video/audio submode bits say nothing useful here -- a Video CD
      sector is Form 2 real-time data carrying both -- so attribute the
      flags to whichever streams the demuxer actually produced. */
   if(submode & 0x10)   /* trigger */
   {
      if(saw_video) RaiseInt(MPEG_INT_VTRG);
      if(saw_audio) RaiseInt(MPEG_INT_ATRG);
   }

   if(submode & 0x01)   /* end of record */
   {
      if(saw_video) RaiseInt(MPEG_INT_VEOR);
      if(saw_audio) RaiseInt(MPEG_INT_AEOR);
   }

   if(rmpeg1_ps_ended(Demux))
      RaiseInt(MPEG_INT_SQEND);
}

uint32_t MPEG_GetESFill(bool is_video)
{
   return is_video ? VFIFO.count : AFIFO.count;
}

uint32_t MPEG_GetESDropped(bool is_video)
{
   return is_video ? VDropped : ADropped;
}

uint64_t MPEG_GetPTS(bool is_video)
{
   return is_video ? VideoPTS : AudioPTS;
}

/*
 *
 * Colour conversion
 *
 * ITU-R BT.601 studio-swing YCbCr to RGB, in integer arithmetic at 16
 * fractional bits.  Fixed point rather than float on purpose: this
 * output feeds savestates and, through VDP2, the frame the rest of the
 * emulator compares against, so it has to be bit-reproducible across
 * hosts and build configurations.
 *
 *   R = 1.164(Y-16)                  + 1.596(Cr-128)
 *   G = 1.164(Y-16) - 0.392(Cb-128)  - 0.813(Cr-128)
 *   B = 1.164(Y-16) + 2.017(Cb-128)
 */

#define YUV_FRAC   16
#define YUV_Y      76309   /* 1.164 */
#define YUV_RV    104597   /* 1.596 */
#define YUV_GU    -25675   /* -0.392 */
#define YUV_GV    -53279   /* -0.813 */
#define YUV_BU    132201   /* 2.017 */

static INLINE uint32_t ClampU5(int32_t v)
{
   /* Shift to 5 bits, then clamp -- clamping after the shift keeps the
      comparison against constants the compiler can fold. */
   v >>= (YUV_FRAC + 3);

   if(v < 0)
      return 0;
   if(v > 31)
      return 31;

   return (uint32_t)v;
}

/* 4:2:0 planar to RGB555, chroma upsampled by replication.  Nearest
   neighbour rather than a proper interpolating upsampler because the
   card's own output was not doing anything cleverer at 352x240 and an
   interpolator would be inventing detail the hardware did not show. */
static void ConvertFrame(const rmpeg1_video_frame_t *f)
{
   unsigned x, y;
   unsigned w = f->width;
   unsigned h = f->height;

   if(w > MPEG_MAX_WIDTH)  w = MPEG_MAX_WIDTH;
   if(h > MPEG_MAX_HEIGHT) h = MPEG_MAX_HEIGHT;

   for(y = 0; y < h; y++)
   {
      const uint8_t *yp = f->y  + (size_t)y * f->y_stride;
      const uint8_t *up = f->cb + (size_t)(y >> 1) * f->c_stride;
      const uint8_t *vp = f->cr + (size_t)(y >> 1) * f->c_stride;
      uint16_t      *dp = FrameBuf + (size_t)y * MPEG_MAX_WIDTH;

      for(x = 0; x < w; x++)
      {
         const int32_t yy = ((int32_t)yp[x] - 16)      * YUV_Y;
         const int32_t cb = (int32_t)up[x >> 1] - 128;
         const int32_t cr = (int32_t)vp[x >> 1] - 128;

         const uint32_t r = ClampU5(yy + YUV_RV * cr);
         const uint32_t g = ClampU5(yy + YUV_GU * cb + YUV_GV * cr);
         const uint32_t b = ClampU5(yy + YUV_BU * cb);

         /* Saturn RGB555 is BGR order in the low 15 bits. */
         dp[x] = (uint16_t)((b << 10) | (g << 5) | r);
      }
   }

   FrameW = w;
   FrameH = h;

   /* The display window follows the decoded picture unless SET_WINDOW
      has said otherwise.  A Video CD is full-screen by default and the
      picture size is not known until the sequence header is parsed, so
      a fixed default would letterbox 352x288 PAL content inside a
      352x240 window for no reason. */
   if(!WindowSizeSet)
   {
      Display.w = (uint16_t)w;
      Display.h = (uint16_t)h;
   }
}

/*
 *
 * Audio ring
 *
 */

static void ARing_Push(const int16_t *pcm, uint32_t frames, uint32_t channels)
{
   uint32_t i;

   if(!ARing)
      return;

   for(i = 0; i < frames; i++)
   {
      int16_t l, r;

      if(ARing_Count >= MPEG_ARING_FRAMES)
      {
         /* Overrun: drop the oldest pair rather than the newest, so a
            momentarily starved drain resumes at the current position
            instead of replaying stale audio. */
         ARing_RP = (ARing_RP + 1) % MPEG_ARING_FRAMES;
         ARing_Count--;
         ADropped++;
      }

      if(channels >= 2)
      {
         l = pcm[i * 2 + 0];
         r = pcm[i * 2 + 1];
      }
      else
         l = r = pcm[i];

      ARing[ARing_WP * 2 + 0] = l;
      ARing[ARing_WP * 2 + 1] = r;

      ARing_WP = (ARing_WP + 1) % MPEG_ARING_FRAMES;
      ARing_Count++;
   }
}

uint32_t MPEG_ReadAudio(int16_t *out, uint32_t frames)
{
   uint32_t got = 0;

   if(!ARing || !out)
      return 0;

   while(got < frames && ARing_Count)
   {
      out[got * 2 + 0] = ARing[ARing_RP * 2 + 0];
      out[got * 2 + 1] = ARing[ARing_RP * 2 + 1];

      ARing_RP = (ARing_RP + 1) % MPEG_ARING_FRAMES;
      ARing_Count--;
      got++;
   }

   return got;
}

void MPEG_GetAudioFormat(uint32_t *rate, uint32_t *channels)
{
   if(rate)     *rate     = AudioRate;
   if(channels) *channels = AudioChannels;
}

bool MPEG_GetAudioSample(uint16_t *out)
{
   if(!Present || !ARing || !out)
      return false;

   /* Nothing decoded yet: leave the SCSP's external input to CD-DA
      rather than muting it.  Substituting silence here would cut CD
      audio the moment the card was enabled. */
   if(!ACurValid && !ARing_Count)
      return false;

   ARatePhase += ARateStep;

   while(ARatePhase >= (1U << 16))
   {
      if(!ARing_Count)
      {
         /* Underrun.  Hold the last sample rather than emitting a zero:
            a dropout in the middle of a decoded stream is a click, and
            holding is both quieter and a better signal that the decoder
            fell behind than silence is. */
         break;
      }

      ACur[0] = ARing[ARing_RP * 2 + 0];
      ACur[1] = ARing[ARing_RP * 2 + 1];

      ARing_RP = (ARing_RP + 1) % MPEG_ARING_FRAMES;
      ARing_Count--;

      ACurValid   = true;
      ARatePhase -= (1U << 16);
   }

   /* An underrun leaves phase above unity; clear it so the backlog does
      not turn into a burst of consumption once audio resumes. */
   if(ARatePhase >= (1U << 16))
      ARatePhase = 0;

   out[0] = (uint16_t)ACur[0];
   out[1] = (uint16_t)ACur[1];

   return true;
}

/*
 *
 * Decode
 *
 */

/* Drain the audio ES FIFO through rmp3.  Separate from the picture
   cadence: an MPEG audio frame is 1152 samples and lines up with
   nothing in particular on the video side. */
static void RunAudio(void)
{
   uint8_t staging[4096];

   if(!Audio || !AFIFO.count)
      return;

   for(;;)
   {
      uint32_t avail = AFIFO.count;
      uint32_t take  = (avail > sizeof(staging)) ? (uint32_t)sizeof(staging) : avail;
      size_t   read  = 0;
      size_t   wrote = 0;
      int      st;

      if(!take)
         break;

      FIFO_Peek(&AFIFO, staging, take);

      rmp3_stream_set_in(Audio, staging, take);
      rmp3_stream_set_out_s16(Audio, APCM, RMP3_MAX_SAMPLES_PER_FRAME / 2);

      st = rmp3_stream_process(Audio, &read, &wrote);

      FIFO_Drop(&AFIFO, (uint32_t)read);

      if(wrote)
      {
         unsigned ch = 2, rate = 44100;

         rmp3_stream_info(Audio, &ch, &rate);

         AudioChannels = ch ? ch : 2;
         AudioRate     = rate ? rate : 44100;

         /* Rounded rather than truncated: at 32000 Hz truncation would
            accumulate a sample of drift every few seconds. */
         ARateStep = (uint32_t)((((uint64_t)AudioRate << 16) + (MPEG_SCSP_RATE / 2))
                                / MPEG_SCSP_RATE);
         if(!ARateStep)
            ARateStep = 1U << 16;

         ARing_Push(APCM, (uint32_t)wrote, AudioChannels);

         AudioStatus = MPEG_STA_DEC | MPEG_STA_OUTL
                     | ((AudioChannels >= 2) ? MPEG_STA_OUTR : 0);

         if(!AudioReady)
         {
            AudioReady = true;
            RaiseInt(MPEG_INT_ASRDY);
         }

         RaiseInt(MPEG_INT_AORDY);

         if(!AudioStarted)
         {
            AudioStarted = true;
            RaiseInt(MPEG_INT_AOSTRT);
         }
      }

      if(st != RMP3_STREAM_OK)
         break;

      /* No forward progress on either side: the decoder wants a whole
         frame it has not been given yet.  Wait for more input rather
         than spinning on the same bytes. */
      if(!read && !wrote)
         break;
   }
}

bool MPEG_RunFrame(void)
{
   uint8_t staging[8192];
   rmpeg1_video_frame_t frame;
   bool produced = false;

   if(!Present || !Video)
      return false;

   RunAudio();

   /* Sequence start is a level condition inside the decoder rather than
      an event it reports, so edge-detect it here.  GOP start has no
      equivalent in rmpeg1_video's interface, so MPEG_INT_GSTRT is never
      raised -- worth knowing if a title turns out to synchronise on it. */
   if(!SeqSeen && rmpeg1_video_has_sequence(Video))
   {
      SeqSeen = true;
      RaiseInt(MPEG_INT_SQSTRT);
   }

   {
      const uint32_t errs = rmpeg1_video_errors(Video);

      if(errs != LastVideoErrors)
      {
         LastVideoErrors = errs;
         VideoStatus |= MPEG_STV_ERR;
         RaiseInt(MPEG_INT_VDERR);
      }
   }

   /* Push video ES until a picture pops out.  rmpeg1_video holds a
      window internally and reports a short write when it is full, which
      is the signal to stop feeding and drain. */
   for(;;)
   {
      uint32_t take;
      size_t   took;

      if(rmpeg1_video_decode(Video, &frame))
      {
         ConvertFrame(&frame);

         FrameValid   = true;
         produced     = true;
         PictureInfo  = frame.coding_type;
         TemporalRef  = (uint8_t)frame.temporal_ref;

         VideoStatus = MPEG_STV_DEC | MPEG_STV_UPDPIC | MPEG_STV_RDY
                     | (Display.enabled ? MPEG_STV_DISP : 0);

         RaiseInt(MPEG_INT_PSTRT);
         RaiseInt(MPEG_INT_VORDY);

         if(!VideoStarted)
         {
            VideoStarted = true;
            VideoStatus |= MPEG_STV_1STPIC;
            RaiseInt(MPEG_INT_VOSTRT);
         }

         break;
      }

      if(!VFIFO.count)
         break;

      take = VFIFO.count;
      if(take > sizeof(staging))
         take = (uint32_t)sizeof(staging);

      FIFO_Peek(&VFIFO, staging, take);
      took = rmpeg1_video_write(Video, staging, take);
      FIFO_Drop(&VFIFO, (uint32_t)took);

      /* Window full and no picture ready: the decoder needs a start
         code it has not seen.  Nothing more to do this call. */
      if(!took)
         break;
   }

   return produced;
}

bool MPEG_WantsPartition(uint8_t pnum)
{
   if(!Present || pnum >= 0x18)
      return false;

   /* CdcMpCon.bn is the buffer partition the decoder draws from.  This
      previously matched on cmod, which is not a selector at all but a
      set of switch and clear condition flags -- routing only appeared
      to work because both fields default to values that never matched
      a live partition.

      Either stream matching is enough and the sector is fed once: on a
      Video CD both elementary streams arrive interleaved in one Program
      Stream through one partition, and the card splits them.  Feeding
      twice because both connections name the same partition would
      duplicate every byte into the demuxer. */
   if(Con[0].vidbufnum != MPEG_NUL_SEL && Con[0].vidbufnum == pnum)
      return true;

   if(Con[0].audbufnum != MPEG_NUL_SEL && Con[0].audbufnum == pnum)
      return true;

   return false;
}

bool MPEG_ConsumesPartition(uint8_t pnum)
{
   if(!MPEG_WantsPartition(pnum))
      return false;

   if(Con[0].vidbufnum == pnum && (Con[0].vidcon & MPEG_CMOD_DEL))
      return true;

   if(Con[0].audbufnum == pnum && (Con[0].audcon & MPEG_CMOD_DEL))
      return true;

   return false;
}

void MPEG_Update(int64_t clocks)
{
   int64_t period;

   if(!Present || clocks <= 0)
      return;

   /* Follow the sequence header once the decoder has one.  11172-2
      codes exact rationals -- 30000/1001 and friends -- so this stays
      integer and the accumulated drift is zero rather than merely
      small. */
   if(Video && rmpeg1_video_has_sequence(Video))
   {
      unsigned num = 0, den = 0;

      rmpeg1_video_framerate(Video, &num, &den);

      if(num && den)
      {
         FrameRateNum = num;
         FrameRateDen = den;
      }
   }

   period = (int64_t)(((uint64_t)MPEG_CDB_CLOCK * FrameRateDen) / FrameRateNum) << 32;

   if(period <= 0)
      return;

   FrameAccum += clocks;

   /* A long stall -- a savestate load, or a large timestamp jump --
      must not turn into thousands of decodes in one call.  Cap the
      backlog and drop the excess: real hardware that fell this far
      behind would have dropped those frames too, and catching up
      instantly would be worse than not catching up at all.

      MPEG_CATCHUP_FRAMES is well above anything normal operation
      produces -- the CD block's event handler runs far more often than
      once every four frames -- so this only bites on a genuine stall. */
   if(FrameAccum > period * MPEG_CATCHUP_FRAMES)
      FrameAccum = period * MPEG_CATCHUP_FRAMES;

   while(FrameAccum >= period)
   {
      FrameAccum -= period;

      VCounter = (uint16_t)(VCounter + 1);

      MPEG_RunFrame();
   }
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
      SFVAR(PendingHIRQ),
      SFVAR(SeqSeen),
      SFVAR(VideoStarted),
      SFVAR(AudioStarted),
      SFVAR(VideoReady),
      SFVAR(AudioReady),
      SFVAR(LastVideoErrors),

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
      SFVAR(TemporalRef),
      SFPTR8N(VideoEffects, sizeof(VideoEffects), "VideoEffects"),
      SFVAR(LSIRegs[0]),
      SFVAR(LSIRegs[1]),

      SFVAR(Display.enabled),
      SFVAR(Display.fbn),
      SFVAR(Display.ofs_x),
      SFVAR(Display.ofs_y),
      SFVAR(Display.rat_x),
      SFVAR(Display.rat_y),
      SFVAR(Display.fade_c),
      SFVAR(Display.x),
      SFVAR(Display.y),
      SFVAR(Display.w),
      SFVAR(Display.h),
      SFVAR(Display.src_x),
      SFVAR(Display.src_y),
      SFVAR(Display.border_color),
      SFVAR(Display.fade),

      SFVAR(ARatePhase),
      SFVAR(ARateStep),
      SFVAR(AudioRate),
      SFVAR(AudioChannels),

      SFVAR(WindowSizeSet),
      SFVAR(FrameAccum),
      SFVAR(FrameRateNum),
      SFVAR(FrameRateDen),

      SFVAR(VideoPTS),
      SFVAR(AudioPTS),
      SFVAR(LastSCR),

      SFEND
   };

   MDFNSS_StateAction(sm, load, data_only, StateRegs, "MPEG", true);

   if(load)
   {
      /* Neither the ES FIFOs nor the demuxer's parse state are
         serialised.  The demuxer holds a partially-parsed packet and a
         start-code hunt position; restoring elementary stream that a
         stale parser then appended to would splice two unrelated
         packets together.  Dropping both is correct and cheap: the CD
         block re-feeds from the restored disc position and the parser
         resynchronises on the next start code, which is exactly the
         mid-stream entry case rmpeg1_ps is built to handle.

         The timestamps above are kept, because they are what the
         decode-timing comparison against SCR needs in order to resume
         at the right presentation point rather than from zero. */
      if(Demux)
         rmpeg1_ps_reset(Demux);

      /* Same reasoning applies one layer down.  rmpeg1_video holds
         reference pictures and a partially-decoded slice, rmp3 holds
         filter-bank history; neither is serialised, and feeding
         restored ES into stale state would decode against the wrong
         references.  Both resynchronise on the next start code, and
         rmpeg1_video counts the unreconstructable pictures it steps
         over rather than emitting garbage. */
      if(Video)
         rmpeg1_video_reset(Video);

      if(Audio)
      {
         rmp3_stream_free(Audio);
         Audio = rmp3_stream_new();
      }

      FIFO_Clear(&VFIFO);
      FIFO_Clear(&AFIFO);

      ARing_RP    = 0;
      ARing_WP    = 0;
      ARing_Count = 0;

      VDropped   = 0;
      ADropped   = 0;

      FrameValid = false;
   }
}
