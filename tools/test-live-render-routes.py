#!/usr/bin/env python3
"""Bounded original campaign/movie-enabled routes with independent X11 region comparisons."""
import argparse
import configparser
import ctypes as c
import importlib.util
import json
import os
from pathlib import Path
import shutil
import struct
import subprocess
import tempfile
import time
from PIL import Image, ImageChops, ImageGrab
ROOT=Path(__file__).resolve().parents[1]
def module(name,path):
    spec=importlib.util.spec_from_file_location(name,ROOT/path);m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);return m
helper=module('route_helpers','tools/test-live-render-game.py')
menu=module('route_decode','tools/test-menu-observer.py')
def wait(test,process,seconds,reason):
    end=time.monotonic()+seconds
    while not test():
        if process.poll() is not None:raise RuntimeError(reason+': process exited')
        if time.monotonic()>end:raise RuntimeError(reason+': deadline')
        time.sleep(.05)
def winpath(path):return 'Z:'+str(path).replace('/','\\')
class XInput:
    def __init__(self,display):
        self.x=c.CDLL('libX11.so.6');self.xt=c.CDLL('libXtst.so.6');x=self.x
        x.XOpenDisplay.argtypes=[c.c_char_p];x.XOpenDisplay.restype=c.c_void_p
        x.XFlush.argtypes=x.XCloseDisplay.argtypes=[c.c_void_p]
        x.XDefaultRootWindow.argtypes=[c.c_void_p];x.XDefaultRootWindow.restype=c.c_ulong
        x.XQueryTree.argtypes=[c.c_void_p,c.c_ulong,c.POINTER(c.c_ulong),c.POINTER(c.c_ulong),c.POINTER(c.POINTER(c.c_ulong)),c.POINTER(c.c_uint)]
        x.XFetchName.argtypes=[c.c_void_p,c.c_ulong,c.POINTER(c.c_void_p)];x.XFree.argtypes=[c.c_void_p]
        x.XGetGeometry.argtypes=[c.c_void_p,c.c_ulong,c.POINTER(c.c_ulong),c.POINTER(c.c_int),c.POINTER(c.c_int),c.POINTER(c.c_uint),c.POINTER(c.c_uint),c.POINTER(c.c_uint),c.POINTER(c.c_uint)]
        x.XTranslateCoordinates.argtypes=[c.c_void_p,c.c_ulong,c.c_ulong,c.c_int,c.c_int,c.POINTER(c.c_int),c.POINTER(c.c_int),c.POINTER(c.c_ulong)]
        self.xt.XTestFakeMotionEvent.argtypes=[c.c_void_p,c.c_int,c.c_int,c.c_int,c.c_ulong]
        self.xt.XTestFakeButtonEvent.argtypes=[c.c_void_p,c.c_uint,c.c_int,c.c_ulong]
        self.d=x.XOpenDisplay(display.encode());assert self.d,'X11 display'
        self.origin=(0,0)
    def locate(self):
        x=self.x;root=x.XDefaultRootWindow(self.d);pending=[root];found=[];seen=set()
        while pending:
            window=pending.pop();seen.add(window);assert len(seen)<=512,'X11 window budget'
            name=c.c_void_p();label=''
            if x.XFetchName(self.d,window,c.byref(name)) and name.value:
                label=c.string_at(name.value).decode(errors='replace');x.XFree(name)
            parent=c.c_ulong();returned=c.c_ulong();children=c.POINTER(c.c_ulong)();count=c.c_uint()
            if x.XQueryTree(self.d,window,c.byref(returned),c.byref(parent),c.byref(children),c.byref(count)):
                pending.extend(children[i] for i in range(count.value) if children[i] not in seen)
                if children:x.XFree(children)
            px=c.c_int();py=c.c_int();width=c.c_uint();height=c.c_uint();border=c.c_uint();depth=c.c_uint()
            if label and x.XGetGeometry(self.d,window,c.byref(returned),c.byref(px),c.byref(py),c.byref(width),c.byref(height),c.byref(border),c.byref(depth)) and width.value==800 and height.value==600:
                child=c.c_ulong();x.XTranslateCoordinates(self.d,window,root,0,0,c.byref(px),c.byref(py),c.byref(child))
                found.append(dict(window=window,name=label,x=px.value,y=py.value,width=width.value,height=height.value))
        candidates=[v for v in found if any(s in v['name'].lower() for s in ['magic','mayhem','chaos','routeobservation']) and v['x']<800]
        assert candidates,('Original 800x600 X11 client not located',found)
        # The named game client takes precedence over its containing Wine desktop.
        chosen=next((v for v in candidates if 'magic' in v['name'].lower() or 'chaos' in v['name'].lower()),candidates[0]);self.origin=(chosen['x'],chosen['y']);return chosen
    def move(self,x,y):self.xt.XTestFakeMotionEvent(self.d,-1,x,y,0);self.x.XFlush(self.d)
    def click(self,x,y,button=1):
        self.move(x+self.origin[0],y+self.origin[1]);time.sleep(.25)
        self.xt.XTestFakeButtonEvent(self.d,button,1,0);self.x.XFlush(self.d);time.sleep(.15)
        self.xt.XTestFakeButtonEvent(self.d,button,0,0);self.x.XFlush(self.d);time.sleep(.15)
    def close(self):self.x.XCloseDisplay(self.d)
def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('build',type=Path);parser.add_argument('--prefix-template',type=Path,default=ROOT/'working/wineprefix-x86_64');parser.add_argument('--mode',choices=['campaign','movies-enabled','both'],default='both');parser.add_argument('--require-world-active',action='store_true',help='Require native World frames before/after failed-reader recovery and at least20 additional frames over2 seconds');parser.add_argument('--world-seconds',type=int,default=3,help='Bounded post-recovery World observation, 3..60 seconds');parser.add_argument('--require-world-pixels',action='store_true',help='Compare independent stable World terrain and portrait regions, dismissing the initial guidance dialog');parser.add_argument('--require-world-summon',action='store_true',help='Select and right-click the original tutorial Zombie summon; require independently read control count and stable pixels');parser.add_argument('--observe-world-summon',action='store_true',help='Require an original tutorial summon and record any native refusal with owned-state diagnostics');parser.add_argument('--ordered-copies',action='store_true',help='Opt into bounded native scheduling across original copy calls');args=parser.parse_args()
    if not 3<=args.world_seconds<=60:parser.error('--world-seconds must be 3..60')
    if args.require_world_summon and args.observe_world_summon:parser.error('Choose strict or diagnostic summon observation')
    if args.require_world_summon or args.observe_world_summon:args.require_world_pixels=True
    if args.require_world_pixels:args.require_world_active=True
    if args.require_world_active and args.mode=='movies-enabled':parser.error('--require-world-active needs the campaign route')
    assert os.environ.get('DISPLAY'),'Run under xvfb-run -a -s "-screen 0 1800x1000x24"'
    parent=ROOT/'working/tests/live-render-routes';parent.mkdir(parents=True,exist_ok=True);run=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(run,flush=True)
    paths=[*sorted((ROOT/'runtime/render').glob('*.[ch]')),*sorted((ROOT/'runtime/menu').glob('*.[ch]'))]
    paths += [ROOT/p for p in ['tools/test-live-render-routes.py','tools/test-live-render-game.py','tools/test-menu-observer.py','tools/build-menu-observer.py','tools/run-opengl-game.py','tools/build-render-bridge.py','tools/prepare-shadow-experiment.py','tests/live-render-route-probe.cpp','renderer/CMakeLists.txt','renderer/commands.cpp','renderer/commands.hpp','renderer/command_consumer.cpp','renderer/command_state.hpp','renderer/blit.cpp','renderer/blit.hpp','renderer/surface_copy.cpp','renderer/surface_copy.hpp','apps/qt-shell/live_command_session.cpp','apps/qt-shell/live_command_session.hpp','apps/qt-shell/render_control.cpp','apps/qt-shell/render_control.hpp','apps/qt-shell/live_command_renderer.cpp','apps/qt-shell/live_command_renderer.hpp','apps/qt-shell/command_channel.cpp','apps/qt-shell/command_channel.hpp','apps/qt-shell/gl_viewport.cpp','apps/qt-shell/gl_viewport.hpp']]
    paths += sorted((ROOT/'protocols/include/mnm').glob('render*.h'))
    paths += sorted((ROOT/'protocols/python/mnm_protocols').glob('render*.py'))
    report=dict(schema=1,success=False,sources={str(p.relative_to(ROOT)):helper.sha(p) for p in paths},cases=[],scope='Original campaign ingress through three forwarded World ticks, bounded World observation, optional independent tutorial summon/control-count checks and classified native refusal, and movie-enabled startup; finite 16 ms native observation, forced failed-reader recovery and independent unsynchronized stable X11 ROI comparisons. No full-frame, gameplay animation, movie frame or hardware-driver equivalence; no replacement.',live_replacement=False,full_frame_equivalence=False,world_active_required=args.require_world_active and not args.observe_world_summon,world_pixels_required=args.require_world_pixels and not args.observe_world_summon,world_initial_active_required=args.require_world_active,world_seconds=args.world_seconds,world_summon_required=args.require_world_summon,world_summon_observed=args.observe_world_summon,ordered_copies=args.ordered_copies)
    probe=args.build.resolve()/'live-render-route-probe'
    cache=args.build.resolve()/'CMakeCache.txt'
    build_type=next((line.split('=',1)[1] for line in cache.read_text().splitlines() if line.startswith('CMAKE_BUILD_TYPE:STRING=')),None)
    report['native_build']=dict(type=build_type,probe_sha256=helper.sha(probe))
    if args.require_world_summon or args.observe_world_summon:
        assert build_type in ['Release','RelWithDebInfo'],'Active World fixture requires an optimized consumer build'
        report['ocr_version']=subprocess.run(['tesseract','--version'],capture_output=True,text=True,check=True,timeout=5).stdout.splitlines()[0]
    env={k:v for k,v in os.environ.items() if not k.startswith('MNM_')};env.update(QT_QPA_PLATFORM='xcb',LIBGL_ALWAYS_SOFTWARE='1',WINEDEBUG='-all',WINEPREFIX=str(run/'wineprefix'));env.pop('WAYLAND_DISPLAY',None)
    source_prefs=ROOT/'working/game-nocd/CFG/prefs.cfg';prefs_hash=helper.sha(source_prefs)
    subprocess.run([str(ROOT/'tools/original-manifest.sh'),'verify'],check=True)
    try:
        subprocess.run(['cp','-a','--reflink=auto',str(args.prefix_template.resolve()),env['WINEPREFIX']],check=True)
        with (run/'wineboot.log').open('w') as log:subprocess.run(['wineboot','-u'],env=env,stdout=log,stderr=log,check=True,timeout=90)
        observer=module('route_menu_build','tools/build-menu-observer.py').build(run/'menu-build')
        for mode in (['campaign','movies-enabled'] if args.mode=='both' else [args.mode]):
            case=run/mode;case.mkdir();qt=wine=inputs=None;record=dict(mode=mode,comparisons=[],phases=[]);report['cases'].append(record)
            try:
                with (case/'qt.log').open('w') as qlog,(case/'wine.log').open('w') as wlog:
                    qt=subprocess.Popen([str(args.build.resolve()/'live-render-route-probe'),str(case)],env=env,stdout=qlog,stderr=qlog,start_new_session=True);wait(lambda:(case/'ready.json').exists(),qt,20,'Native ready')
                    frame=case/'frame.bin';helper.create(frame,helper.frame_v1.initial_header(),helper.frame_v1.SIZE)
                    child=dict(env,MNM_RENDER_ORDERED_COPIES='1' if args.ordered_copies else '0',MNM_RENDER_CONTINUOUS='1',MNM_RENDER_PALETTE_RESOURCES='1',MNM_RENDER_SESSION_ARCHIVE='1')
                    command=['python3',str(ROOT/'tools/run-opengl-game.py'),'--stream',str(frame),'--command-channel',str(case/'commands.bin'),'--render-control',str(case/'commands.bin.control'),'--capture-draws','--stage-only']
                    if mode=='campaign':command.append('--skip-movies')
                    staged=subprocess.run(command,env=child,capture_output=True,text=True,check=True,timeout=90);(case/'stage.log').write_text(staged.stdout+staged.stderr)
                    experiment=Path(next(l.removeprefix('Render experiment: ') for l in staged.stdout.splitlines() if l.startswith('Render experiment: ')));metadata=json.loads((experiment/'manifest.json').read_text());game=experiment/'game'
                    assert helper.sha(game/'Chaos.exe')==metadata['staged_sha256'] and helper.sha(game/'MnmRender.dll')==metadata['dll_sha256']
                    original=(game/'Chaos.exe').read_bytes();patched=module('route_import','tools/prepare-shadow-experiment.py').add_import(original,'MnmMenu.dll','MenuAnchor',b'.mnmenu');(game/'Chaos.exe').write_bytes(patched);shutil.copy2(observer,game/'MnmMenu.dll')
                    record.update(experiment=str(experiment.relative_to(ROOT)),source_sha256=metadata['source_sha256'],combined_executable_sha256=helper.sha(game/'Chaos.exe'),render_dll_sha256=metadata['dll_sha256'],menu_dll_sha256=helper.sha(observer),movies_skipped=mode=='campaign')
                    events=case/'events.bin';campaign=case/'campaign.bin'
                    child.update(MNM_MENU_OBSERVE=winpath(events),MNM_MENU_CAMPAIGN_OBSERVE=winpath(campaign))
                    for key,path in [('MNM_RENDER_STREAM',frame),('MNM_RENDER_COMMAND_CHANNEL',case/'commands.bin'),('MNM_RENDER_CONTROL',case/'commands.bin.control'),('MNM_RENDER_LOCK_CAPTURE_DIR',experiment/'lock-capture'),('MNM_RENDER_CAPTURE_DIR',experiment/'draw-capture'),('MNM_RENDER_FAILURE_LOG',experiment/'surface-failures.log')]:child[key]=winpath(path)
                    wine=subprocess.Popen(['wine','explorer','/desktop=RouteObservation,800x600',str(game/'Chaos.exe')],cwd=game,env=child,stdout=wlog,stderr=wlog,start_new_session=True)
                    def rows():
                        data=events.read_bytes() if events.exists() else b''
                        return menu.decode(data) if len(data)>=16 and not (len(data)-16)%64 else []
                    def ready(mid):return any(r['event'] in (1,4,9,10) and r['menu_id']==mid and r['initialized'] and not r['next_screen'] and not r['returning'] and not r['fade_active'] for r in rows())
                    wait(lambda:ready(3),wine,90,'Original Main readiness');inputs=XInput(env['DISPLAY']);record['original_client']=inputs.locate()
                    # A late reader joins through the failed-session recovery handshake.
                    with (case/'commands.bin').open('r+b') as f:f.seek(32);f.write(struct.pack('<I',1))
                    wait(lambda:struct.unpack_from('<I',(case/'commands.bin').read_bytes(),24)[0]==3,wine,10,'Producer failure acknowledgment');(case/'attach').write_bytes(b'attach')
                    request_id=0
                    def request(op):
                        nonlocal request_id
                        request_id+=1;temporary=case/'request.tmp';temporary.write_text(json.dumps(dict(id=request_id,op=op)));temporary.replace(case/'request.json')
                        reply=case/('reply-%08d.json'%request_id);wait(reply.exists,qt,5,'Probe '+op);return json.loads(reply.read_text())
                    wait(lambda:(case/'progress.json').exists(),qt,15,'Initial native frame');time.sleep(2)
                    def phase(name):
                        value=request('status');record['phases'].append(dict(name=name,**value));print(mode+': '+name+' '+json.dumps(value),flush=True);return value
                    def compare(name,rect):
                        inputs.move(1750,950)
                        value=phase(name)
                        if value['state']!=1:
                            record['comparisons'].append(dict(phase=name,performed=False,reason='Native session inactive',consumer=value));return
                        observations=[]
                        for attempt in range(3):
                            native=request('snapshot');start=time.monotonic();origin=inputs.origin
                            reference=ImageGrab.grab(xdisplay=env['DISPLAY']).crop((origin[0],origin[1],origin[0]+800,origin[1]+600)).convert('RGB');refname=name+'-original-'+str(attempt)+'.png';reference.save(case/refname)
                            image=Image.open(case/native['image']).convert('RGB');assert image.size==(800,600),'Native geometry'
                            difference=ImageChops.difference(image.crop(rect),reference.crop(rect));pixels=difference.tobytes();maximum=max(pixels);outside=sum(max(pixels[i:i+3])>8 for i in range(0,len(pixels),3))
                            observations.append(dict(attempt=attempt,native=native['image'],original=refname,rect=list(rect),max_channel_error=maximum,pixels_above_tolerance=outside,pixels=len(pixels)//3,tolerance=8,reference_capture_seconds=time.monotonic()-start,consumer=native))
                            if not outside:break
                            time.sleep(.3)
                        record['comparisons'].append(dict(phase=name,performed=True,matched=any(not x['pixels_above_tolerance'] for x in observations),synchronized=False,observations=observations))
                    compare('main-title',(100,30,700,180))
                    if mode=='campaign':
                        def click_cfg(relative,section):
                            cfg=configparser.ConfigParser(inline_comment_prefixes=(';',),interpolation=None);cfg.read(game/relative);r=list(map(int,cfg[section]['Rect2'].split(',')));inputs.click((r[0]+r[2])//2,(r[1]+r[3])//2)
                        click_cfg('Interface/MainScreen/screen (MainMenu).cfg','TEXTBUTTON_1');wait(lambda:ready(18),wine,20,'Region Entry readiness');time.sleep(1)
                        compare('region-title',(45,40,220,80));compare('region-difficulty',(45,92,620,114))
                        before=phase('region-before-forced-failure')
                        if before['state']==1:
                            ring=case/('commands.bin.retry-'+str(before['recoveries']));assert ring.exists()
                            with ring.open('r+b') as f:f.seek(32);f.write(struct.pack('<I',1))
                            time.sleep(2);phase('region-after-forced-failure');compare('region-recovered-title',(45,40,220,80))
                        click_cfg('Interface/RegionEntry/screen (Region Entry).cfg','TEXTBUTTON_1')
                        def world():return [r for r in rows() if r['event']==12 and r['menu_id']==2 and r['initialized']==1 and r['active_screen']==0x6cbb78]
                        wait(lambda:len(world())==3,wine,45,'Three original World ticks');record['world_ticks']=world();time.sleep(2);before=phase('world-before-forced-failure')
                        if before['state']==1:
                            ring=case/('commands.bin.retry-'+str(before['recoveries']));assert ring.exists()
                            with ring.open('r+b') as f:f.seek(32);f.write(struct.pack('<I',1))
                            time.sleep(4);phase('world-after-forced-failure')
                        else:record['world_failure_injection']='Refused: session already inactive'
                        if args.require_world_pixels:
                            compare('world-guidance',(160,225,640,375))
                            assert record['comparisons'][-1].get('matched'),'World guidance pixels differ'
                            inputs.click(400,300);inputs.move(1750,950)
                        if args.require_world_summon or args.observe_world_summon:
                            def original_text(name,rect,expected):
                                observations=[]
                                for attempt in range(8):
                                    origin=inputs.origin;image=ImageGrab.grab(xdisplay=env['DISPLAY']).crop((origin[0],origin[1],origin[0]+800,origin[1]+600)).convert('RGB')
                                    path=case/(name+'-original-'+str(attempt)+'.png');image.save(path)
                                    crop=case/(name+'-ocr.png');image.crop(rect).resize(((rect[2]-rect[0])*4,(rect[3]-rect[1])*4)).save(crop)
                                    text=subprocess.run(['tesseract',str(crop),'stdout','--psm','7',*(['-c','tessedit_char_whitelist=0123456789/'] if name.startswith('control-') else [])],capture_output=True,text=True,check=True,timeout=5).stdout.strip()
                                    observations.append(dict(original=path.name,rect=list(rect),text=text,matched=text==expected))
                                    if text==expected:break
                                    time.sleep(.5)
                                record.setdefault('original_ui_checks',[]).append(dict(name=name,expected=expected,observations=observations))
                                assert observations[-1]['matched'],name+' original UI did not reach expected state'
                            original_text('select-zombie',(360,257,590,292),'Select Zombie Spell')
                            inputs.click(523,575);inputs.move(1750,950)
                            original_text('summon-zombie',(340,82,560,123),'Summon Zombie')
                            compare('world-summon-instruction',(340,82,560,123))
                            if args.require_world_summon:assert record['comparisons'][-1].get('matched')
                            original_text('control-before',(716,444,759,466),'0/15')
                            inputs.click(400,280,button=3);inputs.move(1750,950)
                            original_text('control-after',(716,444,759,466),'1/15')
                            compare('world-summon-count',(716,444,759,466))
                            if args.require_world_summon:assert record['comparisons'][-1].get('matched')
                            record['original_summon_validated']=True
                            record['world_summon_validated']=bool(record['comparisons'][-1].get('matched'))
                            record['world_actions']=[dict(name='select-zombie',x=523,y=575,button=1),dict(name='summon-zombie',x=400,y=280,button=3)]
                        started=time.monotonic();sample=0
                        while time.monotonic()-started<args.world_seconds:
                            time.sleep(min(5,args.world_seconds-(time.monotonic()-started)))
                            sample+=1;value=phase('world-sustained-'+str(sample))
                            if args.observe_world_summon and value['state']==3:break
                            assert value['state']==1 and not value['error'],'Sustained World publication refused'
                            if args.require_world_pixels:
                                for label,rect in [('terrain',(0,0,160,180)),('portrait',(700,500,778,561))]:
                                    compare('world-'+label+'-'+str(sample),rect)
                                    comparison=record['comparisons'][-1]
                                    if args.observe_world_summon and comparison.get('reason')=='Native session inactive' and comparison['consumer']['state']==3:break
                                    assert record['comparisons'][-1].get('matched'),'World '+label+' pixels differ'
                        phase('world-end')
                        if args.require_world_active:
                            phases={p['name']:p for p in record['phases']}
                            before=phases['world-before-forced-failure'];after=phases.get('world-after-forced-failure');final=phases['world-end']
                            native_active=all(p['state']==1 and not p['error'] and p['width']==800 and p['height']==600 for p in [before,after,final] if p)
                            assert after and all(p['state']==1 and not p['error'] and p['width']==800 and p['height']==600 for p in [before,after]),'Initial World native publication/recovery refused'
                            if not args.observe_world_summon:assert native_active,'World native publication/recovery refused'
                            assert after['recoveries']==before['recoveries']+1==final['recoveries'],'Unexpected World recovery session'
                            assert final['frames']-after['frames']>=20 and final['ms']-after['ms']>=2000,'World publication did not continue after recovery'
                            record['world_active_validated']=native_active
                            record['world_pixels_validated']=args.require_world_pixels and native_active
                            record['world_observation_ms']=final['ms']-after['ms']
                            record['world_additional_frames']=final['frames']-after['frames']
                        origin=inputs.origin;ImageGrab.grab(xdisplay=env['DISPLAY']).crop((origin[0],origin[1],origin[0]+800,origin[1]+600)).save(case/'world-original.png')
                    else:
                        time.sleep(8);phase('movie-enabled-main-end')
                    record['original_process_alive']=wine.poll() is None;assert record['original_process_alive']
                    record['menu_records']=rows();record['consumer']=request('finish');assert qt.wait(timeout=5)==0
                    diagnostics=experiment/'lock-capture/lifecycle.log';record['lifecycle']=diagnostics.read_text().splitlines() if diagnostics.exists() else []
                    record['ring_headers']={p.name:list(struct.unpack('<16I',p.read_bytes()[:64])) for p in case.glob('commands.bin*') if not p.name.endswith('.control')}
                    record['copy_conflicts']=[dict(zip(['target','thread','source','prepared','before_epoch','after_epoch','before_source_generation','after_source_generation','before_target_generation','after_target_generation','fill','bootstrap','direct','result','source_origin','target_origin','caller','source_caller','source_owner'],[int(x,16) for x in line.split()[1:]])) for line in record['lifecycle'] if line.startswith('blit_commit_refused ')]
                    if args.observe_world_summon and not record['world_active_validated']:
                        assert record['original_summon_validated'] and record['consumer']['state']==3 and record['consumer']['terminal_resources']==0
                        assert any(c['prepared'] and c['before_source_generation']!=c['after_source_generation'] and c['source_origin']==8 for c in record['copy_conflicts']),'Native refusal lacked successful source-copy conflict'
                        record['native_summon_refusal_classified']=True
                    record['refusals']=[line for line in record['lifecycle'] if line.startswith(('session_gap ','checkpoint_admission_refused ','command_queue_refused ','blit_invalidated ','blit_untracked '))]
                    assert any(c.get('matched') for c in record['comparisons']),'No independent stable region matched'
                    candidates=list((experiment/'draw-capture').glob('*.bin'));record['draw_capture_files']=[str(p.relative_to(ROOT)) for p in candidates]
                    # Existing bounded original call-site archive can confirm sample playback.
                    movie_calls=[]
                    for path in candidates:
                        data=path.read_bytes()
                        if data[:8]==b'MNMDRW01':
                            for at in range(16,len(data)-63,64):
                                values=struct.unpack_from('<16I',data,at)
                                if 0x469830<=values[2]<0x469a00:movie_calls.append(dict(sequence=values[0],operation=values[1],caller=values[2],result=values[6]))
                    record['original_movie_sample_calls']=movie_calls;record['movie_playback_confirmed']=bool(movie_calls)
                    artifacts=[case/'summary.json',case/'events.bin',case/'stage.log',case/'qt.log',experiment/'manifest.json',experiment/'bridge-build.json',*case.glob('*.png')]
                    if diagnostics.exists():artifacts.append(diagnostics)
                    record['artifacts']={str(p.relative_to(ROOT)):helper.sha(p) for p in artifacts};record['success']=True
            finally:
                if inputs:inputs.close()
                helper.stop(qt);helper.stop(wine);subprocess.run(['wineserver','-k'],env=env,check=False,timeout=10)
        assert helper.sha(source_prefs)==prefs_hash,'Source preferences changed'
        assert all(helper.sha(ROOT/p)==h for p,h in report['sources'].items()),'Source changed during observation'
        report.update(success=True,original_manifest_verified_before_after=True,source_preferences_unchanged=True)
    finally:
        subprocess.run(['wineserver','-k'],env=env,check=False,timeout=10);subprocess.run([str(ROOT/'tools/original-manifest.sh'),'verify'],check=True)
        (run/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(run/'report.json',flush=True)
if __name__=='__main__':main()
