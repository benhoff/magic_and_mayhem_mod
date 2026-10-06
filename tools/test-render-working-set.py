#!/usr/bin/env python3
"""Bounded PE32 consumer residency with independent complete fixture/GPU pixels."""
import argparse
import importlib.util
import json
import os
from pathlib import Path
import shutil
import struct
import subprocess
import tempfile
ROOT=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('working_set_mutations',ROOT/'tools/test-render-mutations.py');mutation=importlib.util.module_from_spec(spec);spec.loader.exec_module(mutation)
live,ring=mutation.live,mutation.ring
CASES=['copies','held-skip','held-full','dc-skip','alias','pixels']
def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('build',type=Path);parser.add_argument('--case',action='append',choices=CASES);args=parser.parse_args()
    assert os.environ.get('DISPLAY'),'Use xvfb-run -a'
    paths=sorted((ROOT/'runtime/render').glob('*.[ch]'))+[ROOT/p for p in ['tools/test-render-working-set.py','tools/test-render-mutations.py','tools/test-render-resource-lifecycle.py','tools/test-render-continuous-producer.py','tools/test-live-render-channel.py','tools/build-render-bridge.py','tools/prepare-shadow-experiment.py','tests/live-render-channel-test.cpp','renderer/commands.cpp','renderer/commands.hpp','renderer/command_state.hpp','renderer/command_consumer.cpp','renderer/blit.cpp','renderer/blit.hpp','apps/qt-shell/command_channel.cpp','apps/qt-shell/command_channel.hpp','apps/qt-shell/live_command_renderer.cpp','apps/qt-shell/live_command_renderer.hpp','apps/qt-shell/gl_viewport.cpp','apps/qt-shell/gl_viewport.hpp','protocols/include/mnm/render_stream_v2.h','protocols/include/mnm/render_command_ring.h','protocols/include/mnm/render_commands_v1.h','protocols/include/mnm/render_commands_v2.h']]
    sources={str(p.relative_to(ROOT)):live.sha(p) for p in paths};parent=ROOT/'working/tests/render-working-set';parent.mkdir(parents=True,exist_ok=True);run=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(run,flush=True)
    dll=live.load('working_set_build','tools/build-render-bridge.py').build(True);stage=live.load('working_set_stage','tools/prepare-shadow-experiment.py')
    env={k:v for k,v in os.environ.items() if not k.startswith('MNM_')};env.update(QT_QPA_PLATFORM='xcb',LIBGL_ALWAYS_SOFTWARE='1',WINEDEBUG='-all',WINEPREFIX=str(ROOT/'working/tests/render-wine'))
    report=dict(schema=1,sources=sources,cases=[],scope='Actual PE32 finite32-resource/16Mi-pixel residency,65 independent small CPU resources, pinned copy/flip operands, held lock/DC exclusion, all-pinned refusal, alias final Release/address reuse and large pixel pressure. Complete original fixture pixels versus native GPU; no original game/driver equivalence or replacement.')
    for mode in args.case or CASES:
        case=run/mode;case.mkdir();capture=case/'capture';capture.mkdir();channel=case/'commands.bin';frame=case/'frame.bin';header=bytearray(ring.initial_header());struct.pack_into('<I',header,16,123)
        live.create(channel,header,ring.SIZE);live.create(frame,live.frame_v1.initial_header(),live.frame_v1.SIZE);shutil.copyfile(dll,case/dll.name)
        (case/'selftest.exe').write_bytes(stage.add_import((dll.parent/'selftest.exe').read_bytes(),dll='MnmRender.dll',symbol_name='RenderAnchor',section_name=b'.mnmgl'))
        active=case/'producer.active';output=case/'qt.json';child=dict(env,MNM_WORKING_SET_SELFTEST=mode,MNM_RENDER_CONTINUOUS='1',MNM_RENDER_PALETTE_RESOURCES='1',MNM_RENDER_SESSION_ARCHIVE='1',MNM_RENDER_COMMAND_CHANNEL='Z:'+str(channel).replace('/','\\'),MNM_RENDER_STREAM='Z:'+str(frame).replace('/','\\'),MNM_RENDER_LOCK_CAPTURE_DIR='Z:'+str(capture).replace('/','\\'))
        valid=mode!='held-full';wine=qt=None
        with (case/'qt.log').open('w') as qlog,(case/'wine.log').open('w') as wlog:
            try:
                qt=subprocess.Popen([str(args.build.resolve()/'live-render-channel-test'),str(channel),str(active),str(output)],env=env,stdout=qlog,stderr=qlog);live.wait_ready(Path(str(output)+'.ready'),qt)
                wine=subprocess.Popen(['wine',str(case/'selftest.exe')],cwd=case,env=child,stdout=wlog,stderr=wlog);active.write_text(str(wine.pid))
                status=wine.wait(timeout=40);assert status==0,(mode,status,(case/'wine.log').read_text());active.unlink();assert qt.wait(timeout=35)==(0 if valid else 8),(mode,(case/'qt.log').read_text())
            finally:
                for process in [wine,qt]:
                    if process and process.poll() is None:process.terminate();process.wait(timeout=5)
        observed=json.loads(output.read_text());expected=mutation.frames((case/'mutation-frames.bin').read_bytes());assert observed['frames']==(expected if valid else expected[:1]),(mode,observed,expected)
        assert observed['success']==valid and observed.get('native_readbacks',0)==observed.get('rgba_readbacks',0)==observed['viewport_uploads']==observed.get('live_surfaces',0)==0
        data=(capture/'session-00000001.bin').read_bytes();assert data[:8]==b'MNMCMD02';at=16;resources={};created=[];deletes=[];copies=[];updates=[];peak=0;peak_pixels=0;ops=[];ordinal=0
        while at<len(data):
            op,seq,size=struct.unpack_from('<III',data,at);ordinal+=1;assert seq==ordinal;fields=data[at+12:at+12+size];ops.append(op)
            if op==1:
                identity,width,height,bits=struct.unpack_from('<4I',fields);assert identity> (created[-1] if created else 0) and identity not in resources
                resources[identity]=width*height;created.append(identity);assert len(resources)<=32 and sum(resources.values())<=16777216
            elif op==2:
                identity=struct.unpack_from('<I',fields)[0];assert identity in resources;updates.append((seq,identity))
            elif op in [3,11]:
                a,b=struct.unpack_from('<2I',fields);assert a in resources and b in resources;copies.append((seq,a,b))
            elif op==6:assert struct.unpack_from('<I',fields)[0] in resources
            elif op==7:
                identity=struct.unpack_from('<I',fields)[0];assert identity in resources;del resources[identity];deletes.append((seq,identity))
            elif op==8:assert not resources
            peak=max(peak,len(resources));peak_pixels=max(peak_pixels,sum(resources.values()));at+=12+size
        assert at==len(data) and ops[-1]==(8 if valid else 9)
        if valid:assert len(created)>32 if mode!='pixels' else len(created)==8
        if mode=='copies':assert len(expected)==132 and len(copies)==258 and ops.count(11)==2
        if mode in ['held-skip','dc-skip']:
            assert len(created)>=65 and next(seq for seq,i in deletes if i==2)>max(seq for seq,i in updates if i==2),'Borrowed ID evicted before successful original release'
        if mode=='held-full':assert len(created)==32 and not deletes
        if mode=='alias':assert struct.unpack('<II',(case/'alias-counts.bin').read_bytes())==(1,66) and created[-1]==67
        if mode=='pixels':assert peak<32 and peak_pixels==3*2048*2048+24 and len(created)==8
        counts=list(struct.unpack('<10I',(case/'mutation-counts.bin').read_bytes()));assert counts[0]==counts[1]
        cs_counts=list(struct.unpack('<II',(case/'cs-counts.bin').read_bytes()));assert cs_counts[0]==cs_counts[1]
        storage=list(struct.unpack('<6I',(case/'storage.bin').read_bytes()));assert storage[:4]==[0,0,0,1] and not storage[-1]
        report['cases'].append(dict(mode=mode,success=True,valid=valid,created=len(created),deletes=len(deletes),peak_surfaces=peak,peak_pixels=peak_pixels,fixture_owned_bytes=storage[4],original_counts=counts,cs_counts=cs_counts,storage=storage,consumer=observed));print(mode+': passed',flush=True)
    assert all(live.sha(ROOT/p)==h for p,h in sources.items()),'Source changed during execution'
    report.update(success=True,full_frame_comparisons=sum(c['consumer']['presentations'] for c in report['cases']));(run/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(run/'report.json',flush=True)
if __name__=='__main__':main()
