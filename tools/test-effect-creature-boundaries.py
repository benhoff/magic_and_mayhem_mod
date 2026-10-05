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
PATHS = ['reconstruction/rendering/effect_transition.cpp', 'reconstruction/rendering/effect_transition.hpp', 'reconstruction/rendering/effect_cleanup.cpp', 'reconstruction/rendering/effect_cleanup.hpp', 'reconstruction/rendering/effect_projection.cpp', 'reconstruction/rendering/effect_projection.hpp', 'reconstruction/rendering/effect_placement.cpp', 'reconstruction/rendering/effect_placement.hpp', 'reconstruction/rendering/effect_lighting.cpp', 'reconstruction/rendering/effect_lighting.hpp', 'reconstruction/rendering/terrain_lighting_cycle.cpp', 'reconstruction/rendering/terrain_lighting_cycle.hpp', 'reconstruction/rendering/terrain_static_lighting.cpp', 'reconstruction/rendering/terrain_static_lighting.hpp', 'reconstruction/rendering/terrain_creature_lighting.cpp', 'reconstruction/rendering/terrain_creature_lighting.hpp', 'reconstruction/rendering/terrain_lighting.cpp', 'reconstruction/rendering/terrain_lighting.hpp', 'reconstruction/rendering/effect_trajectory.cpp', 'reconstruction/rendering/effect_trajectory.hpp', 'tests/sprite-binary-reference.cpp', 'reconstruction/rendering/creature_occupancy.cpp', 'reconstruction/rendering/creature_occupancy.hpp', 'reconstruction/rendering/creature_footprint.cpp', 'reconstruction/rendering/creature_footprint.hpp', 'reconstruction/rendering/effect_creature_collision.cpp', 'reconstruction/rendering/effect_creature_collision.hpp', 'reconstruction/rendering/effect-creature/CMakeLists.txt', 'tests/effect-creature-boundaries-reference.cpp', 'tests/effect-creature-test.cpp', 'tools/test-effect-creature-boundaries.py']

def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()

def main():
    parent = ROOT / 'working/tests/effect-creature-boundaries'
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
        code = [ROOT / 'tests/effect-creature-boundaries-reference.cpp', *[ROOT / p for p in PATHS if p.startswith('reconstruction/') and p.endswith('.cpp')]]
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
                 ROOT / 'tests/effect-creature-test.cpp', *code[1:], '-o', binary], 'compile-' + name)
            run([binary], name, env)
        build = out / 'cmake'
        run(['cmake', '-S', ROOT / 'reconstruction/rendering/effect-creature', '-B', build], 'configure')
        run(['cmake', '--build', build, '-j2'], 'build')
        run(['ctest', '--test-dir', build, '--output-on-failure'], 'ctest')
        if any(sha(ROOT / p) != h for p, h in sources.items()) or sha(pe) != HASH:
            raise ValueError('Inputs changed during comparison')
        report = dict(all_match=True, live_validated=False, executable_sha256=HASH,
                      **results[0][0], output_sha256=results[0][1], helper_sha256=helpers,
                      registered_source_sha256=sources, unit_passed=True, sanitized_unit_passed=True,
                      cmake_tests_passed=1,
                      boundary='Ordered all-27 candidate positions with narrow/wrapped/vertical boundary grids, competing hits and null/status/creator/invalid filters; type3 hit then terrain/blocked/height exit, no-hit continuation and different-hit overwrite; authored cell/status/position/footprint/id changes between calls. Whole unmodified 004883f0 selected creature-aware movement: initial and refreshed 27-cell candidates, kind 0/68, creator exclusion, null/out-of-catalog entries, status27 filter, wrapped signed coordinate differences, first footprint hit, type3 continuation and other selected types return0; bounded owned world, no kind34 creator exception, other types, installed catalog/capture/live replacement')
        (out / 'report.json').write_text(json.dumps(report, indent=2) + '\n')
        print(json.dumps(report, indent=2), flush=True)
    finally:
        run([ROOT / 'tools/original-manifest.sh', 'verify'], 'original-after')

if __name__ == '__main__':
    main()
