#!/usr/bin/env python3
"""Owned x86 indexed primary pixels and application-observed palette changes."""
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


def main():
    expected = {
        'idx-entries': [0, 0, 1, 2], 'idx-create': [0, 1, 2],
        'idx-alias': [0, 0, 1, 2], 'idx-partial': [0, 0, 0, 1, 2],
        'idx-get-palette': [0, 0, 1, 2], 'idx-reattach': [0, 0, 1, 2, 2, 3, 3, 4],
        'idx-retry': [0, 0, 1, 1, 2], 'idx-detach': [0, 0, 1, 1],
        'idx-flags': [0, 0, 1, 1], 'idx-nested': [0, 0, 1, 1],
        'idx-initialize': [0, 0, 1, 2, 2, 2],
        'idx-offscreen': [0, 0, 0, 0], 'idx-caps': [0, 0, 0, 0],
        'idx-caps-missing': [0, 0, 0, 0], 'idx-read-failed': [0, 0, 0, 0],
        'idx-assign-failed': [0, 0, 0, 0],
        'idx-descriptor': [0, 0, 1, 2, 2, 3],
        'idx-negative': [0, 0, 1, 2], 'idx-legacy': [0, 0, 1, 2],
        'idx-before-lock': [0, 0, 0, 1, 2],
        'idx-release': [0, 0, 1, 2, 2, 2, 2, 2, 3],
        'idx-budget': [0, 0, 1, 2] + list(range(3, 17)) + [16, 16],
    }
    cases = tuple(expected)
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--case', choices=cases, action='append')
    selected = parser.parse_args().case or cases
    dll = load('indexed_build', 'tools/build-render-bridge.py').build(True)
    stage = load('indexed_stage', 'tools/prepare-shadow-experiment.py')
    parent = REPO / 'working/tests/render-owned-palettes'
    parent.mkdir(parents=True, exist_ok=True)
    root = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    build = REPO / 'working/build/qt-shell'
    subprocess.run(['cmake', '-S', str(REPO / 'apps/qt-shell'), '-B', str(build)], check=True)
    subprocess.run(['cmake', '--build', str(build), '--target', 'mnm-qt-shell', '--parallel', '4'], check=True)
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
        counts = expected[mode]
        events = list(struct.iter_unpack('<II', (case / 'events.bin').read_bytes()))
        assert [count for draw, count in events] == counts, (mode, events)
        pixels, previous = b'', 0
        for draw, count in events:
            raw = (case / f'frame-{draw:08x}.bin').read_bytes()
            header = struct.unpack('<16I', raw[:64])
            if count > previous:
                native = (case / f'original-{draw:08x}.bin').read_bytes()
                colors = (case / f'colors-{draw:08x}.bin').read_bytes()
                lookup = tuple(colors[i * 4:i * 4 + 3] + b'\xff' for i in range(256))
                pixels = b''.join(lookup[index] for index in native)
            assert header[4] == 2 * count and header[10] == count and raw[64:] == pixels, (mode, draw)
            if count:
                assert header[5:10] == (800, 600, 3200, 1, 1)
            previous = count
        if previous:
            result = subprocess.run(['xvfb-run', '-a', str(build / 'mnm-qt-shell'), '--stream-test', str(stream)],
                                    env=env, capture_output=True, text=True, timeout=20)
            (case / 'qt.log').write_text(result.stdout + result.stderr)
            assert result.returncode == 0, (mode, result.stderr)
        reasons = [line.split()[0] for line in (capture / 'lifecycle.log').read_text().splitlines()]
        assert ('indexed_presented' in reasons) == bool(previous)  # Lifecycle diagnostics deduplicate repeats.
        reports.append({'mode': mode, 'frames': previous, 'counts': counts, 'qt_readback': bool(previous),
                        'reasons': sorted(set(reasons))})
        print(f'Owned palette fixture {mode}: passed', flush=True)
    (root / 'report.json').write_text(json.dumps({'origin': 'synthetic_x86_owned_indexed_palettes',
        'architecture': 'PE32 i386', 'dll_sha256': hashlib.sha256(dll.read_bytes()).hexdigest(),
        'fixtures': reports}, indent=2) + '\n')
    print(f'Owned indexed palettes passed: {root}')


if __name__ == '__main__':
    main()
