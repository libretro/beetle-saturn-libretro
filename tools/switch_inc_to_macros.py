#!/usr/bin/env python3
"""
switch_inc_to_macros.py — rewrite m68k_instr*.inc to use the
macro-monomorphised concrete-typed functions instead of the C++
template syntax.

Per line:
   HAM<T, AM> var(this);          -> struct M68K_HAM_<t>_<am> var; M68K_HAM_<t>_<am>_init_self(&var, this);
   HAM<T, AM> var(this, arg);     -> struct M68K_HAM_<t>_<am> var; M68K_HAM_<t>_<am>_init_arg(&var, this, arg);
   OP(this[, args...]);           -> M68K_<OP>_<t1>_<am1>[_<t2>_<am2>...](this[, &ham_args..., extras...]);

Preserves leading whitespace.  Line per logical instruction (the .inc
already groups one case block per logical line in the bracketed area).
"""
import re, sys, os

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
INC_FILES = [
    'mednafen/hw_cpu/m68k/m68k_instr.inc',
    'mednafen/hw_cpu/m68k/m68k_instr_split0.inc',
    'mednafen/hw_cpu/m68k/m68k_instr_split1.inc',
]

TYPE_TO_NAME = {
    'uint8_t': 'u8',
    'uint16_t': 'u16',
    'uint32_t': 'u32',
    'std::tuple<>': 'void',
}

def short_t(typ):
    return TYPE_TO_NAME.get(typ.strip(), typ.strip())

def ham_struct_name(T, AM):
    return f'M68K_HAM_{short_t(T)}_{AM}'

# HAM declaration pattern: HAM<T, AM> var(this[, arg])
HAM_DECL_RE = re.compile(
    r'\bHAM<([^,>]+(?:<[^>]*>)?)\s*,\s*([A-Z_][A-Z0-9_]*)>\s+(\w+)\s*\(this(\s*,\s*([^)]+))?\)'
)

# Op call pattern: NAME(this[, args...])
OP_CALL_RE = re.compile(r'\b([A-Z][A-Za-z0-9_]+)\(this((?:\s*,\s*[^;]+?)?)\)')

# Ops we know about (built from gen_op_instances.py's combos plus all the helpers/internals)
# For an op invocation, we need to determine the function name based on HAM types.

def split_op_args(argblob):
    """Split top-level commas in `argblob` (which starts after 'this' and may
    or may not begin with ','). Returns list of arg expressions, possibly empty."""
    s = argblob.strip()
    if s.startswith(','):
        s = s[1:].strip()
    if not s:
        return []
    out = []
    depth = 0
    cur = ''
    for ch in s:
        if ch in '(<[':
            depth += 1; cur += ch
        elif ch in ')>]':
            depth -= 1; cur += ch
        elif ch == ',' and depth == 0:
            out.append(cur.strip()); cur = ''
        else:
            cur += ch
    if cur.strip():
        out.append(cur.strip())
    return out

# Skip these (they're already in concrete-typed form or aren't templates):
NON_TEMPLATE_OPS = {
    'ReadOp', 'CheckPrivilege', 'DBcc', 'Bxx', 'EXG',
    'MOVEP_l_reg_to_mem', 'MOVEP_w_mem_to_reg',
    'MOVEP_l_mem_to_reg', 'MOVEP_w_reg_to_mem',
    'MOVE_USP', 'UNLK', 'RESET', 'RTS', 'ORI_SR', 'EORI_CCR', 'RTR', 'STOP',
    'RTE', 'ORI_CCR', 'SWAP', 'ANDI_CCR', 'TRAP', 'TRAPV', 'LINEF', 'LINEA',
    'NOP', 'LINK', 'EORI_SR', 'ANDI_SR',
    'HAM',  # filtered separately
}


def transform_line(line):
    """Rewrite one .inc line. Returns the rewritten line."""
    # 1. Find all HAM declarations + collect their (T, AM, var, arg) info
    ham_decls = []
    for m in HAM_DECL_RE.finditer(line):
        T = m.group(1).strip()
        AM = m.group(2).strip()
        var = m.group(3)
        arg = m.group(5)  # may be None
        ham_decls.append({
            'span': m.span(),
            'T': T,
            'AM': AM,
            'var': var,
            'arg': arg,
        })

    if not ham_decls:
        # No HAM here; might still be an OP call with no HAM args (rare)
        return line

    # Build var -> (T, AM) map for resolving OP args
    var_to_ham = {h['var']: (h['T'], h['AM']) for h in ham_decls}

    # 2. Find OP calls + transform them
    # We'll do all the replacements left-to-right, accumulating the output
    # First, collect all replacement ranges (HAM decls + OP calls)
    edits = []  # list of (start, end, replacement)

    for h in ham_decls:
        struct = ham_struct_name(h['T'], h['AM'])
        if h['arg'] is None:
            # single-arg ctor: HAM<T,AM> var(this) -> struct ... var; ...init_self(&var, this);
            new = f'struct {struct} {h["var"]}; {struct}_init_self(&{h["var"]}, this)'
        else:
            new = f'struct {struct} {h["var"]}; {struct}_init_arg(&{h["var"]}, this, {h["arg"]})'
        edits.append((h['span'][0], h['span'][1], new))

    # OP calls
    for m in OP_CALL_RE.finditer(line):
        op = m.group(1)
        if op in NON_TEMPLATE_OPS:
            continue
        args = split_op_args(m.group(2))
        # Build new args + collect HAM types for naming
        new_args = []
        ham_types_for_name = []
        for a in args:
            if a in var_to_ham:
                T, AM = var_to_ham[a]
                new_args.append(f'&{a}')
                ham_types_for_name.append((T, AM))
            else:
                new_args.append(a)
        # Build new function name
        suffix_parts = []
        for T, AM in ham_types_for_name:
            suffix_parts.append(short_t(T))
            suffix_parts.append(AM)
        if not suffix_parts:
            continue  # no HAM args -> not a templated op (or unknown)
        new_name = f'M68K_{op}_' + '_'.join(suffix_parts)
        new_call = new_name + '(this' + (', ' + ', '.join(new_args) if new_args else '') + ')'
        edits.append((m.start(), m.end(), new_call))

    # 3. Apply edits right-to-left so offsets stay valid
    edits.sort(key=lambda e: -e[0])
    new_line = line
    for start, end, repl in edits:
        new_line = new_line[:start] + repl + new_line[end:]
    return new_line


total_lines = 0
changed_lines = 0
for inc_path in INC_FILES:
    full = os.path.join(ROOT, inc_path)
    with open(full) as f:
        old_text = f.read()
    new_lines = []
    for line in old_text.split('\n'):
        total_lines += 1
        new_line = transform_line(line)
        if new_line != line:
            changed_lines += 1
        new_lines.append(new_line)
    new_text = '\n'.join(new_lines)
    with open(full, 'w') as f:
        f.write(new_text)
    print(f"{inc_path}: lines changed", file=sys.stderr)

print(f"Total: {changed_lines}/{total_lines} lines changed", file=sys.stderr)
