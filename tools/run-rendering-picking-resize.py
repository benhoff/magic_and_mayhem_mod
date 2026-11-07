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
from campaign_input_observation import decode as admission_decode
from rendering_picking_resize import targets, ready

ROOT = Path(__file__).resolve().parents[1]
PREFIX = 'research/runtime/rendering-picking-resize-results-20261107/'
PLAN = PREFIX + 'input-plan-01.json'


def main():
    p = argparse.ArgumentParser(add_help=False)
    p.add_argument('--report', required=True)
    p.add_argument('--plan', default=PLAN)
    args, remaining = p.parse_known_args()
    if os.environ.get('MNM_INTERACTION_BUS_OWNED') != '1':
        runtime = ROOT/'working/picking-runtime'/Path(args.report).stem
        runtime.mkdir(parents=True, mode=0o700, exist_ok=False)
        env = dict(os.environ, XDG_RUNTIME_DIR=str(runtime), MNM_INTERACTION_BUS_OWNED='1')
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
    with sqlite3.connect(ROOT/'working/picking-requests.sqlite3') as db:
        db.execute('CREATE TABLE IF NOT EXISTS requests(report TEXT PRIMARY KEY, plan TEXT NOT NULL)')
        for path in completed:
            name=path.relative_to(ROOT).as_posix()
            db.execute('INSERT OR IGNORE INTO requests VALUES (?,?)',(name,'retained-completed-request'))
        known={path.relative_to(ROOT).as_posix() for path in completed}
        base.require(all(name in known for name, in db.execute('SELECT report FROM requests')),
                     'Prior public request has no retained terminal result')
        db.execute('INSERT INTO requests VALUES (?,?)',(args.report,args.plan))
    previous_sources = base.native_runtime_source_paths
    controls = {'tools/run-rendering-picking-resize.py', 'tools/prepare-rendering-picking-resize.py',
                'tools/rendering_picking_resize.py','tools/experiment_environment.py',
                'tools/check_inputs.py','tools/native_output_owner.py','tools/directplay_dx6_launch.py',
                'tools/prepare-menu-observer.py','tools/run-menu-observer.py','tools/owned-menu-game-runner.py',
                'tools/terrain_light_session.py','tools/native_render_config.py','tools/menu_preferences_store.py',
                'tools/run-game.sh'}
    base.native_runtime_source_paths = lambda root: previous_sources(root) | controls
    original_admit=base.admit
    def admit(cases,repair_of,maximum_requests,maximum_games):
        with base.Reader(ROOT) as reader:
            cases=[dict(c,result=counted(reader,c['report'],c['result'])) for c in cases]
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

        def pick(self, name, kind):
            previous_world = int(self.viewport.poll()['world'])
            self.pointer([40, 100])
            self.wait_progress(previous_world, minimum=2)
            state, body = self.ready_target()
            generation = state['frame']['generation']
            source = self.source_frames.frames[generation]
            body, foot = targets(source, state['history'], state['position'])
            target = body if kind == 'object' else foot
            target.update(source_generation=generation, frame_input_row=state['ordinal'])
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
            self.click('Quick Battle')
            self.wait(22)
            self.click('Create Single Player Game')
            self.wait(14)
            self.click('Map')
            self.wait(25)
            lists = [(n,v) for n,v in self.access.nodes() if v['role']=='list']
            base.require(len(lists)==1,'Ambiguous physical map list on World recreation')
            self.input.click(lists[0][1]['rect'])
            self.key('rebuild-map-first', 0xff50)
            selected_map = lists[0][0].get_selection_iface().get_selected_child(0)
            base.require(selected_map is not None and selected_map.get_name()==self.last['maps'][0],
                         'Actual public recreated map selection differs')
            self.key('rebuild-map-confirm', 0xff0d)
            selected = self.wait(14)
            base.require(selected['setup']['map']==1 and selected['setup']['map_name']=='Celtic Forest One (small)',
                         'Recreated Quick World changed map')
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
            proof = super().run()
            return dict(proof, picking_windows=self.picking_windows, display_rebuilds=self.rebuilds,
                        picking_scope=__doc__, allocation_generations_proved=False)

    base.InteractionJourney = PickingJourney
    return base.main()


if __name__ == '__main__':
    raise SystemExit(main())
