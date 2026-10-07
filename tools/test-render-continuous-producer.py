#!/usr/bin/env python3
"""Actual PE32 continuous ownership/publication with independent GPU frames."""
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
spec = importlib.util.spec_from_file_location('continuous_live', ROOT/'tools/test-live-render-channel.py')
live = importlib.util.module_from_spec(spec)
spec.loader.exec_module(live)
from mnm_protocols import render_commands_v2 as ring


def archive_records(data):
    assert data[:16] == b'MNMCMD01'+struct.pack('<II', 1, 16)
    at, result = 16, []
    while at < len(data):
        op, seq, size = struct.unpack_from('<III', data, at)
        assert seq == len(result)+1 and at+12+size <= len(data)
        result.append((op, size, data[at+12:at+12+size] if size else b''))
        at += 12+size
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('build', type=Path)
    parser.add_argument('--case', action='append', choices=['no-archive', 'record-archive', 'byte-archive',
                        'failed-archive', 'short-archive', 'delta', 'tiled', 'v1', 'missing-present', 'limit'])
    args = parser.parse_args()
    if not os.environ.get('DISPLAY'):
        parser.error('Run under xvfb-run -a')
    sources = sorted((ROOT/'runtime/render').glob('*.[ch]'))+[ROOT/p for p in [
        'tools/test-render-continuous-producer.py', 'tools/test-live-render-channel.py',
        'tools/build-render-bridge.py', 'tools/prepare-shadow-experiment.py',
        'tools/run-opengl-game.py', 'apps/qt-shell/main.cpp',
        'runtime/shadow/win32_min.h', 'tests/live-render-channel-test.cpp',
        'renderer/blit.cpp', 'renderer/blit.hpp', 'renderer/commands.cpp', 'renderer/commands.hpp',
        'renderer/command_state.hpp', 'renderer/command_consumer.cpp',
        'apps/qt-shell/command_channel.cpp', 'apps/qt-shell/command_channel.hpp',
        'apps/qt-shell/live_command_renderer.cpp', 'apps/qt-shell/live_command_renderer.hpp',
        'apps/qt-shell/gl_viewport.cpp', 'apps/qt-shell/gl_viewport.hpp',
        'protocols/include/mnm/render_commands_v1.h', 'protocols/include/mnm/render_commands_v2.h',
        'protocols/include/mnm/render_command_ring.h',
        'protocols/python/mnm_protocols/render_commands_v1.py', 'protocols/python/mnm_protocols/render_commands_v2.py']]
    fingerprints = {str(p.relative_to(ROOT)):live.sha(p) for p in sources}
    parent = ROOT/'working/tests/render-continuous';parent.mkdir(parents=True, exist_ok=True)
    run = Path(tempfile.mkdtemp(prefix='run-', dir=parent));print(run, flush=True)
    dll = live.load('continuous_build', 'tools/build-render-bridge.py').build(True)
    stage = live.load('continuous_stage', 'tools/prepare-shadow-experiment.py')
    env = {k:v for k,v in os.environ.items() if not k.startswith('MNM_')}
    env.update(QT_QPA_PLATFORM='xcb', LIBGL_ALWAYS_SOFTWARE='1', WINEDEBUG='-all',
               WINEPREFIX=str(ROOT/'working/tests/render-wine'))
    cases = args.case or ['no-archive', 'record-archive', 'byte-archive', 'failed-archive',
                         'short-archive', 'delta', 'tiled', 'v1', 'missing-present', 'limit']
    report = dict(schema=1, sources=fingerprints, cases=[], scope='Synthetic actual PE32 hooked Lock/Unlock '
                  'through owned producer, v2 queue/idle scheduler, native streaming decoder and GPU. '
                  'Independent complete RGB32 display frames and poisoned padded borrowed storage. '
                  'Optional bounded archive refusal does not complete live production. '
                  'No original game, driver equivalence, resource retirement or recovery claim.')
    # Staging must reject missing/v1 channels before touching game media.
    guard = run/'launcher-guards';guard.mkdir()
    guard_frame, guard_channel = guard/'frame.bin', guard/'commands.bin'
    live.create(guard_frame, live.frame_v1.initial_header(), live.frame_v1.SIZE)
    header = bytearray(live.protocol.initial_header());struct.pack_into('<I', header, 16, 123)
    live.create(guard_channel, header, live.protocol.SIZE)
    for name, extra in [('missing', []), ('v1', ['--command-channel', str(guard_channel)])]:
        result = subprocess.run(['python3', str(ROOT/'tools/run-opengl-game.py'), '--stage-only',
                                 '--stream', str(guard_frame), *extra],
                                env=dict(env, MNM_RENDER_CONTINUOUS='1'), capture_output=True, text=True, timeout=15)
        (guard/(name+'.log')).write_text(result.stdout+result.stderr)
        assert result.returncode != 0 and 'requires a' in result.stderr and 'v2 command channel' in result.stderr
        assert 'Render experiment:' not in result.stdout
    report['launcher_refusals_passed'] = True
    for mode in cases:
        case = run/mode;case.mkdir();capture = case/'capture';capture.mkdir()
        archive_path = capture/'session-00000001.bin'
        if mode == 'failed-archive':
            archive_path.mkdir()  # Deterministic CreateFile failure without affecting the channel.
        channel, frame = case/'commands.bin', case/'frame.bin'
        protocol = live.protocol if mode == 'v1' else ring
        header = bytearray(protocol.initial_header());struct.pack_into('<I', header, 16, 123)
        live.create(channel, header, protocol.SIZE)
        live.create(frame, live.frame_v1.initial_header(), live.frame_v1.SIZE)
        shutil.copyfile(dll, case/dll.name)
        (case/'selftest.exe').write_bytes(stage.add_import((dll.parent/'selftest.exe').read_bytes(),
            dll='MnmRender.dll', symbol_name='RenderAnchor', section_name=b'.mnmgl'))
        active, output = case/'producer.active', case/'qt.json'
        child = dict(env, MNM_CONTINUOUS_SELFTEST=mode, MNM_RENDER_CONTINUOUS='1',
                     MNM_RENDER_SESSION_PRESENTATIONS='1',  # Must not terminate continuous mode.
                     MNM_RENDER_COMMAND_CHANNEL='Z:'+str(channel).replace('/', '\\'),
                     MNM_RENDER_STREAM='Z:'+str(frame).replace('/', '\\'),
                     MNM_RENDER_LOCK_CAPTURE_DIR='Z:'+str(capture).replace('/', '\\'))
        archive = mode in ['record-archive', 'byte-archive', 'failed-archive', 'short-archive','delta','tiled']
        if archive:
            child['MNM_RENDER_SESSION_ARCHIVE'] = '1'
        valid = mode not in ['v1', 'missing-present']
        with (case/'qt.log').open('w') as qlog, (case/'wine.log').open('w') as wlog:
            qt = subprocess.Popen([str(args.build.resolve()/'live-render-channel-test'),
                                   str(channel), str(active), str(output)], env=env, stdout=qlog, stderr=qlog)
            wine = None
            try:
                live.wait_ready(Path(str(output)+'.ready'), qt)
                wine = subprocess.Popen(['wine', str(case/'selftest.exe')], cwd=case, env=child, stdout=wlog, stderr=wlog)
                active.write_text(str(wine.pid))
                wine_status = wine.wait(timeout=35)
                assert wine_status == 0, (mode, wine_status, (case/'wine.log').read_text())
                active.unlink()
                assert qt.wait(timeout=35) == (0 if valid else 8), (mode, output.read_text())
            finally:
                for process in [wine, qt]:
                    if process is not None and process.poll() is None:
                        process.terminate();process.wait(timeout=5)
        observed = json.loads(output.read_text())
        locks, unlocks, count = struct.unpack('<III', (case/'engine-counts.bin').read_bytes())
        native_expected=[]
        expected = [];delta_pixels=bytearray(bytes([0,0,0,0xa5])*(512*512))
        for i in range(count):
            rgb = ((i*37)&255, (i*71)&255, (i*19)&255)
            if mode in ['delta','tiled']:
                at=((i*29%512)*512+i*17%512)*4
                if mode=='tiled':
                    if i%5==4:delta_pixels[:]=bytes([rgb[2],rgb[1],rgb[0],0xa5])*(512*512)
                    elif i%5==1:delta_pixels[0]^=0x5a;delta_pixels[-1]^=0x80
                    elif i%5==2:
                        for y in range(8):
                            for x in range(4):delta_pixels[((y*64+11)*512+x*128+17)*4+i%4]^=0x81
                    elif i and i%5==0:delta_pixels[3]^=0x80;delta_pixels[-1]^=0x80
                elif i%3==1:struct.pack_into('<I',delta_pixels,at,0xa5000000|(i*0x254713&0xffffff))
                elif i%3==2:delta_pixels[at+3]^=0x80
                native_expected.append(hashlib.sha256(delta_pixels).hexdigest())
                display=bytearray(delta_pixels);display[3::4]=bytes([255])*(512*512)
                expected.append(hashlib.sha256(display).hexdigest())
            else:expected.append(hashlib.sha256(bytes([rgb[2], rgb[1], rgb[0], 255])*(512*512)).hexdigest())
        assert observed['frames'] == expected and observed['presentations'] == count
        assert locks == unlocks and observed['success'] == valid
        assert observed.get('native_readbacks', 0) == observed.get('rgba_readbacks', 0) == observed['viewport_uploads'] == observed.get('live_surfaces', 0) == 0
        if count:
            i = count-1
            native = bytes([(i*19)&255, (i*71)&255, (i*37)&255, 0xa5])*(512*512)
            if mode in ['delta','tiled']:native=delta_pixels
            assert (case/'engine-final.bin').read_bytes() == native
            assert observed['before_producer_exit']
        with channel.open('rb') as file:
            control = file.read(64)
        published, state, reason = struct.unpack_from('<III', control, 20)
        if valid and mode not in ['short-archive','delta','tiled']:
            assert published > 64*1024*1024 and observed['decoded_commands'] > 4096 and locks > 256 and count > 32
            assert struct.unpack_from('<I', control, 36)[0] == published
        assert state == (2 if valid else 3)
        assert reason == (0 if valid else 5 if mode == 'v1' else 2)
        archived = None
        if mode in ['record-archive', 'byte-archive', 'short-archive','delta','tiled']:
            data = archive_path.read_bytes();records = archive_records(data)
            assert len(data) <= 64*1024*1024 and len(records) <= 4096
            assert not any(op in [5,10] for op,_,_ in records)
            if mode=='tiled':
                states={};presented=[];updates=[]
                for op,_,payload in records:
                    if op==1:
                        sid,w,h,bits,*_=struct.unpack_from('<7I',payload);states[sid]=(w,h,bits//8,bytearray(payload[28:]))
                    elif op==2:
                        sid,x,y,w,h=struct.unpack_from('<5I',payload);sw,sh,b,pixels=states[sid];updates.append((w,h,len(payload)))
                        assert x+w<=sw and y+h<=sh and len(payload)==20+w*h*b
                        for row in range(h):pixels[((y+row)*sw+x)*b:((y+row)*sw+x+w)*b]=payload[20+row*w*b:20+(row+1)*w*b]
                    elif op==6:presented.append(hashlib.sha256(states[struct.unpack('<I',payload)[0]][3]).hexdigest())
                    elif op==7:del states[struct.unpack('<I',payload)[0]]
                    else:assert op==8
                assert presented==native_expected and not states and published==len(data)
                assert sum(w==h==1 for w,h,_ in updates)>400 and any(w==h==512 for w,h,_ in updates)
                assert len(data)<17000000 and [op for op,_,_ in records[-3:]]==[7,7,8]
                report['tiled_native_storage_comparisons']=len(presented)
            elif mode=='delta':
                updates=[payload for op,_,payload in records if op==2]
                # One complete offscreen overwrite plus 46 changed primary
                # pixels; unchanged locks still produce every PRESENT.
                assert len(updates)==47 and len(data)<1100000 and published==len(data)
                assert all(struct.unpack_from('<II',p,12)==(1,1) and len(p)==24 for p in updates[1:])
                assert sum(op==6 for op,_,_ in records)==70
                assert [op for op,_,_ in records[-3:]]==[7,7,8]
            elif mode == 'short-archive':
                assert [op for op,_,_ in records[-3:]] == [7,7,8]
            else:
                assert records[-1] == (9,4,struct.pack('<I',2))
                if mode == 'record-archive':
                    assert len(records) == 4063 and len(data) < 1024*1024
                else:
                    assert len(data) > 60*1024*1024 and len(records) < 4096
                assert 'session_gap' not in (capture/'lifecycle.log').read_text()
            archived = dict(bytes=len(data), records=len(records), terminal_opcode=records[-1][0], sha256=live.sha(archive_path))
        elif mode == 'failed-archive':
            assert archive_path.is_dir() and 'session_file_failed' in (capture/'lifecycle.log').read_text()
        else:
            assert not archive_path.exists()
        report['cases'].append(dict(mode=mode, valid_session=valid, original_locks=locks, original_unlocks=unlocks,
                                   published_bytes=published, state=state, reason=reason, archive=archived, **observed))
        print(mode+': passed', flush=True)
    assert all(live.sha(ROOT/p) == h for p,h in fingerprints.items()), 'Source changed during execution'
    report.update(success=True, full_frame_comparisons=sum(c['presentations'] for c in report['cases']),
                  valid_sessions=sum(c['valid_session'] for c in report['cases']),
                  refused_sessions=sum(not c['valid_session'] for c in report['cases']))
    (run/'report.json').write_text(json.dumps(report, indent=2)+'\n');print(run/'report.json', flush=True)


if __name__ == '__main__':
    main()
