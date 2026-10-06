#!/usr/bin/env python3
"""Check bounded startup through first primary PRESENT with independent fixture pixels."""
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
CASES = ['startup-boundary', 'startup-delayed', 'startup-last', 'startup-failed',
         'startup-missing', 'startup-exit', 'startup-too-late', 'startup-invalidate']
VALID = CASES[:4]


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
        'tools/test-render-session-startup.py', 'tools/test-live-render-channel.py',
        'tools/build-render-bridge.py', 'tools/prepare-shadow-experiment.py',
        'runtime/shadow/win32_min.h', 'tests/live-render-channel-test.cpp',
        'renderer/blit.cpp', 'renderer/blit.hpp', 'renderer/commands.cpp', 'renderer/commands.hpp',
        'renderer/command_state.hpp', 'renderer/command_consumer.cpp',
        'apps/qt-shell/command_channel.cpp', 'apps/qt-shell/command_channel.hpp',
        'apps/qt-shell/live_command_renderer.cpp', 'apps/qt-shell/live_command_renderer.hpp',
        'apps/qt-shell/gl_viewport.cpp', 'apps/qt-shell/gl_viewport.hpp',
        'protocols/include/mnm/render_commands_v1.h', 'protocols/python/mnm_protocols/render_commands_v1.py']]
    fingerprints = {str(p.relative_to(ROOT)): live.sha(p) for p in sources}
    parent = ROOT/'working/tests/render-session-startup'
    parent.mkdir(parents=True, exist_ok=True)
    run = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    print(run, flush=True)
    dll = live.load('startup_build', 'tools/build-render-bridge.py').build(True)
    stage = live.load('startup_stage', 'tools/prepare-shadow-experiment.py')
    oracle = live.load('startup_oracle', 'tools/test-render-lock-blits.py')
    env = {k:v for k,v in os.environ.items() if not k.startswith('MNM_')}
    env.update(QT_QPA_PLATFORM='xcb', LIBGL_ALWAYS_SOFTWARE='1', WINEDEBUG='-all',
               WINEPREFIX=str(ROOT/'working/tests/render-wine'))
    report = dict(schema=1, sources=fingerprints, scope='Finite64-operation startup window through first '
                  'primary PRESENT; ordinary16-operation sample retained. Independent synthetic GPU '
                  'fixture pixels only, no original-driver equivalence or replacement.', cases=[])
    for mode in CASES:
        valid = mode in VALID
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
        setup = 15 if mode == 'startup-boundary' else 63 if mode in ['startup-last', 'startup-failed'] else 64 if mode in ['startup-missing', 'startup-too-late'] else 17
        assert updates == setup-1, (mode, updates)  # CREATE then one UPDATE per admitted setup unlock.
        verify_report = None
        if valid:
            expected = live.original_frames(case, False)
            # After delayed completion, original calls continue but the ordered sample stays ended.
            expected = expected[:1]
            assert observed['frames'] == expected and observed['presentations'] == 1 and observed['before_producer_exit']
            assert session == published and struct.unpack_from('<I', control, 24)[0] == 2
            assert [op for op,_ in decoded[-3:]] == [7,7,8]
            assert sum(op == 3 for op,_ in decoded) == sum(op == 6 for op,_ in decoded) == 1
            events = list(struct.iter_unpack('<6I', (case/'session-events.bin').read_bytes()))
            first = next(step for step,op,subject,status,count,partial in events if count)
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
            assert decoded[-1] == (9,struct.pack('<I',3 if mode == 'startup-invalidate' else 6))
            assert not observed['frames'] and not any(op in [6,8] for op,_ in decoded)
        report['cases'].append(dict(mode=mode, valid_session=valid, setup_operations=setup, updates=updates,
                                    commands=len(decoded), bytes=len(session), native_check_verify=verify_report, **observed))
        print(mode+': passed', flush=True)
    assert all(live.sha(ROOT/p) == h for p,h in fingerprints.items()), 'Source changed during execution'
    report.update(success=True, valid_sessions=4, refused_sessions=4, full_frame_comparisons=4,
                  ordinary_readbacks=0, viewport_uploads=0)
    (run/'report.json').write_text(json.dumps(report, indent=2)+'\n')
    print('Startup sessions passed: '+str(run/'report.json'), flush=True)


if __name__ == '__main__':
    main()
