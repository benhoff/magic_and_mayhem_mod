#!/usr/bin/env python3
"""Execute whole unmodified 004883f0 against bounded owned empty-world cell transitions."""
import hashlib
import json
import os
from pathlib import Path
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
HASH = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
PATHS = ['reconstruction/rendering/effect_transition.cpp', 'reconstruction/rendering/effect_transition.hpp',
         'reconstruction/rendering/effect-transition/CMakeLists.txt','reconstruction/rendering/effect_projection.cpp', 'reconstruction/rendering/effect_projection.hpp',
         'reconstruction/rendering/effect-projection/CMakeLists.txt',
         'reconstruction/rendering/effect_placement.cpp', 'reconstruction/rendering/effect_placement.hpp',
         'reconstruction/rendering/effect_lighting.cpp', 'reconstruction/rendering/effect_lighting.hpp',
         'reconstruction/rendering/terrain_lighting_cycle.cpp', 'reconstruction/rendering/terrain_lighting_cycle.hpp',
         'reconstruction/rendering/terrain_static_lighting.cpp', 'reconstruction/rendering/terrain_static_lighting.hpp',
         'reconstruction/rendering/terrain_creature_lighting.cpp', 'reconstruction/rendering/terrain_creature_lighting.hpp',
         'reconstruction/rendering/terrain_lighting.cpp', 'reconstruction/rendering/terrain_lighting.hpp',
         'reconstruction/rendering/effect_trajectory.cpp',
         'reconstruction/rendering/effect_trajectory.hpp',
         'tests/effect-transition-test.cpp', 'tests/effect-transition-reference.cpp',
         'tests/sprite-binary-reference.cpp', 'tools/test-effect-transition.py']

def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()

def main():
    parent = ROOT / 'working/tests/effect-transition'
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
        code = [ROOT / 'tests/effect-transition-reference.cpp',
                *[ROOT / ('reconstruction/rendering/' + n + '.cpp') for n in
                  ['effect_transition', 'effect_projection', 'effect_trajectory', 'effect_placement', 'effect_lighting']]]
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
                 ROOT / 'tests/effect-transition-test.cpp', *code[1:],
                 *[ROOT / ('reconstruction/rendering/' + n + '.cpp') for n in
                   ['terrain_lighting_cycle', 'terrain_static_lighting', 'terrain_creature_lighting', 'terrain_lighting']],
                 '-o', binary], 'compile-' + name)
            run([binary], name, env)
        build = out / 'cmake'
        run(['cmake', '-S', ROOT / 'reconstruction/rendering/effect-transition', '-B', build], 'configure')
        run(['cmake', '--build', build, '-j2'], 'build')
        run(['ctest', '--test-dir', build, '--output-on-failure'], 'ctest')
        if any(sha(ROOT / p) != h for p, h in sources.items()) or sha(pe) != HASH:
            raise ValueError('Inputs changed during comparison')
        report = dict(all_match=True, live_validated=False, executable_sha256=HASH,
                      **results[0][0], output_sha256=results[0][1], helper_sha256=helpers,
                      registered_source_sha256=sources, unit_passed=True, sanitized_unit_passed=True,
                      cmake_tests_passed=1,
                      boundary='Whole unmodified 004883f0 with real 004df500/004e10e0, five emitting types, empty terrain occupancy/no creatures, metadata kind 0/68 with zero distance allowance, 0..8 iterations, membership-on old/new chains, fine/cell/previous/cache/subcell/recount updates, nonzero terrain catalog references and return 3; no patches or stubs; authored trajectory, no setup, zero-terrain cleanup callback, blocked destinations, occupied collisions, special termination or live replacement')
        (out / 'report.json').write_text(json.dumps(report, indent=2) + '\n')
        print(json.dumps(report, indent=2), flush=True)
    finally:
        run([ROOT / 'tools/original-manifest.sh', 'verify'], 'original-after')

if __name__ == '__main__':
    main()
