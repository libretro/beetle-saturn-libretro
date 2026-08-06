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

/*
   CD block command opcodes handled by the card.

   Recovered by disassembling SEGA_CDC.A from Sega Basic Library 6.01 --
   the SH-2 COFF objects cdc_mdc.o / cdc_mst.o / cdc_mwn.o / cdc_mfb.o /
   cdc_mbu.o / cdc_mls.o.  Each CDC_Mp* entry point builds an 8-byte
   command buffer whose first byte is the opcode, so the mapping is
   unambiguous.  This supersedes the partial and partly speculative list
   that had been derived from Yabause: the range 0x90-0xAF is fully
   accounted for, with no gaps.
*/
enum
{
 MPEG_CMD_GET_STATUS      = 0x90, /* CDC_MpGetCurStat   */
 MPEG_CMD_GET_INTERRUPT   = 0x91, /* CDC_MpGetInt       */
 MPEG_CMD_SET_INT_MASK    = 0x92, /* CDC_MpSetIntMsk    */
 MPEG_CMD_INIT            = 0x93, /* CDC_MpInit         */
 MPEG_CMD_SET_MODE        = 0x94, /* CDC_MpSetMode      */
 MPEG_CMD_PLAY            = 0x95, /* CDC_MpPlay         */
 MPEG_CMD_SET_DECMETHOD   = 0x96, /* CDC_MpSetDec       */
 MPEG_CMD_OUT_DECSYNC     = 0x97, /* CDC_MpOutDsync     */
 MPEG_CMD_GET_TIMECODE    = 0x98, /* CDC_MpGetTc        */
 MPEG_CMD_GET_PTS         = 0x99, /* CDC_MpGetPts       */

 MPEG_CMD_SET_CONNECTION  = 0x9A, /* CDC_MpSetCon       */
 MPEG_CMD_GET_CONNECTION  = 0x9B, /* CDC_MpGetCon       */
 MPEG_CMD_CHANGE_CONN     = 0x9C, /* CDC_MpChgCon       */
 MPEG_CMD_SET_STREAM      = 0x9D, /* CDC_MpSetStm       */
 MPEG_CMD_GET_STREAM      = 0x9E, /* CDC_MpGetStm       */
 MPEG_CMD_GET_PICT_SIZE   = 0x9F, /* CDC_MpGetPictSiz   */

 MPEG_CMD_DISPLAY         = 0xA0, /* CDC_MpDisp         */
 MPEG_CMD_SET_WINDOW      = 0xA1, /* the five CDC_MpSetWin* entry points */
 MPEG_CMD_SET_BORDERCOL   = 0xA2, /* CDC_MpSetBcolor    */
 MPEG_CMD_SET_FADE        = 0xA3, /* CDC_MpSetFade      */
 MPEG_CMD_SET_VIDEOEFF    = 0xA4, /* CDC_MpSetVeff      */
 MPEG_CMD_GET_IMAGE       = 0xA5, /* CDC_MpGetImg       */
 MPEG_CMD_SET_IMAGE       = 0xA6, /* CDC_MpSetImgPos / CDC_MpSetImgSiz */
 MPEG_CMD_READ_IMAGE      = 0xA7, /* CDC_MpReadImg      */
 MPEG_CMD_WRITE_IMAGE     = 0xA8, /* CDC_MpWriteImg     */
 MPEG_CMD_READ_SECTOR     = 0xA9, /* CDC_MpReadSct      */
 MPEG_CMD_WRITE_SECTOR    = 0xAA, /* CDC_MpWriteSct     */

 MPEG_CMD_GET_LSI         = 0xAE, /* CDC_MpGetLsi       */
 MPEG_CMD_SET_LSI         = 0xAF  /* CDC_MpSetLsi       */
};

/*
   SET_WINDOW sub-function, in CR1's low byte.  All five SBL window
   entry points tail-call one builder (cdc_mwn.o's cmdRspWin) which
   emits opcode 0xA1 with this selector, the change flag in CR2's low
   byte, x in CR3 and y in CR4.
*/
enum
{
 MPEG_WIN_FPOS = 0, /* CDC_MpSetWinFpos -- source origin in the frame */
 MPEG_WIN_FRAT = 1, /* CDC_MpSetWinFrat -- source scaling ratio       */
 MPEG_WIN_DPOS = 2, /* CDC_MpSetWinDpos -- display origin             */
 MPEG_WIN_DSIZ = 3, /* CDC_MpSetWinDsiz -- display size               */
 MPEG_WIN_DOFS = 4  /* CDC_MpSetWinDofs -- display offset             */
};

#define MPEG_CMD_IS_MPEG(c) ((c) >= 0x90 && (c) <= 0xAF)

/* HIRQ bits the card asks the CD block to raise.  Mirrors the
   HIRQ_MPED/MPCM/MPST values in cdb.c; kept separate so this TU does
   not have to include cdb.c's private enum.

   Names and meanings from SBL 6.01, sega_cdc.h:
     MPED  bit11  end of MPEG-related processing
     MPCM  bit12  end of an MPEG operation-indeterminate interval
     MPST  bit13  notification of MPEG interrupt status
*/
enum
{
 MPEG_HIRQ_MPED = 0x0800,
 MPEG_HIRQ_MPCM = 0x1000,
 MPEG_HIRQ_MPST = 0x2000
};

/*
   MPEG interrupt factors, verbatim from SBL 6.01's CDC_MPINT_* in
   sega_cdc.h.  GET_INTERRUPT returns these masked by SET_INTERRUPT_MASK
   in CR1's low byte (23..16) and CR2 (15..0), and raises HIRQ_MPST when
   any unmasked factor is pending.

   Comments are translated from the Japanese originals.
*/
enum
{
 MPEG_INT_VSRDY  = 0x00000001, /* video stream ready                     */
 MPEG_INT_VSCHG  = 0x00000002, /* video stream switch complete           */
 MPEG_INT_VORDY  = 0x00000004, /* video output ready                     */
 MPEG_INT_VOSTRT = 0x00000008, /* video output started                   */
 MPEG_INT_VDERR  = 0x00000010, /* video decode error                     */
 MPEG_INT_VSERR  = 0x00000020, /* video stream data error                */
 MPEG_INT_VBERR  = 0x00000040, /* video buffer partition connect error   */
 MPEG_INT_VNERR  = 0x00000080, /* next video stream data error           */
 MPEG_INT_PSTRT  = 0x00000100, /* picture start detected                 */
 MPEG_INT_GSTRT  = 0x00000200, /* GOP start detected                     */
 MPEG_INT_SQEND  = 0x00000400, /* sequence end detected                  */
 MPEG_INT_SQSTRT = 0x00000800, /* sequence start detected                */
 MPEG_INT_VTRG   = 0x00001000, /* trigger bit in a video sector          */
 MPEG_INT_VEOR   = 0x00002000, /* EOR bit in a video sector              */
 MPEG_INT_ATRG   = 0x00004000, /* trigger bit in an audio sector         */
 MPEG_INT_AEOR   = 0x00008000, /* EOR bit in an audio sector             */
 MPEG_INT_ASRDY  = 0x00010000, /* audio stream ready                     */
 MPEG_INT_ASCHG  = 0x00020000, /* audio stream switch complete           */
 MPEG_INT_AORDY  = 0x00040000, /* audio output ready                     */
 MPEG_INT_AOSTRT = 0x00080000, /* audio output started                   */
 MPEG_INT_ADERR  = 0x00100000, /* audio decode error                     */
 MPEG_INT_ASERR  = 0x00200000, /* audio stream data error                */
 MPEG_INT_ABERR  = 0x00400000, /* audio buffer partition connect error   */
 MPEG_INT_ANERR  = 0x00800000  /* next audio stream data error           */
};

/* MPEG video status, CR4 of a status report.  SBL CDC_MPSTV_*. */
enum
{
 MPEG_STV_DEC    = 0x0001, /* video decode running       */
 MPEG_STV_DISP   = 0x0002, /* displaying                 */
 MPEG_STV_PAUSE  = 0x0004, /* paused                     */
 MPEG_STV_FREEZE = 0x0008, /* frozen                     */
 MPEG_STV_LSTPIC = 0x0010, /* showing the last picture   */
 MPEG_STV_FIELD  = 0x0020, /* odd field                  */
 MPEG_STV_UPDPIC = 0x0040, /* picture updated            */
 MPEG_STV_ERR    = 0x0080, /* video error                */
 MPEG_STV_RDY    = 0x0100, /* output ready               */
 MPEG_STV_1STPIC = 0x0800, /* showing the first picture  */
 MPEG_STV_BEMPTY = 0x1000  /* video buffer partition empty */
};

/* MPEG audio status, CR3's low byte.  SBL CDC_MPSTA_*. */
enum
{
 MPEG_STA_DEC    = 0x01, /* audio decode running         */
 MPEG_STA_ILG    = 0x08, /* illegal audio                */
 MPEG_STA_BEMPTY = 0x10, /* audio buffer partition empty */
 MPEG_STA_ERR    = 0x20, /* audio error                  */
 MPEG_STA_OUTL   = 0x40, /* left channel output          */
 MPEG_STA_OUTR   = 0x80  /* right channel output         */
};

/*
   MPEG operation status, CR1's low byte.  Video state in bits 0..2,
   decode-stopped in bit 3, audio state in bits 4..6.  SBL CDC_MPASTV_*
   / CDC_MPASTD_STOP / CDC_MPASTA_*.
*/
enum
{
 MPEG_ASTV_STOP = 0x01,
 MPEG_ASTV_PRE1 = 0x02,
 MPEG_ASTV_PRE2 = 0x03,
 MPEG_ASTV_TRNS = 0x04,  /* transferring / playing */
 MPEG_ASTV_CHG  = 0x05,
 MPEG_ASTV_RCV  = 0x06,

 MPEG_ASTD_STOP = 0x08,  /* MPEG decode stopped */

 MPEG_ASTA_STOP = 0x10,
 MPEG_ASTA_PRE1 = 0x20,
 MPEG_ASTA_PRE2 = 0x30,
 MPEG_ASTA_TRNS = 0x40,
 MPEG_ASTA_CHG  = 0x50,
 MPEG_ASTA_RCV  = 0x60
};

/* Hardware flag bit reported by GET_HWINFO.  SBL CDC_HFLAG_MPEG. */
#define MPEG_HFLAG_PRESENT 0x02

/*
   SET_CONNECTION's CdcMpCon.cmod is a set of switch and clear
   conditions, not a selector.  SBL CDC_MPCMOD_*.
*/
enum
{
 MPEG_CMOD_EOR   = 0x01, /* switch on the EOR bit              */
 MPEG_CMOD_SEC   = 0x02, /* switch on the system end code      */
 MPEG_CMOD_DEL   = 0x04, /* delete the sector after taking it  */
 MPEG_CMOD_IGPTS = 0x08, /* ignore PTS identification          */
 MPEG_CMOD_VCLR  = 0x10, /* clear the VBV                      */
 MPEG_CMOD_VWCLR = 0x20, /* clear VBV and WBC                  */
 MPEG_CMOD_BEF   = 0x40  /* test the end condition before the
                            trailing aperture                  */
};

/*
   CdcMpCon.lay selects which layer the connection carries.  The system
   layer is the whole Program Stream, which the card demultiplexes
   itself; the audio and video layers are already-separated elementary
   streams.  Bits 7..6 are the picture-search mode and are masked off.
   SBL CDC_MPLAY_* / CDC_MPSRCH_*.
*/
enum
{
 MPEG_LAY_SYS   = 0x00,
 MPEG_LAY_ES    = 0x01, /* CDC_MPLAY_AUDIO == CDC_MPLAY_VIDEO == 1 */
 MPEG_LAY_MASK  = 0x01,

 MPEG_SRCH_OFF   = 0x00,
 MPEG_SRCH_VIDEO = 0x80,
 MPEG_SRCH_AV    = 0xC0
};

/*
   SET_DECMETHOD's audio mute field, CR1's low byte.  SBL CDC_MPMUT_*.
   The default value has its own bit rather than being zero, so a caller
   that wants no muting sets 0x04 rather than clearing everything.
*/
enum
{
 MPEG_MUT_R   = 0x01, /* mute the right channel */
 MPEG_MUT_L   = 0x02, /* mute the left channel  */
 MPEG_MUT_DFL = 0x04  /* default: do not mute   */
};

/*
   CDC_PARA_NOCHG: leave a parameter alone.  Byte fields carry 0xFF and
   16-bit fields 0xFFFF; SBL spells both MPG_IGNORE.
*/
#define MPEG_NOCHG8  0xFF
#define MPEG_NOCHG16 0xFFFF

/*
   SET_DECMETHOD's pause and freeze fields are intervals, not modes.
   MPG_MvPause maps its command onto one: 0 holds, 1 is normal speed,
   and anything larger is the caller's interval for slow playback or
   strobe.  MPG_MvFreeze does the same for the freeze field.
*/
#define MPEG_INTVL_HOLD   0
#define MPEG_INTVL_NORMAL 1

/*
   SET_VIDEO_EFFECTS fields.  SBL CDC_MPITP_* / CDC_MPTRP_* /
   CDC_MPSOFT_*.  Interpolation and soften are per-axis switches; the
   transparent-bit field is a luminance key with the threshold in its
   low two bits; mosaic is a per-axis ratio.
*/
enum
{
 MPEG_ITP_YH = 0x01, /* interpolate luma horizontally   */
 MPEG_ITP_CH = 0x02, /* interpolate chroma horizontally */
 MPEG_ITP_YV = 0x04, /* interpolate luma vertically     */
 MPEG_ITP_CV = 0x08, /* interpolate chroma vertically   */

 MPEG_TRP_DFL = 0x00, /* no luminance keying   */
 MPEG_TRP_64  = 0x01, /* key below luma 64     */
 MPEG_TRP_128 = 0x02, /* key below luma 128    */
 MPEG_TRP_256 = 0x03, /* key below luma 256    */
 MPEG_TRP_MASK = 0x03,
 MPEG_TRP_MAG = 0x04, /* enlarge the keyed region */

 MPEG_SOFT_ON = 0x01  /* soften along this axis */
};

/* CDC_NUL_SEL: no selector / no partition. */
#define MPEG_NUL_SEL 0xFF

/*
   SET_IMAGE sub-function, in CR1's low byte.  CDC_MpSetImgPos and
   CDC_MpSetImgSiz tail-call one builder (cdc_mfb.o's cmdRspSetImg)
   which emits opcode 0xA6 with this selector, the frame buffer number
   in CR2's low byte, x in CR3 and y in CR4 -- the same shape as the
   window command.
*/
enum
{
 MPEG_IMG_POS = 0, /* CDC_MpSetImgPos */
 MPEG_IMG_SIZ = 1  /* CDC_MpSetImgSiz */
};

/*
   Frame buffers.  The card has several, selected by the fbn field of
   DISPLAY, SET_IMAGE, READ_IMAGE and WRITE_IMAGE.  Four is what the
   image commands' single-byte field and the Video CD player's usage
   suggest; nothing observed needs more.
*/
#define MPEG_NUM_FBUF 4

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
void MPEG_FeedSector(const uint8_t *data, uint32_t len, uint8_t submode) MDFN_HOT;

/*
   HIRQ bits the card has accumulated since the last call, cleared by
   reading.  Polled by the CD block after MPEG_Update(): the card raises
   HIRQ_MPST when an unmasked interrupt factor becomes pending, and
   nothing else in the CD block is watching the decoders closely enough
   to notice on its own.
*/
uint16_t MPEG_TakePendingHIRQ(void);

/* Bytes of demultiplexed elementary stream currently buffered, for the
   CD block's buffer-full/backpressure decisions and for tests. */
uint32_t MPEG_GetESFill(bool is_video);

/* Bytes of elementary stream dropped because a FIFO was full, since the
   last reset.  Nonzero means the demuxer outran a decoder, which under
   the real sector cadence should not happen -- treat it as a bug signal
   rather than an expected condition. */
uint32_t MPEG_GetESDropped(bool is_video);

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
   True when sectors destined for CD block buffer partition pnum should
   be handed to the card.

   SET_CONNECTION's third field per stream is CdcMpCon.bn, the buffer
   partition number the decoder draws from -- not a filter number, which
   is what this predicate used to test.  CDC_NUL_SEL (0xFF) means no
   connection and is the reset value for both streams, so a disc that
   never configures the card never routes anything to it.
*/
bool MPEG_WantsPartition(uint8_t pnum);

/*
   True when the card consumes sectors it takes rather than leaving them
   in the partition, i.e. CDC_MPCMOD_DEL is set in the connection mode
   for the stream that partition feeds.
*/
bool MPEG_ConsumesPartition(uint8_t pnum);

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
   Pull exactly one stereo sample pair for the SCSP's external audio
   input, the same input CD-DA feeds.  Returns false when the card is
   not producing audio, in which case the caller should fall back to
   CD-DA and out[] is untouched.

   Called once per SCSP sample, i.e. at 44100 Hz.  Video CD audio is
   44.1 kHz by specification, so the common path is a straight 1:1 pull;
   the rate conversion below it exists only for the other MPEG-1 rates
   and is a zero-order hold, chosen because it is exactly reproducible
   rather than because it sounds good.
*/
bool MPEG_GetAudioSample(uint16_t *out) MDFN_HOT;

/*
   Most recently decoded picture, as packed 16bpp RGB555 in the Saturn's
   native component order, ready for VDP2 external-background
   compositing.  Returns NULL when no picture has been decoded.
*/
const uint16_t *MPEG_GetFrame(uint32_t *width, uint32_t *height);

/*
   Luma plane of the same picture, one byte per pixel at the same stride
   as MPEG_GetFrame().  Kept alongside the converted RGB because the
   luminance key in SET_VIDEO_EFFECTS thresholds on luma and can change
   after a picture has been converted, so recovering it from the RGB
   would be both lossy and wrong.
*/
const uint8_t *MPEG_GetFrameLuma(void);

/* Display geometry/appearance state, for the VDP2 compositor. */
typedef struct
{
 bool     enabled;      /* DISPLAY, CR2's high byte               */
 uint8_t  fbn;          /* DISPLAY, frame buffer number           */
 uint16_t x, y;         /* DPOS -- display origin                 */
 uint16_t w, h;         /* DSIZ -- display size                   */
 uint16_t src_x, src_y; /* FPOS -- source origin in the frame     */
 uint16_t ofs_x, ofs_y; /* DOFS -- display offset                 */
 uint16_t rat_x, rat_y; /* FRAT -- source scaling ratio           */
 uint16_t border_color; /* RGB555                                 */
 uint8_t  fade;         /* SET_FADE luma gain, 0x00..0xFF         */
 uint8_t  fade_c;       /* SET_FADE chroma gain                   */

 /* SET_VIDEO_EFFECTS */
 uint8_t  itp;          /* interpolation switches, MPEG_ITP_*     */
 uint8_t  trp;          /* luminance key, MPEG_TRP_*              */
 uint8_t  moz_h, moz_v; /* mosaic ratio per axis, 0 = off         */
 uint8_t  soft_h, soft_v; /* soften switches, MPEG_SOFT_*         */
} MPEG_DisplayState;

const MPEG_DisplayState *MPEG_GetDisplayState(void);

#ifdef __cplusplus
}
#endif

#endif
