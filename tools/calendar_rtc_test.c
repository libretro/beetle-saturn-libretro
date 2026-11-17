/* calendar_rtc_start_tm (calendar_rtc.c): the frontend's start time comes
 * back as the local fields of sec + utc_offset read as UTC, never through
 * localtime(); a missing, old or failing interface, or a local time before
 * 1970 or after 3000, gives false. Prints one line per failed check. C89.
 *
 *    calendar_rtc_test
 */
#include <stdio.h>

#include "calendar_rtc.h"

/* 2001-02-03 04:05:06 UTC, a Saturday */
#define T1 ((int64_t)981173106L)
/* 2026-12-31 23:30:00 UTC, a Thursday */
#define T2 ((int64_t)1798759800L)
/* 3000-12-31 23:59:59 UTC, a Wednesday */
#define Y3000 ((int64_t)32535215 * 1000 + 999)

static struct retro_calendar_time fake_start;
static bool     fake_ok;
static unsigned fake_calls;
static unsigned checks;
static unsigned fails;

static bool RETRO_CALLCONV fake_get_start_time(struct retro_calendar_time *t)
{
   fake_calls++;
   if (!fake_ok)
      return false;
   *t = fake_start;
   return true;
}

static void check(const char *test, const char *what, long got, long want)
{
   checks++;
   if (got != want)
   {
      fails++;
      printf("%s: %s is %ld, want %ld\n", test, what, got, want);
   }
}

static struct retro_calendar_time_interface iface_v1(void)
{
   struct retro_calendar_time_interface iface;
   iface.interface_version = 1;
   iface.get_start_time    = fake_get_start_time;
   iface.get_current_time  = NULL;
   iface.get_zone          = NULL;
   return iface;
}

static bool run(const struct retro_calendar_time_interface *iface,
      bool start_ok, int64_t sec, int32_t utc_offset, struct tm *out)
{
   fake_start.sec        = sec;
   fake_start.nsec       = 0;
   fake_start.utc_offset = utc_offset;
   fake_ok               = start_ok;
   fake_calls            = 0;
   return calendar_rtc_start_tm(iface, out);
}

static void expect_tm(const char *test, int64_t sec, int32_t utc_offset,
      int year, int mon, int mday, int hour, int minute, int second,
      int wday)
{
   struct retro_calendar_time_interface iface = iface_v1();
   struct tm tm;
   bool ok = run(&iface, true, sec, utc_offset, &tm);

   check(test, "result", ok, 1);
   check(test, "get_start_time calls", fake_calls, 1);
   if (!ok)
      return;
   check(test, "tm_year", tm.tm_year, year);
   check(test, "tm_mon", tm.tm_mon, mon);
   check(test, "tm_mday", tm.tm_mday, mday);
   check(test, "tm_hour", tm.tm_hour, hour);
   check(test, "tm_min", tm.tm_min, minute);
   check(test, "tm_sec", tm.tm_sec, second);
   check(test, "tm_wday", tm.tm_wday, wday);
}

static void expect_false(const char *test,
      const struct retro_calendar_time_interface *iface, bool start_ok,
      int64_t sec, int32_t utc_offset, unsigned calls)
{
   struct tm tm;
   bool ok = run(iface, start_ok, sec, utc_offset, &tm);

   check(test, "result", ok, 0);
   check(test, "get_start_time calls", fake_calls, calls);
}

int main(void)
{
   struct retro_calendar_time_interface iface = iface_v1();

   expect_tm("pos",  T1,  19800, 101, 1, 3,  9, 35, 6, 6);
   expect_tm("neg",  T1, -36000, 101, 1, 2, 18,  5, 6, 5);
   expect_tm("year", T2,   3600, 127, 0, 1,  0, 30, 0, 5);
   expect_tm("epoch", 3600, -3600, 70, 0, 1,  0,  0, 0, 4);

   /* A 32-bit time_t ends in 2038. */
   if (sizeof(time_t) < 8)
   {
      expect_false("y3000", &iface, true, Y3000, 0, 1);
      expect_false("y3000off", &iface, true, Y3000 - 3600, 3600, 1);
   }
   else
   {
      expect_tm("y3000", Y3000, 0, 1100, 11, 31, 23, 59, 59, 3);
      expect_tm("y3000off", Y3000 - 3600, 3600,
            1100, 11, 31, 23, 59, 59, 3);
   }

   expect_false("huge", &iface, true, ((int64_t)1 << 62), 0, 1);
   expect_false("overflow", &iface, true,
         (int64_t)(((uint64_t)1 << 63) - 1), 3600, 1);
   expect_false("overflow_neg", &iface, true,
         -(int64_t)(((uint64_t)1 << 63) - 1) - 1, -3600, 1);
   expect_false("startfail", &iface, false, T1, 0, 1);
   expect_false("pre1970", &iface, true, -1, 0, 1);
   expect_false("pre1970off", &iface, true, 0, -1, 1);
   expect_false("y3001", &iface, true, Y3000 + 1, 0, 1);
   expect_false("y3001off", &iface, true, Y3000 + 1 - 3600, 3600, 1);

   iface.interface_version = 0;
   expect_false("v0", &iface, true, T1, 0, 0);

   iface = iface_v1();
   iface.get_start_time = NULL;
   expect_false("nullfn", &iface, true, T1, 0, 0);

   expect_false("nulliface", NULL, true, T1, 0, 0);

   if (fails)
   {
      printf("calendar_rtc_test: %u of %u checks failed\n", fails, checks);
      return 1;
   }
   printf("calendar_rtc_test: %u checks passed\n", checks);
   return 0;
}
