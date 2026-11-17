#!/bin/sh
# calendar_rtc_start_tm regression test (calendar_rtc.c).
#
#   sh tools/calendar_rtc_test.sh              under ASan + UBSan
#   MINGW=1 sh tools/calendar_rtc_test.sh      also mingw-w64 under Wine
#
# Runs with TZ=PST8PDT, so a localtime() slip changes the hour.
set -e
ROOT=$(cd "$(dirname "$0")/.." && pwd)
cd "$ROOT"
WORK=${TMPDIR:-/tmp}/calendar_rtc.$$
mkdir -p "$WORK"
trap 'rm -rf "${WORK:?}"' EXIT

SRC="tools/calendar_rtc_test.c calendar_rtc.c"
FLAGS="-std=c89 -pedantic -O1 -g -Wall -Ilibretro-common/include -I."
# Sanitizer shadow ranges can collide with the 32-bit mmap randomisation
# of current kernels, as on GitHub's ubuntu-24.04 runners
NORAND=
setarch "$(uname -m)" -R true 2>/dev/null && NORAND="setarch $(uname -m) -R"

${CC:-cc} $FLAGS -fsanitize=address,undefined -fno-sanitize-recover=undefined \
   -o "$WORK/t" $SRC
TZ=PST8PDT $NORAND "$WORK/t"

if [ -n "$MINGW" ]; then
   # Wine makes a prefix only under a directory the user owns
   WINEPREFIX=${WINEPREFIX:-$WORK/wineprefix}
   export WINEPREFIX
   x86_64-w64-mingw32-gcc $FLAGS -o "$WORK/t.exe" $SRC -static
   ( cd "$WORK" && TZ=PST8PDT WINEDEBUG=-all timeout 120 wine ./t.exe )
fi
