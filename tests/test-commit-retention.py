#!/usr/bin/env python3
"""Exact changed-batch custody with independent stores and corruption controls."""

import importlib.util
import io
import json
from pathlib import Path
import subprocess
import sys
import unittest
from unittest.mock import patch
import zlib

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "tools"))
from commit_retention import retain_staged_batch
from coverage_evidence import Store, pointer
import evidence_objects as objects

spec = importlib.util.spec_from_file_location(
    "retention_fixtures", ROOT / "tests/test-finish-performance.py"
)
fixtures = importlib.util.module_from_spec(spec)
spec.loader.exec_module(fixtures)


class Batch(unittest.TestCase):
    def setUp(self):
        self.f = fixtures.Backups()
        self.f.setUp()
        self.addCleanup(self.f.doCleanups)
        self.root = self.f.root
        self.git("config", "user.name", "Fixture")
        self.git("config", "user.email", "fixture@example.invalid")
        self.git("config", "core.hooksPath", "/dev/null")
        (self.root / ".gitignore").write_text("working/\n")
        (self.root / "source.py").write_text("baseline\n")
        self.git("add", ".gitignore", "source.py")
        self.git("commit", "-qm", "base")
        self.base = self.git("rev-parse", "HEAD")
        with objects.configured(self.root) as pair:
            self.old = pair.bytes(b"unrelated historical capture" * 1000)

    def git(self, *args):
        return (
            subprocess.check_output(
                ["git", "-C", str(self.root), *args], stderr=subprocess.PIPE
            )
            .decode()
            .strip()
        )

    def stage(self):
        (self.root / "source.py").write_text("exact new source\n")
        self.git("add", "source.py")

    def corrupt(self, path, sha):
        s = Store(path)
        try:
            self.f.unguard(s, "blocks")
            with s.db:
                s.db.execute(
                    "UPDATE blocks SET payload=? WHERE sha256=(SELECT block FROM object_parts WHERE sha256=? LIMIT 1)",
                    (b"corrupt", sha),
                )
        finally:
            s.close()

    def test_only_changed_source_is_retained_and_global_scrub_is_not_run(self):
        self.stage()
        self.corrupt(self.f.replica, self.old)
        with patch.object(
            Store, "verify", side_effect=AssertionError("global scrub requested")
        ):
            result = retain_staged_batch(self.root, self.base)
        self.assertEqual(result["source_files"], 1)
        self.assertFalse(result["retirement_authorized"])
        with objects.configured(self.root) as pair:
            for n in (0, 1):
                data = json.loads(
                    pair.read(
                        result["manifest"]["sha256"], result["manifest"]["size"], n
                    )
                )
                m = data["source_members"][0]
                self.assertEqual(
                    pair.read(m["sha256"], m["size"], n), b"exact new source\n"
                )
                self.assertEqual(m["git_mode"], "100644")

    def test_new_report_payload_is_mirrored_before_commit(self):
        body = b'{"success":true}\n'
        with objects.configured(self.root) as pair:
            with pair.stores[0].db:
                sha = pair.stores[0].put(io.BytesIO(body), len(body))
        (self.root / "report.json").write_bytes(pointer(sha, len(body)))
        self.git("add", "report.json")
        result = retain_staged_batch(self.root, self.base)
        self.assertEqual(result["report_payloads"], 1)
        with objects.configured(self.root) as pair:
            self.assertEqual(pair.read(sha, len(body), 1), body)

    def test_cited_bulk_is_promoted_to_essential_even_if_two_bytes_exist(self):
        policy = self.root / "storage.json"
        data = json.loads(policy.read_bytes())
        data.update(schema=2, bulk_copies=1)
        policy.write_text(json.dumps(data))
        body = b"previous single-copy workspace bulk"
        with objects.configured(self.root) as pair:
            sha = pair.put(io.BytesIO(body), len(body), copies=1)
            with pair.stores[1].db:
                pair.stores[1].put(io.BytesIO(body), len(body))
        (self.root / "cited.bin").write_bytes(pointer(sha, len(body)))
        self.git("add", "cited.bin")
        retain_staged_batch(self.root, self.base)
        with objects.configured(self.root) as pair:
            self.assertEqual(pair.object_copies(sha), 2)

    def test_corruption_of_consumed_second_copy_refuses(self):
        body = b'{"success":true}\n'
        with objects.configured(self.root) as pair:
            sha = pair.bytes(body)
        self.corrupt(self.f.replica, sha)
        (self.root / "report.json").write_bytes(pointer(sha, len(body)))
        self.git("add", "report.json")
        with self.assertRaises((ValueError, zlib.error)):
            retain_staged_batch(self.root, self.base)
        self.assertEqual(self.git("rev-parse", "HEAD"), self.base)

    def test_changed_index_preserves_root_but_refuses_success(self):
        self.stage()
        original = objects.Pair.root

        def mutate(pair, *args, **kwargs):
            result = original(pair, *args, **kwargs)
            (self.root / "source.py").write_text("late staged mutation\n")
            self.git("add", "source.py")
            return result

        with patch.object(objects.Pair, "root", mutate):
            with self.assertRaisesRegex(ValueError, "Index changed"):
                retain_staged_batch(self.root, self.base)
        self.assertEqual(self.git("rev-parse", "HEAD"), self.base)


if __name__ == "__main__":
    unittest.main()
