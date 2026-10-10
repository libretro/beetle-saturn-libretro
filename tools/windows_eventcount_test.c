/* Windows notifications must not survive as an alert for a later frontend
 * wait on the same thread. Compile with SATURN_EVENTCOUNT_SPIN_ITERS=0 to
 * exercise the kernel parking window rather than just the bounded poll.
 * SATURN_TEST_FRONTEND additionally links an unchanged RetroArch eventcount
 * implementation and checks that its stack waiter is removed every round. */
#include <rthreads/retro_eventcount.h>
#include <rthreads/rthreads.h>
#include <windows.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Keep the core and frontend API names separate in this translation unit. */
extern bool saturn_eventcount_init(retro_eventcount_t *ec);
extern void saturn_eventcount_free(retro_eventcount_t *ec);
extern int saturn_eventcount_prepare_wait(retro_eventcount_t *ec);
extern void saturn_eventcount_commit_wait(retro_eventcount_t *ec, int key);
extern void saturn_eventcount_notify(retro_eventcount_t *ec);
extern const char *saturn_eventcount_backend_name(void);

typedef LONG (WINAPI *alert_wait_t)(volatile void*, LARGE_INTEGER*);
static retro_eventcount_t core;
static retro_atomic_int_t sent, completed;
#define ROUNDS 1000000

static void worker(void *unused)
{
   int i;
   (void)unused;
   for (i = 1; i <= ROUNDS; i++)
   {
      while (retro_atomic_load_acquire_int(&sent) != i)
         SwitchToThread();
      saturn_eventcount_notify(&core);
      retro_atomic_store_release_int(&completed, i);
   }
}
int main(void)
{
   int i;
   LARGE_INTEGER poll;
   HMODULE nt = GetModuleHandleA("ntdll.dll");
   alert_wait_t alert_wait = nt ? (alert_wait_t)GetProcAddress(nt,
         "NtWaitForAlertByThreadId") : NULL;
   sthread_t *thread;
#ifdef SATURN_TEST_FRONTEND
   retro_eventcount_t frontend;
   /* Force native NT waits, including zero-timeout polls. */
   _putenv("RETRO_EVENTCOUNT_SPIN=0");
   _putenv("RETRO_EVENTCOUNT_HIRES=0");
   _putenv("RETRO_EVENTCOUNT_SLEEP=alert");
   assert(retro_eventcount_init(&frontend));
   printf("Frontend backend: %s\n", retro_eventcount_backend_name());
#endif
   assert(saturn_eventcount_init(&core));
   assert(!strcmp(saturn_eventcount_backend_name(), "WaitOnAddress"));
   retro_atomic_int_init(&sent, 0);
   retro_atomic_int_init(&completed, 0);
   poll.QuadPart = 0;
   thread = sthread_create(worker, NULL);
   assert(thread);
   for (i = 1; i <= ROUNDS; i++)
   {
      int key = saturn_eventcount_prepare_wait(&core);
      retro_atomic_store_release_int(&sent, i);
      saturn_eventcount_commit_wait(&core, key);
      /* The notifier can finish after commit returned. Wait for it before
       * testing whether any notification escaped the completed core wait. */
      while (retro_atomic_load_acquire_int(&completed) != i)
         SwitchToThread();
#ifdef SATURN_TEST_FRONTEND
      key = retro_eventcount_prepare_wait(&frontend);
      retro_eventcount_commit_wait_timeout(&frontend, key, 0);
      assert(retro_atomic_load_acquire_ptr(&frontend.waitlist) == NULL);
#endif
      if (alert_wait)
         assert(alert_wait(NULL, &poll) == 0x102); /* STATUS_TIMEOUT */
   }
   sthread_join(thread);
   saturn_eventcount_free(&core);
#ifdef SATURN_TEST_FRONTEND
   retro_eventcount_free(&frontend);
#endif
   puts("1,000,000 core waits: no escaped NT alerts or frontend stack waiters.");
   return 0;
}
