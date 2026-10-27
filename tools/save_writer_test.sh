#!/bin/sh
# Save writer regression test (mednafen/save_writer.c).
#
#   sh tools/save_writer_test.sh              threaded under ASan + UBSan,
#                                             under TSan, and unthreaded
#   MINGW=1 sh tools/save_writer_test.sh      also mingw-w64 under Wine
set -e
ROOT=$(cd "$(dirname "$0")/.." && pwd)
cd "$ROOT"
LC=libretro-common
WORK=${TMPDIR:-/tmp}/save_writer.$$
mkdir -p "$WORK"
trap 'rm -rf "$WORK"' EXIT

BASE="tools/save_writer_test.c mednafen/save_writer.c
      $LC/streams/file_stream.c $LC/vfs/vfs_implementation.c
      $LC/file/file_path.c $LC/file/file_path_io.c
      $LC/compat/compat_strl.c $LC/compat/fopen_utf8.c
      $LC/encodings/encoding_utf.c $LC/string/stdstring.c
      $LC/string/rstrtod.c $LC/time/rtime.c $LC/features/features_cpu.c"
THREADS="$LC/rthreads/rthreads.c $LC/queues/retro_spsc.c
      mednafen/thread_queue.c mednafen/thread_eventcount.c
      mednafen/thread_eventcount_common.c"
FLAGS="-O1 -g -Wall -I$LC/include -Imednafen -I."
MT="-DWANT_THREADING -DHAVE_THREADS"

${CC:-cc} $FLAGS $MT -fsanitize=address,undefined -fno-sanitize-recover=undefined \
   -o "$WORK/t" $BASE $THREADS -lpthread
ASAN_OPTIONS=detect_leaks=1:abort_on_error=1 "$WORK/t" "$WORK"

${CC:-cc} $FLAGS $MT -fsanitize=thread -o "$WORK/t_tsan" $BASE $THREADS -lpthread
TSAN_OPTIONS=halt_on_error=1 "$WORK/t_tsan" "$WORK"

${CC:-cc} $FLAGS -fsanitize=address,undefined -fno-sanitize-recover=undefined \
   -o "$WORK/t_st" $BASE
"$WORK/t_st" "$WORK"

if [ -n "$MINGW" ]; then
   x86_64-w64-mingw32-gcc $FLAGS $MT -o "$WORK/t.exe" $BASE $THREADS -static
   ( cd "$WORK" && WINEDEBUG=-all timeout 120 wine ./t.exe . )
fi
