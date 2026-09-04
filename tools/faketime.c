/* LD_PRELOAD shim: pin the wall clock so the core's auto-RTC is reproducible.
 * Only time() is overridden; clock_gettime stays real so timing runs work. */
#include <time.h>
#include <stddef.h>
time_t time(time_t* t) { time_t v = 1000000000; if(t) *t = v; return v; }
