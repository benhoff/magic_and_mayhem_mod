#!/usr/bin/env python3
"""Compare actual primary readbacks with source-owned native scene publications."""
import argparse
import importlib.util
import json
import os
from pathlib import Path
import struct
import subprocess
import tempfile
from drawing_corpus import ROOT, BUILD, load_tool, sha, compare
from drawing_publication import analyze
from drawing_scene_corpus import raster_inputs, asset_name


def source_assets(game,records):
    """Bind real encoded files. Original output planes cannot provide bindings."""
    helper=load_tool('retain-drawing-scene');needed=set();active=False
    for r,p in records:
        if r[2]==11:active=True
        elif r[2]==12:active=False
        elif active and r[2]==9:needed.add(helper.frame_key(p[:r[19]],r[17]))
    files={p.relative_to(game).as_posix().casefold():p for p in game.rglob('*') if p.is_file()}
    selected={}
    for r,p in records:
        if r[2] in (7,20):
            requested=p[:-1].decode('ascii');selected[requested]=files[asset_name(requested).casefold()]
    for name,path in sorted(files.items()):
        if not needed:break
        if path.suffix.lower()!='.spr':continue
        data=path.read_bytes()
        if len(data)<24 or struct.unpack_from('<I',data,8)[0]!=4:continue
        count,palettes=struct.unpack_from('<II',data,12);table=24+palettes*768;base=table+count*4
        if count>65536 or palettes>256 or base>len(data):raise ValueError('Invalid installed SPR source table')
        keys=set()
        for i in range(count):
            at=base+struct.unpack_from('<I',data,table+i*4)[0]
            if at>len(data)-40:raise ValueError('Invalid installed SPR frame offset')
            size=struct.unpack_from('<I',data,at)[0]
            if size<40 or size>len(data)-at:raise ValueError('Invalid installed SPR frame extent')
            keys.add(helper.frame_key(data[at:at+size],palettes!=0))
        if keys&needed:selected[path.relative_to(game).as_posix()]=path;needed-=keys
    if needed:raise ValueError('Unmapped original-owned World source frames')
    return selected


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--capture-report',type=Path,required=True)
    p.add_argument('--executable',type=Path,required=True);p.add_argument('--report',type=Path,required=True)
    p.add_argument('--headless',action='store_true');p.add_argument('--claims',type=Path);a=p.parse_args()
    output=a.report.resolve()
    if output.exists() or not output.is_relative_to(ROOT) or any(output.is_relative_to((ROOT/x).resolve()) for x in ('original','.git')):p.error('Report must be new and inside the repository outside original/.git')
    subprocess.run([str(ROOT/'tools/original-manifest.sh'),'verify'],check=True)
    report={'success':False,'original_pixels_used_as_native_inputs':False,'original_oracles_read_by_native':False,'original_work_bypassed':False,'live_replacement':False}
    try:
        capture_path=a.capture_report.resolve();capture_bytes=capture_path.read_bytes();capture=json.loads(capture_bytes)
        if (capture['success'] is not True or capture['sources_stable'] is not True or capture['original_manifest_verified_before_after'] is not True or
            capture['source_executable_sha256']!=BUILD or capture['live_replacement'] is not False):raise ValueError('Invalid independent original capture')
        experiment=Path(capture['experiment']).resolve();directory=experiment/'capture';publication=analyze(directory)
        if publication!=capture['drawing_publication']:raise ValueError('Original publication artifacts changed')
        if publication['readback_drops'] or publication['producers']['failures'] or not publication['frames']:raise ValueError('Original publication observation is incomplete')
        raw=(directory/'canvas-producers.bin').read_bytes();_,records=load_tool('inspect-canvas-producers').decode(raw,complete=False)
        frames=publication['frames'];final=frames[-1]['source_sequence'];records=records[:final]
        if [f['source_sequence'] for f in frames]!=sorted(set(f['source_sequence'] for f in frames)):raise ValueError('Native publication boundaries are duplicated or reordered')
        assets=source_assets(experiment/'game',records);inputs={}
        for path in [capture_path,directory/'canvas-producers.bin',directory/'canvas-producers.done',directory/'drawing-publication-0000.bin',*assets.values(),*[directory/f['path'] for f in frames]]:
            inputs[path.relative_to(ROOT).as_posix()]=sha(path.read_bytes())
        sources={}
        for top in ('renderer','assets','compat/legacy','protocols/include','reconstruction/rendering'):
            for path in (ROOT/top).rglob('*'):
                if path.is_file() and (path.suffix in ('.c','.h','.cpp','.hpp','.S','.cmake') or path.name=='CMakeLists.txt'):sources[path.relative_to(ROOT).as_posix()]=sha(path.read_bytes())
        for name in ('tools/check-drawing-publication.py','tools/drawing_publication.py','tools/drawing_scene_corpus.py','tools/drawing_corpus.py','tools/inspect-canvas-producers.py','tools/retain-drawing-scene.py'):
            sources[name]=sha((ROOT/name).read_bytes())
        claims=json.loads(a.claims.read_text()) if a.claims else None
        if claims:
            for name,digest in claims['sources'].items():
                if sha((ROOT/name).read_bytes())!=digest:raise ValueError('Prospective source changed: '+name)
                sources[name]=digest
        binary=a.executable.resolve();binary_hash=sha(binary.read_bytes());run=ROOT/'working/tests/drawing-publication';run.mkdir(parents=True,exist_ok=True)
        root=Path(tempfile.mkdtemp(prefix='run-',dir=run));owned=root/'owned';owned.mkdir();game=owned/'assets';game.mkdir()
        for requested,path in assets.items():
            target=game/asset_name(requested).casefold();target.parent.mkdir(parents=True,exist_ok=True);target.write_bytes(path.read_bytes())
        boundary=[]
        for f in frames:
            completed=sum(r[2]==12 for r,_ in records[:f['source_sequence']])
            boundary.append(dict(sequence=f['source_sequence'],queues=completed,canvas=f['canvas'],width=f['width'],height=f['height']))
        prefix=raw[:64+sum(r[0] for r,_ in records)];(owned/'inputs.bin').write_bytes(raster_inputs(prefix))
        config={'schema':2,'boundary':boundary[-1],'frames':boundary,'assets':{name:sha(path.read_bytes()) for name,path in assets.items()},'retain_checkpoints':[]}
        (owned/'scene.json').write_text(json.dumps(config)+'\n')
        command=[str(binary),str(owned/'inputs.bin'),str(game),str(owned/'scene.json'),str(root/'native')];env=dict(os.environ)
        if a.headless:command=['xvfb-run','-a',*command];env.update(QT_QPA_PLATFORM='xcb',LIBGL_ALWAYS_SOFTWARE='1')
        native=subprocess.run(command,env=env,capture_output=True,timeout=600);(root/'native.log').write_bytes(native.stdout+native.stderr)
        if native.returncode:raise ValueError('Native publication refused; inspect '+str(root/'native.log'))
        result=json.loads((root/'native/scene-report.json').read_text())
        if not result['success'] or result['remaining_surfaces'] or result['original_pixels_used_as_native_inputs'] or result['original_oracles_read'] or result['frames_published']!=len(frames):raise ValueError('Incomplete native publication or oracle dependency')
        checks=[]
        for i,(f,b,n) in enumerate(zip(frames,boundary,result['frames']),1):
            if {k:n[k] for k in b}!=b or n['ordinal']!=i:raise ValueError('Native publication source identity changed')
            expected=(directory/f['path']).read_bytes();actual=(root/f'native/frame-{i:04}.565').read_bytes()
            if sha(actual)!=n['sha256']:raise ValueError('Native output hash mismatch')
            check=compare(actual,expected,f['width'],f['height']);checks.append(dict(f,**check))
        if capture_bytes!=capture_path.read_bytes() or sha(binary.read_bytes())!=binary_hash or any(sha((ROOT/name).read_bytes())!=digest for name,digest in {**sources,**inputs}.items()):raise ValueError('Source/input/native binary changed during comparison')
        report={'success':all(c['mismatching_pixels']==0 for c in checks),'sources':sources,'sources_stable':True,'inputs':inputs,'inputs_stable':True,
                'capture_report':capture_path.relative_to(ROOT).as_posix(),'capture_report_sha256':sha(capture_bytes),
                'source_executable_sha256':BUILD,'original_pixels_used_as_native_inputs':False,'original_oracles_read_by_native':False,
                'original_work_bypassed':False,'live_replacement':False,'boundary':'observed_primary_blt','all_primary_routes_observed':False,'actions_bound':False,
                'frames':checks,'pixels_compared':sum(c['pixels'] for c in checks),'mismatches':sum(c['mismatching_pixels'] for c in checks),
                'native_binary_sha256':binary_hash,'native':result,'asset_sources':{name:path.relative_to(ROOT).as_posix() for name,path in assets.items()},
                'scope':'Every primary readback at the guarded opaque-copy sites in this finite startup capture; alternate publication routes, action identities, API failure equivalence and complete journeys remain pending.'}
        if claims:report['claims']=claims['claims']
    except (ValueError,OSError,KeyError,TypeError,subprocess.SubprocessError) as error:
        report.update(error=str(error),sources=locals().get('sources',{}),inputs=locals().get('inputs',{}),sources_stable=False,inputs_stable=False)
        if locals().get('claims'):report['claims']=claims['claims']
    finally:subprocess.run([str(ROOT/'tools/original-manifest.sh'),'verify'],check=True)
    report['original_manifest_verified_before_after']=True;output.parent.mkdir(parents=True,exist_ok=True)
    with output.open('x') as f:json.dump(report,f,indent=2);f.write('\n')
    print(json.dumps({k:report[k] for k in ('success','pixels_compared','mismatches','error') if k in report}));return 0 if report['success'] else 1

if __name__=='__main__':raise SystemExit(main())
