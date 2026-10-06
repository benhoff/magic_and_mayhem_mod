#!/usr/bin/env python3
"""PE32 startup/shutdown transition contention and admitted callback quiescence."""
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
spec = importlib.util.spec_from_file_location('resource_continuous', ROOT/'tools/test-render-continuous-producer.py')
continuous = importlib.util.module_from_spec(spec)
spec.loader.exec_module(continuous)
live, ring = continuous.live, continuous.ring
CASES = ['cross','timeout','reentrant','guard','guard-off','completed']


def rgba_hash(width, height, color):
    return hashlib.sha256(struct.pack('<I', 0xff000000 | (color & 0xffffff))*(width*height)).hexdigest()


def check_archive(data, valid):
    records = continuous.archive_records(data)
    live_ids, created, deleted, peak_pixels, peak_surfaces = {}, [], [], 0, 0
    at = 16
    for op, size, _ in records:
        fields = data[at+12:at+12+size]
        if op == 1:
            rid, width, height, bits, r, g, b = struct.unpack_from('<7I', fields)
            assert rid and (not created or rid > created[-1]) and rid not in live_ids
            assert bits == 32 and (r,g,b) == (0xff0000,0xff00,0xff)
            live_ids[rid] = width*height;created.append(rid)
            peak_pixels = max(peak_pixels, sum(live_ids.values()));peak_surfaces = max(peak_surfaces, len(live_ids))
        elif op in [2,5,6,7]:
            rid = struct.unpack_from('<I', fields)[0];assert rid in live_ids
            if op == 7:
                del live_ids[rid];deleted.append(rid)
        elif op == 8:
            assert not live_ids
        else:
            assert op == 9 and not valid
        at += 12+size
    assert peak_surfaces <= 32 and peak_pixels <= 16777216
    assert records[-1][0] == (8 if valid else 9)
    if valid:
        assert sorted(created) == sorted(deleted) and len(deleted) == len(set(deleted))
    return dict(created=created, deleted=deleted, peak_surfaces=peak_surfaces, peak_pixels=peak_pixels,
                terminal_opcode=records[-1][0], records=len(records), bytes=len(data))


def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('build',type=Path)
    parser.add_argument('--case', action='append', choices=CASES);args=parser.parse_args()
    if not os.environ.get('DISPLAY'):parser.error('Run under xvfb-run -a')
    paths=sorted((ROOT/'runtime/render').glob('*.[ch]'))+[ROOT/p for p in [
        'tools/test-render-lifecycle-races.py','tools/test-render-draw-startup.py','tools/test-render-resource-lifecycle.py','tools/test-render-continuous-producer.py',
        'tools/test-live-render-channel.py','tools/build-render-bridge.py','tools/prepare-shadow-experiment.py',
        'runtime/shadow/win32_min.h','tests/live-render-channel-test.cpp',
        'renderer/blit.cpp','renderer/blit.hpp','renderer/commands.cpp','renderer/commands.hpp',
        'renderer/command_state.hpp','renderer/command_consumer.cpp',
        'apps/qt-shell/command_channel.cpp','apps/qt-shell/command_channel.hpp',
        'apps/qt-shell/live_command_renderer.cpp','apps/qt-shell/live_command_renderer.hpp',
        'apps/qt-shell/gl_viewport.cpp','apps/qt-shell/gl_viewport.hpp',
        'protocols/include/mnm/render_commands_v1.h','protocols/include/mnm/render_commands_v2.h',
        'protocols/include/mnm/render_command_ring.h',
        'protocols/python/mnm_protocols/render_commands_v1.py','protocols/python/mnm_protocols/render_commands_v2.py']]
    paths += [ROOT/'renderer/dib.cpp',ROOT/'renderer/dib.hpp']
    fingerprints={str(p.relative_to(ROOT)):live.sha(p) for p in paths}
    parent=ROOT/'working/tests/render-lifecycle-races';parent.mkdir(parents=True,exist_ok=True)
    run=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(run,flush=True)
    dll=live.load('resource_build','tools/build-render-bridge.py').build(True)
    stage=live.load('resource_stage','tools/prepare-shadow-experiment.py')
    env={k:v for k,v in os.environ.items() if not k.startswith('MNM_')}
    env.update(QT_QPA_PLATFORM='xcb',LIBGL_ALWAYS_SOFTWARE='1',WINEDEBUG='-all',WINEPREFIX=str(ROOT/'working/tests/render-wine'))
    report=dict(schema=1,sources=fingerprints,cases=[],scope='Actual PE32 startup/shutdown full-transition contention, bounded admitted Unlock drain, timeout refusal without stream mutation, same-thread reentrant refusal and default-off lifecycle guard. Independent complete native pixels and terminal ownership. No original game/driver, external recovery collision or terminated-thread guarantee.')
    for mode in args.case or CASES:
        case=run/mode;case.mkdir();capture=case/'capture';capture.mkdir()
        channel,frame=case/'commands.bin',case/'frame.bin'
        header=bytearray(ring.initial_header());struct.pack_into('<I',header,16,123)
        live.create(channel,header,ring.SIZE);live.create(frame,live.frame_v1.initial_header(),live.frame_v1.SIZE)
        shutil.copyfile(dll,case/dll.name)
        (case/'selftest.exe').write_bytes(stage.add_import((dll.parent/'selftest.exe').read_bytes(),
            dll='MnmRender.dll',symbol_name='RenderAnchor',section_name=b'.mnmgl'))
        output,active=case/'qt.json',case/'producer.active'
        child=dict(env,MNM_RENDER_ORDERED_COPIES='0' if mode=='guard-off' else '1',MNM_LIFECYCLE_RACE_SELFTEST=mode,MNM_RENDER_CONTINUOUS='1',
                   MNM_RENDER_OWNED_SESSION='1',MNM_RENDER_SESSION_ARCHIVE='1',
                   MNM_RENDER_COMMAND_CHANNEL='Z:'+str(channel).replace('/', '\\'),
                   MNM_RENDER_STREAM='Z:'+str(frame).replace('/', '\\'),
                   MNM_RENDER_LOCK_CAPTURE_DIR='Z:'+str(capture).replace('/', '\\'))
        valid=True
        with (case/'qt.log').open('w') as qlog,(case/'wine.log').open('w') as wlog:
            qt=subprocess.Popen([str(args.build.resolve()/'live-render-channel-test'),str(channel),str(active),str(output)],env=env,stdout=qlog,stderr=qlog);wine=None
            try:
                live.wait_ready(Path(str(output)+'.ready'),qt)
                wine=subprocess.Popen(['wine',str(case/'selftest.exe')],cwd=case,env=child,stdout=wlog,stderr=wlog)
                active.write_text(str(wine.pid))
                status=wine.wait(timeout=35)
                assert status==0,(mode,status,(case/'wine.log').read_text())
                active.unlink();assert qt.wait(timeout=35)==(0 if valid else 8),(mode,output.read_text())
            finally:
                for process in [wine,qt]:
                    if process is not None and process.poll() is None:process.terminate();process.wait(timeout=5)
        observed=json.loads(output.read_text());counts=struct.unpack('<6I',(case/'resource-counts.bin').read_bytes())
        expected=[rgba_hash(32,16,0xa5123456)]
        if valid:expected.append(rgba_hash(32,16,0xa5654321))
        assert observed['frames']==expected,(mode,observed['frames'],expected)
        assert observed['presentations']==len(expected) and observed['success']==valid and observed['before_producer_exit']
        assert observed.get('native_readbacks',0)==observed.get('rgba_readbacks',0)==observed['viewport_uploads']==observed.get('live_surfaces',0)==0
        assert counts==((3,3,0,1,0,3) if mode=='completed' else (2,2,0,1,0,2)),(mode,counts)
        race_counts=struct.unpack('<3I',(case/'race-counts.bin').read_bytes())
        want=(1,1,0) if mode=='cross' else (1,0,1) if mode=='timeout' else (0,0,0)
        assert race_counts==want,(mode,race_counts,want)
        log=(capture/'lifecycle.log').read_text()
        if mode in ['timeout','reentrant']:assert ('command_shutdown_admission_timeout ' if mode=='timeout' else 'command_shutdown_reentrant ') in log
        control=channel.read_bytes()[:64];published,state,reason=struct.unpack_from('<III',control,20)
        assert state==(2 if valid else 3) and reason==(0 if valid else 2)
        if valid:assert struct.unpack_from('<I',control,36)[0]==published
        archived=check_archive((capture/'session-00000001.bin').read_bytes(),valid)
        assert archived['created']==[1]
        if valid:assert archived['deleted']==[1]
        report['cases'].append(dict(mode=mode,valid_session=valid,original_counts=list(counts),
                                   race_counts=list(race_counts),published_bytes=published,state=state,reason=reason,archive=archived,**observed))
        print(mode+': passed',flush=True)
    assert all(live.sha(ROOT/p)==h for p,h in fingerprints.items()),'Source changed during execution'
    report.update(success=True,full_frame_comparisons=sum(c['presentations'] for c in report['cases']),
                  valid_sessions=sum(c['valid_session'] for c in report['cases']),
                  refused_sessions=sum(not c['valid_session'] for c in report['cases']))
    (run/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(run/'report.json',flush=True)


if __name__=='__main__':main()
