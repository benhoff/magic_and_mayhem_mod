#!/usr/bin/env python3
"""Original render-postlude client state with real clip/frame/decimal helpers."""
from coverage_evidence import read_bytes as evidence_bytes, read_text as evidence_text
import argparse
from collections import Counter
import hashlib
import json
from pathlib import Path
import struct
import subprocess
import tempfile
from drawing_census import ROOT, BUILD_SHA256, instructions


def run(executable, claims):
    if hashlib.sha256(evidence_bytes(executable)).hexdigest() != BUILD_SHA256:
        raise ValueError('Unsupported original postlude input')
    declaration = json.loads(evidence_text(claims)); sources = declaration['sources']
    if any(hashlib.sha256(evidence_bytes(ROOT / p)).hexdigest() != h for p, h in sources.items()):
        raise ValueError('Prospective source changed')
    inventory = json.loads(evidence_text(ROOT / 'research/runtime/coverage/binary-nocd.json'))
    contracts = []
    for address, pop in {0x4d8ab0: 12, 0x58bac0: 8, 0x58b660: 0, 0x4a5ce0: 28,
                         0x57dc20: 4, 0x57dc60: 8, 0x57dc90: 0, 0x59de2f: 0}.items():
        function = next(f for f in inventory['functions'] if f['entry'] == address); returns = []
        for start, end in function['ranges']:
            assembly = subprocess.check_output(['objdump', '-d', '-Mintel', '--start-address=' + hex(start),
                '--stop-address=' + hex(end), str(executable)], text=True)
            for row in instructions(assembly).values():
                if row['mnemonic'] == 'ret':
                    raw = bytes.fromhex(row['bytes']); cleanup = 0 if raw == b'\xc3' else struct.unpack('<H', raw[1:])[0] if len(raw) == 3 and raw[0] == 0xc2 else None
                    if cleanup != pop: raise ValueError('Fixture disagrees with original RET')
                    returns.append(row)
        if not returns: raise ValueError('Missing original RET')
        contracts.append({'address': address, 'pop': pop, 'returns': returns})
    parent = ROOT / 'working/tests/drawing-postlude'; parent.mkdir(parents=True, exist_ok=True)
    out = Path(tempfile.mkdtemp(prefix='run-', dir=parent)); print(out, flush=True)
    with (out / 'build.log').open('x') as log:
        subprocess.run(['g++', '-m32', '-O2', '-std=c++17', '-fno-pie', '-no-pie', '-mno-sse', '-mno-mmx',
            '-Wall', '-Wextra', '-Werror', str(ROOT / 'tests/drawing-postlude-reference.cpp'), '-o', str(out / 'reference')],
            stdout=log, stderr=subprocess.STDOUT, check=True, timeout=90)
    with (out / 'cases.jsonl').open('x') as output, (out / 'execution.log').open('x') as log:
        result = subprocess.run([str(out / 'reference'), str(executable)], stdout=output, stderr=log, timeout=90)
    (out / 'execution-status.json').write_text(json.dumps({'return_code': result.returncode}, indent=2) + '\n')
    if result.returncode: raise ValueError('Original postlude fixture failed: ' + str(out))
    seen, totals, texts = set(), Counter(), set()
    with (out / 'cases.jsonl').open() as source:
        for line in source:
            case = json.loads(line); key = tuple(case[k] for k in ('enabled', 'ready', 'lock', 'colour', 'large', 'width', 'unlock', 'value', 'rows', 'height', 'unused'))
            if key in seen: raise ValueError('Duplicate postlude matrix cell')
            seen.add(key)
            if case['draw']: texts.add(case['text_hex'])
            for call in case['calls']: totals[hex(call['tag'])] += 1
    if len(seen) != 12288 or len(texts) != 6: raise ValueError('Incomplete postlude matrix/text coverage')
    if hashlib.sha256(evidence_bytes(executable)).hexdigest() != BUILD_SHA256:
        raise ValueError('Original input changed')
    if any(hashlib.sha256(evidence_bytes(ROOT / p)).hexdigest() != h for p, h in sources.items()):
        raise ValueError('Sources changed during execution')
    report = {'schema': 1, 'success': True,
        'scope': 'Unchanged original4d8ab0 render-postlude,57dc20frame/57dc60clip/57dc90getter and decimal formatter instructions with owned synthetic object/arrays. Canvas fill/Lock, font body and Surface Unlock are explicit synthetic bindings; client disabled/not-ready/null-Lock exits, prefix/signed decimal text, viewport/row alignment, metadata/real global stores and ignored failure results are checked. No actual pixels/font/canvas/COM/driver behavior, global ownership or live/scenario/full dispatch closure.',
        'sources': sources, 'claims': declaration['claims'], 'sources_stable': True, 'original_sha256': BUILD_SHA256,
        'input_unchanged': True, 'original_instructions_unchanged': True, 'original_cases': len(seen),
        'calls_by_tag': dict(totals), 'drawn_text_hex': sorted(texts), 'original_return_contracts': contracts,
        'artifacts': {str(p.relative_to(ROOT)): {'sha256': hashlib.sha256(evidence_bytes(p)).hexdigest(), 'bytes': p.stat().st_size}
                      for p in [out / 'cases.jsonl', out / 'reference', out / 'build.log', out / 'execution.log', out / 'execution-status.json']}}
    (out / 'report.json').write_text(json.dumps(report, indent=2) + '\n'); print(json.dumps({'output': str(out), 'cases': len(seen), 'calls': dict(totals)}), flush=True)
    return report


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__); parser.add_argument('--executable', type=Path, required=True)
    parser.add_argument('--claims', type=Path, required=True); args = parser.parse_args()
    subprocess.run([str(ROOT / 'tools/original-manifest.sh'), 'verify'], check=True)
    try: run(args.executable, args.claims)
    finally: subprocess.run([str(ROOT / 'tools/original-manifest.sh'), 'verify'], check=True)
