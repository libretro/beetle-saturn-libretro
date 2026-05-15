#!/usr/bin/env python3
"""
Scan for the dangling-else bug class introduced by f827e0a-style folds.

Bug shape:
  if (outer_cond)              <-- unbraced parent
    if (X > Y) Y = X;          <-- unbraced fold (single statement)
  else                         <-- BINDS TO INNER if, not outer
    ...

OR with else:
  else                         <-- parent else, unbraced
    if (X > Y) Y = X;          <-- fold
  else                         <-- binds wrong way

Detection rule:
  - Find any line containing an unbraced "if(X cmp Y) <something>=..."  fold.
  - Look at preceding non-blank, non-comment, non-continuation line: if it
    ends with ")" with no leading "{" (i.e., the line is an `if(...)` or
    `else if(...)` or bare `else`), AND it does NOT introduce a brace,
    AND there is a following `else` on the next non-blank line --
    the inner if's body steals the else.

Approach: tokenize each .c/.cpp/.inc/.h file in mednafen/. For every line
matching the inner-fold pattern, peek up and down to detect the hazard.
"""
import re, sys, os

ROOT = "/home/claude/beetle-saturn-libretro"
SCAN_DIRS = ["mednafen", "."]
EXT = (".c", ".cpp", ".h", ".inc")
SKIP = ("/skills/", "/.git/", "/libretro-common/", "/tools/")

# Pattern for an inner `if(...) stmt;` body (any single statement),
# narrow enough that we don't false-positive on `if(...) {` openers.
# An opening brace at the end means the body is a block -- safe.
INNER_IF = re.compile(r"""
    ^\s*                                          # leading whitespace
    if\s*\(                                       # if (
    [^{};]*                                       # condition body
    \)\s*                                         # )
    (?!\s*$)                                      # must have a body on the same line
    [^{}]*?                                       # body content (no braces)
    ;                                             # ending semicolon
    \s*\\?\s*$                                    # optional line-continuation, EOL
""", re.VERBOSE)

# Original, narrow fold pattern -- kept for reference / validation.
FOLD = re.compile(r"""
    ^\s*                                          # line start (optionally indented)
    if\s*\(                                       # if (
    [^{};]*                                       # condition (no braces / semicolons)
    [<>!=]=?                                      # comparator -- must be present
    [^{};]*                                       # second operand
    \)\s*                                         # )
    [A-Za-z_][\w.\->\[\]]*                        # lvalue
    \s*[+\-*/]?=                                  # = / += / -= / etc.
    [^;{}]*;                                      # value to ;
    \s*\\?\s*$                                    # optional line-continuation, EOL
""", re.VERBOSE)

# Recognize the outer "if(...)" or "else" / "else if(...)" line shape
# WITHOUT an opening brace at the end. Allow trailing macro line-continuation `\`.
OUTER_IF   = re.compile(r"^\s*(?:\}\s*)?(?:else\s+)?if\s*\(.*\)\s*\\?\s*$")
OUTER_ELSE = re.compile(r"^\s*\}?\s*else\s*\\?\s*$")
HAS_OBRACE = re.compile(r"\{\s*\\?\s*$")
NEXT_ELSE  = re.compile(r"^\s*\}?\s*else\b")
COMMENT    = re.compile(r"^\s*(?://|/\*|\*)")

def prev_meaningful(lines, i):
    """Walk back skipping blank lines and pure-comment lines."""
    j = i - 1
    while j >= 0:
        s = lines[j].rstrip()
        if not s.strip():
            j -= 1; continue
        if COMMENT.match(s):
            j -= 1; continue
        return j, s
    return -1, ""

def next_meaningful(lines, i):
    j = i + 1
    n = len(lines)
    while j < n:
        s = lines[j].rstrip()
        if not s.strip():
            j += 1; continue
        if COMMENT.match(s):
            j += 1; continue
        return j, s
    return -1, ""

def scan_file(path, pattern=None):
    if pattern is None:
        pattern = INNER_IF
    with open(path, encoding="utf-8", errors="replace") as f:
        lines = f.readlines()
    hits = []
    for i, line in enumerate(lines):
        if not pattern.match(line):
            continue
        # Found a candidate inner if. Check parent.
        pj, prev = prev_meaningful(lines, i)
        if pj < 0:
            continue
        # If parent line already contains a brace, we're inside { ... } -- safe.
        if HAS_OBRACE.search(prev):
            continue
        is_parent_if   = bool(OUTER_IF.match(prev))
        is_parent_else = bool(OUTER_ELSE.match(prev))
        if not (is_parent_if or is_parent_else):
            continue
        # Check if a following else exists (the hazard).
        nj, nxt = next_meaningful(lines, i)
        if nj < 0:
            continue
        if not NEXT_ELSE.match(nxt):
            continue
        # Confirmed dangling-else hazard.
        hits.append((i + 1, line.rstrip("\n"), pj + 1, prev, nj + 1, nxt))
    return hits

def main():
    files = []
    for d in SCAN_DIRS:
        for base, _, fns in os.walk(os.path.join(ROOT, d)):
            if any(s in base + "/" for s in SKIP):
                continue
            for fn in fns:
                if fn.endswith(EXT):
                    files.append(os.path.join(base, fn))
    total = 0
    for path in sorted(files):
        hits = scan_file(path)
        if not hits:
            continue
        rel = os.path.relpath(path, ROOT)
        for ln, body, pl, prev, nl, nxt in hits:
            print(f"\n{rel}:{ln}")
            print(f"  prev line {pl}: {prev.rstrip()}")
            print(f"  fold line {ln}: {body.rstrip()}")
            print(f"  next line {nl}: {nxt.rstrip()}")
            total += 1
    print(f"\n{total} suspected dangling-else hazards across {len(files)} files")
    return 1 if total else 0

if __name__ == "__main__":
    sys.exit(main())
