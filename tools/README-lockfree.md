The CD reader transfers exclusive sector ownership through two
`retro_waitable_spsc` queues. The worker owns backend I/O; the emulator
owns the 256-entry sector cache. A demand mailbox has one outstanding
request, and advisory hints coalesce. Speculative reads stop when the
32-slot transfer pool fills, so they cannot obstruct a demand. Shutdown
publishes stop, wakes/cancels the endpoints, joins, then frees memory.

The renderer retains its command and burst SPSC rings. Release/acquire
cursors publish payloads and completion. Eventcounts park the consumer,
apply queue/burst back-pressure, and wait for draw/state drains. Builds
with `NEED_THREADING=0` consume the same commands synchronously.

`mednafen/thread_eventcount.c` implements the common API with namespaced
symbols and requires lock-free word atomics. Each event has one waiting
thread. Windows uses `WaitOnAddress` / `WakeByAddressAll`, resolved from
KernelBase with a Kernel32 fallback, and Linux uses private futex waits.
These wakes target the event's epoch address. NT thread alerts are avoided:
a notification racing with the end of a core wait can remain pending on
the thread and wake the frontend's next, unrelated eventcount wait. Some
frontend waiter-list implementations return on that alert without removing
their stack waiter, leaving a dangling pointer for the next notification.
Platforms without address waits recheck using timed sleeps. Upstream common
threading implementations and their platform fallbacks remain intact.

On multiple processors, a bounded poll of 256 iterations precedes parking
to avoid a kernel round trip when work is already in flight. Notifications
only call the kernel when the waiter has committed to parking. This is a
completion check with a fixed budget; no thread spins to acquire a lock.
`SATURN_EVENTCOUNT_SPIN_ITERS=0` disables this budget at compile time.

To reproduce the deterministic checks with GNU make, GCC and Python:

```
python tools/check_lockfree.py --zstd-library /path/to/libzstd
python tools/check_lockfree.py --no-threading --zstd-library /path/to/libzstd
python tools/check_build_matrix.py
# Optional Windows integration with unchanged RetroArch eventcount source:
python tools/check_lockfree.py --zstd-library /path/to/libzstd \
    --frontend-eventcount-source /path/to/RetroArch/libretro-common/rthreads/retro_eventcount.c
```

The stress runner cleans and rebuilds the core. Its tests need no BIOS or
game image. The Zstd shared library is an independent fixture encoder and
is only a test dependency; the core uses `rzstd`. Python supplies the
reference DEFLATE/LZMA encoders, and the fixture writer creates verbatim
FLAC frames independently of the core decoder.

Tests cover sequential/random CD reads, cache hits, errors, hint floods,
slow I/O, inclusive LBA boundaries, parked/pending shutdown, eventcount
wrap/timeout/cancel windows, one million Windows waits with no escaped NT
alerts, optional frontend stack-waiter cleanup checks, tiny renderer queues, bursts, actual draw
completion, state drains and restart. CHD fixtures cover v3/v4 and v5
stored/compressed maps, zlib/LZMA/Zstd/FLAC and CD codec variants. ZIP tests
cover stored/DEFLATE/empty entries, multi-chunk compressed input, short
frontend reads, and wrong CRC/length/truncated data. The I/O harness also
checks mapped/local CD streams, EOF, frontend streams and heap precaches.

Handoff throughput and median/p99 latency are synthetic measurements of
the eventcount protocol, with microsecond timestamp resolution. They are
not game performance measurements. End-to-end BIOS/game testing and
runtime checks on other operating systems remain separate validation.
