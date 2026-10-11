#!/usr/bin/env python3
"""Build signature-pinned observation-only drawing census hooks; no CRT."""
import argparse
import hashlib
import importlib.util
import json
from pathlib import Path
import re
import struct
import subprocess
from drawing_census import ROOT, BUILD_SHA256, instructions, splice_conflicts


def load(name, path):
    spec = importlib.util.spec_from_file_location(name, ROOT / path)
    module = importlib.util.module_from_spec(spec); spec.loader.exec_module(module)
    return module


def site_mode(route, entry, inventory):
    legacy_site = entry in (0x58d3b0, 0x58edb0)
    mode = route.get('observation_mode', 'site' if legacy_site else 'entry')
    if type(mode) is not str or mode not in ('entry', 'site'):
        raise ValueError('Unreviewed observation mode')
    if legacy_site and mode != 'site':
        raise ValueError('Legacy interior cannot acquire a return slot')
    if mode == 'site' and not legacy_site:
        anchor = route.get('original_anchor')
        if type(anchor) is not int or not any(f['entry'] == anchor and
                any(start <= entry < end for start, end in f['ranges'])
                for f in inventory['functions']):
            raise ValueError('Interior site needs an explicit containing original function')
    return mode == 'site'


def hook_rows(executable):
    raw = executable.read_bytes()
    if hashlib.sha256(raw).hexdigest() != BUILD_SHA256:
        raise ValueError('Unsupported original executable; no build')
    offset = load('census_pe', 'tools/export-menu-support.py').image_offset
    catalog = json.loads((ROOT / 'research/runtime/drawing-inventory.json').read_text())
    inventory = json.loads((ROOT / 'research/runtime/coverage/binary-nocd.json').read_text())
    text = (ROOT / 'runtime/scene/canvas_producers.c').read_text()
    known = {int(a, 16): int(pop) for a, pop in re.findall(
        r'\{(0x[0-9a-fA-F]+),\d+,(\d+),\{[^}]+\},producer_enter_\d+,-?\d+\}', text)}
    all_code = instructions(subprocess.check_output(['objdump', '-d', '-Mintel', str(executable)], text=True))
    rows, refused = [], []
    for route in catalog['routes']:
        selectors = [s for s in route['selectors'] if s['kind'] == 'entry']
        if not selectors: continue
        entry = int(selectors[0]['identity'], 0) if isinstance(selectors[0]['identity'], str) else selectors[0]['identity']
        try:
            site = site_mode(route, entry, inventory)
            asm = subprocess.check_output(['objdump', '-d', '-Mintel', '--start-address=' + hex(entry),
                                          '--stop-address=' + hex(entry + 32), str(executable)], text=True)
            code = instructions(asm); cursor = entry; signature = bytearray(); relocs = []
            while len(signature) < 5:
                instruction = code[cursor]; data = bytes.fromhex(instruction['bytes'])
                if instruction['mnemonic'].startswith('j') and data[0] != 0xe9:
                    raise ValueError('Short/conditional prologue jump needs a dedicated relocation')
                if data[0] in (0xe8, 0xe9):
                    if len(data) != 5: raise ValueError('Unreviewed relative instruction')
                    relocs.append(len(signature))
                if instruction['mnemonic'].startswith('ret'):
                    raise ValueError('Return before a complete entry splice')
                signature.extend(data); cursor += len(data)
            crossing = splice_conflicts(entry, len(signature), all_code)
            if crossing: raise ValueError('Original branch enters overwritten interior bytes: ' + repr(crossing))
            if len(signature) > 16 or len(relocs) > 3:
                raise ValueError('Entry splice exceeds bounded signature/relocation storage')
            if bytes(signature) != raw[offset(raw, entry, len(signature)):][:len(signature)]:
                raise ValueError('Disassembly bytes disagree with original file-backed bytes')
            if site: pop = 0
            elif entry in known: pop = known[entry]
            else:
                function = next((f for f in inventory['functions'] if f['entry'] == entry), None)
                if function is None: raise ValueError('No explicit recovered return contract')
                pops = set()
                for start, end in function['ranges']:
                    body = instructions(subprocess.check_output(['objdump', '-d', '-Mintel',
                        '--start-address=' + hex(start), '--stop-address=' + hex(end), str(executable)], text=True))
                    for i in body.values():
                        if i['mnemonic'] in ('ret', 'retw'):
                            b = bytes.fromhex(i['bytes'])
                            if b == b'\xc3': pops.add(0)
                            elif len(b) == 3 and b[0] == 0xc2: pops.add(struct.unpack('<H', b[1:])[0])
                            else: raise ValueError('Unreviewed return instruction')
                if len(pops) != 1: raise ValueError('Missing or inconsistent explicit RET contract')
                pop = pops.pop()
            rows.append({'tag': len(rows), 'entry': entry, 'signature': signature.hex(), 'length': len(signature),
                         'pop': pop, 'site': site, 'relocations': relocs, 'route': route['id']})
        except (KeyError, ValueError) as error:
            refused.append({'entry': entry, 'route': route['id'], 'reason': str(error)})
    if hashlib.sha256(executable.read_bytes()).hexdigest() != BUILD_SHA256:
        raise ValueError('Original executable changed during build preparation')
    if len(rows) > 256: raise ValueError('Hook census exceeds reviewed capacity')
    return rows, refused


def build(executable):
    rows, refused = hook_rows(executable)
    output = ROOT / 'working/build/drawing-census'; output.mkdir(parents=True, exist_ok=True)
    imports = dict(load('census_imports', 'tools/build-shadow-bridge.py').IMPORTS,
                   GetCurrentThreadId=0, GetTickCount=0, Sleep=4, CreateFileMappingA=24,
                   MapViewOfFile=20, UnmapViewOfFile=4)
    definition = output / 'kernel32.def'
    definition.write_text('LIBRARY KERNEL32.dll\nEXPORTS\n' + ''.join(f'{name}@{size}\n' for name, size in imports.items()))
    subprocess.run(['llvm-dlltool', '-m', 'i386', '-D', 'KERNEL32.dll', '-d', str(definition),
                    '-l', str(output / 'kernel32.lib'), '--kill-at'], check=True)
    header = [f'#define CENSUS_HOOKS {len(rows)}',
              'struct Hook {u32 address,length,pop,site,relocs;u8 bytes[16];u32 reloc[3];void(*entry)(void);};',
              'static const u8 original_sha256[32]={' + ','.join(str(v) for v in bytes.fromhex(BUILD_SHA256)) + '};']
    header += [f'extern void census_enter_{r["tag"]}(void);' for r in rows]
    header.append('static const struct Hook hooks[CENSUS_HOOKS]={')
    for r in rows:
        header.append('{%d,%d,%d,%d,%d,{%s},{%s},census_enter_%d},' % (
            r['entry'], r['length'], r['pop'], r['site'], len(r['relocations']),
            ','.join(str(v) for v in bytes.fromhex(r['signature'])),
            ','.join(str(v) for v in r['relocations']) or '0', r['tag']))
    header.append('};'); (output / 'hooks.inc').write_text('\n'.join(header) + '\n')
    assembly = (ROOT / 'runtime/drawing-census/entry.S').read_text()
    assembly += '\n' + '\n'.join(f'census_entry {r["tag"]}' for r in rows) + '\n'
    (output / 'entry.S').write_text(assembly)
    flags = ['clang', '--target=i686-pc-windows-msvc', '-O2', '-ffreestanding', '-fno-builtin',
             '-fno-stack-protector', '-mno-sse', '-mno-mmx', '-Wall', '-Wextra', '-Werror', '-I', str(output)]
    for source, name in [(ROOT / 'runtime/drawing-census/observer.c', 'observer'), (output / 'entry.S', 'entry')]:
        subprocess.run([*flags, '-c', str(source), '-o', str(output / (name + '.obj'))], check=True)
    dll = output / 'MnmDrawingCensus.dll'
    subprocess.run(['lld-link', '/dll', '/machine:x86', '/entry:DllMain@12', '/nodefaultlib',
                    '/safeseh:no', '/timestamp:0', '/out:' + str(dll), str(output / 'observer.obj'),
                    str(output / 'entry.obj'), str(output / 'kernel32.lib')], check=True)
    paths = ['tools/build-drawing-census.py', 'tools/drawing_census.py', 'tools/build-shadow-bridge.py',
             'tools/export-menu-support.py', 'runtime/drawing-census/observer.c', 'runtime/drawing-census/entry.S',
             'runtime/shadow/win32_min.h', 'runtime/scene/canvas_producers.c',
             'protocols/include/mnm/drawing_census_v1.h', 'research/runtime/drawing-inventory.json',
             'research/runtime/coverage/binary-nocd.json']
    report = {'schema': 1, 'original_sha256': BUILD_SHA256, 'dll_sha256': hashlib.sha256(dll.read_bytes()).hexdigest(),
              'hooks': rows, 'unhookable': refused, 'sources': {p: hashlib.sha256((ROOT / p).read_bytes()).hexdigest() for p in paths},
              'scope': 'Observation-only entry/return metadata. Safe splices only; refused entries remain explicit. '
                       'No writer completeness, side-effect equivalence, full dispatch or scenario closure assertion.'}
    (output / 'manifest.json').write_text(json.dumps(report, indent=2) + '\n')
    print(json.dumps({'dll': str(dll), 'hooks': len(rows), 'unhookable': refused}))
    return dll


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__); parser.add_argument('--executable', type=Path, required=True)
    args = parser.parse_args()
    subprocess.run([str(ROOT / 'tools/original-manifest.sh'), 'verify'], check=True)
    try: build(args.executable)
    finally: subprocess.run([str(ROOT / 'tools/original-manifest.sh'), 'verify'], check=True)
