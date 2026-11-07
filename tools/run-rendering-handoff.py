#!/usr/bin/env python3
"""Diagnose second Spellbox publication with same-size and low-size controls."""
import argparse
from contextlib import closing
import importlib.util
import json
import os
from pathlib import Path
import sqlite3
import sys
import time
from coverage_evidence import Reader, write_report
from rendering_handoff import CONTROLS, PREFIX, Sampler, observed, rebuilt_frame
from rendering_pilot import require
ROOT = Path(__file__).resolve().parents[1]


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--plan', required=True)
    p.add_argument('--mode', choices=['same-size', 'low-size'], required=True)
    p.add_argument('--report', required=True)
    a = p.parse_args()
    require(a.report.startswith(PREFIX) and '..' not in Path(a.report).parts
            and a.report.endswith('-acquisition.json') and not (ROOT/a.report).exists(), 'Fresh diagnostic report required')
    if os.environ.get('MNM_INTERACTION_BUS_OWNED') != '1':
        runtime = ROOT/'working/handoff-runtime'/Path(a.report).stem
        runtime.mkdir(parents=True, mode=0o700, exist_ok=False)
        env = dict(os.environ, XDG_RUNTIME_DIR=str(runtime), MNM_INTERACTION_BUS_OWNED='1')
        env.pop('AT_SPI_BUS_ADDRESS', None)
        os.execvpe('dbus-run-session', ['dbus-run-session', '--', sys.executable, __file__, *sys.argv[1:]], env)
    with Reader(ROOT) as r:
        plan = r.json(a.plan)
        require(plan['kind'] == 'second_spellbox_diagnostic_plan'
                and plan['bounds']['modes'] == ['same-size', 'low-size']
                and plan['bounds']['maximum_requests'] == 2+len(plan['preparation_refusals'])
                and plan['bounds']['maximum_original_games'] == 2 and len(plan['preparation_refusals']) <= 1,
                'Wrong prospective diagnostic bounds')
        plan_ref = dict(record=a.plan, sha256=r.digest(a.plan))
    budget = ROOT/'working/handoff-requests.sqlite3'
    budget.parent.mkdir(exist_ok=True)
    with closing(sqlite3.connect(budget)) as db, db:
        db.execute('CREATE TABLE IF NOT EXISTS requests(mode TEXT PRIMARY KEY,report TEXT UNIQUE,plan TEXT)')
        db.execute('BEGIN IMMEDIATE')
        used = list(db.execute('SELECT report FROM requests'))
        require(len(used) < plan['bounds']['maximum_requests'], 'Diagnostic request bound exhausted')
        with Reader(ROOT) as reader:
            for (name,) in used:
                previous = reader.json(name)
                require(type(previous['original_game_runs']) is int, 'Unknown prior startup count')
                require(previous['mode'] != a.mode or (previous['original_game_runs'] == 0
                        and dict(record=name, sha256=reader.digest(name)) in plan['preparation_refusals']),
                        'No repeated actual-game mode or unbound preparation retry')
            if plan['preparation_refusals']:
                require(sum(reader.json(n)['original_game_runs'] for (n,) in used) < 2,
                        'Original-game cap exhausted')
        db.execute('INSERT INTO requests VALUES(?,?,?)', (a.mode+':'+a.report, a.report, a.plan))
    spec = importlib.util.spec_from_file_location('handoff_base', ROOT/'tools/run-rendering-interaction.py')
    base = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(base)
    base.PREFIX, base.PLAN = PREFIX, a.plan
    original_sources = base.native_runtime_source_paths
    base.native_runtime_source_paths = lambda root: original_sources(root) | set(CONTROLS)
    # A prior causal diagnostic is neither a nominal pilot nor a repair ancestor.
    def admit(cases, repair_of, maximum_requests, maximum_games):
        require(not repair_of and len(cases) < maximum_requests
                and maximum_requests == plan['bounds']['maximum_requests'] and maximum_games == 2,
                'No diagnostic repair or expanded budget')
        require(all(c['result']['kind'] == 'second_spellbox_diagnostic_acquisition'
                    and type(c['result']['original_game_runs']) is int for c in cases),
                'Prior request has no exact terminal startup count')
        return dict(requests_before=len(cases), original_games_before=sum(c['result']['original_game_runs'] for c in cases),
                    diagnostic_only=True, mode=a.mode)
    base.admit = admit
    original_write = base.write_report
    journeys = []
    def write(root, name, body):
        if name == a.report:
            q = json.loads(body)
            q.update(kind='second_spellbox_diagnostic_acquisition', prospective_plan=plan_ref,
                     diagnostic_only=True, mode=a.mode, nominal_acceptance=False,
                     scope=plan['scope'], original_game_runs=q.get('original_game_runs'))
            q['configuration']['diagnostic_only'] = True
            if journeys:
                q['handoff_observation'] = journeys[0].observation
            body = (json.dumps(q, indent=2)+'\n').encode()
        return original_write(root, name, body)
    base.write_report = write
    class Journey(base.InteractionJourney):
        def __init__(self, *args):
            super().__init__(*args)
            self.observation = {}
            self.sampler = Sampler(self.out/'handoff.sqlite3', lambda: self.session,
                                   self.out/'wineprefix', self.child.pid)
            journeys.append(self)

        def run(self):
            try:
                return super().run()
            finally:
                try:
                    self.sampler.close()
                    self.observation = observed(self.out/'handoff.sqlite3')
                except ValueError as error:
                    # Preserve the actual journey error. Missing/late sampler
                    # data is an independent diagnostic failure, never success.
                    self.observation = dict(sample_count=0, observation_error=str(error))

        def await_main(self):
            original = self.read
            checked_at = 0
            def checked_read():
                nonlocal checked_at
                if time.monotonic()-checked_at >= 1:
                    checked_at = time.monotonic()
                    require(not any('Menu staging failed' in v['name'] for _, v in self.access.nodes()),
                            'Public host reports terminal menu staging refusal')
                return original()
            self.read = checked_read
            try:
                return super().await_main()
            finally:
                self.read = original

        def discover(self):
            found = super().discover()
            if found:
                parent = (ROOT/'working/experiments').resolve()
                if found.is_relative_to(parent):
                    self.session = ROOT/'working/experiments'/found.relative_to(parent)
            return self.session

        def interaction_schedule(self):
            # No camera/picking matrix: first healthy World, then the original
            # failed route, changing only the accepted Preferences resolution.
            self.held_key('diagnostic-first-results', 0xff1b, .3, .25, advance=False)
            self.wait(26)
            self.click_id('quickResultAction2')
            self.wait(22)
            self.key('diagnostic-return-main', 0xff1b)
            self.wait(3)
            self.click('Preferences')
            before = self.wait(10)
            require(before['preferences'][2] == 0, 'Initial World was not High')
            self.click_id('preferencesRadio' + ('2' if a.mode == 'low-size' else '1'))
            self.click_id('preferencesOk')
            self.wait(3)
            self.click('Quick Battle')
            self.wait(22)
            self.click('Create Single Player Game')
            self.wait(14)
            self.click('Map')
            self.wait(25)
            lists = [(n, v) for n, v in self.access.nodes() if v['role'] == 'list']
            require(len(lists) == 1, 'Ambiguous diagnostic map list')
            self.input.click(lists[0][1]['rect'])
            self.key('diagnostic-map-first', 0xff50)
            self.key('diagnostic-map-confirm', 0xff0d)
            selected = self.wait(14)
            require(selected['setup']['map'] == 1, 'Diagnostic recreated map differs')
            self.click_id('singlePlayerStart')
            self.wait(7, timeout=60)
            old_world = int(self.viewport.poll()['world'])
            self.click_id('spellboxOK')
            self.size = [640, 480] if a.mode == 'low-size' else [800, 600]
            self.viewport_geometry = None
            deadline = time.monotonic()+15
            while time.monotonic() < deadline:
                require(self.child.poll() is None, 'Player exited during diagnostic rebuild')
                if rebuilt_frame(self.viewport.poll(), self.size, old_world):
                    return
                time.sleep(.05)
            raise ValueError('No fresh World at the acknowledged diagnostic size')

    base.InteractionJourney = Journey
    sys.argv = [sys.argv[0], '--report', a.report]
    try:
        return base.main()
    except Exception as error:
        # Preparation refusals count too, even when the reused collector has
        # not entered its process-owning try/finally yet.
        if not (ROOT/a.report).exists():
            write(ROOT, a.report, json.dumps(dict(schema=1, success=False, original_game_runs=0,
                  configuration={}, error=str(error), sources={}, artifacts=[])).encode())
        raise


if __name__ == '__main__':
    raise SystemExit(main())
