/* The save writer (mednafen/save_writer.c): what is handed over lands in
 * its file byte for byte, as it was when it was handed over; the last of
 * two saves to one file is the one there; more saves than its queue holds
 * all land; a write that fails is reported; it stops and starts again;
 * 16-bit words go out big-endian. Built threaded and not. C89.
 *
 *    save_writer_test <workdir>
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libretro.h>
#include "../mednafen/save_writer.h"

static void quiet_log(enum retro_log_level level, const char *fmt, ...)
{
   (void)level;
   (void)fmt;
}
retro_log_printf_t log_cb = quiet_log;

static int fails;

#define CHECK(cond, msg) \
   do { \
      if (cond) \
         printf("  ok: %s\n", msg); \
      else \
      { \
         printf("  FAIL: %s\n", msg); \
         fails++; \
      } \
   } while (0)

static void join(char *out, size_t size, const char *dir, const char *name)
{
   size_t n = strlen(dir);
   if (n + 1 + strlen(name) + 1 > size)
   {
      out[0] = '\0';
      return;
   }
   memcpy(out, dir, n);
   out[n] = '/';
   strcpy(out + n + 1, name);
}

/* 1 when the file at @path holds exactly @len bytes of @want */
static int file_is(const char *path, const unsigned char *want, size_t len)
{
   FILE          *f = fopen(path, "rb");
   unsigned char *got;
   size_t         n;
   int            extra, same;

   if (!f)
      return 0;
   got   = (unsigned char *)malloc(len ? len : 1);
   n     = got ? fread(got, 1, len, f) : 0;
   extra = fgetc(f);
   fclose(f);
   same  = got && n == len && extra == EOF && !memcmp(got, want, len);
   free(got);
   return same;
}

static void fill(unsigned char *p, size_t len, unsigned seed)
{
   size_t i;
   for (i = 0; i < len; i++)
      p[i] = (unsigned char)((i * 2654435761u + seed * 40503u) >> 11);
}

/* one save of @len bytes of @src to @path */
static void save(const char *path, const unsigned char *src, size_t len, bool *failed)
{
   save_job_t *job = save_job_new(path, len);
   if (!job)
   {
      *failed = true;
      return;
   }
   memcpy(save_job_data(job), src, len);
   save_writer_submit(job, failed);
}

int main(int argc, char **argv)
{
   static const size_t sizes[] = { 1, 2047, 32768, 512 * 1024, 4 * 1024 * 1024 };
   const char    *dir = argc > 1 ? argv[1] : ".";
   char           path[1024], name[64];
   unsigned char *a, *b;
   bool           failed = false;
   size_t         i;
   int            ok;

   printf("save writer\n");
   a = (unsigned char *)malloc(4 * 1024 * 1024);
   b = (unsigned char *)malloc(4 * 1024 * 1024);
   if (!a || !b)
      return 1;

   /* every size, and the source changed as soon as it is handed over */
   ok = 1;
   for (i = 0; i < sizeof(sizes) / sizeof(sizes[0]); i++)
   {
      sprintf(name, "size%u.bin", (unsigned)i);
      join(path, sizeof(path), dir, name);
      fill(a, sizes[i], (unsigned)i);
      memcpy(b, a, sizes[i]);
      save(path, a, sizes[i], &failed);
      memset(a, 0xEE, sizes[i]);
   }
   save_writer_drain();
   for (i = 0; i < sizeof(sizes) / sizeof(sizes[0]); i++)
   {
      sprintf(name, "size%u.bin", (unsigned)i);
      join(path, sizeof(path), dir, name);
      fill(b, sizes[i], (unsigned)i);
      ok = ok && file_is(path, b, sizes[i]);
      remove(path);
   }
   CHECK(ok && !failed, "each save lands whole, as it was when handed over");

   /* one file saved over and over, more times than the queue holds:
    * the last is the one there */
   join(path, sizeof(path), dir, "again.bin");
   for (i = 0; i < 40; i++)
   {
      fill(a, 65536, 100 + (unsigned)i);
      save(path, a, 65536, &failed);
   }
   save_writer_drain();
   CHECK(file_is(path, a, 65536) && !failed, "the last of many saves to one file is the one there");
   remove(path);

   /* a file that cannot be made: reported, once drained */
   join(path, sizeof(path), dir, "no/such/dir/x.bin");
   save(path, a, 16, &failed);
   save_writer_drain();
   CHECK(failed, "a write that fails is reported");
   failed = false;

   /* stopped, and started again by the next save */
   save_writer_stop();
   join(path, sizeof(path), dir, "restart.bin");
   fill(a, 1000, 7);
   save(path, a, 1000, &failed);
   save_writer_stop();
   CHECK(file_is(path, a, 1000) && !failed, "stops, and starts again with the next save");
   remove(path);

   /* 16-bit words big-endian, a trailing odd byte as 0 */
   {
      unsigned short words[3];
      unsigned char  want[5];
      save_job_t    *job;

      words[0] = 0x1234;
      words[1] = 0xABCD;
      words[2] = 0x00FF;
      want[0] = 0x12; want[1] = 0x34; want[2] = 0xAB; want[3] = 0xCD; want[4] = 0;
      join(path, sizeof(path), dir, "be16.bin");
      job = save_job_new(path, 5);
      CHECK(job != NULL, "a job");
      if (job)
      {
         save_job_fill_be16(job, words, 5);
         save_writer_submit(job, &failed);
      }
      save_writer_stop();
      CHECK(file_is(path, want, 5) && !failed, "16-bit words go out big-endian");
      remove(path);
   }

   free(a);
   free(b);
   printf(fails ? "save writer: FAILED\n" : "save writer: PASS\n");
   return fails ? 1 : 0;
}
