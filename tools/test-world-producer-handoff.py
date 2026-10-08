#!/usr/bin/env python3
"""Freeze native sources, replay reconstructed World history and run original raster prefixes."""
import argparse, hashlib, importlib.util, json, os, shutil, struct, subprocess, tempfile
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
ORIGINAL_SHA='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
def sha(path): return hashlib.sha256(path.read_bytes()).hexdigest()
def source_paths():
    # Compiler dependencies of the standalone native target and its four tests.
    # Exclude unrelated parent legacy projects; keep every actual GPU dependency.
    paths=['apps/qt-shell/canvas_producer_main.cpp', 'apps/qt-shell/canvas_producer_session.cpp', 'apps/qt-shell/canvas_producer_session.hpp', 'apps/qt-shell/gl_viewport.cpp', 'apps/qt-shell/gl_viewport.hpp', 'assets/CMakeLists.txt', 'assets/animation.cpp', 'assets/animation.hpp', 'assets/animation_decode.cpp', 'assets/asset_file.cpp', 'assets/asset_file.hpp', 'assets/bmp.cpp', 'assets/bmp.hpp', 'assets/jpeg.cpp', 'assets/jpeg.hpp', 'assets/path_resolver.cpp', 'assets/path_resolver.hpp', 'assets/pcx.cpp', 'assets/pcx.hpp', 'assets/resources/CMakeLists.txt', 'assets/resources/resource_manager.cpp', 'assets/resources/resource_manager.hpp', 'assets/sprite_frame_decoder.hpp', 'assets/sprite_loader.cpp', 'assets/sprite_loader.hpp', 'assets/terrain_catalog.cpp', 'assets/terrain_catalog.hpp', 'compat/legacy/canvas-producers/CMakeLists.txt', 'compat/legacy/canvas_producers.cpp', 'compat/legacy/canvas_producers.hpp', 'compat/legacy/canvas_world.cpp', 'compat/legacy/canvas_world.hpp', 'compat/legacy/scene_snapshot.cpp', 'compat/legacy/scene_snapshot.hpp', 'compat/legacy/world_frame.cpp', 'compat/legacy/world_frame.hpp', 'compat/legacy/world_resources.cpp', 'compat/legacy/world_resources.hpp', 'protocols/include/mnm/canvas_producers_v1.h', 'protocols/include/mnm/render_stream_v3.h', 'protocols/include/mnm/scene_snapshot_v1.h', 'protocols/include/mnm/world_frame_v1.h', 'protocols/include/mnm/world_producer_bypass_v1.h', 'renderer/CMakeLists.txt', 'renderer/bitmap_dc.hpp', 'renderer/blit.cpp', 'renderer/blit.hpp', 'renderer/canvas_sequence.cpp', 'renderer/canvas_sequence.hpp', 'renderer/capture.cpp', 'renderer/capture.hpp', 'renderer/command_consumer.cpp', 'renderer/command_state.hpp', 'renderer/commands.cpp', 'renderer/commands.hpp', 'renderer/dib.cpp', 'renderer/dib.hpp', 'renderer/known_pixels.hpp', 'renderer/pixel_rows.cpp', 'renderer/pixel_rows.hpp', 'renderer/resources/CMakeLists.txt', 'renderer/resources/resource_cache.cpp', 'renderer/resources/resource_cache.hpp', 'renderer/resources/sprite_atlas.cpp', 'renderer/resources/sprite_atlas.hpp', 'renderer/scenes/CMakeLists.txt', 'renderer/scenes/scene_history.cpp', 'renderer/scenes/scene_history.hpp', 'renderer/scenes/scene_renderer.cpp', 'renderer/scenes/scene_renderer.hpp', 'renderer/sprites/CMakeLists.txt', 'renderer/sprites/sprite.cpp', 'renderer/sprites/sprite.hpp', 'renderer/surface_access.hpp', 'renderer/surface_backend.cpp', 'renderer/surface_backend.hpp', 'renderer/surface_copy.cpp', 'renderer/surface_copy.hpp', 'tests/canvas-sequence-test.cpp', 'tests/canvas-world-test.cpp', 'tests/resource-fixtures.hpp', 'tests/scene-history-test.cpp', 'tests/scene-renderer-test.cpp']
    paths += [p.relative_to(ROOT).as_posix() for p in (ROOT/'runtime/scene').glob('*.[chS]')]
    paths += ['protocols/include/mnm/'+n for n in ('canvas_producers_v1.h','world_producer_bypass_v1.h','scene_snapshot_v1.h','world_channel_v1.h','world_channel_v2.h','world_frame_v1.h')]
    paths += ['runtime/shadow/win32_min.h','tests/canvas-world-test.cpp','tests/canvas-sequence-test.cpp','tests/canvas-producers-test.c','tests/world-producer-bypass-call.S','tests/canvas-startup-call.S','tests/world-frame-reference.cpp','tests/sprite-binary-reference.cpp','tests/resource-fixtures.hpp','tests/scene-renderer-test.cpp','tests/scene-history-test.cpp','tools/test-world-producer-handoff.py','tools/test-canvas-producers.py','tools/build-scene-observer.py','tools/build-shadow-bridge.py','tools/prepare-scene-observer.py','tools/capture-scene-game.py','tools/scene-game-runner.py','tools/inspect-canvas-producers.py','tools/inspect-startup-queues.py','tools/world_channel.py']
    paths += ['tests/world-producer-reference.cpp','tests/world-raster-queue-reference.cpp','tests/world-raster-reference.hpp','tools/test-world-raster-queue.py','tools/inspect-world-raster-callers.py']
    return sorted(set(paths))
def world_record(r,p):
    width,height=struct.unpack_from('<II',p,4);ox,oy=struct.unpack_from('<ii',p,12)
    x,y=struct.unpack('<ii',struct.pack('<II',*r[8:10]));cl,ct,cr,cb=struct.unpack('<iiii',struct.pack('<4I',*r[10:14]));left,top=x-ox,y-oy
    if r[15] in (0,9) and (left<cl or left+width>=cr):return None
    cover=top+(height//2 if r[14]==5 else 0)
    if not width or not height or left>=cr or left+width<=cl or cover>=cb or cover+height<=ct:return None
    phase=0
    if r[14]==3 and r[15]==7 and (left<cl or left+width>cr):
        offsets=struct.unpack_from('<16I',p,r[19])
        phases=[q for q in range(16) if tuple(abs(8-((j+q)&15)) for j in range(16))==offsets]
        if len(phases)!=1:raise ValueError('Clipped wave phase cannot be uniquely resolved from owned offsets')
        phase=phases[0]
    h=[64+len(p),r[14],r[8],r[9],*r[10:14],r[19],r[17],r[20],0,phase,r[15],0,r[16] if r[14]==3 else 0]
    return struct.pack('<16I',*h)+p
def world_bytes(queue,width,height,records):
    raw=b''.join(records);h=[0]*20;h[:2]=struct.unpack('<2I',b'MNMWRLD1');h[2:9]=[1,80,80+len(raw),queue,width,height,width];h[9]=len(records);h[11]=0x40209ca7
    return struct.pack('<20I',*h)+raw
def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--capture-report',type=Path,required=True);parser.add_argument('--claims',type=Path);parser.add_argument('--complete-raster-queues',action='store_true');parser.add_argument('--prepare-raster-queue',type=int,choices=range(1,17));args=parser.parse_args()
    claims=json.loads(args.claims.read_text()) if args.claims else None
    sources={n:sha(ROOT/n) for n in source_paths()}
    if claims:
        for n,h in claims['sources'].items():
            if sha(ROOT/n)!=h:raise ValueError('Prospective source changed: '+n)
            sources[n]=h
    parent=ROOT/'working/tests/world-producer-handoff';parent.mkdir(parents=True,exist_ok=True);out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(out,flush=True)
    report={'success':False,'sources':sources,'scope':'Frozen source-only native producer/GPU World handoff with every completion externally compared. Private unmodified original raster replay uses only reconstructed native entry state and owned requests, including every selected bypass prefix.','original_pixels_used_as_native_inputs':False,'original_oracles_read_by_native':False,'live_replacement':False}
    if claims:report['claims']=claims['claims']
    inputs={}
    def pin(p):inputs[str(p.resolve().relative_to(ROOT))]=sha(p)
    def run(name,command,timeout=300):
        with (out/(name+'.log')).open('x') as log:subprocess.run([str(x) for x in command],cwd=ROOT,env={**os.environ,'QT_QPA_PLATFORM':'xcb','LIBGL_ALWAYS_SOFTWARE':'1'},stdout=log,stderr=subprocess.STDOUT,check=True,timeout=timeout)
    run('original-before',[ROOT/'tools/original-manifest.sh','verify'])
    try:
        capturePath=args.capture_report.resolve();pin(capturePath);capture=json.loads(capturePath.read_text());experiment=Path(capture['experiment']);directory=experiment/'capture';report['source_executable_sha256']=capture['source_executable_sha256'];assert report['source_executable_sha256']==ORIGINAL_SHA
        spec=importlib.util.spec_from_file_location('producer_inspector',ROOT/'tools/inspect-canvas-producers.py');inspector=importlib.util.module_from_spec(spec);spec.loader.exec_module(inspector)
        analysis=inspector.analyze(directory);assert analysis==capture['canvas_producers'];stream=directory/'canvas-producers.bin';pin(stream);pin(directory/'canvas-producers.done');header,operations=inspector.decode(stream.read_bytes())
        frozen=out/'source';names=subprocess.check_output(['git','ls-files','--cached','--others','--exclude-standard','-z'],cwd=ROOT).decode().split('\0')
        for n in sorted(set(names)|set(sources)):
            p=ROOT/n
            if not n or n.split('/')[0] in ('working','original','.git') or not p.is_file():continue
            if n not in sources and p.suffix not in ('.cpp','.hpp','.c','.h','.S','.py','.cmake') and p.name not in ('CMakeLists.txt','README.md'):continue
            target=frozen/n;target.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(p,target)
        assert all(sha(frozen/n)==h for n,h in sources.items())
        nativeInputs=out/'native-inputs';nativeInputs.mkdir();shutil.copyfile(stream,nativeInputs/stream.name);shutil.copyfile(directory/'canvas-producers.done',nativeInputs/'canvas-producers.done')
        prepared={}
        if args.prepare_raster_queue:
            raw=bytearray(stream.read_bytes());at=64;active=0;ordinal=0
            entries={0:0x595677,1:0x59521a,2:0x59603e,3:0x57de00,4:0x57ec90,5:0x57f0f0,6:0x57f5f0,7:0x5806f0,9:0x5968a4,10:0x57e540}
            for r,payload in operations:
                if r[2]==11:active=r[14]
                elif r[2]==12:active=0
                elif active==args.prepare_raster_queue and r[2]==9:
                    ordinal+=1;base=f'world-raster-{ordinal:06}';width,height=struct.unpack_from('<II',payload,4);origin=struct.unpack_from('<i',payload,12)[0];x=struct.unpack('<i',struct.pack('<I',r[8]))[0];cl,cr=[struct.unpack('<i',struct.pack('<I',r[k]))[0] for k in (10,12)];ax=int(r[15] in (0,9) and width and height and (x-origin<cl or x-origin+width>=cr))
                    struct.pack_into('<I',raw,at+84,ax)
                    h=[0]*16;h[:2]=struct.unpack('<2I',b'MNMWBP02');h[2:11]=[2,64,ordinal,active,r[1],r[3],r[5],r[6],r[5]];h[14]=entries[r[15]];h[15]=ax;(nativeInputs/(base+'.request')).write_bytes(struct.pack('<16I',*h));prepared[r[1]]={'ordinal':ordinal,'sequence':r[1],'queue':active,'original_entry':entries[r[15]],'return_ax':ax,'wire_base':base,'native_before':base+'.before.565'}
                at+=r[0]
            (nativeInputs/stream.name).write_bytes(raw)
        nativeAssets=out/'native-assets';nativeAssets.mkdir();assets=[]
        encoded={p[:-1].decode('ascii').replace('\\','/').casefold() for r,p in operations if r[2] in (7,20)}
        for p in (experiment/'game').rglob('*'):
            if not p.is_file():continue
            relative=p.relative_to(experiment/'game')
            if p.suffix.lower()!='.spr' and relative.as_posix().casefold() not in encoded:continue
            pin(p);target=nativeAssets/relative;target.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(p,target);assets.append({'path':relative.as_posix(),'sha256':sha(p)})
        build=out/'build';run('configure',['cmake','-S',frozen/'compat/legacy/canvas-producers','-B',build,'-DCMAKE_BUILD_TYPE=Debug']);run('build',['cmake','--build',build,'--target','mnm-canvas-producers-live','canvas-world-test','canvas-sequence-test','scene-renderer-test','scene-history-test','-j4'])
        run('native-tests',['ctest','--test-dir',build,'-R','native-canvas-world-handoff|native-canvas-sequence|native-shared-scene-renderer|native-scene-history','--output-on-failure'])
        native=out/'native';run('native-replay',['xvfb-run','-a','-s','-screen 0 1280x1024x24',build/'mnm-canvas-producers-live',nativeInputs/stream.name,nativeAssets,native,'--world-handoff'])
        viewer=json.loads((native/'live-report.json').read_text());assert viewer['success'] and viewer['world_queues']==analysis['queues'] and not viewer['remaining_surfaces']
        comparisons=[]
        for original,completed in zip(analysis['checkpoints'],viewer['checkpoints'],strict=True):
            assert (original['sequence'],original['canvas'],original['oracle'])==(completed['sequence'],completed['canvas'],completed['oracle'])
            old=directory/original['path'];pin(old);a=old.read_bytes();b=(native/completed['path']).read_bytes();assert len(a)==len(b)
            comparisons.append({'oracle':original['oracle'],'pixels':len(a)//2,'equal':a==b})
        reference=out/'world-reference';run('reference-build',['g++','-m32','-std=c++17','-O2','-Wall','-Wextra','-Werror','-no-pie',frozen/'tests/world-frame-reference.cpp','-o',reference]);pe=ROOT/'original/Arcane_Nocd/Chaos.exe';pin(pe);assert sha(pe)==ORIGINAL_SHA
        exact=out/'generic-reference';run('generic-reference-build',['g++','-m32','-std=c++17','-O2','-Wall','-Wextra','-Werror','-no-pie',frozen/('tests/world-raster-queue-reference.cpp' if args.complete_raster_queues or args.prepare_raster_queue else 'tests/world-producer-reference.cpp'),'-o',exact])
        active=0;requests=[];entries={f['queue']:native/f['entry'] for f in viewer['world_frames']};originalChecks=[];bypassChecks=[]
        selected=prepared or {row['sequence']:row for row in capture.get('native_producers',{}).get('native_bypass_replies',[])}
        full=bool(args.complete_raster_queues or args.prepare_raster_queue)
        if prepared:assert len(viewer['native_bypass_replies'])==len(prepared)
        preciseRows=[]
        def originalReplay(label):
            request=out/(label+'.bin');request.write_bytes(world_bytes(active,width,height,requests));dest=out/(label+'.original.565');run(label,[reference,pe,request,dest,entries[active]]);return dest.read_bytes()
        for r,p in operations:
            if r[2]==11:active=r[14];width,height=r[5:7];requests=[]
            elif active and r[2]==9:
                request=world_record(r,p)
                if request is not None:requests.append(request)
                if r[1] in selected and full:
                    row=selected[r[1]];ownedBefore=(native if prepared else experiment/'native-producers')/row['native_before'];reply=(nativeInputs if prepared else directory)/(row['wire_base']+'.reply');pin(ownedBefore);pin(reply);preciseRows.append((r[1],row['original_entry'],row['return_ax'],ownedBefore,reply))
                elif r[1] in selected:
                    row=selected[r[1]];reply=directory/(row.get('wire_base',f"world-bypass-{row['ordinal']:04}")+'.reply');pin(reply);raw=reply.read_bytes();expected=originalReplay(f"bypass-{row['ordinal']:04}");actual=raw[64:];assert len(actual)==len(expected)
                    ownedBefore=experiment/'native-producers'/row['native_before'];pin(ownedBefore);encoded=out/f"bypass-{row['ordinal']:04}.producer.bin";encoded.write_bytes(struct.pack('<24I',*r)+p);precise=out/f"bypass-{row['ordinal']:04}.precise-original.565"
                    run(f"generic-{row['ordinal']:04}",[exact,pe,encoded,ownedBefore,precise,hex(row['original_entry'])]);preciseBytes=precise.read_bytes();assert len(preciseBytes)==len(actual)
                    bypassChecks.append({'sequence':r[1],'queue':active,'ordinal':row['ordinal'],'original_entry':hex(row['original_entry']),'pixels':len(actual)//2,'equal':actual==expected and actual==preciseBytes,'prefix_equal':actual==expected,'exact_entry_equal':actual==preciseBytes,'original_sha256':hashlib.sha256(expected).hexdigest(),'exact_entry_sha256':sha(precise),'native_sha256':hashlib.sha256(actual).hexdigest()})
            elif active and r[2]==10 and r[3]==viewer['world_frames'][active-1]['canvas']:
                expected=originalReplay(f'world-{active:04}');actual=(native/f'native-{r[14]:04}.565').read_bytes();assert len(expected)==len(actual)
                originalChecks.append({'queue':active,'pixels':len(actual)//2,'equal':actual==expected,'original_sha256':hashlib.sha256(expected).hexdigest(),'native_sha256':hashlib.sha256(actual).hexdigest()})
            elif r[2]==12:active=0
        if full:
            prior=None;priorQueue=0
            for seq,entry,ax,before,reply in preciseRows:
                queue=selected[seq]['queue']
                expectedBefore=prior if queue==priorQueue else entries[queue].read_bytes()
                assert before.read_bytes()==expectedBefore,('Native intermediate continuity',seq)
                prior=reply.read_bytes()[64:];priorQueue=queue
            report['contiguous_native_before_equal']=True
            inputList=out/'precise-raster-inputs.txt';outputList=out/'precise-raster-results.tsv'
            inputList.write_text(''.join(f'{seq} {entry} {ax} {before} {reply.name}\n' for seq,entry,ax,before,reply in preciseRows))
            run('all-precise-rasters',[exact,pe,nativeInputs/stream.name,inputList,outputList,nativeInputs if prepared else directory],timeout=3600)
            resultRows=[line.split('\t') for line in outputList.read_text().splitlines()];assert len(resultRows)==len(selected)
            bypassChecks=[{'sequence':int(seq),'original_entry':hex(int(entry)),'return_ax':int(ax),'pixels':int(pixels),'equal':okay=='1','exact_entry_equal':okay=='1'} for seq,entry,ax,pixels,okay in resultRows]
            # Each selected queue is a complete contiguous raster prefix. The
            # exact before/after comparisons inductively check every intermediate.
            selectedQueues=set(row['queue'] for row in selected.values());active=0;expected=[]
            for r,_ in operations:
                if r[2]==11:active=r[14]
                elif r[2]==12:active=0
                elif active in selectedQueues and r[2]==9:expected.append(r[1])
            assert expected==list(selected)
            report.update(complete_raster_queues=sorted(selectedQueues),canonical_entry_preflight=bool(prepared),caller_ax_equal=len(bypassChecks))
        assert len(originalChecks)==analysis['queues'] and len(bypassChecks)==len(selected)
        report.update(native=viewer,asset_sources=assets,checkpoints=comparisons,completed=len(comparisons),equal=sum(c['equal'] for c in comparisons),pixels_compared=sum(c['pixels'] for c in comparisons),original_world_checks=originalChecks,original_world_equal=sum(c['equal'] for c in originalChecks),original_world_pixels_compared=sum(c['pixels'] for c in originalChecks),bypass_checks=bypassChecks,bypass_equal=sum(c['equal'] for c in bypassChecks),bypass_pixels_compared=sum(c['pixels'] for c in bypassChecks),native_binary_sha256=sha(build/'mnm-canvas-producers-live'))
        report['success']=all(c['equal'] for c in comparisons+originalChecks+bypassChecks)
    except Exception as error:report['error']=str(error)
    finally:
        run('original-after',[ROOT/'tools/original-manifest.sh','verify']);report.update(original_manifest_verified_before_after=True,inputs=inputs,inputs_stable=all(sha(ROOT/n)==h for n,h in inputs.items()),sources_stable=all(sha(ROOT/n)==h for n,h in sources.items()));report['success']=report['success'] and report['inputs_stable'] and report['sources_stable'];(out/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(out/'report.json',flush=True)
    if not report['success']:raise SystemExit(1)
if __name__=='__main__':main()
