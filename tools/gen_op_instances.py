#!/usr/bin/env python3
"""
gen_op_instances.py — generate m68k_op_instances.inc.h with all
op instantiations needed by the m68k_instr*.inc dispatch files.

For each (op, ham_arg_signatures) combo used in .inc:
  emit #define OP_NAME ... + macros + #include "m68k_op_<NAME>_body.inc.h"

For helper templates Subtract/ShiftBase/RotateBase, also emit
instantiations needed by the calling op bodies (transitively).
"""
import re, sys, os
from collections import defaultdict, OrderedDict

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
OUT_DIR = os.path.join(ROOT, 'mednafen/hw_cpu/m68k')

# Type mapping
TYPE_TO_NAME = {
    'uint8_t': 'u8',
    'uint16_t': 'u16',
    'uint32_t': 'u32',
    'std::tuple<>': 'void',  # for LEA/PEA/JMP/JSR
    'void': 'void',
}
TYPE_TO_SIZE = {
    'uint8_t': 1,
    'uint16_t': 2,
    'uint32_t': 4,
    'std::tuple<>': 0,
    'void': 0,
}
TYPE_TO_SIGNED = {
    'uint8_t': 'int8_t',
    'uint16_t': 'int16_t',
    'uint32_t': 'int32_t',
}

def short_t(typ):
    return TYPE_TO_NAME.get(typ, typ)

def ham_struct_name(T, AM):
    return f'M68K_HAM_{short_t(T)}_{AM}'


# Parse .inc files to find every (op, HAM-args) combination used
incs = ['mednafen/hw_cpu/m68k/m68k_instr.inc',
        'mednafen/hw_cpu/m68k/m68k_instr_split0.inc',
        'mednafen/hw_cpu/m68k/m68k_instr_split1.inc']
text = ''
for f in incs:
    text += open(os.path.join(ROOT, f)).read() + '\n'

ham_pattern = r'HAM<([^,>]+(?:<[^>]*>)?),\s*([A-Z_][A-Z0-9_]*)>\s+(\w+)\s*\(this(?:,\s*[^)]+)?\)'
combos = defaultdict(set)  # op_name -> set of tuples
for line in text.split('\n'):
    hams = {}
    for m in re.finditer(ham_pattern, line):
        hams[m.group(3)] = (m.group(1).strip(), m.group(2).strip())
    if not hams:
        continue
    for opm in re.finditer(r'\b([A-Z][A-Za-z0-9_]+)\(this((?:,\s*[^;]+?)*)\);', line):
        op = opm.group(1)
        if op == 'HAM' or op == 'ReadOp':
            continue
        argblob = opm.group(2).lstrip(',').strip()
        if not argblob:
            combos[op].add(())
            continue
        # Split top-level commas
        args = []
        depth = 0; cur = ''
        for ch in argblob:
            if ch in '(<[': depth += 1; cur += ch
            elif ch in ')>]': depth -= 1; cur += ch
            elif ch == ',' and depth == 0:
                args.append(cur.strip()); cur = ''
            else:
                cur += ch
        if cur.strip():
            args.append(cur.strip())
        sig = []
        for a in args:
            if a in hams:
                T, AM = hams[a]
                sig.append(('HAM', T, AM))
            else:
                sig.append(('EXTRA', a))
        combos[op].add(tuple(sig))


# Now we need to compute helper-template instantiations too.
# ADD/ADDX/SUB/SUBX/CMP each call Subtract with the SAME T/SAM/DT/DAM.
# CHK calls Subtract internally? Let me check the original bodies for helper calls.

# Read body files to detect helper calls
HELPER_OPS = {'Subtract', 'ShiftBase', 'RotateBase'}

# For each (op, sig) that uses helpers, compute helper instantiation.
# We need to look at the body of each op to know how the helper is called.
# Hardcode the known helper-call patterns:
#
#   ADD<T,DT,SAM,DAM> -> Subtract NOT called  (it's pure add)
#   SUB<T,DT,SAM,DAM> -> Subtract<T,DT,SAM,DAM>
#   SUBX<T,DT,SAM,DAM> -> Subtract<T,DT,SAM,DAM>
#   CMP<T,DT,SAM,DAM> -> Subtract NOT (CMP has own body)
#   NEG<DT,DAM> -> Subtract<DT,DT,DAM,IMMEDIATE>  (dummy_zero is HAM<DT,IMMEDIATE>)
#   NEGX<DT,DAM> -> Subtract<DT,DT,DAM,IMMEDIATE>
#   ASL/ASR/LSL/LSR<T,TAM> -> ShiftBase<T,TAM>
#   ROL/ROR/ROXL/ROXR<T,TAM> -> RotateBase<T,TAM>

def compute_helper_sigs_for(op, sig):
    """Given op name + arg signature, return list of (helper_op, helper_sig)."""
    helpers = []
    ham_sigs = [s for s in sig if s[0] == 'HAM']

    if op in ('SUB', 'SUBX'):
        # SUB<T,DT,SAM,DAM>(src, dst) -> Subtract(z, X, src, dst)
        # Subtract takes <T, DT, SAM, DAM>
        if len(ham_sigs) == 2:
            (_, T_s, AM_s), (_, T_d, AM_d) = ham_sigs
            helpers.append(('Subtract', (
                ('HAM', T_s, AM_s),  # src arg
                ('HAM', T_d, AM_d),  # dst arg
            )))
    elif op in ('NEG', 'NEGX'):
        # NEG<DT,DAM>(dst) -> Subtract(z, X, dst, dummy_zero)
        # where dummy_zero is HAM<DT, IMMEDIATE>
        # So Subtract's <T,DT,SAM,DAM> = <DT, DT, DAM, IMMEDIATE>
        if len(ham_sigs) == 1:
            (_, T_d, AM_d) = ham_sigs[0]
            helpers.append(('Subtract', (
                ('HAM', T_d, AM_d),       # src = dst
                ('HAM', T_d, 'IMMEDIATE'),# dst = dummy_zero
            )))
    elif op in ('ASL', 'ASR', 'LSL', 'LSR'):
        # ASL<T,TAM>(targ, count) -> ShiftBase(z, A, B, targ, count)
        # ShiftBase takes <T, TAM>(z, bool, bool, &targ, count)
        if len(ham_sigs) >= 1:
            (_, T_t, AM_t) = ham_sigs[0]
            helpers.append(('ShiftBase', (
                ('HAM', T_t, AM_t),
            )))
    elif op in ('ROL', 'ROR', 'ROXL', 'ROXR'):
        # ROL<T,TAM>(targ, count) -> RotateBase(z, A, B, targ, count)
        if len(ham_sigs) >= 1:
            (_, T_t, AM_t) = ham_sigs[0]
            helpers.append(('RotateBase', (
                ('HAM', T_t, AM_t),
            )))

    return helpers

# Walk combos and compute helper instantiations
helper_combos = defaultdict(set)
for op, sigs in list(combos.items()):
    for sig in sigs:
        for h_op, h_sig in compute_helper_sigs_for(op, sig):
            helper_combos[h_op].add(h_sig)

# Merge helper combos into combos
for h_op, h_sigs in helper_combos.items():
    for h_sig in h_sigs:
        combos[h_op].add(h_sig)

# ---------- naming convention ----------

def fn_name_for(op, sig):
    """Build the concrete function name for a (op, sig) instantiation."""
    parts = [f'M68K_{op}']
    for s in sig:
        if s[0] == 'HAM':
            parts.append(short_t(s[1]))
            parts.append(s[2])
    return '_'.join(parts)


# ---------- emit instances file ----------

def emit_instance(op, sig, op_info_cache):
    """Emit lines for one instantiation block."""
    lines = []
    # Determine which macros need defining based on op's template params
    # We need to load op's template params from body file header analysis
    # Simpler: we have ham_args list; OP_HAM_<VAR> for each
    # And helper templates: OP_SUBTRACT/etc.

    # Build the #define lines.
    # For each HAM arg in sig, we need OP_HAM_<VAR> = M68K_HAM_<t>_<am>
    # Var names come from the body file (src/dst/targ etc.)
    op_info = op_info_cache.get(op)
    if not op_info:
        return None

    name = fn_name_for(op, sig)
    lines.append(f'#define OP_NAME {name}')

    # Get template-param values from sig
    tpl_values = {}  # tpl_param_name -> value
    ham_args_in_order = [a for a in op_info['ham_args']]
    # Map ham args to sig HAM entries in order
    sig_hams = [s for s in sig if s[0] == 'HAM']
    if len(sig_hams) != len(ham_args_in_order):
        print(f"!! {op}: sig has {len(sig_hams)} HAMs, body expects {len(ham_args_in_order)}", file=sys.stderr)
        return None

    for (T_tpl, AM_tpl, var), (_, T_val, AM_val) in zip(ham_args_in_order, sig_hams):
        tpl_values[T_tpl] = T_val
        tpl_values[AM_tpl] = AM_val

    # Emit #define for each template param
    for kind, pname in op_info['tpl_params']:
        if pname not in tpl_values:
            continue
        val = tpl_values[pname]
        lines.append(f'#define OP_{pname} {val}')

    # Compute OP_TSIZE/OP_DTSIZE based on T/DT
    if 'T' in tpl_values:
        lines.append(f'#define OP_TSIZE {TYPE_TO_SIZE.get(tpl_values["T"], "0")}')
        if tpl_values['T'] in TYPE_TO_SIGNED:
            lines.append(f'#define OP_T_SIGNED {TYPE_TO_SIGNED[tpl_values["T"]]}')
    if 'DT' in tpl_values:
        lines.append(f'#define OP_DTSIZE {TYPE_TO_SIZE.get(tpl_values["DT"], "0")}')
        if tpl_values['DT'] in TYPE_TO_SIGNED:
            lines.append(f'#define OP_DT_SIGNED {TYPE_TO_SIGNED[tpl_values["DT"]]}')

    # OP_HAM_<VAR> for each HAM arg
    for (T_tpl, AM_tpl, var), (_, T_val, AM_val) in zip(ham_args_in_order, sig_hams):
        lines.append(f'#define OP_HAM_{var.upper()} {ham_struct_name(T_val, AM_val)}')

    # OP_HAM_DUMMY_IMM if op uses dummy_zero
    body_text = '\n'.join(op_info['body_lines'])
    if 'dummy_zero' in body_text:
        # dummy_zero is HAM<DT, IMMEDIATE> in NEG/NEGX
        dt_val = tpl_values.get('DT') or tpl_values.get('T')
        lines.append(f'#define OP_HAM_DUMMY_IMM {ham_struct_name(dt_val, "IMMEDIATE")}')

    # Helper macros
    for h_op, h_sig in compute_helper_sigs_for(op, sig):
        h_name = fn_name_for(h_op, h_sig)
        macro_name = f'OP_{h_op.upper()}'
        lines.append(f'#define {macro_name} {h_name}')

    lines.append(f'#include "m68k_op_{op}_body.inc.h"')
    return '\n'.join(lines) + '\n'


# Load op info from m68k_private.h (re-parse, sharing the logic from gen_op_macros.py)
exec(open(os.path.join(ROOT, 'tools/gen_op_macros.py')).read().split("for name, info in ops.items():")[0])
# Now `ops` dict is populated

# Build instances output
out = ['/* m68k_op_instances.inc.h\n'
       ' * Phase-9d-15b: instantiation list for the op macro bodies.\n'
       ' * Auto-generated by tools/gen_op_instances.py from the (op,\n'
       ' * HAM-arg) combinations actually used in m68k_instr*.inc.\n'
       ' *\n'
       ' * Order matters: helper templates (Subtract/ShiftBase/RotateBase)\n'
       ' * are emitted first so the calling ops can reference them via\n'
       ' * the OP_SUBTRACT/OP_SHIFTBASE/OP_ROTATEBASE macros at\n'
       ' * instantiation time. */\n\n']

# Helpers first
HELPER_ORDER = ['Subtract', 'ShiftBase', 'RotateBase']
seen_names = set()  # dedupe by generated function name
def emit_unique(op, sig):
    name = fn_name_for(op, sig)
    if name in seen_names:
        return None
    seen_names.add(name)
    return emit_instance(op, sig, ops)

for h_op in HELPER_ORDER:
    if h_op not in combos:
        continue
    out.append(f'\n/* ===== {h_op} ===== */\n\n')
    for sig in sorted(combos[h_op], key=lambda s: tuple(repr(x) for x in s)):
        block = emit_unique(h_op, sig)
        if block:
            out.append(block + '\n')

# Then everything else
for op in sorted(combos.keys()):
    if op in HELPER_ORDER:
        continue
    out.append(f'\n/* ===== {op} ===== */\n\n')
    for sig in sorted(combos[op], key=lambda s: tuple(repr(x) for x in s)):
        block = emit_unique(op, sig)
        if block:
            out.append(block + '\n')

with open(os.path.join(OUT_DIR, 'm68k_op_instances.inc.h'), 'w') as f:
    f.write(''.join(out))

total = sum(len(v) for v in combos.values())
print(f"Wrote m68k_op_instances.inc.h with {total} instantiations", file=sys.stderr)
