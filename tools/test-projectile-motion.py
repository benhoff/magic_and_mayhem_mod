#!/usr/bin/env python3
"""Execute unmodified 004883f0 against owned native creature footprint lookup."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
HASH = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
PATHS = ['reconstruction/rendering/effect_transition.cpp', 'reconstruction/rendering/effect_transition.hpp', 'reconstruction/rendering/effect_cleanup.cpp', 'reconstruction/rendering/effect_cleanup.hpp', 'reconstruction/rendering/effect_projection.cpp', 'reconstruction/rendering/effect_projection.hpp', 'reconstruction/rendering/effect_placement.cpp', 'reconstruction/rendering/effect_placement.hpp', 'reconstruction/rendering/effect_lighting.cpp', 'reconstruction/rendering/effect_lighting.hpp', 'reconstruction/rendering/terrain_lighting_cycle.cpp', 'reconstruction/rendering/terrain_lighting_cycle.hpp', 'reconstruction/rendering/terrain_static_lighting.cpp', 'reconstruction/rendering/terrain_static_lighting.hpp', 'reconstruction/rendering/terrain_creature_lighting.cpp', 'reconstruction/rendering/terrain_creature_lighting.hpp', 'reconstruction/rendering/terrain_lighting.cpp', 'reconstruction/rendering/terrain_lighting.hpp', 'reconstruction/rendering/effect_trajectory.cpp', 'reconstruction/rendering/effect_trajectory.hpp', 'tests/sprite-binary-reference.cpp', 'reconstruction/rendering/creature_occupancy.cpp', 'reconstruction/rendering/creature_occupancy.hpp', 'reconstruction/rendering/creature_footprint.cpp', 'reconstruction/rendering/creature_footprint.hpp', 'reconstruction/rendering/effect_creature_collision.cpp', 'reconstruction/rendering/effect_creature_collision.hpp', 'reconstruction/rendering/effect-creature/CMakeLists.txt', 'tests/projectile-motion-reference.cpp', 'tests/effect-creature-test.cpp', 'tools/test-projectile-motion.py']

def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()

def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--output',type=Path,required=True);parser.add_argument('--claims',type=Path);args=parser.parse_args();out=args.output.resolve();out.mkdir(parents=True,exist_ok=False)
    claims=json.loads(args.claims.read_text()) if args.claims else {'claims':[],'sources':{}}
    if any(sha(ROOT/p)!=v for p,v in claims['sources'].items()):raise ValueError('Unstable declared inputs')
    print(out, flush=True)
    def run(command, name, env=None):
        result = subprocess.run(list(map(str, command)), capture_output=True, text=True,
                                timeout=180, env=env)
        (out / (name + '.log')).write_text(result.stdout + result.stderr)
        result.check_returncode()
        return result.stdout
    run([ROOT / 'tools/original-manifest.sh', 'verify'], 'original-before')
    try:
        pe = ROOT / 'original/Arcane_Nocd/Chaos.exe'
        if sha(pe) != HASH:
            raise ValueError('Executable hash mismatch')
        sources = {p: sha(ROOT / p) for p in PATHS}
        if args.claims and set(PATHS)-claims['sources'].keys():raise ValueError('Unbound sources')
        code = [ROOT / 'tests/projectile-motion-reference.cpp', *[ROOT / p for p in PATHS if p.startswith('reconstruction/') and p.endswith('.cpp')]]
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
                      registered_source_sha256=sources, source_sha256=claims['sources'], claims=claims['claims'], unit_passed=True, sanitized_unit_passed=True,
                      cmake_tests_passed=1,
                      boundary='Whole unmodified4883f0 type3/spell73 with actual trajectory and footprint helpers: authored 1..20 steps and sixteen repeated calls, creator exclusion, changing cells, wrap, blocked/bitmap/height exits and guarded complete state. Fixed actor world between calls; no ricochet geometry, installed catalog or live replacement')
    finally:
        run([ROOT / 'tools/original-manifest.sh', 'verify'], 'original-after')
    report['sources_unchanged']=all(sha(ROOT/p)==v for p,v in claims['sources'].items())
    report['success']=report['sources_unchanged']
    report['original_manifest_verified_before_after']=True
    (out / 'report.json').write_text(json.dumps(report,indent=2)+'\n')
    if not report['success']:raise ValueError('Declared source changed during comparison')
    print(json.dumps({'success':True,'calls':report['calls'],'creature_hits':report['creature_hits']}))

if __name__ == '__main__':
    main()
