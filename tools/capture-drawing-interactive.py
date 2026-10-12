#!/usr/bin/env python3
"""Acquire a bounded original-active interactive Quick Battle API reference."""
import argparse,importlib.util,json,mmap,os,shutil,signal,struct,subprocess,tempfile,time
from pathlib import Path
from drawing_api_session import BUILD,inspect,sha
from campaign_gameplay import read_rows,creatures,signed
ROOT=Path(__file__).resolve().parents[1]
def load(path):
    s=importlib.util.spec_from_file_location(Path(path).stem,ROOT/path);m=importlib.util.module_from_spec(s);s.loader.exec_module(m);return m

def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--source-game',type=Path,required=True);p.add_argument('--prefix-template',type=Path,required=True);p.add_argument('--schedule',type=Path,default=ROOT/'tests/fixtures/drawing-journeys/interactive-schedule.json');p.add_argument('--report',type=Path,required=True);p.add_argument('--claims',type=Path);p.add_argument('--observer-mode',choices=['full','metadata','inputs','physical'],default='metadata');p.add_argument('--physical-peer',type=Path,help='Reference-only X11 peer; required for physical mode');p.add_argument('--physical-oracle-mib',type=int,choices=range(1,3073),default=1024);p.add_argument('--physical-transient-mib',type=int,choices=range(1,2049),default=256);p.add_argument('--producer-samples',type=int,choices=[*range(1,17),32,120],help='Bind original-active producer metadata through the first1..16or32World returns');p.add_argument('--egl-physical',action='store_true',help='Opt-in separate post-EGL physical viewport witnesses');p.add_argument('--x11-delivery-library',type=Path,help='Optional64-bit Unix metadata observer; no complete display-route claim');p.add_argument('--canvas-storage-transitions',action='store_true',help='Retain every observed wrapper create/release/Lock entry and return');p.add_argument('--whole-updates',action='store_true',help='Retain source postlude through following accepted update');p.add_argument('--producer-update-binding',action='store_true',help='Bind source queue entries/returns to exact observed engine updates');p.add_argument('--engine-cadence',action='store_true',help='Retain actual original update identities, flush on normal original Quit');p.add_argument('--source-held-snapshot',action='store_true',help='Copy external source oracles while diagnostic worker lease holds protection');p.add_argument('--source-worker-lease',action='store_true',help='Retain diagnostic source lease until exact emitted worker returns');p.add_argument('--source-readonly-lease',action='store_true',help='Diagnosticread-onlysourcepagesduringAPIbodyonly');p.add_argument('--source-byte-witness',action='store_true',help='Retain originalCPUsourceoraclesatAPIentry/return');p.add_argument('--wine-worker-context',action='store_true',help='Sample portable activeworkercontextaroundEGLswaps');p.add_argument('--wine-blit-packet',action='store_true',help='Bind selected emitted command objects to worker IDs');p.add_argument('--wine-blit-worker',action='store_true',help='Observe pinned opcode42Wineworker metadata');p.add_argument('--wine-texture-entry',action='store_true',help='Observe pinned internal Wine texture entry lifetimes');p.add_argument('--wine-retained-map-source',action='store_true',help='Query a last observed map address at exact source submission without claiming post-unmap ownership');p.add_argument('--wine-client-memory',action='store_true',help='Observe public client-memory binding and exact submitted source cache point');p.add_argument('--wine-map-storage-point',action='store_true',help='Associate successful public map buffers with known live recorder canvas storage');p.add_argument('--wine-resource-map',action='store_true',help='Observe pinned public Wine resource map/unmap metadata');p.add_argument('--wine-submission',action='store_true',help='Observe pinned Wine11.16DirectDraw submission metadata');p.add_argument('--cadence-control',action='store_true',help='Explicit counter/menu/input-only control; no drawing/source/Wine/display observer');p.add_argument('--control',action='store_true',help='Run the same original menus/inputs with the API observer disabled');a=p.parse_args()
    from drawing_diagnostic_cadence import validate_control
    try:validate_control(a)
    except ValueError as error:p.error(str(error))
    if (a.observer_mode=='physical') != (a.physical_peer is not None):p.error('Physical mode requires exactly one physical peer')
    if a.canvas_storage_transitions and not a.producer_samples:p.error('Storage transitions require producer capture')
    if a.producer_samples and a.observer_mode!='physical':p.error('Producer binding requires physical mode')
    if a.producer_samples==120 and not a.whole_updates:p.error('120source samples require explicit whole-update capture')
    if a.whole_updates and not a.producer_update_binding:p.error('Whole updates require exact producer update binding')
    if a.producer_update_binding and (not a.engine_cadence or not a.producer_samples):p.error('Producer update binding requires cadence and source producer capture')
    if a.engine_cadence and not a.cadence_control and (a.control or a.observer_mode!='physical'):p.error('Cadence observation requires original-active physical acquisition')
    if a.source_held_snapshot and not a.source_worker_lease:p.error('Protected snapshots require worker lease')
    if a.source_worker_lease and (not a.source_readonly_lease or not a.wine_blit_packet):p.error('Worker lease requires read-only lease and exact packet binding')
    if a.source_readonly_lease and not a.source_byte_witness:p.error('Sourcelease requires sourcebytewitness')
    if a.source_byte_witness and not a.producer_samples:p.error('Sourcebytewitness requires producerbinding')
    if a.wine_worker_context and (not a.wine_blit_packet or not a.x11_delivery_library):p.error('Workercontext requires packetbinding/EGLobserver')
    if a.wine_blit_packet and not a.wine_blit_worker:p.error('Packetbinding requires workerobservation')
    if a.wine_blit_worker and not a.wine_texture_entry:p.error('Worker observation requires internaltextureentry mode')
    if a.wine_texture_entry and not a.wine_submission:p.error('Textureentry requires Wine submission mode')
    if a.wine_retained_map_source and (not a.wine_resource_map or not a.producer_samples):p.error('Retained source maps require resource observation and source producers')
    if a.wine_client_memory and (not a.wine_submission or not a.producer_samples):p.error('Client memory observation requires Wine submission and source producer observation')
    if a.wine_map_storage_point and (not a.wine_resource_map or not a.producer_samples):p.error('Map storage points require resource maps and source producer observation')
    if a.wine_resource_map and not a.wine_submission:p.error('Resource map requires Wine submission observation')
    if a.wine_submission and (a.observer_mode!='physical' or a.control):p.error('Wine submission requires original-active physical mode')
    if a.control and a.physical_peer:p.error('Physical API capture cannot be a control run')
    if a.observer_mode!='physical' and (a.physical_oracle_mib!=1024 or a.physical_transient_mib!=256):p.error('Physical budgets require physical mode')
    if a.egl_physical and (a.observer_mode!='physical' or not a.x11_delivery_library):p.error('EGL physical witnesses require physical/API mode and an EGL delivery library')
    if a.report.exists():p.error('Refusing evidence overwrite')
    names=['tools/capture-drawing-interactive.py','tools/drawing-session-runner.py','tools/drawing_call_intervals.py','tools/drawing_api_session.py','tools/build-drawing-session.py','tools/build-drawing-session-input.py','tools/build-menu-observer.py','tools/build-shadow-bridge.py','tools/prepare-shadow-experiment.py','tools/prepare-scene-observer.py','tools/run-game.sh','tools/run-opengl-game.py','tools/campaign_gameplay.py','runtime/drawing/session.c','runtime/drawing/source_points.h','protocols/include/mnm/drawing_source_point_v1.h','runtime/drawing/call_intervals.h','runtime/drawing/physical_fence.h','tools/drawing_api_physical.py','apps/drawing-session/observe.cpp','apps/drawing-session/CMakeLists.txt','runtime/shadow/win32_min.h','tests/drawing-session-input.cpp','apps/qt-shell/window_host.cpp','apps/qt-shell/window_host.hpp']
    names += [str(x.relative_to(ROOT)) for x in (ROOT/'runtime/menu').glob('*') if x.suffix in ('.h','.c','.S')]
    names += [str(x.relative_to(ROOT)) for x in (ROOT/'protocols/include/mnm').glob('menu_v*.h')]
    if a.x11_delivery_library:names += ['runtime/display/x11_delivery.c','tools/build-drawing-x11-delivery.py','tools/drawing_x11_delivery.py','tools/drawing_process_snapshot.py','runtime/display/egl_physical.h','tools/drawing_egl_physical.py']
    if a.producer_samples:
        names += ['reconstruction/rendering/vertical_bar.h','tools/build-scene-observer.py','tools/drawing_producer_points.py','tools/inspect-canvas-producers.py']
        names += [str(x.relative_to(ROOT)) for x in (ROOT/'runtime/scene').glob('*.[chS]')]
        names += [str(x.relative_to(ROOT)) for x in (ROOT/'protocols/include/mnm').glob('*producer*.h')]
        names += [str(x.relative_to(ROOT)) for x in (ROOT/'protocols/include/mnm').glob('world_*.h')]
        names += [str(x.relative_to(ROOT)) for x in (ROOT/'reconstruction/rendering').glob('glyph_backend*') if x.suffix in ('.c','.h','.S')]
    if a.whole_updates:names += ['runtime/engine-cadence/update_boundary.h','runtime/scene/whole_update_capture.h','protocols/include/mnm/canvas_producers_v4.h','runtime/scene/sample_admission.h','tools/drawing_source_completion.py']
    if a.producer_update_binding:names += ['runtime/engine-cadence/queue_point.h','protocols/include/mnm/engine_cadence_queue_point_v1.h','runtime/scene/drawing_update_points.h','tools/drawing_producer_updates.py']
    if a.cadence_control:names += ['tools/drawing_diagnostic_cadence.py']
    if a.engine_cadence:names += ['runtime/engine-cadence/observer.c','runtime/engine-cadence/entry.S','tools/build-engine-cadence.py','tools/engine_cadence.py','tools/drawing_engine_windows.py','tools/drawing_quit_completion.py']
    if a.wine_map_storage_point or a.wine_client_memory or a.wine_retained_map_source:names += ['runtime/drawing/storage_point_exports.h','tools/drawing_storage_point.py']
    if a.canvas_storage_transitions:names += ['runtime/scene/drawing_storage_transitions.h','tools/drawing_storage_transitions.py','protocols/include/mnm/drawing_storage_point_v2.h']
    if a.source_held_snapshot:names += ['runtime/drawing/source_held_snapshot.h','tools/drawing_source_held_snapshot.py','tools/drawing_source_held_refusal.py']
    if a.source_worker_lease:names += ['runtime/drawing/source_worker_lease.h','tools/drawing_source_worker_lease.py']
    if a.source_readonly_lease:names += ['runtime/scene/drawing_source_lease.h','runtime/scene/source_pending_refusal.h','tools/drawing_source_pending_refusal.py','runtime/drawing/source_readonly_lease.h','tools/drawing_source_lease.py']
    if a.source_byte_witness:names += ['runtime/scene/drawing_source_bytes.h','runtime/drawing/source_byte_witness.h','tools/drawing_source_byte_witness.py']
    if a.wine_worker_context:names += ['runtime/drawing/wine_worker_context.h','runtime/display/egl_worker_context.h','protocols/include/mnm/drawing_worker_context_v1.h','tools/drawing_worker_context.py']
    if a.wine_blit_packet:names += ['runtime/drawing/wine_blit_packet.h','runtime/drawing/wine_blit_packet.S','tools/drawing_wine_blit_packet.py']
    if a.wine_blit_worker:names += ['runtime/drawing/wine_blit_worker.h','tools/drawing_wine_blit_worker.py','tools/inspect-drawing-wine-blit-command.py']
    if a.wine_texture_entry:names += ['runtime/drawing/wine_texture_entry.h']
    if a.wine_retained_map_source:names += ['runtime/drawing/wine_retained_map_source.h','tools/drawing_wine_retained_map_source.py','runtime/scene/drawing_storage_point.h','protocols/include/mnm/drawing_storage_point_v1.h']
    if a.wine_client_memory:names += ['runtime/drawing/wine_client_memory.h','tools/drawing_wine_client_memory.py','tools/inspect-drawing-wine-client-memory.py','runtime/scene/drawing_storage_point.h','protocols/include/mnm/drawing_storage_point_v1.h']
    if a.wine_map_storage_point:names += ['runtime/drawing/wine_map_storage_point.h','runtime/scene/drawing_storage_point.h','protocols/include/mnm/drawing_storage_point_v1.h','tools/drawing_wine_map_storage_point.py']
    if a.wine_resource_map:names += ['runtime/drawing/wine_resource_mapping.h','tools/drawing_wine_resource_mapping.py','tools/inspect-drawing-wine-resource-imports.py']
    if a.wine_submission:names += ['runtime/drawing/wine_submission.h','runtime/drawing/wine_lifetime.h','runtime/drawing/wine_buffer.h','tools/drawing_wine_buffer.py','tools/inspect-drawing-wine-imports.py','tools/drawing_wine_submission.py','tools/drawing_wine_lifetime.py']
    sources={n:sha((ROOT/n).read_bytes()) for n in names};sources[str(a.schedule.resolve().relative_to(ROOT))]=sha(a.schedule.read_bytes())
    claims=json.loads(a.claims.read_text()) if a.claims else None
    if claims:sources.update(claims['sources'])
    if any(sha((ROOT/n).read_bytes())!=h for n,h in sources.items()):raise ValueError('Prospective sources changed')
    source=a.source_game.resolve();original=(source/'Chaos.exe').read_bytes()
    if sha(original)!=BUILD:raise ValueError('Unsupported original executable; no staging writes')
    parent=ROOT/'working/experiments/drawing-api-session';parent.mkdir(parents=True,exist_ok=True);run=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(run,flush=True)
    game=run/'game';capture=run/'capture';capture.mkdir();x11_library=None;wine_submission=None
    if a.wine_client_memory:load('tools/inspect-drawing-wine-client-memory.py').inspect(Path('/usr/lib/wine/i386-windows/ddraw.dll').read_bytes(),Path('/usr/lib/wine/i386-windows/wined3d.dll').read_bytes())
    if a.wine_resource_map:load('tools/inspect-drawing-wine-resource-imports.py').inspect(Path('/usr/lib/wine/i386-windows/ddraw.dll').read_bytes(),Path('/usr/lib/wine/i386-windows/wined3d.dll').read_bytes())
    if a.wine_submission:
        pin=load('tools/inspect-drawing-wine-imports.py');wine_submission=pin.inspect(Path('/usr/lib/wine/i386-windows/ddraw.dll').read_bytes(),Path('/usr/lib/wine/i386-windows/wined3d.dll').read_bytes())
    if a.x11_delivery_library:
        library=a.x11_delivery_library.read_bytes()
        if library[:5]!=b'\x7fELF\x02' or struct.unpack_from('<H',library,18)[0]!=62:raise ValueError('Expected64-bit x86 Unix delivery library')
        if env_preload:=os.environ.get('LD_PRELOAD'):raise ValueError('Unpinned existing Unix preload: '+env_preload)
        target=run/'libmnm-drawing-x11-delivery.so';target.write_bytes(library);x11_library=dict(path=str(target),sha256=sha(library))
    process=input_process=physical_peer=egl_peer=None;channel=None;failure=None
    env={k:v for k,v in os.environ.items() if not k.startswith('MNM_')};env.update(WINEPREFIX=str(run/'wineprefix'),WINEDEBUG='-all',MNM_DRAW_SESSION_EXPERIMENT=str(run),QT_QPA_PLATFORM='xcb');env.pop('WAYLAND_DISPLAY',None)
    if a.observer_mode=='physical':env['MNM_DRAW_PHYSICAL_MIB']=str(a.physical_oracle_mib)
    def verify(label):
        out=subprocess.run([str(ROOT/'tools/original-manifest.sh'),'verify'],capture_output=True,text=True,timeout=120);(run/(label+'.log')).write_text(out.stdout+out.stderr);out.check_returncode()
    verify('original-before')
    try:
        load('tools/prepare-scene-observer.py').copy_installation(source,game)
        drawing=None if a.control else load('tools/build-drawing-session.py').build();menu=load('tools/build-menu-observer.py').build();input_binary=load('tools/build-drawing-session-input.py').build();scene=load('tools/build-scene-observer.py').build() if a.producer_samples else None
        cadence=load('tools/build-engine-cadence.py').build(ROOT/'working/build/engine-cadence') if a.engine_cadence else None
        importer=load('tools/prepare-shadow-experiment.py');patched=original if a.control else importer.add_import(original,'MnmDrawing.dll','DrawingAnchor',b'.mndraw');patched=importer.add_import(patched,'MnmMenu.dll','MenuAnchor',b'.mnmenu')
        if scene:patched=importer.add_import(patched,'MnmScene.dll','SceneDrawingSourcePairV1',b'.mnscene')
        if cadence:patched=importer.add_import(patched,'MnmEngineCadence.dll','EngineCadenceAnchor',b'.mncad')
        pe=struct.unpack_from('<I',original,60)[0];count=struct.unpack_from('<H',original,pe+6)[0];table=pe+24+struct.unpack_from('<H',original,pe+20)[0]
        for i in range(count):
            size,at=struct.unpack_from('<II',original,table+40*i+16)
            if original[at:at+size]!=patched[at:at+size]:raise ValueError('Original section changed in import staging')
        (game/'Chaos.exe').write_bytes(patched)
        binaries=([menu] if a.control else [drawing,menu])+([scene] if scene else [])+([cadence] if cadence else [])
        for binary in binaries:shutil.copy2(binary,game/binary.name)
        settings=load('tools/run-opengl-game.py');preferences=settings.disable_cd_music(game)+settings.skip_movies(game)
        manifest={'canvas_storage_transitions':a.canvas_storage_transitions,'cadence_control':a.cadence_control,'wine_retained_map_source':a.wine_retained_map_source,'wine_client_memory':a.wine_client_memory,'wine_map_storage_point':a.wine_map_storage_point,'wine_resource_map':a.wine_resource_map,'whole_updates':a.whole_updates,'producer_update_binding':a.producer_update_binding,'engine_cadence':a.engine_cadence,'source_held_snapshot':a.source_held_snapshot,'source_worker_lease':a.source_worker_lease,'source_readonly_lease':a.source_readonly_lease,'source_byte_witness':a.source_byte_witness,'wine_worker_context':a.wine_worker_context,'wine_blit_packet':a.wine_blit_packet,'wine_blit_worker':a.wine_blit_worker,'wine_texture_entry':a.wine_texture_entry,'wine_submission':wine_submission,'egl_physical':a.egl_physical,'x11_delivery_library':x11_library,'schema':1,'source_sha256':BUILD,'binaries':{n:sha((game/n).read_bytes()) for n in ['Chaos.exe']+[b.name for b in binaries]},'producer_samples':a.producer_samples,'original_sections_unchanged':True,'preferences':preferences,'api_observer_enabled':not a.control,'observer_mode':a.observer_mode,'scope':'Original active stock Quick Battle; API/readback observation and XTest. No original simulation/raster body suppression.'};(run/'manifest.json').write_text(json.dumps(manifest,indent=2)+'\n')
        subprocess.run(['cp','-a','--reflink=auto',str(a.prefix_template.resolve()),env['WINEPREFIX']],check=True)
        with (run/'wineboot.log').open('x') as log:subprocess.run(['wineboot','-u'],env=env,stdout=log,stderr=subprocess.STDOUT,check=True,timeout=90)
        raw=bytearray(106496);raw[:8]=b'MNMMCM12';struct.pack_into('<II',raw,8,12,len(raw));struct.pack_into('<III',raw,16,2,1,1);(run/'channel.bin').write_bytes(raw)
        channel_file=(run/'channel.bin').open('r+b');channel=mmap.mmap(channel_file.fileno(),0);heartbeat=1;sequence=2;request=0;menu_actions=[]
        def state():
            before=struct.unpack_from('<I',channel,128)[0];row=struct.unpack_from('<7I',channel,132);payload=bytes(channel[160:16924]);slots=struct.unpack_from('<63I',channel,18028)
            return (row,payload,slots) if not before&1 and before==struct.unpack_from('<I',channel,128)[0] else None
        def refresh(action=None,generation=0,argument=0,rules=None,slots=None):
            nonlocal heartbeat,sequence,request
            heartbeat+=1;sequence+=2;struct.pack_into('<I',channel,16,sequence-1);struct.pack_into('<II',channel,20,1,heartbeat)
            if action is not None:
                request+=1;struct.pack_into('<4I',channel,28,request,action,generation,argument)
                if rules is not None:struct.pack_into('<17I',channel,44,*rules)
                if slots is not None:struct.pack_into('<63I',channel,40000,*slots)
            struct.pack_into('<I',channel,16,sequence)
        def wait(screen=None,ack=None,timeout=60):
            deadline=time.monotonic()+timeout
            while time.monotonic()<deadline:
                s=state()
                if s and (screen is None or s[0][1]==screen and s[0][2]) and (ack is None or s[0][3]==ack):return s
                if process.poll() is not None:raise ValueError('Original exited before required state')
                refresh();time.sleep(.02)
            raise ValueError('Original menu state timed out')
        def send(screen,action,argument=0,rules=None,slots=None):
            for attempt in range(8):
                s=wait(screen);refresh(action,s[0][0],argument,rules,slots);reply=wait(ack=request)
                menu_actions.append({'request':request,'screen':screen,'action':action,'argument':argument,'status':reply[0][4],'attempt':attempt+1})
                if reply[0][4]==1:return
                if reply[0][4]!=2:raise ValueError('Original menu action refused: '+str(reply[0]))
            raise ValueError('Original menu generation remained stale after eight retained attempts')
        if a.physical_peer:
            with (run/'physical-peer.log').open('x') as log:physical_peer=subprocess.Popen([str(a.physical_peer.resolve()),str(capture),str(run/'physical-report.json'),'--api','--transient-mib',str(a.physical_transient_mib)],env=env,stdout=log,stderr=subprocess.STDOUT)
        if a.egl_physical:
            with (run/'egl-peer.log').open('x') as log:egl_peer=subprocess.Popen([str(a.physical_peer.resolve()),str(capture),str(run/'egl-physical-report.json'),'--egl','--transient-mib',str(a.physical_transient_mib)],env=env,stdout=log,stderr=subprocess.STDOUT)
        with (run/'game.log').open('x') as log:
            process=subprocess.Popen([str(ROOT/'tools/run-game.sh'),'launch','--no-gamescope','--prefix',env['WINEPREFIX'],'--runner',str(ROOT/'tools/drawing-session-runner.py')],env=env,stdout=log,stderr=subprocess.STDOUT,start_new_session=True)
            wait(3)
            if x11_library:
                from drawing_process_snapshot import snapshot_processes
                (run/'process-main.json').write_text(json.dumps(snapshot_processes(run),indent=2)+'\n')
            send(3,1);send(22,4);s=wait(14);rules=list(struct.unpack_from('<13I',s[1],8))+[struct.unpack_from('<I',s[1],60+i*48+12)[0] for i in range(4)];rules[2]=3
            send(14,6,rules=rules);send(25,9,2);send(14,7,rules=rules);send(7,12,slots=wait(7)[2])
            if x11_library:(run/'process-world.json').write_text(json.dumps(snapshot_processes(run),indent=2)+'\n')
            with (run/'input.log').open('x') as ilog:input_process=subprocess.Popen([str(input_binary),str(capture),str(a.schedule.resolve()),str(run/'input.json')],env=env,stdout=ilog,stderr=subprocess.STDOUT)
            deadline=time.monotonic()+120
            while input_process.poll() is None and time.monotonic()<deadline:
                if process.poll() is not None:raise ValueError('Original exited during input schedule')
                refresh();time.sleep(.05)
            if input_process.poll() is None:raise ValueError('Input schedule timed out')
            if input_process.returncode:raise ValueError('Input fixture refused; inspect input.json/log')
            source_completion=None
            if a.whole_updates:
                from drawing_source_completion import wait_for_source
                source_completion=wait_for_source(capture/'canvas-producers.done',a.producer_samples,process,refresh)
            def physical_action(name,action):
                schedule=run/(name+'.json');schedule.write_text(json.dumps({'actions':[action]}))
                with (run/(name+'.log')).open('x') as out:
                    auxiliary=subprocess.Popen([str(input_binary),str(capture),str(schedule),str(run/(name+'-result.json'))],env=env,stdout=out,stderr=subprocess.STDOUT)
                    try:
                        deadline=time.monotonic()+10
                        while auxiliary.poll() is None and time.monotonic()<deadline:refresh();time.sleep(.02)
                        if auxiliary.poll() is None:raise ValueError('Physical action timed out: '+name)
                        if auxiliary.returncode:raise ValueError('Physical action failed: '+name)
                    finally:
                        if auxiliary.poll() is None:auxiliary.terminate();auxiliary.wait(timeout=5)
            # Quick Battle Escape opens the original results screen, not the
            # admitted campaign Mini. Validate that actual route explicitly.
            physical_action('results-open',{'kind':'tap','key':'Escape','after_ms':600})
            wait(26);result_payload=bytes(channel[42100:44708])
            send(26,16);physical_action('results-continue',{'kind':'wait','after_ms':600})
            physical_action('results-reopen',{'kind':'tap','key':'Escape','after_ms':600})
            wait(26);send(26,17);wait(22);send(22,2);returned_main=wait(3)
            physical_action('exit-main',{'kind':'wait','after_ms':200})
            if egl_peer:
                (capture/'egl-physical-close.flag').write_bytes(b'close\n');deadline=time.monotonic()+10
                while not (capture/'egl-physical.done').exists() and time.monotonic()<deadline:refresh();time.sleep(.02)
                if not (capture/'egl-physical.done').exists():raise ValueError('EGL physical interval did not close')
                (capture/'egl-physical-stop.flag').write_bytes(b'stop\n');egl_peer.wait(timeout=10)
                if egl_peer.returncode:raise ValueError('EGL physical peer failed')
            (capture/'stop.flag').write_bytes(b'stop\n');deadline=time.monotonic()+10
            while not a.control and not (capture/'session.done').exists() and time.monotonic()<deadline:refresh();time.sleep(.02)
            if physical_peer:
                (capture/'api-physical-stop.flag').write_bytes(b'stop\n');physical_peer.wait(timeout=10)
                if physical_peer.returncode:raise ValueError('Physical capture peer failed')
            quit_observation=None
            if a.engine_cadence:
                s=wait(3);refresh(3,s[0][0],0);quit_request=request
                from drawing_quit_completion import wait_for_quit
                quit_observation=wait_for_quit(process,state,refresh,quit_request)
                quit_ack=quit_observation['ack_observed']
                menu_actions.append(dict(request=quit_request,screen=3,action=3,argument=0,status=1 if quit_ack else None,attempt=1,normal_exit=True))
            observation={'frames':[],'inputs':[],'control_without_api_observer':True} if a.control else inspect(capture);gameplay=read_rows(capture/'gameplay.bin',complete=True);actors=creatures(gameplay)
            health_changes=[];initial={};movement=[]
            for row in gameplay:
                if row[1]!=1:continue
                key=(row[4],row[5],row[6]);values=(signed(row[8]),row[9],row[10],row[11],row[15]);first=initial.setdefault(key,values)
                if values[0]<first[0]:health_changes.append(row[0])
                if values[1:4]!=first[1:4]:movement.append(row[0])
            physical_observation=None
            if physical_peer:
                from drawing_api_physical import analyze as physical_analyze
                physical_observation=physical_analyze(capture,run/'physical-report.json')
            effects={'gameplay_records':len(gameplay),'distinct_snapshot_ticks':len({r[3] for r in gameplay if r[1]==1}),'sustained_gameplay_observed':len({r[3] for r in gameplay if r[1]==1})>=20,'active_actors':sum(bool(v['active']) for v in actors.values()),'movement_records':len(movement),'health_reduction_records':len(health_changes),'melee_returns':sum(r[1]==2 for r in gameplay),'semantic_selection_verified':False,'semantic_tooltip_verified':False,'semantic_pause_verified':False,'world_exit_verified':True,'results_continue_status':1,'results_quit_status':1,'returned_main_generation':returned_main[0][0]}
            from drawing_call_intervals import inspect_intervals
            intervals=inspect_intervals(capture) if a.observer_mode=='physical' else None
            producer_observation=None
            if scene:
                from drawing_producer_points import inspect_points
                producer_observation=inspect_points(capture)
            report={'cadence_control':a.cadence_control,'quit_observation':quit_observation,'source_completion':source_completion,'producer_observation':producer_observation,'call_intervals':intervals,'schema':1,'success':True,'source_sha256':BUILD,'sources':sources,'sources_stable':True,'experiment':str(run),'original_manifest_verified_before_after':True,'original_sections_unchanged':True,'original_pixels_used_as_native_inputs':False,'original_drawing_active':True,'observation':observation,'input_schedule':json.loads((run/'input.json').read_text()),'physical_presentations':[dict(path=str(x.relative_to(run)),sha256=sha(x.read_bytes()),synchronized_original_frame=False) for x in sorted((capture/'physical').glob('*.png'))],'menu_actions':menu_actions,'original_gameplay_observation':effects,'results_payload_hex':result_payload.hex(),'gameplay_sha256':sha((capture/'gameplay.bin').read_bytes()),'input_binary_sha256':sha(input_binary.read_bytes()),'staging':manifest,'observer_mode':a.observer_mode,'physical_oracle_mib':a.physical_oracle_mib,'physical_transient_mib':a.physical_transient_mib,'physical_peer_sha256':sha(a.physical_peer.read_bytes()) if a.physical_peer else None,'physical_peer_report_sha256':sha((run/'physical-report.json').read_bytes()) if physical_peer else None,'physical_api_observation':physical_observation,'native_parity_performed':False,'complete_journey_coverage':False}
            if claims:report['claims']=claims['claims']
            if sha((source/'Chaos.exe').read_bytes())!=BUILD or any(sha((ROOT/n).read_bytes())!=h for n,h in sources.items()):raise ValueError('Capture sources changed')
    except Exception as error:
        failure=error
        report={'schema':1,'success':False,'admitted':False,'error':str(error),'sources':sources,'experiment':str(run),'original_drawing_active':True,'complete_journey_coverage':False}
        if claims:report['claims']=claims['claims']
    finally:
        if egl_peer and egl_peer.poll() is None:egl_peer.terminate();egl_peer.wait(timeout=5)
        if physical_peer and physical_peer.poll() is None:physical_peer.terminate();physical_peer.wait(timeout=5)
        if input_process and input_process.poll() is None:input_process.terminate();input_process.wait(timeout=5)
        if process and process.poll() is None:
            os.killpg(process.pid,signal.SIGTERM)
            try:process.wait(timeout=5)
            except subprocess.TimeoutExpired:os.killpg(process.pid,signal.SIGKILL);process.wait(timeout=5)
        subprocess.run(['wineserver','-k'],env=env,check=False,timeout=10)
        if channel:channel.close();channel_file.close()
        verify('original-after')
    pending_paths=sorted(capture.glob('source-pending-refusal-*.bin'))
    if pending_paths:
        from drawing_source_pending_refusal import decode as decode_pending_frames
        try:
            report['source_pending_frame_refusals']=[dict(path=p.name,**decode_pending_frames(p.read_bytes())) for p in pending_paths]
        except Exception as error:
            failure=error;report.update(success=False,admitted=False,error=str(error))
    rejection_path=capture/'source-held-refusal.bin'
    if rejection_path.exists():
        from drawing_source_held_refusal import decode as decode_held_refusal
        try:
            report['source_held_rejection']=decode_held_refusal(rejection_path.read_bytes())
            report.update(success=False,admitted=False)
            failure=failure or ValueError('Held-source diagnostic refused')
        except Exception as error:
            failure=error;report.update(success=False,admitted=False,error=str(error))
    if x11_library and report.get('success'):
        from drawing_x11_delivery import decode_delivery
        try:
            if sha(Path(x11_library['path']).read_bytes())!=x11_library['sha256']:raise ValueError('Unix observer binary changed')
            report['x11_delivery_observation']=decode_delivery((capture/'x11-delivery.bin').read_bytes(),complete=False)
            if egl_peer:
                from drawing_egl_physical import analyze_egl_physical
                report['egl_physical_observation']=analyze_egl_physical(capture,run/'egl-physical-report.json')
                report['egl_physical_peer_report_sha256']=sha((run/'egl-physical-report.json').read_bytes())
            report['private_process_snapshots']=[dict(stage=stage,sha256=sha((run/f'process-{stage}.json').read_bytes()),record=json.loads((run/f'process-{stage}.json').read_text())) for stage in ('main','world')]
        except Exception as error:
            failure=error;report.update(success=False,admitted=False,error=str(error))
    if wine_submission and report.get('success'):
        from drawing_wine_submission import decode as decode_submissions
        try:
            calls={r[29]:r for r in struct.iter_unpack('<32I',(capture/'session.bin').read_bytes()[64:-128])}
            report['wine_submission_observation']=decode_submissions((capture/'wine-submission.bin').read_bytes(),calls,(capture/'wine-lifetime.bin').read_bytes())
            if a.wine_client_memory:
                from drawing_wine_client_memory import decode as decode_client_memory
                report['wine_client_memory_observation']=decode_client_memory((capture/'wine-client-memory.bin').read_bytes(),report['wine_submission_observation'],(capture/'wine-submission.bin').read_bytes())
            from drawing_wine_lifetime import decode as decode_lifetimes
            report['wine_lifetime_observation']=decode_lifetimes((capture/'wine-lifetime.bin').read_bytes())
            from drawing_wine_buffer import decode as decode_buffers
            report['wine_buffer_observation']=decode_buffers((capture/'wine-buffer.bin').read_bytes(),(capture/'wine-lifetime.bin').read_bytes())
            if a.wine_blit_worker:
                from drawing_wine_blit_worker import decode as decode_workers
                report['wine_blit_worker_observation']=decode_workers((capture/'wine-blit-worker.bin').read_bytes(),(capture/'wine-lifetime.bin').read_bytes())
            if a.wine_resource_map:
                from drawing_wine_resource_mapping import decode as decode_resource_maps
                report['wine_resource_map_observation']=decode_resource_maps((capture/'wine-resource-map.bin').read_bytes())
            if a.wine_retained_map_source:
                from drawing_wine_retained_map_source import decode as decode_retained_sources
                report['wine_retained_map_source_observation']=decode_retained_sources((capture/'wine-retained-map-source.bin').read_bytes(),(capture/'wine-resource-map.bin').read_bytes(),(capture/'wine-submission.bin').read_bytes(),(capture/'wine-lifetime.bin').read_bytes())
            if a.wine_map_storage_point:
                from drawing_wine_map_storage_point import decode as decode_map_points
                report['wine_map_storage_observation']=decode_map_points((capture/'wine-map-storage-point.bin').read_bytes(),report['wine_resource_map_observation'])
            if a.wine_blit_packet:
                from drawing_wine_blit_packet import decode as decode_packets
                report['wine_blit_packet_observation']=decode_packets((capture/'wine-blit-packet.bin').read_bytes(),{c['call']:c for c in report['wine_submission_observation']['calls']},{c['call']:c for c in report['wine_blit_worker_observation']['calls']})
            if a.wine_worker_context:
                from drawing_worker_context import decode as decode_contexts
                report['egl_worker_context_observation']=decode_contexts((capture/'egl-worker-context.bin').read_bytes())
        except Exception as error:
            failure=error;report.update(success=False,admitted=False,error=str(error))
    if a.canvas_storage_transitions and report.get('success'):
        try:
            from drawing_storage_transitions import decode as decode_storage_transitions
            report['canvas_storage_transition_observation']=decode_storage_transitions((capture/'canvas-storage-transitions-0000.bin').read_bytes())
            if not report['canvas_storage_transition_observation']['closed']:raise ValueError('Storage transition calls remain open')
        except (ValueError,OSError) as error:report.update(success=False,error='Storage transition decoding failed: '+str(error))
    if a.engine_cadence and report.get('success'):
        try:
            from drawing_engine_windows import analyze as analyze_windows
            report['engine_update_observation']=analyze_windows(capture/'engine-cadence.bin')
            report['normal_original_quit_verified']=True
            if a.producer_update_binding:
                from drawing_producer_updates import analyze as analyze_updates
                report['producer_update_observation']=analyze_updates(capture)
        except (ValueError,OSError,KeyError) as error:report.update(success=False,error='Engine update decoding failed: '+str(error))
    if a.source_byte_witness and report.get('success'):
        try:
            from drawing_source_byte_witness import analyze as analyze_source_bytes
            report['api_source_byte_observation']=analyze_source_bytes(capture)
            if a.source_readonly_lease:
                from drawing_source_lease import decode as decode_leases
                report['api_source_lease_observation']=decode_leases((capture/'api-source-lease.bin').read_bytes())
                if a.source_worker_lease:
                    from drawing_source_worker_lease import decode as decode_worker_leases
                    report['api_source_worker_lease_observation']=decode_worker_leases((capture/'api-source-worker-lease.bin').read_bytes())
                    if a.source_held_snapshot:
                        from drawing_source_held_snapshot import analyze as analyze_held
                        report['source_held_snapshot_observation']=analyze_held(capture)
        except Exception as error:
            failure=error;report.update(success=False,admitted=False,error=str(error))
    a.report.parent.mkdir(parents=True,exist_ok=True)
    with a.report.open('x') as f:json.dump(report,f,indent=2);f.write('\n')
    if failure:raise failure
    print(json.dumps({'success':True,'primary_planes':len(observation['frames']),'consumed_input_calls':len(observation['inputs']),'gameplay':effects}))
if __name__=='__main__':main()
