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
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--source-game',type=Path,required=True);p.add_argument('--prefix-template',type=Path,required=True);p.add_argument('--schedule',type=Path,default=ROOT/'tests/fixtures/drawing-journeys/interactive-schedule.json');p.add_argument('--report',type=Path,required=True);p.add_argument('--claims',type=Path);p.add_argument('--observer-mode',choices=['full','metadata','inputs','physical'],default='metadata');p.add_argument('--physical-peer',type=Path,help='Reference-only X11 peer; required for physical mode');p.add_argument('--physical-oracle-mib',type=int,choices=range(1,3073),default=1024);p.add_argument('--physical-transient-mib',type=int,choices=range(1,2049),default=256);p.add_argument('--control',action='store_true',help='Run the same original menus/inputs with the API observer disabled');a=p.parse_args()
    if (a.observer_mode=='physical') != (a.physical_peer is not None):p.error('Physical mode requires exactly one physical peer')
    if a.control and a.physical_peer:p.error('Physical API capture cannot be a control run')
    if a.observer_mode!='physical' and (a.physical_oracle_mib!=1024 or a.physical_transient_mib!=256):p.error('Physical budgets require physical mode')
    if a.report.exists():p.error('Refusing evidence overwrite')
    names=['tools/capture-drawing-interactive.py','tools/drawing-session-runner.py','tools/drawing_call_intervals.py','tools/drawing_api_session.py','tools/build-drawing-session.py','tools/build-drawing-session-input.py','tools/build-menu-observer.py','tools/build-shadow-bridge.py','tools/prepare-shadow-experiment.py','tools/prepare-scene-observer.py','tools/run-game.sh','tools/run-opengl-game.py','tools/campaign_gameplay.py','runtime/drawing/session.c','runtime/drawing/call_intervals.h','runtime/drawing/physical_fence.h','tools/drawing_api_physical.py','apps/drawing-session/observe.cpp','apps/drawing-session/CMakeLists.txt','runtime/shadow/win32_min.h','tests/drawing-session-input.cpp','apps/qt-shell/window_host.cpp','apps/qt-shell/window_host.hpp']
    names += [str(x.relative_to(ROOT)) for x in (ROOT/'runtime/menu').glob('*') if x.suffix in ('.h','.c','.S')]
    names += [str(x.relative_to(ROOT)) for x in (ROOT/'protocols/include/mnm').glob('menu_v*.h')]
    sources={n:sha((ROOT/n).read_bytes()) for n in names};sources[str(a.schedule.resolve().relative_to(ROOT))]=sha(a.schedule.read_bytes())
    claims=json.loads(a.claims.read_text()) if a.claims else None
    if claims:sources.update(claims['sources'])
    if any(sha((ROOT/n).read_bytes())!=h for n,h in sources.items()):raise ValueError('Prospective sources changed')
    source=a.source_game.resolve();original=(source/'Chaos.exe').read_bytes()
    if sha(original)!=BUILD:raise ValueError('Unsupported original executable; no staging writes')
    parent=ROOT/'working/experiments/drawing-api-session';parent.mkdir(parents=True,exist_ok=True);run=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(run,flush=True)
    game=run/'game';capture=run/'capture';capture.mkdir();process=input_process=physical_peer=None;channel=None;failure=None
    env={k:v for k,v in os.environ.items() if not k.startswith('MNM_')};env.update(WINEPREFIX=str(run/'wineprefix'),WINEDEBUG='-all',MNM_DRAW_SESSION_EXPERIMENT=str(run),QT_QPA_PLATFORM='xcb');env.pop('WAYLAND_DISPLAY',None)
    if a.observer_mode=='physical':env['MNM_DRAW_PHYSICAL_MIB']=str(a.physical_oracle_mib)
    def verify(label):
        out=subprocess.run([str(ROOT/'tools/original-manifest.sh'),'verify'],capture_output=True,text=True,timeout=120);(run/(label+'.log')).write_text(out.stdout+out.stderr);out.check_returncode()
    verify('original-before')
    try:
        load('tools/prepare-scene-observer.py').copy_installation(source,game)
        drawing=None if a.control else load('tools/build-drawing-session.py').build();menu=load('tools/build-menu-observer.py').build();input_binary=load('tools/build-drawing-session-input.py').build()
        importer=load('tools/prepare-shadow-experiment.py');patched=original if a.control else importer.add_import(original,'MnmDrawing.dll','DrawingAnchor',b'.mndraw');patched=importer.add_import(patched,'MnmMenu.dll','MenuAnchor',b'.mnmenu')
        pe=struct.unpack_from('<I',original,60)[0];count=struct.unpack_from('<H',original,pe+6)[0];table=pe+24+struct.unpack_from('<H',original,pe+20)[0]
        for i in range(count):
            size,at=struct.unpack_from('<II',original,table+40*i+16)
            if original[at:at+size]!=patched[at:at+size]:raise ValueError('Original section changed in import staging')
        (game/'Chaos.exe').write_bytes(patched)
        for binary in ([menu] if a.control else [drawing,menu]):shutil.copy2(binary,game/binary.name)
        settings=load('tools/run-opengl-game.py');preferences=settings.disable_cd_music(game)+settings.skip_movies(game)
        manifest={'schema':1,'source_sha256':BUILD,'binaries':{n:sha((game/n).read_bytes()) for n in (('Chaos.exe','MnmMenu.dll') if a.control else ('Chaos.exe','MnmDrawing.dll','MnmMenu.dll'))},'original_sections_unchanged':True,'preferences':preferences,'api_observer_enabled':not a.control,'observer_mode':a.observer_mode,'scope':'Original active stock Quick Battle; API/readback observation and XTest. No original simulation/raster body suppression.'};(run/'manifest.json').write_text(json.dumps(manifest,indent=2)+'\n')
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
            s=wait(screen);refresh(action,s[0][0],argument,rules,slots);reply=wait(ack=request)
            if reply[0][4]!=1:raise ValueError('Original menu action refused: '+str(reply[0]))
            menu_actions.append({'request':request,'screen':screen,'action':action,'argument':argument,'status':reply[0][4]})
        if a.physical_peer:
            with (run/'physical-peer.log').open('x') as log:physical_peer=subprocess.Popen([str(a.physical_peer.resolve()),str(capture),str(run/'physical-report.json'),'--api','--transient-mib',str(a.physical_transient_mib)],env=env,stdout=log,stderr=subprocess.STDOUT)
        with (run/'game.log').open('x') as log:
            process=subprocess.Popen([str(ROOT/'tools/run-game.sh'),'launch','--no-gamescope','--prefix',env['WINEPREFIX'],'--runner',str(ROOT/'tools/drawing-session-runner.py')],env=env,stdout=log,stderr=subprocess.STDOUT,start_new_session=True)
            send(3,1);send(22,4);s=wait(14);rules=list(struct.unpack_from('<13I',s[1],8))+[struct.unpack_from('<I',s[1],60+i*48+12)[0] for i in range(4)];rules[2]=3
            send(14,6,rules=rules);send(25,9,2);send(14,7,rules=rules);send(7,12,slots=wait(7)[2])
            with (run/'input.log').open('x') as ilog:input_process=subprocess.Popen([str(input_binary),str(capture),str(a.schedule.resolve()),str(run/'input.json')],env=env,stdout=ilog,stderr=subprocess.STDOUT)
            deadline=time.monotonic()+120
            while input_process.poll() is None and time.monotonic()<deadline:
                if process.poll() is not None:raise ValueError('Original exited during input schedule')
                refresh();time.sleep(.05)
            if input_process.poll() is None:raise ValueError('Input schedule timed out')
            if input_process.returncode:raise ValueError('Input fixture refused; inspect input.json/log')
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
            (capture/'stop.flag').write_bytes(b'stop\n');deadline=time.monotonic()+10
            while not a.control and not (capture/'session.done').exists() and time.monotonic()<deadline:refresh();time.sleep(.02)
            if physical_peer:
                (capture/'api-physical-stop.flag').write_bytes(b'stop\n');physical_peer.wait(timeout=10)
                if physical_peer.returncode:raise ValueError('Physical capture peer failed')
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
            report={'call_intervals':intervals,'schema':1,'success':True,'source_sha256':BUILD,'sources':sources,'sources_stable':True,'experiment':str(run),'original_manifest_verified_before_after':True,'original_sections_unchanged':True,'original_pixels_used_as_native_inputs':False,'original_drawing_active':True,'observation':observation,'input_schedule':json.loads((run/'input.json').read_text()),'physical_presentations':[dict(path=str(x.relative_to(run)),sha256=sha(x.read_bytes()),synchronized_original_frame=False) for x in sorted((capture/'physical').glob('*.png'))],'menu_actions':menu_actions,'original_gameplay_observation':effects,'results_payload_hex':result_payload.hex(),'gameplay_sha256':sha((capture/'gameplay.bin').read_bytes()),'input_binary_sha256':sha(input_binary.read_bytes()),'staging':manifest,'observer_mode':a.observer_mode,'physical_oracle_mib':a.physical_oracle_mib,'physical_transient_mib':a.physical_transient_mib,'physical_peer_sha256':sha(a.physical_peer.read_bytes()) if a.physical_peer else None,'physical_api_observation':physical_observation,'native_parity_performed':False,'complete_journey_coverage':False}
            if claims:report['claims']=claims['claims']
            if sha((source/'Chaos.exe').read_bytes())!=BUILD or any(sha((ROOT/n).read_bytes())!=h for n,h in sources.items()):raise ValueError('Capture sources changed')
    except Exception as error:
        failure=error
        report={'schema':1,'success':False,'admitted':False,'error':str(error),'sources':sources,'experiment':str(run),'original_drawing_active':True,'complete_journey_coverage':False}
        if claims:report['claims']=claims['claims']
    finally:
        if physical_peer and physical_peer.poll() is None:physical_peer.terminate();physical_peer.wait(timeout=5)
        if input_process and input_process.poll() is None:input_process.terminate();input_process.wait(timeout=5)
        if process and process.poll() is None:
            os.killpg(process.pid,signal.SIGTERM)
            try:process.wait(timeout=5)
            except subprocess.TimeoutExpired:os.killpg(process.pid,signal.SIGKILL);process.wait(timeout=5)
        subprocess.run(['wineserver','-k'],env=env,check=False,timeout=10)
        if channel:channel.close();channel_file.close()
        verify('original-after')
    a.report.parent.mkdir(parents=True,exist_ok=True)
    with a.report.open('x') as f:json.dump(report,f,indent=2);f.write('\n')
    if failure:raise failure
    print(json.dumps({'success':True,'primary_planes':len(observation['frames']),'consumed_input_calls':len(observation['inputs']),'gameplay':effects}))
if __name__=='__main__':main()
