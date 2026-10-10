/* Lost-wake, timeout, cancellation and epoch-wrap regression checks. */
#include "../mednafen/thread_eventcount.h"
#include <rthreads/rthreads.h>
#include <features/features_cpu.h>
#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
static int compare_latency(const void *a, const void *b)
{ int64_t x = *(const int64_t*)a, y = *(const int64_t*)b; return (x > y) - (x < y); }

static retro_eventcount_t request, reply;
static retro_atomic_int_t sent, received;
static unsigned rounds;
static void worker(void *unused)
{
   unsigned i;
   (void)unused;
   for (i = 1; i <= rounds; i++)
   {
      while ((unsigned)retro_atomic_load_acquire_int(&sent) != i)
      {
         int key = retro_eventcount_prepare_wait(&request);
         if ((unsigned)retro_atomic_load_acquire_int(&sent) != i)
            retro_eventcount_commit_wait(&request, key);
         else retro_eventcount_cancel_wait(&request);
      }
      retro_atomic_store_release_int(&received, i);
      retro_eventcount_notify(&reply);
   }
}
int main(int argc, char **argv)
{
   sthread_t *thread;
   retro_waitable_spsc_t queue;
   unsigned i;
   int key;
   int64_t start, bench_start, bench_elapsed;
   int64_t *latency;
   (void)argv;
   rounds = argc > 1 ? 100 : 100000;
   assert(retro_eventcount_init(&request));
   assert(retro_eventcount_init(&reply));
   retro_atomic_int_init(&sent, 0);
   retro_atomic_int_init(&received, 0);
   /* A notification before prepare is harmless; one after prepare must
    * make commit return immediately, even if nobody was parked yet. */
   retro_eventcount_notify(&request);
   key = retro_eventcount_prepare_wait(&request);
   retro_eventcount_notify(&request);
   retro_eventcount_commit_wait(&request, key);
   key = retro_eventcount_prepare_wait(&request);
   retro_eventcount_cancel_wait(&request);
   assert(!retro_atomic_load_acquire_int(&request.waiters));
   start = cpu_features_get_time_usec();
   for (;;)
   {
      int64_t left = 1000 - (cpu_features_get_time_usec() - start);
      if (left <= 0) break;
      key = retro_eventcount_prepare_wait(&request);
      if (!retro_eventcount_commit_wait_timeout(&request, key, left)) break;
      /* NT alerts, futex EINTR and other backends allow spurious wakes. */
   }
   assert(cpu_features_get_time_usec() - start >= 1000);
   assert(!retro_atomic_load_acquire_int(&request.waiters));
   retro_atomic_store_release_int(&request.epoch, INT_MAX - 1);
   retro_atomic_store_release_int(&reply.epoch, INT_MAX - 1);
   latency = (int64_t*)malloc(rounds * sizeof(*latency));
   assert(latency);
   thread = sthread_create(worker, NULL);
   assert(thread);
   bench_start = cpu_features_get_time_usec();
   for (i = 1; i <= rounds; i++)
   {
      int64_t trip_start = cpu_features_get_time_usec();
      retro_atomic_store_release_int(&sent, i);
      retro_eventcount_notify(&request);
      while ((unsigned)retro_atomic_load_acquire_int(&received) != i)
      {
         key = retro_eventcount_prepare_wait(&reply);
         if ((unsigned)retro_atomic_load_acquire_int(&received) != i)
            retro_eventcount_commit_wait(&reply, key);
         else retro_eventcount_cancel_wait(&reply);
      }
      latency[i - 1] = cpu_features_get_time_usec() - trip_start;
   }
   bench_elapsed = cpu_features_get_time_usec() - bench_start;
   sthread_join(thread);
   assert(!retro_atomic_load_acquire_int(&request.waiters));
   assert(!retro_atomic_load_acquire_int(&reply.waiters));
   assert(retro_waitable_spsc_init(&queue, sizeof(unsigned)));
   assert(!retro_waitable_spsc_wait_readable(&queue, sizeof(unsigned), 1000));
   assert(retro_waitable_spsc_write(&queue, &i, sizeof(i)) == sizeof(i));
   assert(!retro_waitable_spsc_wait_writable(&queue, sizeof(unsigned), 1000));
   assert(retro_waitable_spsc_read_quiet(&queue, &i, sizeof(i)) == sizeof(i));
   retro_waitable_spsc_cancel(&queue);
   assert(retro_waitable_spsc_cancelled(&queue));
   assert(!retro_waitable_spsc_wait_readable(&queue, sizeof(unsigned), 1000000));
   retro_waitable_spsc_free(&queue);
   retro_eventcount_free(&request);
   retro_eventcount_free(&reply);
   printf("Eventcount PASS: %u round trips, timeout, cancellation, lost-wake windows and epoch wrap (%s)\n",
         rounds, retro_eventcount_backend_name());
   qsort(latency, rounds, sizeof(*latency), compare_latency);
   printf("Handoff: %.0f round trips/s; median=%lld us, p99=%lld us; poll budget=%u\n",
         rounds * 1000000.0 / (bench_elapsed ? bench_elapsed : 1),
         (long long)latency[rounds / 2], (long long)latency[(rounds * 99) / 100],
         retro_eventcount_spin_iters());
   free(latency);
   return 0;
}
