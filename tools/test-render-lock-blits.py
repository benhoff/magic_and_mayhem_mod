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
    assert sequence == 8 and output is not None
    return output


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--case", action="append", help="Run only named fixtures")
    selected = parser.parse_args().case
    dll = load('lock_blit_build', 'tools/build-render-bridge.py').build(True)
    stage = load('lock_blit_stage', 'tools/prepare-shadow-experiment.py')
    parent = REPO / 'working/tests/render-lock-blits'
    parent.mkdir(parents=True, exist_ok=True)
    root = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    build = REPO / 'working/build/renderer'
    subprocess.run(['cmake', '-S', str(REPO / 'renderer'), '-B', str(build)], check=True)
    subprocess.run(['cmake', '--build', str(build), '--target', 'mnm-render-commands', '--parallel', '4'], check=True)
    reports = []
    counts = {'chain': 2, 'keyed': 1, 'alias': 1, 'failed': 1, 'update': 3,
              'rgb24': 2, 'rgb32': 2, 'reseed': 1, 'key-failed': 1, 'budget': 16,
              'unsupported': 0, 'self': 0, 'untracked': 0, 'clipper': 0, 'restore': 0,
              'key-removed': 0, 'release': 0, 'bounds': 0, 'subrect': 1, 'reentrant': 0, 'created': 1, 'alias-conflict': 0, 'unlock-restore': 0}
    if selected:
        assert set(selected) <= counts.keys(), selected
        counts = {mode: counts[mode] for mode in selected}
    for mode, count in counts.items():
        case = root / mode
        capture = case / 'capture'
        capture.mkdir(parents=True)
        frame = case / 'frame.bin'
        with frame.open('wb') as f:
            f.write(b'MNMGL001' + struct.pack('<II', 1, 64) + bytes(48))
            f.truncate(64 + 2048 * 2048 * 4)
        shutil.copy2(dll, case / dll.name)
        (case / 'selftest.exe').write_bytes(stage.add_import((dll.parent / 'selftest.exe').read_bytes(),
            dll='MnmRender.dll', symbol_name='RenderAnchor', section_name=b'.mnmgl'))
        env = {k: v for k, v in os.environ.items() if not k.startswith('MNM_')}
        env.update(WINEPREFIX=str(REPO / 'working/tests/render-wine'), WINEDEBUG='-all',
                   MNM_LOCK_BLIT_SELFTEST=mode, MNM_RENDER_STREAM='Z:' + str(frame).replace('/', '\\'),
                   MNM_RENDER_LOCK_CAPTURE_DIR='Z:' + str(capture).replace('/', '\\'),
                   QT_QPA_PLATFORM='xcb', LIBGL_ALWAYS_SOFTWARE='1')
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
        assert header[10] == 0, (mode, 'offscreen copies must not publish live frames')
        reasons = {line.split()[0] for line in (capture / 'lifecycle.log').read_text().splitlines()}
        if count:
            assert 'blit_propagated' in reasons
        else:
            assert 'blit_invalidated' in reasons
        if mode == 'failed':
            assert 'blit_failed' in reasons
        reports.append({'mode': mode, 'commands': count, 'replays': replays, 'reasons': sorted(reasons)})
        print(f'Lock blit fixture {mode}: passed', flush=True)
    (root / 'report.json').write_text(json.dumps({'origin': 'synthetic_game_owned_blits',
        'architecture': 'PE32 i386', 'dll_sha256': hashlib.sha256(dll.read_bytes()).hexdigest(),
        'fixtures': reports}, indent=2) + '\n')
    print(f'Game-owned blit propagation passed: {root}')


if __name__ == '__main__':
    main()
