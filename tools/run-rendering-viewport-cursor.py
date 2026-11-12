#!/usr/bin/env python3
"""Physical edges and cursor delivery across actual 800/640/800 World recreation."""
import argparse
import importlib.util
import json
import os
from pathlib import Path
import sqlite3
import sys
import time
from coverage_evidence import Reader, write_report
from rendering_pilot import require
from viewport_cursor import PREFIX, CONTROLS, SIZES, cursor_ready, edge_points

ROOT = Path(__file__).resolve().parents[1]


def main():
    p = argparse.ArgumentParser(add_help=False)
    p.add_argument('--report', required=True)
    p.add_argument('--plan', required=True)
    p.add_argument('--diagnose-rebuild', action='store_true')
    args, remaining = p.parse_known_args()
    require(args.report.startswith(PREFIX) and args.report.endswith('-acquisition.json')
            and not (ROOT/args.report).exists(), 'Fresh owned acquisition required')
    if os.environ.get('MNM_INTERACTION_BUS_OWNED') != '1':
        runtime = ROOT/'working/viewport-cursor-runtime'/Path(args.report).stem
        runtime.mkdir(parents=True, mode=0o700, exist_ok=False)
        env = dict(os.environ, XDG_RUNTIME_DIR=str(runtime), MNM_INTERACTION_BUS_OWNED='1',
                   ATSPI_DBUS_IMPLEMENTATION='dbus-daemon')
        env.pop('AT_SPI_BUS_ADDRESS', None)
        os.execvpe('dbus-run-session', ['dbus-run-session', '--', sys.executable,
                                     __file__, *sys.argv[1:]], env)
    spec = importlib.util.spec_from_file_location('viewport_cursor_base', ROOT/'tools/run-rendering-interaction.py')
    base = importlib.util.module_from_spec(spec); spec.loader.exec_module(base)
    base.PREFIX, base.PLAN = PREFIX, args.plan
    paths = base.native_runtime_source_paths
    base.native_runtime_source_paths = lambda root: paths(root) | set(CONTROLS)
    configure=base.configure
    def configured(environment,flags):
        result=configure(environment,flags)
        if args.diagnose_rebuild:
            out=ROOT/'working/interaction-acceptance'/Path(args.report).stem
            result.update(MNM_NATIVE_COPY_DIAGNOSTICS=str(out/'copy-diagnostics'),
                MNM_NATIVE_COPY_DIAGNOSTIC_SCOPE='all',MNM_NATIVE_COPY_DISTINCT_INPUTS='1',
                MNM_NATIVE_COPY_CALLERS='1',MNM_NATIVE_CURSOR_COPY_CALLERS='1',
                MNM_CURSOR_COPY_STATE='1')
        return result
    base.configure=configured
    with Reader(ROOT) as r:
        plan = r.json(args.plan)
        require(plan.get('diagnose_rebuild',False) is args.diagnose_rebuild,
                'Diagnostic mode differs from the prospective plan')
        prior = [(path.relative_to(ROOT).as_posix(), r.json(path.relative_to(ROOT)))
                 for path in sorted((ROOT/PREFIX).glob('*-acquisition.json'))]
        require(len(prior) < plan['bounds']['maximum_requests'] and
                sum(v['original_game_runs'] for _, v in prior) < plan['bounds']['maximum_original_games'],
                'Prospective viewport/cursor budget exhausted')
    database = ROOT/'working/viewport-cursor-requests.sqlite3'
    with sqlite3.connect(database) as db, db:
        db.execute('CREATE TABLE IF NOT EXISTS requests(report TEXT PRIMARY KEY,plan TEXT NOT NULL)')
        known = {n for n, in db.execute('SELECT report FROM requests')}
        require(known <= {n for n, _ in prior}, 'Unresolved prior public request')
        db.execute('INSERT INTO requests VALUES(?,?)', (args.report, args.plan))
    original_admit = base.admit
    def admit(cases, repair, requests, games):
        known = {c['report'] for c in cases}
        for name, result in prior:
            if name not in known:
                cases.append(dict(report=name, repair_of=result.get('repair_of'), result=result,
                                  proved=False, comparison_failed=False, preparation_fixed=False))
        return original_admit(cases, repair, requests, games)
    base.admit = admit
    journeys = []
    original_writer = base.write_report
    def writer(root, path, raw):
        if path == args.report:
            q = json.loads(raw)
            q.update(kind='viewport_cursor_acquisition', prospective_plan=args.plan)
            if q.get('session') or q.get('states'):
                q['original_game_runs'] = 1
            q['configuration'].update(actual_size_schedule=SIZES, same_original_process=True,
                                      same_world=False, physical_actions_only=True,
                                      diagnostic_only=args.diagnose_rebuild)
            if journeys:
                j = journeys[0]
                q['viewport_cursor'] = dict(windows=j.windows, rebuilds=j.rebuilds,
                                            camera_limits=j.camera_limits, menu_moves=j.menu_moves)
            raw = (json.dumps(q, indent=2)+'\n').encode()
        return original_writer(root, path, raw)
    base.write_report = writer

    class Journey(base.InteractionJourney):
        def __init__(self, *a):
            super().__init__(*a)
            self.windows, self.rebuilds, self.camera_limits, self.menu_moves = [], [], [], []
            journeys.append(self)

        def move(self, action, point):
            baseline = self.latest()
            begin = self.mark(action, 'begin'); self.pointer(point)
            released = self.mark(action, 'released'); seen = {}; start = time.monotonic()
            while time.monotonic()-start < 10:
                require(self.child.poll() is None, 'Player exited during viewport/cursor observation')
                raw = (self.session/'frame-input.bin').read_bytes()
                raw = raw[:16+(len(raw)-16)//384*384]
                fact = cursor_ready(base.decode(raw, allow_pending=True)['rows'],
                                    self.viewport.poll(), point, baseline)
                if fact: seen[fact['generation']] = fact
                if len(seen) >= 3:
                    settled = self.mark(action, 'settled')
                    self.windows.append(dict(name=action, size=list(self.size), begin=begin,
                        released=released, settled=settled, baseline_row=baseline['ordinal'],
                        witnesses=[seen[g] for g in sorted(seen)[-3:]], point=point))
                    return
                time.sleep(.04)
            raise ValueError('Three fresh displayed original cursor polls missing: '+action)

        def phase(self, label, limits):
            center = edge_points(self.size)[0][1]
            for mode in ('sharp', 'integer'):
                self.set_scaling(mode)
                if self.latest()['mode'] == 1:
                    self.held_key(label+'-'+mode+'-resume', 0x20, .22, .5)
                for name, point in edge_points(self.size):
                    self.move(label+'-'+mode+'-edge-'+name, point)
                    if name != 'center': self.move(label+'-'+mode+'-return-'+name, center)
                self.held_key(label+'-'+mode+'-center', ord('w'), .22, .5)
            if limits:
                self.pointer(center)
                for name, symbol in [('up', 0xff52), ('right', 0xff53), ('down', 0xff54), ('left', 0xff51)]:
                    before = self.latest()
                    self.held_key(label+'-limit-'+name, symbol, 2.0, .5)
                    after = self.latest()
                    self.camera_limits.append(dict(name=label+'-'+name, size=list(self.size),
                                                   before=before, after=after))
                    self.held_key(label+'-recenter-'+name, ord('w'), .22, .5)

        def menu_motion(self, label):
            _,node = self.access.find('Quick Battle')
            rect = node['rect']; self.input.click(rect)
            self.menu_moves.append(dict(label=label, rect=rect, host_ns=time.monotonic_ns(),
                                        focus=self.input.focus(), screen=self.last['screen']))
            self.wait(22)

        def rebuild(self, low):
            before = self.latest(); old_size = list(self.size)
            old_world = int(self.viewport.poll()['world'])
            self.held_key('exit-for-rebuild-'+str(low), 0xff1b, .3, .2, advance=False)
            self.wait(26); self.click_id('quickResultAction2'); self.wait(22)
            self.key('rebuild-return-main', 0xff1b); self.wait(3)
            self.capture('main-before-rebuild-'+str(low), 'Quick Battle')
            self.click('Preferences'); preferences = self.wait(10)
            require(preferences['preferences'][2] == int(not low), 'Original pre-rebuild selector differs')
            self.click_id('preferencesRadio'+str(2 if low else 1))
            self.click_id('preferencesOk'); returned = self.wait(3)
            self.menu_motion('main-after-rebuild-'+str(low))
            self.click('Create Single Player Game'); selected = self.wait(14)
            require(selected['setup']['map'] == 1
                    and selected['setup']['map_name'] == 'Celtic Forest One (small)',
                    'Original recreated setup must already select the declared map')
            self.actions.append(dict(action='original-selected-map-confirmed',
                setup=selected['setup'],host_ns=time.monotonic_ns(),map_picker_opened=False))
            self.click_id('singlePlayerStart'); self.wait(7, timeout=60)
            self.click_id('spellboxOK')
            self.size = [640,480] if low else [800,600]
            self.viewport_geometry = None
            start = time.monotonic(); paint = None
            while time.monotonic()-start < 30:
                require(self.child.poll() is None, 'Player exited during display rebuild')
                candidate = self.viewport.poll()
                if (candidate and [candidate['frame_width'], candidate['frame_height']] == self.size
                        and int(candidate['world']) >= old_world+5):
                    paint = candidate; break
                time.sleep(.05)
            require(paint, 'No fresh World at acknowledged rebuilt size')
            after = self.latest()
            require(after['high_resolution'] == int(not low), 'Original cursor resolution differs after rebuild')
            self.rebuilds.append(dict(before=old_size, after=list(self.size), original_before=before,
                original_after=after, preferences=preferences, returned=returned,
                confirmed_paint=paint, same_original_process=True, same_world=False,
                host_ns=time.monotonic_ns()))

        def interaction_schedule(self):
            if args.diagnose_rebuild:
                self.move('diagnostic-high-center',edge_points(self.size)[0][1])
                self.rebuild(True)
                self.move('diagnostic-low-center',edge_points(self.size)[0][1])
                return
            self.phase('high-initial', True)
            self.rebuild(True); self.phase('low', True)
            self.rebuild(False); self.phase('high-returned', False)

        def run(self):
            self.input.activate();self.set_scaling('sharp');self.click('Launch game');self.await_main()
            self.capture('main-ready-initial','Quick Battle')
            self.click('Preferences');preferences=self.wait(10)['preferences']
            self.key('preferences-cancel',0xff1b);self.wait(3)
            self.click('Quick Battle');self.wait(22)
            self.click('Create Single Player Game');selected=self.wait(14)
            require(selected['setup']['map']==1 and
                    selected['setup']['map_name']=='Celtic Forest One (small)',
                    'Original initial setup must already select the declared map')
            self.actions.append(dict(action='original-selected-map-confirmed',
                setup=selected['setup'],host_ns=time.monotonic_ns(),map_picker_opened=False))
            self.click_id('singlePlayerStart');self.wait(7,timeout=60)
            self.click_id('spellboxOK');self.wait_progress()
            self.interaction_schedule()
            self.held_key('open-results',0xff1b,.3,.2,advance=False)
            self.wait(26);self.click_id('quickResultAction2');self.wait(22)
            self.key('quick-return',0xff1b);self.wait(3)
            self.capture('main-ready-returned','Quick Battle');self.click('Quit')
            return dict(preferences=preferences,map=selected['setup'],camera_actions=self.camera_actions)

    base.InteractionJourney = Journey
    sys.argv = [sys.argv[0], '--report', args.report, *remaining]
    return base.main()


if __name__ == '__main__': raise SystemExit(main())
