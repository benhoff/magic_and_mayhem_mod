#!/usr/bin/env python3
"""Finite object/cell picking through Main Preferences and actual 800/640/800 World recreation."""
import argparse
import importlib.util
import json
import os
import sqlite3
from pathlib import Path
import sys
import time
import tempfile
import hashlib
from campaign_input_observation import decode as admission_decode
from rendering_picking_resize import targets, ready

ROOT = Path(__file__).resolve().parents[1]
from picking_closeout import PREFIX, CONTROLS, preserved_pilot, quiet_sources
PLAN = PREFIX + 'input-plan-01.json'


def main():
    p = argparse.ArgumentParser(add_help=False)
    p.add_argument('--report', required=True)
    p.add_argument('--plan', default=PLAN)
    args, remaining = p.parse_known_args()
    if os.environ.get('MNM_INTERACTION_BUS_OWNED') != '1':
        runtime = ROOT/'working/picking-closeout-runtime'/Path(args.report).stem
        runtime.mkdir(parents=True, mode=0o700, exist_ok=False)
        env = dict(os.environ, XDG_RUNTIME_DIR=str(runtime), MNM_INTERACTION_BUS_OWNED='1', ATSPI_DBUS_IMPLEMENTATION='dbus-daemon')
        env.pop('AT_SPI_BUS_ADDRESS', None)
        os.execvpe('dbus-run-session', ['dbus-run-session', '--', sys.executable, __file__, *sys.argv[1:]], env)
    spec = importlib.util.spec_from_file_location('picking_base', ROOT/'tools/run-rendering-interaction.py')
    base = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(base)
    base.require(args.plan.startswith(PREFIX), 'Owned prospective picking plan required')
    base.PREFIX, base.PLAN = PREFIX, args.plan
    sys.argv = [sys.argv[0], '--report', args.report, *remaining]
    def counted(reader, name, result):
        if result['original_game_runs'] is not None:
            return result
        base.require(name==PREFIX+'default-04-acquisition.json','Unknown startup count requires separate retained proof')
        ref=PREFIX+'default-04-launch-count-01.json'
        attribution=reader.json(ref)
        base.require(attribution['success'] and attribution['original_game_runs']==1
                     and attribution['acquisition']==dict(record=name,sha256=reader.digest(name)),
                     'Unbound original startup count attribution')
        return dict(result,original_game_runs=1,derived_game_count=dict(record=ref,sha256=reader.digest(ref)))
    # The reused inner runner records only requests that acquired a prefix.
    # Count earlier preparation refusals too; never lose a failed ancestor.
    with base.Reader(ROOT) as reader, sqlite3.connect(ROOT/'working/interaction-attempts.sqlite3') as db:
        plan = reader.json(args.plan)
        db.execute('CREATE TABLE IF NOT EXISTS attempts(report TEXT PRIMARY KEY, repair_of TEXT)')
        completed = sorted((ROOT/PREFIX).glob('*-acquisition.json'))
        games = 0
        for path in completed:
            name = path.relative_to(ROOT).as_posix()
            previous = counted(reader,name,reader.json(name))
            base.require(type(previous['original_game_runs']) is int and previous['original_game_runs'] in (0,1),
                         'Unknown original game count in prior request')
            games += previous['original_game_runs']
            db.execute('INSERT OR IGNORE INTO attempts VALUES (?,?)',(name,previous.get('repair_of')))
        base.require(len(completed)<plan['bounds']['maximum_requests'] and games<plan['bounds']['maximum_original_games'],
                     'Complete public request/game bound exhausted')
    with sqlite3.connect(ROOT/'working/picking-closeout-requests.sqlite3') as db:
        db.execute('CREATE TABLE IF NOT EXISTS requests(report TEXT PRIMARY KEY, plan TEXT NOT NULL)')
        for path in completed:
            name=path.relative_to(ROOT).as_posix()
            db.execute('INSERT OR IGNORE INTO requests VALUES (?,?)',(name,'retained-completed-request'))
        known={path.relative_to(ROOT).as_posix() for path in completed}
        base.require(all(name in known for name, in db.execute('SELECT report FROM requests')),
                     'Prior public request has no retained terminal result')
        db.execute('INSERT INTO requests VALUES (?,?)',(args.report,args.plan))
    previous_sources = base.native_runtime_source_paths
    controls = set(CONTROLS) | {'tools/run-rendering-picking-resize.py', 'tools/prepare-rendering-picking-resize.py',
                'tools/rendering_picking_resize.py','tools/experiment_environment.py',
                'tools/check_inputs.py','tools/native_output_owner.py','tools/directplay_dx6_launch.py',
                'tools/prepare-menu-observer.py','tools/run-menu-observer.py','tools/owned-menu-game-runner.py',
                'tools/terrain_light_session.py','tools/native_render_config.py','tools/menu_preferences_store.py',
                'tools/run-game.sh'}
    base.native_runtime_source_paths = lambda root: previous_sources(root) | controls | set(plan['runtime_sources'])
    original_admit=base.admit
    def admit(cases,repair_of,maximum_requests,maximum_games):
        with base.Reader(ROOT) as reader:
            cases=[dict(c,result=counted(reader,c['report'],c['result'])) for c in cases]
        review = plan.get('retained_pilot')
        if review:
            with base.Reader(ROOT) as reader:
                proof = reader.json(review['comparison']['record'])
                base.require(reader.digest(review['comparison']['record'])==review['comparison']['sha256'],
                             'Retained pilot comparison changed')
                for case in cases:
                    if case['report']==review['acquisition']['record']:
                        base.require(reader.digest(case['report'])==review['acquisition']['sha256']
                                     and proof['acquisition']==review['acquisition'],
                                     'Retained pilot acquisition changed')
                        case['proved'] = preserved_pilot(case['result']['sources'],
                            plan['runtime_sources'],review['source_changes'],proof)
        return original_admit(cases,repair_of,maximum_requests,maximum_games)
    base.admit=admit
    previous_write = base.write_report
    active_journeys = []

    def write(root, path, body):
        if path == args.report:
            result = json.loads(body)
            result.update(kind='public_native_picking_resize_acquisition', scope=__doc__)
            with base.Reader(root) as reader:
                result['prospective_plan'] = dict(record=args.plan, sha256=reader.digest(args.plan))
            result['configuration'].update(actual_size_schedule=[[800,600],[640,480],[800,600]],
                                           source_only_targets=True, live_original_display_rebuilds=True,
                                           rebuild_route='Quick Results -> Main Preferences -> same-map new Quick World')
            if active_journeys:
                journey = active_journeys[0]
                result['partial_picking'] = dict(windows=journey.picking_windows, rebuilds=journey.rebuilds)
            body = (json.dumps(result, indent=2)+'\n').encode()
        return previous_write(root, path, body)
    base.write_report = write
    ram = Path(tempfile.mkdtemp(prefix='mnm-picking-capture-', dir='/tmp'))
    (ram/'capture').mkdir(); (ram/'.retain').touch()
    previous_configure = base.configure
    def configure(environment, flags):
        configured = previous_configure(environment, flags)
        out = ROOT/'working/interaction-acceptance'/Path(args.report).stem
        base.require(not (out/'capture').exists() and not (out/'worlds.bin').exists(),
                     'Capture targets must be fresh')
        (out/'capture').symlink_to(ram/'capture', target_is_directory=True)
        (out/'worlds.bin').symlink_to(ram/'worlds.bin')
        return configured
    base.configure = configure
    staged_write = base.write_report
    def retain_ram(root, path, raw):
        if str(path) == args.report:
            result = json.loads(raw)
            result['ram_capture_root'] = str(ram)
            out = ROOT/'working/interaction-acceptance'/Path(args.report).stem
            existing = {a['path']:a for a in result.get('artifacts',[])}
            for physical in sorted(ram.rglob('*')):
                if not physical.is_file() or physical.name == '.retain': continue
                relative = physical.relative_to(ram)
                logical = (out/relative).relative_to(ROOT).as_posix()
                base.require(physical.stat().st_size <=
                    (2*1024**3 if relative.as_posix()=='worlds.bin' else 1024**3),
                    'Bounded RAM capture exceeded')
                data = physical.read_bytes(); digest = hashlib.sha256(data).hexdigest()
                if logical in existing:
                    base.require(existing[logical]['sha256']==digest, 'Duplicate RAM capture differs')
                else:
                    record = args.report.removesuffix('.json')+'-raw-'+hashlib.sha256(logical.encode()).hexdigest()[:20]+'.bin'
                    result.setdefault('artifacts',[]).append(dict(path=logical,record=record,
                        sha256=previous_write(ROOT,record,data),bytes=len(data)))
            raw = (json.dumps(result,indent=2)+'\n').encode()
        return staged_write(root,path,raw)
    base.write_report = retain_ram

    class PickingJourney(base.InteractionJourney):
        def __init__(self, *a):
            super().__init__(*a)
            self.picking_windows = []
            self.rebuilds = []
            active_journeys.append(self)

        def admissions(self):
            raw = (self.session/'input-admission.bin').read_bytes()
            return admission_decode(raw[:16+(len(raw)-16)//144*144])['rows']

        def discover(self):
            session=super().discover()
            if session:
                output=(ROOT/'working/experiments').resolve()
                if session.is_relative_to(output):
                    self.session=ROOT/'working/experiments'/session.relative_to(output)
            return self.session

        def await_main(self):
            # Stop promptly on the existing visible terminal launch refusal.
            # Do not press Restart, retry Launch or bypass its original guards.
            previous=self.read
            checked_at=0
            def checked_read():
                nonlocal checked_at
                if time.monotonic()-checked_at>=1:
                    checked_at=time.monotonic()
                    base.require(not any(any(t in v['name'] for t in ('Menu staging failed','Resident World producer cancelled'))
                                         for _,v in self.access.nodes()),
                                 'Public host reports a terminal startup refusal')
                return previous()
            self.read=checked_read
            try:
                return super().await_main()
            finally:
                self.read=previous

        def set_scaling(self, mode):
            # Toolbar changes need widget focus, not a gameplay scene click.
            wanted={'sharp':0,'integer':2}[mode];live=self.viewport.poll()
            if self.scaling_geometry is None:
                nodes=[v for _,v in self.access.nodes() if v.get('accessible_id','').endswith('.viewportScaling')]
                base.require(len(nodes)==1 and nodes[0]['enabled'],'Public scaling unavailable')
                origin=self.window_origin();r=nodes[0]['rect']
                self.scaling_geometry=[r[0]-origin[0],r[1]-origin[1],r[2],r[3]]
            if not live or live['scaling']!=wanted:
                self.input.click(self.screen_geometry(self.scaling_geometry))
                self.key('scaling-'+mode,0xff50 if mode=='sharp' else 0xff57)
                self.key('scaling-confirm',0xff0d)
            if live:
                deadline=time.monotonic()+5
                while time.monotonic()<deadline and self.viewport.poll()['scaling']!=wanted:time.sleep(.05)
                base.require(self.viewport.poll()['scaling']==wanted,'Public scaling was not applied')
                self.focus_viewport()
            self.actions.append(dict(action='scaling',mode=mode,scene_buttons_sent=False,
                readiness='Actual original motion/World-return witnesses required at each pick',host_ns=time.monotonic_ns()))

        def pick(self, name, kind):
            previous_world = int(self.viewport.poll()['world'])
            self.pointer([40, 100])
            self.wait_progress(previous_world, minimum=2)
            samples=[];deadline=time.monotonic()+8;target=None
            while time.monotonic()<deadline:
                state,body=self.ready_target();generation=state['frame']['generation']
                source=self.source_frames.frames[generation]
                body,foot=targets(source,state['history'],state['position'])
                candidate=body if kind=='object' else foot
                sample=dict(generation=generation,nonce=state['frame']['nonce'],
                    position=state['position'],anchor=state['history'][:2],
                    point=candidate['point'],view=state['camera'][0])
                if not samples or samples[-1]['generation']!=generation:samples=(samples+[sample])[-3:]
                if quiet_sources(samples):
                    target=candidate
                    target.update(source_generation=generation,frame_input_row=state['ordinal'],
                        quiet_source_generations=[s['generation']for s in samples])
                    break
                time.sleep(.04)
            base.require(target is not None,'No quiet source-only target before physical action: '+name)
            self.actions.append(dict(action='picking-source-target', name=name, kind=kind, **target))
            baseline = self.latest()
            begin = self.mark(name, 'begin')
            self.pointer(target['point'])
            released = self.mark(name, 'released')
            started = time.monotonic()
            seen = {}
            while time.monotonic()-started <= 8:
                base.require(self.child.poll() is None, 'Player exited during picking readiness')
                raw = (self.session/'frame-input.bin').read_bytes()
                rows = base.decode(raw[:16+(len(raw)-16)//384*384], allow_pending=True)['rows']
                fact = ready(rows, self.admissions(), self.viewport.poll(), target, baseline, kind)
                if fact:
                    seen[fact['generation']] = fact
                if len(seen) >= 3 and time.monotonic()-started >= 1.6:
                    settled = self.mark(name, 'settled')
                    self.picking_windows.append(dict(name=name, kind=kind, size=list(self.size),
                        scaling=self.viewport.poll()['scaling'], target=target,
                        begin=begin, released=released, baseline_row=baseline['ordinal'], settled=settled,
                        witnesses=[seen[g] for g in sorted(seen)[-3:]]))
                    return
                time.sleep(.04)
            raise ValueError('Three displayed exact original object/cell picks missing: '+name)

        def phase(self, label, complete):
            for mode in ('sharp', 'integer'):
                self.set_scaling(mode)
                if self.latest()['mode'] == 1:
                    self.held_key(label+'-'+mode+'-resume', 0x20, .22, .6)
                for n in range(2):
                    self.held_key(label+'-'+mode+'-center-'+str(n), ord('w'), .22, .6)
                for view in range(4 if complete else 1):
                    for kind in ('object', 'cell'):
                        self.pick(label+'-'+mode+'-view-'+str(view)+'-'+kind, kind)
                    if complete:
                        self.held_key(label+'-'+mode+'-rotate-'+str(view), ord('.'), .22, .65)
                self.held_key(label+'-'+mode+'-pan', 0xff52, .22, .8)
                for kind in ('object', 'cell'):
                    self.pick(label+'-'+mode+'-panned-'+kind, kind)

        def rebuild(self, low):
            before = self.latest()
            old_size = list(self.size)
            old_world = int(self.viewport.poll()['world'])
            # Quick context Escape ends the battle (unlike Campaign Mini).
            # Use the already supported Main Preferences path; do not invent
            # same-World continuity across recreation of the original World.
            self.held_key('open-rebuild-results-'+str(int(low)), 0xff1b, .3, .25, advance=False)
            results = self.wait(26)
            self.click_id('quickResultAction2')
            self.wait(22)
            self.key('rebuild-quick-return', 0xff1b)
            self.wait(3)
            self.click('Preferences')
            preferences = self.wait(10)
            base.require(preferences['preferences'][2] == int(not low), 'Original initial resolution differs')
            self.click_id('preferencesRadio'+str(2 if low else 1))
            self.capture('draft-resolution-'+str(640 if low else 800))
            self.click_id('preferencesOk')
            returned = self.wait(3)
            # Use the already-selected declared map, as admitted by VC08.
            self.capture('main-before-recreated-quick', 'Quick Battle')
            self.click('Quick Battle'); self.wait(22)
            self.click('Create Single Player Game'); selected = self.wait(14)
            base.require(selected['setup']['map']==1 and
                         selected['setup']['map_name']=='Celtic Forest One (small)',
                         'Recreated Quick World changed its declared map')
            self.click_id('singlePlayerStart')
            self.wait(7, timeout=60)
            self.click_id('spellboxOK')
            self.size = [640,480] if low else [800,600]
            self.viewport_geometry = None
            deadline = time.monotonic()+30
            paint = None
            while time.monotonic()<deadline:
                base.require(self.child.poll() is None, 'Player exited during frame rebuild')
                p = self.viewport.poll()
                if p and [p['frame_width'],p['frame_height']] == self.size and int(p['world']) >= old_world+5:
                    paint = p
                    break
                time.sleep(.05)
            base.require(paint is not None, 'No fresh frame at acknowledged rebuilt size')
            after = self.latest()
            base.require(after['high_resolution'] == int(not low), 'Actual original high-resolution flag differs')
            self.rebuilds.append(dict(before=old_size, after=list(self.size), original_before=before,
                original_after=after, preferences=preferences, results=results, returned=returned,
                selected_map=selected['setup'], same_original_process=True, same_world=False,
                confirmed_paint=paint, host_ns=time.monotonic_ns()))

        def interaction_schedule(self):
            self.phase('high-initial', True)
            self.rebuild(True)
            self.phase('low', True)
            self.rebuild(False)
            self.phase('high-returned', False)

        def run(self):
            # Keep initial public setup identical to the completed VC08 route.
            self.input.activate(); self.set_scaling('sharp')
            self.click('Launch game'); self.await_main()
            self.capture('main-ready-initial', 'Quick Battle')
            self.click('Preferences'); preferences=self.wait(10)['preferences']
            self.key('preferences-cancel',0xff1b); self.wait(3)
            self.click('Quick Battle'); self.wait(22)
            self.click('Create Single Player Game'); selected=self.wait(14)
            base.require(selected['setup']['map']==1 and
                         selected['setup']['map_name']=='Celtic Forest One (small)',
                         'Initial Quick World changed its declared map')
            self.click_id('singlePlayerStart'); self.wait(7,timeout=60)
            self.click_id('spellboxOK'); self.wait_progress()
            self.interaction_schedule()
            self.held_key('open-results',0xff1b,.3,.2,advance=False)
            self.wait(26); self.click_id('quickResultAction2'); self.wait(22)
            self.key('quick-return',0xff1b); self.wait(3)
            self.capture('main-ready-returned','Quick Battle'); self.click('Quit')
            return dict(preferences=preferences,map=selected['setup'],
                        camera_actions=self.camera_actions,
                        picking_windows=self.picking_windows,display_rebuilds=self.rebuilds,
                        picking_scope=__doc__,allocation_generations_proved=False)

    base.InteractionJourney = PickingJourney
    return base.main(source_capture_path=ram/'worlds.bin')


if __name__ == '__main__':
    raise SystemExit(main())
