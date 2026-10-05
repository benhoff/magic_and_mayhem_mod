#!/usr/bin/env python3
"""Compare the complete effects light-table loader with bounded owned native reads."""
import argparse
import configparser
import hashlib
import json
import os
from pathlib import Path
import random
import runpy
import struct
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
HASH = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
PATHS = ['assets/effect_lighting.cpp', 'assets/effect_lighting.hpp',
         'assets/profile.cpp', 'assets/profile.hpp', 'assets/packed_container.cpp',
         'assets/asset_file.hpp', 'assets/path_resolver.hpp', 'assets/persistence.hpp', 'assets/persistence_internal.hpp',
         'assets/CMakeLists.txt', 'reconstruction/rendering/CMakeLists.txt', 'apps/terrain-preview/CMakeLists.txt',
         'assets/asset_file.cpp', 'assets/path_resolver.cpp',
         'reconstruction/rendering/effect_lighting.cpp', 'reconstruction/rendering/effect_lighting.hpp',
         'reconstruction/rendering/terrain_lighting.cpp', 'reconstruction/rendering/terrain_lighting.hpp',
         'reconstruction/rendering/terrain_creature_lighting.cpp', 'reconstruction/rendering/terrain_creature_lighting.hpp',
         'reconstruction/rendering/terrain_static_lighting.cpp', 'reconstruction/rendering/terrain_static_lighting.hpp',
         'reconstruction/rendering/terrain_lighting_cycle.cpp', 'reconstruction/rendering/terrain_lighting_cycle.hpp',
         'tests/effect-lighting-reference.cpp', 'tests/sprite-binary-reference.cpp',
         'tests/effect-lighting-test.cpp', 'tests/effect-lighting-installed.cpp',
         'tools/decode-cfg.py', 'tools/test-effect-lighting.py']

def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()

def encode(fields):
    return b''.join(struct.pack('<I', len(v.encode('ascii'))) + v.encode('ascii') for row in fields for v in row)

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--source-root', type=Path, default=ROOT)
    parser.add_argument('--build', type=Path, required=True)
    parser.add_argument('--sanitized-build', type=Path, required=True)
    args = parser.parse_args()
    source = args.source_root.resolve()
    parent = ROOT / 'working/tests/effect-lighting'
    parent.mkdir(parents=True, exist_ok=True)
    out = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    print(out, flush=True)
    def run(command, name, env=None):
        result = subprocess.run(list(map(str, command)), capture_output=True, text=True, timeout=180, env=env)
        (out / (name + '.log')).write_text(result.stdout + result.stderr)
        result.check_returncode()
        return result.stdout
    run([ROOT / 'tools/original-manifest.sh', 'verify'], 'original-before')
    try:
        pe = ROOT / 'working/game-nocd/Chaos.exe'
        packed = ROOT / 'working/game-clean/CFG/Encrypted/effects.cfg'
        if sha(pe) != HASH:
            raise ValueError('Executable hash mismatch')
        before = {p: sha(source / p) for p in PATHS}
        input_hashes = {str(p.relative_to(ROOT)): sha(p) for p in (pe, packed)}
        decoder = runpy.run_path(str(source / 'tools/decode-cfg.py'))
        _, decoded = decoder['decode'](packed.read_bytes())
        profile = configparser.ConfigParser(interpolation=None)
        profile.read_string(decoded.decode('ascii'))
        keys = ['LightSourceDiameter', 'LightSourceAffected', 'ClippedToHeight']
        installed = [[profile.get('FXA_' + str(i), k, fallback='').strip().strip('\"\'')[:255] for k in keys] for i in range(89)]
        rng = random.Random(0x49c2e0)
        diameters = ['', '0', '1', '2', '32', '33', '34', '-1', '+16tail', 'nonnumeric', '  \t-7',
                     '2147483647', '2147483648', '-2147483648', '-2147483649',
                     '4294967297', '0x10', '9' * 255, '0' * 254 + '3']
        flags = ['', 'TRUE', 'true', 'True', 'tRuE', 'FALSE', 'False', 'TRUE ', ' TRUE', 'TRUE;comment', '1']
        fixtures = [installed, [['', '', ''] for _ in range(89)]]
        fixtures += [[[rng.choice(diameters), rng.choice(flags), rng.choice(flags)] for _ in range(89)] for _ in range(64)]
        fixture = out / 'fixtures.bin'
        fixture.write_bytes(struct.pack('<I', len(fixtures)) + b''.join(encode(f) for f in fixtures))
        env = dict(os.environ, ASAN_OPTIONS='detect_leaks=0', UBSAN_OPTIONS='halt_on_error=1', LIBGL_ALWAYS_SOFTWARE='1', QT_QPA_PLATFORM='xcb')
        for name, build in [('normal', args.build), ('sanitized', args.sanitized_build)]:
            field_file = out / (name + '-installed.bin')
            run([build.resolve() / 'effect-lighting-installed', ROOT / 'working/game-clean', field_file], name + '-installed', env)
            if field_file.read_bytes() != encode(installed):
                raise ValueError('Native packed asset fields differ from independent decoded profile')
        common = ['g++', '-std=c++17', '-I' + str(source / 'reconstruction/rendering')]
        code = [source / 'tests/effect-lighting-reference.cpp', source / 'reconstruction/rendering/effect_lighting.cpp']
        results = []
        helpers = {}
        for name, flags in [('original', ['-O2', '-m32', '-fno-pie', '-no-pie', '-DORIGINAL_REFERENCE']),
                            ('native', ['-O2']), ('sanitized', ['-O1', '-g', '-fsanitize=address,undefined', '-fno-omit-frame-pointer', '-fno-pie', '-no-pie'])]:
            binary = out / name
            run([*common, *flags, *code, '-o', binary], 'compile-' + name)
            output = out / (name + '.bin')
            stats = json.loads(run([binary, *([pe] if name == 'original' else []), fixture, output], name, env))
            results.append((stats, sha(output)))
            helpers[name] = sha(binary)
        if not results[0] == results[1] == results[2]:
            raise ValueError('Original/native/sanitized tables differ')
        suites = {}
        for name, build in [('normal', args.build), ('ASan/UBSan', args.sanitized_build)]:
            result = run(['ctest', '--test-dir', build.resolve(), '--output-on-failure'], 'suite-' + name.replace('/', '-'), env)
            import re
            match = re.search(r'100% tests passed(?:, 0 tests failed)? out of (\d+)', result)
            if not match:
                raise ValueError('Full native suite failed')
            suites[name] = dict(tests_passed=int(match.group(1)))
        if any(sha(source / p) != digest for p, digest in before.items()):
            raise ValueError('Source changed during comparison')
        if any(sha(ROOT / p) != digest for p, digest in input_hashes.items()):
            raise ValueError('Input changed during comparison')
        emitters = {str(i): int(row[0]) for i, row in enumerate(installed) if int(row[0])}
        report = dict(all_match=True, live_validated=False, executable_sha256=HASH,
                      **results[0][0], installed_emitters=emitters, installed_entries=89,
                      full_suites=suites, output_sha256=results[0][1], helper_sha256=helpers,
                      registered_source_sha256=before, input_sha256=input_hashes,
                      boundary='Whole unmodified 0049c2e0 and 0049cf60, private IAT callbacks only for directory/profile reads; original CRT retained; bounded ASCII/C-locale profile outputs and owned reloads; installed packed effects file and authored-position native lighting composition; no source lifecycle, Windows profile call, game screen or live replacement')
        (out / 'report.json').write_text(json.dumps(report, indent=2) + '\n')
        print(json.dumps({k: v for k, v in report.items() if not isinstance(v, dict)}, indent=2), flush=True)
    finally:
        run([ROOT / 'tools/original-manifest.sh', 'verify'], 'original-after')

if __name__ == '__main__':
    main()
