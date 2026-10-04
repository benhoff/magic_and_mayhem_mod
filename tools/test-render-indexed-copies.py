#!/usr/bin/env python3
"""Owned indexed blits and two-buffer Flips: native CPU/GPU replay and Qt colors."""
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
    copy = {mode: ([1, 2], {'blit': {1: 1, 2: 2}}) for mode in (
        'idxcopy-opaque', 'idxcopy-fast', 'idxcopy-alias', 'idxcopy-legacy',
        'idxcopy-negative', 'idxcopy-nested-palette')}
    copy.update({
        'idxcopy-retry': ([0, 1, 2], {'blit': {1: 2, 2: 3}}),
        'idxcopy-nested-assignment': ([0, 1, 2], {'blit': {1: 2, 2: 3}}),
        'idxcopy-unknown-palette': ([0, 0], {}),
        'idxcopy-late-palette': ([0, 1, 2], {'blit': {2: 3}}),
        'idxcopy-offscreen': ([0, 0], {'blit': {1: 1, 2: 2}}),
        'idxcopy-key-range': ([1, 1], {'blit': {1: 1}}),
        'idxcopy-partial': ([0], {}), 'idxcopy-keyed-bootstrap': ([0], {}),
    })
    flips = {mode: ([1, 2, 2, 3, 4, 5, 5], {'flip': {1: 2, 2: 4, 3: 5}, 'blit': {1: 3}})
             for mode in ('idxflip-rotate', 'idxflip-target', 'idxflip-alias', 'idxflip-legacy', 'idxflip-negative')}
    flips.update({
        'idxflip-retry': ([1, 1, 2, 2, 3, 4, 5, 5], {'flip': {1: 3, 2: 5, 3: 6}, 'blit': {1: 4}}),
        'idxflip-unseeded-front': ([0, 1, 1, 1, 1, 1, 1], {}),
        'idxflip-unobserved': ([1, 1], {}),
        'idxflip-unknown-palette': ([0, 0, 0, 0, 0], {'blit': {1: 3}}),
        'idxflip-late-palette': ([0, 0, 1, 1, 2, 3], {'flip': {2: 5, 3: 6}, 'blit': {1: 4}}),
    })
    expected = copy | flips
    cases = tuple(expected)
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--case', choices=cases, action='append')
    selected = parser.parse_args().case or cases
    dll = load('indexed_copy_build', 'tools/build-render-bridge.py').build(True)
    stage = load('indexed_copy_stage', 'tools/prepare-shadow-experiment.py')
    oracle = load('indexed_copy_oracle', 'tools/test-render-lock-blits.py')
    parent = REPO / 'working/tests/render-indexed-copies'
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
        counts, commands = expected[mode]
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
        replay_reports = []
        expected_files = {f'{kind}-{serial:08x}.bin': draw
                          for kind, mapping in commands.items() for serial, draw in mapping.items()}
        actual_files = {path.name for path in capture.glob('blit-*.bin')} | {path.name for path in capture.glob('flip-*.bin')}
        assert actual_files == set(expected_files), (mode, actual_files, expected_files)
        for name, draw in expected_files.items():
            command = capture / name
            native = (case / f'original-{draw:08x}.bin').read_bytes()
            assert oracle.cpu_replay(command.read_bytes()) == native, (mode, name, 'CPU indices')
            colors = (case / f'colors-{draw:08x}.bin').read_bytes()
            lookup = tuple(colors[i * 4:i * 4 + 3] + b'\xff' for i in range(256))
            resolved = b''.join(lookup[index] for index in native)
            output = case / f'{name}.native'
            result = subprocess.run(['xvfb-run', '-a', str(build / 'renderer/mnm-render-commands'), str(command),
                                     '--output', str(output)], env=env, capture_output=True, text=True, timeout=30)
            (case / f'{name}.gpu.log').write_text(result.stdout + result.stderr)
            assert result.returncode == 0, (mode, name, result.stdout, result.stderr)
            report = json.loads(result.stdout)
            assert output.read_bytes() == native, (mode, name, 'GPU indices')
            assert report['presentation_rgba_sha256'] == hashlib.sha256(resolved).hexdigest(), (mode, name, 'GPU palette colors')
            assert report['surface_stats']['palette_updates'] == 1
            replay_reports.append(report)
        if previous:
            result = subprocess.run(['xvfb-run', '-a', str(build / 'mnm-qt-shell'), '--stream-test', str(stream)],
                                    env=env, capture_output=True, text=True, timeout=20)
            (case / 'qt.log').write_text(result.stdout + result.stderr)
            assert result.returncode == 0, (mode, result.stderr)
        reasons = [line.split()[0] for line in (capture / 'lifecycle.log').read_text().splitlines()]
        reports.append({'mode': mode, 'frames': previous, 'counts': counts, 'qt_readback': bool(previous),
                        'reasons': sorted(set(reasons)), 'replays': replay_reports})
        print(f'Indexed copy fixture {mode}: passed', flush=True)
    (root / 'report.json').write_text(json.dumps({'origin': 'synthetic_x86_owned_indexed_copies',
        'architecture': 'PE32 i386', 'dll_sha256': hashlib.sha256(dll.read_bytes()).hexdigest(),
        'fixtures': reports}, indent=2) + '\n')
    print(f'Owned indexed copies passed: {root}')


if __name__ == '__main__':
    main()
