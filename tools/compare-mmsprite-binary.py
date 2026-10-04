#!/usr/bin/env python3
"""Compare pinned MMSprite samples with isolated original PE32 sprite routines."""
import argparse
from collections import Counter
import hashlib
import io
import json
import os
from pathlib import Path
import re
import runpy
import struct
import subprocess
import sys
import tempfile

REPO = Path(__file__).resolve().parents[1]
HASH = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
CLEAN_HASH = '124a0601759f6b0607d6f81c94c47b1bde05e747456bfaba68ab9da84d800214'
RANGES = {
    'spr_load': (0x57d310, 0x57d55c), 'direct_word_565_to_555': (0x57d560, 0x57d5d8),
    'palette_build_dispatch': (0x57d690, 0x57da98),
    'indexed_draw': (0x57e1b0, 0x57e534), 'sprite_draw_dispatch': (0x57e8e0, 0x57e9e0),
    'direct_word_draw': (0x597086, 0x5974d0), 'font_load': (0x4a5ae0, 0x4a5e90),
    'sprite_backend_selection': (0x48450b, 0x48458b),
    'palette_rgb_packing': (0x582750, 0x5827d0),
    'surface_pixel_masks': (0x58d540, 0x58d5a9),
}
CLEAN_RANGES = {'clean_spr_load': (0x564110, 0x56435c),
                'clean_word_conversion': (0x564360, 0x5643d8),
                'clean_draw_dispatch': (0x565050, 0x5650e0)}


def sha(data):
    return hashlib.sha256(data).hexdigest()


def uint(data, offset):
    return struct.unpack_from('<I', data, offset)[0]


def model(data, frame, pals, converted=False):
    """Checked row model for the two widths observed in original draw code."""
    w, h = frame.width, frame.height
    stride = w + 2
    out = [0x1234] * (stride * (h + 2))
    for y in range(h):
        delta = frame.offset + frame.delta_offsets[y]
        pixel = frame.offset + frame.pixel_offsets[y]
        x = 0
        colour = False
        while x < w:
            if not frame.offset <= delta < frame.offset + frame.size:
                raise ValueError('delta outside frame')
            length = data[delta]
            delta += 1
            if x + length > w:
                raise ValueError('run exceeds row width')
            if colour:
                for at in range(length):
                    if pals:
                        index = data[pixel]
                        pixel += 1
                        p = pals[frame.palette_index].pixels[index]
                        value = (p.r >> 3) << 11 | (p.g >> 2) << 5 | (p.b >> 3)
                    else:
                        value = struct.unpack_from('<H', data, pixel)[0]
                        pixel += 2
                        if converted:
                            value = ((value >> 1) & 0x7fe0) | (value & 31)
                    if pixel > frame.offset + frame.size:
                        raise ValueError('pixels outside frame')
                    out[(y + 1) * stride + x + at + 1] = value
            x += length
            colour = not colour
    return struct.pack('<' + 'H' * len(out), *out)


def mm565(rows, frame):
    stride = frame.width + 2
    out = [0x1234] * (stride * (frame.height + 2))
    if len(rows) != frame.height or any(len(row) != frame.width * 4 for row in rows):
        raise ValueError('MMSprite nonrectangular result')
    for y, row in enumerate(rows):
        for x in range(frame.width):
            r, g, b, a = row[x * 4:x * 4 + 4]
            if a:
                out[(y + 1) * stride + x + 1] = (r >> 3) << 11 | (g >> 2) << 5 | (b >> 3)
    return struct.pack('<' + 'H' * len(out), *out)


def native_destination(file, frame, converted=False):
    w, h = frame['width'], frame['height']
    mask = bytes.fromhex(frame['mask_hex'])
    packed = bytes.fromhex(frame['pixels_hex'])
    direct = file['storage'] == 'rgb565'
    if len(mask) != w * h or len(packed) != w * h * (2 if direct else 1):
        raise ValueError('Native pixel/mask length differs from dimensions')
    pixels = [p[0] for p in struct.iter_unpack('<H', packed)] if direct else list(packed)
    palette = None if direct else bytes.fromhex(file['palettes_rgb_hex'][frame['palette_index']])
    out = [0x1234] * ((w + 2) * (h + 2))
    for i, (opaque, pixel) in enumerate(zip(mask, pixels)):
        if opaque not in (0, 1) or (not opaque and pixel != 0):
            raise ValueError('Invalid native mask or nonzero transparent slot')
        if not opaque:
            continue
        if direct:
            colour = ((pixel >> 1) & 0x7fe0) | (pixel & 31) if converted else pixel
        else:
            r, g, b = palette[pixel * 3:pixel * 3 + 3]
            colour = (r >> 3) << 11 | (g >> 2) << 5 | (b >> 3)
        out[(i // w + 1) * (w + 2) + i % w + 1] = colour
    return struct.pack('<' + 'H' * len(out), *out)


def compare_native(inspector, root, evidence, files, cases):
    inspector_hash = sha(inspector.read_bytes())
    samples = {}
    for case in cases:
        samples.setdefault(case['path'], []).append(case['frame'])
    entries = []
    for i, file in enumerate(files):
        mixed = ''.join(c.upper() if n % 2 else c.lower() for n, c in enumerate(file['path'])).replace('/', '\\')
        entries.append({'path': ('c:\\MagicMayhem\\' if i % 2 else '') + mixed,
                        'frames': samples.get(file['path'], [])})
    manifest = evidence / 'native-manifest.json'
    manifest.write_text(json.dumps(entries, indent=2) + '\n')
    completed = subprocess.run([str(inspector), '--root', str(root), '--prefix', 'C:/MagicMayhem',
                                '--manifest', str(manifest)], capture_output=True, text=True, timeout=120)
    (evidence / 'native-inspection.json').write_text(completed.stdout)
    (evidence / 'native-inspection.stderr').write_text(completed.stderr)
    if completed.returncode not in (0, 1):
        completed.check_returncode()
    decoded = json.loads(completed.stdout)['files']
    if len(decoded) != len(files):
        raise ValueError('Native inspection omitted inputs')
    by_name = {}
    for file, entry, native in zip(files, entries, decoded):
        if native['path'] != entry['path']:
            raise ValueError('Native request order/path differs')
        if file['version'] != 4:
            if native['status'] != 'error' or native['code'] != 'unsupportedVersion':
                raise ValueError('Native loader failed to reject legacy SPR version')
        else:
            if native['status'] != 'decoded':
                raise ValueError(f"Native decode failed for {file['path']}: {native}")
            storage = 'indexed8' if file['palettes'] else 'rgb565'
            if native['storage'] != storage or native['frame_count'] != file['frames'] or len(native['palettes_rgb_hex']) != file['palettes']:
                raise ValueError('Native container metadata differs')
            data = (root / file['path']).read_bytes()
            if native['source_bytes'] != len(data) or native['header_flags'] != uint(data, 20):
                raise ValueError('Native source size/header flags differ')
            expected_palettes = [data[24 + p * 768:24 + (p + 1) * 768].hex() for p in range(file['palettes'])]
            if native['palettes_rgb_hex'] != expected_palettes:
                raise ValueError('Native embedded palette bytes differ')
            base = 24 + file['palettes'] * 768 + file['frames'] * 4
            for frame in native['frames']:
                at = base + uint(data, 24 + file['palettes'] * 768 + frame['index'] * 4)
                raw = struct.unpack_from('<IIIii', data, at)
                expected = (raw[0], raw[1], raw[2], raw[3], raw[4], data[at + 20:at + 28].hex(),
                            uint(data, at + 28) if file['palettes'] else None,
                            [uint(data, at + 32), uint(data, at + 36)], at)
                actual = (frame['encoded_size'], frame['width'], frame['height'], frame['origin_x'], frame['origin_y'],
                          frame['name_hex'], frame['palette_index'], frame['auxiliary_offsets'], frame['source_offset'])
                if actual != expected:
                    raise ValueError('Native frame metadata differs from table-selected raw record')
        by_name[file['path']] = native
    for i, case in enumerate(cases):
        native = by_name[case['path']]
        selected = [f for f in native['frames'] if f['index'] == case['frame']]
        if len(selected) != 1:
            raise ValueError('Native sample omitted or duplicated')
        frame = selected[0]
        actual = (evidence / f'case-{i:03}.565').read_bytes()
        case['native_matches_original'] = native_destination(native, frame) == actual
        case['native_pixels_sha256'] = frame['pixels_sha256']
        case['native_mask_sha256'] = frame['mask_sha256']
        if native['storage'] == 'rgb565':
            converted = (evidence / f'case-{i:03}.555').read_bytes()
            case['native_conversion_matches_original'] = native_destination(native, frame, True) == converted
        if not case['native_matches_original'] or case.get('native_conversion_matches_original') is False:
            raise ValueError(f"Native/original mismatch: {case['path']} frame {case['frame']}")
    if sha(inspector.read_bytes()) != inspector_hash:
        raise ValueError('Native inspector changed during comparison')
    supported = [f for f in decoded if f['status'] == 'decoded']
    return {'inspector_sha256': inspector_hash,
            'loader_source_sha256': sha((REPO / 'assets/sprite_loader.cpp').read_bytes()),
            'header_sha256': sha((REPO / 'assets/sprite_loader.hpp').read_bytes()),
            'decoded_files': len(supported), 'legacy_files_rejected': len(decoded) - len(supported),
            'decoded_frames': sum(f['frame_count'] for f in supported),
            'empty_frames': sum(f['empty_frames'] for f in supported),
            'decoded_pixels': sum(f['pixels'] for f in supported),
            'original_draw_matches': sum(c['native_matches_original'] for c in cases),
            'original_conversion_matches': sum(c.get('native_conversion_matches_original', False) for c in cases),
            'request_modes': ['mixed_case_relative_windows', 'mixed_case_aliased_windows'],
            'input_closed_before_inspection': True,
            'original_palette_builder_executed': False, 'live_game_validated': False}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--root', type=Path, default=REPO / 'working/game-clean')
    parser.add_argument('--source', type=Path, default=REPO / 'working/MMSprite')
    parser.add_argument('--native-inspector', type=Path, help='Also validate the compiled native SPR inspector')
    args = parser.parse_args()
    root, source = args.root.resolve(), args.source.resolve()
    helpers = runpy.run_path(str(REPO / 'tools/evaluate-mmsprite.py'))
    upstream = helpers['source_inventory'](source)
    lock = json.loads(helpers['LOCK'].read_text())
    if upstream != lock['tracked_file_sha256']:
        raise ValueError('MMSprite source lock mismatch')
    executable = REPO / 'working/game-nocd/Chaos.exe'
    binary = executable.read_bytes()
    if sha(binary) != HASH:
        raise ValueError('Unsupported binary hash')
    clean_executable = REPO / 'working/game-clean/Chaos.exe'
    if sha(clean_executable.read_bytes()) != CLEAN_HASH:
        raise ValueError('Unsupported clean binary hash')
    parent = REPO / 'working/tests/sprite-binary'
    parent.mkdir(parents=True, exist_ok=True)
    evidence = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    print(f'Sprite binary evidence: {evidence}', flush=True)

    def verify(phase):
        result = subprocess.run([str(REPO / 'tools/original-manifest.sh'), 'verify'], capture_output=True, text=True)
        (evidence / f'original-{phase}.log').write_text(result.stdout + result.stderr)
        print(result.stdout, end='', flush=True)
        result.check_returncode()

    verify('before')
    try:
        snapshot = evidence / 'reference.exe'
        snapshot.write_bytes(binary)
        assembly = subprocess.check_output(['objdump', '-d', '-Mintel', str(executable)], text=True)
        instructions = []
        for line in assembly.splitlines():
            match = re.match(r'\s*([0-9a-f]+):\s', line)
            if match:
                instructions.append((int(match[1], 16), line))
        for name, (start, end) in RANGES.items():
            (evidence / f'{name}.asm').write_text('\n'.join(s for va, s in instructions if start <= va < end) + '\n')
        for name, (start, end) in CLEAN_RANGES.items():
            text = subprocess.check_output(['objdump', '-d', '-Mintel', f'--start-address={start}',
                                            f'--stop-address={end}', str(clean_executable)], text=True)
            (evidence / f'{name}.asm').write_text(text)
        callers = {}
        for name, (start, _) in RANGES.items():
            callers[name] = [{'va': hex(va), 'context': [s for _, s in instructions[max(0, i - 6):i + 2]]}
                             for i, (va, line) in enumerate(instructions)
                             if re.search(r'\bcall\s+0x' + f'{start:x}' + r'\b', line)]
        (evidence / 'callers.json').write_text(json.dumps(callers, indent=2) + '\n')
        harness = evidence / 'sprite-reference'
        command = [os.environ.get('CXX', 'g++'), '-m32', '-std=c++17', '-Wall', '-Wextra', '-Werror',
                   '-fno-pie', '-no-pie', str(REPO / 'tests/sprite-binary-reference.cpp'), '-o', str(harness)]
        subprocess.run(command, check=True)
        sys.dont_write_bytecode = True
        sys.path.insert(0, str(source / 'py'))
        from mm_files import SpriteFile
        files, cases = [], []
        for path in sorted(root.rglob('*')):
            if path.suffix.lower() != '.spr' or not path.is_file():
                continue
            data = path.read_bytes()
            relative = path.relative_to(root).as_posix()
            stream = io.BytesIO(data)
            sprite = SpriteFile(stream)
            table = 24 + sprite.palette_count * 768 - (4 if sprite.version == 2 else 0)
            base = table + sprite.frame_count * 4
            disagreements = sum(f.offset != base + uint(data, table + i * 4) for i, f in enumerate(sprite.frames))
            files.append({'path': relative, 'sha256': sha(data), 'version': sprite.version,
                          'palettes': sprite.palette_count, 'frames': sprite.frame_count,
                          'table_vs_sequential_offset_differences': disagreements,
                          'accepted_version_by_57d310': sprite.version == 4})
            sample = sprite.version == 4 and (sprite.palette_count == 0 or
                     relative in ('Creatures/RedCap.spr', 'Realms/Celtic/Forest/Terrain.spr', 'Sprites/effects3.spr'))
            if not sample:
                continue
            indices = {0, sprite.frame_count // 2, sprite.frame_count - 1}
            indices.add(next((i for i, f in enumerate(sprite.frames) if not f.has_valid_dimensions), -1))
            # Include each valid embedded palette in the selected indexed files.
            indices.update(next((i for i, f in enumerate(sprite.frames)
                                 if f.palette_index == p and f.has_valid_dimensions), -1)
                           for p in range(sprite.palette_count))
            for i in sorted(indices - {-1}):
                frame = sprite.frames[i]
                prefix = evidence / f'case-{len(cases):03}'
                result = subprocess.run([str(harness), str(snapshot), str(path), str(i), str(prefix)],
                                        capture_output=True, text=True, timeout=5)
                (prefix.with_suffix('.log')).write_text(result.stdout + result.stderr)
                result.check_returncode()
                actual = Path(str(prefix) + '.565').read_bytes()
                expected = model(data, frame, sprite.palettes if sprite.palette_count else None)
                case = {'path': relative, 'frame': i, 'width': frame.width, 'height': frame.height,
                        'palette_index': frame.palette_index, 'original_565_sha256': sha(actual),
                        'checked_row_model_matches_original': actual == expected,
                        'empty': not frame.has_valid_dimensions}
                try:
                    rows = frame.get_pixel_data(stream, sprite.palettes)
                    mm = mm565(rows, frame)
                    case['mmsprite_565_sha256'] = sha(mm)
                    case['mmsprite_matches_original'] = actual == mm
                    case['different_words'] = sum(a != b for a, b in zip(struct.iter_unpack('<H', actual), struct.iter_unpack('<H', mm)))
                except Exception as exc:
                    case['mmsprite_error'] = type(exc).__name__
                if not sprite.palette_count:
                    converted = Path(str(prefix) + '.555').read_bytes()
                    case['original_555_sha256'] = sha(converted)
                    case['conversion_model_matches_original'] = converted == model(data, frame, None, True)
                cases.append(case)
            if sha(path.read_bytes()) != sha(data):
                raise ValueError(f'Asset changed: {relative}')
        if not cases:
            raise ValueError('No comparison samples')
        native = compare_native(args.native_inspector.resolve(), root, evidence, files, cases) if args.native_inspector else None
        if sha(executable.read_bytes()) != HASH or sha(snapshot.read_bytes()) != HASH:
            raise ValueError('Executable changed')
        if sha(clean_executable.read_bytes()) != CLEAN_HASH:
            raise ValueError('Clean executable changed')
        if helpers['source_inventory'](source) != upstream:
            raise ValueError('MMSprite changed')
        if any(sha((root / f['path']).read_bytes()) != f['sha256'] for f in files):
            raise ValueError('Installed SPR input changed')
        report = {'executable_sha256': HASH, 'upstream_revision': helpers['REVISION'],
                  'native': native,
                  'clean_executable_sha256': CLEAN_HASH, 'clean_binary_executed': False,
                  'clean_static_ranges': {k: [hex(a), hex(b)] for k, (a, b) in CLEAN_RANGES.items()},
                  'root': str(root), 'ranges': {k: [hex(a), hex(b)] for k, (a, b) in RANGES.items()},
                  'harness_source_sha256': sha((REPO / 'tests/sprite-binary-reference.cpp').read_bytes()),
                  'harness_binary_sha256': sha(harness.read_bytes()), 'compile_command': command,
                  'runner_sha256': sha(Path(__file__).read_bytes()), 'inputs_unchanged': True,
                  'validation': 'isolated original x86 draw and conversion; static loader inspection',
                  'original_loader_executed': False, 'original_palette_builder_executed': False,
                  'live_game_validated': False, 'unshaded_indexed_palette_policy': 'fixture RGB565 table',
                  'summary': {'spr_files': len(files), 'sampled_frames': len(cases),
                              'row_model_matches': sum(c['checked_row_model_matches_original'] for c in cases),
                              'mmsprite_matches': sum(c.get('mmsprite_matches_original', False) for c in cases),
                              'mmsprite_mismatches': sum(c.get('mmsprite_matches_original') is False for c in cases),
                              'mmsprite_errors': dict(Counter(c['mmsprite_error'] for c in cases if 'mmsprite_error' in c)),
                              'word_conversion_matches': sum(c.get('conversion_model_matches_original', False) for c in cases),
                              'table_offset_differences': sum(f['table_vs_sequential_offset_differences'] for f in files)},
                  'files': files, 'cases': cases,
                  'artifacts_sha256': {p.name: sha(p.read_bytes()) for p in evidence.iterdir() if p.suffix == '.asm' or p.name == 'callers.json'}}
        (evidence / 'report.json').write_text(json.dumps(report, indent=2) + '\n')
        print(json.dumps(report['summary'], indent=2), flush=True)
        if native:
            print(json.dumps(native, indent=2), flush=True)
        if any(not c['checked_row_model_matches_original'] or
               c.get('conversion_model_matches_original') is False for c in cases):
            raise ValueError('Original routine differs from checked row/conversion model')
    finally:
        verify('after')


if __name__ == '__main__':
    main()
