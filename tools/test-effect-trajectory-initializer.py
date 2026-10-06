#!/usr/bin/env python3
"""Execute unmodified 004df3a0 against owned native trajectory initialization and chained stepping."""
import hashlib
import json
import os
from pathlib import Path
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
HASH = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
PATHS = ['reconstruction/rendering/effect_trajectory.cpp',
         'reconstruction/rendering/effect_trajectory.hpp',
         'reconstruction/rendering/effect_trajectory_initializer.cpp', 'reconstruction/rendering/effect_trajectory_initializer.hpp',
         'reconstruction/rendering/effect-trajectory-initializer/CMakeLists.txt',
         'tests/effect-trajectory-initializer-test.cpp', 'tests/effect-trajectory-initializer-reference.cpp',
         'tests/sprite-binary-reference.cpp', 'tools/test-effect-trajectory-initializer.py']

def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()

def main():
    parent = ROOT / 'working/tests/effect-trajectory-initializer'
    parent.mkdir(parents=True, exist_ok=True)
    out = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    print(out, flush=True)
    def run(command, name, env=None):
        result = subprocess.run(list(map(str, command)), capture_output=True, text=True,
                                timeout=180, env=env)
        (out / (name + '.log')).write_text(result.stdout + result.stderr)
        result.check_returncode()
        return result.stdout
    run([ROOT / 'tools/original-manifest.sh', 'verify'], 'original-before')
    try:
        pe = ROOT / 'working/game-nocd/Chaos.exe'
        if sha(pe) != HASH:
            raise ValueError('Executable hash mismatch')
        sources = {p: sha(ROOT / p) for p in PATHS}
        code = [ROOT / 'tests/effect-trajectory-initializer-reference.cpp',
                ROOT / 'reconstruction/rendering/effect_trajectory.cpp', ROOT / 'reconstruction/rendering/effect_trajectory_initializer.cpp']
        common = ['g++', '-std=c++17', '-I' + str(ROOT / 'reconstruction/rendering')]
        sanitize = ['-fsanitize=address,undefined', '-fno-omit-frame-pointer', '-fno-pie', '-no-pie']
        env = dict(os.environ, ASAN_OPTIONS='detect_leaks=0', UBSAN_OPTIONS='halt_on_error=1')
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
            raise ValueError('Original/native/sanitized states differ')
        for name, flags in [('unit', ['-O0', '-g']), ('unit-sanitized', ['-O1', '-g', *sanitize])]:
            binary = out / name
            run([*common, *flags, '-Wall', '-Wextra', '-Werror', '-pedantic',
                 ROOT / 'tests/effect-trajectory-initializer-test.cpp', *code[1:], '-o', binary], 'compile-' + name)
            run([binary], name, env)
        build = out / 'cmake'
        run(['cmake', '-S', ROOT / 'reconstruction/rendering/effect-trajectory-initializer', '-B', build], 'configure')
        run(['cmake', '--build', build, '-j2'], 'build')
        run(['ctest', '--test-dir', build, '--output-on-failure'], 'ctest')
        if any(sha(ROOT / p) != h for p, h in sources.items()) or sha(pe) != HASH:
            raise ValueError('Inputs changed during comparison')
        report = dict(all_match=True, live_validated=False, executable_sha256=HASH,
                      **results[0][0], output_sha256=results[0][1], helper_sha256=helpers,
                      registered_source_sha256=sources, unit_passed=True, sanitized_unit_passed=True,
                      cmake_tests_passed=1,
                      boundary='Whole unmodified 004df3a0 initializer including original 004df260, authored coordinates/periods/multiplier and retained state word3; guarded complete state/counter plus 32 chained 004df500 steps; no effect creation parent, installed inputs or live replacement')
        (out / 'report.json').write_text(json.dumps(report, indent=2) + '\n')
        print(json.dumps(report, indent=2), flush=True)
    finally:
        run([ROOT / 'tools/original-manifest.sh', 'verify'], 'original-after')

if __name__ == '__main__':
    main()
