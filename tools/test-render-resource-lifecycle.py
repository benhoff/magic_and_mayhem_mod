#!/usr/bin/env python3
"""Actual PE32 continuous resource identities, alias lifetime and native cleanup."""
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
CASES = ['cross-release', 'release-timeout', 'cross-alias', 'alias-timeout', 'cross-create', 'create-timeout','churn', 'pixels', 'alias', 'untracked', 'held', 'dc', 'contention', 'conflict', 'bounded']


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
        'tools/test-render-resource-lifecycle.py','tools/test-render-continuous-producer.py',
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
    parent=ROOT/'working/tests/render-resources';parent.mkdir(parents=True,exist_ok=True)
    run=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(run,flush=True)
    dll=live.load('resource_build','tools/build-render-bridge.py').build(True)
    stage=live.load('resource_stage','tools/prepare-shadow-experiment.py')
    env={k:v for k,v in os.environ.items() if not k.startswith('MNM_')}
    env.update(QT_QPA_PLATFORM='xcb',LIBGL_ALWAYS_SOFTWARE='1',WINEDEBUG='-all',WINEPREFIX=str(ROOT/'working/tests/render-wine'))
    report=dict(schema=1,sources=fingerprints,cases=[],scope='Actual synthetic PE32 COM lifetime '
                'through owned v2 producer and native GPU. Increasing IDs, final/nonfinal and untracked '
                'Release, observed aliases, address/slot reuse, reclaimed surface/pixel/snapshot budgets. '
                'Held lock/DC, contention, ambiguous aliases and default bounded Release refuse. '
                'No original game, COM implementation equivalence, automatic recovery or ID exhaustion execution.')
    for mode in args.case or CASES:
        case=run/mode;case.mkdir();capture=case/'capture';capture.mkdir()
        channel,frame=case/'commands.bin',case/'frame.bin'
        header=bytearray(ring.initial_header());struct.pack_into('<I',header,16,123)
        live.create(channel,header,ring.SIZE);live.create(frame,live.frame_v1.initial_header(),live.frame_v1.SIZE)
        shutil.copyfile(dll,case/dll.name)
        (case/'selftest.exe').write_bytes(stage.add_import((dll.parent/'selftest.exe').read_bytes(),
            dll='MnmRender.dll',symbol_name='RenderAnchor',section_name=b'.mnmgl'))
        output,active=case/'qt.json',case/'producer.active'
        child=dict(env,MNM_RENDER_ORDERED_COPIES='1',MNM_RESOURCE_SELFTEST=mode,MNM_RENDER_CONTINUOUS='0' if mode=='bounded' else '1',
                   MNM_RENDER_OWNED_SESSION='1',MNM_RENDER_SESSION_ARCHIVE='0' if mode=='pixels' else '1',
                   MNM_RENDER_COMMAND_CHANNEL='Z:'+str(channel).replace('/', '\\'),
                   MNM_RENDER_STREAM='Z:'+str(frame).replace('/', '\\'),
                   MNM_RENDER_LOCK_CAPTURE_DIR='Z:'+str(capture).replace('/', '\\'))
        valid=mode in ['churn','pixels','alias','untracked','cross-release','cross-alias','cross-create']
        with (case/'qt.log').open('w') as qlog,(case/'wine.log').open('w') as wlog:
            qt=subprocess.Popen([str(args.build.resolve()/'live-render-channel-test'),str(channel),str(active),str(output)],
                                env=env,stdout=qlog,stderr=qlog);wine=None
            try:
                live.wait_ready(Path(str(output)+'.ready'),qt)
                wine=subprocess.Popen(['wine',str(case/'selftest.exe')],cwd=case,env=child,stdout=wlog,stderr=wlog)
                active.write_text(str(wine.pid));status=wine.wait(timeout=35)
                assert status==0,(mode,status,(case/'wine.log').read_text())
                active.unlink();assert qt.wait(timeout=35)==(0 if valid else 8),(mode,output.read_text())
            finally:
                for process in [wine,qt]:
                    if process is not None and process.poll() is None:process.terminate();process.wait(timeout=5)
        observed=json.loads(output.read_text());counts=struct.unpack('<6I',(case/'resource-counts.bin').read_bytes())
        expected=[rgba_hash(32,16,0xa5123456)]
        if mode in ['churn','pixels']:
            for i in range(40 if mode=='churn' else 12):
                color=((i*37)&255)<<16|((i*71)&255)<<8|((i*19)&255)
                expected.append(rgba_hash(32+(i%2)*32 if mode=='churn' else 2048,16 if mode=='churn' else 1024,color))
        elif mode in ['alias','untracked','cross-release','cross-alias','cross-create']:expected.append(rgba_hash(32,16,0xa5654321))
        assert observed['frames']==expected,(mode,observed['frames'],expected)
        assert observed['presentations']==len(expected) and observed['success']==valid and observed['before_producer_exit']
        assert observed.get('native_readbacks',0)==observed.get('rgba_readbacks',0)==observed['viewport_uploads']==observed.get('live_surfaces',0)==0
        expected_counts={'churn':(196,196,0,196,0,41),'pixels':(13,13,0,13,0,13),
                         'alias':(62,62,20,61,0,2),'untracked':(2,2,0,161,0,2),
                         'held':(3,2,0,1,0,1),'dc':(2,2,0,1,1,1),
                         'contention':(3,3,0,1,0,2),'conflict':(2,2,1,0,0,1),'bounded':(2,2,0,1,0,1)}
        expected_counts.update({'cross-release':(4,4,1,4,0,2),'release-timeout':(4,4,1,4,0,2),'cross-alias':(3,3,1,3,0,2),'alias-timeout':(3,3,1,3,0,2),'cross-create':(3,3,0,2,0,2),'create-timeout':(3,3,0,2,0,2)})
        if mode in ['cross-release', 'release-timeout', 'cross-alias', 'alias-timeout', 'cross-create', 'create-timeout']:
            assert struct.unpack('<3I',(case/'lifetime-counts.bin').read_bytes())==(2 if 'create' in mode else 0,1,1)
            assert ('copy_order_timeout ' if mode.endswith('timeout') else 'copy_order_wait_acquired ') in (capture/'lifecycle.log').read_text()
        assert counts==expected_counts[mode],(mode,counts)
        control=channel.read_bytes()[:64];published,state,reason=struct.unpack_from('<III',control,20)
        assert state==(2 if valid else 3) and reason==(0 if valid else 2)
        if valid:assert struct.unpack_from('<I',control,36)[0]==published
        archived=None;archive=capture/'session-00000001.bin'
        if mode=='pixels':
            assert not archive.exists() and published>96*1024*1024
        else:
            archived=check_archive(archive.read_bytes(),valid)
            if valid:
                count={'churn':196,'alias':41,'untracked':1,'cross-release':3,'cross-alias':2,'cross-create':2}[mode]
                assert archived['created']==list(range(1,count+1))
                if mode=='churn':assert archived['peak_surfaces']==32
            elif mode not in ['cross-release', 'release-timeout', 'cross-alias', 'alias-timeout', 'cross-create', 'create-timeout']:assert not archived['deleted']
        report['cases'].append(dict(mode=mode,valid_session=valid,original_counts=list(counts),
                                   published_bytes=published,state=state,reason=reason,archive=archived,**observed))
        print(mode+': passed',flush=True)
    assert all(live.sha(ROOT/p)==h for p,h in fingerprints.items()),'Source changed during execution'
    report.update(success=True,full_frame_comparisons=sum(c['presentations'] for c in report['cases']),
                  valid_sessions=sum(c['valid_session'] for c in report['cases']),
                  refused_sessions=sum(not c['valid_session'] for c in report['cases']))
    (run/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(run/'report.json',flush=True)


if __name__=='__main__':main()
