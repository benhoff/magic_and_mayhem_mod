#!/usr/bin/env python3
"""Verify Qt-written input polling state through synthetic PE32 Wine hooks."""
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


def main():
    parent = REPO / 'working/tests/render-input'
    parent.mkdir(parents=True, exist_ok=True)
    root = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    build = REPO / 'working/build/qt-shell'
    subprocess.run(['cmake', '-S', str(REPO / 'apps/qt-shell'), '-B', str(build)], check=True)
    subprocess.run(['cmake', '--build', str(build), '--target', 'qt-input-test', '--parallel', '4'], check=True)
    env = {key: value for key, value in os.environ.items() if not key.startswith('MNM_')}
    env.update(QT_QPA_PLATFORM='xcb', LIBGL_ALWAYS_SOFTWARE='1')
    state = root / 'input.bin'
    with (root / 'qt.log').open('w') as log:
        subprocess.run(['xvfb-run', '-a', str(build / 'qt-input-test'), str(state)], env=env, stdout=log, stderr=log, check=True, timeout=20)
    initial = state.read_bytes()
    assert initial[:16] == b'MNMINK01' + struct.pack('<II', 1, 1088)
    assert struct.unpack_from('<6I', initial, 16) == (2, 1, 400, 300, 800, 600)
    assert struct.unpack_from('<I', initial, 64+65*4)[0] == 0x80000001
    dll = load('input_build', 'tools/build-render-bridge.py').build(True)
    stage = load('input_stage', 'tools/prepare-shadow-experiment.py')
    shutil.copy2(dll, root / dll.name)
    (root / 'selftest.exe').write_bytes(stage.add_import((dll.parent / 'selftest.exe').read_bytes(), dll='MnmRender.dll', symbol_name='RenderAnchor', section_name=b'.mnmgl'))
    stream = root / 'frame.bin'
    with stream.open('wb') as file:
        file.write(b'MNMGL001' + struct.pack('<II', 1, 64) + bytes(48))
        file.truncate(64+2048*2048*4)
    env.update(WINEPREFIX=str(REPO / 'working/tests/render-wine'), WINEDEBUG='-all', MNM_INPUT_SELFTEST='1',
               MNM_RENDER_INPUT='Z:'+str(state).replace('/', '\\'), MNM_RENDER_STREAM='Z:'+str(stream).replace('/', '\\'))
    with (root / 'wine.log').open('w') as log:
        subprocess.run(['wine', str(root / 'selftest.exe')], cwd=root, env=env, stdout=log, stderr=log, check=True, timeout=30)
    report = {'origin': 'synthetic_qt_to_pe32_input_polling', 'architecture': 'PE32 i386',
              'dll_sha256': hashlib.sha256(dll.read_bytes()).hexdigest(), 'initial_state_sha256': hashlib.sha256(initial).hexdigest(),
              'checks': ['held_key', 'press_bit_consumption', 'key_release', 'press_between_polls', 'client_to_screen',
                         'last_error', 'lock_key_fallback', 'odd_sequence', 'inactive_fallback', 'invalid_coordinates', 'stale_heartbeat',
                         'guarded_iat_install', 'cooperative_window_forwarding'],
              'live_game_validated': False}
    (root / 'report.json').write_text(json.dumps(report, indent=2)+'\n')
    print(f'Qt -> x86 input polling passed: {root}')


if __name__ == '__main__':
    main()
