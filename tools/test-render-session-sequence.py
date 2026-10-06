#!/usr/bin/env python3
"""Check bounded successive primary PRESENTs with independent fixture pixels."""
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
ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('live_fixture', ROOT / 'tools/test-live-render-channel.py')
live = importlib.util.module_from_spec(spec)
spec.loader.exec_module(live)
CASES = ['sequence-three', 'sequence-max', 'sequence-failed', 'sequence-budget',
         'sequence-exit', 'sequence-invalidate', 'sequence-default', 'sequence-malformed']
VALID = ['sequence-three', 'sequence-max', 'sequence-failed', 'sequence-default', 'sequence-malformed']


def records(data):
    assert data[:16] == b'MNMCMD01' + struct.pack('<II', 1, 16)
    result, at = [], 16
    while at < len(data):
        op, sequence, length = struct.unpack_from('<III', data, at)
        assert sequence == len(result)+1 and at+12+length <= len(data)
        result.append((op, data[at+12:at+12+length]))
        at += 12+length
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('build', type=Path)
    args = parser.parse_args()
    if not os.environ.get('DISPLAY'):
        parser.error('Run under xvfb-run -a')
    sources = sorted((ROOT/'runtime/render').glob('*.[ch]')) + [ROOT/p for p in [
        'tools/test-render-session-sequence.py', 'tools/test-live-render-channel.py',
        'tools/build-render-bridge.py', 'tools/prepare-shadow-experiment.py',
        'runtime/shadow/win32_min.h', 'tests/live-render-channel-test.cpp',
        'renderer/blit.cpp', 'renderer/blit.hpp', 'renderer/commands.cpp', 'renderer/commands.hpp',
        'renderer/command_state.hpp', 'renderer/command_consumer.cpp',
        'apps/qt-shell/command_channel.cpp', 'apps/qt-shell/command_channel.hpp',
        'apps/qt-shell/live_command_renderer.cpp', 'apps/qt-shell/live_command_renderer.hpp',
        'apps/qt-shell/gl_viewport.cpp', 'apps/qt-shell/gl_viewport.hpp',
        'protocols/include/mnm/render_commands_v1.h', 'protocols/python/mnm_protocols/render_commands_v1.py']]
    fingerprints = {str(p.relative_to(ROOT)): live.sha(p) for p in sources}
    parent = ROOT/'working/tests/render-session-sequence'
    parent.mkdir(parents=True, exist_ok=True)
    run = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    print(run, flush=True)
    dll = live.load('startup_build', 'tools/build-render-bridge.py').build(True)
    stage = live.load('startup_stage', 'tools/prepare-shadow-experiment.py')
    oracle = live.load('startup_oracle', 'tools/test-render-lock-blits.py')
    env = {k:v for k,v in os.environ.items() if not k.startswith('MNM_')}
    env.update(QT_QPA_PLATFORM='xcb', LIBGL_ALWAYS_SOFTWARE='1', WINEDEBUG='-all',
               WINEPREFIX=str(ROOT/'working/tests/render-wine'))
    report = dict(schema=1, sources=fingerprints, scope='Bounded successive PRESENTs with64-operation startup and256-operation total ceiling; '
                  'ordinary16-operation sample retained. Independent synthetic GPU '
                  'fixture pixels only, no original-driver equivalence or replacement.', cases=[])
    for mode in CASES:
        valid = mode in VALID
        target = 32 if mode == 'sequence-max' else 3
        expected_count = 32 if mode == 'sequence-max' else 1 if mode in ['sequence-budget', 'sequence-invalidate', 'sequence-default', 'sequence-malformed'] else 2 if mode == 'sequence-exit' else 3
        case = run/mode
        capture = case/'capture'
        capture.mkdir(parents=True)
        channel, frame = case/'commands.bin', case/'frame.bin'
        header = bytearray(live.protocol.initial_header())
        struct.pack_into('<I', header, 16, 123)
        live.create(channel, header, live.protocol.SIZE)
        live.create(frame, live.frame_v1.initial_header(), live.frame_v1.SIZE)
        shutil.copyfile(dll, case/dll.name)
        (case/'selftest.exe').write_bytes(stage.add_import((dll.parent/'selftest.exe').read_bytes(),
            dll='MnmRender.dll', symbol_name='RenderAnchor', section_name=b'.mnmgl'))
        active, output = case/'producer.active', case/'qt.json'
        child = dict(env, MNM_OWNED_SESSION_SELFTEST=mode, MNM_RENDER_OWNED_SESSION='1', MNM_COMMAND_TEST_DELAY='1',
                     MNM_RENDER_COMMAND_CHANNEL='Z:'+str(channel).replace('/', '\\'),
                     MNM_RENDER_STREAM='Z:'+str(frame).replace('/', '\\'),
                     MNM_RENDER_LOCK_CAPTURE_DIR='Z:'+str(capture).replace('/', '\\'))
        if mode != 'sequence-default':
            child['MNM_RENDER_SESSION_PRESENTATIONS'] = '33' if mode == 'sequence-malformed' else str(target)
        if mode == 'sequence-budget':
            child['MNM_COMMAND_TEST_DELAY'] = '2'
        with (case/'qt.log').open('w') as qlog, (case/'wine.log').open('w') as wlog:
            qt = subprocess.Popen([str(args.build.resolve()/'live-render-channel-test'),
                                   str(channel), str(active), str(output)], env=env, stdout=qlog, stderr=qlog)
            try:
                live.wait_ready(Path(str(output)+'.ready'), qt)
                wine = subprocess.Popen(['wine', str(case/'selftest.exe')], cwd=case, env=child, stdout=wlog, stderr=wlog)
                active.write_text(str(wine.pid))
                try:
                    assert wine.wait(timeout=45) == 0, mode
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
        session = (capture/'session-00000001.bin').read_bytes()
        decoded = records(session)
        with channel.open('rb') as file:
            control = file.read(64)
            published = file.read(struct.unpack_from('<I', control, 20)[0])
        assert session[:len(published)] == published and len(decoded) <= 4096 and len(session) <= 64*1024*1024
        assert observed['success'] == valid and observed.get('native_readbacks', 0) == observed.get('rgba_readbacks', 0) == observed['viewport_uploads'] == observed.get('live_surfaces', 0) == 0
        updates = sum(op == 2 for op, _ in decoded)
        assert updates >= 16 and len(decoded) <= 4096
        assert any(op == 5 for op, _ in decoded) == (mode in ['sequence-default', 'sequence-malformed'])
        verify_report = None
        if valid:
            expected = live.original_frames(case, False)
            # Original calls continue after completion; the ordered stream stays ended.
            expected = expected[:expected_count]
            assert observed['frames'] == expected and observed['presentations'] == expected_count and observed['before_producer_exit']
            assert session == published and struct.unpack_from('<I', control, 24)[0] == 2
            assert [op for op,_ in decoded[-3:]] == [7,7,8]
            assert sum(op == 3 for op,_ in decoded) == sum(op == 6 for op,_ in decoded) == expected_count
            events = list(struct.iter_unpack('<6I', (case/'session-events.bin').read_bytes()))
            first = next(step for step,op,subject,status,count,partial in events if count == expected_count)
            native = (case/f'front-{first:08x}.bin').read_bytes()
            assert oracle.cpu_replay(session) == native
            target = case/'native-verified.bin'
            verify = subprocess.run([str(args.build.resolve()/'mnm-render-commands'),
                                     str(capture/'session-00000001.bin'), '--output', str(target)],
                                    env=env, capture_output=True, text=True, timeout=30)
            (case/'native-verify.log').write_text(verify.stdout+verify.stderr)
            assert verify.returncode == 0 and target.read_bytes() == native, (mode, verify.stderr)
            verify_report = json.loads(verify.stdout)
        else:
            assert struct.unpack_from('<II', control, 24) == (3,2)
            assert decoded[-1] == (9,struct.pack('<I',3 if mode == 'sequence-invalidate' else 6))
            assert observed['frames'] == live.original_frames(case, False)[:expected_count]
            assert observed['presentations'] == expected_count and not any(op == 8 for op,_ in decoded)
        report['cases'].append(dict(mode=mode, valid_session=valid,
                                    requested_presentations=child.get('MNM_RENDER_SESSION_PRESENTATIONS'), updates=updates,
                                    commands=len(decoded), bytes=len(session), native_check_verify=verify_report, **observed))
        print(mode+': passed', flush=True)
    assert all(live.sha(ROOT/p) == h for p,h in fingerprints.items()), 'Source changed during execution'
    report.update(success=True, valid_sessions=5, refused_sessions=3, full_frame_comparisons=sum(len(c['frames']) for c in report['cases']),
                  ordinary_readbacks=0, viewport_uploads=0)
    (run/'report.json').write_text(json.dumps(report, indent=2)+'\n')
    print('Sequence sessions passed: '+str(run/'report.json'), flush=True)


if __name__ == '__main__':
    main()
