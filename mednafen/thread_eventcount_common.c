/* libretro-common's own threaded code (the VFS read-ahead thread on
 * Windows) calls the eventcount by its common names, which this core
 * otherwise maps to saturn_*. Windows only, where the thread exists: a
 * DLL binds its own definitions, so these never meet the frontend's. */
#include "thread_eventcount.h"

#if defined(_WIN32) && !defined(_XBOX) && defined(HAVE_THREADS)
#undef retro_eventcount_init
#undef retro_eventcount_free
#undef retro_eventcount_notify
#undef retro_eventcount_prepare_wait
#undef retro_eventcount_cancel_wait
#undef retro_eventcount_commit_wait

bool retro_eventcount_init(retro_eventcount_t *ec)
{ return saturn_eventcount_init(ec); }
void retro_eventcount_free(retro_eventcount_t *ec)
{ saturn_eventcount_free(ec); }
void retro_eventcount_notify(retro_eventcount_t *ec)
{ saturn_eventcount_notify(ec); }
int retro_eventcount_prepare_wait(retro_eventcount_t *ec)
{ return saturn_eventcount_prepare_wait(ec); }
void retro_eventcount_cancel_wait(retro_eventcount_t *ec)
{ saturn_eventcount_cancel_wait(ec); }
void retro_eventcount_commit_wait(retro_eventcount_t *ec, int key)
{ saturn_eventcount_commit_wait(ec, key); }
#else
typedef int saturn_eventcount_common_unused;
#endif
