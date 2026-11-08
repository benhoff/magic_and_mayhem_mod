#!/usr/bin/env python3
"""Refusal and exact-recovery controls for explicitly compacted local storage."""
from contextlib import closing
import hashlib
import json
import os
import shutil
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
from evidence_objects import Pair
from worktree_compaction import compact, inspect, restore
from snapshot_consolidation import consolidate, restore_catalog
from sqlite_snapshot_store import Store, CHUNK
import zlib


class Compaction(unittest.TestCase):
    def setUp(self):
        self.ram = tempfile.TemporaryDirectory(prefix='mm-compaction-')
        self.addCleanup(self.ram.cleanup)
        self.path = Path(self.ram.name)
        disk = Path(os.environ.get('MNM_TEST_DISK_ROOT', ROOT / 'working/tests/storage-compaction'))
        disk.mkdir(parents=True, exist_ok=True)
        self.disk = tempfile.TemporaryDirectory(dir=disk)
        self.addCleanup(self.disk.cleanup)
        self.other = Path(self.disk.name)
        self.pair = Pair(self.path / 'working/objects.sqlite3', self.other / 'working/objects.sqlite3')
        self.addCleanup(self.pair.close)

    def make_worktree(self):
        main = self.path / 'main'
        subprocess.run(['git', 'init', '-q', '-b', 'master', str(main)], check=True)
        for k, v in [('user.name', 'Storage fixture'), ('user.email', 'fixture@example.invalid')]:
            subprocess.run(['git', '-C', str(main), 'config', k, v], check=True)
        for name, data in [('research/negative.md', b'retained failure\n'), ('tests/input.bin', b'unique fixture\0'), ('tools/keep.py', b'kept source\n')]:
            p = main / name
            p.parent.mkdir(parents=True, exist_ok=True)
            p.write_bytes(data)
        (main / '.gitattributes').write_text('research/** filter=mm-evidence\n')
        subprocess.run(['git', '-C', str(main), 'add', '.'], check=True)
        subprocess.run(['git', '-C', str(main), 'commit', '-qm', 'fixture'], check=True)
        tree = self.path / 'tree'
        subprocess.run(['git', '-C', str(main), 'worktree', 'add', '-qb', 'completed', str(tree)], check=True)
        (tree / 'working').mkdir()
        (tree / 'working/.retain').write_text('held execution')
        (tree / 'research/extra.log').write_text('untracked')
        return main, tree

    def test_tracked_compaction_and_real_restore_keep_outputs_and_registration(self):
        main, tree = self.make_worktree()
        result = compact(tree, main, self.pair, recent_hours=0, scratch=self.path)
        self.assertTrue(result['real_restore_verified'])
        self.assertFalse((tree / 'tests/input.bin').exists())
        self.assertTrue((tree / '.git').is_file())
        self.assertEqual((tree / 'working/.retain').read_text(), 'held execution')
        self.assertEqual((tree / 'research/extra.log').read_text(), 'untracked')
        output = restore(self.pair, result['manifest'], self.path / 'restored')
        self.assertEqual((output / 'tests/input.bin').read_bytes(), b'unique fixture\0')
        self.assertFalse(subprocess.check_output(['git', '-C', str(tree), 'status', '--porcelain', '--untracked-files=no']))

    def test_modified_recent_pinned_and_active_sources_refuse(self):
        main, tree = self.make_worktree()
        with self.assertRaisesRegex(ValueError, 'Recent'):
            inspect(tree, main)
        (tree / 'research/.retain').write_text('pin')
        with self.assertRaisesRegex(ValueError, 'Pinned'):
            inspect(tree, main, recent_hours=0)
        (tree / 'research/.retain').unlink()
        with patch('worktree_compaction.idle', side_effect=ValueError('active')):
            with self.assertRaisesRegex(ValueError, 'active'):
                inspect(tree, main, recent_hours=0)
        (tree / 'tests/input.bin').write_bytes(b'changed')
        with self.assertRaisesRegex(ValueError, 'Modified'):
            compact(tree, main, self.pair, recent_hours=0, scratch=self.path)

    def test_restore_failure_prevents_sparse_change(self):
        main, tree = self.make_worktree()
        with patch('worktree_compaction.restore', side_effect=ValueError('bad restore')):
            with self.assertRaisesRegex(ValueError, 'bad restore'):
                compact(tree, main, self.pair, recent_hours=0, scratch=self.path)
        self.assertTrue((tree / 'tests/input.bin').is_file())
        self.assertFalse((tree / 'WORKTREE-ARCHIVED.txt').exists())

    def test_running_source_script_refuses_after_its_source_file_is_closed(self):
        main, tree = self.make_worktree()
        script = tree / 'tools/holder.py'
        script.write_text("import time; print('ready', flush=True); time.sleep(30)\n")
        child = subprocess.Popen([sys.executable, str(script)], cwd=self.path,
                                 stdout=subprocess.PIPE, text=True)
        try:
            self.assertEqual(child.stdout.readline().strip(), 'ready')
            with self.assertRaisesRegex(ValueError, 'Active command'):
                inspect(tree, main, recent_hours=0)
        finally:
            child.terminate(); child.wait(timeout=5); child.stdout.close()

    def test_selected_research_only_preserves_pinned_tests_and_avoids_filter(self):
        main, tree = self.make_worktree()
        (tree / 'tests/.retain').write_text('current fixture')
        subprocess.run(['git', '-C', str(main), 'config', 'filter.mm-evidence.process', '/not/a/real/filter'], check=True)
        result = compact(tree, main, self.pair, recent_hours=0, scratch=self.path, prefixes=('research',))
        self.assertTrue((tree / 'tests/input.bin').is_file())
        self.assertTrue((tree / 'tests/.retain').is_file())
        self.assertFalse((tree / 'research/negative.md').exists())
        self.assertTrue(result['success'])

    def snapshot(self, catalog, value):
        dbpath = self.path / ('input-' + value + '.sqlite3')
        if not dbpath.exists():
            with closing(sqlite3.connect(dbpath)) as d:
                d.execute('CREATE TABLE t(v BLOB)')
                d.execute('INSERT INTO t VALUES(?)', ((value * 10000).encode(),))
                d.commit()
        raw = dbpath.read_bytes()
        chunks = [raw[i:i + CHUNK] for i in range(0, len(raw), CHUNK)]
        parts = [hashlib.sha256(x).hexdigest() for x in chunks]
        stage = sqlite3.connect(':memory:')
        stage.execute('CREATE TABLE chunks(sha256 TEXT,size INTEGER,payload BLOB)')
        stage.executemany('INSERT INTO chunks VALUES(?,?,?)', [(h, len(b), zlib.compress(b, 1)) for h, b in zip(parts, chunks)])
        store = Store(catalog)
        sha = hashlib.sha256(raw).hexdigest()
        store.import_stage(stage, sha, len(raw), parts)
        store.receipt(dbpath, sha, 'original', {'negative': value, 'scope': 'fixture'})
        store.close()
        stage.close()
        return sha

    def catalogs(self):
        a, b = self.path / 'a.sqlite3', self.path / 'b.sqlite3'
        common = self.snapshot(a, 'common')
        self.snapshot(b, 'common')
        unique = self.snapshot(b, 'unique')
        return a, b, common, unique

    def run_union(self, sources, **kw):
        return consolidate(sources, [self.path / 'union.sqlite3', self.other / 'union.sqlite3'], self.pair, self.path, floors=(0, 0), **kw)

    def test_union_preserves_unique_snapshot_and_exact_source_rows(self):
        a, b, common, unique = self.catalogs()
        result = self.run_union([a, b], retire=True)
        self.assertEqual(result['snapshots'], 2)
        self.assertTrue(a.is_symlink())
        self.assertTrue(b.is_symlink())
        self.assertIn(unique, result['actual_restores'])
        restored = restore_catalog(self.other / 'union.sqlite3', b, self.path / 'old-logical.sqlite3')
        with closing(sqlite3.connect(restored)) as d:
            self.assertEqual(d.execute('SELECT count(*) FROM snapshots').fetchone()[0], 2)
            self.assertEqual(d.execute('SELECT count(*) FROM receipts').fetchone()[0], 2)
        self.assertFalse(result['sources'][0]['byte_exact_container_restore'])

    def test_unknown_auxiliary_table_refuses_without_retirement(self):
        a, b, common, unique = self.catalogs()
        with closing(sqlite3.connect(a)) as d:
            d.execute('CREATE TABLE unknown(important TEXT)')
            d.commit()
        with self.assertRaisesRegex(ValueError, 'Unknown'):
            self.run_union([a, b], retire=True)
        self.assertFalse(a.is_symlink())

    def test_corrupt_chunk_refuses_without_retirement(self):
        a, b, common, unique = self.catalogs()
        with closing(sqlite3.connect(b)) as d:
            d.execute('DROP TRIGGER chunks_no_update')
            d.execute("UPDATE chunks SET payload=x'00'")
            d.commit()
        with self.assertRaises(Exception):
            self.run_union([a, b], retire=True)
        self.assertFalse(a.is_symlink())
        self.assertFalse(b.is_symlink())

    def test_failed_snapshot_restore_retains_original_catalogs(self):
        a, b, common, unique = self.catalogs()
        with patch('snapshot_consolidation.Store.restore', side_effect=ValueError('restore failure')):
            with self.assertRaisesRegex(ValueError, 'restore failure'):
                self.run_union([a, b], retire=True)
        self.assertFalse(a.is_symlink())
        self.assertFalse(b.is_symlink())

    def test_source_catalog_restore_capacity_refuses_before_output(self):
        a, b, common, unique = self.catalogs()
        self.run_union([a, b])
        target = self.path / 'no-capacity.sqlite3'
        usage = shutil.disk_usage(self.path)
        with patch('snapshot_consolidation.shutil.disk_usage', return_value=usage._replace(free=0)):
            with self.assertRaisesRegex(ValueError, 'capacity'):
                restore_catalog(self.other / 'union.sqlite3', a, target)
        self.assertFalse(target.exists())

    def test_resume_verified_prefix_without_replacing_original_sources_early(self):
        a, b, common, unique = self.catalogs()
        with patch('snapshot_consolidation.Store.copy', side_effect=ValueError('interrupted verification')):
            with self.assertRaisesRegex(ValueError, 'interrupted'):
                self.run_union([a, b], retire=True)
        self.assertFalse(a.is_symlink())
        result = self.run_union([a, b], retire=True, resume=True)
        self.assertTrue(result['resumed'])
        self.assertTrue(a.is_symlink())
        self.assertTrue(b.is_symlink())

    def test_resume_refuses_unrecognized_destination_without_removal(self):
        a, b, common, unique = self.catalogs()
        target = self.path / 'union.sqlite3'
        with closing(sqlite3.connect(target)) as db:
            db.execute('CREATE TABLE unique_data(v TEXT)')
            db.execute("INSERT INTO unique_data VALUES('preserve')")
            db.commit()
        with self.assertRaisesRegex(ValueError, 'Unknown existing'):
            self.run_union([a, b], retire=True, resume=True)
        with closing(sqlite3.connect(target)) as db:
            self.assertEqual(db.execute('SELECT * FROM unique_data').fetchall(), [('preserve',)])
        self.assertFalse(a.is_symlink())

    def test_cli_constructs_private_union_while_shared_writer_is_leased(self):
        a, b, common, unique = self.catalogs()
        subprocess.run(['git', 'init', '-q', str(self.path)], check=True)
        policy = self.path / 'policy.json'
        policy.write_text(json.dumps(dict(schema=1, durable_copies=2, primary_minimum_free_gib=0, replica_minimum_free_gib=0)))
        for key, value in [('mm.evidencePrimary', self.pair.paths[0]), ('mm.evidenceReplica', self.pair.paths[1]), ('mm.evidenceStoragePolicy', policy)]:
            subprocess.run(['git', '-C', str(self.path), 'config', key, str(value)], check=True)
        first, second = self.path / 'cli-union.sqlite3', self.other / 'cli-union.sqlite3'
        command = [sys.executable, str(ROOT / 'tools/compact-evidence-storage.py'), '--root', str(self.path), 'snapshots',
                   '--source', str(a), '--source', str(b), '--destination', str(first), '--destination', str(second),
                   '--scratch', str(self.path), '--primary-minimum-free-gib', '0', '--replica-minimum-free-gib', '0']
        child = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        def stop():
            if child.poll() is None:
                child.terminate(); child.wait(timeout=5)
        self.addCleanup(stop)
        ready = False
        deadline = time.monotonic() + 15
        while child.poll() is None and time.monotonic() < deadline:
            if second.exists():
                try:
                    with closing(sqlite3.connect(second, timeout=0.1)) as db:
                        ready = db.execute('SELECT count(*) FROM source_catalogs').fetchone()[0] == 2
                except sqlite3.Error:
                    pass
            if ready:
                break
            time.sleep(0.03)
        self.assertTrue(ready, 'Private construction must progress while a different shared writer holds the lease')
        self.pair.close()
        stdout, stderr = child.communicate(timeout=15)
        self.assertEqual(child.returncode, 0, stderr.decode())
        result = json.loads(stdout)
        self.assertEqual(result['snapshots'], 2)
        self.assertTrue(result['all_distinct_source_catalogs_restored_and_compared'])


if __name__ == '__main__':
    unittest.main()
