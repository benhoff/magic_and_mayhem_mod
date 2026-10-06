#!/usr/bin/env python3
"""Installed owned effect catalog/forward binding; static producer boundaries only."""
import configparser
import hashlib
import json
import os
from pathlib import Path
import runpy
import shutil
import struct
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
HASH = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
SOURCES = [
    'assets/effect_animation_catalog.cpp', 'assets/effect_animation_catalog.hpp',
    'assets/CMakeLists.txt', 'assets/config_loader.cpp', 'assets/packed_container.cpp',
    'assets/persistence.hpp', 'assets/persistence_internal.hpp',
    'assets/asset_file.cpp', 'assets/asset_file.hpp', 'assets/path_resolver.cpp', 'assets/path_resolver.hpp',
    'assets/animation.cpp', 'assets/animation_decode.cpp', 'assets/animation.hpp',
    'assets/sprite_loader.cpp', 'assets/sprite_loader.hpp',
    'reconstruction/rendering/effect_animation_catalog_binding.cpp',
    'reconstruction/rendering/effect_animation_catalog_binding.hpp',
    'reconstruction/rendering/effect-animation-catalog/CMakeLists.txt',
    'reconstruction/rendering/effect_animation_binding.cpp', 'reconstruction/rendering/effect_animation_binding.hpp',
    'reconstruction/rendering/effect-animation-binding/CMakeLists.txt',
    'reconstruction/rendering/effect_animation_selection.cpp', 'reconstruction/rendering/effect_animation_selection.hpp',
    'reconstruction/animation/no_cd.cpp', 'reconstruction/animation/no_cd.hpp',
    'tests/effect-animation-catalog-test.cpp', 'tools/test-effect-animation-catalog.py', 'tools/decode-cfg.py',
]

def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()

def main():
    parent = ROOT / 'working/tests/effect-animation-catalog'
    parent.mkdir(parents=True, exist_ok=True)
    out = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    print(out, flush=True)
    env = dict(os.environ, ASAN_OPTIONS='detect_leaks=0', UBSAN_OPTIONS='halt_on_error=1')
    def run(args, name, expect=True):
        p = subprocess.run(list(map(str, args)), cwd=ROOT, env=env, text=True, capture_output=True, timeout=240)
        (out / (name + '.log')).write_text(p.stdout + p.stderr)
        if expect:
            p.check_returncode()
        elif p.returncode == 0:
            raise ValueError('Expected refusal: ' + name)
        return p
    run([ROOT / 'tools/original-manifest.sh', 'verify'], 'original-before')
    try:
        before = {p: sha(ROOT / p) for p in SOURCES}
        game = ROOT / 'working/game-clean'
        pe = ROOT / 'working/game-nocd/Chaos.exe'
        if sha(pe) != HASH:
            raise ValueError('Wrong original executable')
        cfg = game / 'CFG/Encrypted/effectani.cfg'
        _, decoded = runpy.run_path(str(ROOT / 'tools/decode-cfg.py'))['decode'](cfg.read_bytes())
        text = decoded.decode('ascii')
        text = text[text.index('[HEADER]'):]
        profile = configparser.ConfigParser(interpolation=None, inline_comment_prefixes=(';',))
        profile.read_string(text)
        files = profile.getint('HEADER', 'NumberOfEffectsFile')
        count = profile.getint('HEADER', 'NumberofAnimations')
        printers = {'NORMAL': 31, 'TRANSPARENT': 2, 'TRANSPARENT25': 3, 'TRANSPARENT50': 4, 'TRANSPARENT75': 5}
        expected = struct.pack('<II', files, count)
        rows = []
        for i in range(count):
            section = profile['ANI_' + str(i)]
            reference = section['AnimationFileRef'].strip().upper()
            index = ['EFFECTS' + str(j) for j in range(files)].index(reference)
            row = [int(section['AnimationNo']), index, printers[section['SpritePrinter'].strip().upper()], int(section['Data1'])]
            rows.append(row)
            expected += struct.pack('<4I', *row)
        def sprite_file(i, suffix):
            matches = [p for p in (game / 'Sprites').iterdir() if p.name.lower() == 'effects'+str(i)+suffix]
            if len(matches) != 1:
                raise ValueError('Missing/ambiguous installed effect file')
            return matches[0]
        inputs = {str(p.relative_to(ROOT)): sha(p) for p in [pe, cfg, *[sprite_file(i, suffix) for i in range(files) for suffix in ['.ani', '.spr']]]}
        # Retain byte-bound static evidence, distinct from execution of the loader.
        binary = pe.read_bytes()
        header = struct.unpack_from('<I', binary, 60)[0]
        sections = struct.unpack_from('<H', binary, header + 6)[0]
        optional = struct.unpack_from('<H', binary, header + 20)[0]
        def original_bytes(address, length):
            for i in range(sections):
                _, va, size, offset = struct.unpack_from('<IIII', binary, header + 24 + optional + 40*i + 8)
                relative = address - 0x400000 - va
                if 0 <= relative and relative + length <= size:
                    return binary[offset+relative:offset+relative+length]
            raise ValueError('Static range outside file')
        anchors = {}
        for address, word in [(0x5e026c, 'NORMAL'), (0x5e0260, 'TRANSPARENT'), (0x5e0250, 'TRANSPARENT25'), (0x5e0240, 'TRANSPARENT50'), (0x5e0230, 'TRANSPARENT75'), (0x5e0228, 'Data1')]:
            raw = original_bytes(address, len(word)+1)
            if raw != word.encode()+b'\0':
                raise ValueError('Static string anchor changed')
            anchors[hex(address)] = raw.hex()
        windows = {}
        for start, end in [(0x49c653, 0x49c735), (0x49c841, 0x49caae), (0x49cabe, 0x49cc11)]:
            raw = original_bytes(start, end-start)
            path = out / (hex(start) + '.bin')
            path.write_bytes(raw)
            windows[hex(start)] = dict(end=hex(end), sha256=sha(path))
        results = {}
        for name, flags in [('normal', []), ('sanitized', ['-DCMAKE_CXX_FLAGS=-fsanitize=address,undefined -fno-omit-frame-pointer -fno-pie', '-DCMAKE_EXE_LINKER_FLAGS=-fsanitize=address,undefined -no-pie'])]:
            build = out / name
            run(['cmake', '-S', ROOT / 'reconstruction/rendering/effect-animation-catalog', '-B', build, '-DBUILD_TESTING=ON', *flags], name+'-configure')
            run(['cmake', '--build', build, '--target', 'effect-animation-catalog-test', '-j4'], name+'-build')
            exe = build / 'effect-animation-catalog-test'
            run(['ctest', '--test-dir', build, '-R', '^effect-animation-catalog$', '--output-on-failure'], name+'-units')
            output = out / (name+'.bin')
            result = json.loads(run([exe, game, output], name+'-installed').stdout)
            if output.read_bytes() != expected or result['entries'] != count or result['states'] != count*65:
                raise ValueError('Installed catalog mismatch')
            results[name] = result
            # Copied private fixtures exercise failures after previous ANI loads.
            for mode in ['missing-config', 'missing-third-ani', 'malformed-first-ani']:
                root = out / (name+'-'+mode)
                (root / 'CFG/Encrypted').mkdir(parents=True)
                (root / 'Sprites').mkdir()
                if mode != 'missing-config':
                    shutil.copyfile(cfg, root / 'CFG/Encrypted/effectani.cfg')
                if mode == 'missing-third-ani':
                    for i in range(2):
                        shutil.copyfile(sprite_file(i, '.ani'), root / ('Sprites/EFFECTS'+str(i)+'.ani'))
                if mode == 'malformed-first-ani':
                    (root / 'Sprites/EFFECTS0.ani').write_bytes(b'invalid ANI')
                refusal = run([exe, root, out / 'refused.bin'], name+'-'+mode, False)
                path = 'effectani.cfg' if mode == 'missing-config' else ('EFFECTS2.ani' if mode == 'missing-third-ani' else 'EFFECTS0.ani')
                if path not in refusal.stderr or 'AddressSanitizer' in refusal.stderr or 'runtime error:' in refusal.stderr:
                    raise ValueError('Unexpected refusal: '+refusal.stderr)
        if results['normal'] != results['sanitized']:
            raise ValueError('Sanitized result mismatch')
        if any(sha(ROOT/p) != h for p,h in before.items()) or any(sha(ROOT/p) != h for p,h in inputs.items()):
            raise ValueError('Sources or inputs changed')
        report = dict(all_match=True, original_loader_executed=False, live_validated=False,
                      executable_sha256=HASH, registered_source_sha256=before, input_sha256=inputs,
                      installed_files=files, installed_entries=count, metadata_rows=rows, printer_codes=printers,
                      **{'normal': results['normal'], 'sanitized': results['sanitized']},
                      output_sha256=hashlib.sha256(expected).hexdigest(),
                      static_string_anchors=anchors, static_windows=windows, run=str(out.relative_to(ROOT)),
                      refusal_cases_per_build=3,
                      boundary='Independent checksum-checked Python CFG metadata versus native owned catalog; all selected ANI sprite records checked against paired decoded SPR; 65 forward states per entry, selector-to-catalog binding and destroyed catalog ownership. Static pinned loader inspection only; no original loader execution, Windows profile equivalence, renderer/lifecycle/scheduling or live replacement.')
        (out / 'report.json').write_text(json.dumps(report, indent=2)+'\n')
        print(json.dumps({k:report[k] for k in ['all_match','installed_files','installed_entries','normal','original_loader_executed']}, indent=2), flush=True)
    finally:
        run([ROOT / 'tools/original-manifest.sh', 'verify'], 'original-after')

if __name__ == '__main__':
    main()
