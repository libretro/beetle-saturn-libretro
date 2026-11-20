#include "calendar_rtc.h"

/* INT64_MAX without a C99 long long constant */
#define CALENDAR_RTC_INT64_MAX ((int64_t)(((uint64_t)1 << 63) - 1))
#define CALENDAR_RTC_INT64_MIN (-CALENDAR_RTC_INT64_MAX - 1)
/* 3000-12-31 23:59:59 UTC. Past it the MS CRT's gmtime can abort the
 * process, and before 1970 it fails where glibc converts. */
#define CALENDAR_RTC_LOCAL_MAX ((int64_t)32535215 * 1000 + 999)

bool calendar_rtc_start_tm(const struct retro_calendar_time_interface *iface,
      struct tm *out)
{
   struct retro_calendar_time start;
   int64_t local;
   time_t t;
   struct tm *tm;

   if (!iface || iface->interface_version < 1 || !iface->get_start_time
         || !iface->get_start_time(&start))
      return false;

   if (start.utc_offset > 0
         ? start.sec > CALENDAR_RTC_INT64_MAX - start.utc_offset
         : start.sec < CALENDAR_RTC_INT64_MIN - start.utc_offset)
      return false;
   local = start.sec + start.utc_offset;
   if (local < 0 || local > CALENDAR_RTC_LOCAL_MAX)
      return false;

   /* A 32-bit time_t falls back rather than wrapping. */
   t = (time_t)local;
   if ((int64_t)t != local)
      return false;

   if (!(tm = gmtime(&t)))
      return false;
   *out = *tm;
   return true;
}
