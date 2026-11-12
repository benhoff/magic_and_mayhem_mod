#!/usr/bin/env python3
"""Bounded physical Grimoire navigation with original COPY routing observations."""
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import secrets
import signal
import sqlite3
import subprocess
import sys
import time

from coverage_evidence import Reader, write_report
from native_only_contract import configure, outcome
from native_prefix_template import borrow
from rendering_build_binding import native_runtime_source_paths
from rendering_pilot import require

ROOT = Path(__file__).resolve().parents[1]
PREFIX = 'research/runtime/rendering-copy-natural-results-20261108/'
PLAN = PREFIX+'input-plan-01.json'
PARENT = 'research/runtime/rendering-recovery-four-results-20261108/input-plan-01.json'


def module(name):
    spec = importlib.util.spec_from_file_location('copy_'+name.replace('-', '_'), ROOT/'tools'/(name+'.py'))
    value = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(value)
    return value


menus = module('run-rendering-menu-acceptance')
sha = menus.sha


def source_paths(parent):
    return set(parent['compiler_sources']) | native_runtime_source_paths(ROOT) | {
        p.relative_to(ROOT).as_posix() for p in (ROOT/'tools').glob('*.py')}


def fresh_nodes(read, alive, clock=time.monotonic, sleep=time.sleep, reset=lambda:None):
    """Only a destroyed AT-SPI node is retryable; reacquire the entire tree."""
    deadline=clock()+3
    while True:
        require(alive(), 'Host exited during accessibility observation')
        try:return read()
        except Exception as error:
            message=str(error)
            if 'atspi_error' not in message or 'No such object path' not in message or clock()>=deadline:raise
            reset()
            sleep(.025)


def uncache_accessibility(access):
    """Discard the read-only client's cached child graph before reacquisition."""
    desktop=access.a.get_desktop(0)
    desktop.clear_cache()
    for i in range(desktop.get_child_count()):
        app=desktop.get_child_at_index(i)
        if app and app.get_process_id()==access.pid:
            app.set_cache_mask(access.a.Cache(0))
            app.clear_cache()


def optional_accessible_ids(nodes):
    for _,item in nodes:
        if item.get('accessible_id') is None:item['accessible_id']=''
    return nodes


def host_geometry(input):
    import ctypes as c
    root=c.c_ulong();x=c.c_int();y=c.c_int();w=c.c_uint();h=c.c_uint();border=c.c_uint();depth=c.c_uint()
    f=input.x.XGetGeometry
    f.argtypes=[c.c_void_p,c.c_ulong,c.POINTER(c.c_ulong),c.POINTER(c.c_int),c.POINTER(c.c_int),
                c.POINTER(c.c_uint),c.POINTER(c.c_uint),c.POINTER(c.c_uint),c.POINTER(c.c_uint)]
    require(f(input.display,input.window,c.byref(root),c.byref(x),c.byref(y),c.byref(w),c.byref(h),c.byref(border),c.byref(depth)), 'Host geometry unavailable')
    return [root.value,x.value,y.value,w.value,h.value,border.value,depth.value]


def close_after_retirement(child,input):
    # Normal Quit can destroy the window before the host process has exited.
    # Waiting first avoids sending X11 requests to its already retired window.
    try:code=child.wait(timeout=20)
    except subprocess.TimeoutExpired:
        import ctypes as c
        class Error(c.Structure):
            _fields_=[('type',c.c_int),('display',c.c_void_p),('resource',c.c_ulong),('serial',c.c_ulong),
                      ('error',c.c_ubyte),('request',c.c_ubyte),('minor',c.c_ubyte)]
        errors=[]
        callback_type=c.CFUNCTYPE(c.c_int,c.c_void_p,c.POINTER(Error))
        def receive(display,event):
            e=event.contents;errors.append((e.error,e.request,e.resource));return 0
        callback=callback_type(receive);x=input.x
        x.XSetErrorHandler.argtypes=[c.c_void_p];x.XSetErrorHandler.restype=c.c_void_p
        x.XSync.argtypes=[c.c_void_p,c.c_int]
        old=x.XSetErrorHandler(c.cast(callback,c.c_void_p))
        try:input.close_window();x.XSync(input.display,0)
        finally:x.XSetErrorHandler(old)
        require(all(e==(3,25,input.window) for e in errors),'Unexpected host-close X11 error')
        code=child.wait(timeout=20)
    require(code==0,'Native host failed')


class Journey(menus.Journey):
    def __init__(self,*args):
        super().__init__(*args)
        uncache_accessibility(self.access)
        original=self.access.nodes
        self.access.nodes=lambda:optional_accessible_ids(fresh_nodes(original,lambda:self.child.poll() is None,
                                            reset=lambda:uncache_accessibility(self.access)))

    def read(self):
        if not getattr(self,'staging',None):
            log=self.out/'qt.log'
            for line in log.read_text(errors='replace').splitlines() if log.exists() else []:
                if line.startswith('NATIVE_MENU_ASSETS '):
                    data=json.loads(line.removeprefix('NATIVE_MENU_ASSETS '));path=Path(data['root']).parent
                    require(data['before_producer'] is True and path.resolve().is_relative_to(ROOT/'working/experiments/menu-observer'), 'Foreign staged session')
                    self.staging=path
        if getattr(self,'staging',None):
            path=self.staging/'native-terminal-state.json'
            if path.exists():
                terminal=json.loads(path.read_bytes())
                require(not terminal['state']['terminal'], 'Native terminal before requested menu: '+terminal['error'])
        return super().read()

    def click_id(self, name):
        found = [v for _, v in self.access.nodes() if v.get('accessible_id', '').endswith('.'+name)]
        require(len(found) == 1 and found[0]['enabled'], 'Unavailable physical control: '+name)
        self.actions.append(dict(action='click', object=name, host_ns=time.monotonic_ns(), physical=self.input.click(found[0]['rect'])))

    def await_main(self):
        skipped = set()
        end = time.monotonic()+420
        while time.monotonic() < end:
            require(self.child.poll() is None, 'Player exited during startup')
            value = self.read()
            if value and value[0]['ready'] and value[0]['screen'] == 3:
                self.wait(3, 0, timeout=2)
                return
            if self.session:
                path = self.session/'native-media.bin'
                if path.exists():
                    import struct
                    raw = path.read_bytes()
                    if len(raw) == 2048 and raw[:8] == b'MNMMED01':
                        request, ident, operation = struct.unpack_from('<3I', raw, 16)
                        response, status, _, accepted, sequence = struct.unpack_from('<5I', raw, 512)
                        if not request&1 and not sequence&1 and ident and ident == response == accepted and operation == 1 and status == 2 and ident not in skipped:
                            require(len(skipped) < 2, 'Unexpected extra intro request')
                            nodes = [(n,v) for n,v in self.access.nodes() if v.get('accessible_id', '').endswith('.nativeGameViewport')]
                            require(len(nodes) == 1 and nodes[0][0].get_component_iface().grab_focus(), 'Intro viewport focus refused')
                            self.input.activate();self.key('physical-intro-skip', 0xff1b);skipped.add(ident)
            time.sleep(.1)
        raise ValueError('Original initial Main readiness deadline')

    def run(self):
        self.input.activate()
        self.click('Launch game')
        self.await_main()
        main=self.capture('main-ready', 'Quick Battle')
        quit_targets=[v for v in main['nodes'] if v.get('accessible_id','').endswith('.mainMenuAction4')]
        require(len(quit_targets)==1 and quit_targets[0]['enabled'],'Unique initial Main Quit target required')
        geometry=host_geometry(self.input)
        self.click_id('mainMenuAction0')
        self.wait(18, 1)
        self.click_id('regionEntryAuxiliary0')
        self.wait(27, 2)
        self.capture('book-open')
        self.click_id('grimoireTab0')
        # The book rebuilds text accessibility objects during page changes.
        # Use its existing public PageDown/PageUp/Escape controls thereafter;
        # pixel-only screenshots do not claim an accessibility-tree snapshot.
        for ack,symbol in ((3,None),(4,0xff56),(5,0xff56),(6,0xff55)):
            if symbol:self.key('book-page-key',symbol)
            self.wait(27, ack)
            time.sleep(.7)
            from PIL import ImageGrab
            path=self.out/('book-'+str(ack)+'.png');ImageGrab.grab(xdisplay=self.env['DISPLAY']).save(path)
            self.screenshots.append(dict(label='book-'+str(ack),host_ns=time.monotonic_ns(),state_index=len(self.states)-1,
                path=str(path.relative_to(ROOT)),sha256=sha(path),accessibility_observed=False,host_focus=self.input.focus()))
            require(self.input.focus()['host_focused'],'Book lost host focus')
        self.key('book-close-escape',0xff1b);self.wait(18,7)
        self.key('region-cancel-escape',0xff1b);self.wait(3,8)
        require(host_geometry(self.input)==geometry and self.input.focus()['host_focused'], 'Main target geometry/focus changed')
        self.actions.append(dict(action='quit-at-retained-main-target',source_snapshot='main-ready',
            original_ready_state_index=len(self.states)-1,geometry=geometry,host_ns=time.monotonic_ns(),
            physical=self.input.click(quit_targets[0]['rect'])))


def prepare(output, repair_of, player_build):
    with Reader(ROOT) as reader:
        parent = reader.json(PARENT)
        if player_build:
            import shutil
            build=Path(player_build).resolve()
            compiler=module('test-native-campaign').linked_compiler_sources(build,ROOT)
            require(compiler and not set(parent['compiler_sources'])-compiler, 'New player lost linked compiler inputs')
            target=ROOT/'working/build/copy-stretch-player/mnm-qt-shell'
            target.parent.mkdir(parents=True,exist_ok=True)
            if target.exists():require(sha(target)==sha(build/'mnm-qt-shell'), 'Refuse overwriting a tested player')
            else:shutil.copy2(build/'mnm-qt-shell',target)
            record=output.removesuffix('.json')+'-player.bin'
            binary=dict(path=target.relative_to(ROOT).as_posix(),record=record,sha256=write_report(ROOT,record,target.read_bytes()))
            parent['compiler_sources']={n:reader.digest(n) for n in sorted(compiler)}
        else:
            require(all(reader.digest(n) == h for n, h in parent['compiler_sources'].items()), 'Retained player compiler drift')
            binary = parent['player_binary']
            require(sha(ROOT/binary['path']) == binary['sha256'] == reader.digest(binary['record']), 'Retained player differs')
        names = source_paths(parent)
        result = dict(schema=1, success=True, kind='natural_copy_collection_plan',
                      player_binary=binary, compiler_sources=parent['compiler_sources'],
                      sources={n: reader.digest(n) for n in sorted(names)},
                      source_revision=subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=ROOT, text=True).strip(),
                      old_grimoire_scope='Original-graphics UI67 is a static action/identity lead only; no current native-rendering credit is inherited.',
                      parent=dict(record=PARENT, sha256=reader.digest(PARENT)),
                      bounds=dict(maximum_requests=5 if player_build else 3, maximum_charged_original_games=4 if player_build else 2, stop_on_game_failure=True,
                                  case_deadline_seconds=600, retained_artifact_bytes=2*1024**3),
                      configurations=[dict(software=s, size=list(size)) for s in (False, True) for size in ((640, 480), (800, 600))],
                      diagnostic_scope='all', distinct_copy_inputs=False, movies_skipped=False,
                      audio_policy='Existing default native voices/music; match proven public startup profile.',
                      temporary_evidence_staging=str(os.environ.get('MNM_COPY_STAGING_DB', '')),
                      paired_verified_handoff_required_before_commit=True,
                      original_game_runs=0, newly_passed_requirements=0,
                      purpose='Observe actual stretch/shared-backing arguments before adding any pixel collector or fixture. Reuse the existing player; current native adapters are built by the existing launcher.')
        require(repair_of, 'This phase is one reviewed startup repair only')
        old = reader.json(repair_of)
        require(not old['success'], 'Unexpected repair target')
        pool_refusal = (old['original_game_runs'] == 0 and old.get('charged_original_games') == 0
                        and old.get('error') == 'Both environment slots are busy or held; no additional clone was created'
                        and old.get('admitted_original_sessions') == 0 and not old.get('states'))
        cache_repair = (old.get('original_game_runs') == 1 and old.get('error','').startswith('atspi_error: No such object path')
                        and old['plan']['record'] == PREFIX+'input-plan-06.json')
        optional_id_repair = (old.get('original_game_runs') == 1
                              and old.get('error') == "'NoneType' object has no attribute 'endswith'"
                              and old['plan']['record'] == PREFIX+'input-plan-08.json')
        keyboard_repair = (old.get('original_game_runs')==1
                           and old.get('error','').startswith('atspi_error: No such object path')
                           and old['plan']['record']==PREFIX+'input-plan-09.json'
                           and "self.capture('book-'" in old.get('traceback',''))
        if player_build:
            if pool_refusal:
                previous = reader.json(old['plan']['record'])
                require(reader.digest(old['plan']['record']) == old['plan']['sha256']
                        and previous['bounds']['maximum_requests'] == 5,
                        'Only one zero-game pool-refusal retry is admitted')
                result['bounds']['maximum_requests'] = 6
            elif old['original_game_runs'] is None:
                corrected=reader.json(PREFIX+'zero-game-preparation-03.json')
                require(corrected['success'] and corrected['acquisition']==dict(record=repair_of,sha256=reader.digest(repair_of))
                        and corrected['corrected_original_game_runs']==0,'Unreviewed original startup count')
                result['preparation_count_review']=dict(record=PREFIX+'zero-game-preparation-03.json',sha256=reader.digest(PREFIX+'zero-game-preparation-03.json'))
            else:
                require(old['original_game_runs']==1 and (keyboard_repair or optional_id_repair or cache_repair or old['error']=='Unavailable physical control: grimoireNext' and len(old['states'])==4
                        or old['error'].startswith('atspi_error: No such object path') and len(old['states'])>=5),
                        'Current repair requires the retained stretch or destroyed-accessibility-node refusal')
        else:
            require(old['original_game_runs']==1 and not old['states'] and old['error']=='Original ready menu deadline: (3, 0)', 'Unexpected startup repair')
        result['repair_of'] = dict(record=repair_of, sha256=reader.digest(repair_of))
        result['repair_reason'] = ('Retain the failed1-start diagnostic profile without causal attribution. '
            'Use the existing proven native audio/music and movies-enabled public startup profile, including '
            'its420second Main readiness and600second case bounds, instead of introducing legacy audio/skip flags. '
            'Exactly one repair start is allowed; no timeout explanation or acceptance is inferred.')
        if player_build:
            result['repair_reason']='Two charged failures remain counted. First current NCS1 source-prefix implementation run only: actual prior42successful stretch calls provoked fail27/tag41 and cancellation on prefix drain. Four Python wire controls and fresh C++ canvas/decoder controls pass. No diagnostic budget is raised; no original driver/caller policy changes. Stop on failure before expansion.'
            result['reviewed_failures']=[PREFIX+'default640-01-acquisition.json',PREFIX+'default640-02-acquisition.json']
            if old.get('error','').startswith('atspi_error: No such object path'):
                result['reviewed_failures'].append(repair_of)
                result['repair_reason']='Retain three charged failures and the separately proved zero-game preparation refusal. Current prefix processed a source chunk without renderer failure, then a destroyed AT-SPI object aborted the collector after ack4. Reacquire only the stale accessibility tree within3seconds; other errors still fail. One new trial only, with unchanged tested player/native prefix implementation and all-COPY diagnostics. No rendering credit is inherited from the failed acquisition.'
            if cache_repair:
                prior=reader.json(old['plan']['record'])
                require(reader.digest(old['plan']['record'])==old['plan']['sha256'], 'Prior cache repair plan changed')
                result['bounds'].update(maximum_requests=7,maximum_charged_original_games=5)
                result['reviewed_failures']=[*prior['reviewed_failures'],repair_of]
                result['repair_reason']=('One current cache-policy trial after the retained retry still traversed a destroyed AT-SPI node. '
                    'Disable the client application child cache and clear it before retrying the exact stale-object error. '
                    'Preserve four charged failures and both zero-game refusals. Explicit prospective cap: seven requests/five original starts; '
                    'unchanged tested player, NCS1 path, readback budgets and600second deadline. Stop on failure; no expansion.')
            if optional_id_repair:
                prior=reader.json(old['plan']['record'])
                require(reader.digest(old['plan']['record'])==old['plan']['sha256'], 'Prior optional-ID repair plan changed')
                result['bounds'].update(maximum_requests=8,maximum_charged_original_games=6)
                result['reviewed_failures']=[*prior['reviewed_failures'],repair_of]
                result['repair_reason']=('One trial after the cache-disabled collector progressed to page navigation but a null optional accessibility ID '
                    'failed string filtering. Normalize only missing IDs to empty; preserve strict unique/enabled target checks and original acknowledgements. '
                    'Preserve five charged failures and both zero-game refusals. Prospective cap eight managed requests/six actual starts. '
                    'Player, native path, pixels, deadlines and actions are unchanged. Stop on failure; no expansion.')
            if keyboard_repair:
                prior=reader.json(old['plan']['record'])
                require(reader.digest(old['plan']['record'])==old['plan']['sha256'], 'Prior keyboard repair plan changed')
                result['bounds'].update(maximum_requests=9,maximum_charged_original_games=7)
                result['reviewed_failures']=[*prior['reviewed_failures'],repair_of]
                result['repair_reason']=('One trial replacing full accessibility-tree screenshots after page refresh with pixel-only screenshots and '
                    'existing public PageDown/PageUp/Escape physical keys. Exact retained traceback locates the failure in screenshot tree enumeration, '
                    'after real original ack4. Main Quit uses its initial unique enabled rectangle only after original Main readiness and exact unchanged '
                    'X11 window geometry/focus. No guest writes, injected draws or new fixture. Preserve six charged failures, both zero-game managed '
                    'refusals and the separate unadmitted07setup error. Prospective cap nine managed requests/seven actual starts. Stop on failure.')
            if pool_refusal:
                result['reviewed_failures'] = previous['reviewed_failures']
                result['repair_reason'] = ('One retry after the exact retained zero-game pool refusal. '
                    'Preserve all five requests and three charged original failures; the original-start cap remains four. '
                    'No renderer, collector interaction, diagnostic or case deadline changes. '
                    'The pool must grant an exclusive idle lease; held or active foreign loans remain untouched.')
        raw = Path(__file__).read_bytes()
        source_name = output.removesuffix('.json')+'-collector.bin'
        result['collector_source'] = dict(record=source_name, sha256=write_report(ROOT, source_name, raw))
    print(write_report(ROOT, output, (json.dumps(result, indent=2)+'\n').encode()), flush=True)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--prepare', action='store_true')
    parser.add_argument('--plan', default=PLAN)
    parser.add_argument('--repair-of')
    parser.add_argument('--player-build')
    parser.add_argument('--name')
    parser.add_argument('--low', action='store_true')
    parser.add_argument('--software', action='store_true')
    args = parser.parse_args()
    if args.prepare:
        prepare(args.plan, args.repair_of, args.player_build)
        return 0
    require(args.name and all(c.isalnum() or c in '-_' for c in args.name), 'Owned case name required')
    if os.environ.get('MNM_COPY_BUS_OWNED') != '1':
        runtime = ROOT/'working/copy-natural-runtime'/args.name
        runtime.mkdir(parents=True, mode=0o700, exist_ok=False)
        env = dict(os.environ, XDG_RUNTIME_DIR=str(runtime), MNM_COPY_BUS_OWNED='1', ATSPI_DBUS_IMPLEMENTATION='dbus-daemon')
        env.pop('AT_SPI_BUS_ADDRESS', None)
        os.execvpe('dbus-run-session', ['dbus-run-session', '--', sys.executable, __file__, *sys.argv[1:]], env)
    name = PREFIX+args.name+'-acquisition.json'
    require(not (ROOT/name).exists(), 'Fresh acquisition required')
    out = ROOT/'working/copy-natural-runs'/args.name
    out.mkdir(parents=True, exist_ok=False)
    (out/'.retain').touch()
    with Reader(ROOT) as reader:
        plan = reader.json(args.plan)
        require(plan['success'] and all(reader.digest(n) == h for n, h in plan['sources'].items()), 'Frozen collection source drift')
        require(sha(ROOT/plan['player_binary']['path']) == plan['player_binary']['sha256'], 'Player changed')
        plan_ref = dict(record=args.plan, sha256=reader.digest(args.plan))
    size = [640, 480] if args.low else [800, 600]
    require(dict(software=args.software, size=size) in plan['configurations'], 'Unplanned domain')
    budget = sqlite3.connect(ROOT/'working/copy-natural-attempts.sqlite3')
    budget.execute('CREATE TABLE IF NOT EXISTS attempts(name TEXT PRIMARY KEY,charged INTEGER,success INTEGER)')
    with budget:
        budget.execute('BEGIN IMMEDIATE')
        rows = list(budget.execute('SELECT name,charged,success FROM attempts'))
        require(len(rows) < plan['bounds']['maximum_requests'] and sum(r[1] for r in rows) < plan['bounds']['maximum_charged_original_games'],
                'Collection budget exhausted')
        failed = [r for r in rows if r[1] and r[2] != 1]
        expected_failures=plan.get('reviewed_failures',[plan['repair_of']['record']])
        require({r[0] for r in failed}==set(expected_failures), 'Only exactly reviewed failures can precede this repair')
        if any(r[1] and r[2] == 1 for r in rows):
            with Reader(ROOT) as reader:
                first = next(r[0] for r in rows if r[1] and r[2] == 1)
                review = reader.json(first.removesuffix('-acquisition.json')+'-branch-review.json')
                require(review['success'] and review['interesting_branches'] > 0,
                        'No positive branch in scout; expansion is not justified')
        budget.execute('INSERT INTO attempts VALUES(?,0,NULL)', (name,))
    result = dict(schema=1, kind='native_grimoire_copy_branch_acquisition', success=False,
                  session_identity=secrets.token_hex(16), plan=plan_ref, sources=plan['sources'],
                  player_binary=plan['player_binary'], compiler_sources=plan['compiler_sources'],
                  configuration=dict(software=args.software, actual_size=size, skip_movies=False,
                                     copy_scope='all', distinct_inputs=False, compiled_test_driver_invoked=False),
                  original_game_runs=0, charged_original_games=0, newly_passed_requirements=0,
                  temporary_root=str(out), original_pixels_used_as_native_inputs=False)
    hs = ls = child = journey = lease = None
    env = {}
    before = False
    preferences = ROOT/'working/game-nocd/CFG/prefs.cfg'
    initial_preferences = preferences.read_bytes()
    def timeout(signum, frame):
        raise TimeoutError('Frozen600second Grimoire case deadline')
    signal.signal(signal.SIGALRM, timeout)
    signal.alarm(plan['bounds']['case_deadline_seconds'])
    try:
        subprocess.run([ROOT/'tools/original-manifest.sh', 'verify'], check=True)
        before = True
        menus.module('run-opengl-game.py').patch_staged_preferences(ROOT/'working/game-nocd', 'VIDEO', {'IsHighRes':'FALSE' if args.low else 'TRUE'})
        result['installation_inputs'] = {str(f.relative_to(ROOT/'working/game-nocd')):dict(sha256=sha(f), bytes=f.stat().st_size)
                                         for f in sorted((ROOT/'working/game-nocd').rglob('*')) if f.is_file()}
        from rendering_progression import prepare_launch_manifest
        raw = prepare_launch_manifest(ROOT/'working/game-nocd', ROOT/'working/manifests/game-nocd.tsv')
        result['launch_manifest'] = dict(record=PREFIX+args.name+'-launch-manifest.bin', sha256=write_report(ROOT, PREFIX+args.name+'-launch-manifest.bin', raw))
        host, owner = out/'host', out/'owner'
        host.mkdir();owner.mkdir()
        (out/'worlds').mkdir()
        launcher = menus.module('run-native-owned.py')
        hs, hr, _ = launcher.start_display(host, allow_glx=True)
        ls, lr, cookie = launcher.start_display(owner)
        env = configure({k:v for k,v in os.environ.items() if not k.startswith(('MNM_', 'WINE'))}, ['--software-rendering'] if args.software else [])
        env.update(DISPLAY=hr['display'], XAUTHORITY=hr['authority'], MNM_NATIVE_OUTPUT_OWNER=str(owner/'owner.json'),
                   MNM_LEGACY_DISPLAY=lr['display'], MNM_LEGACY_XCOOKIE=cookie.hex(),
                   MNM_MENU_GAME_SOURCE=str(ROOT/'working/game-nocd'), MNM_MENU_DIRECTPLAY_DX6='1',
                   MNM_CACHE_ADAPTERS='1', MNM_VERIFY_ORIGINALS='0', MNM_VERIFY_ASSETS='0',
                   MNM_TERRAIN_LIGHTING='native', QT_QPA_PLATFORM='xcb', QT_LINUX_ACCESSIBILITY_ALWAYS_ON='1',
                   QT_ACCESSIBILITY='1', LP_NUM_THREADS='4', QT_SCALE_FACTOR='1', WINEDEBUG='-all',
                   XDG_CONFIG_HOME=str(out/'config'), XDG_CACHE_HOME=str(out/'cache'),
                   XDG_RUNTIME_DIR=os.environ['XDG_RUNTIME_DIR'], LIBGL_ALWAYS_SOFTWARE='1',
                   __EGL_VENDOR_LIBRARY_FILENAMES='/usr/share/glvnd/egl_vendor.d/50_mesa.json',
                   MNM_EXPERIMENTAL_CAMPAIGN_MENUS='1', MNM_GRIMOIRE_TEST_OBSERVE='1',
                   MNM_NATIVE_COPY_DIAGNOSTICS=str(out/'copy-diagnostics'), MNM_NATIVE_COPY_DIAGNOSTIC_SCOPE='all',
                   MNM_NATIVE_COPY_DISTINCT_INPUTS='0', MNM_ACCEPTANCE_CAPTURE_DIRECTORY=str(out/'capture'),
                   MNM_ACCEPTANCE_CAPTURE_ID=result['session_identity'], MNM_ACCEPTANCE_COMMAND_HISTORY='1',
                   MNM_RESIDENT_STAGE_PROFILE='1', MNM_RESIDENT_PROFILE_CAPTURE_DIRECTORY=str(out/'worlds'),
                   MNM_RESIDENT_CAPTURE_SOURCE_CHUNKS='1')
        prefix = ROOT/'working/copy-natural-prefixes'/args.name
        prefix.parent.mkdir(parents=True, exist_ok=True)
        lease, result['prefix_pool'] = borrow(ROOT/'working/fixtures/prefix-seed', prefix, env)
        env['MNM_OWNED_WINE_PREFIX'] = str(prefix)
        command = [str(ROOT/plan['player_binary']['path']), '--repo', str(ROOT), '--renderer', 'opengl',
                   '--live-menus', '--native-commands', '--resident-world', '--native-only']
        if args.software:
            command.append('--software-rendering')
        result['command'] = command
        with (out/'qt.log').open('xb') as log:
            child = subprocess.Popen(command, cwd=ROOT, env=env, stdout=log, stderr=subprocess.STDOUT, start_new_session=True)
            os.environ.update({k:env[k] for k in ('DISPLAY','XAUTHORITY','XDG_RUNTIME_DIR','XDG_CONFIG_HOME','XDG_CACHE_HOME')})
            subprocess.run([sys.executable, '-c', "import gi;gi.require_version('Atspi','2.0');from gi.repository import Atspi;Atspi.init();assert Atspi.get_desktop(0) is not None"], env=env, check=True, timeout=20)
            journey = Journey(child, out, env)
            end = time.monotonic()+20
            while time.monotonic() < end:
                require(child.poll() is None, 'Host exited before Launch')
                if journey.input.find_window() and any(v['name'] == 'Launch game' for _,v in journey.access.nodes()):
                    break
                time.sleep(.1)
            else:
                raise ValueError('Host Launch readiness deadline')
            with budget:
                budget.execute('UPDATE attempts SET charged=1 WHERE name=?', (name,))
            result['charged_original_games'] = 1
            journey.run()
            end = time.monotonic()+20
            while time.monotonic() < end:
                try:
                    result['retirement'] = outcome(journey.session)
                    break
                except (ValueError, FileNotFoundError):
                    time.sleep(.1)
            require(result.get('retirement', {}).get('state') == 'completed', 'Original Quit did not retire normally')
            close_after_retirement(child,journey.input)
        subprocess.run(['wineserver', '-w'], env=dict(env, WINEPREFIX=str(prefix)), check=True, timeout=10)
        result['private_wine_stopped'] = True
        result['success'] = True
    except Exception as error:
        result['error'] = str(error)
        import traceback
        result['traceback'] = traceback.format_exc()
    finally:
        signal.alarm(0)
        if journey:
            if not result['success']:
                try:
                    journey.capture('failure')
                except Exception as error:
                    result['failure_capture_error'] = str(error)
                if ls and ls.poll() is None:
                    from PIL import ImageGrab
                    old_authority = os.environ.get('XAUTHORITY')
                    try:
                        os.environ['XAUTHORITY'] = lr['authority']
                        ImageGrab.grab(xdisplay=lr['display']).save(out/'failure-private-desktop.png')
                    except Exception as error:
                        result['private_failure_capture_error'] = str(error)
                    finally:
                        if old_authority is None:os.environ.pop('XAUTHORITY', None)
                        else:os.environ['XAUTHORITY'] = old_authority
            result.update(states=journey.states, actions=journey.actions, screenshots=journey.screenshots,
                          session=str(journey.session) if journey.session else None)
            journey.input.close()
        if child and child.poll() is None:
            os.killpg(child.pid, signal.SIGTERM)
            try:
                child.wait(timeout=12)
            except subprocess.TimeoutExpired:
                os.killpg(child.pid, signal.SIGKILL);child.wait(timeout=10)
        if lease:
            try:
                subprocess.run(['wineserver', '-k'], env=dict(env, WINEPREFIX=str(prefix)), timeout=10)
                subprocess.run(['wineserver', '-w'], env=dict(env, WINEPREFIX=str(prefix)), timeout=10)
                from experiment_environment import _idle
                _idle(prefix, 'wineprefix')
                lease.close()
                result['prefix_pool']['returned'] = True
            except Exception as error:
                result.update(success=False, prefix_return_error=str(error))
        for server in (hs, ls):
            if server and server.poll() is None:
                server.terminate();server.wait(timeout=10)
        preferences.write_bytes(initial_preferences)
        if before:
            subprocess.run([ROOT/'tools/original-manifest.sh', 'verify'], check=True)
            result['original_manifest_verified_before_after'] = True
        admissions = list((out/'owner/sessions').glob('*.json'))
        result['admitted_original_sessions'] = len(admissions)
        result['original_game_runs'] = len(admissions) if admissions else (None if result['charged_original_games'] else 0)
        result['sources_stable'] = all(sha(ROOT/n) == h for n,h in plan['sources'].items())
        result['success'] &= result['sources_stable']
        roots = [Path(json.loads(f.read_bytes())['session']) for f in admissions]
        if journey and getattr(journey,'staging',None) and journey.staging not in roots:
            roots.append(journey.staging)
        files = [f for f in out.rglob('*') if f.is_file() and f.name != 'Xauthority']
        for session in roots:
            files += [f for f in session.rglob('*') if f.is_file() and
                      ('game' not in f.relative_to(session).parts or f.suffix.lower() in ('.dll','.cfg','.sav','.ini') or f.name == 'Chaos.exe')]
        files = sorted(set(f for f in files if not f.is_symlink()))
        require(sum(f.stat().st_size for f in files) <= plan['bounds']['retained_artifact_bytes'], 'Frozen artifact retention budget exceeded')
        result['artifacts'] = []
        for f in files:
            require(f.stat().st_size <= 1024**3, 'Artifact exceeds per-file bound')
            record = PREFIX+args.name+'-raw-'+hashlib.sha256(str(f).encode()).hexdigest()[:20]+'.bin'
            raw = f.read_bytes()
            result['artifacts'].append(dict(path=str(f.relative_to(ROOT)), record=record,
                                            sha256=write_report(ROOT, record, raw), bytes=len(raw)))
        digest = write_report(ROOT, name, (json.dumps(result, indent=2)+'\n').encode())
        with budget:
            budget.execute('UPDATE attempts SET success=? WHERE name=?', (int(result['success']), name))
        budget.close()
        print(json.dumps(dict(success=result['success'], error=result.get('error'), sha256=digest,
                              original_game_runs=result['original_game_runs'], record=name)), flush=True)
    return 0 if result['success'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
