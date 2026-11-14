"""Derive one explicitly versioned caller policy without editing the base runtime."""
import ast
import copy
import hashlib
import json
import re

BASE = 'runtime/scene/world_raster_callers.h'
BASE_SHA = 'ecdeaffa7825aaeb363e0b543e12b09329f61f20027df8c981105366a8e8863a'
ORIGINAL = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
ADDITIONS = (
    dict(address=0x500a16, tag=12, signature='83c40c664866f7d81bc0'),
    dict(address=0x500a32, tag=17, signature='83c4146689461ee9c702'),
)


def require(value, message):
    if not value:
        raise ValueError(message)


def configuration(value):
    require(type(value) is dict and type(value.get('schema')) is int, 'Typed policy schema required')
    expected = dict(schema=1, policy='secondary-world-black-exact-callers-v1', base_header=BASE,
                    base_sha256=BASE_SHA, original_sha256=ORIGINAL, additions=list(ADDITIONS))
    require(value == expected, 'Different secondary caller policy')
    return copy.deepcopy(expected)


def derive_header(base, policy):
    policy = configuration(policy)
    require(hashlib.sha256(base).hexdigest() == policy['base_sha256'], 'Base caller header changed')
    text = base.decode('utf-8')
    opening = 'static const struct RasterCaller {u32 address;unsigned long long tags;u8 bytes[10];} raster_callers[]={\n'
    require(text.count(opening) == 1, 'Missing exact ordinary caller table')
    start = text.index(opening) + len(opening)
    end = text.index('\n};', start)
    existing = set(int(n, 16) for n in re.findall(r'\{(0x[0-9a-f]+),', text[start:end]))
    require(existing and all(v['address'] not in existing for v in policy['additions']), 'Caller already exists')
    rows = ''.join('\n {0x%x,0x%xull,{%s}},' %
                   (v['address'], 1 << v['tag'], ','.join(hex(n) for n in bytes.fromhex(v['signature'])))
                   for v in policy['additions'])
    result = text[:end] + rows + text[end:]
    require(result[:end] + result[end + len(rows):] == text, 'Base caller bytes were changed')
    return result.encode(), dict(insertion_offset=end, inserted_bytes=rows.encode().hex(),
                                 base_sha256=hashlib.sha256(base).hexdigest(),
                                 derived_sha256=hashlib.sha256(result.encode()).hexdigest(),
                                 existing_ordinary_callers=len(existing), added_exact_pairs=2,
                                 all_base_bytes_retained=True)


def original_bindings(image, policy):
    configuration(policy)
    result = []
    for row, target in zip(ADDITIONS, (0x595677, 0x57de00)):
        pc = row['address']
        require(image.read(pc, 10).hex() == row['signature'], 'Original caller signature changed')
        call = image.read(pc - 5, 5)
        require(call[0] == 0xe8 and pc + int.from_bytes(call[1:], 'little', signed=True) == target,
                'Original direct call target changed')
        result.append(dict(row, direct_target=target, call_bytes=call.hex()))
    require(image.read(0x500a16, 16).hex() == '83c40c664866f7d81bc0406685c074ac',
            'Black short AX branch changed')
    require(image.read(0x500a32, 12).hex() == '83c4146689461ee9c7020000',
            'Clipped AX store changed')
    return result


def compiler_overlay(virtual, physical):
    require(virtual.is_absolute() and physical.is_absolute() and virtual != physical,
            'Distinct absolute overlay paths required')
    return dict(version=0, **{'case-sensitive': 'true', 'roots': [
        dict(type='file', name=str(virtual), **{'external-contents': str(physical)})]})


def selected_builder(source):
    """Change only output selection and add the explicit Clang VFS input flag."""
    original = ast.parse(source)
    tree = copy.deepcopy(original)
    build = [n for n in tree.body if isinstance(n, ast.FunctionDef) and n.name == 'build']
    require(len(build) == 1, 'Missing original build function')
    changed = {}
    for node in ast.walk(build[0]):
        if not isinstance(node, ast.Assign) or len(node.targets) != 1 or not isinstance(node.targets[0], ast.Name):
            continue
        name = node.targets[0].id
        if name == 'output':
            require(name not in changed, 'Repeated output selection')
            changed[name] = copy.deepcopy(node.value)
            node.value = ast.copy_location(ast.Name(id='_secondary_output', ctx=ast.Load()), node.value)
        elif name == 'flags':
            require(name not in changed and isinstance(node.value, ast.List), 'Changed original compiler flags')
            changed[name] = copy.deepcopy(node.value)
            extra = ast.parse("['-ivfsoverlay', str(_secondary_overlay)]", mode='eval').body
            node.value = ast.copy_location(ast.BinOp(left=node.value, op=ast.Add(), right=extra), node.value)
    require(set(changed) == {'output', 'flags'}, 'Incomplete compiler input selection')
    check = copy.deepcopy(tree)
    for node in ast.walk(check):
        if isinstance(node, ast.Assign) and len(node.targets) == 1 and isinstance(node.targets[0], ast.Name):
            if node.targets[0].id in changed:
                node.value = copy.deepcopy(changed[node.targets[0].id])
    require(ast.dump(check, include_attributes=False) == ast.dump(original, include_attributes=False),
            'Other original build nodes changed')
    return ast.unparse(ast.fix_missing_locations(tree)) + '\n'
