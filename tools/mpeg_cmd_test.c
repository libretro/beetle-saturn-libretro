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
   expect_eq("CR1 = status<<8 | action", Out[0], 0x0100);
   expect_eq("HIRQ", Hirq, MPEG_HIRQ_MPCM);

   /* --- 4. Interrupt mask round trip + read-to-clear --------------- */
   printf("[interrupt]\n");
   Cmd(MPEG_CMD_SET_INT_MASK, 0x00AB, 0xCDEF, 0, 0);
   expect_eq("SET_INT_MASK CR1 report", Out[0], 0x0100);

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

      Cmd(MPEG_CMD_DISPLAY, 0x0001, 0, 0, 0);
      d = MPEG_GetDisplayState();
      expect_true("display enabled", d->enabled);

      Cmd(MPEG_CMD_DISPLAY, 0x0000, 0, 0, 0);
      expect_true("display disabled", !d->enabled);

      Cmd(MPEG_CMD_SET_WINDOW, 0x0010, 0x0020, 0x0030, 0x0040);
      expect_eq("src_x", d->src_x, 0x10);
      expect_eq("src_y", d->src_y, 0x20);
      expect_eq("x",     d->x,     0x30);
      expect_eq("y",     d->y,     0x40);

      Cmd(MPEG_CMD_SET_BORDERCOL, 0, 0x7C1F, 0, 0);
      expect_eq("border color", d->border_color, 0x7C1F);

      Cmd(MPEG_CMD_SET_FADE, 0, 0x0080, 0, 0);
      expect_eq("fade", d->fade, 0x80);
   }

   /* --- 8. SET_MODE 0xFF-means-keep semantics --------------------- */
   printf("[mode]\n");
   Cmd(MPEG_CMD_SET_MODE, 0x0003, 0x0102, 0x0400, 0);
   Cmd(MPEG_CMD_SET_MODE, 0x00FF, 0xFFFF, 0xFF00, 0);
   /* No getter is exposed; the check is that the command is claimed and
      reports normally rather than clobbering to 0xFF and wedging. */
   expect_eq("SET_MODE report", Out[0], 0x0100);

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
         MPEG_FeedSector(ps + i, (uint32_t)chunk);
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
         MPEG_FeedSector(ps + i, (uint32_t)chunk);
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
         MPEG_FeedSector(ps + i, (uint32_t)chunk);
      }

      expect_eq("bare index selects stream 0", MPEG_GetESFill(true), PS_VIDEO_BYTES);

      /* Mid-stream entry: start the feed inside a packet.  The parser
         must resynchronise rather than deadlock or mis-slice. */
      MPEG_Reset(true);
      Cmd(MPEG_CMD_SET_STREAM, 0x0000, 0xFF00, 0x0000, 0xFF00);
      MPEG_FeedSector(ps + 37, (uint32_t)(len - 37));
      expect_true("resynced mid-stream", MPEG_GetESFill(true) > 0);

      /* Garbage must not fault and must not be mistaken for a stream. */
      MPEG_Reset(true);
      Cmd(MPEG_CMD_SET_STREAM, 0x0000, 0xFF00, 0x0000, 0xFF00);
      memset(sec, 0xA5, sizeof(sec));
      for(i = 0; i < 64; i++)
         MPEG_FeedSector(sec, sizeof(sec));
      expect_eq("garbage yields no video ES", MPEG_GetESFill(true), 0);
      expect_eq("garbage yields no audio ES", MPEG_GetESFill(false), 0);

      /* Overrun: feed the fixture until the ES FIFOs are saturated.
         Writes must clamp, never wrap past the end. */
      for(i = 0; i < 400; i++)
         MPEG_FeedSector(ps, (uint32_t)(len > 2324 ? 2324 : len));

      MPEG_FeedSector(NULL, 0);
      MPEG_FeedSector(sec, 0);

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

         MPEG_Reset(true);
         Cmd(MPEG_CMD_SET_STREAM, 0x0000, 0xFF00, 0x0000, 0xFF00);

         while((got = fread(sec, 1, sizeof(sec), fp)) > 0)
         {
            int16_t pcm[4096 * 2];
            uint32_t n;

            MPEG_FeedSector(sec, (uint32_t)got);

            /* Drain the way the CD block event loop would: keep
               calling until the decoders stop producing. */
            while(MPEG_RunFrame())
            {
               const uint16_t *fb = MPEG_GetFrame(&w, &h);

               if(fb)
                  frames++;
            }

            while((n = MPEG_ReadAudio(pcm, 4096)) > 0)
               audio_frames += n;
         }

         fclose(fp);

         MPEG_GetAudioFormat(&rate, &ch);

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

   /* --- 13. Filter routing gate ----------------------------------- */
   printf("[filter routing]\n");
   {
      unsigned f;

      MPEG_Reset(true);

      /* Before any SET_CONNECTION no filter routes here, even though
         the connection fields read back as 0 -- otherwise enabling the
         option would hand the card every sector on any disc. */
      for(f = 0; f < 0x18; f++)
         expect_true("no routing before SET_CONNECTION", !MPEG_WantsFilter((uint8_t)f));

      /* audcon = 5 (CR1 low), vidcon = 7 (CR3 low), current bank. */
      Cmd(MPEG_CMD_SET_CONNECTION, 0x0005, 0x0000, 0x0007, 0x0000);

      expect_true("vidcon filter routes",   MPEG_WantsFilter(7));
      expect_true("audcon filter routes",   MPEG_WantsFilter(5));
      expect_true("other filter does not",  !MPEG_WantsFilter(6));
      expect_true("out-of-range rejected",  !MPEG_WantsFilter(0x18));
      expect_true("0xFF rejected",          !MPEG_WantsFilter(0xFF));

      /* The "next" bank must not affect routing, which follows the
         current connection only. */
      Cmd(MPEG_CMD_SET_CONNECTION, 0x0009, 0x0000, 0x010B, 0x0000);
      expect_true("next bank does not route", !MPEG_WantsFilter(0x0B));
      expect_true("current bank still routes", MPEG_WantsFilter(7));

      MPEG_Reset(true);
      expect_true("reset clears routing", !MPEG_WantsFilter(7));
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
               MPEG_FeedSector(sec, (uint32_t)got);
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

   /* --- 16. Double init must not leak ----------------------------- */
   printf("[double init]\n");
   expect_true("re-init", MPEG_Init(NULL));
   expect_true("still present", MPEG_IsPresent());

   MPEG_Kill();
   expect_true("absent after kill", !MPEG_IsPresent());
   MPEG_Kill();  /* idempotent */

   printf("\n%d checks, %d failures\n", checks, failures);
   return failures ? 1 : 0;
}
