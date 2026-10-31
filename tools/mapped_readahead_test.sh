#!/bin/sh
# Mapped disc image read-ahead regression test.
#
#   sh tools/mapped_readahead_test.sh            Linux, ASan + UBSan, leaks on
#   MINGW=1 sh tools/mapped_readahead_test.sh    also mingw-w64 under Wine,
#                                                in each RETRO_VFS_PREFETCH mode,
#                                                and with a slow stand-in for
#                                                PrefetchVirtualMemory: no read
#                                                waits for it, it is never on
#                                                the reading thread
set -e
ROOT=$(cd "$(dirname "$0")/.." && pwd)
cd "$ROOT"
LC=libretro-common
WORK=${TMPDIR:-/tmp}/mapped_readahead.$$
mkdir -p "$WORK"
trap 'rm -rf "$WORK"' EXIT

SRC="tools/mapped_readahead_test.c mednafen/cdstream.c
     mednafen/thread_eventcount.c mednafen/thread_eventcount_common.c
     $LC/streams/file_stream.c $LC/vfs/vfs_implementation.c
     $LC/file/file_path.c $LC/file/file_path_io.c
     $LC/compat/compat_strl.c $LC/compat/fopen_utf8.c
     $LC/encodings/encoding_utf.c $LC/string/stdstring.c
     $LC/string/rstrtod.c $LC/time/rtime.c $LC/rthreads/rthreads.c
     $LC/features/features_cpu.c"
FLAGS="-O1 -g -Wall -DHAVE_THREADS -I$LC/include -Imednafen -I."
WRAP="-Wl,--wrap=filestream_prefetch"
# Sanitizer shadow ranges can collide with the 32-bit mmap randomisation
# of current kernels, as on GitHub's ubuntu-24.04 runners
NORAND=
setarch "$(uname -m)" -R true 2>/dev/null && NORAND="setarch $(uname -m) -R"

${CC:-cc} $FLAGS -DHAVE_MMAP -fsanitize=address,undefined \
   -fno-sanitize-recover=undefined -o "$WORK/t" $SRC $WRAP -lpthread
( cd "$WORK" && ASAN_OPTIONS=detect_leaks=1:abort_on_error=1 $NORAND ./t )

if [ -n "$MINGW" ]; then
   x86_64-w64-mingw32-gcc $FLAGS -o "$WORK/t.exe" $SRC $WRAP -static
   for m in "" touch none; do
      ( cd "$WORK" && RETRO_VFS_PREFETCH=$m WINEDEBUG=-all \
         timeout 120 wine ./t.exe )
   done
   # PrefetchVirtualMemory a stand-in that sleeps like a cold disk: the
   # reads never wait for it, and it never runs on the reader's thread
   x86_64-w64-mingw32-gcc $FLAGS -o "$WORK/t_hook.exe" $SRC $WRAP -static \
      -DVFS_WIN32_PREFETCH_HOOK=saturn_prefetch_hook \
      -include tools/mapped_readahead_hook.h
   ( cd "$WORK" && WINEDEBUG=-all timeout 120 wine ./t_hook.exe )
fi
