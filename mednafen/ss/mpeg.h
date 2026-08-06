/******************************************************************************/
/* Mednafen Sega Saturn Emulation Module                                      */
/******************************************************************************/
/* mpeg.h - Video CD Card (MPEG card) emulation
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
   The Video CD Card (a.k.a. MPEG card / Movie Card) is NOT a
   cartridge-slot device -- it does not live on the A-bus, and so it is
   deliberately not modelled as a CartInfo.  It plugs into the dedicated
   MPEG expansion connector and hangs off three buses:

     1) The CD block (SH-1).  All host-visible control happens through
        CD block commands 0x90-0xAF, plus AUTH_DEVICE/GET_AUTH with
        CR2 == 1.  MPEG sector data is routed to the card by the CD
        block's filter/partition machinery.
     2) VDP2.  Decoded frames are presented as the external background
        (EXBGEN in the VDP2 EXTEN register) and composited with normal
        priority/colour-calculation.
     3) SCSP.  Decoded MPEG audio is mixed into the analogue output
        path.

   This translation unit models (1) in full and holds the state that
   (2) and (3) consume.  All three codec layers come from
   libretro-common: rmpeg1_ps demultiplexes the Program Stream,
   rmpeg1_video decodes the MPEG-1 video elementary stream, and rmp3
   decodes the MPEG-1 Layer II audio elementary stream.  Nothing about
   MPEG lives in this core beyond the glue.
*/

#ifndef __MDFN_SS_MPEG_H
#define __MDFN_SS_MPEG_H

#include <stdint.h>
#include <boolean.h>

#include <streams/file_stream.h>
#include <formats/rmpeg1_ps.h>	/* RMPEG1_PS_NO_PTS */

#include "../state.h"
#include "../mednafen-types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* CD block command opcodes handled by the card.  Confirmed against
   Yabause's cs2.c and the SBL MPEG library entry points.  Opcodes in
   the 0x90-0xAF range that are absent here are accepted and answered
   with a plain status report (see MPEG_Command) rather than rejected,
   because a rejection is far more likely to wedge a caller than a
   benign no-op is. */
enum
{
 MPEG_CMD_GET_STATUS      = 0x90,
 MPEG_CMD_GET_INTERRUPT   = 0x91,
 MPEG_CMD_SET_INT_MASK    = 0x92,
 MPEG_CMD_INIT            = 0x93,
 MPEG_CMD_SET_MODE        = 0x94,
 MPEG_CMD_PLAY            = 0x95,
 MPEG_CMD_SET_DECMETHOD   = 0x96,

 MPEG_CMD_SET_CONNECTION  = 0x9A,
 MPEG_CMD_GET_CONNECTION  = 0x9B,
 MPEG_CMD_SET_STREAM      = 0x9D,
 MPEG_CMD_GET_STREAM      = 0x9E,

 MPEG_CMD_DISPLAY         = 0xA0,
 MPEG_CMD_SET_WINDOW      = 0xA1,
 MPEG_CMD_SET_BORDERCOL   = 0xA2,
 MPEG_CMD_SET_FADE        = 0xA3,
 MPEG_CMD_SET_VIDEOEFF    = 0xA4,

 MPEG_CMD_SET_LSI         = 0xAF
};

#define MPEG_CMD_IS_MPEG(c) ((c) >= 0x90 && (c) <= 0xAF)

/* HIRQ bits the card asks the CD block to raise.  Mirrors the
   HIRQ_MPED/MPCM/MPST values in cdb.c; kept separate so this TU does
   not have to include cdb.c's private enum. */
enum
{
 MPEG_HIRQ_MPED = 0x0800,
 MPEG_HIRQ_MPCM = 0x1000,
 MPEG_HIRQ_MPST = 0x2000
};

/* Video CD / MPEG-1 constrained-parameter frame geometry. */
#define MPEG_MAX_WIDTH   352
#define MPEG_MAX_HEIGHT  288

/*
   Lifecycle.

   MPEG_Init() takes an already-open handle to the card's firmware ROM,
   or NULL.  The card is reported present either way -- a real card with
   a bad ROM still answers CD block commands -- but MPEG_Auth() will
   only succeed with a ROM loaded, which is what gates the BIOS Video CD
   player.  Returns false only on allocation failure.
*/
bool MPEG_Init(RFILE *rom_fp) MDFN_COLD;
void MPEG_Kill(void) MDFN_COLD;
void MPEG_Reset(bool powering_up) MDFN_COLD;
void MPEG_StateAction(StateMem *sm, const unsigned load, const bool data_only) MDFN_COLD;

bool MPEG_IsPresent(void);

/* Card firmware ROM, for the eventual BIOS-side Video CD player boot
   path.  Returns NULL and sets *size to 0 when no ROM was loaded. */
const uint8_t *MPEG_GetROM(uint32_t *size);

/*
   AUTH_DEVICE / GET_AUTH with CR2 == 1 target the card rather than the
   disc.  MPEG_Auth() latches the authenticated state; MPEG_GetAuth()
   returns the value the CD block should place in CR2 (2 == card
   authenticated, 0 == not).
*/
void MPEG_Auth(void);
uint16_t MPEG_GetAuth(void);

/*
   Command entry point, called from cdb.c's command loop.

   base_status is the CD block status byte the caller would otherwise
   pass to its own report builder (i.e. MakeBaseStatus(false, 0)); the
   card folds it into CR1 exactly as the CD block does for its own
   commands.

   cd[]  : CR1..CR4 as written by the host.
   out[] : CR1..CR4 to hand back to the host.
   hirq  : OR-in mask of HIRQ bits to raise.  HIRQ_CMOK is the caller's
           responsibility, since the caller owns the command handshake.

   Returns false if cmd is not an MPEG opcode, in which case out[] and
   *hirq are untouched and the caller should fall through to its own
   dispatch.
*/
bool MPEG_Command(uint8_t cmd, const uint16_t cd[4],
                  uint8_t base_status, uint16_t out[4], uint16_t *hirq);

/*
   Sector feed.  The CD block calls this for every sector whose filter
   output connector routes to the MPEG decoder rather than to a
   partition, passing the raw payload -- 2324 bytes for the Mode 2 Form
   2 sectors of a Video CD track, 2048 for Mode 1.

   The payload is an MPEG-1 Program Stream fragment, not an elementary
   stream: on a VCD the program stream is simply the concatenation of
   the Form 2 payloads.  Splitting it is the card's job, not the CD
   block's, so there is deliberately no video/audio selector here.
   Demultiplexing and substream selection happen inside, driven by the
   stream IDs that SET_STREAM configured.
*/
void MPEG_FeedSector(const uint8_t *data, uint32_t len) MDFN_HOT;

/* Bytes of demultiplexed elementary stream currently buffered, for the
   CD block's buffer-full/backpressure decisions and for tests. */
uint32_t MPEG_GetESFill(bool is_video);

/* Presentation timestamp most recently attached to a demultiplexed
   packet of the selected substream, in 90 kHz units, or
   RMPEG1_PS_NO_PTS when none has been seen. */
uint64_t MPEG_GetPTS(bool is_video);

/*
   Advance the decoders.  Returns true if a new picture was produced, in
   which case MPEG_GetFrame() is valid.  Audio is decoded independently
   of the picture cadence and accumulates in the ring MPEG_ReadAudio()
   drains, because MPEG frames carry 1152 samples apiece and do not line
   up with video frames.
*/
bool MPEG_RunFrame(void) MDFN_HOT;

/*
   Clock the card.  Called from the CD block's event handler with the
   same 32.32 fixed-point CD block clocks (11289600 Hz) that Drive_Run()
   takes, so the card runs on the disc's clock domain rather than the
   host frame rate -- which is what SET_MODE's PTS-synced decode timing
   will need, and what keeps decode rate independent of how fast the
   frontend is calling retro_run().
*/
void MPEG_Update(int64_t clocks) MDFN_HOT;

/*
   True when sectors passing CD block filter fnum should be handed to
   the card.  MPEG_SetConnection names the filter whose output feeds the
   decoders; before any SET_CONNECTION this is false for every filter,
   so a disc that never touches the card never pays for it.
*/
bool MPEG_WantsFilter(uint8_t fnum);

/*
   Drain decoded audio.  Writes up to frames interleaved stereo s16
   sample pairs and returns how many were actually available.  Output is
   at the stream's own rate -- 44.1 kHz for Video CD -- and needs
   resampling to the SCSP rate by the caller.
*/
uint32_t MPEG_ReadAudio(int16_t *out, uint32_t frames) MDFN_HOT;

/* Decoded audio rate and channel count, or 0 before the first frame. */
void MPEG_GetAudioFormat(uint32_t *rate, uint32_t *channels);

/*
   Most recently decoded picture, as packed 16bpp RGB555 in the Saturn's
   native component order, ready for VDP2 external-background
   compositing.  Returns NULL when no picture has been decoded.
*/
const uint16_t *MPEG_GetFrame(uint32_t *width, uint32_t *height);

/* Display geometry/appearance state, for the VDP2 compositor. */
typedef struct
{
 bool     enabled;      /* MPEG_CMD_DISPLAY                      */
 uint16_t x, y;         /* Window origin, display coordinates    */
 uint16_t w, h;         /* Window size                           */
 uint16_t src_x, src_y; /* Source origin within the decoded frame */
 uint16_t border_color; /* RGB555                                */
 uint8_t  fade;         /* 0x00 = black .. 0xFF = full           */
} MPEG_DisplayState;

const MPEG_DisplayState *MPEG_GetDisplayState(void);

#ifdef __cplusplus
}
#endif

#endif
