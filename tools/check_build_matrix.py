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
    "-DNEED_TREMOR -DNDEBUG"
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
    "mednafen/hw_cpu/m68k/*.c", "mednafen/hw_cpu/m68k/*.cpp",
    "mednafen/video/*.c", "mednafen/video/*.cpp",
    "mednafen/*.c", "mednafen/*.cpp",
    "libretro.c", "disc.c", "input.c",
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

# C++-in-C-file detection.  -std=gnu99 accepts several C++ constructs
# as GCC extensions that stricter C compilers (older mingw, MSVC,
# embedded toolchains) reject outright.  The smpc/cdb cpp-to-c
# conversions shipped `enum : int { ... }` (C2X typed-enum syntax)
# because gnu99 silently accepted it -- broke the user's
# x86_64-w64-mingw32.static-gcc build.  This pre-compile pattern gate
# catches the class of leakage before the matrix compile step.
#
# Patterns match the *visible* code after stripping /* ... */ and
# // ... comments -- a typed-enum mention in a comment doesn't trip.
CXX_IN_C_PATTERNS = [
    (re.compile(r'^[ \t]*enum[ \t]*:[ \t]*\w', re.MULTILINE),
     "C++ typed enum -- use plain `enum { NAME = VAL };` (C: values are int)"),
    (re.compile(r'(?<![A-Za-z0-9_])template[ \t]*<'),
     "C++ template"),
    (re.compile(r'^[ \t]*namespace[ \t]+\w', re.MULTILINE),
     "C++ namespace"),
    (re.compile(r'(?<![A-Za-z0-9_])nullptr(?![A-Za-z0-9_])'),
     "C++ nullptr -- use NULL"),
    (re.compile(r'(?<![A-Za-z0-9_])static_assert[ \t]*\('),
     "C++ static_assert -- use _Static_assert (or the portable "
     "typedef-char-array trick: typedef char NAME[1-2*!cond];)"),
    (re.compile(r'^[ \t]*class[ \t]+[A-Za-z_]\w*[ \t]*[{:]', re.MULTILINE),
     "C++ class"),
    (re.compile(r'(?<![A-Za-z0-9_])extern[ \t]+"C"[ \t]*\{?'),
     "C++ `extern \"C\"` (C files don't need it; remove or guard with #ifdef __cplusplus)"),
    (re.compile(r'(?<![A-Za-z0-9_])alignas[ \t]*\('),
     "C++ alignas -- C wants `_Alignas` (C11)"),
    (re.compile(r'#[ \t]*include[ \t]*<(atomic|algorithm|array|vector|string|memory'
                r'|iostream|cstdint|cstdio|cstring|cstdlib|chrono|thread'
                r'|mutex|condition_variable|functional|tuple|map|set'
                r'|unordered_map|unordered_set|list|deque)>'),
     "C++ STL header"),
]

# Function-definition unnamed-parameter detection.  C requires names on
# function-definition parameters (only declarations may omit them);
# C++ allows omitting names in either.  Two definitions slipped through
# libretro.c -> .c with unnamed params and broke the standard Linux
# build.  This catches them in the gate.
#
# We match `<rtype> <name>(<params>) {` and tokenize <params> to flag
# any param that has no identifier (e.g. `unsigned`, `const char *`,
# `size_t`, plain pointer-to-struct with no name).
TYPE_KEYWORDS = {
    'unsigned', 'signed', 'int', 'bool', 'void', 'char', 'short',
    'long', 'float', 'double', 'size_t', 'ssize_t', 'ptrdiff_t',
}
FN_DEF_RX = re.compile(
    r'^(?P<rtype>[\w *]+?)\s+'
    r'(?P<name>[a-zA-Z_][a-zA-Z0-9_]*)\s*'
    r'\((?P<params>[^()]*)\)\s*'
    r'(?:\n[ \t]*)?\{',
    re.MULTILINE
)
def _param_is_unnamed(p):
    """Return True if param string `p` lacks an identifier name."""
    p = p.strip()
    if not p or p == 'void' or p == '...':
        return False
    # Pointer-to-anything with nothing after: `const char *`, `struct X *`
    if p.endswith('*'):
        return True
    # Tokenize: drop pointer chars + brackets, look at last word
    toks = re.findall(r'[a-zA-Z_][a-zA-Z0-9_]*|\*+', p)
    if not toks:
        return False
    last = toks[-1]
    if last in TYPE_KEYWORDS:
        return True
    # Typedef-like (_t suffix) without trailing identifier
    if re.match(r'.*_t$', last) and len(toks) == 1:
        return True
    return False

def strip_comments(src):
    """Replace /* block */ and // line comments with same-length
    whitespace so pattern matches don't trip on in-comment text,
    while line numbers stay aligned to the original source.  Newlines
    inside block comments are preserved; everything else inside a
    comment becomes a space."""
    def _block(m):
        s = m.group(0)
        # Keep newlines, blank out everything else
        return "".join(ch if ch == "\n" else " " for ch in s)
    src = re.sub(r'/\*.*?\*/', _block, src, flags=re.DOTALL)
    # Line comments: leave the newline alone
    src = re.sub(r'//[^\n]*', lambda m: " " * len(m.group(0)), src)
    return src

def cxx_pattern_check(f):
    """Return None on clean, or a list of (line_no, pattern_msg, line_text)
    on detection.  Applies to .c files only."""
    if not f.endswith(".c"):
        return None
    with open(f, "r", encoding="utf-8", errors="replace") as fh:
        raw = fh.read()
    cleaned = strip_comments(raw)
    raw_lines = raw.split("\n")
    fails = []
    for rx, msg in CXX_IN_C_PATTERNS:
        for m in rx.finditer(cleaned):
            line_no = cleaned.count("\n", 0, m.start()) + 1
            raw_line = raw_lines[line_no - 1] if line_no <= len(raw_lines) else ""
            fails.append((line_no, msg, raw_line.strip()))
    # Unnamed-parameter detection: function definitions only.
    for m in FN_DEF_RX.finditer(cleaned):
        params = m.group('params').strip()
        if not params:
            continue
        parts = [p.strip() for p in params.split(',')]
        if any(_param_is_unnamed(p) for p in parts):
            line_no = cleaned.count("\n", 0, m.start()) + 1
            raw_line = raw_lines[line_no - 1] if line_no <= len(raw_lines) else ""
            fails.append((
                line_no,
                "C function definition with unnamed parameter (C requires "
                "names on definitions; only declarations may omit them)",
                raw_line.strip()
            ))
    return fails or None

def main():
    sources = project_sources()

    # Run the C++-in-C pattern gate first.  This is the cheapest
    # check and surfaces the class of leakage that -std=gnu99
    # silently swallows (typed enums, nullptr, etc).  Failures here
    # are reported with file:line and the offending source text.
    print("check_build_matrix: scanning .c files for C++ leakage")
    pattern_fails = 0
    for f in sources:
        hits = cxx_pattern_check(f)
        if hits:
            for line_no, msg, line_text in hits:
                print(f"  {f}:{line_no}: {msg}")
                print(f"    {line_text[:120]}")
                pattern_fails += 1
    if pattern_fails:
        print(f"check_build_matrix: {pattern_fails} C++-in-C pattern hit(s) "
              "-- fix before compile gate runs")
        sys.exit(1)
    print("  -- C++-in-C scan clean")

    # Files that are only in SOURCES under specific flags, per
    # Makefile.common.  Excluded from configs where the flag is off.
    M68K_SPLIT_ONLY = (
        "mednafen/hw_cpu/m68k/m68k_instr_split0.c",
        "mednafen/hw_cpu/m68k/m68k_instr_split1.c",
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
