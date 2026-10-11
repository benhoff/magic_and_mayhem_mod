#!/usr/bin/env python3
"""Execute unchanged pinned movie bodies with explicit synthetic COM contracts."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import tempfile
from drawing_census import ROOT, BUILD_SHA256


def run(executable, claims=None):
    if hashlib.sha256(executable.read_bytes()).hexdigest() != BUILD_SHA256:
        raise ValueError('Unsupported original movie executable')
    paths = ['tools/test-drawing-movie.py', 'tests/drawing-movie-reference.cpp',
             'tests/sprite-binary-reference.cpp', 'tools/drawing_census.py',
             'research/runtime/coverage/binary-nocd.json']
    sources = {p: hashlib.sha256((ROOT / p).read_bytes()).hexdigest() for p in paths}
    declaration = json.loads(claims.read_text()) if claims else None
    if declaration:
        for p, digest in declaration['sources'].items():
            if hashlib.sha256((ROOT / p).read_bytes()).hexdigest() != digest:
                raise ValueError('Changed declaration source: ' + p)
            sources[p] = digest
    parent = ROOT / 'working/tests/drawing-movie'; parent.mkdir(parents=True, exist_ok=True)
    out = Path(tempfile.mkdtemp(prefix='run-', dir=parent)); print(out, flush=True)
    with (out / 'build.log').open('x') as log:
        subprocess.run(['g++', '-m32', '-O2', '-std=c++17', '-fno-pie', '-no-pie',
            '-mno-sse', '-mno-mmx', '-Wall', '-Wextra', '-Werror',
            str(ROOT / 'tests/drawing-movie-reference.cpp'), '-o', str(out / 'reference')],
            stdout=log, stderr=subprocess.STDOUT, check=True, timeout=90)
    with (out / 'cases.jsonl').open('x') as output, (out / 'execution.log').open('x') as log:
        subprocess.run([str(out / 'reference'), str(executable)], stdout=output, stderr=log,
                       check=True, timeout=90)
    cases = [json.loads(line) for line in (out / 'cases.jsonl').read_text().splitlines()]
    if len(cases) != 1760 or {r['failure_stage'] for r in cases} != set(range(-1, 10)):
        raise ValueError('Incomplete original movie case matrix')
    if sources != {p: hashlib.sha256((ROOT / p).read_bytes()).hexdigest() for p in sources}:
        raise ValueError('Sources changed during original movie execution')
    if hashlib.sha256(executable.read_bytes()).hexdigest() != BUILD_SHA256:
        raise ValueError('Original executable changed during execution')
    report = {'schema': 1, 'success': True, 'sources': sources, 'sources_stable': True,
        'original_sha256': BUILD_SHA256, 'input_unchanged': True,
        'original_movie_instructions_unchanged': True, 'original_cases': len(cases),
        'body_addresses': [0x4696e0, 0x469830, 0x469a00],
        'external_bindings': {'logger': 0x59ca38, 'creation_iat': 0x5c5318,
            'wide_conversion_iat': 0x5c5088, 'key_iat': 0x5c5204},
        'artifacts': {str(p.relative_to(ROOT)): {'sha256': hashlib.sha256(p.read_bytes()).hexdigest(),
            'bytes': p.stat().st_size} for p in [out / 'cases.jsonl', out / 'reference']},
        'scope': 'Unchanged original stream setup/sample/wrapper: selected negative HRESULT at each of ten stages, previous skip0/1, signed controls0/1/-1/127/-128, Escape0/1, Update/Blt E_FAIL0/1 and ordinary/wrapping positions. Check exact external arguments, short-circuit, placement, metadata, return policy, reference balance and original release order. EDX supplies distinct synthetic DirectDraw device; first stack word supplies destination surface. Mock APIs/COM only: original codecs/audio/pixels, Windows implementation, conversion failures, positive HRESULT variants, arbitrary COM outputs, exceptions, backend internal retention and full live playback excluded.'}
    if declaration: report['claims'] = declaration['claims']
    (out / 'report.json').write_text(json.dumps(report, indent=2) + '\n')
    print(json.dumps({'output': str(out), 'cases': len(cases)}), flush=True)
    return report


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--executable', type=Path, required=True)
    parser.add_argument('--claims', type=Path)
    args = parser.parse_args()
    subprocess.run([str(ROOT / 'tools/original-manifest.sh'), 'verify'], check=True)
    try: run(args.executable.resolve(), args.claims)
    finally: subprocess.run([str(ROOT / 'tools/original-manifest.sh'), 'verify'], check=True)
