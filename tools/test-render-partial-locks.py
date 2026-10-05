#!/usr/bin/env python3
"""Test partial CPU Lock merges through real PE32 hooks without launching Chaos."""
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

CASES = ('modern', 'legacy', 'surface2', 'surface7', 'negative', 'rgb24', 'rgb32',
         'indexed', 'chain', 'retry', 'failed-lock', 'no-base', 'readonly', 'discard',
         'layout', 'changed', 'argument', 'invalidate', 'budget', 'blit', 'memory-budget',
         'indexed-palette', 'indexed-palette-change', 'replay-budget', 'file-failed')
REJECTED = {'no-base', 'readonly', 'discard', 'layout', 'changed', 'argument', 'invalidate'}


def load(name, path):
    spec = importlib.util.spec_from_file_location(name, REPO / path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def rgba(native, bits, masks):
    result = bytearray()
    for (value,) in struct.iter_unpack({16: '<H', 32: '<I'}.get(bits, '<3s'), native):
        if bits == 24:
            value = int.from_bytes(value, 'little')
        for mask in masks:
            shift = (mask & -mask).bit_length() - 1
            maximum = mask >> shift
            result.append((((value & mask) >> shift) * 255) // maximum)
        result.append(255)
    return bytes(result)


def records(data):
    assert data[:16] == b'MNMCMD01' + struct.pack('<II', 1, 16)
    at = 16
    result = []
    while at < len(data):
        op, sequence, length = struct.unpack_from('<III', data, at)
        at += 12
        assert sequence == len(result) + 1 and length <= len(data) - at
        result.append((op, data[at:at + length], at))
        at += length
    return result


def resolved(native, bits, masks, colors=None):
    if bits != 8:
        return rgba(native, bits, masks)
    lookup = tuple(colors[i * 4:i * 4 + 3] + b'\xff' for i in range(256))
    return b''.join(lookup[index] for index in native)


def check_updates(case, capture, commits, bits, masks, mode, build, env, oracle):
    expected = {f'update-{serial:08x}.bin': (step, commits[serial - 2])
                for serial, step in enumerate(commits, 1) if serial > 1 and mode not in ('indexed', 'file-failed')}
    if mode == 'replay-budget':
        expected = {name: steps for name, steps in expected.items() if int(name[7:15], 16) <= 4}
    if mode == 'file-failed':
        assert (capture / 'update-00000002.bin').read_bytes() == b'existing-file-sentinel'
        actual = {p.name for p in capture.glob('update-*.bin')} - {'update-00000002.bin'}
    else:
        actual = {p.name for p in capture.glob('update-*.bin')}
    assert actual == set(expected), (mode, expected)
    reports = []
    for name, (step, previous) in expected.items():
        command = capture / name
        data = command.read_bytes()
        rs = records(data)
        snapshot = (capture / name.replace('update-', 'lock-')).read_bytes()
        left, top, right, bottom = struct.unpack_from('<4i', snapshot, 64)
        native = (case / f'original-{step:08x}.bin').read_bytes()
        base = (case / f'original-{previous:08x}.bin').read_bytes()
        width, height = struct.unpack_from('<II', snapshot, 36)
        assert rs[0][0] == 1 and struct.unpack_from('<7I', rs[0][1]) == (1, width, height, bits, *masks)
        assert rs[0][1][28:] == base, (mode, name, 'base')
        ops = [1] + ([4] if bits == 8 else []) + [2] * (bottom - top) + [5, 6, 7, 8]
        assert [op for op, payload, offset in rs] == ops
        colors = (case / f'colors-{step:08x}.bin').read_bytes() if bits == 8 else None
        if colors is not None:
            assert rs[1][1] == struct.pack('<III', 1, 0, 256) + b''.join(colors[i * 4:i * 4 + 3] for i in range(256))
        for y, (op, payload, offset) in enumerate(record for record in rs if record[0] == 2):
            assert struct.unpack_from('<5I', payload) == (1, left, top + y, right - left, 1)
            start = ((top + y) * width + left) * (bits // 8)
            assert payload[20:] == native[start:start + (right - left) * (bits // 8)]
        assert oracle.cpu_replay(data) == native, (mode, name, 'CPU update')
        output = case / f'{name}.native'
        result = subprocess.run(['xvfb-run', '-a', str(build / 'renderer/mnm-render-commands'), str(command),
                                 '--output', str(output)], env=env, capture_output=True, text=True, timeout=30)
        (case / f'{name}.gpu.log').write_text(result.stdout + result.stderr)
        assert result.returncode == 0, (mode, name, result.stdout, result.stderr)
        report = json.loads(result.stdout)
        assert output.read_bytes() == native, (mode, name, 'GPU update')
        assert report['presentation_rgba_sha256'] == hashlib.sha256(resolved(native, bits, masks, colors)).hexdigest()
        assert report['surface_stats']['uploads'] == 1 + bottom - top
        assert report['surface_stats']['copies'] == 0
        assert report['surface_stats']['palette_updates'] == int(bits == 8)
        reports.append(report)
        if mode == 'modern':
            # CHECK must compare generated pixels; poisoned expected bytes are never an upload.
            poisoned = bytearray(data)
            check = next(record for record in rs if record[0] == 5)
            poisoned[check[2] + 4] ^= 1
            bad = case / 'poisoned-check.bin'
            bad.write_bytes(poisoned)
            bad_output = case / 'poisoned-check.native'
            result = subprocess.run(['xvfb-run', '-a', str(build / 'renderer/mnm-render-commands'), str(bad),
                                     '--output', str(bad_output)], env=env, capture_output=True, text=True, timeout=30)
            (case / 'poisoned-check.log').write_text(result.stdout + result.stderr)
            assert result.returncode == 2 and not bad_output.exists()
    return reports


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--case', choices=CASES, action='append')
    selected = parser.parse_args().case or CASES
    dll = load('partial_build', 'tools/build-render-bridge.py').build(True)
    oracle = load('partial_update_oracle', 'tools/test-render-lock-blits.py')
    stage = load('partial_stage', 'tools/prepare-shadow-experiment.py')
    parent = REPO / 'working/tests/render-partial-locks'
    parent.mkdir(parents=True, exist_ok=True)
    root = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    build = REPO / 'working/build/qt-shell'
    subprocess.run(['cmake', '-S', str(REPO / 'apps/qt-shell'), '-B', str(build)], check=True)
    subprocess.run(['cmake', '--build', str(build), '--target', 'mnm-qt-shell', 'mnm-render-commands', '--parallel', '4'], check=True)
    reports = []
    for mode in selected:
        case = root / mode
        capture = case / 'capture'
        capture.mkdir(parents=True)
        if mode == 'file-failed':
            (capture / 'update-00000002.bin').write_bytes(b'existing-file-sentinel')
        stream = case / 'frame.bin'
        with stream.open('wb') as f:
            f.write(frame_protocol.initial_header())
            f.truncate(frame_protocol.SIZE)
        shutil.copy2(dll, case / dll.name)
        (case / 'selftest.exe').write_bytes(stage.add_import((dll.parent / 'selftest.exe').read_bytes(),
            dll='MnmRender.dll', symbol_name='RenderAnchor', section_name=b'.mnmgl'))
        env = {key: value for key, value in os.environ.items() if not key.startswith('MNM_')}
        env.update(WINEPREFIX=str(REPO / 'working/tests/render-wine'), WINEDEBUG='-all',
                   QT_QPA_PLATFORM='xcb', LIBGL_ALWAYS_SOFTWARE='1', MNM_PARTIAL_LOCK_SELFTEST=mode,
                   MNM_RENDER_STREAM='Z:' + str(stream).replace('/', '\\'),
                   MNM_RENDER_LOCK_CAPTURE_DIR='Z:' + str(capture).replace('/', '\\'))
        with (case / 'wine.log').open('w') as log:
            subprocess.run(['wine', str(case / 'selftest.exe')], cwd=case, env=env,
                           stdout=log, stderr=log, check=True, timeout=30)
        if mode == 'memory-budget':
            assert not list(capture.glob('update-*.bin'))
            files = sorted(capture.glob('lock-*.bin'))
            assert len(files) == 8
            for file in files:
                raw = file.read_bytes()
                h = struct.unpack('<16I', raw[:64])
                assert raw[:8] == b'MNMLOCK1' and h[9:12] == (1024, 2048, 32)
                assert h[15] == 8 * 1024 * 1024 and raw[64:] == b'\x55' * h[15]
            reasons = {line.split()[0] for line in (capture / 'lifecycle.log').read_text().splitlines()}
            assert 'lock_partial_accepted' in reasons and 'unlock_limit' not in reasons
            reports.append({'mode': mode, 'snapshots': 8, 'native_bytes': 64 * 1024 * 1024,
                            'primary_frames': 0, 'reasons': sorted(reasons), 'qt_readback': False})
            print(f'Partial Lock fixture {mode}: passed', flush=True)
            continue
        if mode == 'replay-budget':
            files = sorted(capture.glob('lock-*.bin'))
            assert len(files) == 7
            for step, file in enumerate(files):
                raw = file.read_bytes()
                h = struct.unpack('<16I', raw[:64])
                assert h[9:12] == (1024, 2048, 32) and h[15] == 8 * 1024 * 1024
                assert raw[h[3]:] == (case / f'original-{step:08x}.bin').read_bytes()
            replays = check_updates(case, capture, list(range(7)), 32, (0xff0000, 0xff00, 0xff), mode, build, env, oracle)
            reasons = {line.split()[0] for line in (capture / 'lifecycle.log').read_text().splitlines()}
            assert 'update_limit' in reasons and 'unlock_limit' not in reasons
            assert sum(p.stat().st_size for p in capture.glob('update-*.bin')) <= 64 * 1024 * 1024
            reports.append({'mode': mode, 'snapshots': 7, 'replays': replays, 'reasons': sorted(reasons)})
            print(f'Partial Lock fixture {mode}: passed', flush=True)
            continue
        bits = {'indexed': 8, 'indexed-palette': 8, 'indexed-palette-change': 8, 'rgb24': 24, 'rgb32': 32}.get(mode, 16)
        masks = (0, 0, 0) if bits == 8 else (0xf800, 0x7e0, 0x1f) if bits == 16 else (0xff0000, 0xff00, 0xff)
        commits = [] if mode == 'no-base' else [2]
        if mode not in REJECTED:
            commits += [5 if mode in ('retry', 'failed-lock') else 4]
        if mode == 'chain':
            commits += [6]
        if mode == 'budget':
            commits = list(range(2, 33, 2))
        files = sorted(capture.glob('lock-*.bin'))
        assert len(files) == len(commits), (mode, files, commits)
        for number, (file, step) in enumerate(zip(files, commits), 1):
            raw = file.read_bytes()
            h = struct.unpack('<16I', raw[:64])
            partial = number > 1
            assert raw[:8] == (b'MNMLOCK2' if partial else b'MNMLOCK1')
            assert h[2:5] == (2 if partial else 1, 80 if partial else 64, number)
            assert h[9:12] == (4, 4, bits) and h[15] == 16 * (bits // 8)
            assert len(raw) == h[3] + h[15]
            assert raw[h[3]:] == (case / f'original-{step:08x}.bin').read_bytes(), (mode, step)
            if partial:
                rect = (0, 0, 2, 2) if mode == 'chain' and number == 3 else (1, 1, 3, 3)
                assert struct.unpack('<4i', raw[64:80]) == rect
        if mode == 'blit':
            replay = load('partial_replay', 'tools/test-render-lock-blits.py').cpu_replay
            output = replay(next(capture.glob('blit-*.bin')).read_bytes())
            assert output == (case / 'original-00000005.bin').read_bytes()
        replays = check_updates(case, capture, commits, bits, masks, mode, build, env, oracle)
        pixels = b''
        counts = []
        for frame in sorted(case.glob('frame-*.bin')):
            step = int(frame.stem[6:], 16)
            raw = frame.read_bytes()
            h = struct.unpack('<16I', raw[:64])
            count = int(step == 5) if mode == 'blit' else 0 if mode == 'indexed' else sum(commit <= step for commit in (list(range(2, 41, 2)) if mode == 'budget' else commits))
            assert h[10] == count and h[4] == count * 2, (mode, step, h, commits)
            if count and count > (counts[-1] if counts else 0):
                colors = (case / f'colors-{step:08x}.bin').read_bytes() if bits == 8 else None
                pixels = resolved((case / f'original-{step:08x}.bin').read_bytes(), bits, masks, colors)
            assert raw[64:] == pixels, (mode, step, raw[64:].hex(), pixels.hex())
            counts.append(count)
        reasons = {line.split()[0] for line in (capture / 'lifecycle.log').read_text().splitlines()}
        if len(commits) > 1:
            assert 'lock_partial_accepted' in reasons
        if mode in REJECTED:
            assert ('lock_partial' if mode in {'no-base', 'readonly', 'discard', 'layout'} else
                    'unlock_unmatched' if mode == 'invalidate' else 'unlock_argument') in reasons, (mode, reasons)
        if replays:
            assert 'update_recorded' in reasons
        if mode == 'file-failed':
            assert 'update_file_failed' in reasons
        if mode == 'indexed':
            assert 'update_palette_unobserved' in reasons
        if mode == 'retry':
            assert 'unlock_failed' in reasons
        if mode == 'budget':
            assert 'unlock_limit' not in reasons and counts[-1] == 20
        if counts[-1]:
            result = subprocess.run(['xvfb-run', '-a', str(build / 'mnm-qt-shell'), '--stream-test', str(stream)],
                                    env=env, capture_output=True, text=True, timeout=20)
            (case / 'qt.log').write_text(result.stdout + result.stderr)
            assert result.returncode == 0, (mode, result.stderr)
        reports.append({'mode': mode, 'snapshots': len(files), 'primary_frames': counts[-1],
                        'counts': counts, 'reasons': sorted(reasons), 'replays': replays, 'qt_readback': bool(counts[-1])})
        print(f'Partial Lock fixture {mode}: passed', flush=True)
    (root / 'report.json').write_text(json.dumps({'origin': 'synthetic_x86_partial_cpu_locks',
        'architecture': 'PE32 i386', 'dll_sha256': hashlib.sha256(dll.read_bytes()).hexdigest(),
        'fixtures': reports}, indent=2) + '\n')
    print(f'Partial CPU Locks passed: {root}')


if __name__ == '__main__':
    main()
