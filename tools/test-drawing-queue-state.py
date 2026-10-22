#!/usr/bin/env python3
"""Execute pinned original queue state/clip/postlude branches with synthetic backends."""
from coverage_evidence import read_bytes as evidence_bytes, read_text as evidence_text
import argparse
from collections import Counter
import hashlib
import gzip
import importlib.util
import json
from pathlib import Path
import struct
import subprocess
import tempfile
import threading
from drawing_census import ROOT, BUILD_SHA256, instructions


def run(executable, claims):
    if hashlib.sha256(evidence_bytes(executable)).hexdigest() != BUILD_SHA256:
        raise ValueError('Unsupported original consumer input')
    declaration = json.loads(evidence_text(claims))
    sources = declaration['sources']
    if any(hashlib.sha256(evidence_bytes(ROOT / p)).hexdigest() != h for p, h in sources.items()):
        raise ValueError('Prospective source changed before execution')
    spec = importlib.util.spec_from_file_location('dispatch_fixture', ROOT / 'tools/test-drawing-dispatch.py')
    module = importlib.util.module_from_spec(spec); spec.loader.exec_module(module)
    inventory = json.loads(evidence_text(ROOT / 'research/runtime/coverage/binary-nocd.json'))
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
    raw_digest = hashlib.sha256(); raw_bytes = 0; expired = [False]
    with (out / 'cases.jsonl.gz').open('xb') as output, (out / 'execution.log').open('x') as log:
        process = subprocess.Popen([str(out / 'reference'), str(executable)], stdout=subprocess.PIPE, stderr=log)
        def expire():
            if process.poll() is None: expired[0] = True; process.kill()
        timer = threading.Timer(180, expire); timer.daemon = True; timer.start()
        try:
            with gzip.GzipFile(fileobj=output, mode='wb', compresslevel=1, mtime=0) as archive:
                while True:
                    chunk = process.stdout.read(1024 * 1024)
                    if not chunk: break
                    archive.write(chunk); raw_digest.update(chunk); raw_bytes += len(chunk)
            process.wait(timeout=5)
        finally:
            timer.cancel(); process.stdout.close()
            if process.poll() is None: process.kill(); process.wait(timeout=5)
    (out / 'execution-status.json').write_text(json.dumps({'return_code': process.returncode,
        'timed_out': expired[0], 'raw_case_sha256': raw_digest.hexdigest(), 'raw_case_bytes': raw_bytes,
        'binary_sha256': hashlib.sha256(evidence_bytes(out / 'reference')).hexdigest()}, indent=2) + '\n')
    if process.returncode or expired[0]: raise ValueError('Original state fixture exit ' + str(process.returncode) + '; retained attempt: ' + str(out))
    totals, kinds, returns_seen, count = Counter(), Counter(), Counter(), 0
    axes = {'indexed': {0, 1}, 'offset': {-127, 3}, 'video': {0, 1}, 'count': {0, 1, 2}, 'postlude': {0, 1}, 'kind': set(range(-2, 36)),
        'y': {-20, -12, 0, 1, 100, 468, 480, 588, 600},
        'first': {0x8ad0, 0xfff3, 0xfff4, 0xfff5, 0, 5},
        'prior': {0x8ad0, 0, 1, 0x1234, 0xffff}, 'height_flag': {0, 1},
        'result': {0, 1, 0x12340001, 0x12345678, 0xffffffff}}
    seen = set()
    decoded_digest = hashlib.sha256(); decoded_bytes = 0
    with gzip.open(out / 'cases.jsonl.gz', 'rb') as source:
        for line in source:
            decoded_digest.update(line); decoded_bytes += len(line)
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
    if decoded_digest.hexdigest() != raw_digest.hexdigest() or decoded_bytes != raw_bytes:
        raise ValueError('Compressed case archive differs from original output')
    if count != 4924800 or kinds != {k: 129600 for k in range(-2, 36)}:
        raise ValueError('Incomplete original state matrix')
    if hashlib.sha256(evidence_bytes(executable)).hexdigest() != BUILD_SHA256:
        raise ValueError('Original input changed during execution')
    if any(hashlib.sha256(evidence_bytes(ROOT / p)).hexdigest() != h for p, h in sources.items()):
        raise ValueError('Sources changed during execution')
    report = {'schema': 1, 'success': True, 'scope': 'Unchanged pinned consumer with synthetic backend/clip/cleanup bindings:4924800unique empty/one/two row cells, all kinds-2..35, Video0/1, direct/indexed object markers, ordinary/-127 offsets and AX1/high-EAX=12340001 distinctions, sentinel and changed prior metadata, signed spans, strict clip and480/600height branches, raw0/1/high/negative backend results, postlude0/1. Original row/container and counter mutations and exact opaque CALL arguments are checked. Actual helper side effects, pixels, external objects/graphics and full dispatch/scenario closure remain outside this fixture.',
        'sources': sources, 'claims': declaration['claims'], 'sources_stable': True,
        'original_sha256': BUILD_SHA256, 'input_unchanged': True,
        'original_consumer_instructions_unchanged': True, 'original_cases': count,
        'unique_matrix_cells': len(seen), 'matrix_axes': {k: sorted(v) for k, v in axes.items()},
        'case_archive': {'compression': 'gzip', 'raw_sha256': raw_digest.hexdigest(), 'raw_bytes': raw_bytes,
                         'decoded_sha256': decoded_digest.hexdigest(), 'decoded_bytes': decoded_bytes},
        'cases_by_kind': dict(kinds), 'calls_by_tag': dict(totals), 'calls_by_exact_return_pc': dict(returns_seen),
        'backend_return_contracts': contracts,
        'artifacts': {str(p.relative_to(ROOT)): {'sha256': hashlib.sha256(evidence_bytes(p)).hexdigest(), 'bytes': p.stat().st_size}
                      for p in [out / 'cases.jsonl.gz', out / 'reference', out / 'build.log', out / 'execution.log', out / 'execution-status.json']}}
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
