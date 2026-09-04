#include <time.h>
#include <stddef.h>
#include <dlfcn.h>
time_t time(time_t* t) { time_t v = 1000000000; if(t) *t = v; return v; }
int clock_gettime(clockid_t id, struct timespec* ts)
{
 static int (*real)(clockid_t, struct timespec*);
 if(!real) real = dlsym(RTLD_NEXT, "clock_gettime");
 if(id == CLOCK_REALTIME || id == CLOCK_REALTIME_COARSE) { ts->tv_sec = 1000000000; ts->tv_nsec = 0; return 0; }
 return real(id, ts);
}
