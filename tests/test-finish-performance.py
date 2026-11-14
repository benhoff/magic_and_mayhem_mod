#!/usr/bin/env python3
"""Fresh-integrity and failure controls for faster finish backups."""
from contextlib import closing
import hashlib
import io
import json
import os
import random
from pathlib import Path
import sqlite3
import subprocess
import sys
import tempfile
import time
import unittest
import zlib
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
import evidence_objects as objects


class Backups(unittest.TestCase):
    def setUp(self):
        self.ram = tempfile.TemporaryDirectory(prefix='mm-finish-performance-')
        self.addCleanup(self.ram.cleanup)
        parent = ROOT / 'working/tests/finish-performance'
        parent.mkdir(parents=True, exist_ok=True)
        self.disk = tempfile.TemporaryDirectory(prefix='run-', dir=parent)
        self.addCleanup(self.disk.cleanup)
        self.root = Path(self.ram.name)
        self.primary = self.root / 'working/primary.sqlite3'
        self.replica = Path(self.disk.name) / 'replica.sqlite3'
        subprocess.run(['git', 'init', '-q', str(self.root)], check=True)
        policy = self.root / 'storage.json'
        policy.write_text(json.dumps(dict(schema=1, durable_copies=2,
            primary_minimum_free_gib=0, replica_minimum_free_gib=0)))
        for key, value in [('mm.evidencePrimary', self.primary), ('mm.evidenceReplica', self.replica),
                           ('mm.evidenceStoragePolicy', policy)]:
            subprocess.run(['git', '-C', str(self.root), 'config', key, str(value)], check=True)
        self.source = self.root / 'working/source.sqlite3'
        self.source.parent.mkdir(exist_ok=True)
        self.payload = b'original negative evidence\0' * 2048
        with closing(sqlite3.connect(self.source)) as db, db:
            db.execute('CREATE TABLE t(value TEXT, payload BLOB)')
            db.execute('INSERT INTO t(rowid,value,payload) VALUES(17,?,?)', ('text\0tail', self.payload))
            db.execute('CREATE TABLE drafts(id TEXT PRIMARY KEY,value TEXT) WITHOUT ROWID')
            db.execute('INSERT INTO drafts VALUES(?,?)', ('held', 'pending unique draft'))
        self.source.chmod(0o444)

    def unguard(self, store, table):
        for name, in store.db.execute("SELECT name FROM sqlite_master WHERE type='trigger' AND tbl_name=?", (table,)).fetchall():
            store.db.execute('DROP TRIGGER "' + name.replace('"', '""') + '"')

    def test_commit_checkpoint_keeps_complete_records_without_restore_credit(self):
        with patch.object(objects,'restore_logical',wraps=objects.restore_logical)as observed:
            result=objects.logical_checkpoint(self.root,self.source,verify_restore=False)
            self.assertFalse(result['logical_restore_verified'])
            self.assertFalse(result['real_restore_performed'])
            self.assertFalse(result['retirement_authorized'])
            self.assertEqual(observed.call_count,0)
            retired=objects.logical_checkpoint(self.root,self.source)
            self.assertTrue(retired['logical_restore_verified'])
            self.assertEqual(observed.call_count,1)

    def test_unchanged_readonly_reuses_restore_proof_but_freshly_hashes_both_copies(self):
        restore = objects.restore_logical
        with patch.object(objects, 'restore_logical', wraps=restore) as observed:
            first = objects.logical_checkpoint(self.root, self.source)
            self.assertFalse(first['checkpoint_reused'])
            self.assertTrue(first['real_restore_performed'])
            with patch.object(objects, '_sqlite_catalog', side_effect=AssertionError('catalog rebuilt')):
                second = objects.logical_checkpoint(self.root, self.source)
            self.assertEqual(observed.call_count, 1)
        self.assertTrue(second['checkpoint_reused'])
        self.assertFalse(second['real_restore_performed'])
        self.assertEqual(first['manifest'], second['manifest'])
        self.assertEqual(second['source_sha256'], hashlib.sha256(self.source.read_bytes()).hexdigest())
        self.assertFalse(second['byte_exact_container'])

    def test_mutable_and_wal_sources_never_reuse(self):
        self.source.chmod(0o644)
        first = objects.logical_checkpoint(self.root, self.source)
        second = objects.logical_checkpoint(self.root, self.source)
        self.assertFalse(first['checkpoint_reused'] or second['checkpoint_reused'])
        db = sqlite3.connect(self.source)
        try:
            db.execute('PRAGMA journal_mode=WAL')
            db.execute('INSERT INTO t VALUES(?,?)', ('committed WAL', b'new')); db.commit()
            self.source.chmod(0o444)
            self.assertIsNone(objects._sealed_source(self.source))
            result = objects.logical_checkpoint(self.root, self.source)
            self.assertFalse(result['checkpoint_reused'])
        finally:
            db.close()

    def test_changed_readonly_source_rebuilds_and_preserves_previous_proof(self):
        first = objects.logical_checkpoint(self.root, self.source)
        self.source.chmod(0o644)
        with closing(sqlite3.connect(self.source)) as db, db:
            db.execute('INSERT INTO t VALUES(?,?)', ('new state', b'changed'))
        self.source.chmod(0o444)
        second = objects.logical_checkpoint(self.root, self.source)
        self.assertFalse(second['checkpoint_reused'])
        self.assertNotEqual(first['manifest'], second['manifest'])
        with objects.configured(self.root) as pair:
            self.assertEqual(pair.stores[0].db.execute("SELECT COUNT(*) FROM retention_roots WHERE kind='sqlite_logical_verified'").fetchone()[0], 2)
        self.assertTrue(objects.logical_checkpoint(self.root, self.source)['checkpoint_reused'])

    def test_codec_drift_requires_a_new_restore(self):
        objects.logical_checkpoint(self.root, self.source)
        with patch.object(objects, '_checkpoint_codec', return_value='changed codec'):
            self.assertFalse(objects.logical_checkpoint(self.root, self.source)['checkpoint_reused'])

    def test_failed_restore_does_not_admit_a_cached_proof(self):
        with patch.object(objects, 'restore_logical', side_effect=ValueError('restore failed')):
            with self.assertRaisesRegex(ValueError, 'restore failed'):
                objects.logical_checkpoint(self.root, self.source)
        with objects.configured(self.root) as pair:
            self.assertEqual(pair.stores[0].db.execute("SELECT COUNT(*) FROM retention_roots WHERE kind='sqlite_logical_verified'").fetchone()[0], 0)
        self.assertFalse(objects.logical_checkpoint(self.root, self.source)['checkpoint_reused'])

    def test_source_change_during_restore_prevents_reuse_marker(self):
        restore = objects.restore_logical
        def changed(*args, **kwargs):
            value = restore(*args, **kwargs)
            self.source.chmod(0o644)
            with closing(sqlite3.connect(self.source)) as db, db:
                db.execute('INSERT INTO t VALUES(?,?)', ('after snapshot', b'new'))
            self.source.chmod(0o444)
            return value
        with patch.object(objects, 'restore_logical', side_effect=changed):
            self.assertFalse(objects.logical_checkpoint(self.root, self.source)['checkpoint_reused'])
        self.assertFalse(objects.logical_checkpoint(self.root, self.source)['checkpoint_reused'])

    def test_corrupt_replica_leaf_is_detected_on_cache_hit(self):
        objects.logical_checkpoint(self.root, self.source)
        sha = hashlib.sha256(self.payload).hexdigest()
        with objects.configured(self.root) as pair:
            store = pair.stores[1]
            with store.db:
                self.unguard(store, 'blocks')
                block = store.db.execute('SELECT block FROM object_parts WHERE sha256=?', (sha,)).fetchone()[0]
                store.db.execute('UPDATE blocks SET payload=? WHERE sha256=?', (b'corrupt', block))
        with self.assertRaises((ValueError, zlib.error)):
            objects.logical_checkpoint(self.root, self.source)
        self.assertEqual(self.source.read_bytes()[:16], b'SQLite format 3\0')

    def test_missing_replica_proof_is_not_called_verified(self):
        objects.logical_checkpoint(self.root, self.source)
        with objects.configured(self.root) as pair:
            with pair.stores[1].db:
                self.unguard(pair.stores[1], 'retention_roots')
                pair.stores[1].db.execute("DELETE FROM retention_roots WHERE kind='sqlite_logical_verified'")
        with self.assertRaisesRegex(ValueError, 'missing.*proof'):
            objects.logical_checkpoint(self.root, self.source)

    def test_source_checksum_runs_again_even_with_unchanged_timestamps(self):
        objects.logical_checkpoint(self.root, self.source)
        with patch.object(objects.hashlib, 'file_digest', wraps=hashlib.file_digest) as observed:
            self.assertTrue(objects.logical_checkpoint(self.root, self.source)['checkpoint_reused'])
        self.assertEqual(observed.call_count, 1)

    def test_synced_catalog_uses_bounded_queries_not_one_has_per_object(self):
        with objects.configured(self.root) as pair:
            with pair.batch():
                for n in range(1000):
                    pair.bytes(('immutable object '+str(n)).encode())
                pair.root('owned', 'fixture', dict(negative=True))
            statements = []
            for store in pair.stores: store.db.set_trace_callback(statements.append)
            result = objects._checkpoint_objects(pair)
            self.assertEqual(result['transferred_objects'], 0)
            lookups = [q for q in statements if q.startswith('SELECT size FROM objects WHERE')]
            self.assertLess(len(lookups), 20)
            self.assertEqual(result['catalog_comparison'], 'bulk-sql')

    def test_replica_loss_is_repaired_including_roots_and_git_bindings(self):
        with objects.configured(self.root) as pair:
            ref = pair.root('owned', 'source', dict(value='retained negative'))
            with pair.stores[0].db:
                pair.stores[0].db.execute('INSERT INTO versions VALUES(?,?,?)', ('a'*40, 'research/runtime/test.json', ref['sha256']))
        self.replica.unlink()
        result = objects.checkpoint_objects(self.root)
        self.assertGreater(result['transferred_objects'], 0)
        with objects.configured(self.root) as pair:
            objects.verify_pair(pair)

    def test_extra_replica_objects_are_retained_and_refused(self):
        with objects.configured(self.root) as pair:
            with pair.stores[1].db:
                sha = pair.stores[1].put(io.BytesIO(b'unique'), 6)
            with self.assertRaisesRegex(ValueError, 'divergent'):
                objects._checkpoint_objects(pair)
            self.assertTrue(pair.stores[1].has(sha))

    def test_existing_root_verification_runs_without_the_writer_lease(self):
        with objects.configured(self.root) as pair:
            ref = pair.root('parallel', 'fixture', dict(value='retained negative'))
            # Pair still owns the exclusive writer lease. A second Pair would
            # block here; independent read-only verification must finish.
            verified = objects.verify_existing_roots(self.root)
            self.assertIn(('parallel', 'fixture', ref['sha256'], ref['size']), verified)

    def test_new_roots_after_readonly_verification_still_get_locked_verification(self):
        with objects.configured(self.root) as pair:
            old = pair.root('parallel', 'old', dict(value='old'))
        verified = objects.verify_existing_roots(self.root)
        with objects.configured(self.root) as pair:
            new = pair.root('parallel', 'new', dict(value='new'))
            original = pair.read
            calls = []
            def observed(sha, *args):
                calls.append(sha)
                return original(sha, *args)
            with patch.object(pair, 'read', side_effect=observed):
                self.assertTrue(objects._checkpoint_objects(pair, verified_roots=verified)['success'])
            self.assertNotIn(old['sha256'], calls)
            self.assertEqual(calls.count(new['sha256']), 2)

    def test_readonly_root_verification_detects_replica_corruption(self):
        with objects.configured(self.root) as pair:
            ref = pair.root('parallel', 'fixture', dict(value='negative'))
            store = pair.stores[1]
            with store.db:
                self.unguard(store, 'blocks')
                block = store.db.execute('SELECT block FROM object_parts WHERE sha256=?', (ref['sha256'],)).fetchone()[0]
                store.db.execute('UPDATE blocks SET payload=? WHERE sha256=?', (b'broken', block))
        with self.assertRaises((ValueError, zlib.error)):
            objects.checkpoint_objects(self.root)

    def test_conflicting_git_bindings_refuse_without_rewriting(self):
        with objects.configured(self.root) as pair:
            a, b = pair.bytes(b'first'), pair.bytes(b'second')
            for store, sha in zip(pair.stores, (a, b)):
                with store.db: store.db.execute('INSERT INTO versions VALUES(?,?,?)', ('b'*40, 'same', sha))
            with self.assertRaisesRegex(ValueError, 'binding conflict'):
                objects._checkpoint_objects(pair)
            self.assertEqual(pair.stores[1].db.execute('SELECT sha256 FROM versions').fetchone()[0], b)


def benchmark():
    """Bounded reproducible workload; stdout only, no durable report by default."""
    fixture = Backups('runTest')
    fixture.setUp()
    try:
        fixture.source.chmod(0o644)
        body = random.Random(20261108).randbytes(8*1024**2)
        with closing(sqlite3.connect(fixture.source)) as db, db:
            db.execute('INSERT INTO t VALUES(?,?)', ('bounded benchmark', body))
        fixture.source.chmod(0o444)
        timings = {}
        def timed(name, call):
            started = time.perf_counter()
            value = call()
            timings[name] = time.perf_counter()-started
            return value
        first = timed('first_checkpoint_seconds', lambda: objects.logical_checkpoint(fixture.root, fixture.source))
        second = timed('reused_checkpoint_seconds', lambda: objects.logical_checkpoint(fixture.root, fixture.source))
        with objects.configured(fixture.root) as pair:
            with pair.batch():
                for n in range(5000):
                    pair.bytes(('bounded catalog object '+str(n)).encode())
            rows = list(pair.stores[0].db.execute('SELECT sha256,size FROM objects'))
            target = pair.stores[1]
            queries = []
            target.db.set_trace_callback(queries.append)
            assert timed('per_object_probe_seconds', lambda: all(target.has(*r) for r in rows))
            old_queries = len(queries);queries.clear()
            mirrored = timed('bulk_checkpoint_seconds', lambda: objects._checkpoint_objects(pair))
            bulk_queries = len(queries)
            target.db.set_trace_callback(None)
        from coverage_evidence import MAGIC, Reader
        (fixture.root/'.evidence-store.json').write_text('{"schema":1,"archives":[]}')
        name = fixture.root/'research/runtime/benchmark.json'
        name.parent.mkdir(parents=True)
        with objects.configured(fixture.root) as pair:
            payload = json.dumps(dict(value='fresh verified read '*30000)).encode()
            sha = pair.bytes(payload)
        name.write_bytes(MAGIC+sha.encode()+b' '+str(len(payload)).encode()+b'\n')
        reads = {}
        for memoize in (False, True):
            with patch.dict(os.environ, MM_EVIDENCE_DB=str(fixture.primary)), Reader(fixture.root,memoize=memoize) as reader:
                with patch.object(reader.resolver, 'copy', wraps=reader.resolver.copy) as copied:
                    def workload():
                        for _ in range(20):
                            reader.digest(name);reader.json(name)
                    timed('reader_memoized_seconds' if memoize else 'reader_default_seconds', workload)
                    reads[str(memoize)] = copied.call_count
        return dict(schema=1,kind='bounded_finish_performance_benchmark',timings=timings,
            source_bytes=fixture.source.stat().st_size,source_sha256=hashlib.sha256(fixture.source.read_bytes()).hexdigest(),
            first_real_restore=first['real_restore_performed'],reused_real_restore=second['real_restore_performed'],
            matching_manifests=first['manifest']==second['manifest'],objects=mirrored['objects'],
            transferred_objects=mirrored['transferred_objects'],per_object_queries=old_queries,bulk_queries=bulk_queries,
            reader_payload_bytes=len(payload),reader_resolutions=reads,
            scope='Synthetic RAM-primary/disk-replica fixture. Probe baseline excludes old root/version work. No full-repository merge-time prediction.')
    finally:
        fixture.doCleanups()


if __name__ == '__main__':
    if sys.argv[1:] == ['--benchmark']:
        print(json.dumps(benchmark(),indent=2))
    else:
        unittest.main()
