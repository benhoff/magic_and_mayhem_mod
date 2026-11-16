#!/usr/bin/env python3
"""Finite custody must not weaken acceptance, history or survivor guarantees."""
from contextlib import closing
from datetime import datetime, timedelta, timezone
import importlib.util
import json
from pathlib import Path
import sqlite3
import subprocess
import sys
import unittest
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
import evidence_lifecycle as lifecycle
from coverage_scoped import capture, write_index
from coverage_gate import REGISTER, check, changes, draft, snapshot
from coverage_evidence import write_report, Store, pending_path, artifact
from coverage_bookkeeping import Workspace, finish

spec = importlib.util.spec_from_file_location('lifecycle_scoped', ROOT/'tests/test-scoped-coverage.py')
fixtures = importlib.util.module_from_spec(spec)
spec.loader.exec_module(fixtures)


class Lifecycle(unittest.TestCase):
    def setUp(self):
        self.f = fixtures.Scoped()
        self.f.setUp()
        self.addCleanup(self.f.doCleanups)
        self.root = self.f.root
        self.f.f.json(lifecycle.POLICY, dict(schema=1, review_days=14,
            maximum_evidence_bytes=1024**2, finish_backups=False))
        # Use real SQLite pointers so missing supporting bytes are observable.
        self.f.f.write('.gitignore', '/working/\n')
        self.f.f.json('.evidence-store.json', dict(schema=1, archives=[]))
        body = (self.root/'research/record.json').read_bytes()
        write_report(self.root, 'research/runtime/proof.json', body)
        self.f.f.register['evidence'][0]['record'] = 'research/runtime/proof.json'
        self.f.update()
        write_index(self.root)
        self.f.f.register = json.loads((self.root/REGISTER).read_bytes())
        self.f.git('add', '.')
        self.f.git('commit', '-qm', 'finite policy and SQLite proof')
        self.commit = self.f.git('rev-parse', 'HEAD')
        self.start = datetime(2026, 1, 1, tzinfo=timezone.utc)
        with patch.object(lifecycle, 'now', return_value=self.start):
            lifecycle.integrated(self.root, [self.commit])

    def plan(self):
        return lifecycle.prepare_retirement(self.root, ['old-original'], self.commit,
            owner='fixture task', reason='Review complete; expire supporting report',
            consumer_review='No active comparisons, pins or writers; only historical claim remains',
            result='Original bounded comparison passed', reproduction='Commit plus fixture inputs; run comparison')

    def retire(self):
        with patch.object(lifecycle, 'now', return_value=self.start + timedelta(days=15)):
            plan = self.plan()
            lifecycle.export_retirement(self.root, plan['plan'])
        self.f.git('add', lifecycle.RETIREMENTS)
        self.f.git('commit', '-qm', 'reviewed retirement declaration')

    def remove_payload(self, sha=None):
        # Simulate loss of only this report, preserving operational indexes.
        sha = sha or self.f.f.register['evidence'][0]['sha256']
        with closing(sqlite3.connect(pending_path(self.root))) as db, db:
            db.execute('DROP TRIGGER objects_no_delete')
            db.execute('DROP TRIGGER chunks_no_delete')
            db.execute('DELETE FROM versions WHERE sha256=?', (sha,))
            db.execute('DELETE FROM chunks WHERE sha256=?', (sha,))
            db.execute('DELETE FROM objects WHERE sha256=?', (sha,))

    def receipt(self, a, b):
        row = draft(changes(a, b))['reviews'][0]
        row['reason'] = 'Reviewed exact source change and retired-proof limitation'
        row['validation']['reason'] = 'Original comparison remains pending; no validation promotion'
        return dict(schema=1, reviews=[row])

    def test_accepted_old_review_shard_can_be_unavailable_but_new_shard_cannot(self):
        from coverage_reviews import ARCHIVES, REVIEWS
        import hashlib
        data = b'{"schema":1,"reviews":[]}\n'
        sha = hashlib.sha256(data).hexdigest()
        name = ARCHIVES + sha + '.json'
        write_report(self.root, name, data)
        self.f.f.json(REVIEWS, dict(schema=1, archives=[dict(path=name, sha256=sha)], reviews=[]))
        self.f.git('add', '.')
        self.f.git('commit', '-qm', 'accepted shard')
        source = pending_path(self.root)
        with closing(sqlite3.connect(source)) as db, db:
            db.execute('DROP TRIGGER objects_no_delete')
            db.execute('DROP TRIGGER chunks_no_delete')
            db.execute('DELETE FROM chunks WHERE sha256=?', (sha,))
            db.execute('DELETE FROM objects WHERE sha256=?', (sha,))
        self.f.f.write('docs/note.md', 'unrelated prose\n')
        a, b, report, journal, base = capture(self.root, 'HEAD')
        self.assertEqual(report['checked_scope']['evidence'], [])
        workspace = Workspace(self.root)
        self.addCleanup(workspace.close)
        with workspace.locked():
            workspace.sync()
        # Mutating the actual pinned reference is still refused.
        from coverage_evidence import pointer
        (self.root/name).write_bytes(pointer('f'*64, len(data)))
        with self.assertRaisesRegex(ValueError, 'identity changed'):
            capture(self.root, 'HEAD')

    def test_retirement_declarations_remain_authored_control_bytes(self):
        self.assertFalse(artifact(lifecycle.RETIREMENTS))
        self.retire()
        from coverage_scoped import View
        view = View(self.root, revision='HEAD')
        try:
            self.assertEqual(json.loads(view.raw(lifecycle.RETIREMENTS))['schema'], 1)
        finally:
            view.close()

    def test_staged_policy_cannot_be_weakened_by_an_unstaged_budget(self):
        settings = lifecycle.policy(self.root)
        settings['maximum_evidence_bytes'] = 1
        self.f.f.json(lifecycle.POLICY, settings)
        write_report(self.root, 'research/runtime/new-budget-report.json', b'{\"new\":true}')
        self.f.git('add', lifecycle.POLICY, 'research/runtime/new-budget-report.json')
        settings['maximum_evidence_bytes'] = 1024**2
        self.f.f.json(lifecycle.POLICY, settings)
        with self.assertRaisesRegex(ValueError, 'budget exceeded'):
            lifecycle.verify_acceptance(self.root, 'HEAD', {'old-original'})

    def test_new_missing_review_shard_cannot_use_opaque_archive_path(self):
        import hashlib
        from coverage_reviews import ARCHIVES, REVIEWS
        from coverage_evidence import pointer
        body = b'{"schema":1,"reviews":[]}\n'
        sha = hashlib.sha256(body).hexdigest()
        name = ARCHIVES + sha + '.json'
        self.f.f.write(name, pointer(sha, len(body)).decode())
        self.f.f.json(REVIEWS, dict(schema=1, archives=[dict(path=name, sha256=sha)], reviews=[]))
        with self.assertRaises(ValueError):
            capture(self.root, 'HEAD')

    def test_reused_reports_are_verified_without_charging_new_custody_budget(self):
        settings = lifecycle.policy(self.root)
        settings['maximum_evidence_bytes'] = 1
        self.f.f.json(lifecycle.POLICY, settings)
        self.f.git('add', lifecycle.POLICY)
        payloads = lifecycle.verify_acceptance(self.root, 'HEAD', {'old-original'})
        self.assertTrue(payloads)

    def test_expiry_starts_at_observed_integration_not_old_commit_date(self):
        with patch.object(lifecycle, 'now', return_value=self.start + timedelta(days=13)):
            with self.assertRaisesRegex(ValueError, 'not expired'):
                self.plan()
        with patch.object(lifecycle, 'now', return_value=self.start + timedelta(days=16)):
            first = lifecycle.integrated(self.root, [self.commit])
            self.assertEqual(first[self.commit], self.start.isoformat())
            self.assertTrue(self.plan()['plan'])

    def test_unmerged_commit_cannot_start_clock(self):
        self.f.git('switch', '-c', 'owned')
        self.f.f.write('docs/note.md', 'unfinished\n')
        self.f.git('add', '.')
        self.f.git('commit', '-qm', 'unmerged')
        with self.assertRaises(subprocess.CalledProcessError):
            lifecycle.integrated(self.root, ['HEAD'])

    def test_declaration_checks_original_bytes_before_retirement(self):
        self.retire()
        a, b, report, journal, base = capture(self.root, self.commit)
        self.assertEqual(report['errors'], [])
        self.remove_payload()
        # A fresh declaration cannot silently legitimize already missing proof.
        with self.assertRaises(ValueError):
            capture(self.root, self.commit)

    def test_retired_payload_allows_pending_source_review_but_no_validation_credit(self):
        self.retire()
        self.remove_payload()
        self.f.f.write('assets/model.cpp', 'changed code after expiry\n')
        self.f.update()
        a, b, report, journal, base = capture(self.root, 'HEAD')
        self.assertEqual(report['findings']['retired_evidence'], ['old-original'])
        self.assertEqual(report['behaviors'][0]['evidence_freshness'], 'retired')
        receipt = self.receipt(a, b)
        self.assertEqual(check(a, b, receipt, report)['errors'], [])
        receipt['reviews'][0]['validation'] = dict(status='recorded', reason='Attempt historical credit', evidence=['old-original'])
        self.assertTrue(any('stale' in e for e in check(a, b, receipt, report)['errors']))
        with self.assertRaises(ValueError):
            snapshot(self.root)  # A full audit never treats a declaration as proof.

    def test_retired_payload_cannot_support_status_promotion(self):
        self.retire()
        self.remove_payload()
        self.f.f.register['behaviors'][0]['status']['implementation'] = 'scoped'
        self.f.update()
        a, b, report, journal, base = capture(self.root, 'HEAD')
        self.assertTrue(any('promotion' in e for e in check(a, b, self.receipt(a, b), report)['errors']))

    def test_unavailable_unchanged_proof_is_historical_only_not_retirement(self):
        self.remove_payload()
        self.f.f.write('assets/model.cpp', 'new code, original proof missing\n')
        self.f.update()
        a, b, report, journal, base = capture(self.root, 'HEAD')
        self.assertEqual(report['findings']['unavailable_evidence'], ['old-original'])
        self.assertNotIn('retired_evidence', report['findings'])
        self.assertEqual(check(a, b, self.receipt(a, b), report)['errors'], [])
        receipt = self.receipt(a, b)
        receipt['reviews'][0]['validation'] = dict(status='recorded', reason='Attempt missing proof', evidence=['old-original'])
        self.assertTrue(check(a, b, receipt, report)['errors'])

    def test_missing_inventory_preserves_only_unchanged_mapping_without_credit(self):
        import hashlib
        body = (self.root/'research/inventory.json').read_bytes()
        write_report(self.root, 'research/runtime/inventory.json', body)
        self.f.f.register['builds'][0].update(inventory='research/runtime/inventory.json', inventory_sha256=hashlib.sha256(body).hexdigest())
        self.f.update()
        self.f.git('add', '.')
        self.f.git('commit', '-qm', 'accepted original inventory')
        self.remove_payload(hashlib.sha256(body).hexdigest())
        self.f.f.write('assets/model.cpp', 'source edit with mapping unchanged\n')
        self.f.update()
        a, b, report, journal, base = capture(self.root, 'HEAD')
        self.assertEqual(report['findings']['unavailable_inventories'], ['fixture'])
        self.assertEqual(report['behaviors'][0]['current_validation']['comparison']['state'], 'pending')
        self.f.f.register['behaviors'][0]['original'][0]['address'] += 1
        self.f.update()
        with self.assertRaisesRegex(ValueError, 'mapping requires its inventory'):
            capture(self.root, 'HEAD')

    def test_available_corrupt_report_is_an_error_not_unavailability(self):
        source = pending_path(self.root)
        sha = self.f.f.register['evidence'][0]['sha256']
        with closing(sqlite3.connect(source)) as db, db:
            db.execute('DROP TRIGGER chunks_no_update')
            db.execute('UPDATE chunks SET payload=? WHERE sha256=?', (b'corrupt', sha))
        self.f.f.write('assets/model.cpp', 'source change\n')
        self.f.update()
        with self.assertRaises((ValueError, __import__('zlib').error)):
            capture(self.root, 'HEAD')

    def test_acceptance_payload_budget_refuses_before_commit(self):
        self.f.git('switch', '-c', 'owned')
        settings = lifecycle.policy(self.root)
        settings['maximum_evidence_bytes'] = 1
        self.f.f.json(lifecycle.POLICY, settings)
        self.f.f.write('assets/model.cpp', 'bounded source edit\n')
        before = self.f.git('rev-parse', 'HEAD')
        workspace = Workspace(self.root)
        self.addCleanup(workspace.close)
        with self.assertRaisesRegex(ValueError, 'budget exceeded'):
            finish(workspace, 'HEAD', 'Reviewed edit', 'Pending original comparison', 'Refused budget', 'Reviewed links')
        self.assertEqual(self.f.git('rev-parse', 'HEAD'), before)

    def test_retired_provenance_does_not_require_an_obsolete_source_checkout_path(self):
        self.retire()
        self.remove_payload()
        (self.root/'assets/model.cpp').rename(self.root/'assets/replacement.cpp')
        self.f.f.register['behaviors'][0]['implementation'] = ['assets/replacement.cpp']
        self.f.update()
        a, b, report, journal, base = capture(self.root, 'HEAD')
        self.assertEqual(report['findings']['retired_evidence'], ['old-original'])
        self.assertEqual(check(a, b, self.receipt(a, b), report)['errors'], [])

    def test_retirement_history_cannot_be_removed_or_rebound(self):
        self.retire()
        (self.root/lifecycle.RETIREMENTS).unlink()
        with self.assertRaisesRegex(ValueError, 'removed or rewritten'):
            capture(self.root, 'HEAD')

    def test_pruning_preserves_unrelated_exact_bytes_and_releases_no_active_claim(self):
        self.retire()
        write_report(self.root, 'research/runtime/keep.json', b'{"keep":"unique negative evidence"}')
        source = pending_path(self.root)
        preview = lifecycle.prune_store(self.root, source, consumer_review='All fixture consumers stopped')
        self.assertEqual(preview['selected'], 1)
        self.assertFalse(preview['deletion_performed'])
        result = lifecycle.prune_store(self.root, source, consumer_review='All fixture consumers stopped', execute=True)
        self.assertTrue(result['deletion_performed'])
        from coverage_evidence import Reader
        with Reader(self.root) as reader:
            self.assertEqual(reader.json('research/runtime/keep.json'), {'keep': 'unique negative evidence'})
            with self.assertRaises(ValueError):
                reader.json('research/runtime/proof.json')

    def test_unknown_sqlite_tables_and_sidecars_refuse_pruning(self):
        self.retire()
        source = pending_path(self.root)
        with closing(sqlite3.connect(source)) as db, db:
            db.execute('CREATE TABLE unique_source(payload BLOB)')
        with self.assertRaisesRegex(ValueError, 'Unknown/shared'):
            lifecycle.prune_store(self.root, source, consumer_review='Reviewed')
        Path(str(source)+'-wal').touch()
        with self.assertRaisesRegex(ValueError, 'active'):
            lifecycle.prune_store(self.root, source, consumer_review='Reviewed')

    def test_finite_finish_checks_payloads_and_hook_without_database_snapshots(self):
        self.f.git('switch', '-c', 'owned')
        self.f.f.write('assets/model.cpp', 'bounded source change\n')
        self.f.git('config', 'core.hooksPath', str(self.root/'.git/hooks'))
        hook = self.root/'.git/hooks/pre-commit'
        hook.write_text('#!/bin/sh\nprintf checked > .git/hook-ran\n')
        hook.chmod(0o755)
        workspace = Workspace(self.root)
        self.addCleanup(workspace.close)
        with patch('evidence_local.retained_backup', side_effect=AssertionError('automatic backup')):
            result = finish(workspace, 'HEAD', 'Reviewed source', 'Pending original comparison',
                            'Finite acceptance', 'Reviewed census')
        self.assertTrue((self.root/'.git/hook-ran').is_file())
        self.assertEqual(result['backups'], [])
        self.assertEqual(result['acceptance']['tree'], self.f.git('rev-parse', 'HEAD^{tree}'))
        self.assertTrue(result['acceptance']['payloads'])
        self.assertFalse((workspace.path.parent/'backups').exists())

    def test_finite_finish_refuses_missing_required_report(self):
        import copy, hashlib
        body = (json.dumps(self.f.f.record, sort_keys=True) + '\n').encode()
        write_report(self.root, 'research/runtime/new-proof.json', body)
        evidence = copy.deepcopy(self.f.f.register['evidence'][0])
        evidence.update(id='new-proof', record='research/runtime/new-proof.json', sha256=hashlib.sha256(body).hexdigest())
        self.f.f.register['evidence'].append(evidence)
        self.f.update()
        write_index(self.root)
        self.f.f.register = json.loads((self.root/REGISTER).read_bytes())
        self.remove_payload(evidence['sha256'])
        self.f.f.write('assets/model.cpp', 'change needing missing original proof\n')
        workspace = Workspace(self.root)
        self.addCleanup(workspace.close)
        before = self.f.git('rev-parse', 'HEAD')
        with self.assertRaises(ValueError):
            finish(workspace, 'HEAD', 'Reviewed source', 'Pending comparison', 'Refused', 'Census')
        self.assertEqual(self.f.git('rev-parse', 'HEAD'), before)


if __name__ == '__main__':
    unittest.main()
