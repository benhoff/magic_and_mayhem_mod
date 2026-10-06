#!/usr/bin/env python3
"""Execute unmodified 004883f0 against owned native creature footprint lookup."""
import hashlib
import json
import os
from pathlib import Path
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
HASH = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
PATHS = ['reconstruction/rendering/effect_transition.cpp', 'reconstruction/rendering/effect_transition.hpp', 'reconstruction/rendering/effect_cleanup.cpp', 'reconstruction/rendering/effect_cleanup.hpp', 'reconstruction/rendering/effect_projection.cpp', 'reconstruction/rendering/effect_projection.hpp', 'reconstruction/rendering/effect_placement.cpp', 'reconstruction/rendering/effect_placement.hpp', 'reconstruction/rendering/effect_lighting.cpp', 'reconstruction/rendering/effect_lighting.hpp', 'reconstruction/rendering/terrain_lighting_cycle.cpp', 'reconstruction/rendering/terrain_lighting_cycle.hpp', 'reconstruction/rendering/terrain_static_lighting.cpp', 'reconstruction/rendering/terrain_static_lighting.hpp', 'reconstruction/rendering/terrain_creature_lighting.cpp', 'reconstruction/rendering/terrain_creature_lighting.hpp', 'reconstruction/rendering/terrain_lighting.cpp', 'reconstruction/rendering/terrain_lighting.hpp', 'reconstruction/rendering/effect_trajectory.cpp', 'reconstruction/rendering/effect_trajectory.hpp', 'tests/sprite-binary-reference.cpp', 'reconstruction/rendering/creature_occupancy.cpp', 'reconstruction/rendering/creature_occupancy.hpp', 'reconstruction/rendering/creature_footprint.cpp', 'reconstruction/rendering/creature_footprint.hpp', 'reconstruction/rendering/effect_creature_collision.cpp', 'reconstruction/rendering/effect_creature_collision.hpp', 'reconstruction/rendering/effect-creature/CMakeLists.txt', 'tests/effect-creature-boundaries-reference.cpp', 'tests/effect-creature-reference.cpp', 'tests/effect-kind34-reference.cpp', 'tests/effect-kind34-test.cpp', 'tests/effect-creature-test.cpp', 'tools/test-effect-kind34.py']

def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()

def main():
    parent = ROOT / 'working/tests/effect-kind34'
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
        code = [None, *[ROOT / p for p in PATHS if p.startswith('reconstruction/') and p.endswith('.cpp')]]
        common = ['g++', '-std=c++17', '-I' + str(ROOT / 'reconstruction/rendering')]
        sanitize = ['-fsanitize=address,undefined', '-fno-omit-frame-pointer', '-fno-pie', '-no-pie']
        env = dict(os.environ, ASAN_OPTIONS='detect_leaks=0', UBSAN_OPTIONS='halt_on_error=1')
        suites = {}; helpers = {}
        for suite, reference in [('baseline','effect-creature-reference.cpp'),('boundaries','effect-creature-boundaries-reference.cpp'),('kind34','effect-kind34-reference.cpp')]:
            results = []
            for name, flags in [('original', ['-O2', '-m32', '-fno-pie', '-no-pie', '-DORIGINAL_REFERENCE']),
                                ('native', ['-O2']), ('sanitized', ['-O1', '-g', *sanitize])]:
                label = suite + '-' + name
                binary = out / label
                run([*common, *flags, ROOT / ('tests/' + reference), *code[1:], '-o', binary], 'compile-' + label)
                output = out / (label + '.bin')
                stats = json.loads(run([binary, *([pe] if name == 'original' else []), output], label, env))
                results.append((stats, sha(output)));helpers[label] = sha(binary)
            if not results[0] == results[1] == results[2]:
                raise ValueError('Original/native/sanitized states differ: ' + suite)
            suites[suite] = dict(**results[0][0], output_sha256=results[0][1])
        for test in ['effect-creature-test.cpp', 'effect-kind34-test.cpp']:
            for name, flags in [('unit', ['-O0', '-g']), ('unit-sanitized', ['-O1', '-g', *sanitize])]:
                label = test[:-4] + '-' + name;binary = out / label
                run([*common, *flags, '-Wall', '-Wextra', '-Werror', '-pedantic',
                     ROOT / ('tests/' + test), *code[1:], '-o', binary], 'compile-' + label)
                run([binary], label, env)
        build = out / 'cmake'
        run(['cmake', '-S', ROOT / 'reconstruction/rendering/effect-creature', '-B', build], 'configure')
        run(['cmake', '--build', build, '-j2'], 'build')
        run(['ctest', '--test-dir', build, '--output-on-failure'], 'ctest')
        if any(sha(ROOT / p) != h for p, h in sources.items()) or sha(pe) != HASH:
            raise ValueError('Inputs changed during comparison')
        report = dict(all_match=True, live_validated=False, executable_sha256=HASH,
                      suites=suites, calls=sum(x["calls"] for x in suites.values()), helper_sha256=helpers,
                      registered_source_sha256=sources, unit_passed=True, sanitized_unit_passed=True,
                      cmake_tests_passed=2,
                      boundary='Whole unmodified 004883f0 selected effect types3/13/22/24/36, kinds0/34/68, raw creator ordinal: kind34 permits creator in initial and refreshed candidates, status/occupancy/ordering still gate hits; metadata threshold DWORD authored zero. Fresh ordinary/boundary baseline reruns, mixed exits, narrow/wrapped grids and authored mutations; no other kinds/types, installed inputs, creator ownership initialization or live replacement')
        (out / 'report.json').write_text(json.dumps(report, indent=2) + '\n')
        print(json.dumps(report, indent=2), flush=True)
    finally:
        run([ROOT / 'tools/original-manifest.sh', 'verify'], 'original-after')

if __name__ == '__main__':
    main()
