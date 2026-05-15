#!/usr/bin/env python3
# -----------------------------------------------------------------------------
# check_no_mirrored_constants.py
#
# Guard against the bug class that broke the VDP1 C conversion: a module
# converted from C++ to C can no longer #include the still-C++ subsystem
# headers (ss.h, scu.h, vdp2.h, ...), so whoever did the conversion re-typed
# the enum values it needed by hand -- and miscounted. SS_EVENT_VDP1 became 7
# instead of 6, SCU_INT_VDP1 became 4 instead of 13, and games hung right
# after the BIOS handoff.
#
# The fix is structural: cross-boundary constants live in ss_c_abi.h, a plain
# leaf header valid in both C and C++, included by BOTH the C++ owners and the
# converted C modules. One definition, so nothing can drift.
#
# This script enforces that. It fails if any converted C module (*.c under
# mednafen/ss/) locally defines a constant name -- via `#define` or an `enum`
# enumerator -- that a C++ header in the same directory also defines. That
# collision is the signature of a hand-transcribed mirror. The cure is always
# the same: delete the local copy, put the constant in ss_c_abi.h, #include it.
#
# Run before converting VDP2 / SCU (and from CI):
#     python3 tools/check_no_mirrored_constants.py
# Exit status 0 = clean, 1 = transcription found.
# -----------------------------------------------------------------------------

import os
import re
import sys

SS_DIR = os.path.join(os.path.dirname(__file__), "..", "mednafen", "ss")

# ss_c_abi.h is the sanctioned shared header -- it is *meant* to define these
# names for both worlds, so it is never a source of "mirroring".
SHARED_HEADER = "ss_c_abi.h"

DEFINE_RE = re.compile(r"^\s*#\s*define\s+([A-Za-z_][A-Za-z0-9_]*)\b")
ENUM_BLOCK_RE = re.compile(r"\benum\b[^{;]*\{(.*?)\}", re.DOTALL)
ENUMERATOR_RE = re.compile(r"([A-Za-z_][A-Za-z0-9_]*)\s*(?:=[^,}]*)?")


def strip_comments(text):
    text = re.sub(r"/\*.*?\*/", " ", text, flags=re.DOTALL)
    text = re.sub(r"//[^\n]*", " ", text)
    return text


def constants_defined_in(path):
    """Return the set of object-like-macro and enumerator names a file defines."""
    with open(path, "r", errors="replace") as fh:
        raw = fh.read()
    names = set()

    # Object-like #defines (skip function-like macros: NAME immediately ( ).
    for line in raw.splitlines():
        m = DEFINE_RE.match(line)
        if not m:
            continue
        name = m.group(1)
        after = line[m.end():]
        if after.startswith("("):  # function-like macro, not a constant
            continue
        names.add(name)

    # enum { ... } enumerators
    code = strip_comments(raw)
    for block in ENUM_BLOCK_RE.findall(code):
        for body_item in block.split(","):
            m = ENUMERATOR_RE.match(body_item.strip())
            if m and m.group(1):
                names.add(m.group(1))

    return names


def main():
    ss_dir = os.path.normpath(SS_DIR)
    if not os.path.isdir(ss_dir):
        print("check_no_mirrored_constants: cannot find %s" % ss_dir, file=sys.stderr)
        return 2

    c_files = []
    h_files = []
    for entry in sorted(os.listdir(ss_dir)):
        full = os.path.join(ss_dir, entry)
        if not os.path.isfile(full):
            continue
        if entry.endswith(".c"):
            c_files.append(full)
        elif entry.endswith(".h") and entry != SHARED_HEADER:
            h_files.append(full)

    # name -> header file that defines it (the authoritative C++ side)
    header_defs = {}
    for hf in h_files:
        for name in constants_defined_in(hf):
            header_defs.setdefault(name, hf)

    violations = []
    for cf in c_files:
        for name in sorted(constants_defined_in(cf)):
            if name in header_defs:
                violations.append((cf, name, header_defs[name]))

    if violations:
        print("ERROR: converted C module(s) transcribe constants that a C++ "
              "header also defines.")
        print("       This is exactly the bug that broke the VDP1 conversion. "
              "Move the")
        print("       constant into mednafen/ss/%s and #include it from both "
              "sides.\n" % SHARED_HEADER)
        for cf, name, hf in violations:
            print("  %-24s defines %-32s also defined in %s"
                  % (os.path.basename(cf), name, os.path.basename(hf)))
        return 1

    print("check_no_mirrored_constants: OK (%d C modules, %d C++ headers "
          "scanned, no transcribed constants)" % (len(c_files), len(h_files)))
    return 0


if __name__ == "__main__":
    sys.exit(main())
