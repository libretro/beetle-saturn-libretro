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

   /* --- 11. Sector feed does not fault ---------------------------- */
   printf("[sector feed]\n");
   {
      static uint8_t sec[2324];
      unsigned i;

      memset(sec, 0xA5, sizeof(sec));

      /* Overrun the video FIFO several times over: writes must clamp,
         never wrap past the end or corrupt the heap. */
      for(i = 0; i < 200; i++)
         MPEG_FeedSector(sec, sizeof(sec), true);

      for(i = 0; i < 200; i++)
         MPEG_FeedSector(sec, sizeof(sec), false);

      MPEG_FeedSector(NULL, 0, true);
      MPEG_FeedSector(sec, 0, true);

      expect_true("no frame yet", MPEG_RunFrame() == false);
      expect_true("no frame buffer yet", MPEG_GetFrame(NULL, NULL) == NULL);
   }

   /* --- 12. Double init must not leak ----------------------------- */
   printf("[double init]\n");
   expect_true("re-init", MPEG_Init(NULL));
   expect_true("still present", MPEG_IsPresent());

   MPEG_Kill();
   expect_true("absent after kill", !MPEG_IsPresent());
   MPEG_Kill();  /* idempotent */

   printf("\n%d checks, %d failures\n", checks, failures);
   return failures ? 1 : 0;
}
