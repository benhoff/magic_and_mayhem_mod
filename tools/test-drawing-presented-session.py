#!/usr/bin/env python3
"""Execute actual native physical presentation; independent references stay here."""
import argparse
import copy
import importlib.util
import json
import os
import re
from pathlib import Path
import struct
import subprocess
import tempfile
from drawing_presented_session import sha

ROOT=Path(__file__).resolve().parents[1]


def load(name):
    spec=importlib.util.spec_from_file_location(name,ROOT/'tools'/name)
    module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module);return module


def write_inputs(root, segments, producer_inputs, assets, identity, formats, actions):
    root.mkdir(parents=True)
    inputs=[];asset_map={};frames=[];frame_index=0
    for index,(name,data) in enumerate(assets):
        token=f'asset-{index:04}';filename=token+'.bin';(root/filename).write_bytes(data)
        inputs.append(dict(id=token,path=filename,role='encoded-asset',sha256=sha(data)));asset_map[name]=token
    for token,data in producer_inputs.items():
        filename=token+'.bin';(root/filename).write_bytes(data)
        inputs.append(dict(id=token,path=filename,role='operation-input',sha256=sha(data)))
    for segment in segments:
        for f in segment['frames']:
            b=f['boundary'];assert f['frame_index']==frame_index
            index=17+frame_index
            frames.append(dict(identity=dict(session=identity,presentation_index=index,width=b['width'],height=b['height'],
                pixel_format=formats[frame_index],actions_completed=sum(a['before_presentation']<=index for a in actions)),
                input_ids=['state',segment['producer'],*asset_map.values()]))
            frame_index+=1
    state=dict(schema=1,kind='mnm-drawing-history-v1',session='18446744073709551615',assets=asset_map,segments=segments)
    data=json.dumps(state).encode();(root/'state.json').write_bytes(data)
    inputs.insert(0,dict(id='state',path='state.json',role='owned-state',sha256=sha(data)))
    config=dict(schema=1,session_identity=identity,inputs=inputs,actions=actions,frames=frames,effects=[])
    (root/'session.json').write_text(json.dumps(config));return config


def synthetic(root):
    helper=load('test-native-drawing-session-runner.py')
    rgb=[];raw=[]
    # Every RGB565 word is independently encoded into a source BMP. Its explicit
    # RGB triples are the physical expected plane, outside both child inputs.
    for value in range(65536):
        red=(value>>11)*8+(value>>13);green=((value>>5)&63)*4+((value>>9)&3)
        blue=(value&31)*8+((value&31)//4)
        rgb.append(bytes((red,green,blue,255)));raw.append(bytes((blue,green,red)))
    pixel_bytes=b''.join(b''.join(raw[y*256:(y+1)*256]) for y in range(255,-1,-1))
    bitmap=struct.pack('<2sIHHI',b'BM',54+len(pixel_bytes),0,0,54)+struct.pack(
        '<IiiHHIIiiII',40,256,256,1,24,0,len(pixel_bytes),0,0,0,0)+pixel_bytes
    descriptions=[('menu',8,6,0xabcd),('loading',256,256,0),('results',9,5,0x07e0)]
    segments=[];producers={};expected=[]
    for index,(stage,w,h,colour) in enumerate(descriptions):
        records=[helper.record(1,1,w,h),helper.record(2,5,w,h,colour)]
        if index==1:records.append(helper.record(3,20,w,h,payload=b'colours.bmp\0'))
        token=f'producer-{index}';producers[token]=helper.stream(records)
        segments.append(dict(stage=stage,producer=token,frames=[dict(frame_index=index,
            boundary=dict(sequence=len(records),queues=0,canvas=7,width=w,height=h))]))
        expected.append(b''.join(rgb) if index==1 else struct.pack('<'+str(w*h)+'H',*([colour]*(w*h))))
    actions=[dict(index=0,action='native-menu-source',before_presentation=17),
             dict(index=1,action='native-loading-source',before_presentation=18)]
    config=write_inputs(root,segments,producers,[('colours.bmp',bitmap)],'synthetic-physical',['rgb565','rgba8','rgb565'],actions)
    return config,expected


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--composition',type=Path,required=True);p.add_argument('--presenter',type=Path,required=True)
    p.add_argument('--report',type=Path,required=True);p.add_argument('--claims',type=Path)
    p.add_argument('--skip-retained',action='store_true',help='Development fixture only; no retained original comparison')
    a=p.parse_args()
    if a.report.exists():p.error('Refusing report overwrite')
    claims=json.loads(a.claims.read_text()) if a.claims else None
    paths=set(claims['sources'] if claims else [])
    paths.update(['tools/drawing_presented_session.py','tools/replay-drawing-presented-session.py',
        'tools/test-drawing-presented-session.py','tests/test-drawing-presented-session.py',
        'tools/test-native-drawing-session-runner.py','apps/drawing-session/main.cpp',
        'apps/drawing-session/CMakeLists.txt','apps/qt-shell/gl_viewport.cpp','apps/qt-shell/gl_viewport.hpp'])
    if not a.skip_retained:
        paths.update(str(f.relative_to(ROOT)) for f in (ROOT/'tests/fixtures/drawing-publication').rglob('*') if f.is_file())
        paths.update(['tools/drawing_publication_corpus.py','tools/drawing_scene_corpus.py','tools/drawing_publication.py'])
    sources={n:sha((ROOT/n).read_bytes()) for n in sorted(paths)}
    if claims and any(sources[n]!=h for n,h in claims['sources'].items()):p.error('Prospective source changed')
    parent=ROOT/'working/tests/drawing-presented-session';parent.mkdir(parents=True,exist_ok=True)
    run=Path(tempfile.mkdtemp(prefix='run-',dir=parent));env=dict(os.environ,QT_QPA_PLATFORM='xcb',LIBGL_ALWAYS_SOFTWARE='1',QT_SCALE_FACTOR='1')
    composition=a.composition.resolve();presenter=a.presenter.resolve();binaries={str(b):sha(b.read_bytes()) for b in (composition,presenter)}
    checks=[]
    def execute(name,input_root,output,expected_code=0,extra_env=None):
        command=['xvfb-run','-a','-s','-screen 0 2048x2048x24','python3',str(ROOT/'tools/replay-drawing-presented-session.py'),str(input_root),str(output),
                 '--composition',str(composition),'--presenter',str(presenter)]
        result=subprocess.run(command,env=env|dict(extra_env or {}),capture_output=True,timeout=600)
        (run/(name+'.log')).write_bytes(result.stdout+result.stderr)
        if result.returncode!=expected_code:raise ValueError(name+' failed; inspect '+str(run/(name+'.log')))
        checks.append(dict(name=name,exit_code=result.returncode));return result
    guard=subprocess.run(['python3','-B',str(ROOT/'tests/test-drawing-presented-session.py')],capture_output=True,timeout=60)
    (run/'guards.log').write_bytes(guard.stdout+guard.stderr);guard.check_returncode()
    config,expected=synthetic(run/'synthetic-input');output=run/'synthetic-output'
    execute('physical-colours-and-transitions',run/'synthetic-input',output)
    for index,plane in enumerate(expected):
        if (output/f'frame-{index:06}.bin').read_bytes()!=plane:raise ValueError('Independent physical expected plane differs')
    # Expected outcomes are not transported to the backend. Unknown contracts
    # cannot become successful caller effects by echoing the comparison oracle.
    original=json.loads((run/'synthetic-input/session.json').read_text())
    config=copy.deepcopy(original);config['effects']=[dict(id='effect',operation='flags-and-return-values',presentation_index=17)]
    (run/'synthetic-input/session.json').write_text(json.dumps(config))
    execute('unsupported-caller-effect',run/'synthetic-input',run/'unsupported-output',64)
    if (run/'unsupported-output').exists():raise ValueError('Unsupported contract published an output')
    config=copy.deepcopy(original);config['frames'][0]['identity']['oracle_sha256']='a'*64
    (run/'synthetic-input/session.json').write_text(json.dumps(config))
    execute('oracle-identity-refusal',run/'synthetic-input',run/'contaminated-output',1)
    (run/'synthetic-input/session.json').write_text(json.dumps(original))
    execute('preserve-existing-output',run/'synthetic-input',output,1)
    offscreen=subprocess.run([str(presenter),str(output/'presentation-input'),str(run/'offscreen-output')],
        env=env|{'QT_QPA_PLATFORM':'offscreen'},capture_output=True,timeout=60)
    (run/'offscreen-refusal.log').write_bytes(offscreen.stdout+offscreen.stderr)
    if offscreen.returncode==0 or (run/'offscreen-output').exists():raise ValueError('Nonphysical backend accepted as X11 presentation')
    checks.append(dict(name='non-x11-presentation-refusal',exit_code=offscreen.returncode))
    small=subprocess.run(['xvfb-run','-a','-s','-screen 0 320x240x24',str(presenter),
        str(output/'presentation-input'),str(run/'small-screen-output')],env=env,capture_output=True,timeout=60)
    (run/'small-screen-refusal.log').write_bytes(small.stdout+small.stderr)
    if small.returncode==0 or (run/'small-screen-output').exists():raise ValueError('Cropped physical screen was accepted')
    checks.append(dict(name='physical-screen-containment-refusal',exit_code=small.returncode))
    comparison=None
    if not a.skip_retained:
        from drawing_publication_corpus import admit
        from drawing_scene_corpus import raster_inputs
        register=json.loads((ROOT/'research/runtime/coverage/register.json').read_text())
        catalog=json.loads((ROOT/'tests/fixtures/drawing-publication/publication.json').read_text())
        raw,assets,boundaries,planes=admit(ROOT,catalog,register)
        segment=dict(stage='scene',producer='history',frames=[dict(frame_index=i,boundary=b) for i,b in enumerate(boundaries)])
        inputs=run/'retained-input';target=run/'retained-output'
        write_inputs(inputs,[segment],dict(history=raster_inputs(raw)),[(name,data) for name,_,data in assets],
            'retained-startup-physical',['rgb565']*len(planes),[dict(index=0,action='replay-retained-history',before_presentation=17)])
        execute('retained-startup-physical-comparison',inputs,target)
        for index,(_,plane) in enumerate(planes):
            if (target/f'frame-{index:06}.bin').read_bytes()!=plane:raise ValueError('Historical original/native physical frame mismatch')
        comparison=dict(frames=len(planes),pixels=sum(p['width']*p['height'] for p,_ in planes),mismatches=0,
            original_evidence=catalog['evidence'],original_report_sha256=catalog['historical_report_sha256'],
            original_boundary='observed_primary_blt',native_boundary='x11-screen-after-buffer-swap',
            new_original_execution=False,complete_presented_journey=False)
    if any(sha((ROOT/n).read_bytes())!=h for n,h in sources.items()) or any(sha(Path(n).read_bytes())!=h for n,h in binaries.items()):
        raise ValueError('Physical session sources or binaries changed during execution')
    report=dict(schema=1,success=True,sources=sources,sources_stable=True,claims=claims['claims'] if claims else [],
        binaries=binaries,checks=checks,admission_guard_methods=int(re.search(rb'Ran ([0-9]+) tests',guard.stderr)[1]),synthetic_frames=3,all_rgb565_colours=65536,
        synthetic_pixels=65536+48+45,historical_original_comparison=comparison,
        physical_presentation=True,original_execution=False,original_oracles_read_by_native=False,
        original_pixels_used_as_native_inputs=False,complete_journey_coverage=False,draw02_complete=False,
        remaining=['Independent original physical/action frame acquisition','General owned battle/campaign/save-load producers',
                   'Native caller-effect contracts','Real driver recovery equivalence'],artifacts=str(run.relative_to(ROOT)))
    a.report.parent.mkdir(parents=True,exist_ok=True)
    with a.report.open('x') as f:json.dump(report,f,indent=2);f.write('\n')
    print(json.dumps(dict(success=True,checks=len(checks),historical_original_comparison=comparison,report=str(a.report))))

if __name__=='__main__':main()
