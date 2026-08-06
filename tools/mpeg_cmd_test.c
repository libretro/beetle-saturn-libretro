/*
   Standalone regression oracle for the Video CD Card command surface.

   Links mednafen/ss/mpeg.c against stubs for the savestate and VFS
   layers so the CD-block-visible behaviour can be exercised without
   standing up a whole Saturn.

   Build:
     cc -std=gnu89 -Wall -Wextra -Wdeclaration-after-statement -g \
        -I. -Imednafen -Imednafen/include -Ilibretro-common/include \
        -D__LIBRETRO__ \
        tools/mpeg_cmd_test.c mednafen/ss/mpeg.c -o /tmp/mpeg_cmd_test
     /tmp/mpeg_cmd_test

   The end-to-end decode section is skipped unless MPEG_TEST_STREAM
   points at a Video CD elementary file. Generate one with:

     ffmpeg -f lavfi -i testsrc=size=352x240:rate=30000/1001:duration=1.5 \
            -f lavfi -i sine=frequency=440:sample_rate=44100:duration=1.5 \
            -target ntsc-vcd vcd.mpg

   The colour conversion was cross-checked against ffmpeg's own decode
   of the same picture: mean absolute error 0.73/255, max 9, with 0.01%
   of samples differing by more than 8. That residual is RGB555
   quantisation plus the chroma upsampler -- swscale interpolates, this
   replicates, matching what the card did at 352x240.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "mednafen/ss/mpeg.h"

/* ---- stubs ------------------------------------------------------- */

int MDFNSS_StateAction(void *st, int load, int data_only,
                       SFORMAT *sf, const char *name, bool optional)
{
   (void)st; (void)load; (void)data_only; (void)sf; (void)name; (void)optional;
   return 1;
}

/* The harness never opens a real ROM, so these only need to link. */
int64_t filestream_seek(RFILE *stream, int64_t offset, int seek_position)
{
   (void)stream; (void)offset; (void)seek_position;
   return -1;
}

int64_t filestream_tell(RFILE *stream)
{
   (void)stream;
   return -1;
}

int64_t filestream_read(RFILE *stream, void *data, int64_t len)
{
   (void)stream; (void)data; (void)len;
   return -1;
}

/* ---- harness ----------------------------------------------------- */

static int failures;
static int checks;

static void expect_eq(const char *what, unsigned long got, unsigned long want)
{
   checks++;
   if(got != want)
   {
      failures++;
      printf("  FAIL %-42s got 0x%04lX want 0x%04lX\n", what, got, want);
   }
}

static void expect_true(const char *what, int cond)
{
   checks++;
   if(!cond)
   {
      failures++;
      printf("  FAIL %-42s\n", what);
   }
}

/* ---- MPEG-1 Program Stream fixture ------------------------------- */

/* A minimal but structurally valid ISO/IEC 11172-1 stream: one pack
   header, then a video packet with a PTS, an audio packet with a PTS, a
   padding packet, a second video packet without a PTS, and the end
   code.  Payload sizes are chosen to be distinctive so a mis-sliced
   packet shows up as a wrong byte count rather than a plausible one. */

#define PS_VIDEO_A_BYTES 1000
#define PS_VIDEO_B_BYTES  600
#define PS_AUDIO_BYTES    800
#define PS_VIDEO_BYTES   (PS_VIDEO_A_BYTES + PS_VIDEO_B_BYTES)

#define PS_VIDEO_PTS 0x00012345UL
#define PS_AUDIO_PTS 0x00012300UL

static uint8_t *put_startcode(uint8_t *p, uint8_t id)
{
   *p++ = 0x00; *p++ = 0x00; *p++ = 0x01; *p++ = id;
   return p;
}

/* 33-bit timestamp in the 5-byte marker-interleaved form.  tag is 0x02
   for a lone PTS. */
static uint8_t *put_ts(uint8_t *p, uint8_t tag, uint64_t ts)
{
   *p++ = (uint8_t)((tag << 4) | (((ts >> 30) & 0x07) << 1) | 1);
   *p++ = (uint8_t)((ts >> 22) & 0xFF);
   *p++ = (uint8_t)((((ts >> 15) & 0x7F) << 1) | 1);
   *p++ = (uint8_t)((ts >> 7) & 0xFF);
   *p++ = (uint8_t)(((ts & 0x7F) << 1) | 1);
   return p;
}

static uint8_t *put_packet(uint8_t *p, uint8_t stream_id,
                           uint64_t pts, size_t payload, uint8_t fill)
{
   size_t hdr = (pts != RMPEG1_PS_NO_PTS) ? 5 : 1;
   size_t len = payload + hdr;
   size_t i;

   p = put_startcode(p, stream_id);
   *p++ = (uint8_t)(len >> 8);
   *p++ = (uint8_t)(len & 0xFF);

   if(pts != RMPEG1_PS_NO_PTS)
      p = put_ts(p, 0x02, pts);
   else
      *p++ = 0x0F;   /* no PTS/DTS */

   for(i = 0; i < payload; i++)
      *p++ = fill;

   return p;
}

static size_t build_ps(uint8_t *buf, size_t cap)
{
   uint8_t *p = buf;
   size_t i;

   (void)cap;

   /* pack header: start code, SCR (tag 0x02), mux_rate */
   p = put_startcode(p, 0xBA);
   p = put_ts(p, 0x02, 0x00012000UL);
   *p++ = 0x80 | ((3528 >> 15) & 0x7F);
   *p++ = (uint8_t)((3528 >> 7) & 0xFF);
   *p++ = (uint8_t)(((3528 & 0x7F) << 1) | 1);

   p = put_packet(p, 0xE0, PS_VIDEO_PTS, PS_VIDEO_A_BYTES, 0x11);
   p = put_packet(p, 0xC0, PS_AUDIO_PTS, PS_AUDIO_BYTES,   0x22);

   /* padding packet -- must be consumed and never surface */
   p = put_startcode(p, 0xBE);
   *p++ = 0x00; *p++ = 0x20;
   for(i = 0; i < 0x20; i++)
      *p++ = 0xFF;

   p = put_packet(p, 0xE0, RMPEG1_PS_NO_PTS, PS_VIDEO_B_BYTES, 0x33);

   p = put_startcode(p, 0xB9);   /* ISO_11172_end_code */

   return (size_t)(p - buf);
}

/* ---- harness state ----------------------------------------------- */

static uint16_t Out[4];
static uint16_t Hirq;

static bool Cmd(uint8_t op, uint16_t c1, uint16_t c2, uint16_t c3, uint16_t c4)
{
   uint16_t cd[4];

   cd[0] = c1; cd[1] = c2; cd[2] = c3; cd[3] = c4;

   memset(Out, 0, sizeof(Out));
   Hirq = 0;

   return MPEG_Command(op, cd, 0x01 /* STATUS_PAUSE */, Out, &Hirq);
}

int main(void)
{
   printf("MPEG card command surface\n");

   /* --- 1. No card: every opcode in range answers 0xFF/invalid ----- */
   printf("[no card]\n");
   expect_true("GET_STATUS claimed",      Cmd(MPEG_CMD_GET_STATUS, 0, 0, 0, 0));
   expect_eq  ("  CR1",  Out[0], 0xFF00);
   expect_eq  ("  CR2",  Out[1], 0xFFFF);
   expect_eq  ("  HIRQ", Hirq,   MPEG_HIRQ_MPCM);
   expect_true("non-MPEG opcode not claimed", !Cmd(0x02, 0, 0, 0, 0));
   expect_eq  ("auth reports 0 with no card", MPEG_GetAuth(), 0);

   /* --- 2. Card present, no firmware ------------------------------ */
   printf("[card present, no firmware ROM]\n");
   expect_true("MPEG_Init", MPEG_Init(NULL));
   expect_true("IsPresent", MPEG_IsPresent());

   MPEG_Auth();
   expect_eq("auth refused without ROM", MPEG_GetAuth(), 0);

   /* INIT reports 0xFF00 while unauthenticated. */
   Cmd(MPEG_CMD_INIT, 0, 0, 0, 0);
   expect_eq("INIT CR1 unauthenticated", Out[0], 0xFF00);
   expect_eq("INIT HIRQ (hard)", Hirq, MPEG_HIRQ_MPED | MPEG_HIRQ_MPST);

   Cmd(MPEG_CMD_INIT, 0, 1, 0, 0);
   expect_eq("INIT HIRQ (soft, CR2 bit0)", Hirq,
             MPEG_HIRQ_MPED | MPEG_HIRQ_MPST | MPEG_HIRQ_MPCM);

   /* --- 3. Status report layout ----------------------------------- */
   printf("[status report]\n");
   Cmd(MPEG_CMD_GET_STATUS, 0, 0, 0, 0);
   /* Low byte is the MPEG operation status: video stopped, decode
      stopped, audio stopped.  SBL CDC_MPASTV_STOP | CDC_MPASTD_STOP |
      CDC_MPASTA_STOP. */
   expect_eq("CR1 = status<<8 | action", Out[0],
             0x0100 | MPEG_ASTV_STOP | MPEG_ASTD_STOP | MPEG_ASTA_STOP);
   expect_eq("HIRQ", Hirq, MPEG_HIRQ_MPCM);

   /* --- 4. Interrupt mask round trip + read-to-clear --------------- */
   printf("[interrupt]\n");
   Cmd(MPEG_CMD_SET_INT_MASK, 0x00AB, 0xCDEF, 0, 0);
   expect_eq("SET_INT_MASK CR1 report", Out[0],
             0x0100 | MPEG_ASTV_STOP | MPEG_ASTD_STOP | MPEG_ASTA_STOP);

   Cmd(MPEG_CMD_GET_INTERRUPT, 0, 0, 0, 0);
   expect_eq("GET_INTERRUPT CR1 (no flags)", Out[0], 0x0100);
   expect_eq("GET_INTERRUPT CR2 (no flags)", Out[1], 0x0000);

   /* --- 5. Connection round trip, both banks ---------------------- */
   printf("[connection]\n");
   Cmd(MPEG_CMD_SET_CONNECTION, 0x0011, 0x2233, 0x0044, 0x5566);
   Cmd(MPEG_CMD_GET_CONNECTION, 0, 0, 0x0000, 0);
   expect_eq("cur CR1 audcon",          Out[0], 0x0111);
   expect_eq("cur CR2 audlay|audbufnum", Out[1], 0x2233);
   expect_eq("cur CR3 vidcon",          Out[2], 0x0044);
   expect_eq("cur CR4 vidlay|vidbufnum", Out[3], 0x5566);

   /* Bank select lives in CR3's high byte; writing "next" must not
      disturb "current". */
   Cmd(MPEG_CMD_SET_CONNECTION, 0x0077, 0x8899, 0x01AA, 0xBBCC);
   Cmd(MPEG_CMD_GET_CONNECTION, 0, 0, 0x0100, 0);
   expect_eq("next CR1", Out[0], 0x0177);
   expect_eq("next CR2", Out[1], 0x8899);
   expect_eq("next CR3", Out[2], 0x00AA);
   expect_eq("next CR4", Out[3], 0xBBCC);

   Cmd(MPEG_CMD_GET_CONNECTION, 0, 0, 0x0000, 0);
   expect_eq("current bank undisturbed", Out[1], 0x2233);

   /* --- 6. Stream round trip -------------------------------------- */
   printf("[stream]\n");
   Cmd(MPEG_CMD_SET_STREAM, 0x000E, 0x0102, 0x00E0, 0x0304);
   Cmd(MPEG_CMD_GET_STREAM, 0, 0, 0x0000, 0);
   expect_eq("cur CR1 audstm", Out[0], 0x010E);
   expect_eq("cur CR2",        Out[1], 0x0102);
   expect_eq("cur CR3 vidstm", Out[2], 0x00E0);
   expect_eq("cur CR4",        Out[3], 0x0304);

   /* --- 7. Display state ------------------------------------------ */
   printf("[display]\n");
   {
      const MPEG_DisplayState *d;

      /* CDC_MpDisp(dspsw, fbn): switch in CR2's high byte, frame buffer
         number in the low byte.  A write to CR1 must do nothing -- that
         was the old misreading, and it kept the display permanently
         off. */
      Cmd(MPEG_CMD_DISPLAY, 0x0001, 0x0000, 0, 0);
      d = MPEG_GetDisplayState();
      expect_true("CR1 does not enable display", !d->enabled);

      Cmd(MPEG_CMD_DISPLAY, 0x0000, 0x0102, 0, 0);
      expect_true("CR2 high byte enables display", d->enabled);
      expect_eq  ("frame buffer number", d->fbn, 0x02);

      Cmd(MPEG_CMD_DISPLAY, 0x0000, 0x0000, 0, 0);
      expect_true("display disabled", !d->enabled);

      /* One opcode, five sub-functions selected by CR1's low byte;
         x in CR3, y in CR4. */
      Cmd(MPEG_CMD_SET_WINDOW, MPEG_WIN_FPOS, 0x0001, 0x0011, 0x0022);
      expect_eq("FPOS src_x", d->src_x, 0x11);
      expect_eq("FPOS src_y", d->src_y, 0x22);

      Cmd(MPEG_CMD_SET_WINDOW, MPEG_WIN_DPOS, 0x0001, 0x0033, 0x0044);
      expect_eq("DPOS x", d->x, 0x33);
      expect_eq("DPOS y", d->y, 0x44);

      Cmd(MPEG_CMD_SET_WINDOW, MPEG_WIN_DSIZ, 0x0001, 0x0140, 0x00F0);
      expect_eq("DSIZ w", d->w, 0x140);
      expect_eq("DSIZ h", d->h, 0x0F0);

      Cmd(MPEG_CMD_SET_WINDOW, MPEG_WIN_DOFS, 0x0001, 0x0055, 0x0066);
      expect_eq("DOFS x", d->ofs_x, 0x55);
      expect_eq("DOFS y", d->ofs_y, 0x66);

      Cmd(MPEG_CMD_SET_WINDOW, MPEG_WIN_FRAT, 0x0001, 0x0077, 0x0088);
      expect_eq("FRAT x", d->rat_x, 0x77);
      expect_eq("FRAT y", d->rat_y, 0x88);

      /* Each sub-function must touch only its own pair. */
      expect_eq("FPOS untouched by later writes", d->src_x, 0x11);
      expect_eq("DPOS untouched by later writes", d->x,     0x33);

      Cmd(MPEG_CMD_SET_BORDERCOL, 0, 0x7C1F, 0, 0);
      expect_eq("border color", d->border_color, 0x7C1F);

      /* CDC_MpSetFade(gain_y, gain_c). */
      Cmd(MPEG_CMD_SET_FADE, 0, 0x8040, 0, 0);
      expect_eq("fade luma",   d->fade,   0x80);
      expect_eq("fade chroma", d->fade_c, 0x40);
   }

   /* --- 8. SET_MODE 0xFF-means-keep semantics --------------------- */
   printf("[mode]\n");
   Cmd(MPEG_CMD_SET_MODE, 0x0003, 0x0102, 0x0400, 0);
   Cmd(MPEG_CMD_SET_MODE, 0x00FF, 0xFFFF, 0xFF00, 0);
   /* No getter is exposed; the check is that the command is claimed and
      reports normally rather than clobbering to 0xFF and wedging. */
   expect_eq("SET_MODE report", Out[0],
             0x0100 | MPEG_ASTV_STOP | MPEG_ASTD_STOP | MPEG_ASTA_STOP);

   /* --- 9. Unimplemented in-range opcodes are benign no-ops ------- */
   printf("[unimplemented opcodes]\n");
   {
      unsigned op;
      for(op = 0x90; op <= 0xAF; op++)
      {
         expect_true("claimed", Cmd((uint8_t)op, 0, 0, 0, 0));
         if(op != MPEG_CMD_INIT)
            expect_true("reports non-rejected status", (Out[0] >> 8) != 0xFF);
      }
   }

   /* --- 10. Reset clears volatile state --------------------------- */
   printf("[reset]\n");
   MPEG_Reset(false);
   Cmd(MPEG_CMD_GET_CONNECTION, 0, 0, 0, 0);
   expect_eq("audbufnum back to 0xFF", Out[1] & 0xFF, 0xFF);
   expect_true("display off after reset", !MPEG_GetDisplayState()->enabled);

   /* --- 11. Program Stream demux --------------------------------- */
   printf("[program stream demux]\n");
   {
      static uint8_t ps[8192];
      static uint8_t sec[2324];
      size_t len;
      unsigned i;
      uint32_t vfill, afill;

      /* Default stream selection is 0x00, which after the SET_STREAM
         above is no longer "match anything" -- reset to a known state
         and select "don't care" on both. */
      MPEG_Reset(true);
      Cmd(MPEG_CMD_SET_STREAM, 0x0000, 0xFF00, 0x0000, 0xFF00);

      len = build_ps(ps, sizeof(ps));

      /* Feed it a sector at a time, the way the CD block would. */
      for(i = 0; i < len; i += 2324)
      {
         size_t chunk = len - i;
         if(chunk > 2324)
            chunk = 2324;
         MPEG_FeedSector(ps + i, (uint32_t)chunk, 0x00);
      }

      vfill = MPEG_GetESFill(true);
      afill = MPEG_GetESFill(false);

      expect_eq  ("video ES demuxed", vfill, PS_VIDEO_BYTES);
      expect_eq  ("audio ES demuxed", afill, PS_AUDIO_BYTES);
      expect_true("padding not buffered", vfill + afill == PS_VIDEO_BYTES + PS_AUDIO_BYTES);
      expect_eq  ("video PTS recovered", (unsigned long)MPEG_GetPTS(true),  PS_VIDEO_PTS);
      expect_eq  ("audio PTS recovered", (unsigned long)MPEG_GetPTS(false), PS_AUDIO_PTS);

      /* Substream selection must actually filter: ask for video
         stream 3 when the fixture carries stream 0. */
      MPEG_Reset(true);
      Cmd(MPEG_CMD_SET_STREAM, 0x0000, 0xFF00, 0x0000, 0xE300);

      for(i = 0; i < len; i += 2324)
      {
         size_t chunk = len - i;
         if(chunk > 2324)
            chunk = 2324;
         MPEG_FeedSector(ps + i, (uint32_t)chunk, 0x00);
      }

      expect_eq("unselected video stream dropped", MPEG_GetESFill(true), 0);
      expect_eq("audio still selected",            MPEG_GetESFill(false), PS_AUDIO_BYTES);

      /* Bare substream index form of the same selector. */
      MPEG_Reset(true);
      Cmd(MPEG_CMD_SET_STREAM, 0x0000, 0xFF00, 0x0000, 0x0000);

      for(i = 0; i < len; i += 2324)
      {
         size_t chunk = len - i;
         if(chunk > 2324)
            chunk = 2324;
         MPEG_FeedSector(ps + i, (uint32_t)chunk, 0x00);
      }

      expect_eq("bare index selects stream 0", MPEG_GetESFill(true), PS_VIDEO_BYTES);

      /* Mid-stream entry: start the feed inside a packet.  The parser
         must resynchronise rather than deadlock or mis-slice. */
      MPEG_Reset(true);
      Cmd(MPEG_CMD_SET_STREAM, 0x0000, 0xFF00, 0x0000, 0xFF00);
      MPEG_FeedSector(ps + 37, (uint32_t)(len - 37), 0x00);
      expect_true("resynced mid-stream", MPEG_GetESFill(true) > 0);

      /* Garbage must not fault and must not be mistaken for a stream. */
      MPEG_Reset(true);
      Cmd(MPEG_CMD_SET_STREAM, 0x0000, 0xFF00, 0x0000, 0xFF00);
      memset(sec, 0xA5, sizeof(sec));
      for(i = 0; i < 64; i++)
         MPEG_FeedSector(sec, sizeof(sec), 0x00);
      expect_eq("garbage yields no video ES", MPEG_GetESFill(true), 0);
      expect_eq("garbage yields no audio ES", MPEG_GetESFill(false), 0);

      /* Overrun: feed the fixture until the ES FIFOs are saturated.
         Writes must clamp, never wrap past the end. */
      for(i = 0; i < 400; i++)
         MPEG_FeedSector(ps, (uint32_t)(len > 2324 ? 2324 : len), 0x00);

      MPEG_FeedSector(NULL, 0, 0x00);
      MPEG_FeedSector(sec, 0, 0x00);

      expect_true("no frame yet", MPEG_RunFrame() == false);
      expect_true("no frame buffer yet", MPEG_GetFrame(NULL, NULL) == NULL);
   }

   /* --- 12. End-to-end decode against a real VCD stream ----------- */
   printf("[end-to-end decode]\n");
   {
      const char *path = getenv("MPEG_TEST_STREAM");
      FILE *fp = path ? fopen(path, "rb") : NULL;

      if(!fp)
         printf("  skipped (set MPEG_TEST_STREAM to a VCD .mpg)\n");
      else
      {
         static uint8_t sec[2324];
         uint32_t w = 0, h = 0;
         uint32_t rate = 0, ch = 0;
         unsigned frames = 0;
         unsigned audio_frames = 0;
         size_t got;
         const int64_t sector_period = ((int64_t)(11289600 / 75)) << 32;
         uint16_t last_vc = 0;

         MPEG_Reset(true);
         Cmd(MPEG_CMD_SET_STREAM, 0x0000, 0xFF00, 0x0000, 0xFF00);

         /* Feed at the real single-speed Video CD cadence -- 75
            sectors per second -- and clock the card between sectors,
            exactly as the CD block does. A tight feed loop would hide
            any imbalance between how fast the demuxer fills the ES
            FIFOs and how fast the decoders drain them. */
         while((got = fread(sec, 1, sizeof(sec), fp)) > 0)
         {
            int16_t pcm[4096 * 2];
            uint32_t n;

            MPEG_FeedSector(sec, (uint32_t)got, 0x00);
            MPEG_Update(sector_period);

            while((n = MPEG_ReadAudio(pcm, 4096)) > 0)
               audio_frames += n;

            /* VCounter advances once per decode period, so it is what
               says a new picture was due; MPEG_GetFrame() then says
               whether one actually came out. */
            Cmd(MPEG_CMD_GET_STATUS, 0, 0, 0, 0);

            if(Out[1] != last_vc)
            {
               last_vc = Out[1];

               if(MPEG_GetFrame(&w, &h))
                  frames++;
            }
         }

         fclose(fp);

         MPEG_GetAudioFormat(&rate, &ch);

         expect_eq  ("no video ES dropped", MPEG_GetESDropped(true),  0);
         expect_eq  ("no audio ES dropped", MPEG_GetESDropped(false), 0);
         expect_true("decoded at least one picture", frames > 0);
         expect_eq  ("picture width",  w, 352);
         expect_eq  ("picture height", h, 240);
         expect_true("decoded audio",  audio_frames > 0);
         expect_eq  ("audio rate",     rate, 44100);
         expect_eq  ("audio channels", ch, 2);

         printf("  %u pictures, %u audio frames, %ux%u @ %u Hz x%u\n",
                frames, audio_frames, w, h, rate, ch);

         /* The display window must have followed the decoded picture,
            since SET_WINDOW never supplied a size.  VDP2 compositing
            reads these, so a stale 352x240 default would letterboxed
            PAL content for no reason. */
         {
            const MPEG_DisplayState *d = MPEG_GetDisplayState();

            expect_eq("window width follows picture",  d->w, w);
            expect_eq("window height follows picture", d->h, h);
         }

         /* Colour conversion must stay inside RGB555. */
         {
            const uint16_t *fb = MPEG_GetFrame(&w, &h);
            unsigned i, bad = 0;

            if(fb)
            {
               for(i = 0; i < w * h; i++)
                  if(fb[i] & 0x8000)
                     bad++;
            }

            expect_eq("no bits above RGB555", bad, 0);
         }
      }
   }

   /* --- 13. Partition routing gate -------------------------------- */
   printf("[partition routing]\n");
   {
      unsigned p;

      MPEG_Reset(true);

      /* Both bn fields reset to CDC_NUL_SEL, so nothing routes. */
      for(p = 0; p < 0x18; p++)
         expect_true("no routing before SET_CONNECTION", !MPEG_WantsPartition((uint8_t)p));

      /* cmod is a flags field, not a selector: setting it alone must
         not make anything route.  This is what the old filter-number
         reading got wrong. */
      Cmd(MPEG_CMD_SET_CONNECTION, 0x0005, 0x00FF, 0x0007, 0x00FF);
      for(p = 0; p < 0x18; p++)
         expect_true("cmod alone routes nothing", !MPEG_WantsPartition((uint8_t)p));

      /* audio bn = 5, video bn = 7. */
      Cmd(MPEG_CMD_SET_CONNECTION, 0x0000, 0x0005, 0x0000, 0x0007);

      expect_true("video partition routes",  MPEG_WantsPartition(7));
      expect_true("audio partition routes",  MPEG_WantsPartition(5));
      expect_true("other partition does not", !MPEG_WantsPartition(6));
      expect_true("out-of-range rejected",   !MPEG_WantsPartition(0x18));
      expect_true("NUL_SEL rejected",        !MPEG_WantsPartition(MPEG_NUL_SEL));

      /* CDC_MPCMOD_DEL controls whether the card consumes the sector. */
      expect_true("not consumed without DEL", !MPEG_ConsumesPartition(7));

      Cmd(MPEG_CMD_SET_CONNECTION, 0x0000, 0x0005, 0x0004, 0x0007);
      expect_true("consumed with DEL on video", MPEG_ConsumesPartition(7));
      expect_true("audio still not consumed",  !MPEG_ConsumesPartition(5));

      /* The "next" bank must not affect routing. */
      Cmd(MPEG_CMD_SET_CONNECTION, 0x0000, 0x0009, 0x0100, 0x000B);
      expect_true("next bank does not route", !MPEG_WantsPartition(0x0B));
      expect_true("current bank still routes", MPEG_WantsPartition(7));

      MPEG_Reset(true);
      expect_true("reset clears routing", !MPEG_WantsPartition(7));
   }

   /* --- 14. Decode clock ------------------------------------------ */
   printf("[decode clock]\n");
   {
      /* CD block clocks are 32.32 fixed point at 11289600 Hz. One NTSC
         frame period is 11289600 * 1001 / 30000 clocks. */
      const int64_t frame_ntsc = ((int64_t)((11289600ULL * 1001) / 30000)) << 32;
      uint16_t vc0, vc1;

      MPEG_Reset(true);

      Cmd(MPEG_CMD_GET_STATUS, 0, 0, 0, 0);
      vc0 = Out[1];

      /* Just under a frame: nothing should tick. */
      MPEG_Update(frame_ntsc - 1);
      Cmd(MPEG_CMD_GET_STATUS, 0, 0, 0, 0);
      expect_eq("vcounter idle below one frame", Out[1], vc0);

      /* Cross the boundary. */
      MPEG_Update(2);
      Cmd(MPEG_CMD_GET_STATUS, 0, 0, 0, 0);
      expect_eq("vcounter ticks at one frame", Out[1], (uint16_t)(vc0 + 1));

      /* Three frames in one call, below the catch-up cap. */
      MPEG_Update(frame_ntsc * 3);
      Cmd(MPEG_CMD_GET_STATUS, 0, 0, 0, 0);
      vc1 = Out[1];
      expect_eq("three frames tick three times", vc1, (uint16_t)(vc0 + 4));

      /* A long stall must be capped, not replayed as a burst.  The cap
         is MPEG_CATCHUP_FRAMES (4), plus at most one more for whatever
         was already accumulated. */
      MPEG_Update(frame_ntsc * 1000);
      Cmd(MPEG_CMD_GET_STATUS, 0, 0, 0, 0);
      expect_true("stall capped, not replayed",
                  (uint16_t)(Out[1] - vc1) <= 5);
      expect_true("stall still advances", (uint16_t)(Out[1] - vc1) > 0);

      /* Negative and zero deltas are inert. */
      vc1 = Out[1];
      MPEG_Update(0);
      MPEG_Update(-frame_ntsc);
      Cmd(MPEG_CMD_GET_STATUS, 0, 0, 0, 0);
      expect_eq("non-positive delta inert", Out[1], vc1);
   }

   /* --- 15. SCSP audio pull --------------------------------------- */
   printf("[scsp pull]\n");
   {
      uint16_t smp[2];
      unsigned i;

      MPEG_Reset(true);

      /* Nothing decoded: the card must decline so CD-DA keeps the
         SCSP's external input. */
      smp[0] = 0xDEAD; smp[1] = 0xBEEF;
      expect_true("declines with no audio", !MPEG_GetAudioSample(smp));
      expect_eq  ("output untouched on decline", smp[0], 0xDEAD);

      /* Feed the real stream so the ring fills at 44.1 kHz. */
      if(getenv("MPEG_TEST_STREAM"))
      {
         FILE *fp = fopen(getenv("MPEG_TEST_STREAM"), "rb");

         if(fp)
         {
            static uint8_t sec[2324];
            size_t got;
            unsigned pulled = 0, nonzero = 0;
            uint32_t rate = 0, ch = 0;

            Cmd(MPEG_CMD_SET_STREAM, 0x0000, 0xFF00, 0x0000, 0xFF00);

            /* Enough sectors to get well past the first audio frame. */
            for(i = 0; i < 64 && (got = fread(sec, 1, sizeof(sec), fp)) > 0; i++)
            {
               MPEG_FeedSector(sec, (uint32_t)got, 0x00);
               MPEG_RunFrame();
            }

            fclose(fp);

            MPEG_GetAudioFormat(&rate, &ch);
            expect_eq("stream is 44.1 kHz", rate, 44100);

            /* At 44.1 kHz the pull is 1:1, so a thousand calls must
               all succeed off a ring this full. */
            for(i = 0; i < 1000; i++)
            {
               if(!MPEG_GetAudioSample(smp))
                  break;

               pulled++;

               if(smp[0] || smp[1])
                  nonzero++;
            }

            expect_eq  ("1000 pulls at 44.1 kHz", pulled, 1000);
            expect_true("audio is not silence",   nonzero > 0);

            /* Drain to underrun: the card must keep answering (holding
               the last sample) rather than declining mid-stream, which
               would hand the SCSP back to a stopped CD-DA path. */
            for(i = 0; i < 200000; i++)
               if(!MPEG_GetAudioSample(smp))
                  break;

            expect_true("holds through underrun", MPEG_GetAudioSample(smp));
         }
      }

      MPEG_Reset(true);
      expect_true("reset returns input to CD-DA", !MPEG_GetAudioSample(smp));
   }

   /* --- 14b. Layer selection -------------------------------------- */
   printf("[layer selection]\n");
   {
      static uint8_t ps[8192];
      static uint8_t raw[1024];
      size_t len;
      unsigned i;

      len = build_ps(ps, sizeof(ps));

      /* System layer (lay 0): the card demultiplexes, so a Program
         Stream yields both elementary streams. */
      MPEG_Reset(true);
      Cmd(MPEG_CMD_SET_STREAM, 0x0000, 0xFF00, 0x0000, 0xFF00);
      Cmd(MPEG_CMD_SET_CONNECTION, 0x0000, 0x0005, 0x0000, 0x0007);

      for(i = 0; i < len; i += 2324)
      {
         size_t chunk = len - i;
         if(chunk > 2324)
            chunk = 2324;
         MPEG_FeedSector(ps + i, (uint32_t)chunk, 0x00);
      }

      expect_eq("system layer demuxes video", MPEG_GetESFill(true),  PS_VIDEO_BYTES);
      expect_eq("system layer demuxes audio", MPEG_GetESFill(false), PS_AUDIO_BYTES);

      /* Video layer (lay 1): the payload is already an elementary
         stream, so it must bypass the demuxer and arrive verbatim.
         Feeding raw bytes through a Program Stream parser would find no
         pack headers and discard every one of them. */
      MPEG_Reset(true);
      Cmd(MPEG_CMD_SET_STREAM, 0x0000, 0xFF00, 0x0000, 0xFF00);
      Cmd(MPEG_CMD_SET_CONNECTION, 0x0000, 0xFF00 | MPEG_NUL_SEL,
                                   0x0000, 0x0100 | 0x0007);

      memset(raw, 0x5A, sizeof(raw));
      MPEG_FeedSector(raw, sizeof(raw), 0x00);

      expect_eq("video layer bypasses demux", MPEG_GetESFill(true), sizeof(raw));
      expect_eq("nothing leaked to audio",    MPEG_GetESFill(false), 0);

      /* The picture-search bits in lay must not change the layer. */
      MPEG_Reset(true);
      Cmd(MPEG_CMD_SET_CONNECTION, 0x0000, 0xFF00 | MPEG_NUL_SEL,
                                   0x0000, (MPEG_SRCH_VIDEO << 8) | 0x0007);

      for(i = 0; i < len; i += 2324)
      {
         size_t chunk = len - i;
         if(chunk > 2324)
            chunk = 2324;
         MPEG_FeedSector(ps + i, (uint32_t)chunk, 0x00);
      }

      expect_eq("search bits do not select ES layer",
                MPEG_GetESFill(true), PS_VIDEO_BYTES);
   }

   /* --- 15b. Picture size query ----------------------------------- */
   printf("[picture size]\n");
   {
      /* CDC_MpGetPictSiz: horizontal in CR3, vertical in CR4. */
      MPEG_Reset(true);
      Cmd(MPEG_CMD_GET_PICT_SIZE, 0, 0, 0, 0);
      expect_eq("no picture yet: width",  Out[2], 0);
      expect_eq("no picture yet: height", Out[3], 0);

      if(getenv("MPEG_TEST_STREAM"))
      {
         FILE *fp = fopen(getenv("MPEG_TEST_STREAM"), "rb");

         if(fp)
         {
            static uint8_t sec[2324];
            size_t got;
            unsigned i;

            Cmd(MPEG_CMD_SET_STREAM, 0x0000, 0xFF00, 0x0000, 0xFF00);

            for(i = 0; i < 64 && (got = fread(sec, 1, sizeof(sec), fp)) > 0; i++)
            {
               MPEG_FeedSector(sec, (uint32_t)got, 0x00);
               MPEG_RunFrame();
            }

            fclose(fp);

            Cmd(MPEG_CMD_GET_PICT_SIZE, 0, 0, 0, 0);
            expect_eq("picture width",  Out[2], 352);
            expect_eq("picture height", Out[3], 240);
         }
      }
   }

   /* --- 15c. Timestamp queries ------------------------------------ */
   printf("[timestamps]\n");
   {
      static uint8_t ps[8192];
      size_t len;
      unsigned i;

      MPEG_Reset(true);
      Cmd(MPEG_CMD_SET_STREAM, 0x0000, 0xFF00, 0x0000, 0xFF00);

      /* Nothing decoded: both must answer zero rather than garbage. */
      Cmd(MPEG_CMD_GET_PTS, 0, 0, 0, 0);
      expect_eq("no PTS yet (hi)", Out[2], 0);
      expect_eq("no PTS yet (lo)", Out[3], 0);

      len = build_ps(ps, sizeof(ps));

      for(i = 0; i < len; i += 2324)
      {
         size_t chunk = len - i;
         if(chunk > 2324)
            chunk = 2324;
         MPEG_FeedSector(ps + i, (uint32_t)chunk, 0x00);
      }

      /* CDC_MpGetPts returns the audio PTS as one 32-bit value across
         CR3:CR4. */
      Cmd(MPEG_CMD_GET_PTS, 0, 0, 0, 0);
      expect_eq("audio PTS high half", Out[2], (uint16_t)(PS_AUDIO_PTS >> 16));
      expect_eq("audio PTS low half",  Out[3], (uint16_t)PS_AUDIO_PTS);

      /* Timecode is derived from the video PTS: 0x12345 / 90000 is 0
         seconds, so h:m:s must all be zero and only the picture index
         can be non-zero. */
      Cmd(MPEG_CMD_GET_TIMECODE, 0, 0, 0, 0);
      expect_eq("bank",        Out[0] & 0xFF, 0);
      expect_eq("hour",        Out[2] >> 8,   0);
      expect_eq("minute",      Out[2] & 0xFF, 0);
      expect_eq("second",      Out[3] >> 8,   0);

      /* A PTS an exact number of seconds in must come back as that
         many seconds with picture 0.  Build a stream whose video PTS
         is 90000 * 3661 -- one hour, one minute, one second. */
      {
         static uint8_t ps2[8192];
         uint8_t *p = ps2;
         const uint64_t pts = (uint64_t)90000 * 3661;

         MPEG_Reset(true);
         Cmd(MPEG_CMD_SET_STREAM, 0x0000, 0xFF00, 0x0000, 0xFF00);

         p = put_startcode(p, 0xBA);
         p = put_ts(p, 0x02, 0);
         *p++ = 0x80; *p++ = 0x00; *p++ = 0x01;
         p = put_packet(p, 0xE0, pts, 64, 0x11);

         MPEG_FeedSector(ps2, (uint32_t)(p - ps2), 0x00);

         Cmd(MPEG_CMD_GET_TIMECODE, 0, 0, 0, 0);
         expect_eq("hour 1",    Out[2] >> 8,   1);
         expect_eq("minute 1",  Out[2] & 0xFF, 1);
         expect_eq("second 1",  Out[3] >> 8,   1);
         expect_eq("picture 0", Out[3] & 0xFF, 0);
      }
   }

   /* --- 15d. Image window ----------------------------------------- */
   printf("[image window]\n");
   {
      MPEG_Reset(true);

      /* Unconfigured: GET_IMAGE reports the whole decoded picture,
         which is zero before anything decodes. */
      Cmd(MPEG_CMD_GET_IMAGE, 0, 0, 0, 0);
      expect_eq("no image yet (hi)", Out[0] & 0xFF, 0);
      expect_eq("no image yet (lo)", Out[1], 0);

      /* SET_IMAGE: selector in CR1's low byte, frame buffer in CR2's
         low byte, x in CR3 and y in CR4. */
      Cmd(MPEG_CMD_SET_IMAGE, MPEG_IMG_SIZ, 0x0000, 352, 240);
      Cmd(MPEG_CMD_GET_IMAGE, 0, 0, 0, 0);

      /* 352*240 luma plus two 176*120 chroma planes, in longwords. */
      {
         const uint32_t expect_dw = (352u * 240u + 2u * 176u * 120u) / 4u;

         expect_eq("transfer size hi", Out[0] & 0xFF, (expect_dw >> 16) & 0xFF);
         expect_eq("transfer size lo", Out[1], expect_dw & 0xFFFF);
      }

      /* A second frame buffer keeps its own window. */
      Cmd(MPEG_CMD_SET_IMAGE, MPEG_IMG_SIZ, 0x0001, 176, 120);
      Cmd(MPEG_CMD_GET_IMAGE, 0, 0, 0, 0);
      {
         const uint32_t expect_dw = (176u * 120u + 2u * 88u * 60u) / 4u;

         expect_eq("second buffer size", Out[1], expect_dw & 0xFFFF);
      }

      /* Selecting the first again must report its own size back, not
         the one just written. */
      Cmd(MPEG_CMD_SET_IMAGE, MPEG_IMG_POS, 0x0000, 16, 32);
      Cmd(MPEG_CMD_GET_IMAGE, 0, 0, 0, 0);
      {
         const uint32_t expect_dw = (352u * 240u + 2u * 176u * 120u) / 4u;

         expect_eq("first buffer size preserved", Out[1], expect_dw & 0xFFFF);
      }

      /* Odd dimensions must round the chroma planes up, not down. */
      Cmd(MPEG_CMD_SET_IMAGE, MPEG_IMG_SIZ, 0x0002, 3, 3);
      Cmd(MPEG_CMD_GET_IMAGE, 0, 0, 0, 0);
      expect_eq("odd dimensions round up", Out[1], (3u * 3u + 2u * 2u * 2u + 3u) / 4u);

      /* An out-of-range frame buffer must be ignored, not written past
         the end of the array. */
      Cmd(MPEG_CMD_SET_IMAGE, MPEG_IMG_SIZ, 0x00FF, 640, 480);
      Cmd(MPEG_CMD_GET_IMAGE, 0, 0, 0, 0);
      expect_eq("out-of-range fbn ignored", Out[1], (3u * 3u + 2u * 2u * 2u + 3u) / 4u);

      /* READ_IMAGE and WRITE_IMAGE are recognised but unimplemented;
         they must answer with a normal status rather than a rejection. */
      Cmd(MPEG_CMD_READ_IMAGE, 0, 0, 0, 0);
      expect_true("READ_IMAGE not rejected", (Out[0] >> 8) != 0xFF);
      Cmd(MPEG_CMD_WRITE_IMAGE, 0, 0, 0, 0);
      expect_true("WRITE_IMAGE not rejected", (Out[0] >> 8) != 0xFF);
   }

   /* --- 15e. Pause, freeze and mute -------------------------------- */
   printf("[pause/freeze/mute]\n");
   if(getenv("MPEG_TEST_STREAM"))
   {
      const int64_t frame_ntsc = ((int64_t)((11289600ULL * 1001) / 30000)) << 32;
      static uint8_t buf[300000];
      size_t len;
      FILE *fp = fopen(getenv("MPEG_TEST_STREAM"), "rb");

      if(!fp)
         printf("  skipped\n");
      else
      {
         unsigned i, frames;
         uint16_t last_vc;

         len = fread(buf, 1, sizeof(buf), fp);
         fclose(fp);

         /* Normal speed: a picture per decode period. */
         MPEG_Reset(true);
         Cmd(MPEG_CMD_SET_STREAM, 0x0000, 0xFF00, 0x0000, 0xFF00);

         for(i = 0; i + 2324 <= len && i < 2324 * 60; i += 2324)
            MPEG_FeedSector(buf + i, 2324, 0x00);

         frames = 0;
         for(i = 0; i < 20; i++)
         {
            MPEG_Update(frame_ntsc);
            if(MPEG_GetFrame(NULL, NULL))
               frames++;
         }
         expect_true("normal speed produces pictures", frames > 0);

         /* Pause: the decoder holds, but VCounter must keep running --
            it is the card's vertical counter, not a picture counter,
            and software polls it to see the card is alive. */
         Cmd(MPEG_CMD_GET_STATUS, 0, 0, 0, 0);
         last_vc = Out[1];

         Cmd(MPEG_CMD_SET_DECMETHOD, 0x00FF, MPEG_INTVL_HOLD, 0, MPEG_NOCHG16);
         Cmd(MPEG_CMD_GET_STATUS, 0, 0, 0, 0);
         expect_true("pause status bit set", (Out[3] & MPEG_STV_PAUSE) != 0);

         for(i = 0; i < 10; i++)
            MPEG_Update(frame_ntsc);

         Cmd(MPEG_CMD_GET_STATUS, 0, 0, 0, 0);
         expect_eq("vcounter runs while paused", Out[1], (uint16_t)(last_vc + 10));

         /* Resuming must clear the status bit. */
         Cmd(MPEG_CMD_SET_DECMETHOD, 0x00FF, MPEG_INTVL_NORMAL, 0, MPEG_NOCHG16);
         Cmd(MPEG_CMD_GET_STATUS, 0, 0, 0, 0);
         expect_true("pause bit cleared", (Out[3] & MPEG_STV_PAUSE) == 0);

         /* Freeze is a separate field and must not be disturbed by a
            pause-only write, nor vice versa. */
         Cmd(MPEG_CMD_SET_DECMETHOD, 0x00FF, MPEG_NOCHG16, 0, MPEG_INTVL_HOLD);
         Cmd(MPEG_CMD_GET_STATUS, 0, 0, 0, 0);
         expect_true("freeze bit set",       (Out[3] & MPEG_STV_FREEZE) != 0);
         expect_true("pause still cleared",  (Out[3] & MPEG_STV_PAUSE) == 0);

         Cmd(MPEG_CMD_SET_DECMETHOD, 0x00FF, MPEG_INTVL_HOLD, 0, MPEG_NOCHG16);
         Cmd(MPEG_CMD_GET_STATUS, 0, 0, 0, 0);
         expect_true("freeze survives a pause write", (Out[3] & MPEG_STV_FREEZE) != 0);
      }
   }

   printf("[mute]\n");
   {
      int16_t pcm[64 * 2];
      uint16_t smp[2];
      uint32_t n;
      unsigned i;
      int nonzero_l, nonzero_r;

      /* Drive the ring directly through a decode so there is audio to
         mute, then check each channel independently. */
      if(getenv("MPEG_TEST_STREAM"))
      {
         FILE *fp = fopen(getenv("MPEG_TEST_STREAM"), "rb");

         if(fp)
         {
            static uint8_t sec[2324];
            size_t got;

            MPEG_Reset(true);
            Cmd(MPEG_CMD_SET_STREAM, 0x0000, 0xFF00, 0x0000, 0xFF00);

            for(i = 0; i < 64 && (got = fread(sec, 1, sizeof(sec), fp)) > 0; i++)
            {
               MPEG_FeedSector(sec, (uint32_t)got, 0x00);
               MPEG_RunFrame();
            }
            fclose(fp);

            /* Default: nothing muted. */
            n = MPEG_ReadAudio(pcm, 64);
            nonzero_l = nonzero_r = 0;
            for(i = 0; i < n; i++)
            {
               if(pcm[i * 2 + 0]) nonzero_l = 1;
               if(pcm[i * 2 + 1]) nonzero_r = 1;
            }
            expect_true("left audible by default",  nonzero_l);
            expect_true("right audible by default", nonzero_r);

            /* Mute left only. */
            Cmd(MPEG_CMD_SET_DECMETHOD, MPEG_MUT_L, MPEG_NOCHG16, 0, MPEG_NOCHG16);
            n = MPEG_ReadAudio(pcm, 64);
            nonzero_l = nonzero_r = 0;
            for(i = 0; i < n; i++)
            {
               if(pcm[i * 2 + 0]) nonzero_l = 1;
               if(pcm[i * 2 + 1]) nonzero_r = 1;
            }
            expect_true("left muted",        !nonzero_l);
            expect_true("right still audible", nonzero_r);

            /* The SCSP pull path must mute too, not just the host one. */
            expect_true("scsp pull answers", MPEG_GetAudioSample(smp));
            expect_eq  ("scsp left muted",   smp[0], 0);

            /* The default bit overrides the channel bits. */
            Cmd(MPEG_CMD_SET_DECMETHOD, MPEG_MUT_DFL | MPEG_MUT_L,
                MPEG_NOCHG16, 0, MPEG_NOCHG16);
            n = MPEG_ReadAudio(pcm, 64);
            nonzero_l = 0;
            for(i = 0; i < n; i++)
               if(pcm[i * 2 + 0]) nonzero_l = 1;
            expect_true("default bit overrides channel mute", nonzero_l);
         }
      }
   }

   /* --- 15f. Video effects ----------------------------------------- */
   printf("[video effects]\n");
   {
      const MPEG_DisplayState *d;

      MPEG_Reset(true);
      d = MPEG_GetDisplayState();

      expect_eq("effects clear after reset", d->itp | d->trp | d->moz_h
                                           | d->moz_v | d->soft_h | d->soft_v, 0);

      /* CDC_MpSetVeff(itp, trp, moz_h, moz_v, soft_h, soft_v) packs six
         bytes across CR2..CR4, two per register. */
      Cmd(MPEG_CMD_SET_VIDEOEFF, 0,
          (MPEG_ITP_CH << 8) | MPEG_TRP_128,
          (0x03 << 8) | 0x07,
          (MPEG_SOFT_ON << 8) | MPEG_SOFT_ON);

      expect_eq("interpolation", d->itp,    MPEG_ITP_CH);
      expect_eq("luminance key", d->trp,    MPEG_TRP_128);
      expect_eq("mosaic h",      d->moz_h,  0x03);
      expect_eq("mosaic v",      d->moz_v,  0x07);
      expect_eq("soften h",      d->soft_h, MPEG_SOFT_ON);
      expect_eq("soften v",      d->soft_v, MPEG_SOFT_ON);

      /* The luma plane must track the decoded picture, since the
         luminance key thresholds on it and cannot recover it from the
         converted RGB. */
      expect_true("no luma before decode", MPEG_GetFrameLuma() == NULL);

      if(getenv("MPEG_TEST_STREAM"))
      {
         FILE *fp = fopen(getenv("MPEG_TEST_STREAM"), "rb");

         if(fp)
         {
            static uint8_t sec[2324];
            size_t got;
            unsigned i;
            uint32_t fw = 0, fh = 0;

            MPEG_Reset(true);
            Cmd(MPEG_CMD_SET_STREAM, 0x0000, 0xFF00, 0x0000, 0xFF00);

            for(i = 0; i < 64 && (got = fread(sec, 1, sizeof(sec), fp)) > 0; i++)
            {
               MPEG_FeedSector(sec, (uint32_t)got, 0x00);
               MPEG_RunFrame();
            }
            fclose(fp);

            expect_true("luma available after decode", MPEG_GetFrameLuma() != NULL);
            expect_true("frame available", MPEG_GetFrame(&fw, &fh) != NULL);

            /* Luma must be real 8-bit video, not a constant. */
            {
               const uint8_t *l = MPEG_GetFrameLuma();
               unsigned lo = 255, hi = 0;
               unsigned x, y;

               for(y = 0; y < fh; y++)
                  for(x = 0; x < fw; x++)
                  {
                     const unsigned v = l[y * MPEG_MAX_WIDTH + x];

                     if(v < lo) lo = v;
                     if(v > hi) hi = v;
                  }

               expect_true("luma plane has range", hi > lo);
            }

            /* Chroma interpolation must change the converted output.
               Compare the same picture converted twice rather than two
               different pictures: decode a fixed prefix of the stream
               with the switch off, snapshot a row, then start over with
               the switch on and compare. */
            {
               static uint16_t before[MPEG_MAX_WIDTH];
               static uint8_t prefix[2324 * 64];
               size_t plen;
               const uint16_t *f;
               unsigned x, diff = 0;

               fp = fopen(getenv("MPEG_TEST_STREAM"), "rb");
               plen = fp ? fread(prefix, 1, sizeof(prefix), fp) : 0;
               if(fp)
                  fclose(fp);

               f = MPEG_GetFrame(NULL, NULL);
               memcpy(before, f + (size_t)(fh / 2) * MPEG_MAX_WIDTH,
                      fw * sizeof(uint16_t));

               MPEG_Reset(true);
               Cmd(MPEG_CMD_SET_STREAM, 0x0000, 0xFF00, 0x0000, 0xFF00);
               Cmd(MPEG_CMD_SET_VIDEOEFF, 0, MPEG_ITP_CH << 8, 0, 0);

               for(i = 0; i + 2324 <= plen; i += 2324)
               {
                  MPEG_FeedSector(prefix + i, 2324, 0x00);
                  MPEG_RunFrame();
               }

               f = MPEG_GetFrame(NULL, NULL);

               if(f)
               {
                  for(x = 0; x < fw; x++)
                     if(f[(size_t)(fh / 2) * MPEG_MAX_WIDTH + x] != before[x])
                        diff++;
               }

               expect_true("chroma interpolation changes output", diff > 0);
            }
         }
      }
   }

   /* --- 15g. Mode fields, decode strobe, connection switch --------- */
   printf("[mode / decsync / chgcon]\n");
   {
      const int64_t frame_ntsc = ((int64_t)((11289600ULL * 1001) / 30000)) << 32;
      const int64_t frame_pal  = ((int64_t)(11289600ULL / 25)) << 32;

      MPEG_Reset(true);

      /* Output mode gates the compositor. */
      expect_true("VDP2 output by default", MPEG_DirectOutput());

      Cmd(MPEG_CMD_SET_MODE, 0x00FF, (0x00FF << 8) | MPEG_OUT_HOST, 0xFF00, 0);
      expect_true("host output stands the compositor down", !MPEG_DirectOutput());

      Cmd(MPEG_CMD_SET_MODE, 0x00FF, (0x00FF << 8) | MPEG_OUT_VDP2, 0xFF00, 0);
      expect_true("VDP2 output restored", MPEG_DirectOutput());

      /* Scan mode sets the pre-sequence-header decode period.  Check it
         through VCounter, which ticks once per period. */
      {
         uint16_t vc0;

         MPEG_Reset(true);
         Cmd(MPEG_CMD_SET_MODE, 0x00FF, 0xFFFF, MPEG_SCN_PAL_NI << 8, 0);

         Cmd(MPEG_CMD_GET_STATUS, 0, 0, 0, 0);
         vc0 = Out[1];

         MPEG_Update(frame_pal - 1);
         Cmd(MPEG_CMD_GET_STATUS, 0, 0, 0, 0);
         expect_eq("PAL period not yet elapsed", Out[1], vc0);

         MPEG_Update(2);
         Cmd(MPEG_CMD_GET_STATUS, 0, 0, 0, 0);
         expect_eq("PAL period elapsed", Out[1], (uint16_t)(vc0 + 1));

         /* An NTSC period must not tick twice at PAL rate. */
         MPEG_Reset(true);
         Cmd(MPEG_CMD_SET_MODE, 0x00FF, 0xFFFF, MPEG_SCN_NTSC_NI << 8, 0);
         Cmd(MPEG_CMD_GET_STATUS, 0, 0, 0, 0);
         vc0 = Out[1];
         MPEG_Update(frame_ntsc);
         Cmd(MPEG_CMD_GET_STATUS, 0, 0, 0, 0);
         expect_eq("NTSC period elapsed once", Out[1], (uint16_t)(vc0 + 1));
      }

      /* Host-synced decode: the clock must not decode on its own. */
      if(getenv("MPEG_TEST_STREAM"))
      {
         FILE *fp = fopen(getenv("MPEG_TEST_STREAM"), "rb");

         if(fp)
         {
            static uint8_t sec[2324];
            size_t got;
            unsigned i;

            MPEG_Reset(true);
            Cmd(MPEG_CMD_SET_STREAM, 0x0000, 0xFF00, 0x0000, 0xFF00);
            Cmd(MPEG_CMD_SET_MODE, 0x00FF, (MPEG_DEC_HOST << 8) | 0x00FF, 0xFF00, 0);

            for(i = 0; i < 64 && (got = fread(sec, 1, sizeof(sec), fp)) > 0; i++)
               MPEG_FeedSector(sec, (uint32_t)got, 0x00);
            fclose(fp);

            for(i = 0; i < 30; i++)
               MPEG_Update(frame_ntsc);

            expect_true("host-sync: clock does not decode",
                        MPEG_GetFrame(NULL, NULL) == NULL);

            /* The strobe does. */
            for(i = 0; i < 8; i++)
               Cmd(MPEG_CMD_OUT_DECSYNC, 0, 0x0002, 0, 0);

            expect_true("OUT_DECSYNC decodes", MPEG_GetFrame(NULL, NULL) != NULL);
            expect_eq  ("frame buffer selected",
                        MPEG_GetDisplayState()->fbn, 0x02);

            /* In VSYNC mode the strobe must be inert -- the clock is
               already driving the decoder. */
            MPEG_Reset(true);
            Cmd(MPEG_CMD_SET_MODE, 0x00FF, (MPEG_DEC_VSYNC << 8) | 0x00FF, 0xFF00, 0);
            Cmd(MPEG_CMD_OUT_DECSYNC, 0, 0x0003, 0, 0);
            expect_eq("VSYNC mode: strobe inert",
                      MPEG_GetDisplayState()->fbn, 0);
         }
      }

      /* CHANGE_CONN promotes the next bank to current. */
      MPEG_Reset(true);
      Cmd(MPEG_CMD_SET_CONNECTION, 0x0000, 0x0005, 0x0000, 0x0007);
      Cmd(MPEG_CMD_SET_CONNECTION, 0x0000, 0x0009, 0x0100, 0x000B);

      expect_true("next bank inactive before switch", !MPEG_WantsPartition(0x0B));

      Cmd(MPEG_CMD_CHANGE_CONN, 0,
          (MPEG_COF_CHG << 8) | MPEG_COF_CHG,
          (MPEG_CLA_OFF << 8) | MPEG_CLV_FRM, 0);

      expect_true("next video partition now active",  MPEG_WantsPartition(0x0B));
      expect_true("next audio partition now active",  MPEG_WantsPartition(0x09));
      expect_true("old video partition inactive",    !MPEG_WantsPartition(0x07));

      /* Abort disconnects instead of switching. */
      Cmd(MPEG_CMD_SET_CONNECTION, 0x0000, 0x000D, 0x0100, 0x000E);
      Cmd(MPEG_CMD_CHANGE_CONN, 0,
          (MPEG_COF_ABT << 8) | MPEG_COF_ABT,
          (MPEG_CLA_ON << 8) | MPEG_CLV_FRM, 0);

      expect_true("abort disconnects video", !MPEG_WantsPartition(0x0E));
      expect_true("abort disconnects audio", !MPEG_WantsPartition(0x0D));
      expect_eq  ("buffers cleared", MPEG_GetESFill(true), 0);
   }

   /* --- 16. Interrupt factors ------------------------------------- */
   printf("[interrupts]\n");
   {
      static uint8_t ps[8192];
      size_t len;
      unsigned i;
      uint32_t pend;

      MPEG_Reset(true);
      Cmd(MPEG_CMD_SET_STREAM, 0x0000, 0xFF00, 0x0000, 0xFF00);

      /* Masked off, nothing may be reported and no HIRQ may be owed --
         this is the whole point of the mask. */
      Cmd(MPEG_CMD_SET_INT_MASK, 0x0000, 0x0000, 0, 0);
      (void)MPEG_TakePendingHIRQ();

      len = build_ps(ps, sizeof(ps));

      for(i = 0; i < len; i += 2324)
      {
         size_t chunk = len - i;
         if(chunk > 2324)
            chunk = 2324;
         MPEG_FeedSector(ps + i, (uint32_t)chunk, 0x00);
      }

      Cmd(MPEG_CMD_GET_INTERRUPT, 0, 0, 0, 0);
      expect_eq  ("masked: nothing reported (hi)", Out[0] & 0xFF, 0);
      expect_eq  ("masked: nothing reported (lo)", Out[1], 0);
      expect_true("masked: no MPST owed", MPEG_TakePendingHIRQ() == 0);

      /* Unmask everything and re-feed.  Both stream-ready factors must
         appear, and MPST must be owed to the CD block -- without that
         the host never knows to come and read this register. */
      MPEG_Reset(true);
      Cmd(MPEG_CMD_SET_STREAM, 0x0000, 0xFF00, 0x0000, 0xFF00);
      Cmd(MPEG_CMD_SET_INT_MASK, 0x00FF, 0xFFFF, 0, 0);
      (void)MPEG_TakePendingHIRQ();

      for(i = 0; i < len; i += 2324)
      {
         size_t chunk = len - i;
         if(chunk > 2324)
            chunk = 2324;
         MPEG_FeedSector(ps + i, (uint32_t)chunk, 0x00);
      }

      expect_true("MPST owed after factors set",
                  (MPEG_TakePendingHIRQ() & MPEG_HIRQ_MPST) != 0);
      expect_true("MPST drained by reading", MPEG_TakePendingHIRQ() == 0);

      Cmd(MPEG_CMD_GET_INTERRUPT, 0, 0, 0, 0);
      pend = ((uint32_t)(Out[0] & 0xFF) << 16) | Out[1];

      expect_true("video stream ready raised", (pend & MPEG_INT_VSRDY) != 0);
      expect_true("audio stream ready raised", (pend & MPEG_INT_ASRDY) != 0);
      expect_true("sequence end raised",       (pend & MPEG_INT_SQEND) != 0);

      /* Read-to-clear. */
      Cmd(MPEG_CMD_GET_INTERRUPT, 0, 0, 0, 0);
      expect_eq("factors cleared by read (hi)", Out[0] & 0xFF, 0);
      expect_eq("factors cleared by read (lo)", Out[1], 0);

      /* Sector trigger and EOR submode bits become factors, and only
         for the streams the sector actually carried. */
      MPEG_Reset(true);
      Cmd(MPEG_CMD_SET_STREAM, 0x0000, 0xFF00, 0x0000, 0xFF00);
      Cmd(MPEG_CMD_SET_INT_MASK, 0x00FF, 0xFFFF, 0, 0);

      for(i = 0; i < len; i += 2324)
      {
         size_t chunk = len - i;
         if(chunk > 2324)
            chunk = 2324;
         MPEG_FeedSector(ps + i, (uint32_t)chunk, 0x11);  /* trigger | EOR */
      }

      Cmd(MPEG_CMD_GET_INTERRUPT, 0, 0, 0, 0);
      pend = ((uint32_t)(Out[0] & 0xFF) << 16) | Out[1];

      expect_true("video trigger raised", (pend & MPEG_INT_VTRG) != 0);
      expect_true("video EOR raised",     (pend & MPEG_INT_VEOR) != 0);
      expect_true("audio trigger raised", (pend & MPEG_INT_ATRG) != 0);
      expect_true("audio EOR raised",     (pend & MPEG_INT_AEOR) != 0);

      /* A sector with no flags must not manufacture them. */
      MPEG_Reset(true);
      Cmd(MPEG_CMD_SET_STREAM, 0x0000, 0xFF00, 0x0000, 0xFF00);
      Cmd(MPEG_CMD_SET_INT_MASK, 0x00FF, 0xFFFF, 0, 0);
      MPEG_FeedSector(ps, (uint32_t)(len > 2324 ? 2324 : len), 0x00);

      Cmd(MPEG_CMD_GET_INTERRUPT, 0, 0, 0, 0);
      pend = ((uint32_t)(Out[0] & 0xFF) << 16) | Out[1];

      expect_eq("no trigger without the submode bit", pend & MPEG_INT_VTRG, 0);
      expect_eq("no EOR without the submode bit",     pend & MPEG_INT_VEOR, 0);

      /* INIT clears the register and anything owed. */
      Cmd(MPEG_CMD_INIT, 0, 0, 0, 0);
      (void)MPEG_TakePendingHIRQ();
      Cmd(MPEG_CMD_GET_INTERRUPT, 0, 0, 0, 0);
      expect_eq("INIT clears factors", ((uint32_t)(Out[0] & 0xFF) << 16) | Out[1], 0);

      /* Status fields must carry the SBL encodings, not placeholders. */
      Cmd(MPEG_CMD_GET_STATUS, 0, 0, 0, 0);
      expect_eq("stopped action status after INIT",
                Out[0] & 0xFF, MPEG_ASTV_STOP | MPEG_ASTD_STOP | MPEG_ASTA_STOP);
      expect_eq("video buffer empty after INIT", Out[3], MPEG_STV_BEMPTY);
      expect_eq("audio buffer empty after INIT", Out[2] & 0xFF, MPEG_STA_BEMPTY);

      Cmd(MPEG_CMD_PLAY, 0, 0, 0, 0);
      Cmd(MPEG_CMD_GET_STATUS, 0, 0, 0, 0);
      expect_eq("playing action status after PLAY",
                Out[0] & 0xFF, MPEG_ASTV_TRNS | MPEG_ASTA_TRNS);
   }

   /* --- 17. Double init must not leak ----------------------------- */
   printf("[double init]\n");
   expect_true("re-init", MPEG_Init(NULL));
   expect_true("still present", MPEG_IsPresent());

   MPEG_Kill();
   expect_true("absent after kill", !MPEG_IsPresent());
   MPEG_Kill();  /* idempotent */

   printf("\n%d checks, %d failures\n", checks, failures);
   return failures ? 1 : 0;
}
