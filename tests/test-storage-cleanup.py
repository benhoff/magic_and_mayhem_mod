#!/usr/bin/env python3
"""Synthetic deletion, preservation, stale-plan and dry-default controls."""
from contextlib import closing, contextmanager, nullcontext
import json
import os
from pathlib import Path
import sqlite3
import subprocess
import sys
import tempfile
import time
import unittest
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
import storage_cleanup as cleanup
from evidence_objects import Pair
from evidence_workspace import restore


class Cleanup(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory(prefix='mm-cleanup-test-')
        self.addCleanup(self.tmp.cleanup)
        self.base = Path(self.tmp.name)
        self.root = self.base / 'repo'
        self.root.mkdir()
        subprocess.run(['git', 'init', '-qb', 'master', str(self.root)], check=True)
        (self.root / '.gitignore').write_text('working/\n')
        (self.root / 'tools').mkdir()
        (self.root / 'tools/working-retention.json').write_bytes((ROOT / 'tools/working-retention.json').read_bytes())
        subprocess.run(['git', '-C', str(self.root), 'add', '.'], check=True)
        subprocess.run(['git', '-C', str(self.root), '-c', 'user.name=Fixture', '-c', 'user.email=fixture@example.invalid',
                        'commit', '-qm', 'fixture'], check=True)
        self.run = self.root / 'working/tests/captures/run-old'
        self.run.mkdir(parents=True)
        (self.run / 'pixels.bin').write_bytes(b'unique negative capture\0' * 100)
        (self.run / 'result.json').write_text('{"success":false}\n')
        (self.run / 'settings.ini').write_text('exact config\n')
        self.age(self.run)
        for n in ('run-new1', 'run-new2'):
            p = self.run.parent / n
            p.mkdir()
            (p / 'current.bin').write_bytes(b'current')
        self.output = self.root / 'working/retention/cleanup/plan.sqlite3'
        self.scratch = self.base / 'scratch'
        self.scratch.mkdir()

    def age(self, path):
        old = time.time() - 3*86400
        for p in [*path.rglob('*'), path]:
            os.utime(p, (old, old), follow_symlinks=False)

    def plan(self):
        value = cleanup.make_plan(self.root, [self.run], 'Producer stopped and batch completed',
                                  'Reviewed historical SQLite artifacts, branches, fixtures and consumers')
        return cleanup.write_database(self.output, value)

    def inspect(self):
        return cleanup.inspect(self.root, self.run, cleanup.rules_for(self.root), {}, [self.root])

    @contextmanager
    def archives(self):
        paths = (self.base / 'archive-a/working/primary.sqlite3', self.base / 'archive-b/working/replica.sqlite3')
        for p in paths:
            p.parent.mkdir(parents=True)
        with Pair(*paths, require_independent=False) as pair:
            # These fixtures deliberately share a tmpfs. Production configured()
            # keeps its independent-filesystem requirement.
            with patch.object(cleanup, 'configured', return_value=nullcontext(pair)), \
                 patch.object(cleanup, 'settings', return_value=paths), \
                 patch.object(cleanup, 'host_process_view', return_value=True), \
                 patch.object(cleanup, 'storage_policy', return_value=dict(primary_minimum_free_gib=0, replica_minimum_free_gib=0)), \
                 patch.object(cleanup.shutil, 'disk_usage', return_value=type('Capacity', (), {'free':1024**4})()):
                yield pair

    def test_default_cli_is_inventory_and_leaves_payload_and_stores_unchanged(self):
        before = sorted(str(p.relative_to(self.root)) for p in self.root.rglob('*'))
        command = [sys.executable, '-B', str(ROOT / 'tools/cleanup-working.py'), '--root', str(self.root)]
        result = subprocess.run(command, capture_output=True, text=True, check=True)
        data = json.loads(result.stdout)
        self.assertEqual(data['kind'], 'working_cleanup_inventory')
        self.assertGreater(data['states']['review-required'], 0)
        self.assertEqual(before, sorted(str(p.relative_to(self.root)) for p in self.root.rglob('*')))
        self.assertFalse(self.output.exists())

    def test_apply_is_dry_and_does_not_open_archive_writer_or_write_events(self):
        sha = self.plan()
        before = self.output.read_bytes()
        with patch.object(cleanup, 'configured', side_effect=AssertionError('archive writer used')):
            result = cleanup.apply_plan(self.output)
        self.assertTrue(result['dry_run'])
        self.assertEqual(result['plan_sha256'], sha)
        self.assertEqual(self.output.read_bytes(), before)
        self.assertTrue((self.run / 'pixels.bin').exists())

    def test_explicit_execution_preserves_both_stores_and_real_restore(self):
        sha = self.plan()
        original = {p.name:p.read_bytes() for p in self.run.iterdir()}
        with self.archives() as pair:
            result = cleanup.apply_plan(self.output, expected_sha=sha, execute=True, scratch=self.scratch)
            self.assertFalse(result['dry_run'])
            self.assertTrue(result['results'][0]['real_restore_verified'])
            self.assertEqual(sorted(p.name for p in self.run.iterdir()), ['.evidence-workspace', '.retain'])
            manifest = result['results'][0]['manifest']
            self.assertEqual(pair.read(manifest['sha256'], manifest['size'], 0), pair.read(manifest['sha256'], manifest['size'], 1))
            restore(self.root, self.run, pair)
        self.assertEqual({p.name:p.read_bytes() for p in self.run.iterdir()}, original)
        with closing(sqlite3.connect(self.output)) as db:
            states = [json.loads(row[0])['state'] for row in db.execute('SELECT payload FROM events ORDER BY sequence')]
        self.assertEqual(states, ['started', 'completed', 'finished'])

    def test_execute_requires_digest_and_scratch(self):
        sha = self.plan()
        for options in ({}, {'expected_sha':sha}, {'expected_sha':'f'*64, 'scratch':self.scratch}):
            with self.subTest(options=options), self.assertRaises(ValueError):
                cleanup.apply_plan(self.output, execute=True, **options)
        self.assertTrue((self.run / 'pixels.bin').exists())

    def test_plan_and_events_are_immutable_and_tampered_plan_rejected(self):
        self.plan()
        with closing(sqlite3.connect(self.output)) as db, db:
            with self.assertRaises(sqlite3.IntegrityError):
                db.execute('DELETE FROM plan')
            db.execute('DROP TRIGGER immutable_plan_update')
            db.execute("UPDATE plan SET payload=x'7b7d'")
        with self.assertRaisesRegex(ValueError, 'integrity'):
            cleanup.read_database(self.output)

    def test_recent_nested_file_directory_pin_alias_hardlink_and_database_are_held(self):
        cases = ['recent-file', 'recent-directory', 'pin', 'dangling-pin', 'alias', 'hardlink', 'database', 'source', 'original', 'checkout']
        for case in cases:
            with self.subTest(case=case), tempfile.TemporaryDirectory(dir=self.run.parent) as tmp:
                path = Path(tmp)
                (path / 'bytes.bin').write_bytes(b'preserve')
                self.age(path)
                target = path / 'guard'
                if case == 'recent-file':
                    os.utime(path / 'bytes.bin', None)
                elif case == 'recent-directory':
                    os.utime(path, None)
                elif case == 'pin':
                    (path / '.retain').write_text('held')
                elif case == 'dangling-pin':
                    (path / '.retain').symlink_to('missing')
                elif case == 'alias':
                    target.symlink_to(self.run / 'pixels.bin')
                elif case == 'hardlink':
                    os.link(path / 'bytes.bin', target)
                elif case == 'database':
                    (path / 'receipts.sqlite3').touch()
                elif case == 'source':
                    (path / 'producer.py').write_text('unique source')
                elif case == 'original':
                    (path / 'original').mkdir()
                else:
                    (path / '.git').write_text('gitdir: retained')
                if case not in ('recent-file', 'recent-directory', 'pin', 'dangling-pin'):
                    self.age(path)
                row = cleanup.inspect(self.root, path, cleanup.rules_for(self.root), {}, [self.root])
                self.assertEqual(row['state'], 'held', row)

    def test_ancestor_pin_newest_run_tracked_reference_and_worktree_are_held(self):
        (self.run.parent / '.retain').write_text('held ancestor')
        self.assertEqual(self.inspect()['state'], 'held')
        (self.run.parent / '.retain').unlink()
        newest = self.run.parent / 'run-new2'
        self.age(newest)
        row = cleanup.inspect(self.root, newest, cleanup.rules_for(self.root), {}, [self.root])
        self.assertIn('one of the newest', ' '.join(row['reasons']))
        row = cleanup.inspect(self.root, self.run, cleanup.rules_for(self.root),
                              {'working/tests/captures':{'research/proof.md'}}, [self.root])
        self.assertIn('tracked metadata', ' '.join(row['reasons']))
        row = cleanup.inspect(self.root, self.run, cleanup.rules_for(self.root), {}, [self.root, self.run / 'source'])
        self.assertIn('worktree', ' '.join(row['reasons']))

    def test_process_open_file_is_held(self):
        child = subprocess.Popen([sys.executable, '-c',
            "import sys,time; f=open(sys.argv[1],'rb'); print('ready',flush=True); time.sleep(30)", str(self.run / 'pixels.bin')],
            stdout=subprocess.PIPE, text=True)
        try:
            self.assertEqual(child.stdout.readline().strip(), 'ready')
            self.assertEqual(self.inspect()['state'], 'held')
        finally:
            child.terminate(); child.wait(timeout=5); child.stdout.close()

    def test_original_alias_and_overlapping_selections_refuse(self):
        outside = self.root / 'original'
        outside.mkdir()
        with self.assertRaises(ValueError):
            cleanup.make_plan(self.root, [outside], 'completed', 'reviewed')
        with self.assertRaisesRegex(ValueError, 'nonoverlapping'):
            cleanup.make_plan(self.root, [self.run, self.run / 'nested'], 'completed', 'reviewed')
        alias = self.root / 'working/alias'
        alias.symlink_to(self.run)
        with self.assertRaises(ValueError):
            cleanup.make_plan(self.root, [alias], 'completed', 'reviewed')

    def test_plan_detects_changed_bytes_and_new_pin_before_archive(self):
        sha = self.plan()
        (self.run / 'pixels.bin').write_bytes(b'changed')
        self.age(self.run)
        with patch.object(cleanup, 'configured', side_effect=AssertionError('writer opened')):
            with self.assertRaisesRegex(ValueError, 'changed'):
                cleanup.apply_plan(self.output, expected_sha=sha)
        (self.run / '.retain').write_text('new pin')
        with self.assertRaisesRegex(ValueError, 'held'):
            cleanup.apply_plan(self.output)

    def test_new_reference_or_consumer_after_restore_prevents_retirement(self):
        sha = self.plan()
        real_check = cleanup.revalidate
        calls = []
        def changing(plan, row):
            calls.append(1)
            if len(calls) == 3:
                (self.run / '.retain').write_text('new consumer pin while archiving')
            return real_check(plan, row)
        with self.archives(), patch.object(cleanup, 'revalidate', side_effect=changing):
            with self.assertRaisesRegex(ValueError, 'held'):
                cleanup.apply_plan(self.output, expected_sha=sha, execute=True, scratch=self.scratch)
        self.assertTrue((self.run / 'pixels.bin').exists())
        self.assertFalse((self.run / '.evidence-workspace').exists())
        with closing(sqlite3.connect(self.output)) as db:
            self.assertEqual(json.loads(db.execute('SELECT payload FROM events ORDER BY sequence DESC LIMIT 1').fetchone()[0])['state'], 'failed')

    def test_failed_restore_leaves_source_intact(self):
        sha = self.plan()
        with self.archives(), patch('evidence_workspace.materialize', side_effect=ValueError('restore failed')):
            with self.assertRaisesRegex(ValueError, 'restore failed'):
                cleanup.apply_plan(self.output, expected_sha=sha, execute=True, scratch=self.scratch)
        self.assertTrue((self.run / 'pixels.bin').exists())

    def test_inventory_limit_reports_held_and_plan_never_hashes_it(self):
        row = cleanup.inspect(self.root, self.run, cleanup.rules_for(self.root), {}, [self.root], max_entries=1)
        self.assertEqual(row['state'], 'held')
        with patch.object(cleanup, 'scan', side_effect=AssertionError('unbounded hash')):
            with self.assertRaises(ValueError):
                cleanup.make_plan(self.root, [self.run], 'complete', 'reviewed', max_entries=1)

    def test_report_cannot_modify_originals_and_existing_reports_are_not_overwritten(self):
        with self.assertRaises(ValueError):
            cleanup.write_database(self.root / 'original/working/retention/report.sqlite3', {})
        self.plan()
        before = self.output.read_bytes()
        with self.assertRaises(FileExistsError):
            cleanup.write_database(self.output, {})
        self.assertEqual(before, self.output.read_bytes())

    def test_future_and_expired_plans_refuse(self):
        value = cleanup.make_plan(self.root, [self.run], 'stopped', 'reviewed')
        for offset in (-25*3600, 3600):
            output = self.output.with_name(str(offset) + '.sqlite3')
            cleanup.write_database(output, dict(value, created_ns=time.time_ns() + offset*10**9))
            with self.assertRaisesRegex(ValueError, 'expired'):
                cleanup.apply_plan(output)

    def test_force_tracked_working_file_is_held(self):
        subprocess.run(['git', '-C', str(self.root), 'add', '-f', str(self.run / 'pixels.bin')], check=True)
        self.assertIn('tracked files', ' '.join(self.inspect()['reasons']))

    def test_private_pid_namespace_cannot_retire_data(self):
        sha = self.plan()
        with patch.object(cleanup, 'host_process_view', return_value=False):
            with self.assertRaisesRegex(ValueError, 'host process view'):
                cleanup.apply_plan(self.output, expected_sha=sha, execute=True, scratch=self.scratch)
        self.assertTrue((self.run / 'pixels.bin').exists())

    def test_indexed_references_preserve_parent_child_and_exact_links(self):
        refs = cleanup.ReferenceHints({'working/tests':{'parent.md'},
            'working/tests/captures/run-old/pixels.bin':{'child.md'},
            'working/tests/captures/run-old':{'exact.md'},
            'working/unrelated':{'unrelated.md'}})
        self.assertEqual(refs.overlapping('working/tests/captures/run-old'),
                         ['child.md', 'exact.md', 'parent.md'])

    def test_selecting_parent_cannot_bypass_newest_two_runs(self):
        self.age(self.run.parent)
        row = cleanup.inspect(self.root, self.run.parent, cleanup.rules_for(self.root), {}, [self.root])
        self.assertEqual(row['state'], 'held')
        self.assertIn('newest runs', ' '.join(row['reasons']))


if __name__ == '__main__':
    unittest.main()
