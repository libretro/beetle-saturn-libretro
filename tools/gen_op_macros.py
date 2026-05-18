#!/usr/bin/env python3
"""
gen_op_macros.py — generate macro-parameterised op body files
from the C++ template bodies in m68k_private.h.

Uses balanced-paren parsing for HAM method calls and helper-template
calls so nested calls (e.g. dst.write(Subtract(...))) work.
"""
import re, sys, os

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
PRIV = os.path.join(ROOT, 'mednafen/hw_cpu/m68k/m68k_private.h')
OUT_DIR = os.path.join(ROOT, 'mednafen/hw_cpu/m68k')


def find_matching_paren(s, start):
    assert s[start] == '('
    depth = 0
    for i in range(start, len(s)):
        if s[i] == '(': depth += 1
        elif s[i] == ')':
            depth -= 1
            if depth == 0:
                return i
    return -1


def split_args(arg_str):
    args = []
    depth = 0
    cur = ''
    for ch in arg_str:
        if ch in '(<[': depth += 1; cur += ch
        elif ch in ')>]': depth -= 1; cur += ch
        elif ch == ',' and depth == 0:
            args.append(cur.strip()); cur = ''
        else:
            cur += ch
    if cur.strip():
        args.append(cur.strip())
    return args


# Parse the file
with open(PRIV) as f:
    src = f.read()
lines = src.split('\n')

ops = {}
i = 0
while i < len(lines):
    line = lines[i]
    if re.match(r'^template\s*<', line) and i + 1 < len(lines):
        next_line = lines[i + 1]
        if next_line.startswith('struct '):
            i += 1; continue
        m = re.match(r'^INLINE\s+(\S+(?:\s+\S+)*?)\s+(\w+)\(M68K\* z(.*?)\)\s*(?://.*)?$', next_line)
        if not m:
            i += 1; continue
        ret_type = m.group(1)
        op_name = m.group(2)
        rest_args = m.group(3).strip()
        if rest_args.startswith(','):
            rest_args = rest_args[1:].strip()

        tpl_match = re.match(r'^template\s*<(.*?)>$', line)
        tpl_params = []
        for p in tpl_match.group(1).split(','):
            p = p.strip()
            tm = re.match(r'^(typename|AddressMode)\s+(\w+)$', p)
            if tm:
                tpl_params.append((tm.group(1), tm.group(2)))

        args = split_args(rest_args)
        # Preserve ORIGINAL ARG ORDER, tagging each as HAM or extra.
        ordered_args = []  # list of ('ham', T, AM, var) or ('extra', decl)
        ham_args = []
        for arg in args:
            hm = re.match(r'^M68K::HAM<(\w+),\s*(\w+)>\s*&\s*(\w+)$', arg)
            if hm:
                ordered_args.append(('ham', hm.group(1), hm.group(2), hm.group(3)))
                ham_args.append((hm.group(1), hm.group(2), hm.group(3)))
            else:
                ordered_args.append(('extra', arg))

        body_start = i + 2
        depth = 0; started = False
        k = body_start
        while k < len(lines):
            for ch in lines[k]:
                if ch == '{': depth += 1; started = True
                elif ch == '}': depth -= 1
            k += 1
            if started and depth == 0:
                break
        body_lines = lines[body_start:k]

        ops[op_name] = {
            'tpl_params': tpl_params,
            'ret_type': ret_type,
            'ham_args': ham_args,
            'ordered_args': ordered_args,
            'body_lines': body_lines,
        }
        i = k
    else:
        i += 1

print(f"Parsed {len(ops)} ops", file=sys.stderr)


def transform_ham_method_call(body, var_name, fn_macro_name):
    out = []
    i = 0
    while i < len(body):
        m = re.match(rf'\b{re.escape(var_name)}\.(\w+)\(', body[i:])
        if not m:
            out.append(body[i]); i += 1; continue
        method = m.group(1)
        paren_open = i + m.end() - 1
        paren_close = find_matching_paren(body, paren_open)
        if paren_close == -1:
            out.append(body[i]); i += 1; continue
        inner = body[paren_open + 1:paren_close]
        args = split_args(inner) if inner.strip() else []
        if method == 'write' and len(args) == 1:
            args.append('2')
        call_args = ', '.join([var_name] + args)
        out.append(f'{fn_macro_name}(_{method})({call_args})')
        i = paren_close + 1
    return ''.join(out)


def transform_helper_call(body, helper_name, macro_name, var_subst=None):
    out = []
    i = 0
    while i < len(body):
        m = re.match(rf'\b{re.escape(helper_name)}\(', body[i:])
        if not m:
            out.append(body[i]); i += 1; continue
        paren_open = i + m.end() - 1
        paren_close = find_matching_paren(body, paren_open)
        if paren_close == -1:
            out.append(body[i]); i += 1; continue
        inner = body[paren_open + 1:paren_close]
        if var_subst:
            for s, d in var_subst.items():
                inner = re.sub(rf'\b{s}\b', d, inner)
        out.append(f'{macro_name}({inner})')
        i = paren_close + 1
    return ''.join(out)


def transform_body(op_info):
    body = '\n'.join(op_info['body_lines'])

    # 1. Drop static_assert
    body = re.sub(r'^\s*static_assert\(.*?\);\s*$', '', body, flags=re.MULTILINE)

    # 2. HAM<DT, IMMEDIATE> dummy_zero(z, 0); -> struct init
    body = re.sub(
        r'M68K::HAM<\s*(\w+)\s*,\s*IMMEDIATE\s*>\s+dummy_zero\(z,\s*0\);',
        'struct OP_HAM_DUMMY_IMM dummy_zero; HAM_DUMMY_FN(_init_arg)(&dummy_zero, z, 0);',
        body
    )

    # 3. HAM method calls
    ham_vars = [(var, f'HAM_{var.upper()}_FN') for tp, am, var in op_info['ham_args']]
    if 'dummy_zero' in body:
        ham_vars.append(('dummy_zero', 'HAM_DUMMY_FN'))
    for var, fn_macro in ham_vars:
        body = transform_ham_method_call(body, var, fn_macro)

    # 4. var.field -> var->field
    for var, fn_macro in ham_vars:
        body = re.sub(rf'\b{var}\.(\w)', rf'{var}->\1', body)

    # 5. Helper template calls
    body = transform_helper_call(body, 'Subtract', 'OP_SUBTRACT', var_subst={'dummy_zero': '&dummy_zero'})
    body = transform_helper_call(body, 'ShiftBase', 'OP_SHIFTBASE')
    body = transform_helper_call(body, 'RotateBase', 'OP_ROTATEBASE')

    # 6. sizeof(T)/sizeof(DT)
    body = re.sub(r'\bsizeof\(T\)', 'OP_TSIZE', body)
    body = re.sub(r'\bsizeof\(DT\)', 'OP_DTSIZE', body)

    # 7. typename std::make_signed
    body = re.sub(r'typename\s+std::make_signed\s*<\s*T\s*>::type', 'OP_T_SIGNED', body)
    body = re.sub(r'typename\s+std::make_signed\s*<\s*DT\s*>::type', 'OP_DT_SIGNED', body)

    # 8. static_cast<X>(y)
    body = re.sub(r'static_cast<\s*([^>]+)\s*>\((.*?)\)', r'(\1)(\2)', body)

    # 9. Template params: T/DT/SAM/DAM/TAM -> OP_T/etc.
    tpl_names = sorted([n for _, n in op_info['tpl_params']], key=len, reverse=True)
    for name in tpl_names:
        body = re.sub(rf'\b{name}\b', f'OP_{name}', body)

    # 10. Cleanup blank lines
    body = re.sub(r'\n\s*\n\s*\n+', '\n\n', body)
    return body


def gen_body_file(name, op_info):
    transformed = transform_body(op_info)

    # Transform return type: replace template-param identifiers (T/DT/TAM/etc.)
    ret_type = op_info['ret_type']
    for tp_name in sorted([n for _, n in op_info['tpl_params']], key=len, reverse=True):
        ret_type = re.sub(rf'\b{tp_name}\b', f'OP_{tp_name}', ret_type)

    # Emit params in ORIGINAL ORDER
    params = ''
    fn_macros = []
    fn_macros_undef = []
    struct_undefs = []
    seen_ham_vars = set()
    for entry in op_info['ordered_args']:
        if entry[0] == 'ham':
            _, T, AM, var = entry
            ms = f'OP_HAM_{var.upper()}'
            mf = f'HAM_{var.upper()}_FN'
            params += f', struct {ms}* {var}'
            if var not in seen_ham_vars:
                fn_macros.append(f'#define {mf}(n) _OP_PASTE({ms}, n)')
                fn_macros_undef.append(f'#undef {mf}')
                struct_undefs.append(f'#undef {ms}')
                seen_ham_vars.add(var)
        else:
            _, extra = entry
            params += f', {extra}'

    if 'dummy_zero' in '\n'.join(op_info['body_lines']):
        fn_macros.append('#define HAM_DUMMY_FN(n) _OP_PASTE(OP_HAM_DUMMY_IMM, n)')
        fn_macros_undef.append('#undef HAM_DUMMY_FN')
        struct_undefs.append('#undef OP_HAM_DUMMY_IMM')

    tpl_undefs = [f'#undef OP_{n}' for _, n in op_info['tpl_params']]
    # Always undef every OP_* macro the instances generator might define for this op.
    # #undef of a non-existent macro is a no-op in C.
    defensive_undefs = ['#undef OP_TSIZE', '#undef OP_DTSIZE',
                        '#undef OP_T_SIGNED', '#undef OP_DT_SIGNED',
                        '#undef OP_SUBTRACT', '#undef OP_SHIFTBASE',
                        '#undef OP_ROTATEBASE']

    out = '/* m68k_op_' + name + '_body.inc.h\n'
    out += ' * Phase-9d-15b: macro-parameterised body of the ' + name + ' op.\n'
    out += ' * Auto-generated by tools/gen_op_macros.py. */\n\n'
    out += '#ifndef OP_NAME\n'
    out += '#error "OP_NAME must be defined"\n'
    out += '#endif\n\n'
    out += '#define _OP_PASTE2(a, b) a ## b\n'
    out += '#define _OP_PASTE(a, b) _OP_PASTE2(a, b)\n'
    out += '\n'.join(fn_macros) + '\n\n'
    # The three helper templates (ShiftBase / RotateBase / Subtract) take
    # runtime bool parameters (Arithmetic / ShiftLeft / X_form) that every
    # caller passes as literal true/false.  With plain INLINE, gcc cloned
    # them via .constprop but left the actual call out-of-line, so the
    # inner loops still branched on those constants at runtime.  Force
    # inlining so each caller gets a fully constant-folded copy and the
    # bool branches disappear.
    helper_force_inline = name in ('ShiftBase', 'RotateBase', 'Subtract')
    inline_kw = 'MDFN_FORCE_INLINE' if helper_force_inline else 'INLINE'
    out += f'static {inline_kw} {ret_type} OP_NAME(M68K* z{params})\n'
    out += transformed.strip() + '\n\n'
    out += '#undef _OP_PASTE2\n#undef _OP_PASTE\n'
    out += '\n'.join(fn_macros_undef) + '\n#undef OP_NAME\n'
    out += '\n'.join(struct_undefs + tpl_undefs + defensive_undefs) + '\n'
    return out


for name, info in ops.items():
    out = gen_body_file(name, info)
    path = os.path.join(OUT_DIR, f'm68k_op_{name}_body.inc.h')
    with open(path, 'w') as f:
        f.write(out)
print(f"Wrote {len(ops)} body files", file=sys.stderr)
