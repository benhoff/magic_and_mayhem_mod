#!/usr/bin/env python3
"""Execute bounded unchanged original timer cursor type/outer-wait gates."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
BUILD = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--executable', type=Path, required=True)
    parser.add_argument('--claims', type=Path, required=True)
    parser.add_argument('--report', type=Path, required=True)
    args = parser.parse_args()
    if args.report.exists():
        parser.error('Refusing evidence overwrite')
    subprocess.run([str(ROOT / 'tools/original-manifest.sh'), 'verify'], check=True)
    try:
        declaration = json.loads(args.claims.read_text())
        sources = declaration['sources']
        if {name: sha(ROOT / name) for name in sources} != sources or sha(args.executable) != BUILD:
            raise ValueError('Declared sources/original differ')
        parent = ROOT / 'working/tests/drawing-cursor-waits'
        parent.mkdir(parents=True, exist_ok=True)
        output = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
        print(output, flush=True)
        with (output / 'build.log').open('x') as log:
            subprocess.run(['g++', '-m32', '-O2', '-std=c++17', '-fno-pie', '-no-pie', '-mno-sse', '-mno-mmx', '-Wall', '-Wextra', '-Werror', str(ROOT / 'tests/drawing-cursor-wait-reference.cpp'), '-o', str(output / 'reference')], stdout=log, stderr=subprocess.STDOUT, check=True, timeout=90)
        with (output / 'cases.json').open('x') as rows, (output / 'execution.log').open('x') as log:
            result = subprocess.run([str(output / 'reference'), str(args.executable)], stdout=rows, stderr=log, timeout=90)
        (output / 'execution-status.json').write_text(json.dumps(dict(return_code=result.returncode)) + '\n')
        if result.returncode:
            raise ValueError('Original cursor fixture refused; retained at ' + str(output))
        matrix = json.loads((output / 'cases.json').read_text())
        if matrix != dict(success=True, cases=240, wait_calls=96, release_calls=24):
            raise ValueError('Original gate matrix differs')
        if {name: sha(ROOT / name) for name in sources} != sources or sha(args.executable) != BUILD:
            raise ValueError('Sources/original changed during execution')
        artifacts = {str(p.relative_to(ROOT)): dict(sha256=sha(p), bytes=p.stat().st_size) for p in sorted(output.iterdir()) if p.is_file()}
        report = dict(schema=1, success=True, sources=sources, claims=declaration['claims'], sources_stable=True, original_sha256=BUILD, input_unchanged=True, original_timer_instructions_unchanged=True, original_game_executed=False, matrix=matrix, artifacts=artifacts, scope='Unchanged privately mapped original4a4910 timer gate over240type/screen/outer-Wait result/countdown/Release result cells. Five type values include signednegative and22/23out-of-range; four raw Wait responses0/80/102/FFFFFFFF; screen0/1, countdown0/1/2, Release0/1. Owned fake Wait/Release imports only, inner cursor enabled flag fixedzero. Checks exact wait10args, nonzero-result skip/noRelease, acquired earlyRelease, conditional countdown decrement and unchanged code/unrelated globals. No cursor coordinate/drawing/nested-wait execution, real OS mutex ownership/lifetime, abandoned semantics, complete lock order or safe source protection/native parity.')
        with args.report.open('x') as out:
            json.dump(report, out, indent=2)
            out.write('\n')
        print(json.dumps(matrix))
    finally:
        subprocess.run([str(ROOT / 'tools/original-manifest.sh'), 'verify'], check=True)


if __name__ == '__main__':
    main()
