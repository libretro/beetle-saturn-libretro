#!/bin/sh
# Build the core as the libretro linux-x64 image does: GCC 9 handing its
# LTO output to the binutils 2.26 assembler of Ubuntu 16.04.  Code that
# needs a newer assembler (AVX-512 VPCLMULQDQ, for one) only fails at the
# final link there, which no current distribution reproduces.
#
#   sh tools/old_binutils_build.sh       needs gcc-9, g++-9, curl, dpkg-deb
#
# BINUTILS_CACHE keeps the downloaded package between runs.
set -e
ROOT=$(cd "$(dirname "$0")/.." && pwd)
cd "$ROOT"

DEB=binutils_2.26.1-1ubuntu1~16.04.8_amd64.deb
SHA=5a82213d14c172ebd9b6a4dbc5b69c0f8e9b1c9de4665aaa2c5e3c0c1fde988e
CACHE=${BINUTILS_CACHE:-${TMPDIR:-/tmp}/old-binutils}
mkdir -p "$CACHE"

if ! echo "$SHA  $CACHE/$DEB" | sha256sum -c - >/dev/null 2>&1; then
   rm -f "$CACHE/$DEB"
   for m in http://archive.ubuntu.com/ubuntu http://old-releases.ubuntu.com/ubuntu; do
      curl -fsSL -o "$CACHE/$DEB" "$m/pool/main/b/binutils/$DEB" && break
   done
   echo "$SHA  $CACHE/$DEB" | sha256sum -c -
fi

BIN=$CACHE/bin
rm -rf "$CACHE/root" "$BIN"
dpkg-deb -x "$CACHE/$DEB" "$CACHE/root"
mkdir -p "$BIN"
cat > "$BIN/as" <<EOF
#!/bin/sh
LD_LIBRARY_PATH=$CACHE/root/usr/lib/x86_64-linux-gnu exec $CACHE/root/usr/bin/as "\$@"
EOF
chmod +x "$BIN/as"
"$BIN/as" --version | head -n 1

# Distribution GCC turns on CET, which this assembler predates; the
# libretro image's GCC 9 does not
T="-B$BIN/ -fcf-protection=none"
make clean >/dev/null
make -j"$(nproc)" platform=unix CC="gcc-9 $T" CXX="g++-9 $T"
echo "old binutils build: PASS"
