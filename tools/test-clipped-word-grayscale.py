#!/usr/bin/env python3
"""Compare pinned original 589c90 with authored channel and clipping expectations."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
BUILD = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'


def sha(data):
    return hashlib.sha256(data).hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--pe', type=Path, required=True)
    parser.add_argument('--claims', type=Path, required=True)
    parser.add_argument('--report', type=Path, required=True)
    args = parser.parse_args()
    if args.report.exists():
        parser.error('Refusing evidence overwrite')
    declaration = json.loads(args.claims.read_bytes())
    sources = declaration['sources']
    if sha(args.pe.read_bytes()) != BUILD or any(sha((ROOT / p).read_bytes()) != h for p, h in sources.items()):
        raise ValueError('Prospective sources or original PE changed')
    parent = ROOT / 'working/tests/clipped-word-grayscale'
    parent.mkdir(parents=True, exist_ok=True)
    run = Path(tempfile.mkdtemp(prefix='run-', dir=parent))

    def verify(label):
        result = subprocess.run([str(ROOT / 'tools/original-manifest.sh'), 'verify'], capture_output=True, timeout=120)
        (run / (label + '.log')).write_bytes(result.stdout + result.stderr)
        result.check_returncode()

    verify('original-before')
    execution = None
    try:
        binary = run / 'reference'
        build = subprocess.run(['g++', '-m32', '-std=c++17', '-O2', '-Wall', '-Wextra', '-Werror', '-pedantic',
                                str(ROOT / 'tests/clipped-word-grayscale-reference.cpp'), '-o', str(binary)], capture_output=True)
        (run / 'build.log').write_bytes(build.stdout + build.stderr)
        if build.returncode == 0:
            execution = subprocess.run([str(binary), str(args.pe.resolve()), str(run / 'pixels')], capture_output=True, timeout=60)
            (run / 'reference.log').write_bytes(execution.stdout + execution.stderr)
        if sha(args.pe.read_bytes()) != BUILD or any(sha((ROOT / p).read_bytes()) != h for p, h in sources.items()):
            raise ValueError('Sources or original PE changed during comparison')
    finally:
        verify('original-after')
    result = json.loads(execution.stdout) if execution is not None and execution.returncode == 0 else None
    success = result is not None and result['success'] and result['cases'] == 8 and result['full_source_words'] == 65536 and result['different_words'] == 0
    report = dict(schema=1, success=success, sources=sources, claims=declaration['claims'],
                  original_pe_sha256=BUILD, build_exit=build.returncode,
                  native_exit=execution.returncode if execution is not None else None,
                  result=result, artifacts=str(run.relative_to(ROOT)),
                  executable_sha256=sha(binary.read_bytes()) if binary.exists() else None,
                  original_manifest_verified_before_after=True, original_game_executed=False,
                  reference_process_launched=execution is not None,
                  original_body_executed=True if success else None, original_code_modified=False,
                  scope='Isolated unmodified original589c90 compared against authored unpacked-channel scalar expectations and clipped/transparency/origin/padding/guard conditions in eight private-memory cases, including all65536RGB565words. Full-width/vertical-only routes convert to gray; horizontally clipped routes retain colored words. Private fixture globals use the zero-format branch. No original output supplied to native renderer, Windows/session integration, RGB555 or indexed validation, full caller ABI or complete presentation claim.')
    with args.report.open('x') as output:
        json.dump(report, output, indent=2)
        output.write('\n')
    print(json.dumps(dict(success=success, build_exit=build.returncode, native_exit=report['native_exit'], result=result)))


if __name__ == '__main__':
    main()
