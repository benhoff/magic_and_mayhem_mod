#!/usr/bin/env python3
"""Unchanged original Lock wrapper under bounded synthetic external results."""
import argparse
from collections import Counter
import hashlib
import itertools
import json
from pathlib import Path
import struct
import subprocess
import tempfile
from drawing_census import ROOT, BUILD_SHA256, instructions


def run(executable, claims):
    declaration = json.loads(claims.read_text()); sources = declaration['sources']
    def source_hashes():
        return {p: hashlib.sha256((ROOT / p).read_bytes()).hexdigest() for p in sources}
    if source_hashes() != sources: raise ValueError('Prospective sources changed')
    if hashlib.sha256(executable.read_bytes()).hexdigest() != BUILD_SHA256: raise ValueError('Wrong original input')
    inventory = json.loads((ROOT / 'research/runtime/coverage/binary-nocd.json').read_text()); contracts = []
    for address, cleanup in {0x58b660: 0, 0x484c00: 4, 0x4a2f90: 0}.items():
        f = next(f for f in inventory['functions'] if f['entry'] == address); returns = []
        for start, end in f['ranges']:
            assembly = subprocess.check_output(['objdump', '-d', '-Mintel', '--start-address=' + hex(start), '--stop-address=' + hex(end), str(executable)], text=True)
            for row in instructions(assembly).values():
                if row['mnemonic'] == 'ret':
                    raw = bytes.fromhex(row['bytes']); pop = 0 if raw == b'\xc3' else struct.unpack('<H', raw[1:])[0] if len(raw) == 3 and raw[0] == 0xc2 else None
                    if pop != cleanup: raise ValueError('Original RET mismatch')
                    returns.append(row)
        if not returns: raise ValueError('Missing RET')
        contracts.append({'address': address, 'pop': cleanup, 'returns': returns})
    imports = [x for x in inventory['imports'] if x['iat_va'] == 0x5c5208]
    if len(imports) != 1 or imports[0]['name'] != 'DestroyWindow': raise ValueError('Wrong teardown import')
    parent = ROOT / 'working/tests/drawing-lock-recovery'; parent.mkdir(parents=True, exist_ok=True)
    out = Path(tempfile.mkdtemp(prefix='run-', dir=parent)); print(out, flush=True)
    with (out / 'build.log').open('x') as log:
        subprocess.run(['g++', '-m32', '-O2', '-std=c++17', '-fno-pie', '-no-pie', '-mno-sse', '-mno-mmx', '-Wall', '-Wextra', '-Werror', str(ROOT / 'tests/drawing-lock-recovery-reference.cpp'), '-o', str(out / 'reference')], stdout=log, stderr=subprocess.STDOUT, check=True, timeout=90)
    with (out / 'cases.jsonl').open('x') as output, (out / 'execution.log').open('x') as log:
        process = subprocess.run([str(out / 'reference'), str(executable)], stdout=output, stderr=log, timeout=90)
    (out / 'execution-status.json').write_text(json.dumps({'return_code': process.returncode}, indent=2) + '\n')
    if process.returncode: raise ValueError('Original Lock fixture failed: ' + str(out))
    axes = {'present': {0, 1}, 'initial': {0, 1, 0x887601c2, 0x80004005}, 'restored': {0, 1, 0x80004005}, 'retry': {0, 1, 0x887601c2, 0x80004005}, 'pointer': {0, 1}, 'colour': {0, 1}, 'reload': {0, 1, 2}, 'teardown': {0, 1, 2}, 'release_early': {0, 1}, 'key_result': {0, 0x80004005}}
    seen = set(); totals = Counter(); divergent = returned_after_failure = 0
    with (out / 'cases.jsonl').open() as source:
        for line in source:
            row = json.loads(line); key = tuple(row[k] for k in axes)
            if any(type(row[k]) is not int or row[k] not in v for k, v in axes.items()) or key in seen: raise ValueError('Unknown/duplicated matrix cell')
            seen.add(key); totals.update(row['tags'])
            saved = 0xdeadface if not row['present'] else row['restored'] if row['initial'] == 0x887601c2 else row['initial']
            if row['stored_result'] != saved: raise ValueError('Unexpected stored result')
            if row['present'] and row['initial'] == 0x887601c2 and saved != row['retry']: divergent += 1
            if row['present'] and saved and row['returned_nonnull']: returned_after_failure += 1
    if len(seen) != 13824 or len(seen) != len(list(itertools.product(*axes.values()))): raise ValueError('Incomplete matrix')
    if source_hashes() != sources or hashlib.sha256(executable.read_bytes()).hexdigest() != BUILD_SHA256: raise ValueError('Sources/input changed during execution')
    report = {'schema': 1, 'success': True, 'scope': 'Unchanged original58b660 Lock wrapper client only: owned mock interfaces,13824unique selected result/callback/key/teardown/early-Unlock cells, descriptor zeroing/size and pitch/pointer writes, absent surface, exact lost code, Restore result retained separately from retryLock result, low16 key, zero/callback/sentinel endpoints, ordered teardown globals and DestroyWindow, optional auxiliary Unlock. External API/key/release/display/UI/sentinel bodies are scripted; list is empty. No real COM/driver recovery, actual pixels/font/asset reload, populated destructor list, reentrancy/null mutation, native equivalence or full scenario/dispatch completion.', 'sources': sources, 'claims': declaration['claims'], 'sources_stable': True, 'original_sha256': BUILD_SHA256, 'input_unchanged': True, 'original_wrapper_instructions_unchanged': True, 'original_cases': len(seen), 'unique_matrix_cells': len(seen), 'matrix_axes': {k: sorted(v) for k, v in axes.items()}, 'calls_by_tag': dict(totals), 'stored_result_differs_from_retry_result': divergent, 'nonnull_return_with_nonzero_stored_result': returned_after_failure, 'original_return_contracts': contracts, 'destroy_window_import': imports[0], 'artifacts': {str(p.relative_to(ROOT)): {'sha256': hashlib.sha256(p.read_bytes()).hexdigest(), 'bytes': p.stat().st_size} for p in [out / 'cases.jsonl', out / 'reference', out / 'build.log', out / 'execution.log', out / 'execution-status.json']}}
    (out / 'report.json').write_text(json.dumps(report, indent=2) + '\n'); print(json.dumps({'output': str(out), 'cases': len(seen), 'calls': dict(totals)}), flush=True)
    return report


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__); parser.add_argument('--executable', type=Path, required=True); parser.add_argument('--claims', type=Path, required=True); args = parser.parse_args()
    subprocess.run([str(ROOT / 'tools/original-manifest.sh'), 'verify'], check=True)
    try: run(args.executable, args.claims)
    finally: subprocess.run([str(ROOT / 'tools/original-manifest.sh'), 'verify'], check=True)
