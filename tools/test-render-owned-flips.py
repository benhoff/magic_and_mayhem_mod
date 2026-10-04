#!/usr/bin/env python3
"""Observed x86 double-buffer Flip routing without observer surface calls."""
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

# Shared wire definitions are repository-local; no package installation required.
import sys
sys.path.insert(0, str(REPO / "protocols/python"))
from mnm_protocols import frame_v1 as frame_protocol



def load(name, path):
    spec = spec_from_file_location(name, REPO / path)
    module = module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def main():
    cases = ('flip-rotate', 'flip-target', 'flip-retry', 'flip-no-reseed',
             'flip-chain', 'flip-flags', 'flip-unobserved', 'flip-attachment-failed',
             'flip-wrong-target', 'flip-nested', 'flip-alias', 'flip-count-missing', 'flip-budget', 'flip-mutated')
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--case', choices=cases, action='append')
    selected = parser.parse_args().case or cases
    dll = load('flip_build', 'tools/build-render-bridge.py').build(True)
    stage = load('flip_stage', 'tools/prepare-shadow-experiment.py')
    bootstrap = load('flip_bootstrap', 'tools/test-render-bootstrap.py')
    parent = REPO / 'working/tests/render-owned-flips'
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
            f.write(frame_protocol.initial_header())
            f.truncate(frame_protocol.SIZE)
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
        counts = ([1, 2, 2, 3] + list(range(4, 17)) + [16, 16]) if mode == 'flip-budget' else [1, 2, 2, 3] if mode in ('flip-rotate', 'flip-target', 'flip-alias') else (
            [0, 1, 2, 2, 3] if mode == 'flip-retry' else [1, 1] if mode == 'flip-no-reseed' else [0])
        events = list(struct.iter_unpack('<II', (case / 'events.bin').read_bytes()))
        assert [count for draw, count in events] == counts, (mode, events)
        pixels, previous = b'', 0
        for draw, count in events:
            raw = (case / f'frame-{draw:08x}.bin').read_bytes()
            header = struct.unpack('<16I', raw[:64])
            if count > previous:
                pixels = bootstrap.rgba((case / f'original-{draw:08x}.bin').read_bytes(), 16)
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
        assert ('flip_presented' in reasons) == bool(previous)  # Lifecycle diagnostics deduplicate repeats.
        reports.append({'mode': mode, 'frames': previous, 'counts': counts, 'qt_readback': bool(previous),
                        'reasons': sorted(set(reasons))})
        print(f'Owned Flip fixture {mode}: passed', flush=True)
    (root / 'report.json').write_text(json.dumps({'origin': 'synthetic_x86_owned_double_buffers',
        'architecture': 'PE32 i386', 'dll_sha256': hashlib.sha256(dll.read_bytes()).hexdigest(),
        'fixtures': reports}, indent=2) + '\n')
    print(f'Owned Flip routing passed: {root}')


if __name__ == '__main__':
    main()
