#!/usr/bin/env python3
"""Execute original effect catalog metadata windows, preserving policy differences."""
import configparser
import hashlib
import json
import os
from pathlib import Path
import random
import re
import runpy
import struct
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
HASH = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
SOURCES = [
    'reconstruction/rendering/effect_animation_metadata.hpp',
    'reconstruction/rendering/effect_animation_metadata.cpp',
    'reconstruction/rendering/effect-animation-metadata/CMakeLists.txt',
    'tests/effect-animation-metadata-reference.cpp', 'tests/effect-animation-metadata-test.cpp',
    'tests/sprite-binary-reference.cpp', 'tools/test-effect-animation-metadata.py', 'tools/decode-cfg.py',
    'assets/effect_animation_catalog.cpp', 'assets/effect_animation_catalog.hpp', 'assets/CMakeLists.txt',
    'assets/asset_file.cpp', 'assets/asset_file.hpp', 'assets/path_resolver.cpp', 'assets/path_resolver.hpp',
    'assets/animation.cpp', 'assets/animation_decode.cpp', 'assets/animation.hpp',
    'assets/config_loader.cpp', 'assets/packed_container.cpp', 'assets/persistence.hpp', 'assets/persistence_internal.hpp',
]

def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()

def count_value(text):
    match = re.match(r'[ \t\n\r\v\f]*([+-]?)([0-9]+)', text)
    value = 0 if not match else int((match[1] or '') + match[2]) & 0xffffffff
    return value if 1 < value < 0x80000000 else 1

def main():
    parent = ROOT / 'working/tests/effect-animation-metadata'
    parent.mkdir(parents=True, exist_ok=True)
    out = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    print(out, flush=True)
    env = dict(os.environ, ASAN_OPTIONS='detect_leaks=0', UBSAN_OPTIONS='halt_on_error=1')
    def run(args, name):
        p = subprocess.run(list(map(str, args)), cwd=ROOT, env=env, text=True, capture_output=True, timeout=240)
        (out / (name+'.log')).write_text(p.stdout+p.stderr)
        p.check_returncode()
        return p.stdout
    run([ROOT / 'tools/original-manifest.sh', 'verify'], 'original-before')
    try:
        before = {p: sha(ROOT/p) for p in SOURCES}
        pe = ROOT / 'working/game-nocd/Chaos.exe'
        cfg = ROOT / 'working/game-clean/CFG/Encrypted/effectani.cfg'
        if sha(pe) != HASH:
            raise ValueError('Wrong original build')
        inputs = {str(p.relative_to(ROOT)): sha(p) for p in [pe, cfg]}
        _, decoded = runpy.run_path(str(ROOT / 'tools/decode-cfg.py'))['decode'](cfg.read_bytes())
        text = decoded.decode('ascii')
        profile = configparser.ConfigParser(interpolation=None, inline_comment_prefixes=(';',))
        profile.read_string(text[text.index('[HEADER]'):])
        keys = ['AnimationNo', 'AnimationFileRef', 'SpritePrinter', 'Data1']
        installed = [[profile.get('ANI_'+str(i), key).strip() for key in keys]
                     for i in range(profile.getint('HEADER', 'NumberofAnimations'))]
        fixtures = []
        def add(name, rows, count='13'):
            parsed = count_value(count)
            fixtures.append(dict(name=name, count_text=count, metadata_file_count=parsed if parsed<=64 else 13, rows=rows))
        add('installed-complete-222', installed)
        printers = ['NORMAL', 'TRANSPARENT', 'TRANSPARENT25', 'TRANSPARENT50', 'TRANSPARENT75']
        add('all-printers-and-case', [['0', 'effects0', p.lower(), '4294967295'] for p in printers])
        add('highest-file-ordinal', [['0', 'eFfEcTs63', 'nOrMaL', '0']], '64')
        counts = ['', '0', '1', '2', '13', '64', '65', '-1', '-2147483648',
                  '2147483647', '2147483648', '4294967295', '4294967296', '4294967297',
                  '+13tail', '13tail', ' 13 ', 'nonnumeric', '0x10', '9'*255]
        for i, count in enumerate(counts):
            add('count-'+str(i), [['0', 'EFFECTS0', 'NORMAL', '0']], count)
        numbers = ['', '0', '1', '-1', '+12tail', '12tail', 'nonnumeric', ' \t-7',
                   '2147483647', '2147483648', '-2147483648', '-2147483649',
                   '4294967295', '4294967296', '4294967297', '0x10', '9'*255]
        references = ['', 'EFFECTS0', 'effects12', 'EfFeCtS1', 'EFFECTS13', 'EFFECTS00',
                      'EFFECTS-1', 'UNKNOWN', 'EFFECTS0tail', ' EFFECTS0', 'EFFECTS0 ', 'EFFECTS0;comment']
        print_values = ['', *printers, 'normal', 'tRaNsPaReNt50', 'unknown',
                        ' NORMAL', 'NORMAL ', 'NORMAL;comment', 'TRANSPARENT100']
        choices = [numbers, references, print_values, numbers]
        for column, values in enumerate(choices):
            for i, value in enumerate(values):
                row = ['0', 'EFFECTS0', 'NORMAL', '0']
                row[column] = value
                add('field-'+str(column)+'-'+str(i), [row])
        add('all-empty-retains-seeds', [['', '', '', '']])
        rng = random.Random(0x49c836)
        for i in range(48):
            add('mixed-'+str(i), [[rng.choice(values) for values in choices] for _ in range(8)])
        def string(s):
            return struct.pack('<I', len(s))+s.encode('ascii')
        fixture = out / 'fixtures.bin'
        fixture.write_bytes(struct.pack('<I', len(fixtures))+b''.join(
            string(f['count_text'])+struct.pack('<II', f['metadata_file_count'], len(f['rows']))+
            b''.join(string(s) for row in f['rows'] for s in row) for f in fixtures))
        (out / 'fixtures.json').write_text(json.dumps(fixtures, indent=2)+'\n')
        original = out / 'original'
        run(['g++', '-std=c++17', '-O2', '-m32', '-fno-pie', '-no-pie', '-DORIGINAL_REFERENCE',
             '-I', ROOT / 'reconstruction/rendering', ROOT / 'tests/effect-animation-metadata-reference.cpp',
             ROOT / 'reconstruction/rendering/effect_animation_metadata.cpp', '-o', original], 'original-build')
        original_stats = json.loads(run([original, pe, fixture, out / 'original.bin'], 'original'))
        results = {}
        for name, flags in [('normal', []), ('sanitized', ['-DCMAKE_CXX_FLAGS=-fsanitize=address,undefined -fno-omit-frame-pointer -fno-pie', '-DCMAKE_EXE_LINKER_FLAGS=-fsanitize=address,undefined -no-pie'])]:
            build = out / name
            run(['cmake', '-S', ROOT / 'reconstruction/rendering/effect-animation-metadata', '-B', build, '-DBUILD_TESTING=ON', *flags], name+'-configure')
            run(['cmake', '--build', build, '--target', 'effect-animation-metadata-reference', 'effect-animation-metadata-test', '-j4'], name+'-build')
            results[name] = json.loads(run([build / 'effect-animation-metadata-reference', fixture, out / (name+'.bin')], name))
            run(['ctest', '--test-dir', build, '-R', '^effect-animation-metadata$', '--output-on-failure'], name+'-units')
            if any(results[name][key]!=value for key,value in original_stats.items()) or sha(out/(name+'.bin'))!=sha(out/'original.bin'):
                raise ValueError('Original/model output mismatch')
        if results['normal'] != results['sanitized']:
            raise ValueError('Native policy mismatch between builds')
        differences = results['normal']['native_difference_fixtures']
        refused = results['normal']['native_refusal_fixtures']
        if 0 in differences or 0 in refused:
            raise ValueError('Installed complete catalog differs from original metadata')
        # Accepted divergences must be only explicit trim/semicolon native policy.
        allowed = {i for i,f in enumerate(fixtures) if any(
            value != value.strip() or ';' in value for row in f['rows'] for value in row)}
        if set(differences)-allowed:
            raise ValueError('Unclassified native policy difference')
        if any(sha(ROOT/p)!=h for p,h in before.items()) or any(sha(ROOT/p)!=h for p,h in inputs.items()):
            raise ValueError('Sources or inputs changed')
        report = dict(all_match=True, original_regions_executed=True, original_whole_loader_executed=False,
                      live_validated=False, executable_sha256=HASH, registered_source_sha256=before,
                      input_sha256=inputs, **original_stats, native_policy=results['normal'],
                      installed_entries=len(installed), seeds=[0,165,255],
                      metadata_windows=len(fixtures)*3, profile_reads=len(fixtures)+original_stats['metadata_rows']*4,
                      output_sha256=sha(out/'original.bin'), fixture_sha256=sha(fixture), original_helper_sha256=sha(original),
                      native_policy_differences=[dict(index=i,name=fixtures[i]['name'],rows=fixtures[i]['rows']) for i in differences],
                      native_policy_refusals=[dict(index=i,name=fixtures[i]['name']) for i in refused],
                      run=str(out.relative_to(ROOT)),
                      boundary='Unmodified 0049c6e2..0049c735 file-count parse/clamp and 0049c836..0049caae metadata loop with real CRT atoi/itoa/stricmp. Authored register/stack/allocation preconditions, checked profile-output callback, hardware execution breakpoint endpoints. Seeded words/guards, ordinal/cursor/request count and code immutability checked. No loader prologue/allocation/Windows profile/filesystem/ANI-SPR loading/cache/lifecycle or live execution.')
        (out / 'report.json').write_text(json.dumps(report, indent=2)+'\n')
        print(json.dumps({k:report[k] for k in ['all_match','fixtures','count_windows','metadata_windows','metadata_rows','profile_reads','native_policy']}, indent=2), flush=True)
    finally:
        run([ROOT / 'tools/original-manifest.sh', 'verify'], 'original-after')

if __name__ == '__main__':
    main()
