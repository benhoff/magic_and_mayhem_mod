#!/usr/bin/env python3
"""Compare original queue routing/metadata under synthetic backend contracts."""
import argparse
import hashlib
import importlib.util
import json
from pathlib import Path
import struct
import subprocess
import tempfile
from drawing_census import ROOT, BUILD_SHA256, instructions
from drawing_dispatch import slices

BINDINGS = {0x4df840:20, 0x57dc60:8, 0x57de00:0, 0x57e540:4, 0x57e8e0:12,
            0x57ec90:12, 0x57f0f0:12, 0x57f5f0:12, 0x57fe10:12, 0x580260:12,
            0x5806f0:8, 0x580a90:12, 0x581200:20, 0x595677:0, 0x4d8ab0:12}


def run(executable, claims=None):
    raw = executable.read_bytes()
    if hashlib.sha256(raw).hexdigest() != BUILD_SHA256: raise ValueError('Unsupported original consumer input')
    paths = ['tools/test-drawing-dispatch.py', 'tools/drawing_dispatch.py', 'tools/drawing_census.py',
             'tools/inspect-world-dispatch.py', 'tests/drawing-dispatch-reference.cpp',
             'tests/sprite-binary-reference.cpp', 'tests/test-drawing-dispatch.py',
             'research/runtime/coverage/binary-nocd.json']
    sources = {p: hashlib.sha256((ROOT / p).read_bytes()).hexdigest() for p in paths}
    declaration = json.loads(claims.read_text()) if claims else None
    if declaration:
        for p, digest in declaration['sources'].items():
            if hashlib.sha256((ROOT / p).read_bytes()).hexdigest() != digest: raise ValueError('Changed declaration source: ' + p)
            sources[p] = digest
    inventory = json.loads((ROOT / 'research/runtime/coverage/binary-nocd.json').read_text())
    contracts = []
    for address, pop in BINDINGS.items():
        function = next(f for f in inventory['functions'] if f['entry'] == address)
        returns = []
        for start, end in function['ranges']:
            assembly = subprocess.check_output(['objdump', '-d', '-Mintel', '--start-address=' + hex(start),
                '--stop-address=' + hex(end), str(executable)], text=True)
            for row in instructions(assembly).values():
                if row['mnemonic'] == 'ret':
                    data = bytes.fromhex(row['bytes'])
                    cleanup = 0 if data == b'\xc3' else struct.unpack('<H', data[1:])[0] if len(data) == 3 and data[0] == 0xc2 else None
                    if cleanup != pop: raise ValueError('Stub disagrees with explicit original RET contract')
                    returns.append(row)
        if not returns: raise ValueError('Backend lacks explicit original return contract')
        contracts.append({'address': address, 'pop': pop, 'returns': returns})
    parent = ROOT / 'working/tests/drawing-dispatch'; parent.mkdir(parents=True, exist_ok=True)
    out = Path(tempfile.mkdtemp(prefix='run-', dir=parent)); print(out, flush=True)
    def command(name, argv):
        with (out / (name + '.log')).open('x') as log:
            subprocess.run([str(v) for v in argv], stdout=log, stderr=subprocess.STDOUT, check=True, timeout=90)
    command('build', ['g++', '-m32', '-O2', '-std=c++17', '-fno-pie', '-no-pie', '-mno-sse', '-mno-mmx',
        '-Wall', '-Wextra', '-Werror', ROOT / 'tests/drawing-dispatch-reference.cpp', '-o', out / 'reference'])
    with (out / 'cases.jsonl').open('x') as output, (out / 'execution.log').open('x') as log:
        subprocess.run([str(out / 'reference'), str(executable)], stdout=output, stderr=log, check=True, timeout=90)
    cases = [json.loads(line) for line in (out / 'cases.jsonl').read_text().splitlines()]
    if len(cases) != 2432 or {r['kind'] for r in cases} != set(range(-2, 36)): raise ValueError('Incomplete original case matrix')
    wave_cases = colour_cases = 0
    for case in cases:
        for call in case['calls']:
            if call['tag'] == 0x5806f0:
                if not call['ecx_object'] or call['edx'] != 120 or call['args'][:2] != [100, case['kind'] - 14]:
                    raise ValueError('Original wave argument contract differs')
                wave_cases += 1
            if call['tag'] == 0x581200:
                colours = {12:[244,198,13], 13:[10,10,255], 14:[214,95,172]}
                if call['args'][2:5] != colours[case['kind']] or call['args'][1] != (case['offset'] & 0xffffffff if case['video'] else 0):
                    raise ValueError('Original literal-colour argument contract differs')
                colour_cases += 1
    spec = importlib.util.spec_from_file_location('dispatch_export', ROOT / 'tools/inspect-world-dispatch.py')
    exporter = importlib.util.module_from_spec(spec); spec.loader.exec_module(exporter)
    static = exporter.inspect(executable); static['branch_slices'] = slices(static)
    (out / 'static.json').write_text(json.dumps(static, indent=2) + '\n')
    command('slice-guards', ['python3', '-B', ROOT / 'tests/test-drawing-dispatch.py'])
    if hashlib.sha256(executable.read_bytes()).hexdigest() != BUILD_SHA256: raise ValueError('Original input changed during comparison')
    if sources != {p: hashlib.sha256((ROOT / p).read_bytes()).hexdigest() for p in sources}:
        raise ValueError('Sources changed during comparison')
    report = {'schema': 1, 'success': True, 'sources': sources, 'sources_stable': True,
        'original_sha256': BUILD_SHA256, 'input_unchanged': True, 'original_consumer_instructions_unchanged': True,
        'original_cases': len(cases), 'table_entries': 105, 'wave_calls_checked': wave_cases,
        'colour_calls_checked': colour_cases, 'backend_return_contracts': contracts,
        'artifacts': {str(p.relative_to(ROOT)): {'sha256': hashlib.sha256(p.read_bytes()).hexdigest(),
            'bytes': p.stat().st_size} for p in [out / 'cases.jsonl', out / 'static.json', out / 'reference']},
        'scope': 'Unmodified pinned consumer with synthetic backend bindings: 0..34 kinds and -2/-1/35 defaults, Video0/1, selected auxiliary span, palette-marker and -127/ordinary offset variants, AX0/1/1234/ffff. Check metadata zero/default/boolean/truncation policies and literal wave/colour arguments. All105 raw branch slices retained. Downstream pixels, arbitrary virtual objects, helper global effects, changed prior metadata, clip-boundary auxiliary paths and live graphics API/lifetime equivalence are excluded.'}
    if declaration: report['claims'] = declaration['claims']
    (out / 'report.json').write_text(json.dumps(report, indent=2) + '\n')
    print(json.dumps({'output': str(out), 'cases': len(cases), 'wave_calls': wave_cases, 'colour_calls': colour_cases}), flush=True)
    return report


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--executable', type=Path, required=True)
    parser.add_argument('--claims', type=Path)
    args = parser.parse_args()
    subprocess.run([str(ROOT / 'tools/original-manifest.sh'), 'verify'], check=True)
    try: run(args.executable, args.claims)
    finally: subprocess.run([str(ROOT / 'tools/original-manifest.sh'), 'verify'], check=True)
