#!/usr/bin/env python3
"""Compare bounded effect placement with the original common initializer."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import runpy
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
HASH = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'

def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--source-root', type=Path, default=ROOT)
    parser.add_argument('--build', type=Path, required=True)
    parser.add_argument('--sanitized-build', type=Path, required=True)
    args = parser.parse_args()
    source = args.source_root.resolve()
    parent = ROOT / 'working/tests/effect-placement'
    parent.mkdir(parents=True, exist_ok=True)
    out = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    print(out, flush=True)
    def run(command, name, env=None, timeout=180):
        result = subprocess.run(list(map(str, command)), capture_output=True, text=True, timeout=timeout, env=env)
        (out / (name + '.log')).write_text(result.stdout + result.stderr)
        result.check_returncode()
        return result.stdout
    run([ROOT / 'tools/original-manifest.sh', 'verify'], 'original-before')
    try:
        pe = ROOT / 'working/game-nocd/Chaos.exe'
        if sha(pe) != HASH:
            raise ValueError('Executable hash mismatch')
        model = source / 'reconstruction/rendering'
        table_runner = runpy.run_path(str(source / 'tools/test-effect-lighting.py'))
        paths = sorted(set(table_runner['PATHS'] + [
            'reconstruction/rendering/effect_placement.cpp', 'reconstruction/rendering/effect_placement.hpp',
            'tests/effect-placement-reference.cpp', 'tests/effect-placement-test.cpp',
            'tools/test-effect-placement.py']))
        before = {p: sha(source / p) for p in paths}
        common = ['g++', '-std=c++17', '-I' + str(model)]
        code = [source / 'tests/effect-placement-reference.cpp', model / 'effect_placement.cpp', model / 'effect_lighting.cpp']
        sanitize = ['-fsanitize=address,undefined', '-fno-omit-frame-pointer', '-fno-pie', '-no-pie']
        env = dict(os.environ, ASAN_OPTIONS='detect_leaks=0', UBSAN_OPTIONS='halt_on_error=1', QT_QPA_PLATFORM='xcb', LIBGL_ALWAYS_SOFTWARE='1')
        results = []
        helpers = {}
        for name, flags in [('original', ['-O2', '-m32', '-fno-pie', '-no-pie', '-DORIGINAL_REFERENCE']),
                            ('native', ['-O2']), ('sanitized', ['-O1', '-g', *sanitize])]:
            binary = out / name
            run([*common, *flags, *code, '-o', binary], 'compile-' + name)
            output = out / (name + '.bin')
            stats = json.loads(run([binary, *([pe] if name == 'original' else []), output], name, env))
            results.append((stats, sha(output)))
            helpers[name] = sha(binary)
        if not results[0] == results[1] == results[2]:
            raise ValueError('Original/native/sanitized placement states differ')
        for name, flags in [('unit', ['-O0', '-g']), ('unit-sanitized', ['-O1', '-g', *sanitize])]:
            binary = out / name
            run([*common, *flags, '-Wall', '-Wextra', '-Werror', '-pedantic',
                 source / 'tests/effect-placement-test.cpp', *code[1:], '-o', binary], 'compile-' + name)
            run([binary], name, env)
        # Existing table proof is repeated because its build metadata changed;
        # this also runs the now-expanded complete suites once per build.
        table_output = run(['python3', ROOT / 'tools/test-effect-lighting.py', '--source-root', source,
                            '--build', args.build.resolve(), '--sanitized-build', args.sanitized_build.resolve()], 'table-comparison', env, 360)
        table_path = Path(table_output.splitlines()[0]) / 'report.json'
        table = json.loads(table_path.read_text())
        if any(sha(source / p) != h for p, h in before.items()):
            raise ValueError('Sources changed during comparison')
        if sha(pe) != HASH:
            raise ValueError('Executable changed during comparison')
        report = dict(all_match=True, live_validated=False, executable_sha256=HASH,
                      **results[0][0], output_sha256=results[0][1], helper_sha256=helpers,
                      registered_source_sha256=before, full_suites=table['full_suites'],
                      effect_table_report=str(table_path.relative_to(ROOT)), effect_table_report_sha256=sha(table_path),
                      boundary='Unmodified 00493f20 common creation path for types 3/13/22/24/36 with no creator, logging disabled and preallocated owned grid/records; only 00494ab0 type setup redirected to checked type assignment callback in private memory; no original type-specific animation, allocator, movement, height-search type 21 or live replacement')
        (out / 'report.json').write_text(json.dumps(report, indent=2) + '\n')
        print(json.dumps({k: v for k, v in report.items() if not isinstance(v, dict)}, indent=2), flush=True)
    finally:
        run([ROOT / 'tools/original-manifest.sh', 'verify'], 'original-after')

if __name__ == '__main__':
    main()
