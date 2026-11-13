#!/usr/bin/env python3
"""Evidence admission controls plus optional actual native effect stress checks."""
import argparse
from contextlib import closing
import copy
import importlib.util
import json
import os
from pathlib import Path
import sqlite3
import subprocess
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('effect_stress', ROOT / 'tools/effect-stress.py')
stress = importlib.util.module_from_spec(spec); spec.loader.exec_module(stress)
OPTIONS = None


def fixture():
    rows = []
    for repeat in range(2):
        for frame, elapsed in enumerate([1000., 1., 49., 51.]):
            rows.append(dict(repeat=repeat, frame=frame, warmup=frame == 0,
                             input_sha256=f'{frame:064x}', pixel_sha256=f'{frame + 4:064x}',
                             expected_sha256=f'{frame + 4:064x}', pixels_equal=True,
                             submit_ms=elapsed / 2, completion_ms=elapsed,
                             counters=dict(draw_calls=8, snapshot_pixels=64, canvas_clones=2, texture_upload_bytes=0)))
    native = dict(success=True, case='wave', level=1, definition={'seed': 1}, renderer='authored',
                  repeats=2, frames=3, warmup=1, ordinary_readbacks=0, repeat_pixels_equal=True, samples=rows)
    return dict(success=True, native=native, summary=stress.summarize(native, 50))


class AdmissionTests(unittest.TestCase):
    def test_warmup_is_retained_but_excluded_from_performance(self):
        report = fixture(); s = report['summary']
        self.assertEqual(s['completion_ms']['samples'], 6)
        self.assertEqual(s['completion_ms']['maximum'], 51)
        self.assertEqual(s['completion_ms']['p95'], 51)
        self.assertEqual(s['over_budget_frames'], 2)
        self.assertFalse(s['p95_within_budget'])
        self.assertEqual(len(report['native']['samples']), 8)

    def test_refuses_missing_frames_pixels_readbacks_and_frozen_inputs(self):
        valid = fixture()['native']
        mutations = [lambda r: r['samples'].pop(),
                     lambda r: r['samples'].reverse(),
                     lambda r: r.update(ordinary_readbacks=1),
                     lambda r: r.update(repeat_pixels_equal=False),
                     lambda r: r['samples'][1].update(warmup=True),
                     lambda r: r['samples'][1].update(pixels_equal=False),
                     lambda r: r['samples'][1].update(pixel_sha256='f' * 64),
                     lambda r: r['samples'][1].update(completion_ms=0),
                     lambda r: r['samples'][-1].update(input_sha256='f' * 64),
                     lambda r: [row.update(input_sha256='0' * 64) for row in r['samples']]]
        for mutate in mutations:
            candidate = copy.deepcopy(valid); mutate(candidate)
            with self.assertRaises(ValueError):
                stress.summarize(candidate, 50)

    def test_baseline_requires_identical_input_and_pixels(self):
        before = fixture(); after = copy.deepcopy(before)
        self.assertTrue(stress.compare(before, after)['pixels_equal'])
        after['native']['renderer'] = 'different driver'
        self.assertFalse(stress.compare(before, after)['same_driver'])
        for field in ('input_sha256', 'pixel_sha256'):
            after = copy.deepcopy(before); after['native']['samples'][0][field] = 'f' * 64
            with self.assertRaises(ValueError):
                stress.compare(before, after)
        after = copy.deepcopy(before); after['native']['definition']['seed'] = 2
        with self.assertRaises(ValueError):
            stress.compare(before, after)


class NativeTests(unittest.TestCase):
    def setUp(self):
        if not OPTIONS.renderer:
            self.skipTest('Pass --renderer for actual native controls')
        self.temp = tempfile.TemporaryDirectory(); self.addCleanup(self.temp.cleanup)
        self.directory = Path(self.temp.name)
        self.results = (OPTIONS.output / 'results.sqlite3') if OPTIONS.output else self.directory / 'results.sqlite3'

    def run_cli(self, extra=()):
        command = [sys.executable, str(ROOT / 'tools/effect-stress.py'), 'run', '--levels', '1',
                   '--frames', '2', '--repeat', '2', '--warmup', '1', '--executable', str(OPTIONS.renderer),
                   '--backend', 'hardware' if OPTIONS.hardware else 'software', '--results', str(self.results), *extra]
        result = subprocess.run(command, capture_output=True, text=True, timeout=180)
        lines = [json.loads(line) for line in result.stdout.splitlines()]
        self.assertTrue(lines, result.stderr)
        return result, lines[-1]

    def test_describe_and_invalid_domains_do_not_start_a_renderer(self):
        result = subprocess.run([str(OPTIONS.renderer), '--describe'], capture_output=True, text=True)
        r = json.loads(result.stdout); self.assertTrue(r['success']); self.assertEqual(len(r['cases']), 12)
        self.assertEqual({x['case'] for x in r['cases']}, set(stress.CASES))
        for args in [('wave', '4', '1', '1', '0', '0'), ('wave', '1', '120', '5', '8', '0'), ('unknown', '1', '1', '1', '0', '0')]:
            env = dict(os.environ, QT_QPA_PLATFORM='unavailable-for-test')
            p = subprocess.run([str(OPTIONS.renderer), *args], env=env, capture_output=True, text=True)
            self.assertEqual(p.returncode, 1); self.assertFalse(json.loads(p.stdout)['success'])

    def test_all_families_move_and_repeat_with_budget_observations(self):
        result, batch = self.run_cli()
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr); self.assertTrue(batch['success'])
        with closing(sqlite3.connect(self.results)) as db:
            for ident in batch['run_ids']:
                r = json.loads(db.execute('SELECT payload FROM runs WHERE id=?', (ident,)).fetchone()[0])
                native = r['native']; self.assertEqual(native['ordinary_readbacks'], 0)
                self.assertEqual(len(native['samples']), 6)
                self.assertEqual(len({x['input_sha256'] for x in native['samples']}), 3)
                self.assertTrue(all(x['pixels_equal'] for x in native['samples']))
            self.assertGreater(db.execute('SELECT count(*) FROM inputs').fetchone()[0], 10)
        again, compared = self.run_cli(('--against', str(batch['batch_id'])))
        self.assertEqual(again.returncode, 0, again.stdout + again.stderr); self.assertTrue(compared['success'])

    def test_keyed_capacity_and_outer_batch_semantics(self):
        command = [str(OPTIONS.renderer), 'guards']; env = dict(os.environ, QT_QPA_PLATFORM='xcb')
        if not OPTIONS.hardware:
            env.update(LIBGL_ALWAYS_SOFTWARE='1', GALLIUM_DRIVER='llvmpipe'); command = ['xvfb-run', '-a', *command]
        result = subprocess.run(command, env=env, capture_output=True, text=True, timeout=30)
        native = json.loads(result.stdout); self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertEqual(native['keyed_pixel_cap'], 4096); self.assertTrue(native['oversize_refused_without_mutation'])
        self.assertTrue(native['unbatched_keyed_pixels_match']); self.assertTrue(native['batched_keyed_pixels_match'])
        with closing(stress.connect(self.results)) as db, db:
            db.execute('CREATE TABLE IF NOT EXISTS controls(kind TEXT,payload TEXT)')
            db.execute('INSERT INTO controls VALUES(?,?)', ('keyed-guard', json.dumps(dict(command=command,returncode=result.returncode,stdout=result.stdout,stderr=result.stderr))))

    def test_corrupt_baseline_pixels_are_a_recorded_failure(self):
        result, batch = self.run_cli(('--case', 'keyed-overlap'))
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        for mutation, message in [('pixels', 'Baseline pixels differ'), ('palette', 'Baseline effect definition differs')]:
            damaged_path = (OPTIONS.output or self.directory) / f'bad-baseline-{batch["batch_id"]}-{mutation}.sqlite3'
            self.assertFalse(damaged_path.exists())
            with closing(sqlite3.connect(self.results)) as source, closing(sqlite3.connect(damaged_path)) as db:
                source.backup(db)
                ident = batch['run_ids'][0]
                original = db.execute('SELECT payload FROM runs WHERE id=?', (ident,)).fetchone()[0]
                damaged = json.loads(original)
                if mutation == 'pixels':
                    damaged['native']['samples'][0]['pixel_sha256'] = '0' * 64
                else:
                    self.assertEqual(damaged['native']['definition']['input_schema'], 2)
                    self.assertEqual(len(damaged['native']['definition']['palette_sha256']), 64)
                    damaged['native']['definition']['palette_sha256'] = '0' * 64
                db.execute('UPDATE runs SET payload=? WHERE id=?', (json.dumps(damaged), ident)); db.commit()
            result, failed = self.run_cli(('--case', 'keyed-overlap', '--against', str(batch['batch_id']), '--results', str(damaged_path)))
            self.assertEqual(result.returncode, 1, result.stdout + result.stderr); self.assertFalse(failed['success'])
            with closing(sqlite3.connect(damaged_path)) as db:
                report = json.loads(db.execute('SELECT payload FROM runs WHERE id=?', (failed['run_ids'][0],)).fetchone()[0])
                self.assertEqual(report['error'], message)
            with closing(stress.connect(self.results)) as db, db:
                self.assertEqual(db.execute('SELECT payload FROM runs WHERE id=?', (ident,)).fetchone()[0], original)
                db.execute('CREATE TABLE IF NOT EXISTS controls(kind TEXT,payload TEXT)')
                db.execute('INSERT INTO controls VALUES(?,?)', ('baseline-refusal', json.dumps(dict(report=report,stdout=result.stdout,stderr=result.stderr,baseline_batch=batch['batch_id'],mutation=f'In a separate SQLite copy replace the {mutation} hash with64zeroes; original result remains unchanged.'))))


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--renderer', type=Path); parser.add_argument('--hardware', action='store_true')
    parser.add_argument('--output', type=Path)
    OPTIONS, remainder = parser.parse_known_args()
    if OPTIONS.output:
        OPTIONS.output.mkdir(parents=True, exist_ok=True)
    if OPTIONS.renderer:
        OPTIONS.renderer = OPTIONS.renderer.resolve()
    unittest.main(argv=[sys.argv[0], *remainder])
