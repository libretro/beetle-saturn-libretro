/* LD_PRELOAD shim: pin the wall clock so the core's auto-RTC is reproducible. */
#include <time.h>
#include <stddef.h>
time_t time(time_t* t) { time_t v = 1000000000; if(t) *t = v; return v; }
int clock_gettime(clockid_t id, struct timespec* ts)
{
 (void)id; if(ts) { ts->tv_sec = 1000000000; ts->tv_nsec = 0; } return 0;
}
