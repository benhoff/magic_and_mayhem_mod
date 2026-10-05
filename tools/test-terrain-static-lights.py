#!/usr/bin/env python3
"""Bounded original static-light admission/stamping/refresh vs 32/64-bit owned native cycles."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess
import tempfile
ROOT = Path(__file__).resolve().parents[1]
HASH = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()
def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--source-root', type=Path, default=ROOT)
    args = parser.parse_args()
    source = args.source_root.resolve()
    parent = ROOT / 'working/tests/terrain-static-lights'
    parent.mkdir(parents=True, exist_ok=True)
    out = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    print(out, flush=True)
    def run(command, name, env=None):
        result = subprocess.run(list(map(str, command)), capture_output=True,
                                text=True, timeout=120, env=env)
        (out / (name + '.log')).write_text(result.stdout + result.stderr)
        result.check_returncode()
        return result.stdout
    run([ROOT / 'tools/original-manifest.sh', 'verify'], 'original-before')
    try:
        pe = ROOT / 'working/game-nocd/Chaos.exe'
        if sha(pe) != HASH:
            raise ValueError('Executable hash mismatch')
        inputs = [pe, Path(__file__), *[source / p for p in [
            'tests/sprite-binary-reference.cpp',
            'tests/terrain-static-lighting-reference.cpp',
            'tests/terrain-static-lighting-test.cpp',
            'reconstruction/rendering/terrain_lighting.cpp',
            'reconstruction/rendering/terrain_lighting.hpp',
            'reconstruction/rendering/terrain_creature_lighting.cpp',
            'reconstruction/rendering/terrain_creature_lighting.hpp',
            'reconstruction/rendering/terrain_static_lighting.cpp',
            'reconstruction/rendering/terrain_static_lighting.hpp']]]
        before = {p: sha(p) for p in inputs}
        model = source / 'reconstruction/rendering'
        common = ['g++', '-std=c++17', '-I' + str(model)]
        sources = [source / 'tests/terrain-static-lighting-reference.cpp',
                   model / 'terrain_lighting.cpp', model / 'terrain_creature_lighting.cpp',
                   model / 'terrain_static_lighting.cpp']
        sanitizer = ['-fsanitize=address,undefined', '-fno-omit-frame-pointer', '-fno-pie', '-no-pie']
        env = dict(os.environ, ASAN_OPTIONS='detect_leaks=0', UBSAN_OPTIONS='halt_on_error=1')
        results = []
        helpers = {}
        for name, flags in [('original', ['-O2', '-m32', '-fno-pie', '-no-pie', '-DORIGINAL_REFERENCE']),
                            ('native', ['-O2']), ('sanitized', ['-O1', '-g', *sanitizer])]:
            binary = out / name
            run([*common, *flags, *sources, '-o', binary], 'compile-' + name)
            stats = json.loads(run([binary, *([pe] if name == 'original' else []), out / (name + '.bin')], name, env))
            results.append((stats, sha(out / (name + '.bin'))))
            helpers[name] = sha(binary)
        if not results[0] == results[1] == results[2]:
            raise ValueError('32-bit original/native, 64-bit native, sanitized output mismatch')
        for name, flags in [('unit', ['-O0', '-g']), ('unit-sanitized', ['-O1', '-g', *sanitizer])]:
            binary = out / name
            run([*common, *flags, source / 'tests/terrain-static-lighting-test.cpp',
                 *sources[1:], '-o', binary], 'compile-' + name)
            run([binary], name, env)
            helpers[name] = sha(binary)
        if any(sha(p) != digest for p, digest in before.items()):
            raise ValueError('Inputs changed during experiment')
        report = dict(all_match=True, live_validated=False, executable_sha256=HASH,
                      **results[0][0], compared_builds=['original and native 32-bit', 'native 64-bit', 'native 64-bit ASan/UBSan'],
                      output_sha256=results[0][1], helper_sha256=helpers,
                      source_and_input_sha256={str(p.relative_to(ROOT)): digest for p, digest in before.items()},
                      boundary='Unmodified 004ff700/004ff820 admission, 0049cf60 light-index selection, 004f1c60 additive stamp and 004f2ad0 two-phase updater; 64 combined 004f27a0 creature/static fixtures with preallocated guarded fields; bounded indices/coordinates and owned records; no captured entity inputs, weighted interpolation or live replacement')
        (out / 'report.json').write_text(json.dumps(report, indent=2) + '\n')
        print(json.dumps(report, indent=2), flush=True)
    finally:
        run([ROOT / 'tools/original-manifest.sh', 'verify'], 'original-after')
if __name__ == '__main__':
    main()
