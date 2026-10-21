#!/usr/bin/env python3
"""Lossless SQLite migration, export recovery, and real Git workflow guards."""
from contextlib import redirect_stdout
import copy
import importlib.util
import io
import json
from pathlib import Path
import sqlite3
import subprocess
import sys
import unittest
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT/'tools'))
from coverage_bookkeeping import Workspace, atomic_replace, prepare, record, refresh_index
from coverage_gate import BASELINE, REGISTER, check, snapshot
from coverage_reviews import REVIEWS, load_journal, split_journal

spec = importlib.util.spec_from_file_location('bookkeeping_gate_fixtures', ROOT/'tests/test-coverage-gate.py')
fixtures = importlib.util.module_from_spec(spec)
spec.loader.exec_module(fixtures)
spec = importlib.util.spec_from_file_location('bookkeeping_cli', ROOT/'tools/coverage-bookkeeping.py')
cli = importlib.util.module_from_spec(spec)
spec.loader.exec_module(cli)


class BookkeepingTests(unittest.TestCase):
    def setUp(self):
        self.fixture = fixtures.GateTests()
        self.fixture.setUp()
        self.addCleanup(self.fixture.doCleanups)
        self.root = self.fixture.root
        self.fixture.json(REVIEWS, {'schema': 1, 'reviews': []})
        self.workspace = Workspace(self.root)
        self.addCleanup(self.workspace.close)

    def git(self, *args):
        return subprocess.check_output(['git', '-C', str(self.root), *args], stderr=subprocess.PIPE).decode().strip()

    def repository(self):
        self.fixture.json(BASELINE, self.fixture.before)
        self.git('init', '-q')
        self.git('config', 'user.name', 'Fixture')
        self.git('config', 'user.email', 'fixture@example.invalid')
        self.git('add', 'assets', 'tests', 'research')
        self.git('commit', '-qm', 'Fixture baseline')

    def receipt(self, rid='fixture-receipt'):
        return {'id': rid, 'reason': 'Reviewed fixture source and exact contract.',
                'files': [{'path': 'assets/model.cpp', 'before_sha256': 'a'*64, 'after_sha256': 'b'*64}],
                'behaviors': [{'id': 'MV.test', 'before_sha256': 'c'*64, 'after_sha256': 'd'*64}],
                'pending_gaps': ['gap:fixture'], 'validation': {'status': 'pending',
                'reason': 'No original comparison was rerun.', 'evidence': []},
                'future_extension': {'nested': [None, False, '\u2603']}}

    def sync(self):
        with self.workspace.locked():
            return self.workspace.sync()

    def export(self):
        with self.workspace.locked():
            return self.workspace.export()

    def test_lossless_sharded_import_and_byte_identical_noop_export(self):
        receipts = [self.receipt('receipt-' + str(i)) for i in range(5)]
        self.fixture.json(REVIEWS, {'schema': 1, 'reviews': receipts, 'extension': 'preserved'})
        split_journal(self.root, max_bytes=1800)
        journal, paths = load_journal(self.root)
        before = {p: (self.root/p).read_bytes() for p in paths}
        self.assertEqual(self.sync()['reviews'], receipts)
        self.assertEqual(self.export(), [])
        self.assertEqual(before, {p: (self.root/p).read_bytes() for p in paths})
        imported = list(reversed(self.workspace.find(limit=10)))
        self.assertEqual(imported, journal['reviews'])
        self.assertEqual(self.workspace.db.execute('PRAGMA integrity_check').fetchone()[0], 'ok')

    def test_export_merges_external_appends_and_preserves_archive_bytes(self):
        old = [self.receipt('old-' + str(i)) for i in range(4)]
        self.fixture.json(REVIEWS, {'schema': 1, 'reviews': old})
        split_journal(self.root, max_bytes=1800)
        self.sync()
        _, paths = load_journal(self.root)
        archive_bytes = {p: (self.root/p).read_bytes() for p in paths if p != REVIEWS}
        with self.workspace.locked():
            self.workspace.insert(self.receipt('local'))
        active = json.loads((self.root/REVIEWS).read_text())
        active['reviews'].append(self.receipt('external'))
        self.fixture.json(REVIEWS, active)
        self.assertEqual(self.export(), ['local'])
        self.assertEqual(load_journal(self.root)[0]['reviews'], old + [self.receipt('external'), self.receipt('local')])
        self.assertEqual(archive_bytes, {p: (self.root/p).read_bytes() for p in archive_bytes})
        first = (self.root/REVIEWS).read_bytes()
        self.assertEqual(self.export(), [])
        self.assertEqual((self.root/REVIEWS).read_bytes(), first)

    def test_export_crash_after_file_replace_is_recoverable(self):
        self.sync()
        with self.workspace.locked():
            self.workspace.insert(self.receipt())
        with self.assertRaisesRegex(RuntimeError, 'crash'):
            with self.workspace.locked():
                self.workspace.export()
                raise RuntimeError('crash after replace but before DB commit')
        self.assertEqual(self.export(), [])
        self.assertEqual(len(load_journal(self.root)[0]['reviews']), 1)

    def test_failed_export_keeps_accepted_record_and_retry_succeeds(self):
        self.sync()
        with self.workspace.locked():
            self.workspace.insert(self.receipt())
        with patch('coverage_bookkeeping.atomic_replace', side_effect=OSError('disk full')):
            with self.assertRaisesRegex(OSError, 'disk full'):
                self.export()
        self.assertEqual(load_journal(self.root)[0]['reviews'], [])
        self.assertEqual(self.export(), ['fixture-receipt'])

    def test_history_removal_rewrite_and_reorder_are_rejected(self):
        original = {'schema': 1, 'reviews': [self.receipt('first'), self.receipt('second')]}
        self.fixture.json(REVIEWS, original)
        self.sync()
        for mode in ('remove', 'rewrite', 'reorder', 'duplicate'):
            value = copy.deepcopy(original)
            if mode == 'remove':
                value['reviews'].pop()
            elif mode == 'rewrite':
                value['reviews'][0]['reason'] = 'Changed old reason'
            elif mode == 'reorder':
                value['reviews'].reverse()
            else:
                value['reviews'].append(value['reviews'][0])
            self.fixture.json(REVIEWS, value)
            with self.subTest(mode=mode), self.assertRaises(ValueError):
                self.sync()
        self.fixture.json(REVIEWS, original)
        self.assertEqual(self.sync(), original)

    def test_receipts_are_immutable_and_queries_use_normalized_links(self):
        with self.workspace.locked():
            self.workspace.insert(self.receipt())
        for sql in ('DELETE FROM receipts', "UPDATE receipts SET reason='changed'"):
            with self.assertRaises(sqlite3.IntegrityError), self.workspace.db:
                self.workspace.db.execute(sql)
        self.assertEqual(self.workspace.find(path='assets/model.cpp', behavior='MV.test', status='pending'), [self.receipt()])
        self.assertEqual(self.workspace.find(path='different.cpp'), [])
        with self.workspace.locked(), self.assertRaisesRegex(ValueError, 'reused'):
            changed = self.receipt()
            changed['reason'] = 'Different'
            self.workspace.insert(changed)

    def test_atomic_replace_refuses_changed_target_and_symlink(self):
        target = self.root/'research/output.json'
        target.write_bytes(b'concurrent')
        with self.assertRaisesRegex(ValueError, 'concurrently'):
            atomic_replace(target, b'new', b'old')
        self.assertEqual(target.read_bytes(), b'concurrent')
        link = self.root/'research/link.json'
        link.symlink_to(target)
        with self.assertRaisesRegex(ValueError, 'symlink'):
            atomic_replace(link, b'new', b'concurrent')

    def test_database_cannot_be_put_in_durable_or_original_directories(self):
        for path in ('original/reviews.sqlite', 'research/reviews.sqlite'):
            with self.subTest(path=path), self.assertRaisesRegex(ValueError, 'working/'):
                Workspace(self.root, self.root/path)

    def test_real_git_prepare_record_export_matches_direct_gate(self):
        self.repository()
        self.fixture.write('assets/model.cpp', 'bounded changed fixture\n')
        old_index = (self.root/'research/index.json').read_bytes()
        prepared = prepare(self.workspace, index_reason='Reviewed existing source links; no inferred semantic mapping.')
        self.assertEqual((self.root/'research/index.json').read_bytes(), old_index)
        rid = prepared['draft']['id']
        self.assertIn('MV.test', [b['id'] for b in prepared['draft']['behaviors']])
        result = record(self.workspace, rid, 'Reviewed bounded fixture change and its linked behavior.',
                        'Original comparison remains historical; validation of changed code is pending.')
        self.assertTrue(result['gate_passed'], result)
        self.assertEqual(result['exported'], [rid])
        current, audit_report = snapshot(self.root)
        direct = check(self.fixture.before, current, load_journal(self.root)[0], audit_report)
        self.assertEqual(direct['errors'], result['gate_errors'])
        self.assertEqual(len(self.workspace.find(behavior='MV.test')), 1)
        again = record(self.workspace, rid, 'Reviewed bounded fixture change and its linked behavior.',
                       'Original comparison remains historical; validation of changed code is pending.')
        self.assertEqual(again['exported'], [])

    def test_stale_draft_source_and_head_changes_refuse_without_recording(self):
        self.repository()
        self.fixture.write('assets/model.cpp', 'first change\n')
        first = prepare(self.workspace, index_reason='Reviewed explicit fixture links.')
        self.fixture.write('assets/model.cpp', 'second change\n')
        refresh_index(self.root)
        with self.assertRaisesRegex(ValueError, 'stale'):
            record(self.workspace, first['draft']['id'], 'Reviewed fixture.', 'Pending comparison.')
        second = prepare(self.workspace)
        self.git('commit', '--allow-empty', '-qm', 'HEAD moved')
        with self.assertRaisesRegex(ValueError, 'stale'):
            record(self.workspace, second['draft']['id'], 'Reviewed fixture.', 'Pending comparison.')
        self.assertEqual(self.workspace.find(), [])

    def test_invalid_validation_and_missing_reasons_cannot_record(self):
        self.repository()
        self.fixture.write('assets/model.cpp', 'changed fixture\n')
        rid = prepare(self.workspace, index_reason='Reviewed fixture links.')['draft']['id']
        for options in ({'status': 'recorded', 'evidence': ['old-original']}, {'status': 'not_required'},
                        {'status': 'pending', 'stages': ['implementation']}):
            with self.subTest(options=options), self.assertRaises(ValueError):
                record(self.workspace, rid, 'Reviewed fixture source.', 'Reviewed validation boundary.', **options)
        with self.assertRaisesRegex(ValueError, 'reasons'):
            record(self.workspace, rid, '', 'Pending comparison.')
        self.assertEqual(self.workspace.find(), [])
        self.assertEqual(load_journal(self.root)[0]['reviews'], [])

    def test_fresh_bound_recorded_evidence_and_latest_draft_export(self):
        self.repository()
        f = self.fixture
        f.write('assets/model.cpp', 'new bounded implementation\n')
        execution = copy.deepcopy(f.record)
        execution['sources']['assets/model.cpp'] = f.sha('assets/model.cpp')
        f.json('research/new-execution.json', execution)
        evidence = {**f.register['evidence'][0], 'id': 'fresh-bound', 'record': 'research/new-execution.json',
                    'sha256': f.sha('research/new-execution.json')}
        f.register['evidence'].append(evidence)
        f.register['behaviors'][0]['evidence'].append(evidence['id'])
        f.sync()
        prepare(self.workspace)
        result = record(self.workspace, None, 'Reviewed implementation and new bound execution.',
                        'Fixture execution fingerprints the new source and exact declared scope.',
                        status='recorded', evidence=['fresh-bound'], stages=['implementation'], export=False)
        self.assertTrue(result['gate_passed'], result)
        self.assertEqual(result['exported'], [])
        self.assertEqual(load_journal(self.root)[0]['reviews'], [])
        self.assertEqual(self.export(), [result['receipt']])

    def test_scoped_receipt_reports_remaining_gate_errors(self):
        self.repository()
        self.fixture.write('assets/model.cpp', 'changed model\n')
        self.fixture.write('tools/unrelated.py', '# another contributor\n')
        prepared = prepare(self.workspace, paths=['assets/model.cpp'], index_reason='Reviewed fixture links and new unlinked tool.')
        self.assertEqual([f['path'] for f in prepared['draft']['files']], ['assets/model.cpp'])
        self.assertEqual([b['id'] for b in prepared['draft']['behaviors']], ['MV.test'])
        result = record(self.workspace, prepared['draft']['id'], 'Reviewed selected model and shared contracts.',
                        'Changed model has not been compared to original.')
        self.assertFalse(result['gate_passed'])
        self.assertTrue(any('tools/unrelated.py' in e for e in result['gate_errors']))
        self.assertEqual(result['exported'], [prepared['draft']['id']])

    def test_prepare_refuses_unknown_selectors_and_empty_index_reason(self):
        self.repository()
        with self.assertRaisesRegex(ValueError, 'cannot be empty'):
            prepare(self.workspace, index_reason=' ')
        with self.assertRaisesRegex(ValueError, 'Selectors'):
            prepare(self.workspace, paths=['absent.cpp'])

    def test_cli_sync_find_and_export_need_no_local_gate_cache(self):
        self.fixture.json(REVIEWS, {'schema': 1, 'reviews': [self.receipt()]})
        for arguments in (['sync'], ['find', '--path', 'assets/model.cpp'], ['export']):
            output = io.StringIO()
            with redirect_stdout(output), patch.object(sys, 'argv', ['workflow', '--root', str(self.root), *arguments]):
                self.assertEqual(cli.main(), 0)
            self.assertIsInstance(json.loads(output.getvalue()), dict)


if __name__ == '__main__':
    unittest.main()
