#!/usr/bin/env python3
"""
check_build_matrix.py

Compile-syntax-check every project TU under each supported
build-flag combination, to surface "decl gated, use ungated"
regressions before they ship.

The Makefile hardcodes NEED_DEINTERLACER=1, HAVE_CHD=1, etc., so a
local default build is never the alternate-flag build.  This guard
walks the matrix.

Currently the matrix is:
  - default LE
  - default BE        (MSB_FIRST)
  - no_deint LE       (NEED_DEINTERLACER off)
  - no_chd LE         (HAVE_CHD off; CDAccess_CHD.c excluded)
  - no_tremor LE      (NEED_TREMOR off)
  - no_threading LE   (NEED_THREADING off)
  - m68k_split LE     (M68K_SPLIT_SWITCH on)

Reports the first failure per (config, TU) pair.  Exit 0 on all-OK.
"""

import os
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
os.chdir(ROOT)

INCLUDES = [
    ".",
    "mednafen", "mednafen/include", "mednafen/intl",
    "mednafen/hw_sound", "mednafen/hw_cpu", "mednafen/hw_misc",
    "libretro-common/include",
    "deps/libchdr", "deps/libchdr/include", "deps/libchdr/include/libchdr",
]
INCLUDE_FLAGS = [f"-I{p}" for p in INCLUDES]

DEFAULTS = (
    "-DWANT_THREADING -DHAVE_THREADS -DNEED_DEINTERLACER -DWANT_32BPP "
    "-DNO_COMPUTED_GOTO -DNEED_CD -DHAVE_CHD -D_7ZIP_ST -DZSTD_DISABLE_ASM "
    "-DNEED_TREMOR"
).split()

CONFIGS = {
    "default_LE":  DEFAULTS,
    "default_BE":  DEFAULTS + ["-DMSB_FIRST"],
    "no_deint_LE":  [f for f in DEFAULTS if f != "-DNEED_DEINTERLACER"],
    "no_chd_LE":    [f for f in DEFAULTS
                     if f not in ("-DHAVE_CHD", "-D_7ZIP_ST", "-DZSTD_DISABLE_ASM")],
    "no_tremor_LE": [f for f in DEFAULTS if f != "-DNEED_TREMOR"],
    "no_threading_LE": [f for f in DEFAULTS
                       if f not in ("-DWANT_THREADING", "-DHAVE_THREADS")],
    "m68k_split_LE":   DEFAULTS + ["-DM68K_SPLIT_SWITCH"],
}

# Source list — derived from a quick Makefile.common parse.  Limited to
# project sources, third-party (libretro-common/, deps/, tremor/, sound/)
# excluded.  CHD-only file excluded from no_chd config.
SOURCE_GLOBS = [
    "mednafen/ss/*.c", "mednafen/ss/*.cpp",
    "mednafen/ss/cart/*.c",
    "mednafen/ss/input/*.cpp",
    "mednafen/cdrom/*.c",
    "mednafen/hash/*.c",
    "mednafen/hw_cpu/m68k/*.cpp",
    "mednafen/video/*.c", "mednafen/video/*.cpp",
    "mednafen/*.c", "mednafen/*.cpp",
    "libretro.cpp", "disc.cpp", "input.cpp",
]

def project_sources():
    out = []
    for g in SOURCE_GLOBS:
        for p in sorted(ROOT.glob(g)):
            rel = p.relative_to(ROOT).as_posix()
            out.append(rel)
    # Strip duplicates while preserving order
    seen = set(); result = []
    for s in out:
        if s not in seen:
            seen.add(s); result.append(s)
    return result

def compile_one(f, flags):
    cc = ["g++", "-std=c++11"] if f.endswith(".cpp") else ["gcc", "-std=gnu99"]
    cmd = cc + ["-O2"] + INCLUDE_FLAGS + flags + ["-fsyntax-only", f]
    r = subprocess.run(cmd, capture_output=True, text=True)
    if r.returncode != 0:
        # Filter dllexport warnings on non-Windows hosts
        err = "\n".join(
            l for l in r.stderr.splitlines()
            if "dllexport" not in l and "[-Wattributes]" not in l
        )
        # If the error was *only* dllexport noise, accept
        # (gcc returns nonzero only on hard errors; warnings stay
        # in stderr but don't cause returncode != 0).  Check for
        # real "error:" lines.
        if re.search(r"\berror:", err):
            return err
    return None

def main():
    sources = project_sources()
    # Files that are only in SOURCES under specific flags, per
    # Makefile.common.  Excluded from configs where the flag is off.
    M68K_SPLIT_ONLY = (
        "mednafen/hw_cpu/m68k/m68k_instr_split0.cpp",
        "mednafen/hw_cpu/m68k/m68k_instr_split1.cpp",
    )
    CHD_ONLY = ("mednafen/cdrom/CDAccess_CHD.c",)

    print(f"check_build_matrix: {len(sources)} sources, {len(CONFIGS)} configs")
    total_fail = 0
    for cfg_name, flags in CONFIGS.items():
        cfg_sources = list(sources)
        # Per-config source filtering, mirroring Makefile.common's
        # conditional SOURCES_C/CXX blocks.
        if cfg_name != "m68k_split_LE":
            cfg_sources = [s for s in cfg_sources if s not in M68K_SPLIT_ONLY]
        if cfg_name == "no_chd_LE":
            cfg_sources = [s for s in cfg_sources if s not in CHD_ONLY]
        fails = []
        for f in cfg_sources:
            err = compile_one(f, flags)
            if err:
                fails.append((f, err))
        if fails:
            print(f"  {cfg_name}: {len(fails)} failure(s)")
            for f, err in fails[:5]:
                print(f"    --- {f} ---")
                for line in err.splitlines()[:5]:
                    print(f"      {line}")
            total_fail += len(fails)
        else:
            print(f"  {cfg_name}: OK ({len(cfg_sources)} TUs)")
    if total_fail:
        sys.exit(1)
    print("check_build_matrix: OK (all configs, all TUs)")

if __name__ == "__main__":
    main()
