#!/usr/bin/env python3
"""Offline PE32 game-owned pixels -> blit propagation -> independent OpenGL replay."""
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import shutil
import struct
import subprocess
import tempfile

REPO = Path(__file__).resolve().parents[1]

# Shared wire definitions are repository-local; no package installation required.
import sys
sys.path.insert(0, str(REPO / "protocols/python"))
from mnm_protocols import frame_v1 as frame_protocol



def load(name, path):
    spec = importlib.util.spec_from_file_location(name, REPO / path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def cpu_replay(data):
    assert data[:16] == b'MNMCMD01' + struct.pack('<II', 1, 16)
    at, sequence, surfaces, output = 16, 0, {}, None
    while at < len(data):
        op, seq, size = struct.unpack_from('<III', data, at)
        at += 12
        sequence += 1
        assert seq == sequence and size <= len(data) - at
        payload = data[at:at + size]
        at += size
        if op == 1:
            sid, width, height, bits, *masks = struct.unpack_from('<7I', payload)
            pixels = bytearray(payload[28:])
            assert len(pixels) == width * height * (bits // 8)
            surfaces[sid] = (width, height, bits // 8, pixels)
        elif op == 2:
            sid, x, y, width, height = struct.unpack_from('<5I', payload)
            sw, sh, stride, pixels = surfaces[sid]
            assert width and height and x + width <= sw and y + height <= sh
            assert len(payload) == 20 + width * height * stride
            for row in range(height):
                begin = ((y + row) * sw + x) * stride
                patch = 20 + row * width * stride
                pixels[begin:begin + width * stride] = payload[patch:patch + width * stride]
        elif op == 3:
            source, target, sx, sy, right, bottom, dx, dy, keyed, key = struct.unpack('<10I', payload)
            width, height = right - sx, bottom - sy
            sw, sh, stride, src = surfaces[source]
            dw, dh, dst_stride, dst = surfaces[target]
            assert stride == dst_stride and sx + width <= sw and sy + height <= sh
            assert dx + width <= dw and dy + height <= dh
            for y in range(height):
                for x in range(width):
                    a = ((sy + y) * sw + sx + x) * stride
                    b = ((dy + y) * dw + dx + x) * stride
                    pixel = src[a:a + stride]
                    if not keyed or int.from_bytes(pixel, 'little') != key:
                        dst[b:b + stride] = pixel
        elif op == 4:
            sid, first, count = struct.unpack_from('<3I', payload)
            assert sid in surfaces and count and first < 256 and count <= 256 - first and len(payload) == 12 + count * 3
        elif op == 11:
            a, b = struct.unpack('<2I', payload)
            assert a != b and surfaces[a][:3] == surfaces[b][:3]
            surfaces[a], surfaces[b] = surfaces[b], surfaces[a]
        elif op == 5:
            sid, = struct.unpack_from('<I', payload)
            assert payload[4:] == surfaces[sid][3]
        elif op == 6:
            sid, = struct.unpack('<I', payload)
            output = bytes(surfaces[sid][3])
        elif op == 7:
            sid, = struct.unpack('<I', payload)
            del surfaces[sid]
        else:
            assert op == 8 and size == 0 and at == len(data) and not surfaces
    assert sequence >= 6 and output is not None
    return output


def check_primary_frames(case, mode, count, unknown):
    bits = 24 if mode == 'rgb24' else 32 if mode == 'rgb32' else 16
    masks = (0xf800, 0x7e0, 0x1f) if bits == 16 else (0xff0000, 0xff00, 0xff)
    def rgba(native):
        out = bytearray()
        for at in range(0, len(native), bits // 8):
            value = int.from_bytes(native[at:at + bits // 8], 'little')
            for mask in masks:
                low = mask & -mask
                out.append(((value & mask) // low) * 255 // (mask // low))
            out.append(255)
        return bytes(out)
    primary_c = mode in ('chain', 'update', 'rgb24', 'rgb32', 'budget')
    seed_ids = {3 if primary_c else 1 if mode == 'untracked' else 2}
    if mode == 'reseed':
        seed_ids.add(4)
    draw_ids = {index + (mode in ('failed', 'reseed')) for index in range(1, (18 if mode == 'budget' else count) + 1)}
    if primary_c:
        draw_ids &= set(range(2, 19)) if mode == 'budget' else {3 if mode == 'update' else 2}
    last_counter, last_pixels = 0, b''
    events = (case / 'frame-events.bin').read_bytes()
    for seed, index in struct.iter_unpack('<II', events):
        prefix = 'seed-frame' if seed else 'draw-frame'
        raw = (case / f'{prefix}-{index:08x}.bin').read_bytes()
        header = struct.unpack('<16I', raw[:64])
        publish = not unknown and index in (seed_ids if seed else draw_ids)
        expected_counter = last_counter + int(publish)
        assert header[10] == expected_counter and header[4] == expected_counter * 2, (mode, seed, index, header)
        if publish:
            native = (case / f'seed-frame-{index:08x}.raw' if seed else case / f'original-{index:08x}.bin').read_bytes()
            last_pixels = rgba(native)
        assert raw[64:] == last_pixels, (mode, seed, index, 'RGBA differs from independent engine pixels')
        if expected_counter:
            assert header[5:10] == (4, 3, 16, 1, 1), (mode, header)
        last_counter = expected_counter
    with (case / 'frame.bin').open('rb') as f:
        final = f.read(64 + len(last_pixels))
    assert final == raw and last_counter == (0 if unknown else len(seed_ids) + len(draw_ids))
    return last_counter


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--case", action="append", help="Run only named fixtures")
    parser.add_argument("--primary", action="store_true", help="Also validate live primary frame publication and Qt readback")
    parser.add_argument("--unknown-primary-caps", action="store_true", help="Omit returned caps provenance; no frame may publish")
    args = parser.parse_args()
    if args.unknown_primary_caps and not args.primary:
        parser.error("--unknown-primary-caps requires --primary")
    selected = args.case
    dll = load('lock_blit_build', 'tools/build-render-bridge.py').build(True)
    stage = load('lock_blit_stage', 'tools/prepare-shadow-experiment.py')
    parent = REPO / 'working/tests/render-lock-blits'
    parent.mkdir(parents=True, exist_ok=True)
    root = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    build = REPO / 'working/build/renderer'
    subprocess.run(['cmake', '-S', str(REPO / 'renderer'), '-B', str(build)], check=True)
    subprocess.run(['cmake', '--build', str(build), '--target', 'mnm-render-commands', '--parallel', '4'], check=True)
    qt_build = REPO / 'working/build/qt-shell'
    if args.primary:
        subprocess.run(['cmake', '-S', str(REPO / 'apps/qt-shell'), '-B', str(qt_build)], check=True)
        subprocess.run(['cmake', '--build', str(qt_build), '--target', 'mnm-qt-shell', '--parallel', '4'], check=True)
    qt_layouts = []
    if args.primary:
        layout_env = os.environ.copy()
        layout_env.update(QT_QPA_PLATFORM='xcb', LIBGL_ALWAYS_SOFTWARE='1')
        for width, height in ((800, 600), (129, 257)):
            layout = root / f'layout-{width}x{height}.bin'
            header = bytearray(frame_protocol.initial_header())
            header[:frame_protocol.MAGIC_SIZE] = frame_protocol.MAGIC
            struct.pack_into('<7I', header, frame_protocol.SEQUENCE_OFFSET, 2, width, height,
                             width * frame_protocol.BYTES_PER_PIXEL, frame_protocol.PIXEL_FORMAT_RGBA8888,
                             frame_protocol.STATUS_FRAME_PUBLISHED, 1)
            pixels = bytes(channel for y in range(height) for x in range(width)
                           for channel in ((x * 17 + y * 11) & 255, (x * 3 + y * 19) & 255, (x ^ y) & 255, 255))
            with layout.open('wb') as f:
                f.write(header)
                f.write(pixels)
                f.truncate(frame_protocol.SIZE)
            result = subprocess.run(['xvfb-run', '-a', str(qt_build / 'mnm-qt-shell'), '--stream-test', str(layout)],
                                    env=layout_env, capture_output=True, text=True, timeout=20)
            assert result.returncode == 0, (width, height, result.stderr)
            qt_layouts.append({'width': width, 'height': height, 'qt_readback': True})
    reports = []
    counts = {'chain': 2, 'keyed': 1, 'alias': 1, 'failed': 1, 'update': 3,
              'rgb24': 2, 'rgb32': 2, 'reseed': 1, 'key-failed': 1, 'budget': 16,
              'unsupported': 0, 'self': 0, 'untracked': 0, 'clipper': 0, 'restore': 0,
              'key-removed': 0, 'release': 0, 'bounds': 0, 'subrect': 1, 'reentrant': 0, 'created': 1, 'alias-conflict': 0, 'unlock-restore': 0, 'destination-alias': 1}
    if selected:
        assert set(selected) <= counts.keys(), selected
        counts = {mode: counts[mode] for mode in selected}
    for mode, count in counts.items():
        case = root / mode
        capture = case / 'capture'
        capture.mkdir(parents=True)
        frame = case / 'frame.bin'
        with frame.open('wb') as f:
            f.write(frame_protocol.initial_header())
            f.truncate(frame_protocol.SIZE)
        shutil.copy2(dll, case / dll.name)
        (case / 'selftest.exe').write_bytes(stage.add_import((dll.parent / 'selftest.exe').read_bytes(),
            dll='MnmRender.dll', symbol_name='RenderAnchor', section_name=b'.mnmgl'))
        env = {k: v for k, v in os.environ.items() if not k.startswith('MNM_')}
        env.update(WINEPREFIX=str(REPO / 'working/tests/render-wine'), WINEDEBUG='-all',
                   MNM_LOCK_BLIT_SELFTEST=mode, MNM_RENDER_STREAM='Z:' + str(frame).replace('/', '\\'),
                   MNM_RENDER_LOCK_CAPTURE_DIR='Z:' + str(capture).replace('/', '\\'),
                   QT_QPA_PLATFORM='xcb', LIBGL_ALWAYS_SOFTWARE='1')
        if args.primary:
            env['MNM_LOCK_BLIT_PRIMARY_SELFTEST'] = '1'
            if args.unknown_primary_caps:
                env['MNM_LOCK_BLIT_PRIMARY_UNKNOWN_SELFTEST'] = '1'
        with (case / 'wine.log').open('w') as log:
            subprocess.run(['wine', str(case / 'selftest.exe')], cwd=case, env=env,
                           stdout=log, stderr=log, check=True, timeout=30)
        if mode == 'unlock-restore':
            assert len(list(capture.glob('lock-*.bin'))) == 2
        files = sorted(capture.glob('blit-*.bin'))
        assert len(files) == count, (mode, files, (capture / 'lifecycle.log').read_text())
        replays = []
        for index, source in enumerate(files, 1):
            output = cpu_replay(source.read_bytes())
            original_index = index + (mode in ('failed', 'reseed'))
            assert output == (case / f'original-{original_index:08x}.bin').read_bytes(), (mode, index)
            target = case / f'gpu-{index:08x}.bin'
            result = subprocess.run(['xvfb-run', '-a', str(build / 'mnm-render-commands'), str(source),
                                     '--output', str(target)], env=env, capture_output=True, text=True, timeout=20)
            report = json.loads(result.stdout)
            assert result.returncode == 0, (mode, report, result.stderr)
            assert target.read_bytes() == output and report['checks'] == 1 and report['presentations'] == 1
            replays.append(report)
        with frame.open('rb') as f:
            header = struct.unpack('<16I', f.read(64))
        if args.primary:
            frames = check_primary_frames(case, mode, count, args.unknown_primary_caps)
            if frames:
                qt = subprocess.run(['xvfb-run', '-a', str(qt_build / 'mnm-qt-shell'), '--stream-test', str(frame)],
                                    env=env, capture_output=True, text=True, timeout=20)
                assert qt.returncode == 0, (mode, qt.stderr)
        else:
            frames = 0
            assert header[10] == 0, (mode, 'offscreen copies must not publish live frames')
        reasons = {line.split()[0] for line in (capture / 'lifecycle.log').read_text().splitlines()}
        if count:
            assert 'blit_propagated' in reasons
        else:
            assert 'blit_invalidated' in reasons
        if mode == 'failed':
            assert 'blit_failed' in reasons
        reports.append({'mode': mode, 'commands': count, 'replays': replays, 'primary_frames': frames, 'qt_readback': args.primary and frames > 0, 'reasons': sorted(reasons)})
        print(f'Lock blit fixture {mode}: passed', flush=True)
    (root / 'report.json').write_text(json.dumps({'origin': 'synthetic_game_owned_blits',
        'architecture': 'PE32 i386', 'primary_publication': args.primary, 'unknown_primary_caps': args.unknown_primary_caps, 'qt_layouts': qt_layouts, 'dll_sha256': hashlib.sha256(dll.read_bytes()).hexdigest(),
        'fixtures': reports}, indent=2) + '\n')
    print(f'Game-owned blit propagation passed: {root}')


if __name__ == '__main__':
    main()
