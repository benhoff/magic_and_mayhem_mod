#!/usr/bin/env python3
"""Finite public Quick camera/cursor/picking matrix; stop on every failed case."""
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import secrets
import signal
import sqlite3
import struct
import subprocess
import sys
import time
import ctypes as c

from coverage_evidence import Reader, write_report
from native_only_contract import configure, outcome
from native_prefix_template import borrow
from rendering_pilot import require
from rendering_build_binding import native_runtime_source_paths

ROOT = Path(__file__).resolve().parents[1]
PREFIX = 'research/runtime/rendering-interaction-results-20261107/'
PLAN = PREFIX + 'input-plan-07.json'


def module(name):
    s = importlib.util.spec_from_file_location('camera_' + name.replace('-', '_'), ROOT / 'tools' / (name + '.py'))
    m = importlib.util.module_from_spec(s)
    s.loader.exec_module(m)
    return m


menus = module('run-rendering-menu-acceptance')
sha = menus.sha


camera = module('run-rendering-camera-acceptance')
from rendering_interaction import decode
from rendering_interaction_live import LiveSources,SpriteNotReady
from rendering_interaction_budget import admit

class InteractionJourney(camera.CameraJourney):
    def __init__(self,*args):
        super().__init__(*args);self.source_frames=LiveSources(self.out/'worlds.bin');self.viewport_geometry=None;self.scaling_geometry=None
        self.input.x.XTranslateCoordinates.argtypes=[c.c_void_p,c.c_ulong,c.c_ulong,c.c_int,c.c_int,c.POINTER(c.c_int),c.POINTER(c.c_int),c.POINTER(c.c_ulong)]

    def window_origin(self):
        x,y,child=c.c_int(),c.c_int(),c.c_ulong()
        require(self.input.x.XTranslateCoordinates(self.input.display,self.input.window,self.input.x.XDefaultRootWindow(self.input.display),0,0,c.byref(x),c.byref(y),c.byref(child)),'Host geometry unavailable')
        return [x.value,y.value]

    def screen_geometry(self,offset):
        origin=self.window_origin();return [origin[0]+offset[0],origin[1]+offset[1],offset[2],offset[3]]
    def latest(self):
        raw=(self.session/'frame-input.bin').read_bytes()
        # Streamed writes may be in progress. Only complete records are observed
        # here; the stopped offline decoder rejects every torn terminal byte.
        raw=raw[:16+(len(raw)-16)//384*384]
        rows=decode(raw,allow_pending=True)['rows']
        return next(r for r in reversed(rows) if r['kind']==1 and r['frame'])

    def mark(self, action, phase):
        point=super().mark(action,phase)
        state=self.latest()
        point.update(frame_input_row=state['ordinal'],frame_generation=state['frame']['generation'],
                     original_frame_tick=state['tick'],scaling=self.viewport.poll()['scaling'])
        return point

    def ready_target(self):
        # Wait only for an admitted visible body at the tracked draw anchor.
        # Copy and quarter/half/three-quarter body passes share sprite geometry;
        # neither hit feedback nor a guessed screen point supplies the target.
        deadline=time.monotonic()+15
        while time.monotonic()<deadline:
            require(self.child.poll() is None,'Player exited before visible wizard body')
            state=self.latest();t=state['tracked']
            require(t and t['slot']==0 and t['type']==0 and t['owner']==0 and t['active'] and t['health']>0,'Original living wizard role changed')
            try:return state,self.source_frames.target(state['frame']['generation'],state['history'])
            except SpriteNotReady:time.sleep(.1)
        raise ValueError('Original visible wizard body readiness deadline')

    def focus_viewport(self):
        nodes=[(n,v) for n,v in self.access.nodes() if v.get('accessible_id','').endswith('.nativeGameViewport')]
        require(len(nodes)==1,'Original public viewport is unavailable')
        require(nodes[0][0].get_component_iface().grab_focus(),'Public accessibility focus refused')
        self.input.activate()

    def set_scaling(self, mode):
        wanted={'sharp':0,'integer':2}[mode];live=self.viewport.poll()
        if self.scaling_geometry is None:
            nodes=[v for _,v in self.access.nodes() if v.get('accessible_id','').endswith('.viewportScaling')]
            require(len(nodes)==1 and nodes[0]['enabled'],'Public scaling unavailable')
            origin=self.window_origin();r=nodes[0]['rect'];self.scaling_geometry=[r[0]-origin[0],r[1]-origin[1],r[2],r[3]]
        if not live or live['scaling']!=wanted:
            self.input.click(self.screen_geometry(self.scaling_geometry));self.key('scaling-'+mode,0xff50 if mode=='sharp' else 0xff57);self.key('scaling-confirm',0xff0d)
        if live:
            deadline=time.monotonic()+5
            while time.monotonic()<deadline and self.viewport.poll()['scaling']!=wanted:time.sleep(.05)
            require(self.viewport.poll()['scaling']==wanted,'Actual viewport did not adopt public scaling')
        self.actions.append(dict(action='scaling',mode=mode,host_ns=time.monotonic_ns(),control_rect=self.screen_geometry(self.scaling_geometry)))
        if self.session and self.viewport.poll():
            # A real scene click establishes the widget's mouse focus after
            # using the toolbar. Do not count an accessibility focus request as
            # proof that the original input channel became active.
            state,target=self.ready_target()
            self.actions.append(dict(action='source-sprite-focus',mode=mode,frame_input_row=state['ordinal'],**target))
            self.pointer(target['point'])
            for down in (1,0):
                require(self.input.xt.XTestFakeButtonEvent(self.input.display,1,down,0),'Physical scene focus click refused')
                self.input.x.XFlush(self.input.display);time.sleep(.12)
            self.wait_progress(int(self.viewport.poll()['world']),minimum=5)

    def pointer(self, point):
        p=self.viewport.poll()
        if self.viewport_geometry is None:
            nodes=[v for _,v in self.access.nodes() if v.get('accessible_id','').endswith('.nativeGameViewport')]
            require(len(nodes)==1 and nodes[0]['rect'],'Viewport screen rectangle missing')
            origin=self.window_origin();r=nodes[0]['rect'];self.viewport_geometry=[r[0]-origin[0],r[1]-origin[1],r[2],r[3]]
            self.actions.append(dict(action='viewport-geometry-observed',rect=r,host_origin=origin,host_ns=time.monotonic_ns()))
        rect=self.screen_geometry(self.viewport_geometry);x,y=point
        require(rect[2:]==[p['width'],p['height']],'Viewport resized outside the fixed-window case')
        require(0<=x<self.size[0] and 0<=y<self.size[1],'Source wizard anchor outside frame')
        # DPR is explicitly1. Choose a display pixel centre inside the desired
        # logical texel; retain both coordinates for independent input checking.
        px=int((x+.5)*p['image_width']/self.size[0])+p['left'];py=int((y+.5)*p['image_height']/self.size[1])+p['top']
        require(self.input.xt.XTestFakeMotionEvent(self.input.display,-1,rect[0]+px,rect[1]+py,0),'Physical pointer refused')
        self.input.x.XFlush(self.input.display)
        self.actions.append(dict(action='pointer',logical=list(point),screen=[rect[0]+px,rect[1]+py],viewport=rect,viewport_offset=self.viewport_geometry,host_origin=[rect[0]-self.viewport_geometry[0],rect[1]-self.viewport_geometry[1]],paint=p['paint'],world=p['world'],host_ns=time.monotonic_ns()))

    def settle(self, action, released, delay=.55):
        time.sleep(delay);self.wait_progress(int(released['world']),minimum=3);self.mark(action,'settled')

    def move(self, action, point):
        self.mark(action,'begin');self.pointer(point);released=self.mark(action,'released');self.settle(action,released)

    def toggle(self, action, point):
        self.mark(action,'begin');self.pointer(point)
        code=self.input.x.XKeysymToKeycode(self.input.display,0xffe1)
        require(code and self.input.xt.XTestFakeKeyEvent(self.input.display,code,1,0),'Physical Shift down refused')
        self.input.x.XFlush(self.input.display);time.sleep(.3)
        for down in (1,0):
            require(self.input.xt.XTestFakeButtonEvent(self.input.display,1,down,0),'Physical selection click refused')
            self.input.x.XFlush(self.input.display);time.sleep(.10)
        require(self.input.xt.XTestFakeKeyEvent(self.input.display,code,0,0),'Physical Shift up refused')
        self.input.x.XFlush(self.input.display)
        released=self.mark(action,'released');self.settle(action,released)

    def pick_pair(self, name):
        state,target=self.ready_target();require(state['tracked']['slot']==0 and state['tracked']['type']==0 and state['tracked']['owner']==0 and state['tracked']['health']>0,'Original living wizard identity differs')
        require(state['history'] is not None,'Wizard draw history missing')
        self.actions.append(dict(action='source-sprite-target',name=name,frame_input_row=state['ordinal'],**target))
        point=target['point']
        self.move(name+'-hover',point)
        hit=self.latest();require(hit['hit'] and hit['hit']['slot']==0 and hit['hit']['type']==0 and hit['hit']['owner']==0,'Source-derived wizard hover did not hit original wizard')
        self.toggle(name+'-toggle-1',point)
        self.move(name+'-away-1',[40,100])
        self.move(name+'-hover-2',point)
        self.toggle(name+'-toggle-2',point)
        self.move(name+'-away-2',[40,100])

    def interaction_schedule(self):
        for mode in ('sharp','integer'):
            self.set_scaling(mode)
            if self.latest()['mode']==1:self.held_key(mode+'-resume',0x20,.22,.6)
            for n in range(2):self.held_key(mode+'-center-'+str(n),ord('w'),.22,.6)
            for view in range(4):
                self.pick_pair(mode+'-view-'+str(view))
                self.held_key(mode+'-rotate-'+str(view),ord('.'),.22,.65)
            self.held_key(mode+'-pan',0xff52,.22,.8)
            self.pick_pair(mode+'-panned')

    def await_main(self):
        # Movie output is outside this camera case. Keep movies enabled and
        # use the existing physical Escape action once per accepted intro.
        skipped=set();deadline=time.monotonic()+420
        while time.monotonic()<deadline:
            require(self.child.poll() is None,'Player exited during startup')
            value=self.read()
            if value and value[0]['ready'] and value[0]['screen']==3:
                self.wait(3,0,timeout=2);return
            if self.session:
                path=self.session/'native-media.bin'
                if path.exists():
                    raw=path.read_bytes()
                    if len(raw)==2048 and raw[:8]==b'MNMMED01':
                        request,ident,operation=struct.unpack_from('<3I',raw,16)
                        response,status,_,accepted,sequence=struct.unpack_from('<5I',raw,512)
                        if not request&1 and not sequence&1 and ident and ident==response==accepted and operation==1 and status==2 and ident not in skipped:
                            require(len(skipped)<2,'More than two original intro requests')
                            self.focus_viewport();self.key('physical-intro-skip',0xff1b);skipped.add(ident)
                            self.actions.append(dict(action='intro-skip-request',request=ident,path=raw[64:324].split(b'\0',1)[0].decode('ascii')))
            time.sleep(.1)
        raise ValueError('Initial Main readiness deadline')

    def run(self):
        self.input.activate()
        self.set_scaling('sharp')
        self.click('Launch game')
        self.await_main()
        self.capture('main-ready-initial', 'Quick Battle')
        self.click('Preferences')
        preferences = self.wait(10, 1)['preferences']
        self.key('preferences-cancel', 0xff1b)
        self.wait(3, 2)
        self.click('Quick Battle')
        self.wait(22, 3)
        self.click('Create Single Player Game')
        self.wait(14, 4)
        self.click('Map')
        self.wait(25, 5)
        lists = [(n, v) for n, v in self.access.nodes() if v['role'] == 'list']
        require(len(lists) == 1, 'Ambiguous physical map list')
        self.input.click(lists[0][1]['rect'])
        self.key('map-first', 0xff50)
        selected_map = lists[0][0].get_selection_iface().get_selected_child(0)
        require(selected_map is not None and selected_map.get_name() == self.last['maps'][0], 'Actual public map selection differs')
        self.key('map-confirm-existing-keyboard-route', 0xff0d)
        selected = self.wait(14, 6)
        require(selected['setup']['map'] == 1 and selected['setup']['map_name'] == 'Celtic Forest One (small)', 'Wrong admitted Quick map')
        self.click_id('singlePlayerStart')
        self.wait(7, 7, timeout=60)
        self.click_id('spellboxOK')
        self.wait_progress()
        self.interaction_schedule()
        self.held_key('open-results', 0xff1b, .6, .2, advance=False)
        # Public sessions may expose the ordinary battle Mini first. Observe
        # the real ready screen and use its existing physical Quit Battle.
        deadline = time.monotonic() + 30
        while time.monotonic() < deadline:
            value = self.read()
            if value and value[0]['ready'] and value[0]['screen'] in (17, 26):
                break
            time.sleep(.05)
        require(value and value[0]['ready'] and value[0]['screen'] in (17, 26), 'No native Mini/Results after Escape')
        if value[0]['screen'] == 17:
            self.wait(17)
            self.click_id('miniMenuButton7')
        self.wait(26)
        self.click_id('quickResultAction2')
        self.wait(22)
        self.key('quick-return', 0xff1b)
        self.wait(3)
        self.capture('main-ready-returned', 'Quick Battle')
        self.click('Quit')
        return dict(preferences=preferences, map=selected['setup'], camera_actions=self.camera_actions)


def main(*, source_capture_path=None):
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--report', required=True)
    p.add_argument('--software', action='store_true')
    p.add_argument('--low', action='store_true')
    p.add_argument('--repair-of')
    p.add_argument('--repair-reason')
    args = p.parse_args()
    if os.environ.get('MNM_INTERACTION_BUS_OWNED')!='1':
        runtime=ROOT/'working/interaction-runtime'/Path(args.report).stem
        runtime.mkdir(parents=True,mode=0o700,exist_ok=False)
        isolated=dict(os.environ,XDG_RUNTIME_DIR=str(runtime),MNM_INTERACTION_BUS_OWNED='1')
        isolated.pop('AT_SPI_BUS_ADDRESS',None)
        os.execvpe('dbus-run-session',['dbus-run-session','--',sys.executable,__file__,*sys.argv[1:]],isolated)
    require(args.report.startswith(PREFIX) and '..' not in Path(args.report).parts and not (ROOT / args.report).exists(), 'Fresh owned report required')
    require(bool(args.repair_of) == bool(args.repair_reason), 'Repair needs retained failure and concrete reason')
    out = ROOT / 'working/interaction-acceptance' / Path(args.report).stem
    out.mkdir(parents=True, exist_ok=False)
    (out / '.retain').touch()
    require(args.report.endswith('-acquisition.json'),'Acquisition report suffix required')
    size = [640, 480] if args.low else [800, 600]
    names = set(native_runtime_source_paths(ROOT)) | {'tools/run-rendering-interaction.py', 'tools/rendering_interaction.py', 'tools/run-rendering-camera-acceptance.py',
        'tools/run-rendering-menu-acceptance.py', 'tools/rendering_menu_accessibility.py', 'tools/rendering_interaction_live.py', 'tools/rendering_interaction_budget.py', 'tools/resident_capture.py', 'tools/inspect-canvas-producers.py',
        'tools/native_only_contract.py', 'tools/native_prefix_template.py', 'tools/run-native-owned.py',
        'tools/rendering_build_binding.py', 'tools/menu-game-runner.py'}
    with Reader(ROOT) as reader:
        plan = reader.json(PLAN)
        require(plan['success'] and all(reader.digest(n) == h for n, h in plan['compiler_sources'].items()), 'Accepted player source drift')
        require(all(reader.digest(n)==h for n,h in plan['runtime_sources'].items()) and all(reader.digest(n)==h for n,h in plan['sources'].items()),'Frozen runtime/controller source drift')
        names.update(plan['compiler_sources'])
        binary = ROOT / plan['player_binary']['path']
        require(sha(binary) == plan['player_binary']['sha256'], 'Accepted player binary differs')
    sources = {n: sha(ROOT / n) for n in sorted(names)}
    result = dict(schema=1, kind='public_native_interaction_acquisition', success=False, sources=sources,
        session_identity=secrets.token_hex(16), configuration=dict(software_flag=args.software, high_resolution=not args.low,
        actual_size=size, skip_intros=False, terrain_lighting='native', compiled_test_driver_invoked=False,
        original_state_samplers=True, viewport_scaling='sharp-and-integer', physical_intro_skip=True, viewport_capture_policy='campaign-lossless-viewport-v1'),
        original_game_runs=0, original_pixels_used_as_native_inputs=False, player_binary=plan['player_binary'],
        compiler_sources=plan['compiler_sources'], temporary_root=str(out), scope=__doc__)
    result.update(repair_of=args.repair_of, repair_reason=args.repair_reason)
    hs = ls = child = journey = lease = None
    before = False
    env = {}
    try:
        subprocess.run([ROOT / 'tools/original-manifest.sh', 'verify'], check=True)
        before = True
        menus.module('run-opengl-game.py').patch_staged_preferences(ROOT / 'working/game-nocd', 'VIDEO', {'IsHighRes': 'FALSE' if args.low else 'TRUE'})
        result['installation_inputs'] = {str(f.relative_to(ROOT / 'working/game-nocd')): dict(sha256=sha(f), bytes=f.stat().st_size)
                                         for f in sorted((ROOT / 'working/game-nocd').rglob('*')) if f.is_file()}
        from rendering_progression import prepare_launch_manifest
        raw = prepare_launch_manifest(ROOT / 'working/game-nocd', ROOT / 'working/manifests/game-nocd.tsv')
        n = args.report.removesuffix('.json') + '-launch-manifest.bin'
        result['launch_manifest'] = dict(record=n, sha256=write_report(ROOT, n, raw))
        host, owner = out / 'host', out / 'owner'
        host.mkdir(); owner.mkdir()
        launcher = menus.module('run-native-owned.py')
        hs, hr, _ = launcher.start_display(host, allow_glx=True)
        ls, lr, cookie = launcher.start_display(owner)
        env = configure({k: v for k, v in os.environ.items() if not k.startswith(('MNM_', 'WINE'))}, ['--software-rendering'] if args.software else [])
        env.update(DISPLAY=hr['display'], XAUTHORITY=hr['authority'], MNM_NATIVE_OUTPUT_OWNER=str(owner / 'owner.json'),
            MNM_LEGACY_DISPLAY=lr['display'], MNM_LEGACY_XCOOKIE=cookie.hex(), MNM_MENU_GAME_SOURCE=str(ROOT / 'working/game-nocd'),
            MNM_MENU_DIRECTPLAY_DX6='1', MNM_CACHE_ADAPTERS='1', MNM_VERIFY_ORIGINALS='0', MNM_VERIFY_ASSETS='0',
            MNM_TERRAIN_LIGHTING='native', QT_QPA_PLATFORM='xcb', QT_LINUX_ACCESSIBILITY_ALWAYS_ON='1', QT_ACCESSIBILITY='1',
            LP_NUM_THREADS='4', QT_SCALE_FACTOR='1', WINEDEBUG='-all', XDG_CONFIG_HOME=str(out / 'config'),
            MNM_RESIDENT_STAGE_PROFILE='1', MNM_RESIDENT_PROFILE_CAPTURE=str(source_capture_path if source_capture_path is not None else out / 'worlds.bin'), MNM_RESIDENT_CAPTURE_SOURCE_CHUNKS='1',
            MNM_ACCEPTANCE_CAPTURE_DIRECTORY=str(out / 'capture'), MNM_ACCEPTANCE_CAPTURE_ID=result['session_identity'],
            MNM_ACCEPTANCE_NORMAL='1', MNM_ACCEPTANCE_CAMPAIGN='1', MNM_ACCEPTANCE_GAMEPLAY='1', MNM_ACCEPTANCE_FRAME_INPUT='1',
            XDG_RUNTIME_DIR=os.environ['XDG_RUNTIME_DIR'], XDG_CACHE_HOME=str(out/'cache'),
            LIBGL_ALWAYS_SOFTWARE='1', __EGL_VENDOR_LIBRARY_FILENAMES='/usr/share/glvnd/egl_vendor.d/50_mesa.json')
        lease, result['prefix_pool'] = borrow(ROOT / 'working/fixtures/prefix-seed', out / 'wineprefix', env)
        env['MNM_OWNED_WINE_PREFIX'] = str(out / 'wineprefix')
        with sqlite3.connect(ROOT / 'working/interaction-attempts.sqlite3') as budget:
            budget.execute('CREATE TABLE IF NOT EXISTS attempts(report TEXT PRIMARY KEY, repair_of TEXT)')
            budget.execute('BEGIN IMMEDIATE')
            old = list(budget.execute('SELECT report,repair_of FROM attempts'))
            with Reader(ROOT) as r:
                cases=[]
                for n,repair in old:
                    data=r.json(n);parent=r.json(repair) if repair else None
                    comparisons=[r.json(path.relative_to(ROOT)) for path in sorted((ROOT/PREFIX).glob(Path(n).name.replace('-acquisition.json','-comparison')+'*.json'))]
                    matching=[c for c in comparisons if c.get('acquisition')==dict(record=n,sha256=r.digest(n))]
                    same_phase=(data.get('player_binary',{}).get('sha256')==plan['player_binary']['sha256'] and all(data.get('sources',{}).get(name)==h for name,h in plan['runtime_sources'].items()))
                    proved=data['success'] and same_phase and any(c.get('success') is True and c.get('kind')=='public_native_interaction_comparison' and c.get('sources_stable') is True and c.get('comparison',{}).get('mismatching_pixels')==0 and c.get('proof',{}).get('verified') is True for c in matching)
                    preparation_fixed=bool(parent and parent['original_game_runs']==0 and data['original_game_runs']==1 and any(v['path'].endswith('/accessibility-preflight.log') and r.read_bytes(v['record'])==b'private accessibility bus ready\n' for v in data['artifacts']))
                    cases.append(dict(report=n,repair_of=repair,result=data,proved=proved,comparison_failed=any(c.get('success') is False for c in matching),preparation_fixed=preparation_fixed))
                result['attempt_admission']=admit(cases,args.repair_of,plan['bounds']['maximum_requests'],plan['bounds']['maximum_original_games'])
            budget.execute('INSERT INTO attempts VALUES(?,?)',(args.report,args.repair_of))
        command = [str(binary), '--repo', str(ROOT), '--renderer', 'opengl', '--live-menus', '--native-commands', '--resident-world', '--native-only']
        if args.software:
            command.append('--software-rendering')
        result['command'] = command
        with (out / 'qt.log').open('xb') as log:
            child = subprocess.Popen(command, cwd=ROOT, env=env, stdout=log, stderr=subprocess.STDOUT, start_new_session=True)
            os.environ.update({k:env[k] for k in ('DISPLAY','XAUTHORITY','XDG_RUNTIME_DIR','XDG_CONFIG_HOME','XDG_CACHE_HOME')})
            # libatspi can abort the process on a bad bus. Exercise it in a
            # disposable client before constructing the controller or Launch.
            with (out/'accessibility-preflight.log').open('xb') as preflight:
                subprocess.run([sys.executable,'-c',"import gi;gi.require_version('Atspi','2.0');from gi.repository import Atspi;Atspi.init();assert Atspi.get_desktop(0) is not None;print('private accessibility bus ready')"],env=env,stdout=preflight,stderr=subprocess.STDOUT,check=True,timeout=20)
            journey = InteractionJourney(child, out, env, size)
            deadline = time.monotonic() + 20
            while time.monotonic() < deadline:
                require(child.poll() is None, 'Qt host failed to open')
                if journey.input.find_window() and any(v['name'] == 'Launch game' for _, v in journey.access.nodes()):
                    break
                time.sleep(.1)
            result['proof'] = journey.run()
            require(result['proof']['preferences'][2] == int(args.low), 'Original High/Low selector differs')
            deadline = time.monotonic() + 30
            while time.monotonic() < deadline:
                try:
                    result['retirement'] = outcome(journey.session)
                    break
                except (ValueError, FileNotFoundError):
                    time.sleep(.1)
            require(result.get('retirement', {}).get('state') == 'completed', 'Original Quit/owner retirement incomplete')
            result['resident'] = json.loads((journey.session / 'resident-world-report.json').read_text())
            result['native_admission'] = json.loads((journey.session / 'native-output-admission.json').read_text())
            child.wait(timeout=10)
            result['exit_code'] = child.returncode
            require(child.returncode == 0 and int(result['resident']['completed']) > 0, 'Native host failed normal completion')
        subprocess.run(['wineserver', '-w'], env=dict(env, WINEPREFIX=str(out / 'wineprefix')), check=True, timeout=10)
        result['private_wine_stopped'] = True
        result['success'] = True
    except Exception as error:
        result['error'] = str(error)
        if journey:
            try: journey.capture('failure-last-observation')
            except Exception as capture_error: result['failure_capture_error'] = str(capture_error)
    finally:
        if journey:
            # Failure may precede the controller's next poll even though the
            # launcher has already admitted and started the original session.
            # Discover from the existing owner journal before counting or
            # retaining artifacts; no accessibility or game interaction here.
            try:journey.discover()
            except Exception as error:
                result.update(success=False,session_discovery_error=str(error))
            result.update(states=journey.states, actions=journey.actions, camera_actions=journey.camera_actions,
                          screenshots=journey.screenshots, session=str(journey.session) if journey.session else None)
            journey.input.close()
            journey.source_frames.close()
        if child and child.poll() is None:
            os.killpg(child.pid, signal.SIGTERM)
            try: child.wait(timeout=12)
            except subprocess.TimeoutExpired: os.killpg(child.pid, signal.SIGKILL); child.wait(timeout=10)
        if lease and not result['success']:
            subprocess.run(['wineserver', '-k'], env=dict(env, WINEPREFIX=str(out / 'wineprefix')), timeout=10)
            subprocess.run(['wineserver', '-w'], env=dict(env, WINEPREFIX=str(out / 'wineprefix')), timeout=10)
            result['forced_private_prefix_stop'] = True
        for server in (hs, ls):
            if server and server.poll() is None:
                server.terminate(); server.wait(timeout=10)
        if lease:
            try:
                from experiment_environment import _idle
                deadline = time.monotonic() + 20
                while True:
                    try: _idle(out / 'wineprefix', 'wineprefix'); break
                    except (ValueError, subprocess.TimeoutExpired):
                        if time.monotonic() >= deadline: raise
                        time.sleep(.2)
                lease.close(); result['prefix_pool']['returned'] = True
            except Exception as error:
                result.update(success=False, prefix_return_error=str(error))
        if before:
            try:
                subprocess.run([ROOT / 'tools/original-manifest.sh', 'verify'], check=True)
                result['original_manifest_verified_before_after'] = True
            except Exception as error:
                result.update(success=False, original_verification_error=str(error))
        if journey and journey.session:
            f = journey.session / 'original-process-result.json'
            result['original_game_runs'] = int(type(json.loads(f.read_text()).get('original_exit_code')) is int) if f.exists() else (1 if journey.states else None)
        result['sources_stable'] = all(sha(ROOT / n) == h for n, h in sources.items())
        result['player_binary_stable'] = sha(binary) == plan['player_binary']['sha256']
        result['success'] &= result['sources_stable'] and result['player_binary_stable']
        candidates = [f for f in out.rglob('*') if f.is_file() and 'wineprefix' not in f.relative_to(out).parts and f.name != 'Xauthority']
        if journey and journey.session:
            candidates += [f for f in journey.session.rglob('*') if f.is_file() and ('game' not in f.relative_to(journey.session).parts
                or f.suffix.lower() in ('.dll', '.cfg', '.sav', '.ini') or f.name.startswith('native-') and f.suffix == '.bin')]
        result['artifacts'] = []
        for f in sorted(set(candidates)):
            limit = 2 * 1024**3 if f == out / 'worlds.bin' else 1024**3 if f.parent == out / 'capture' else 512 * 1024**2
            if f.is_symlink() or f.stat().st_size > limit:
                continue
            n = args.report.removesuffix('.json') + '-raw-' + hashlib.sha256(str(f.relative_to(ROOT)).encode()).hexdigest()[:20] + '.bin'
            raw = f.read_bytes()
            result['artifacts'].append(dict(path=str(f.relative_to(ROOT)), record=n, sha256=write_report(ROOT, n, raw), bytes=len(raw)))
        h = write_report(ROOT, args.report, (json.dumps(result, indent=2) + '\n').encode())
        print(json.dumps(dict(success=result['success'], error=result.get('error'), record=args.report, sha256=h,
                              original_game_runs=result['original_game_runs'], camera_markers=len(result.get('camera_actions', [])))), flush=True)
    return 0 if result['success'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
