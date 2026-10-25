#!/usr/bin/env python3
"""Local commit, consistent backup, and fail-closed sharing boundaries."""
import hashlib
import importlib.util
import io
import json
import os
from pathlib import Path
import sqlite3
import subprocess
import sys
import tempfile
import unittest
from contextlib import closing
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT/'tools'))
from coverage_evidence import CONFIG, Store, Reader, digest_file, pointer, write_report
from evidence_local import backup_sqlite, local_evidence
from coverage_bookkeeping import Workspace, finish
from coverage_gate import BASELINE
from coverage_reviews import REVIEWS, load_journal

spec = importlib.util.spec_from_file_location('workflow_sync', ROOT/'tools/sync-evidence.py')
sync = importlib.util.module_from_spec(spec); spec.loader.exec_module(sync)
spec = importlib.util.spec_from_file_location('workflow_gate_fixtures', ROOT/'tests/test-coverage-gate.py')
fixtures = importlib.util.module_from_spec(spec); spec.loader.exec_module(fixtures)


class LocalTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.env = patch.dict(os.environ, {'MM_EVIDENCE_DB': '',
                                          'MM_EVIDENCE_CACHE': str(self.root/'cache')})
        self.env.start(); self.addCleanup(self.env.stop)
        (self.root/CONFIG).write_text('{"schema":1,"archives":[]}')

    def pending(self, data=b'{"success":true}'):
        path = self.root/'working/coverage/evidence-pending.sqlite3'
        store = Store(path)
        try:
            with store.db:
                sha = store.put(io.BytesIO(data), len(data))
        finally:
            store.close()
        report = self.root/'research/runtime/result.json'
        report.parent.mkdir(parents=True, exist_ok=True)
        report.write_bytes(pointer(sha, len(data)))
        return path, report

    def test_pending_evidence_survives_temporary_staged_tree_without_network(self):
        database, report = self.pending()
        tree = self.root/'staged'; tree.mkdir()
        (tree/CONFIG).write_text((self.root/CONFIG).read_text())
        copy = tree/'research/runtime/result.json'; copy.parent.mkdir(parents=True)
        copy.write_bytes(report.read_bytes())
        previous = dict(os.environ)
        with patch('coverage_evidence.urllib.request.urlopen', side_effect=AssertionError('Network forbidden')):
            with local_evidence(self.root), Reader(tree) as reader:
                self.assertEqual(reader.json(copy), {'success': True})
                self.assertEqual(Path(os.environ['MM_EVIDENCE_DB']), database)
        self.assertEqual(dict(os.environ), previous)

    def test_missing_or_corrupt_cached_archive_refuses_without_fetch(self):
        (self.root/CONFIG).write_text(json.dumps({'schema': 1, 'archives': [{'sha256': 'a'*64}]}))
        with patch('coverage_evidence.urllib.request.urlopen', side_effect=AssertionError('Network forbidden')):
            with self.assertRaisesRegex(ValueError, 'unavailable locally'):
                with local_evidence(self.root): pass
            cache = self.root/'cache'; cache.mkdir()
            (cache/('a'*64 + '.sqlite3')).write_bytes(b'corrupt')
            with self.assertRaisesRegex(ValueError, 'unavailable locally'):
                with local_evidence(self.root): pass

    def test_backup_includes_live_wal_and_can_restore_verified_payloads(self):
        source, report = self.pending()
        live = sqlite3.connect(source)
        self.addCleanup(live.close)
        live.execute('PRAGMA journal_mode=WAL')
        live.execute('CREATE TABLE committed(value TEXT)')
        live.execute("INSERT INTO committed VALUES ('survives')"); live.commit()
        result = backup_sqlite(source, self.root/'backups')
        self.assertEqual(result['objects_verified'], 1)
        self.assertEqual(digest_file(result['path']), result['sha256'])
        with closing(sqlite3.connect(result['path'])) as restored:
            self.assertEqual(restored.execute('SELECT value FROM committed').fetchone(), ('survives',))
        self.assertTrue((self.root/'backups/.retain').is_file())
        self.assertEqual(backup_sqlite(source, self.root/'backups')['path'], result['path'])
        with patch.dict(os.environ, {'MM_EVIDENCE_DB': result['path']}):
            with Reader(self.root) as reader:
                self.assertTrue(reader.json(report)['success'])

    def test_corrupt_backup_input_does_not_create_retained_snapshot(self):
        bad = self.root/'broken.sqlite3'; bad.write_bytes(b'broken')
        with self.assertRaises(sqlite3.Error):
            backup_sqlite(bad, self.root/'backups')
        self.assertEqual(list((self.root/'backups').iterdir()), [])

    def git(self, *args):
        return subprocess.check_output(['git', '-C', str(self.root), *args], stderr=subprocess.PIPE)

    def repository(self):
        self.git('init', '-q'); self.git('config', 'user.name', 'Fixture')
        self.git('config', 'user.email', 'fixture@example.invalid')

    def test_sync_preserves_intermediate_evidence_absent_at_head(self):
        self.repository(); _, report = self.pending(b'{"version":1}')
        self.git('add', 'research', CONFIG); self.git('commit', '-qm', 'First report')
        self.pending(b'{"version":2}')
        self.git('add', 'research'); self.git('commit', '-qm', 'Second report')
        result = sync.collect_missing(self.root, 'HEAD', self.root/'outgoing.sqlite3')
        self.assertEqual(result['missing_objects'], 2)
        store = Store(self.root/'outgoing.sqlite3', readonly=True)
        try:
            self.assertTrue(store.has(hashlib.sha256(b'{"version":1}').hexdigest()))
            self.assertEqual(store.verify(), 2)
        finally: store.close()

    def test_sync_refuses_raw_evidence_before_publication(self):
        self.repository(); report = self.root/'research/runtime/raw.json'
        report.parent.mkdir(parents=True); report.write_text('{"raw":true}')
        self.git('add', 'research', CONFIG); self.git('commit', '-qm', 'Unconverted fixture')
        with self.assertRaisesRegex(ValueError, 'Invalid evidence pointer'):
            sync.collect_missing(self.root, 'HEAD', self.root/'outgoing.sqlite3')

    def test_sync_never_pushes_when_publication_fails(self):
        self.repository(); self.git('add', CONFIG); self.git('commit', '-qm', 'Clean fixture')
        with patch.object(sync, 'collect_missing', return_value={'missing_objects': 1}), \
             patch.object(sync, 'publish_missing', side_effect=RuntimeError('Upload failed')) as publish, \
             patch.object(sync, 'push_branch') as push:
            with self.assertRaisesRegex(RuntimeError, 'Upload failed'):
                sync.synchronize(self.root, Path('working/sync'), 'evidence-fixture', push=True)
        self.assertEqual(publish.call_count, 1)
        push.assert_not_called()

    def test_sync_refuses_dirty_worktree_without_publishing(self):
        self.repository(); self.git('add', CONFIG); self.git('commit', '-qm', 'Clean fixture')
        (self.root/'changed.txt').write_text('owned but not committed')
        with patch.object(sync, 'collect_missing', side_effect=AssertionError('Must not publish')):
            with self.assertRaisesRegex(ValueError, 'clean, owned worktree'):
                sync.synchronize(self.root, Path('working/sync'), 'evidence-fixture')


class FinishTests(unittest.TestCase):
    def setUp(self):
        self.fixture = fixtures.GateTests(); self.fixture.setUp()
        self.addCleanup(self.fixture.doCleanups)
        self.root = self.fixture.root
        self.fixture.json(REVIEWS, {'schema': 1, 'reviews': []})
        self.fixture.json(BASELINE, self.fixture.before)
        self.git('init', '-q'); self.git('config', 'user.name', 'Fixture')
        self.git('config', 'user.email', 'fixture@example.invalid')
        (self.root/'.gitignore').write_text('working/\n')
        self.git('add', '.'); self.git('commit', '-qm', 'Fixture baseline')
        self.workspace = Workspace(self.root); self.addCleanup(self.workspace.close)

    def git(self, *args):
        return subprocess.check_output(['git', '-C', str(self.root), *args], stderr=subprocess.PIPE).decode().strip()

    def run_finish(self):
        return finish(self.workspace, 'HEAD', 'Reviewed owned source and existing links.',
                      'Current original comparison remains pending; no milestone promoted.',
                      'Finish owned batch', 'Reviewed source census links')

    def test_local_finish_records_backups_and_commits_with_hook_without_upload(self):
        hook = self.root/'.git/hooks/pre-commit'
        hook.write_text('#!/bin/sh\nprintf checked > .git/hook-ran\n')
        hook.chmod(0o755)
        self.fixture.write('assets/model.cpp', 'changed owned source\n')
        with patch('coverage_evidence.urllib.request.urlopen', side_effect=AssertionError('Network forbidden')):
            result = self.run_finish()
        self.assertEqual(self.git('rev-parse', 'HEAD'), result['commit'])
        self.assertTrue((self.root/'.git/hook-ran').is_file())
        self.assertTrue(load_journal(self.root)[0]['reviews'])
        self.assertTrue(result['backups'])
        self.assertFalse(self.git('status', '--porcelain'))

    def test_rejected_commit_keeps_accepted_receipts_source_and_backup(self):
        before = self.git('rev-parse', 'HEAD')
        hook = self.root/'.git/hooks/pre-commit'
        hook.write_text('#!/bin/sh\nexit 1\n'); hook.chmod(0o755)
        self.fixture.write('assets/model.cpp', 'changed owned source\n')
        with self.assertRaises(subprocess.CalledProcessError): self.run_finish()
        self.assertEqual(self.git('rev-parse', 'HEAD'), before)
        self.assertTrue(load_journal(self.root)[0]['reviews'])
        self.assertTrue(list((self.workspace.path.parent/'backups').glob('*.sqlite3')))

    def test_finish_optional_timings_include_backups_and_normal_commit(self):
        self.fixture.write('assets/model.cpp', 'changed owned source\n')
        result = finish(self.workspace, 'HEAD', 'Reviewed owned source links.',
                        'Original comparison remains pending.', 'Finish timed fixture',
                        'Reviewed census links', timings=True)
        self.assertEqual(set(result['timings_seconds']), {
            'evidence_preflight', 'source_index', 'snapshots', 'review_check',
            'review_export', 'review_backup', 'git_add', 'git_commit_with_hook'})
        self.assertTrue(all(value >= 0 for value in result['timings_seconds'].values()))
        self.assertEqual(self.git('rev-parse', 'HEAD'), result['commit'])

    def test_missing_promotion_proof_cannot_create_commit(self):
        self.fixture.register['behaviors'][0]['status']['implementation'] = 'scoped'
        self.fixture.sync()
        with self.assertRaisesRegex(ValueError, 'Status promotion requires'):
            self.run_finish()
        self.assertEqual(load_journal(self.root)[0]['reviews'], [])

    def test_explicit_promotion_recording_refuses_stale_proof_without_changing_status(self):
        self.fixture.register['behaviors'][0]['status']['implementation'] = 'scoped'
        self.fixture.write('assets/model.cpp', 'source changed since original fixture proof\n')
        self.fixture.sync()
        with self.assertRaisesRegex(ValueError, 'Current execution evidence required'):
            finish(self.workspace, 'HEAD', 'Reviewed explicit scoped declaration.',
                   'Require current complete-source execution for that declared milestone.',
                   'Must refuse stale proof', 'Reviewed source links', record_promotions=True)
        self.assertEqual(load_journal(self.root)[0]['reviews'], [])
        status = json.loads((self.root/'research/runtime/coverage/register.json').read_text())['behaviors'][0]['status']
        self.assertEqual(status['implementation'], 'scoped')

    def test_real_staged_hook_resolves_local_unpublished_sqlite(self):
        self.fixture.json(CONFIG, {'schema': 1, 'archives': []})
        self.git('add', CONFIG); self.git('commit', '-qm', 'Adopt fixture SQLite store')
        write_report(self.root, self.root/'research/runtime/local-result.json', b'{"success":true}')
        self.fixture.write('assets/model.cpp', 'owned source with local-only evidence\n')
        hook = self.root/'.git/hooks/pre-commit'
        import shlex
        hook.write_text('#!/bin/sh\nexec ' + shlex.quote(sys.executable) + ' ' +
                        shlex.quote(str(ROOT/'tools/check-re-coverage.py')) +
                        ' --root . --staged --base HEAD\n')
        hook.chmod(0o755)
        result = self.run_finish()
        self.assertEqual(self.git('rev-parse', 'HEAD'), result['commit'])
        self.assertEqual(json.loads((self.root/CONFIG).read_text())['archives'], [])


if __name__ == '__main__': unittest.main()
