#!/usr/bin/env python3
"""Compare owned fill sessions with independent PE32 fixture pixels at every PRESENT."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import struct
import subprocess
import tempfile
import sys
ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
import importlib.util
spec = importlib.util.spec_from_file_location('live_fixture', ROOT / 'tools/test-live-render-channel.py')
live = importlib.util.module_from_spec(spec)
spec.loader.exec_module(live)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('build', type=Path)
    args = parser.parse_args()
    if not os.environ.get('DISPLAY'):
        parser.error('Run under xvfb-run -a')
    sources = sorted((ROOT / 'runtime/render').glob('*.[ch]')) + [ROOT / p for p in [
        'tools/test-render-session-fills.py', 'tools/test-live-render-channel.py', 'tools/test-render-bootstrap.py',
        'tools/build-render-bridge.py', 'tools/prepare-shadow-experiment.py',
        'runtime/shadow/win32_min.h', 'tests/live-render-channel-test.cpp',
        'renderer/blit.cpp', 'renderer/blit.hpp', 'renderer/commands.cpp',
        'renderer/commands.hpp', 'renderer/command_state.hpp', 'renderer/command_consumer.cpp',
        'apps/qt-shell/command_channel.cpp', 'apps/qt-shell/command_channel.hpp',
        'apps/qt-shell/live_command_renderer.cpp', 'apps/qt-shell/live_command_renderer.hpp',
        'apps/qt-shell/gl_viewport.cpp', 'apps/qt-shell/gl_viewport.hpp',
        'protocols/include/mnm/render_commands_v1.h',
        'protocols/python/mnm_protocols/render_commands_v1.py']]
    hashes = {str(p.relative_to(ROOT)): live.sha(p) for p in sources}
    parent = ROOT / 'working/tests/render-session-fills'
    parent.mkdir(parents=True, exist_ok=True)
    run = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    print(run, flush=True)
    dll = live.load('fill_build', 'tools/build-render-bridge.py').build(True)
    stage = live.load('fill_stage', 'tools/prepare-shadow-experiment.py')
    oracle = live.load('fill_oracle', 'tools/test-render-lock-blits.py')
    env = {k: v for k, v in os.environ.items() if not k.startswith('MNM_')}
    env.update(QT_QPA_PLATFORM='xcb', LIBGL_ALWAYS_SOFTWARE='1', WINEDEBUG='-all',
               WINEPREFIX=str(ROOT / 'working/tests/render-wine'))
    report = dict(schema=1, sources=hashes, scope='Synthetic owned constant fills through unchanged '
                  'v1 UPDATE admission into live native GPU presentation; independent complete original '
                  'fixture pixels. No original-game pixel equivalence or replacement.', cases=[])
    for mode in ['fill8', 'fill', 'fill24', 'fill32', 'fill-failed', 'fill-partial', 'fill-update', 'fill-session']:
        case = run / mode
        capture = case / 'capture'
        capture.mkdir(parents=True)
        channel, frame = case / 'commands.bin', case / 'frame.bin'
        header = bytearray(live.protocol.initial_header())
        struct.pack_into('<I', header, 16, 123)
        live.create(channel, header, live.protocol.SIZE)
        live.create(frame, live.frame_v1.initial_header(), live.frame_v1.SIZE)
        shutil.copyfile(dll, case / dll.name)
        (case / 'selftest.exe').write_bytes(stage.add_import((dll.parent / 'selftest.exe').read_bytes(),
            dll='MnmRender.dll', symbol_name='RenderAnchor', section_name=b'.mnmgl'))
        output, active = case / 'qt.json', case / 'producer.active'
        child = dict(env, MNM_BOOTSTRAP_SELFTEST=mode, MNM_RENDER_OWNED_SESSION='1',
                     MNM_COMMAND_TEST_DELAY='1',
                     MNM_RENDER_COMMAND_CHANNEL='Z:' + str(channel).replace('/', '\\'),
                     MNM_RENDER_STREAM='Z:' + str(frame).replace('/', '\\'),
                     MNM_RENDER_LOCK_CAPTURE_DIR='Z:' + str(capture).replace('/', '\\'))
        with (case / 'qt.log').open('w') as qlog, (case / 'wine.log').open('w') as wlog:
            qt = subprocess.Popen([str(args.build.resolve() / 'live-render-channel-test'),
                                   str(channel), str(active), str(output)], env=env, stdout=qlog, stderr=qlog)
            try:
                live.wait_ready(Path(str(output) + '.ready'), qt)
                wine = subprocess.Popen(['wine', str(case / 'selftest.exe')], cwd=case, env=child,
                                        stdout=wlog, stderr=wlog)
                active.write_text(str(wine.pid))
                try:
                    code = wine.wait(timeout=45)
                    assert code == 0, (mode, code)
                finally:
                    if wine.poll() is None:
                        wine.terminate()
                        wine.wait(timeout=5)
                active.unlink()
                assert qt.wait(timeout=35) == 0, mode
            finally:
                if qt.poll() is None:
                    qt.terminate()
                    qt.wait(timeout=5)
        observed = json.loads(output.read_text())
        bits = 8 if mode == 'fill8' else 24 if mode == 'fill24' else 32 if mode == 'fill32' else 16
        expected, count = [], 0
        for step, frames in struct.iter_unpack('<II', (case / 'events.bin').read_bytes()):
            if frames > count:
                raw = (case / f'original-{step:08x}.bin').read_bytes()
                packed = bytearray()
                masks = (0xf800, 0x7e0, 0x1f) if bits == 16 else (0xff0000, 0xff00, 0xff)
                for at in range(0, len(raw), bits // 8):
                    value = int.from_bytes(raw[at:at + bits // 8], 'little')
                    colors = ([(value * (j * 6 + 1) + j * 13) & 255 for j in range(3)] if bits == 8 else
                              [((value & m) // (m & -m)) * 255 // (m // (m & -m)) for m in masks])
                    packed += struct.pack('<I', 0xff000000 | colors[0] << 16 | colors[1] << 8 | colors[2])
                expected.append(hashlib.sha256(packed).hexdigest())
            count = frames
        assert observed['success'] and observed['frames'] == expected, (mode, observed)
        assert observed['native_readbacks'] == observed['rgba_readbacks'] == observed['viewport_uploads'] == observed['live_surfaces'] == 0
        session = (capture / 'session-00000001.bin').read_bytes()
        assert oracle.cpu_replay(session) == raw, mode
        native = case / 'native-verified.bin'
        verified = subprocess.run([str(args.build.resolve() / 'mnm-render-commands'),
                                   str(capture / 'session-00000001.bin'), '--output', str(native)],
                                  env=env, capture_output=True, text=True, timeout=30)
        (case / 'native-verify.log').write_text(verified.stdout + verified.stderr)
        assert verified.returncode == 0 and native.read_bytes() == raw, (mode, verified.stderr)
        with channel.open('rb') as file:
            control = file.read(64)
            assert file.read(struct.unpack_from('<I', control, 20)[0]) == session
        at, updates = 16, []
        while at < len(session):
            op, seq, length = struct.unpack_from('<III', session, at)
            p = session[at + 12:at + 12 + length]
            if op == 2 and struct.unpack_from('<I', p)[0] == 2:
                fields = struct.unpack_from('<5I', p)
                value = 0x34 if bits == 8 else (0x4567 if mode == 'fill-update' and updates else 0x1234) if bits == 16 else 0x123456 | (0x80000000 if bits == 32 else 0)
                assert p[20:] == value.to_bytes(bits // 8, 'little') * (fields[3] * fields[4])
                updates.append(list(fields))
            at += 12 + length
        assert updates == ([[2, 0, 0, 800, 600], [2, 1, 2, 19, 28]] if mode == 'fill-update' else [[2, 0, 0, 800, 600]]), (mode, updates)
        report['cases'].append(dict(mode=mode, bits=bits, fill_updates=updates,
                                    native_check_verify=json.loads(verified.stdout), **observed))
        print(mode + ': passed', flush=True)
    assert all(live.sha(ROOT / p) == h for p, h in hashes.items())
    report.update(success=True, full_frame_comparisons=sum(len(c['frames']) for c in report['cases']),
                  ordinary_readbacks=0, viewport_uploads=0)
    (run / 'report.json').write_text(json.dumps(report, indent=2) + '\n')
    print('Fill sessions passed: ' + str(run / 'report.json'), flush=True)


if __name__ == '__main__':
    main()
