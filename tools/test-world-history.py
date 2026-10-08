#!/usr/bin/env python3
"""Validate native canvas history and diagnose original pre-consumer pixels."""
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import struct
import subprocess
import tempfile

ROOT=Path(__file__).resolve().parents[1]


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--capture-report',type=Path,required=True)
    parser.add_argument('--startup-report',type=Path)
    args=parser.parse_args()
    if not os.environ.get('DISPLAY'):parser.error('Run under xvfb-run -a')
    spec=importlib.util.spec_from_file_location('world',ROOT/'tools/test-world-frames.py')
    world=importlib.util.module_from_spec(spec);spec.loader.exec_module(world)
    parent=ROOT/'working/tests/world-history';parent.mkdir(parents=True,exist_ok=True)
    out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(out,flush=True)
    sources=world.sources()
    for name in ['tools/test-world-history.py','tests/scene-history-test.cpp']:
        sources[name]=world.sha(ROOT/name)
    report={'success':False,'sources':sources,'source_executable_sha256':world.BUILD_HASH,
        'scope':'Native contiguous owned canvas history with explicit zero initialization compared to private original execution; separate live before/after diagnostics. Original startup initialization and live native history delivery remain pending.',
        'original_pixels_used_as_native_inputs':False,'original_work_bypassed':False}
    build=ROOT/'working/build/world-frame';reference=out/'reference'
    inputs={}
    def run(name,command,expected=0):
        with (out/(name+'.log')).open('x') as log:
            result=subprocess.run([str(c) for c in command],stdout=log,stderr=subprocess.STDOUT,timeout=180,
                env={**os.environ,'QT_QPA_PLATFORM':'xcb','LIBGL_ALWAYS_SOFTWARE':'1'})
        if result.returncode!=expected:raise RuntimeError(name+' failed; inspect '+str(out/(name+'.log')))
    def pin(path):inputs[str(path.relative_to(ROOT))]=world.sha(path)
    def words(bytes_):return struct.unpack('<'+'H'*(len(bytes_)//2),bytes_)
    run('original-before',[ROOT/'tools/original-manifest.sh','verify'])
    try:
        pe=ROOT/'working/game-nocd/Chaos.exe'
        if world.sha(pe)!=world.BUILD_HASH:raise ValueError('Unsupported original executable')
        report['entry_anchors']=world.reference_anchor(pe);pin(pe)
        run('configure',['cmake','-S',ROOT/'compat/legacy','-B',build,'-DCMAKE_BUILD_TYPE=Debug'])
        run('build',['cmake','--build',build,'--target','mnm-world-history-preview','mnm-world-frame-preview','scene-history-test','scene-renderer-test','world-frame-test','-j4'])
        run('ctest',['ctest','--test-dir',build,'-R','^(native-scene-history|native-shared-scene-renderer|native-world-frame)$','--output-on-failure'])
        run('reference-build',['g++','-m32','-std=c++17','-O2','-Wall','-Wextra','-Werror','-fno-pie','-no-pie',ROOT/'tests/world-frame-reference.cpp','-o',reference])

        fixture=out/'fixture';fixture.mkdir();assetroot=fixture/'assets';assetroot.mkdir()
        full=world.encoded_frame(32,12);masked=world.encoded_frame(6,5,masked=True)
        for name,frame in [('full',full),('mask',masked)]:
            path=assetroot/(name+'.spr');path.write_bytes(world.sprite(frame));pin(path)
        colours=[((i*97)&65535) for i in range(256)];clip=(0,0,32,12)
        draws=[world.wire_record(full,0,0,0,clip,colours)]
        draws += [world.wire_record(masked,op,4+op*2,2,clip,colours,offsets=[i%3 for i in range(16)] if op==3 else None) for op in range(1,6)]
        draws += [world.wire_record(masked,0,-3,-2,clip,colours),world.wire_record(masked,0,40,20,clip,colours)]
        timeline=[];snapshots=[]
        for i,draw in enumerate(draws):
            path=fixture/(str(i+1)+'.bin');path.write_bytes(world.world_wire([draw]));pin(path);snapshots.append(path)
            timeline.append({'canvas':1,'queue':i+1,'reset':i==0,'snapshot':path.name,'sha256':world.sha(path)})
        bindings=fixture/'bindings.json';bindings.write_text(json.dumps(world.pinned_bindings(assetroot,{world.identity(full,1),world.identity(masked,1)}),indent=2)+'\n');pin(bindings)
        timelinePath=fixture/'timeline.json';timelinePath.write_text(json.dumps({'version':1,'native_clear_word':0,'frames':timeline},indent=2)+'\n');pin(timelinePath)
        native=out/'native-history'
        run('native-history',[build/'mnm-world-history-preview','--root',assetroot,'--bindings',bindings,'--timeline',timelinePath,'--output',native])
        previous=None;results=[]
        for i,path in enumerate(snapshots):
            destination=out/('original-history-'+str(i+1)+'.565')
            run('original-history-'+str(i+1),[reference,pe,path,destination,*([previous] if previous else [])])
            actual=(native/(str(i+1)+'.565')).read_bytes();expected=destination.read_bytes()
            if actual!=expected:raise RuntimeError('Native history differs from independent original frame '+str(i+1))
            results.append({'queue':i+1,'pixels':len(actual)//2,'native_sha256':hashlib.sha256(actual).hexdigest(),'original_sha256':world.sha(destination)})
            previous=destination
        run('stateless-final',[reference,pe,snapshots[-1],out/'stateless-final.565'])
        if previous.read_bytes()==(out/'stateless-final.565').read_bytes():raise RuntimeError('History fixture did not require retained pixels')
        # A gap must be refused before creating any rendered output, even if the
        # wire publication counters happen to be adjacent.
        bad=json.loads(timelinePath.read_text());bad['frames'][1]['queue']=3
        gap=fixture/'gap.json';gap.write_text(json.dumps(bad,indent=2)+'\n');pin(gap)
        run('gap-refusal',[build/'mnm-world-history-preview','--root',assetroot,'--bindings',bindings,'--timeline',gap,'--output',out/'gap-output'],2)
        if (out/'gap-output').exists():raise RuntimeError('Rejected gap created rendered output')
        report.update(native_history_frames=results,native_history_pixels=sum(x['pixels'] for x in results),
            native_history_original_pixels_match=True,history_required=True,gap_refused=True)

        capturePath=args.capture_report.resolve();pin(capturePath);capture=json.loads(capturePath.read_text())
        if not capture['success'] or capture['source_executable_sha256']!=world.BUILD_HASH:raise ValueError('Invalid diagnostic capture report')
        experiment=ROOT/capture['experiment'];snapshots=sorted((experiment/'capture').glob('world-*.bin'))
        for name,fingerprint in capture['world_frames'].items():
            path=experiment/name
            if world.sha(path)!=fingerprint:raise ValueError('Captured diagnostic bytes changed')
            pin(path)
        needed={world.identity(raw,header[9]) for path in snapshots for header,raw in world.records(path.read_bytes())}
        bindings=out/'live-bindings.json';bindings.write_text(json.dumps(world.pinned_bindings(experiment/'game',needed),indent=2)+'\n');pin(bindings)
        diagnostics=[]
        for i,path in enumerate(snapshots):
            before=path.with_suffix('.before.565');after=path.with_suffix('.after.565');pin(before);pin(after)
            run('live-private-zero-'+str(i),[reference,pe,path,out/('live-zero-'+str(i)+'.565')])
            run('live-private-before-'+str(i),[reference,pe,path,out/('live-before-'+str(i)+'.565'),before])
            prefix=out/('live-native-'+str(i))
            run('live-native-'+str(i),[build/'mnm-world-frame-preview','--root',experiment/'game','--snapshot',path,'--bindings',bindings,'--output',prefix])
            nativePixels=prefix.with_suffix('.565').read_bytes();zero=(out/('live-zero-'+str(i)+'.565')).read_bytes()
            initialized=(out/('live-before-'+str(i)+'.565')).read_bytes();header=struct.unpack_from('<18I',path.read_bytes(),8);width,height,stride=header[4:7]
            crop=lambda raw:b''.join(raw[y*stride*2:(y*stride+width)*2] for y in range(height))
            live=crop(after.read_bytes());prior=crop(before.read_bytes())
            if nativePixels!=zero or initialized!=live:raise RuntimeError('Live diagnostic comparison differs beyond initial canvas')
            liveWords=words(live);priorWords=words(prior)
            differences=[j for j,(a,b) in enumerate(zip(words(zero),liveWords)) if a!=b]
            carried=[j for j in differences if priorWords[j]==liveWords[j]]
            diagnostics.append({'snapshot':str(path.relative_to(ROOT)),'pixels':width*height,'native_zero_equals_original_zero':True,
                'private_original_before_equals_live_after':True,'zero_live_mismatches':len(differences),
                'unchanged_prior_mismatches':len(carried),'mismatch_offsets':differences,
                'snapshot_sha256':world.sha(path),'before_sha256':world.sha(before),'after_sha256':world.sha(after)})
        report.update(live_diagnostics=diagnostics,live_diagnostic_pixels=sum(x['pixels'] for x in diagnostics),
            original_before_used_only_by_private_original_reference=True,
            retained_initial_pixels_confirmed=any(x['zero_live_mismatches'] and x['unchanged_prior_mismatches']==x['zero_live_mismatches'] for x in diagnostics))
        if args.startup_report:
            startupPath=args.startup_report.resolve();pin(startupPath);startup=json.loads(startupPath.read_text())
            if not startup['success'] or startup['source_executable_sha256']!=world.BUILD_HASH:raise ValueError('Invalid startup capture report')
            path=ROOT/startup['experiment']/startup['canvas_lifetime']['path'];pin(path)
            if world.sha(path)!=startup['canvas_lifetime']['sha256']:raise ValueError('Lifetime bytes changed')
            lifetime=list(struct.iter_unpack('<16I',path.read_bytes()[64:]));
            report.update(startup_first_canvas_nonzero_pixels=lifetime[0][7],startup_pointer_tokens=sorted({x[3] for x in lifetime}),
                startup_refusals=startup.get('world_refusals',[]),original_initialization_recovered=False)
        if any(world.sha(ROOT/path)!=fingerprint for path,fingerprint in sources.items()):raise RuntimeError('Sources changed during comparison')
        report.update(success=True,sources_stable=True,inputs=inputs,
            binaries={str(path.relative_to(ROOT)):world.sha(path) for path in [reference,build/'mnm-world-history-preview',build/'mnm-world-frame-preview']})
    finally:
        run('original-after',[ROOT/'tools/original-manifest.sh','verify']);report['original_manifest_verified_before_after']=True
        (out/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(out/'report.json',flush=True)


if __name__=='__main__':main()
