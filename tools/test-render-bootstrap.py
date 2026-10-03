#!/usr/bin/env python3
"""Un-Locked RGB primaries: x86 metadata -> complete 800x600 copy -> Qt/OpenGL."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import struct
import subprocess
import tempfile
from importlib.util import spec_from_file_location, module_from_spec

REPO = Path(__file__).resolve().parents[1]


def load(name, path):
    spec = spec_from_file_location(name, REPO / path)
    module = module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def rgba(native, bits):
    masks = (0xf800, 0x7e0, 0x1f) if bits == 16 else (0xff0000, 0xff00, 0xff)
    output = bytearray()
    for at in range(0, len(native), bits // 8):
        value = int.from_bytes(native[at:at + bits // 8], 'little')
        for mask in masks:
            low = mask & -mask
            output.append(((value & mask) // low) * 255 // (mask // low))
        output.append(255)
    return bytes(output)


def main():
    cases = ('opaque', 'fast', 'legacy', 'negative', 'rgb24', 'rgb32', 'alias', 'created', 'retry',
             'nested-description', 'offscreen', 'partial', 'keyed', 'caps-missing',
             'dimensions-missing', 'format-missing', 'bad-mask', 'failed-description', 'lock-description-change')
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--case', choices=cases, action='append')
    selected = parser.parse_args().case or cases
    dll = load('bootstrap_build', 'tools/build-render-bridge.py').build(True)
    stage = load('bootstrap_stage', 'tools/prepare-shadow-experiment.py')
    oracle = load('bootstrap_oracle', 'tools/test-render-lock-blits.py')
    parent = REPO / 'working/tests/render-bootstrap'
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
        stream = case / 'frame.bin'
        with stream.open('wb') as f:
            f.write(b'MNMGL001' + struct.pack('<II', 1, 64) + bytes(48))
            f.truncate(64 + 2048 * 2048 * 4)
        shutil.copy2(dll, case / dll.name)
        (case / 'selftest.exe').write_bytes(stage.add_import((dll.parent / 'selftest.exe').read_bytes(),
            dll='MnmRender.dll', symbol_name='RenderAnchor', section_name=b'.mnmgl'))
        env = {key: value for key, value in os.environ.items() if not key.startswith('MNM_')}
        env.update(WINEPREFIX=str(REPO / 'working/tests/render-wine'), WINEDEBUG='-all',
                   QT_QPA_PLATFORM='xcb', LIBGL_ALWAYS_SOFTWARE='1', MNM_BOOTSTRAP_SELFTEST=mode,
                   MNM_RENDER_STREAM='Z:' + str(stream).replace('/', '\\'),
                   MNM_RENDER_LOCK_CAPTURE_DIR='Z:' + str(capture).replace('/', '\\'))
        with (case / 'wine.log').open('w') as log:
            subprocess.run(['wine', str(case / 'selftest.exe')], cwd=case, env=env,
                           stdout=log, stderr=log, check=True, timeout=30)
        accepted = mode not in ('partial', 'keyed', 'caps-missing', 'dimensions-missing',
                               'format-missing', 'bad-mask', 'failed-description', 'lock-description-change')
        files = sorted(capture.glob('blit-*.bin'))
        assert len(files) == (2 if accepted else 0), (mode, files)
        # Only source and sprite Locks are allowed; the destination is never Locked.
        locks = sorted(capture.glob('lock-*.bin'))
        assert len(locks) == (2 if accepted else 0 if mode == 'lock-description-change' else 1)
        bits = 24 if mode == 'rgb24' else 32 if mode == 'rgb32' else 16
        replays = []
        offset = mode in ('retry', 'nested-description')
        for index, source in enumerate(files, 1):
            predicted = oracle.cpu_replay(source.read_bytes())
            expected = (case / f'original-{index + offset:08x}.bin').read_bytes()
            assert predicted == expected, (mode, index, 'CPU differs from original engine')
            if index == 1:
                data = source.read_bytes()
                at = 16
                # Check that the synthetic destination-before contains only zeros;
                # this is allowed solely because the first BLIT covers every pixel.
                for sid in (1, 2):
                    op, seq, length = struct.unpack_from('<III', data, at)
                    assert op == 1 and seq == sid
                    if sid == 2:
                        assert data[at + 12 + 28:at + 12 + length] == bytes(800 * 600 * (bits // 8))
                    at += 12 + length
            output = case / f'gpu-{index}.bin'
            result = subprocess.run(['xvfb-run', '-a', str(build / 'renderer/mnm-render-commands'), str(source),
                                     '--output', str(output)], env=env, capture_output=True, text=True, timeout=30)
            (case / f'gpu-{index}.log').write_text(result.stdout + result.stderr)
            assert result.returncode == 0, (mode, result.returncode, result.stdout, result.stderr)
            report = json.loads(result.stdout)
            assert result.returncode == 0 and output.read_bytes() == expected, (mode, report, result.stderr)
            replays.append(report)
        events = (case / 'events.bin').read_bytes()
        frame_count, last_pixels = 0, b''
        published = set(range(1 + offset, 3 + offset)) if accepted and mode != 'offscreen' else set()
        for draw, count in struct.iter_unpack('<II', events):
            raw = (case / f'frame-{draw:08x}.bin').read_bytes()
            header = struct.unpack('<16I', raw[:64])
            if draw in published:
                frame_count += 1
                last_pixels = rgba((case / f'original-{draw:08x}.bin').read_bytes(), bits)
            assert count == frame_count and header[10] == frame_count and header[4] == frame_count * 2
            assert raw[64:] == last_pixels, (mode, draw, 'RGBA differs from original engine')
            if frame_count:
                assert header[5:10] == (800, 600, 3200, 1, 1)
        qt_readback = False
        if frame_count:
            result = subprocess.run(['xvfb-run', '-a', str(build / 'mnm-qt-shell'), '--stream-test', str(stream)],
                                    env=env, capture_output=True, text=True, timeout=20)
            assert result.returncode == 0, (mode, result.stderr)
            qt_readback = True
        reasons = {line.split()[0] for line in (capture / 'lifecycle.log').read_text().splitlines()}
        if accepted:
            assert 'blit_initialized' in reasons and 'blit_propagated' in reasons
        else:
            assert 'blit_initialized' not in reasons
        reports.append({'mode': mode, 'commands': len(files), 'frames': frame_count,
                        'destination_locks': 0, 'qt_readback': qt_readback, 'reasons': sorted(reasons), 'replays': replays})
        print(f'Bootstrap fixture {mode}: passed', flush=True)
    (root / 'report.json').write_text(json.dumps({'origin': 'synthetic_x86_unlocked_primaries',
        'architecture': 'PE32 i386', 'dll_sha256': hashlib.sha256(dll.read_bytes()).hexdigest(),
        'fixtures': reports}, indent=2) + '\n')
    print(f'Primary bootstrap passed: {root}')


if __name__ == '__main__':
    main()
