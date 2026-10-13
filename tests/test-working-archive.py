#!/usr/bin/env python3
"""Verify archive crash boundaries, deduplication, restoration and path guards."""
import json
import os
from pathlib import Path
import tempfile
import time
import unittest
from unittest.mock import patch
import sys
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
import working_archive as storage


class ColdStorage(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.root = Path(self.tmp.name)
        self.destination = self.root / 'cold'
        self.destination.mkdir()
        self.archive = storage.Archive(self.root, self.destination, 0)

    def payload(self, name='one.bin'):
        path = self.root / 'working/tests/unit/run-old' / name
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_bytes(bytes(range(256)) * 8192)
        return path

    def test_verified_prune_and_exact_restore(self):
        path = self.payload()
        before = path.read_bytes()
        mode = path.stat().st_mode & 0o777
        mtime = path.stat().st_mtime_ns
        released = self.archive.store(path, True)
        self.assertGreater(released, 0)
        self.assertFalse(path.exists())
        storage.restore(self.root, self.destination, path.relative_to(self.root).as_posix())
        self.assertEqual(path.read_bytes(), before)
        self.assertEqual(path.stat().st_mode & 0o777, mode)
        self.assertEqual(path.stat().st_mtime_ns, mtime)

    def test_shared_bytes_create_one_object(self):
        self.archive.store(self.payload('one.bin'))
        self.archive.store(self.payload('two.bin'))
        self.assertEqual(len(list((self.destination / 'objects').rglob('*.zst'))), 1)

    def test_cli_fallback_roundtrip(self):
        path = self.payload()
        expected = path.read_bytes()
        with patch.object(storage, 'ZSTD', None):
            archive = storage.Archive(self.root, self.destination, 0)
            archive.store(path, True)
            storage.restore(self.root, self.destination, path.relative_to(self.root).as_posix())
        self.assertEqual(path.read_bytes(), expected)

    def test_failed_verification_never_prunes(self):
        path = self.payload()
        with patch.object(storage, 'verify_object', side_effect=ValueError('corrupt archive')):
            with self.assertRaises(ValueError):
                self.archive.store(path, True)
        self.assertTrue(path.exists())
        self.assertFalse(list((self.destination / 'objects').rglob('*.zst')))

    def test_failed_receipt_never_prunes(self):
        path = self.payload()
        with patch.object(self.archive, 'record', side_effect=OSError('no disk space')):
            with self.assertRaises(OSError):
                self.archive.store(path, True)
        self.assertTrue(path.exists())

    def test_changed_destination_not_overwritten(self):
        path = self.payload()
        self.archive.store(path, True)
        path.write_bytes(b'new active data')
        with self.assertRaises(ValueError):
            storage.restore(self.root, self.destination, path.relative_to(self.root).as_posix())
        self.assertEqual(path.read_bytes(), b'new active data')

    def test_corrupt_blob_prevents_restore(self):
        path = self.payload()
        self.archive.store(path, True)
        next((self.destination / 'objects').rglob('*.zst')).write_bytes(b'bad')
        with self.assertRaises(ValueError):
            storage.restore(self.root, self.destination, path.relative_to(self.root).as_posix())
        self.assertFalse(path.exists())

    def test_originals_and_parent_symlinks_refused(self):
        for name in ('original/game.exe', 'working/../original/game.exe', '/tmp/file'):
            with self.assertRaises(ValueError):
                storage.safe_source(self.root, name)
        (self.root / 'working').mkdir()
        (self.root / 'working/alias').symlink_to(self.destination, target_is_directory=True)
        with self.assertRaises(ValueError):
            storage.safe_source(self.root, 'working/alias/payload.bin')

    def test_pruning_stops_when_free_space_reserve_is_restored(self):
        from types import SimpleNamespace
        path = self.payload()
        second = self.payload('two.bin')
        rules = {'archive_root': str(self.destination), 'max_working_gib': None,
                 'minimum_free_gib': 100}
        with patch.object(storage.subprocess, 'check_output', return_value=b'21474836480 working'), \
                patch.object(storage, 'candidate_runs', return_value=[(0, path.parent, [(2,path),(1,second)])]), \
                patch.object(storage, 'active_paths', return_value=set()), \
                patch.object(storage, 'Archive') as archive, \
                patch.object(storage.shutil, 'disk_usage', side_effect=[SimpleNamespace(free=n*storage.GIB) for n in (99,99,100)]):
            archive.return_value.store.return_value = storage.GIB
            result = storage.maintain(self.root, rules, prune=True)
        archive.return_value.store.assert_called_once_with(path, True)
        self.assertIsNone(result['target_bytes'])
        self.assertEqual(result['minimum_free_bytes'], 100*storage.GIB)
        self.assertEqual(result['files_verified'], 1)

    def test_pruning_keeps_legacy_size_target_and_free_space_reserve(self):
        from types import SimpleNamespace
        path = self.payload()
        second = self.payload('two.bin')
        rules = {'archive_root': str(self.destination), 'max_working_gib': 10,
                 'minimum_free_gib': 100}
        with patch.object(storage.subprocess, 'check_output', return_value=b'21474836480 working'), \
                patch.object(storage, 'candidate_runs', return_value=[(0, path.parent, [(2,path),(1,second)])]), \
                patch.object(storage, 'active_paths', return_value=set()), \
                patch.object(storage, 'Archive') as archive, \
                patch.object(storage.shutil, 'disk_usage', return_value=SimpleNamespace(free=200*storage.GIB)):
            archive.return_value.store.return_value = 10*storage.GIB
            result = storage.maintain(self.root, rules, prune=True)
        archive.return_value.store.assert_called_once_with(path, True)
        self.assertEqual(result['target_bytes'], 10*storage.GIB)

    def test_selection_protects_active_recent_pinned_and_source_trees(self):
        old = time.time() - 7200
        base = self.root / 'working/tests/unit'
        for name in ('run-cold', 'run-active', 'run-pinned', 'run-source', 'run-recent'):
            p = base / name
            p.mkdir(parents=True)
            (p / 'payload.bin').write_bytes(b'x' * 1024**2)
        (base / 'run-pinned/.retain').touch()
        (base / 'run-source/AGENTS.md').write_text('source snapshot')
        for path in base.rglob('*'):
            os.utime(path, (old, old))
        os.utime(base / 'run-recent/payload.bin', None)
        rules = {'keep_latest_runs': 0, 'archive_quiet_minutes': 60}
        with patch.object(storage, 'active_paths', return_value={base / 'run-active/payload.bin'}):
            selected = storage.candidate_runs(self.root, rules)
        self.assertEqual([p.name for _, p, _ in selected], ['run-cold'])


if __name__ == '__main__':
    unittest.main()
