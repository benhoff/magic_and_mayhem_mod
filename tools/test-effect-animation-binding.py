#!/usr/bin/env python3
"""Compare effect ANI binding with whole original forward bind/start/tick routines."""
import hashlib
import json
import os
from pathlib import Path
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
HASH = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
PATHS = ['reconstruction/rendering/effect_animation_binding.cpp',
         'reconstruction/rendering/effect_animation_binding.hpp',
         'reconstruction/rendering/effect-animation-binding/CMakeLists.txt',
         'tests/effect-animation-binding-test.cpp', 'tests/effect-animation-binding-reference.cpp',
         'reconstruction/animation/no_cd.cpp', 'reconstruction/animation/no_cd.hpp',
         'assets/animation.hpp', 'assets/asset_file.hpp',
         'reconstruction/rendering/effect_animation_selection.cpp', 'reconstruction/rendering/effect_animation_selection.hpp',
         'tests/sprite-binary-reference.cpp', 'tools/test-effect-animation-binding.py']

def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()

def main():
    parent = ROOT / 'working/tests/effect-animation-binding'
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
        code = [ROOT / 'tests/effect-animation-binding-reference.cpp',
                ROOT / 'reconstruction/rendering/effect_animation_binding.cpp', ROOT / 'reconstruction/animation/no_cd.cpp', ROOT / 'reconstruction/rendering/effect_animation_selection.cpp']
        common = ['g++', '-std=c++17', '-I' + str(ROOT / 'reconstruction/rendering'), '-I' + str(ROOT / 'reconstruction/animation'), '-I' + str(ROOT / 'assets')]
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
                 ROOT / 'tests/effect-animation-binding-test.cpp', *code[1:], '-o', binary], 'compile-' + name)
            run([binary], name, env)
        build = out / 'cmake'
        run(['cmake', '-S', ROOT / 'reconstruction/rendering/effect-animation-binding', '-B', build], 'configure')
        run(['cmake', '--build', build, '-j2'], 'build')
        run(['ctest', '--test-dir', build, '--output-on-failure'], 'ctest')
        if any(sha(ROOT / p) != h for p, h in sources.items()) or sha(pe) != HASH:
            raise ValueError('Inputs changed during comparison')
        report = dict(all_match=True, live_validated=False, executable_sha256=HASH,
                      **results[0][0], output_sha256=results[0][1], helper_sha256=helpers,
                      registered_source_sha256=sources, unit_passed=True, sanitized_unit_passed=True,
                      cmake_tests_passed=1,
                      boundary='Owned effect metadata/sequence adapter with static 00494be3..00494c26 resolution; whole original 00464ca0/00464cb0 forward bind/start and 00464ec0 ticks on authored normalized ANI arrays; complete modeled controller state, preserved guards/reverse counters and immutable input tables; caller binding window/full creation dispatch, installed assets/metadata, reverse playback and live replacement remain separate')
        (out / 'report.json').write_text(json.dumps(report, indent=2) + '\n')
        print(json.dumps(report, indent=2), flush=True)
    finally:
        run([ROOT / 'tools/original-manifest.sh', 'verify'], 'original-after')

if __name__ == '__main__':
    main()
