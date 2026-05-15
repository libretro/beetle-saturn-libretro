#!/usr/bin/env python3
# -----------------------------------------------------------------------------
# check_engine_inlined.py
#
# Guard against the performance regression class introduced by the C++ -> C
# conversions: a C++ rasterizer *template* (DrawLine<...>, PlotPixel<...>,
# TexFetch<...>) is replaced in C by ONE function taking the former template
# parameters as `const` arguments, plus thin per-slot wrapper functions that
# call it with literal constants. That only matches the template's performance
# if the engine function is *force-inlined* into every wrapper -- then the
# const args fold and the dead branches strip, exactly like template
# specialization.
#
# If the engine is marked plain INLINE (a hint) instead of MDFN_FORCE_INLINE,
# the compiler refuses it at -O2 (the function is far too big to inline into
# ~1728 sites) and instead emits ONE generic out-of-line copy with all 13
# branches live, plus ~1728 six-byte thunks. Correct output, but every pixel
# now runs runtime branches the C++ build had compiled away. This actually
# shipped in the VDP1 conversion (VDP1_DrawLine_impl / VDP1_PlotPixel /
# TexFetch_impl were `static INLINE`).
#
# Fingerprint, and what this script checks: when force-inlining succeeds, the
# engine function DISAPPEARS from the compiled object -- it has no standalone
# symbol because it was inlined everywhere. When it fails, the symbol is
# present (often as `<name>` or `<name>.constprop.N`). So: compile each
# converted module at the core's real -O2 and fail if any engine function is
# still a defined symbol.
#
# This is deterministic, needs no C++ baseline, and runs in CI. Run it before
# and after converting VDP2 / SCU.
#
#   python3 tools/check_engine_inlined.py
#
# When you convert a new subsystem: add its module + engine function names to
# ENGINES below. An engine function is any C stand-in for a C++ template whose
# whole point is to be inlined into per-slot wrappers.
# -----------------------------------------------------------------------------

import os
import subprocess
import sys

REPO = os.path.normpath(os.path.join(os.path.dirname(__file__), ".."))
SS = os.path.join("mednafen", "ss")

# Include flags mirror Makefile.common's INCFLAGS (the ss-relevant subset).
INCLUDES = [
    "-I" + REPO,
    "-I" + os.path.join(REPO, "mednafen"),
    "-I" + os.path.join(REPO, "mednafen", "include"),
    "-I" + os.path.join(REPO, "mednafen", "intl"),
    "-I" + os.path.join(REPO, "mednafen", "hw_sound"),
    "-I" + os.path.join(REPO, "mednafen", "hw_cpu"),
    "-I" + os.path.join(REPO, "mednafen", "hw_misc"),
    "-I" + os.path.join(REPO, "libretro-common", "include"),
]

# module .c  ->  engine functions that MUST be inlined away (no standalone symbol)
ENGINES = {
    os.path.join(SS, "vdp1.c"):        ["TexFetch_impl"],
    os.path.join(SS, "vdp1_line.c"):   ["VDP1_DrawLine_impl", "VDP1_PlotPixel"],
    os.path.join(SS, "vdp1_sprite.c"): ["VDP1_DrawLine_impl", "VDP1_PlotPixel"],
    os.path.join(SS, "vdp1_poly.c"):   ["VDP1_DrawLine_impl", "VDP1_PlotPixel"],
    # VDP2 / SCU: add modules + engine functions here as they are converted.
}


def defined_symbols(obj_path):
    """Names of functions/objects DEFINED in an object file (nm types T/t/W/w/...)."""
    out = subprocess.run(["nm", obj_path], capture_output=True, text=True).stdout
    names = set()
    for line in out.splitlines():
        parts = line.split()
        if len(parts) >= 3 and parts[1] not in ("U", "u"):  # U = undefined
            names.add(parts[2])
    return names


def main():
    cc = os.environ.get("CC", "cc")
    failures = []
    checked = 0

    for rel_c, engines in sorted(ENGINES.items()):
        src = os.path.join(REPO, rel_c)
        if not os.path.isfile(src):
            print("WARN: %s not found (skipped)" % rel_c)
            continue

        obj = src + ".inlinecheck.o"
        cmd = [cc, "-std=gnu99", "-O2", "-c", src, "-o", obj] + INCLUDES
        proc = subprocess.run(cmd, capture_output=True, text=True)
        if proc.returncode != 0:
            print("ERROR: failed to compile %s:\n%s" % (rel_c, proc.stderr.strip()))
            return 2

        syms = defined_symbols(obj)
        os.remove(obj)
        checked += 1

        for fn in engines:
            # force-inlined away  -> no symbol at all
            # inlining failed     -> `fn` or `fn.constprop.N` / `fn.isra.N` etc.
            leaked = sorted(s for s in syms if s == fn or s.startswith(fn + "."))
            if leaked:
                failures.append((rel_c, fn, leaked))

    if failures:
        print("ERROR: template-engine function(s) were NOT inlined away -- the")
        print("       per-slot specialization has collapsed into runtime branching.")
        print("       Mark the function MDFN_FORCE_INLINE (not plain INLINE).\n")
        for rel_c, fn, leaked in failures:
            print("  %-24s %s left standalone symbol(s): %s"
                  % (os.path.basename(rel_c), fn, ", ".join(leaked)))
        return 1

    print("check_engine_inlined: OK (%d modules compiled at -O2, every template "
          "engine fully inlined)" % checked)
    return 0


if __name__ == "__main__":
    sys.exit(main())
