#!/usr/bin/env python3
"""
tools/fold_source.py - constexpr-fold C++ source bodies for variant generation.

Operates on the text of a function body, given a substitution dict for
template parameters and types.  Performs in-place source-level dead-code
elimination so each generated variant contains ONLY the code that
actually executes for its tuple.  No constexpr-shadow declarations
needed; no `if(IsWrite)` left in the body when IsWrite is known.

Substitution dict format:
  {
    'T':        'uint16_t',    # type name; folds sizeof(T) too
    'IsWrite':  True,          # bool
    'SH32':     False,         # bool
    'WriteBus': 1,             # int
  }

Folds applied:
  - identifier substitution (T -> concrete type)
  - sizeof(T) -> N
  - if (true) X else Y       -> X
  - if (false) X else Y      -> Y    (Y can be empty -> deleted)
  - if (false) X             -> (deleted)
  - if (true) X              -> X (block contents)
  - cond ? A : B  with cond literal
  - switch(N) with N literal: keeps just the matched `case` body
    (no `break;`, no other arms, no default unless N is unmatched)
  - boolean: !true, !false, true && X, false && X, true || X, false || X
  - comparison: literal == literal, literal != literal
  - static_assert(true, ...);  removed
  - static_assert(false, ...); raised as fold-time error

Strategy:
  - tokenize into source/comment/string-literal segments (folds only
    in source, never inside comments or string literals)
  - iterate: apply all rules once; stop when text stabilizes
  - brace-matching done by simple depth counting on source segments
"""

import re
import sys


SIZEOF_MAP = {
    'uint8_t': 1, 'int8_t': 1,
    'uint16_t': 2, 'int16_t': 2,
    'uint32_t': 4, 'int32_t': 4,
    'uint64_t': 8, 'int64_t': 8,
}


# ----------------------------------------------------------------------
# Tokenization: split text into (kind, content) chunks where kind is
# 'code', 'block_comment', 'line_comment', 'string', 'char'.
# Folds operate only on 'code' chunks.
# ----------------------------------------------------------------------

def tokenize(text):
    chunks = []
    i = 0
    n = len(text)
    buf = []
    def flush():
        if buf:
            chunks.append(('code', ''.join(buf)))
            buf.clear()
    while i < n:
        c = text[i]
        # Block comment
        if c == '/' and i + 1 < n and text[i+1] == '*':
            flush()
            end = text.find('*/', i + 2)
            if end < 0:
                raise ValueError("unterminated block comment")
            chunks.append(('block_comment', text[i:end+2]))
            i = end + 2
            continue
        # Line comment
        if c == '/' and i + 1 < n and text[i+1] == '/':
            flush()
            end = text.find('\n', i + 2)
            if end < 0:
                end = n - 1
            chunks.append(('line_comment', text[i:end+1]))
            i = end + 1
            continue
        # String literal
        if c == '"':
            flush()
            j = i + 1
            while j < n and text[j] != '"':
                if text[j] == '\\' and j + 1 < n:
                    j += 2
                else:
                    j += 1
            chunks.append(('string', text[i:j+1]))
            i = j + 1
            continue
        # Char literal
        if c == "'":
            flush()
            j = i + 1
            while j < n and text[j] != "'":
                if text[j] == '\\' and j + 1 < n:
                    j += 2
                else:
                    j += 1
            chunks.append(('char', text[i:j+1]))
            i = j + 1
            continue
        # Preprocessor line: leave whole line as code (we won't fold across them)
        # Actually, treat as code too -- our patterns don't match preprocessor stuff.
        buf.append(c)
        i += 1
    flush()
    return chunks


def reassemble(chunks):
    return ''.join(c[1] for c in chunks)


def fold_only_code(text, fold_fn):
    """Apply fold_fn(str)->str to code chunks only."""
    chunks = tokenize(text)
    out = []
    for kind, content in chunks:
        if kind == 'code':
            out.append((kind, fold_fn(content)))
        else:
            out.append((kind, content))
    return reassemble(out)


# ----------------------------------------------------------------------
# Brace / paren matching helpers (operate on a substring of source).
# ----------------------------------------------------------------------

def find_matching(text, open_pos, open_ch='{', close_ch='}'):
    """Given text[open_pos] == open_ch, return idx of matching close."""
    assert text[open_pos] == open_ch
    depth = 1
    i = open_pos + 1
    n = len(text)
    while i < n:
        c = text[i]
        # Skip string/char/comment content (rough but enough for our well-
        # behaved sources; for robustness in pathological inputs, the caller
        # should split tokens first)
        if c == '/' and i + 1 < n and text[i+1] == '*':
            end = text.find('*/', i + 2)
            i = end + 2 if end >= 0 else n
            continue
        if c == '/' and i + 1 < n and text[i+1] == '/':
            end = text.find('\n', i + 2)
            i = end + 1 if end >= 0 else n
            continue
        if c == '"':
            j = i + 1
            while j < n and text[j] != '"':
                if text[j] == '\\' and j + 1 < n: j += 2
                else: j += 1
            i = j + 1
            continue
        if c == "'":
            j = i + 1
            while j < n and text[j] != "'":
                if text[j] == '\\' and j + 1 < n: j += 2
                else: j += 1
            i = j + 1
            continue
        if c == open_ch:
            depth += 1
        elif c == close_ch:
            depth -= 1
            if depth == 0:
                return i
        i += 1
    raise ValueError(f"unmatched {open_ch} at {open_pos}")


def skip_ws_and_comments(text, pos):
    n = len(text)
    while pos < n:
        c = text[pos]
        if c.isspace():
            pos += 1
            continue
        if c == '/' and pos + 1 < n and text[pos+1] == '*':
            end = text.find('*/', pos + 2)
            pos = end + 2 if end >= 0 else n
            continue
        if c == '/' and pos + 1 < n and text[pos+1] == '/':
            end = text.find('\n', pos + 2)
            pos = end + 1 if end >= 0 else n
            continue
        break
    return pos


def parse_controlled_stmt(text, pos):
    """Parse the statement at text[pos:] -- either a {...} block, a
    control-flow statement (if/while/for/switch/do-while), or a single
    statement terminated by ;.  Returns (end_pos_exclusive, full_text)."""
    pos = skip_ws_and_comments(text, pos)
    n = len(text)
    if pos >= n:
        raise ValueError("nothing to parse at end of text")

    # Block statement
    if text[pos] == '{':
        end = find_matching(text, pos, '{', '}')
        return end + 1, text[pos:end+1]

    # `if (...) STMT [else STMT]` -- recursive (else-stmt may itself be an if)
    m = re.match(r'if\s*\(', text[pos:])
    if m:
        paren_open = pos + m.end() - 1
        paren_close = find_matching(text, paren_open, '(', ')')
        then_end, _ = parse_controlled_stmt(text, paren_close + 1)
        # Optional else
        next_pos = skip_ws_and_comments(text, then_end)
        if (next_pos + 4 <= n
                and text[next_pos:next_pos+4] == 'else'
                and (next_pos + 4 == n
                     or not (text[next_pos+4].isalnum() or text[next_pos+4] == '_'))):
            else_end, _ = parse_controlled_stmt(text, next_pos + 4)
            return else_end, text[pos:else_end]
        return then_end, text[pos:then_end]

    # `do STMT while (...) ;`
    m = re.match(r'do\b', text[pos:])
    if m:
        body_end, _ = parse_controlled_stmt(text, pos + m.end())
        wpos = skip_ws_and_comments(text, body_end)
        wm = re.match(r'while\s*\(', text[wpos:])
        if not wm:
            raise ValueError(f"`do` without matching `while` at {wpos}")
        paren_open = wpos + wm.end() - 1
        paren_close = find_matching(text, paren_open, '(', ')')
        i = paren_close + 1
        while i < n and text[i] != ';':
            if not text[i].isspace():
                raise ValueError(f"do-while: expected `;` after `(...)` near {i}")
            i += 1
        return i + 1, text[pos:i+1]

    # `while|for|switch (...) STMT`
    m = re.match(r'(while|for|switch)\s*\(', text[pos:])
    if m:
        kw = m.group(1)
        paren_open = pos + m.end() - 1
        paren_close = find_matching(text, paren_open, '(', ')')
        if kw == 'switch':
            block_open = skip_ws_and_comments(text, paren_close + 1)
            if block_open >= n or text[block_open] != '{':
                raise ValueError(f"switch without `{{` at {block_open}")
            block_close = find_matching(text, block_open, '{', '}')
            return block_close + 1, text[pos:block_close+1]
        body_end, _ = parse_controlled_stmt(text, paren_close + 1)
        return body_end, text[pos:body_end]

    # Single statement: scan to `;` at top depth
    depth_paren = 0
    depth_brace = 0
    i = pos
    while i < n:
        c = text[i]
        if c == '/' and i + 1 < n and text[i+1] == '*':
            end = text.find('*/', i + 2); i = end + 2 if end >= 0 else n; continue
        if c == '/' and i + 1 < n and text[i+1] == '/':
            end = text.find('\n', i + 2); i = end + 1 if end >= 0 else n; continue
        if c == '"':
            j = i + 1
            while j < n and text[j] != '"':
                if text[j] == '\\' and j + 1 < n: j += 2
                else: j += 1
            i = j + 1; continue
        if c == "'":
            j = i + 1
            while j < n and text[j] != "'":
                if text[j] == '\\' and j + 1 < n: j += 2
                else: j += 1
            i = j + 1; continue
        if c == '(': depth_paren += 1
        elif c == ')': depth_paren -= 1
        elif c == '{': depth_brace += 1
        elif c == '}': depth_brace -= 1
        elif c == ';' and depth_paren == 0 and depth_brace == 0:
            return i + 1, text[pos:i+1]
        i += 1
    raise ValueError(f"unterminated statement at {pos}")


def unwrap_block(stmt):
    """If stmt is `{...}`, return its inner contents (with surrounding
    whitespace).  Otherwise return stmt unchanged."""
    s = stmt.strip()
    if s.startswith('{') and s.endswith('}'):
        return s[1:-1]
    return stmt


# ----------------------------------------------------------------------
# Fold passes (applied repeatedly until stable)
# ----------------------------------------------------------------------

def fold_unary_not(text):
    text = re.sub(r'!\s*true\b',  'false', text)
    text = re.sub(r'!\s*false\b', 'true',  text)
    text = re.sub(r'!\s*\(\s*true\s*\)',  'false', text)
    text = re.sub(r'!\s*\(\s*false\s*\)', 'true',  text)
    return text


def fold_int_compare(text):
    # literal == literal, literal != literal (both unsigned)
    def repl_eq(m):
        a, b = int(m.group(1)), int(m.group(2))
        return 'true' if a == b else 'false'
    def repl_ne(m):
        a, b = int(m.group(1)), int(m.group(2))
        return 'false' if a == b else 'true'
    text = re.sub(r'\b(\d+)\s*==\s*(\d+)\b', repl_eq, text)
    text = re.sub(r'\b(\d+)\s*!=\s*(\d+)\b', repl_ne, text)
    return text


def fold_logical(text):
    # true && X, false && X (only safe if X has no side effects -- we
    # require it to be a single identifier or parenthesized group for
    # safety).  Same for ||.  Apply only the cases that are obviously safe:
    # `true && (` -> `(`     (then on stable iteration, the trailing
    #                         expression gets re-evaluated)
    # `false &&` swallows up to the next `||` or paren close at same depth
    # That's hard.  Use simpler: only fold the `LIT && LIT` and `LIT || LIT`
    # cases (i.e., after other folds have produced literal&&literal).
    def repl_and(m):
        a, b = m.group(1), m.group(2)
        return 'true' if (a == 'true' and b == 'true') else 'false'
    def repl_or(m):
        a, b = m.group(1), m.group(2)
        return 'true' if (a == 'true' or b == 'true') else 'false'
    text = re.sub(r'\b(true|false)\s*&&\s*(true|false)\b', repl_and, text)
    text = re.sub(r'\b(true|false)\s*\|\|\s*(true|false)\b', repl_or, text)
    return text


def fold_if(text):
    """Find `if (true|false) STMT [else STMT]` patterns and rewrite."""
    out = []
    i = 0
    n = len(text)
    while i < n:
        # Look for `if (`
        m = re.compile(r'\bif\s*\(').search(text, i)
        if not m:
            out.append(text[i:])
            break
        start = m.start()
        out.append(text[i:start])
        paren_open = m.end() - 1
        paren_close = find_matching(text, paren_open, '(', ')')
        cond = text[paren_open+1:paren_close].strip()
        if cond not in ('true', 'false'):
            # Not a fold target; keep `if (...)` and move past it
            out.append(text[start:paren_close+1])
            i = paren_close + 1
            continue
        # Parse then-statement
        then_end, then_stmt = parse_controlled_stmt(text, paren_close + 1)
        # Look for optional `else`
        next_pos = skip_ws_and_comments(text, then_end)
        else_stmt = None
        else_end = then_end
        if next_pos < n and text[next_pos:next_pos+4] == 'else' and (
                next_pos + 4 >= n or not (text[next_pos+4].isalnum() or text[next_pos+4] == '_')):
            else_kw_end = next_pos + 4
            else_end, else_stmt = parse_controlled_stmt(text, else_kw_end)
        # Decide replacement
        if cond == 'true':
            chosen = then_stmt
        else:  # false
            chosen = else_stmt if else_stmt is not None else ''
        # Don't unwrap blocks: keeping the `{...}` around the chosen body
        # preserves scope.  Stripping the braces caused a scope-leakage bug
        # in cases like switch-case bodies with variable declarations --
        # `case X: if(false) {...} else { uint16_t tmp = ...; }; break;`
        # would unwrap to `case X: uint16_t tmp = ...; break;` and the next
        # `case` label would cross `tmp`'s initialization (compile error
        # "jump to case label").  Variable scope leakage can also cause
        # silent shadowing bugs.  fold_if's iteration still finds nested
        # `if(true|false)` inside a kept `{...}` block, so unwrapping is
        # not needed for fold correctness.
        out.append(chosen)
        i = else_end
    return ''.join(out)


def fold_ternary(text):
    """Fold `(true|false) ? A : B` -- handle nested ternaries by
    iterating in fold_passes()."""
    # Find `LIT ?` occurrences and parse A, B carefully.
    out = []
    i = 0
    n = len(text)
    while i < n:
        m = re.compile(r'\b(true|false)\s*\?').search(text, i)
        if not m:
            out.append(text[i:])
            break
        lit = m.group(1)
        cond_start = m.start()
        q_pos = m.end() - 1  # the '?'
        out.append(text[i:cond_start])
        # Parse A: from q_pos+1 to a ':' at same paren/brace/ternary-? depth
        a_start = q_pos + 1
        depth_paren = 0; depth_brace = 0; ternary_depth = 0
        j = a_start
        while j < n:
            c = text[j]
            if c == '/' and j + 1 < n and text[j+1] == '*':
                e = text.find('*/', j+2); j = e+2 if e >= 0 else n; continue
            if c == '/' and j + 1 < n and text[j+1] == '/':
                e = text.find('\n', j+2); j = e+1 if e >= 0 else n; continue
            if c == '"':
                k = j + 1
                while k < n and text[k] != '"':
                    if text[k] == '\\' and k+1 < n: k += 2
                    else: k += 1
                j = k + 1; continue
            if c == "'":
                k = j + 1
                while k < n and text[k] != "'":
                    if text[k] == '\\' and k+1 < n: k += 2
                    else: k += 1
                j = k + 1; continue
            if c == '(': depth_paren += 1
            elif c == ')':
                if depth_paren == 0:
                    # Hit enclosing close; ternary missing colon? bail
                    raise ValueError(f"ternary at {cond_start}: unexpected ) at {j}")
                depth_paren -= 1
            elif c == '{': depth_brace += 1
            elif c == '}': depth_brace -= 1
            elif c == '?' and depth_paren == 0 and depth_brace == 0:
                ternary_depth += 1
            elif c == ':' and depth_paren == 0 and depth_brace == 0:
                if ternary_depth > 0:
                    ternary_depth -= 1
                else:
                    break
            j += 1
        else:
            raise ValueError(f"ternary at {cond_start}: no matching `:`")
        a_end = j  # at ':'
        a_text = text[a_start:a_end]
        # Parse B: from a_end+1 to the next `;` or `,` or `)` at same depth
        # Stop at the first character that ends an expression at this depth.
        b_start = a_end + 1
        depth_paren = 0; depth_brace = 0; ternary_depth = 0
        j = b_start
        while j < n:
            c = text[j]
            if c == '/' and j + 1 < n and text[j+1] == '*':
                e = text.find('*/', j+2); j = e+2 if e >= 0 else n; continue
            if c == '/' and j + 1 < n and text[j+1] == '/':
                e = text.find('\n', j+2); j = e+1 if e >= 0 else n; continue
            if c == '"':
                k = j + 1
                while k < n and text[k] != '"':
                    if text[k] == '\\' and k+1 < n: k += 2
                    else: k += 1
                j = k + 1; continue
            if c == "'":
                k = j + 1
                while k < n and text[k] != "'":
                    if text[k] == '\\' and k+1 < n: k += 2
                    else: k += 1
                j = k + 1; continue
            if c == '(': depth_paren += 1
            elif c == ')':
                if depth_paren == 0:
                    break
                depth_paren -= 1
            elif c == '{': depth_brace += 1
            elif c == '}':
                if depth_brace == 0:
                    break
                depth_brace -= 1
            elif c == '?' and depth_paren == 0 and depth_brace == 0:
                ternary_depth += 1
            elif c == ':' and depth_paren == 0 and depth_brace == 0:
                if ternary_depth > 0:
                    ternary_depth -= 1
                else:
                    break
            elif (c == ';' or c == ',') and depth_paren == 0 and depth_brace == 0:
                break
            j += 1
        b_end = j
        b_text = text[b_start:b_end]
        chosen = a_text if lit == 'true' else b_text
        out.append(chosen)
        i = b_end
    return ''.join(out)


def fold_switch(text):
    """Find `switch(LITERAL)` patterns and rewrite to just the matched
    case body."""
    out = []
    i = 0
    n = len(text)
    while i < n:
        m = re.compile(r'\bswitch\s*\(').search(text, i)
        if not m:
            out.append(text[i:])
            break
        start = m.start()
        out.append(text[i:start])
        paren_open = m.end() - 1
        paren_close = find_matching(text, paren_open, '(', ')')
        operand = text[paren_open+1:paren_close].strip()
        # Only fold if operand is a numeric literal
        if not re.match(r'^\d+$', operand):
            # Not foldable, emit as-is up to and including the switch block
            block_open = skip_ws_and_comments(text, paren_close + 1)
            block_close = find_matching(text, block_open, '{', '}')
            out.append(text[start:block_close+1])
            i = block_close + 1
            continue
        operand_val = int(operand)
        # Parse switch body
        block_open = skip_ws_and_comments(text, paren_close + 1)
        if text[block_open] != '{':
            raise ValueError(f"switch at {start}: expected `{{` after `(...)`")
        block_close = find_matching(text, block_open, '{', '}')
        body = text[block_open+1:block_close]
        # Walk through `case N:` and `default:` labels at depth-0
        cases = []  # list of (label_value_or_None, start_in_body, end_in_body)
        depth_paren = 0; depth_brace = 0
        cur_label = None; cur_start = 0
        j = 0
        m_body_n = len(body)
        while j < m_body_n:
            c = body[j]
            # skip strings/comments
            if c == '/' and j+1 < m_body_n and body[j+1] == '*':
                e = body.find('*/', j+2); j = e+2 if e >= 0 else m_body_n; continue
            if c == '/' and j+1 < m_body_n and body[j+1] == '/':
                e = body.find('\n', j+2); j = e+1 if e >= 0 else m_body_n; continue
            if c == '"':
                k = j + 1
                while k < m_body_n and body[k] != '"':
                    if body[k] == '\\' and k+1 < m_body_n: k += 2
                    else: k += 1
                j = k + 1; continue
            if c == "'":
                k = j + 1
                while k < m_body_n and body[k] != "'":
                    if body[k] == '\\' and k+1 < m_body_n: k += 2
                    else: k += 1
                j = k + 1; continue
            if c == '(': depth_paren += 1; j += 1; continue
            if c == ')': depth_paren -= 1; j += 1; continue
            if c == '{': depth_brace += 1; j += 1; continue
            if c == '}': depth_brace -= 1; j += 1; continue
            if depth_paren == 0 and depth_brace == 0:
                # Check `case N:` or `default:`
                mc = re.match(r'case\s+(\w+)\s*:', body[j:])
                md = re.match(r'default\s*:', body[j:])
                if mc:
                    # New case label
                    if j > cur_start or cur_label is not None:
                        cases.append((cur_label, cur_start, j))
                    val = mc.group(1)
                    try: val = int(val)
                    except ValueError: pass  # symbolic, keep as-is
                    cur_label = val
                    cur_start = j + mc.end()
                    j += mc.end()
                    continue
                if md:
                    if j > cur_start or cur_label is not None:
                        cases.append((cur_label, cur_start, j))
                    cur_label = 'default'
                    cur_start = j + md.end()
                    j += md.end()
                    continue
            j += 1
        # Last case
        cases.append((cur_label, cur_start, m_body_n))
        # Find matched case
        matched_body = None
        # First case that has the literal as label
        for k, (lbl, s, e) in enumerate(cases):
            if lbl == operand_val:
                matched_body = body[s:e]
                break
        if matched_body is None:
            for lbl, s, e in cases:
                if lbl == 'default':
                    matched_body = body[s:e]
                    break
        if matched_body is None:
            matched_body = ''
        # Strip trailing `break;` (cleanup)
        matched_body = re.sub(r'\bbreak\s*;\s*$', '', matched_body.rstrip())
        out.append(matched_body)
        i = block_close + 1
    return ''.join(out)


def fold_static_assert(text):
    # Remove `static_assert(true, ...);`
    text = re.sub(r'static_assert\s*\(\s*true\s*,[^)]*\)\s*;', '', text)
    return text


def fold_passes(text):
    """Run all fold passes until text stabilizes."""
    prev = None
    iters = 0
    while text != prev:
        prev = text
        # Simple regex folds apply per-code-chunk (skip comments/strings).
        text = fold_only_code(text, fold_unary_not)
        text = fold_only_code(text, fold_int_compare)
        text = fold_only_code(text, fold_logical)
        # Structural folds need the whole text: their internal parsers
        # already skip comments/strings via find_matching / parse_controlled_stmt.
        # Operating on the full text lets brace-matching work across
        # comments interleaved with control structures.
        text = fold_static_assert(text)
        text = fold_ternary(text)
        text = fold_if(text)
        text = fold_switch(text)
        iters += 1
        if iters > 50:
            raise RuntimeError("fold_passes did not converge after 50 iterations")
    return text


# ----------------------------------------------------------------------
# Public API
# ----------------------------------------------------------------------

def substitute(text, subs):
    """Apply identifier substitution + sizeof(T) folding."""
    # sizeof(T) first
    if 'T' in subs:
        type_name = subs['T']
        size = SIZEOF_MAP[type_name]
        text = fold_only_code(text, lambda s: re.sub(r'\bsizeof\(\s*T\s*\)', str(size), s))
    # Then identifier subs (use word-boundary)
    for name, val in subs.items():
        if name == 'T':
            # T as type name -> concrete (only as standalone identifier)
            text = fold_only_code(text, lambda s, n=name, v=val: re.sub(rf'\b{re.escape(n)}\b', v, s))
            continue
        if isinstance(val, bool):
            lit = 'true' if val else 'false'
        else:
            lit = str(val)
        text = fold_only_code(text, lambda s, n=name, v=lit: re.sub(rf'\b{re.escape(n)}\b', v, s))
    return text


def fold(body_text, subs):
    """Top-level: substitute + fold + return cleaned body."""
    text = substitute(body_text, subs)
    text = fold_passes(text)
    # Final cleanup: collapse runs of >2 blank lines
    text = re.sub(r'\n\s*\n\s*\n+', '\n\n', text)
    return text


# ----------------------------------------------------------------------
# Self-test (run with: python3 tools/fold_source.py --test)
# ----------------------------------------------------------------------

def _test():
    # Test 1: simple if-elimination
    body = """
 if(IsWrite)
 {
  do_write();
 }
 else
 {
  do_read();
 }
"""
    out_w = fold(body, {'IsWrite': True})
    assert 'do_write()' in out_w and 'do_read()' not in out_w, repr(out_w)
    out_r = fold(body, {'IsWrite': False})
    assert 'do_read()' in out_r and 'do_write()' not in out_r, repr(out_r)
    print("test1 if-elim: OK")
    print("--- IsWrite=True ---")
    print(out_w)
    print("--- IsWrite=False ---")
    print(out_r)
    print("---")

    # Test 2: sizeof(T) fold + switch
    body = """
 switch(sizeof(T))
 {
  case 1: mask = 0xFF; break;
  case 2: mask = 0xFFFF; break;
  case 4: mask = 0xFFFFFFFF; break;
 }
"""
    for tname, expected in (('uint8_t', '0xFF;'), ('uint16_t', '0xFFFF;'), ('uint32_t', '0xFFFFFFFF;')):
        out = fold(body, {'T': tname})
        assert expected in out, f"T={tname}: expected {expected} in:\n{out}"
        assert 'switch' not in out, f"T={tname}: switch should be folded:\n{out}"
    print("test2 switch-fold: OK")

    # Test 3: ternary
    body = ' int x = IsWrite ? (SH32 ? 0 : 6) : 10;\n'
    out = fold(body, {'IsWrite': True, 'SH32': False})
    # Inner ternary's surrounding parens stay -- that's fine, equivalent code
    assert re.search(r'=\s*\(?\s*6\s*\)?\s*;', out), repr(out)
    out = fold(body, {'IsWrite': False, 'SH32': True})
    assert re.search(r'=\s*10\s*;', out), repr(out)
    print("test3 ternary: OK")

    # Test 4: static_assert removed when true
    body = ' static_assert(IsWrite || sizeof(T) == 2, "Wrong type.");\n'
    out = fold(body, {'IsWrite': True, 'T': 'uint8_t'})
    assert 'static_assert' not in out, repr(out)
    out = fold(body, {'IsWrite': False, 'T': 'uint16_t'})
    assert 'static_assert' not in out, repr(out)
    print("test4 static_assert: OK")

    # Test 5: nested if with composite
    body = """
 if(!SH32)
 {
  *p = 1;
  if(IsWrite)
  {
   *p += 2;
  }
 }
"""
    out = fold(body, {'SH32': False, 'IsWrite': True})
    assert '*p = 1' in out and '*p += 2' in out and 'if' not in out, repr(out)
    out = fold(body, {'SH32': True, 'IsWrite': True})
    assert '*p = 1' not in out and 'if' not in out, repr(out)
    out = fold(body, {'SH32': False, 'IsWrite': False})
    assert '*p = 1' in out and '*p += 2' not in out, repr(out)
    print("test5 nested if: OK")

    print("ALL TESTS PASS")


if __name__ == '__main__':
    if len(sys.argv) > 1 and sys.argv[1] == '--test':
        _test()
    else:
        print(__doc__)
        sys.exit(1)
