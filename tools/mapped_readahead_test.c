/* Read-ahead on a memory-mapped disc image.
 *
 *    mapped_readahead_test
 *
 * Windows starts no read-ahead on a mapped view, so the disc readers
 * hint the next few MiB of the file ahead of a reader that goes on in
 * order (mednafen/mapped_readahead.h); nowhere else do they hint at
 * all. And a mapping opened under Pre-Cache (image_memcache) asks for
 * the whole file to be read in, rather than taking the mapping as
 * already loaded. filestream_prefetch is wrapped (the .sh links with
 * --wrap) to count the hints.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../mednafen/cdstream.h"

#if defined(_WIN32) && defined(VFS_WIN32_PREFETCH_HOOK)
/* The off-thread build: PrefetchVirtualMemory is a stand-in that sleeps
 * as a cold disk would, and a read must never wait for it. */
#include <windows.h>
#define SLOW_MS       200
#define READ_LIMIT_MS 100
static volatile LONG hook_calls, hook_on_reader;
static DWORD         reader_thread;

int saturn_prefetch_hook(void *addr, size_t len)
{
   (void)addr;
   (void)len;
   InterlockedIncrement(&hook_calls);
   if (GetCurrentThreadId() == reader_thread)
      InterlockedIncrement(&hook_on_reader);
   Sleep(SLOW_MS);
   return 1;
}
#endif

#define FIXTURE_LEN (8u * 1024u * 1024u)
#define STEP        2048u

static unsigned hints;
static uint64_t hint_off, hint_len;
static int      fails;

void __real_filestream_prefetch(RFILE *stream, uint64_t offset, uint64_t len);

void __wrap_filestream_prefetch(RFILE *stream, uint64_t offset, uint64_t len)
{
   hints++;
   hint_off = offset;
   hint_len = len;
   __real_filestream_prefetch(stream, offset, len);
}

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

static void check_step(void)
{
   mapped_readahead ra;
   uint64_t         pos;
   unsigned         n = 0;
   bool             ok = true;

   memset(&ra, 0, sizeof(ra));
   ok = ok && mapped_readahead_step(&ra, 0, STEP);
   ok = ok && !mapped_readahead_step(&ra, STEP, STEP);
   for (pos = 2 * STEP; pos < MAPPED_RA_AHEAD; pos += STEP)
      n += mapped_readahead_step(&ra, pos, STEP);
   ok = ok && n == 1;
   ok = ok && !mapped_readahead_step(&ra, 100u * MAPPED_RA_AHEAD, STEP);
   ok = ok && mapped_readahead_step(&ra, 100u * MAPPED_RA_AHEAD + STEP, STEP);
   pos = 100u * MAPPED_RA_AHEAD + 2 * STEP + MAPPED_RA_GAP / 2;
   ok = ok && !mapped_readahead_step(&ra, pos, STEP);
   ok = ok && !mapped_readahead_step(&ra, pos + STEP, STEP);
   ok = ok && !mapped_readahead_step(&ra, 0, STEP);
   CHECK(ok, "hints follow a reader in order; jumps hint nothing");
}

int main(void)
{
   const char *path = "mapped_readahead_fixture.bin";
   uint8_t    *data = (uint8_t*)malloc(FIXTURE_LEN);
   uint8_t     buf[STEP];
   cdstream    s;
   FILE       *f;
   uint32_t    i;
   bool        same = true;

   printf("mapped read-ahead\n");
   check_step();

   for (i = 0; i < FIXTURE_LEN; i++)
      data[i] = (uint8_t)((i * 2654435761u) >> 13);
   f = fopen(path, "wb");
   if (!f || fwrite(data, 1, FIXTURE_LEN, f) != FIXTURE_LEN)
   {
      printf("  FAIL: cannot write %s\n", path);
      return 1;
   }
   fclose(f);

   CHECK(cdstream_open(&s, path), "open");
   if (!s.buf)
      printf("  skip: no mapping on this build\n");
   else
   {
#if defined(_WIN32) && defined(VFS_WIN32_PREFETCH_HOOK)
      LARGE_INTEGER freq, t0, t1;
      double        worst_ms = 0.0;
      int           waited;
      QueryPerformanceFrequency(&freq);
      reader_thread = GetCurrentThreadId();
#endif
      hints    = 0;
      hint_off = hint_len = 0;
      for (i = 0; i + STEP <= 6u * 1024u * 1024u; i += STEP)
      {
#if defined(_WIN32) && defined(VFS_WIN32_PREFETCH_HOOK)
         QueryPerformanceCounter(&t0);
#endif
         if (cdstream_read(&s, buf, STEP) != STEP
               || memcmp(buf, data + i, STEP))
            same = false;
#if defined(_WIN32) && defined(VFS_WIN32_PREFETCH_HOOK)
         QueryPerformanceCounter(&t1);
         {
            const double ms = (double)(t1.QuadPart - t0.QuadPart) * 1000.0 / (double)freq.QuadPart;
            if (ms > worst_ms)
               worst_ms = ms;
         }
#endif
      }
#if defined(_WIN32) && defined(VFS_WIN32_PREFETCH_HOOK)
      /* the stand-in is the file's thread's to call, in its own time */
      for (waited = 0; waited < 50 && hook_calls == 0; waited++)
         Sleep(20);
      CHECK(hook_calls > 0, "the read-ahead is taken up (off-thread build)");
      CHECK(hook_on_reader == 0, "never on the reading thread");
      CHECK(worst_ms < READ_LIMIT_MS, "no read waits for it");
#endif
      CHECK(same, "reads through the mapping return the file");
#ifdef _WIN32
      CHECK(hints == 3, "a reader in order is hinted ahead (Windows)");
      /* the last hint was made by a read ending where it starts */
      CHECK(hint_len == MAPPED_RA_AHEAD && hint_off % STEP == 0 && hint_off > 0,
            "a hint starts past the read that made it");
#else
      CHECK(hints == 0, "and nothing is hinted where the kernel reads ahead");
#endif
   }
   cdstream_close(&s);

   hints = 0;
   CHECK(cdstream_open_memcached(&s, path), "open under Pre-Cache");
   if (s.buf && s.fp)
      CHECK(hints == 1 && hint_off == 0 && hint_len == FIXTURE_LEN,
            "a mapping under Pre-Cache asks for the whole file");
   cdstream_close(&s);
   remove(path);
   free(data);

   printf(fails ? "mapped read-ahead: FAILED\n" : "mapped read-ahead: PASS\n");
   return fails ? 1 : 0;
}
