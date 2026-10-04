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


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--root', type=Path, default=REPO / 'working/game-clean')
    parser.add_argument('--source', type=Path, default=REPO / 'working/MMSprite')
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
        if sha(executable.read_bytes()) != HASH or sha(snapshot.read_bytes()) != HASH:
            raise ValueError('Executable changed')
        if sha(clean_executable.read_bytes()) != CLEAN_HASH:
            raise ValueError('Clean executable changed')
        if helpers['source_inventory'](source) != upstream:
            raise ValueError('MMSprite changed')
        if any(sha((root / f['path']).read_bytes()) != f['sha256'] for f in files):
            raise ValueError('Installed SPR input changed')
        report = {'executable_sha256': HASH, 'upstream_revision': helpers['REVISION'],
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
        if any(not c['checked_row_model_matches_original'] or
               c.get('conversion_model_matches_original') is False for c in cases):
            raise ValueError('Original routine differs from checked row/conversion model')
    finally:
        verify('after')


if __name__ == '__main__':
    main()
