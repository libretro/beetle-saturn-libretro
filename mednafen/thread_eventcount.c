/* Core-specific address-wait implementation of the libretro-common
 * eventcount API. Upstream backends remain unchanged in rthreads/.
 * Each event has exactly one waiting thread. No waiter-list lock,
 * semaphore or allocation is needed here. */
#if defined(__linux__) && !defined(_GNU_SOURCE)
#define _GNU_SOURCE
#endif
#define RETRO_ATOMIC_REQUIRE_LOCK_FREE
#include "thread_eventcount.h"
#include <retro_timers.h>
#include <features/features_cpu.h>
#include <limits.h>
#include <errno.h>
#include <stdlib.h>
#if defined(_MSC_VER) && (defined(_M_IX86) || defined(_M_X64))
#include <emmintrin.h>
#endif
#ifndef SATURN_EVENTCOUNT_SPIN_ITERS
#define SATURN_EVENTCOUNT_SPIN_ITERS 256
#endif
static retro_atomic_int_t spin_budget;
#if defined(__linux__) && !defined(ANDROID_NO_FUTEX) && !defined(SATURN_EVENTCOUNT_FORCE_SLEEP)
#include <sys/syscall.h>
#include <unistd.h>
#include <linux/futex.h>
#elif defined(_WIN32) && !defined(_XBOX) && !defined(SATURN_EVENTCOUNT_FORCE_SLEEP)
#include <windows.h>
typedef LONG (WINAPI *nt_wait_t)(volatile void*, LARGE_INTEGER*);
typedef LONG (WINAPI *nt_alert_t)(HANDLE);
static retro_atomic_ptr_t nt_wait, nt_alert;
typedef BOOL (WINAPI *address_wait_t)(volatile VOID*, PVOID, SIZE_T, DWORD);
typedef VOID (WINAPI *address_wake_t)(PVOID);
static retro_atomic_ptr_t address_wait;
static retro_atomic_ptr_t address_wake;
#endif

bool retro_eventcount_init(retro_eventcount_t *ec)
{
   retro_atomic_store_release_int(&spin_budget, cpu_features_get_core_amount() > 1 ? SATURN_EVENTCOUNT_SPIN_ITERS : 0);
   ec->lock = NULL;
   ec->cond = NULL;
   retro_atomic_int_init(&ec->epoch, 0);
   retro_atomic_int_init(&ec->waiters, 0);
#ifdef RETRO_ATOMIC_HAS_PTR
   retro_atomic_ptr_init(&ec->waitlist, NULL);
#endif
#if defined(_WIN32) && !defined(_XBOX) && !defined(SATURN_EVENTCOUNT_FORCE_SLEEP)
   /* Concurrent initializers resolve the same immutable exports. They
    * publish independently; nobody spins waiting for an initializer. */
   {
      HMODULE nt = GetModuleHandleA("ntdll.dll");
      HMODULE module = GetModuleHandleA("kernelbase.dll");
      if (nt)
      {
#ifndef SATURN_EVENTCOUNT_FORCE_ADDRESS_WAIT
         retro_atomic_store_release_ptr(&nt_wait,
               (void*)GetProcAddress(nt, "NtWaitForAlertByThreadId"));
         retro_atomic_store_release_ptr(&nt_alert,
               (void*)GetProcAddress(nt, "NtAlertThreadByThreadId"));
#endif
      }
      if (!module)
         module = GetModuleHandleA("kernel32.dll");
      if (module)
      {
         retro_atomic_store_release_ptr(&address_wait,
               (void*)GetProcAddress(module, "WaitOnAddress"));
         retro_atomic_store_release_ptr(&address_wake,
               (void*)GetProcAddress(module, "WakeByAddressAll"));
      }
   }
#endif
   return true;
}
void retro_eventcount_free(retro_eventcount_t *ec) { (void)ec; }

void retro_eventcount_notify(retro_eventcount_t *ec)
{
   retro_atomic_fetch_add_seq_cst_int(&ec->epoch, 1);
   if (retro_atomic_load_seq_cst_int(&ec->waiters) < 2)
      return;
#if defined(__linux__) && !defined(ANDROID_NO_FUTEX) && !defined(SATURN_EVENTCOUNT_FORCE_SLEEP)
   syscall(SYS_futex, (void*)&ec->epoch, FUTEX_WAKE_PRIVATE,
         INT_MAX, NULL, NULL, 0);
#elif defined(_WIN32) && !defined(_XBOX) && !defined(SATURN_EVENTCOUNT_FORCE_SLEEP)
   {
      address_wake_t wake = (address_wake_t)retro_atomic_load_acquire_ptr(&address_wake);
      nt_alert_t alert = (nt_alert_t)retro_atomic_load_acquire_ptr(&nt_alert);
      nt_wait_t wait = (nt_wait_t)retro_atomic_load_acquire_ptr(&nt_wait);
      if (alert && wait)
      {
         void *tid = retro_atomic_load_acquire_ptr(&ec->waitlist);
         if (tid) alert((HANDLE)tid);
      }
      else if (wake) wake((void*)&ec->epoch);
   }
#endif
}
int retro_eventcount_prepare_wait(retro_eventcount_t *ec)
{
#if defined(_WIN32) && !defined(_XBOX) && !defined(SATURN_EVENTCOUNT_FORCE_SLEEP)
   /* The SPSC endpoints, renderer and CD work event each have one
    * waiter. Store only its thread ID, never a pointer to stack data.
    * An alert arriving before NtWait is remembered by the kernel. */
   if (retro_atomic_exchange_ptr(&ec->waitlist,
         (void*)(uintptr_t)GetCurrentThreadId()) != NULL)
      abort(); /* violated the core's single-waiter contract */
#endif
   retro_atomic_fetch_add_seq_cst_int(&ec->waiters, 1);
   return retro_atomic_load_seq_cst_int(&ec->epoch);
}
void retro_eventcount_cancel_wait(retro_eventcount_t *ec)
{
#if defined(_WIN32) && !defined(_XBOX) && !defined(SATURN_EVENTCOUNT_FORCE_SLEEP)
   retro_atomic_store_release_ptr(&ec->waitlist, NULL);
#endif
   retro_atomic_fetch_sub_int(&ec->waiters, 1);
}
static bool kernel_park(retro_eventcount_t *ec, int key, int64_t timeout_us)
{
   if (retro_atomic_load_seq_cst_int(&ec->epoch) != key)
      return true;
#if defined(__linux__) && !defined(ANDROID_NO_FUTEX) && !defined(SATURN_EVENTCOUNT_FORCE_SLEEP)
   {
      struct timespec ts;
      struct timespec *bound = NULL;
      if (timeout_us >= 0)
      {
         ts.tv_sec = timeout_us / 1000000;
         ts.tv_nsec = (timeout_us % 1000000) * 1000;
         bound = &ts;
      }
      return syscall(SYS_futex, (void*)&ec->epoch, FUTEX_WAIT_PRIVATE,
            key, bound, NULL, 0) == 0 || errno != ETIMEDOUT;
   }
#elif defined(_WIN32) && !defined(_XBOX) && !defined(SATURN_EVENTCOUNT_FORCE_SLEEP)
   {
      address_wait_t wait = (address_wait_t)retro_atomic_load_acquire_ptr(&address_wait);
      address_wake_t wake = (address_wake_t)retro_atomic_load_acquire_ptr(&address_wake);
      nt_wait_t ntwait = (nt_wait_t)retro_atomic_load_acquire_ptr(&nt_wait);
      nt_alert_t alert = (nt_alert_t)retro_atomic_load_acquire_ptr(&nt_alert);
      if (ntwait && alert)
      {
         LARGE_INTEGER bound;
         LONG status;
         int64_t start = timeout_us < 0 ? 0 : cpu_features_get_time_usec();
         int64_t left = timeout_us;
         for (;;)
         {
            if (timeout_us >= 0)
               bound.QuadPart = -(left > INT64_MAX / 10 ? INT64_MAX : left * 10);
            status = ntwait((void*)&ec->epoch, timeout_us < 0 ? NULL : &bound);
            if (status != 0x102) return true; /* spurious wakes are allowed */
            if (timeout_us < 0) continue;
            left = timeout_us - (cpu_features_get_time_usec() - start);
            if (left <= 0) return false;
            /* NT's timeout rounding may expire before the QPC deadline. */
            if (retro_atomic_load_acquire_int(&ec->epoch) != key) return true;
         }
      }
      if (wait && wake)
      {
         DWORD ms = timeout_us < 0 ? INFINITE :
               (DWORD)(timeout_us / 1000 + (timeout_us % 1000 != 0));
         if (timeout_us >= 0 && timeout_us / 1000 >= INFINITE - 1)
            ms = INFINITE - 1;
         return wait((void*)&ec->epoch, &key, sizeof(key), ms) != 0
               || GetLastError() != ERROR_TIMEOUT;
      }
   }
#endif
   /* Platforms without address waits use timed sleeps, never an active
    * spin or a lock-based fallback. Recheck at 1 ms intervals. A wake
    * between the check and sleep only delays progress by one interval. */
   {
      int64_t start = cpu_features_get_time_usec();
      while (retro_atomic_load_acquire_int(&ec->epoch) == key)
      {
         int64_t remaining = timeout_us < 0 ? 1000 :
               timeout_us - (cpu_features_get_time_usec() - start);
         if (remaining <= 0) return false;
         retro_sleep_us((unsigned)(remaining < 1000 ? remaining : 1000));
      }
   }
   return true;
}
/* A short bounded poll avoids parking for work already in flight. No
 * ownership word is spun on; after this budget the thread sleeps. */
static bool address_park(retro_eventcount_t *ec, int key, int64_t timeout_us)
{
   unsigned i, budget = retro_eventcount_spin_iters();
   bool ret;
   for (i = 0; i < budget; i++)
   {
      if (retro_atomic_load_acquire_int(&ec->epoch) != key) return true;
#if defined(_MSC_VER) && (defined(_M_IX86) || defined(_M_X64))
      _mm_pause();
#elif defined(__i386__) || defined(__x86_64__)
      __asm__ __volatile__("pause");
#elif defined(__aarch64__)
      __asm__ __volatile__("yield");
#endif
   }
   /* 1 means registered, 2 means about to park. The seq_cst RMW and
    * epoch recheck pair with notify's RMW/load: either the predicate
    * changed before parking, or the notifier sees a parked waiter.
    * Notifications during the bounded poll need no kernel call. */
   retro_atomic_fetch_add_seq_cst_int(&ec->waiters, 1);
   ret = kernel_park(ec, key, timeout_us);
   retro_atomic_fetch_sub_int(&ec->waiters, 1);
   return ret;
}
void retro_eventcount_commit_wait(retro_eventcount_t *ec, int key)
{
   address_park(ec, key, -1);
   retro_eventcount_cancel_wait(ec);
}
bool retro_eventcount_commit_wait_timeout(retro_eventcount_t *ec,
      int key, int64_t timeout_us)
{
   bool ret = address_park(ec, key, timeout_us < 0 ? 0 : timeout_us);
   retro_eventcount_cancel_wait(ec);
   return ret;
}
unsigned retro_eventcount_spin_iters(void)
{ return (unsigned)retro_atomic_load_acquire_int(&spin_budget); }
const char *retro_eventcount_backend_name(void)
{
#if defined(__linux__) && !defined(ANDROID_NO_FUTEX) && !defined(SATURN_EVENTCOUNT_FORCE_SLEEP)
   return "futex address wait";
#elif defined(_WIN32) && !defined(_XBOX) && !defined(SATURN_EVENTCOUNT_FORCE_SLEEP)
   if (retro_atomic_load_acquire_ptr(&nt_wait) && retro_atomic_load_acquire_ptr(&nt_alert))
      return "NT thread alert";
   return retro_atomic_load_acquire_ptr(&address_wait)
      && retro_atomic_load_acquire_ptr(&address_wake)
      ? "WaitOnAddress" : "timed sleep";
#else
   return "timed sleep";
#endif
}
