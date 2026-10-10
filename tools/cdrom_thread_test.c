/* Deterministic CD reader ownership, demand, hint and teardown stress.
 * Build/run through tools/check_lockfree.py. No game image is required. */
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libretro.h>
#include <retro_timers.h>
#include <features/features_cpu.h>
#include "../mednafen/cdrom/CDAccess.h"
#include "../mednafen/cdrom/CDUtility.h"
#include "../mednafen/cdrom/cdromif.h"

static void test_log(enum retro_log_level level, const char *fmt, ...)
{ (void)level; (void)fmt; }
retro_log_printf_t log_cb = test_log;
static bool slow_io;
static bool fake_read(CDAccess *self, uint8_t *buf, int32_t lba)
{
   unsigned i;
   (void)self;
   if (slow_io) retro_sleep_us(50);
   for (i = 0; i < 2448; i++) buf[i] = (uint8_t)((uint32_t)lba * 17 + i * 13);
   return lba != 333;
}
static bool fake_pw(CDAccess *self, uint8_t *buf, int32_t lba)
{ (void)self; (void)buf; (void)lba; return false; }
static bool fake_toc(CDAccess *self, TOC *toc)
{
   (void)self;
   memset(toc, 0, sizeof(*toc));
   toc->first_track = toc->last_track = 1;
   return true;
}
CDAccess *CDAccess_Open(const char *path, bool memcache)
{
   CDAccess *self = (CDAccess*)calloc(1, sizeof(*self));
   (void)path; (void)memcache;
   self->Read_Raw_Sector = fake_read;
   self->Fast_Read_Raw_PW_TSRE = fake_pw;
   self->Read_TOC = fake_toc;
   self->destroy = (void (*)(CDAccess*))free;
   return self;
}
bool edc_lec_check_and_correct(uint8_t *buf, bool xa)
{ (void)buf; (void)xa; return true; }
static void check(CDIF *cd, int32_t lba)
{
   uint8_t data[2448];
   unsigned i;
   bool ok = CDIF_ReadRawSector(cd, data, lba);
   assert(ok == (lba != 333));
   for (i = 0; i < sizeof(data); i++)
      assert(data[i] == (uint8_t)((uint32_t)lba * 17 + i * 13));
}
int main(int argc, char **argv)
{
   unsigned round, i, seed = 12345;
   int64_t start;
   (void)argv;
   slow_io = argc > 1;
   start = cpu_features_get_time_usec();
   for (round = 0; round < 3; round++)
   {
      CDIF *cd = CDIF_Open("fake", false);
      TOC toc;
      assert(cd);
      CDIF_ReadTOC(cd, &toc);
      assert(toc.first_track == 1 && toc.last_track == 1);
      for (i = 0; i < 10000; i++) check(cd, i);
      for (i = 0; i < 1000; i++)
      {
         seed = seed * 1664525u + 1013904223u;
         check(cd, (int32_t)(seed % 40000));
         check(cd, (int32_t)(seed % 40000)); /* immediate cache hit */
      }
      for (i = 0; i < 20000; i++) CDIF_HintReadSector(cd, i % 40000);
      check(cd, -150);
      check(cd, 449849); /* inclusive upper bound */
      check(cd, 333); /* backend error payload and status */
      {
         uint8_t data[2448];
         assert(!CDIF_ReadRawSector(cd, data, -151));
         assert(!CDIF_ReadRawSector(cd, data, 449850));
      }
      retro_sleep(2); /* close while the worker is parked */
      CDIF_Close(cd);
   }
   for (round = 0; round < 20; round++)
   {
      CDIF *cd = CDIF_Open("fake", false);
      assert(cd);
      CDIF_HintReadSector(cd, round * 99);
      CDIF_Close(cd); /* close during pending read-ahead */
   }
   {
      CDIF *cd = CDIF_Open("fake", true);
      assert(cd);
      check(cd, 333);
      check(cd, 99);
      CDIF_Close(cd);
   }
   printf("CD reader PASS: 36,009 reads, 60,000 hints, 24 open/close cycles; %.3f s\n",
         (cpu_features_get_time_usec() - start) / 1000000.0);
   return 0;
}
