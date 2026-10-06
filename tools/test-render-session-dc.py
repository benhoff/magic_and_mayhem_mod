#!/usr/bin/env python3
"""Validate bounded ordered application-DC handoff with independent Wine DIB pixels."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import struct
import subprocess
import tempfile
import importlib.util
ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('live_fixture', ROOT / 'tools/test-live-render-channel.py')
live = importlib.util.module_from_spec(spec)
spec.loader.exec_module(live)
CASES = ['dcs', 'dcs24', 'dcs32', 'dcs-bottom-up', 'dcs-retry', 'dcs-get-failed',
         'dcs-repeat', 'dcs-format', 'dcs-swapped', 'dcs-mutation', 'dcs-thread',
         'dcs-contended', 'dcs-pending', 'dc', 'dc-rgb24', 'dc-rgb32', 'dc-bottom-up', 'dc-retry']
REFUSED = ['dcs-format', 'dcs-swapped', 'dcs-mutation', 'dcs-thread', 'dcs-contended', 'dcs-pending']


def records(data):
    assert data[:16] == b'MNMCMD01' + struct.pack('<II', 1, 16)
    result, at = [], 16
    while at < len(data):
        op, seq, length = struct.unpack_from('<III', data, at)
        assert seq == len(result) + 1 and at + 12 + length <= len(data)
        result.append((op, data[at + 12:at + 12 + length]))
        at += 12 + length
    return result


def rgba_hash(raw, bits):
    masks = (0xf800, 0x7e0, 0x1f) if bits == 16 else (0xff0000, 0xff00, 0xff)
    output = bytearray()
    for at in range(0, len(raw), bits // 8):
        pixel = int.from_bytes(raw[at:at + bits // 8], 'little')
        rgb = [((pixel & m) // (m & -m)) * 255 // (m // (m & -m)) for m in masks]
        output += struct.pack('<I', 0xff000000 | rgb[0] << 16 | rgb[1] << 8 | rgb[2])
    return hashlib.sha256(output).hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('build', type=Path)
    parser.add_argument('--case', action='append', choices=CASES)
    args = parser.parse_args()
    if not os.environ.get('DISPLAY'):
        parser.error('Run under xvfb-run -a')
    sources = sorted((ROOT / 'runtime/render').glob('*.[ch]')) + [ROOT / p for p in [
        'tools/test-render-session-dc.py', 'tools/test-live-render-channel.py',
        'tools/build-render-bridge.py', 'tools/prepare-shadow-experiment.py',
        'runtime/shadow/win32_min.h', 'tests/live-render-channel-test.cpp',
        'renderer/blit.cpp', 'renderer/blit.hpp', 'renderer/commands.cpp', 'renderer/commands.hpp',
        'renderer/command_state.hpp', 'renderer/command_consumer.cpp',
        'apps/qt-shell/command_channel.cpp', 'apps/qt-shell/command_channel.hpp',
        'apps/qt-shell/live_command_renderer.cpp', 'apps/qt-shell/live_command_renderer.hpp',
        'apps/qt-shell/gl_viewport.cpp', 'apps/qt-shell/gl_viewport.hpp',
        'protocols/include/mnm/render_commands_v1.h', 'protocols/python/mnm_protocols/render_commands_v1.py']]
    hashes = {str(p.relative_to(ROOT)): live.sha(p) for p in sources}
    parent = ROOT / 'working/tests/render-session-dc'
    parent.mkdir(parents=True, exist_ok=True)
    run = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    print(run, flush=True)
    dll = live.load('dc_build', 'tools/build-render-bridge.py').build(True)
    stage = live.load('dc_stage', 'tools/prepare-shadow-experiment.py')
    oracle = live.load('dc_oracle', 'tools/test-render-lock-blits.py')
    env = {k: v for k, v in os.environ.items() if not k.startswith('MNM_')}
    env.update(QT_QPA_PLATFORM='xcb', LIBGL_ALWAYS_SOFTWARE='1', WINEDEBUG='-all',
               WINEPREFIX=str(ROOT / 'working/tests/render-wine'))
    report = dict(schema=1, sources=hashes, scope='Bounded application-owned DC bitmap checkpoint '
                  'input into unchanged v1 native GPU commands. Independent synthetic engine pixels; '
                  'no GDI operation replay, original-game pixel equivalence or replacement.', cases=[])
    for mode in args.case or CASES:
        valid = mode not in REFUSED
        bits = 24 if mode in ['dcs24', 'dc-rgb24'] else 32 if mode in ['dcs32', 'dc-rgb32'] else 16
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
        child = dict(env, MNM_BOOTSTRAP_SELFTEST=mode, MNM_RENDER_OWNED_SESSION='1', MNM_COMMAND_TEST_DELAY='1',
                     MNM_RENDER_COMMAND_CHANNEL='Z:' + str(channel).replace('/', '\\'),
                     MNM_RENDER_STREAM='Z:' + str(frame).replace('/', '\\'),
                     MNM_RENDER_LOCK_CAPTURE_DIR='Z:' + str(capture).replace('/', '\\'))
        with (case / 'qt.log').open('w') as qlog, (case / 'wine.log').open('w') as wlog:
            qt = subprocess.Popen([str(args.build.resolve() / 'live-render-channel-test'),
                                   str(channel), str(active), str(output)], env=env, stdout=qlog, stderr=qlog)
            try:
                live.wait_ready(Path(str(output) + '.ready'), qt)
                wine = subprocess.Popen(['wine', str(case / 'selftest.exe')], cwd=case, env=child, stdout=wlog, stderr=wlog)
                active.write_text(str(wine.pid))
                try:
                    code = wine.wait(timeout=45)
                    assert code == 0, (mode, code)
                finally:
                    if wine.poll() is None:
                        wine.terminate()
                        wine.wait(timeout=5)
                active.unlink()
                assert qt.wait(timeout=35) == (0 if valid else 8), mode
            finally:
                if qt.poll() is None:
                    qt.terminate()
                    qt.wait(timeout=5)
        observed = json.loads(output.read_text())
        expected, checkpoints, count = [], {}, 0
        for step, frames in struct.iter_unpack('<II', (case / 'events.bin').read_bytes()):
            raw = (case / f'original-{step:08x}.bin').read_bytes()
            if frames > count:
                expected.append(rgba_hash(raw, bits))
                checkpoints[step] = raw
            count = frames
        assert observed['success'] == valid, (mode, observed)
        assert observed.get('native_readbacks', 0) == observed.get('rgba_readbacks', 0) == observed['viewport_uploads'] == observed.get('live_surfaces', 0) == 0
        session = (capture / 'session-00000001.bin').read_bytes()
        decoded = records(session)
        with channel.open('rb') as file:
            control = file.read(64)
            published = file.read(struct.unpack_from('<I', control, 20)[0])
        assert session[:len(published)] == published
        target_updates = [p for op, p in decoded if op == 2 and struct.unpack_from('<I', p)[0] == 1]
        verified_report = None
        if valid:
            assert observed['frames'] == expected and observed['before_producer_exit'], (mode, observed)
            assert session == published and struct.unpack_from('<I', control, 24)[0] == 2 and decoded[-1][0] == 8
            assert oracle.cpu_replay(session) == raw
            native = case / 'native-verified.bin'
            verified = subprocess.run([str(args.build.resolve() / 'mnm-render-commands'),
                                       str(capture / 'session-00000001.bin'), '--output', str(native)],
                                      env=env, capture_output=True, text=True, timeout=30)
            (case / 'native-verify.log').write_text(verified.stdout + verified.stderr)
            assert verified.returncode == 0 and native.read_bytes() == raw, (mode, verified.stderr)
            verified_report = json.loads(verified.stdout)
            if mode.startswith('dcs'):
                dc_steps = [3] if mode == 'dcs-retry' else [2, 3] if mode == 'dcs-repeat' else [2]
                assert len(target_updates) == 1 + len(dc_steps), mode
                for data, step in zip(target_updates[1:], dc_steps):
                    assert struct.unpack_from('<5I', data) == (1, 0, 0, 800, 600)
                    assert data[20:] == checkpoints[step], mode
                if mode == 'dcs-retry':
                    events = list(struct.iter_unpack('<II', (case / 'events.bin').read_bytes()))
                    assert events[:3] == [(1, 1), (2, 1), (3, 2)], events
        else:
            assert observed['frames'] == expected[:len(observed['frames'])] and len(target_updates) == 1
            assert struct.unpack_from('<II', control, 24) == (3, 2)
            assert decoded[-1] == (9, struct.pack('<I', 3))
        report['cases'].append(dict(mode=mode, valid_session=valid, bits=bits,
                                    target_updates=len(target_updates), native_check_verify=verified_report, **observed))
        print(mode + ': passed', flush=True)
    assert all(live.sha(ROOT / p) == h for p, h in hashes.items()), 'Source changed during execution'
    report.update(success=True, full_frame_comparisons=sum(len(c['frames']) for c in report['cases']),
                  valid_sessions=sum(c['valid_session'] for c in report['cases']),
                  refused_sessions=sum(not c['valid_session'] for c in report['cases']),
                  ordinary_readbacks=0, viewport_uploads=0)
    (run / 'report.json').write_text(json.dumps(report, indent=2) + '\n')
    print('DC sessions passed: ' + str(run / 'report.json'), flush=True)


if __name__ == '__main__':
    main()
