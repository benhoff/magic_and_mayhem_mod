#!/usr/bin/env python3
"""Execute pinned original queue state/clip/postlude branches with synthetic backends."""
import argparse
from collections import Counter
import hashlib
import importlib.util
import json
from pathlib import Path
import struct
import subprocess
import tempfile
from drawing_census import ROOT, BUILD_SHA256, instructions


def run(executable, claims):
    if hashlib.sha256(executable.read_bytes()).hexdigest() != BUILD_SHA256:
        raise ValueError('Unsupported original consumer input')
    declaration = json.loads(claims.read_text())
    sources = declaration['sources']
    if any(hashlib.sha256((ROOT / p).read_bytes()).hexdigest() != h for p, h in sources.items()):
        raise ValueError('Prospective source changed before execution')
    spec = importlib.util.spec_from_file_location('dispatch_fixture', ROOT / 'tools/test-drawing-dispatch.py')
    module = importlib.util.module_from_spec(spec); spec.loader.exec_module(module)
    inventory = json.loads((ROOT / 'research/runtime/coverage/binary-nocd.json').read_text())
    contracts = []
    for address, pop in module.BINDINGS.items():
        function = next(f for f in inventory['functions'] if f['entry'] == address)
        returns = []
        for start, end in function['ranges']:
            assembly = subprocess.check_output(['objdump', '-d', '-Mintel', '--start-address=' + hex(start),
                '--stop-address=' + hex(end), str(executable)], text=True)
            for row in instructions(assembly).values():
                if row['mnemonic'] == 'ret':
                    raw = bytes.fromhex(row['bytes'])
                    cleanup = 0 if raw == b'\xc3' else struct.unpack('<H', raw[1:])[0] if len(raw) == 3 and raw[0] == 0xc2 else None
                    if cleanup != pop: raise ValueError('Synthetic binding differs from original RET')
                    returns.append(row)
        if not returns: raise ValueError('Binding lacks original RET evidence')
        contracts.append({'address': address, 'pop': pop, 'returns': returns})
    parent = ROOT / 'working/tests/drawing-queue-state'; parent.mkdir(parents=True, exist_ok=True)
    out = Path(tempfile.mkdtemp(prefix='run-', dir=parent)); print(out, flush=True)
    with (out / 'build.log').open('x') as log:
        subprocess.run(['g++', '-m32', '-O2', '-std=c++17', '-fno-pie', '-no-pie', '-mno-sse', '-mno-mmx',
            '-Wall', '-Wextra', '-Werror', str(ROOT / 'tests/drawing-queue-state-reference.cpp'),
            '-o', str(out / 'reference')], stdout=log, stderr=subprocess.STDOUT, check=True, timeout=90)
    with (out / 'cases.jsonl').open('x') as output, (out / 'execution.log').open('x') as log:
        result = subprocess.run([str(out / 'reference'), str(executable)], stdout=output, stderr=log, timeout=90)
    (out / 'execution-status.json').write_text(json.dumps({'return_code': result.returncode,
        'binary_sha256': hashlib.sha256((out / 'reference').read_bytes()).hexdigest()}, indent=2) + '\n')
    if result.returncode: raise ValueError('Original state fixture exit ' + str(result.returncode) + '; retained attempt: ' + str(out))
    totals, kinds, returns_seen, count = Counter(), Counter(), Counter(), 0
    axes = {'video': {0, 1}, 'count': {0, 1, 2}, 'postlude': {0, 1}, 'kind': {9, 15, 35},
        'y': {-20, -12, 0, 1, 100, 468, 480, 588, 600},
        'first': {0x8ad0, 0xfff3, 0xfff4, 0xfff5, 0, 5},
        'prior': {0x8ad0, 0, 1, 0x1234, 0xffff}, 'height_flag': {0, 1},
        'result': {0, 0x12345678, 0xffffffff}}
    seen = set()
    with (out / 'cases.jsonl').open() as source:
        for line in source:
            case = json.loads(line); count += 1
            key = tuple(case[field] for field in axes)
            if any(type(case[field]) is not int or case[field] not in allowed for field, allowed in axes.items()) or key in seen:
                raise ValueError('Unknown or duplicated original state matrix cell')
            seen.add(key)
            kinds[case['kind']] += 1
            for call in case['calls']:
                totals[hex(call['tag'])] += 1; returns_seen[hex(call['return_pc'])] += 1
                if call['tag'] == 0x4d8ab0 and (call['ecx'] != 0x6ddf78 or call['args'][:3] != [0x10203040, 0x11223344, 0xaabbccdd]):
                    raise ValueError('Postlude state arguments differ')
            if case['postlude_after'] != ([0, 0, 0] if case['postlude'] else [0x10203040, 0x11223344, 0xaabbccdd]):
                raise ValueError('Original postlude counter reset differs')
    if count != 58320 or kinds != {9: 19440, 15: 19440, 35: 19440}:
        raise ValueError('Incomplete original state matrix')
    if hashlib.sha256(executable.read_bytes()).hexdigest() != BUILD_SHA256:
        raise ValueError('Original input changed during execution')
    if any(hashlib.sha256((ROOT / p).read_bytes()).hexdigest() != h for p, h in sources.items()):
        raise ValueError('Sources changed during execution')
    report = {'schema': 1, 'success': True, 'scope': 'Unchanged pinned consumer with synthetic backend/clip/cleanup bindings: empty/one/two rows, Video0/1, sentinel and changed prior metadata, signed spans, strict clip and480/600height branches, raw high/negative backend results, postlude0/1. Original row/container and counter mutations and exact opaque CALL arguments are checked. No actual helper side effects, pixels, external objects/graphics or full dispatch/scenario closure.',
        'sources': sources, 'claims': declaration['claims'], 'sources_stable': True,
        'original_sha256': BUILD_SHA256, 'input_unchanged': True,
        'original_consumer_instructions_unchanged': True, 'original_cases': count,
        'unique_matrix_cells': len(seen), 'matrix_axes': {k: sorted(v) for k, v in axes.items()},
        'cases_by_kind': dict(kinds), 'calls_by_tag': dict(totals), 'calls_by_exact_return_pc': dict(returns_seen),
        'backend_return_contracts': contracts,
        'artifacts': {str(p.relative_to(ROOT)): {'sha256': hashlib.sha256(p.read_bytes()).hexdigest(), 'bytes': p.stat().st_size}
                      for p in [out / 'cases.jsonl', out / 'reference', out / 'build.log', out / 'execution.log']}}
    (out / 'report.json').write_text(json.dumps(report, indent=2) + '\n')
    print(json.dumps({'output': str(out), 'cases': count, 'calls_by_tag': dict(totals)}), flush=True)
    return report


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--executable', type=Path, required=True)
    parser.add_argument('--claims', type=Path, required=True)
    args = parser.parse_args()
    subprocess.run([str(ROOT / 'tools/original-manifest.sh'), 'verify'], check=True)
    try: run(args.executable, args.claims)
    finally: subprocess.run([str(ROOT / 'tools/original-manifest.sh'), 'verify'], check=True)
