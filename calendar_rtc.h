#ifndef __CALENDAR_RTC_HDR__
#define __CALENDAR_RTC_HDR__

#include <time.h>

#include <boolean.h>
#include <libretro.h>

/* Defined in libretro.c; zeroed when the frontend lacks the interface. */
extern struct retro_calendar_time_interface calendar_iface;

/* Fills *out with the local fields of the frontend's start time
 * (sec + utc_offset read as UTC). False when the interface is absent,
 * get_start_time fails, or the local time is before 1970, after 3000 or
 * cannot be converted. */
bool calendar_rtc_start_tm(const struct retro_calendar_time_interface *iface,
      struct tm *out);

#endif
