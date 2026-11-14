#!/usr/bin/env python3
"""Real Git candidates: expensive validation outside admission, refused races."""

from contextlib import contextmanager
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "tools"))
import integration_batch as batch
from integration_queue import integration_lock, status


class Candidate(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.main = Path(self.tmp.name) / "main"
        self.main.mkdir()
        self.root = Path(self.tmp.name) / "owned"
        self.git(self.main, "init", "-qb", "master")
        self.git(self.main, "config", "user.name", "Fixture")
        self.git(self.main, "config", "user.email", "fixture@example.invalid")
        self.git(self.main, "config", "core.hooksPath", "/dev/null")
        (self.main / ".gitignore").write_text("working/\n")
        (self.main / "source.txt").write_text("baseline\n")
        self.git(self.main, "add", ".")
        self.git(self.main, "commit", "-qm", "base")
        self.base = self.git(self.main, "rev-parse", "HEAD")
        self.git(self.main, "switch", "-qc", "ready")
        (self.main / "source.txt").write_text("tested feature\n")
        self.git(self.main, "commit", "-qam", "ready")
        self.tip = self.git(self.main, "rev-parse", "HEAD")
        self.git(self.main, "switch", "master")
        self.git(self.main, "worktree", "add", "-qb", "owned", str(self.root), self.tip)
        self.events = []
        self.mutation = None
        self.validation_error = False

    def git(self, root, *args):
        return (
            subprocess.check_output(
                ["git", "-C", str(root), *args], stderr=subprocess.PIPE
            )
            .decode()
            .strip()
        )

    def invoke(self):
        original = subprocess.run

        def run(command, *args, **kwargs):
            if any(str(c).endswith("/finish-work.py") for c in command):
                self.events.append(("validation", status(self.root)["busy"]))
                self.assertEqual(
                    (self.root / "source.txt").read_text(), "tested feature\n"
                )
                if self.validation_error:
                    raise subprocess.CalledProcessError(1, command)
                self.git(self.root, "commit", "-qm", "checked candidate")
                if self.mutation == "master":
                    (self.main / "other.txt").write_text("concurrent ready work\n")
                    self.git(self.main, "add", "other.txt")
                    self.git(self.main, "commit", "-qm", "independent master")
                checked = self.git(self.root, "rev-parse", "HEAD")
                tree = self.git(self.root, "rev-parse", "HEAD^{tree}")
                token = command[command.index("--result-token") + 1]
                if self.mutation == "head":
                    (self.root / "unvalidated.txt").write_text(
                        "unvalidated successor\n"
                    )
                    self.git(self.root, "add", "unvalidated.txt")
                    self.git(self.root, "commit", "-qm", "unvalidated successor")
                result = f"MM_FINISH_CHECKED {token} {checked} {tree} {self.base}\n"
                if self.mutation == "receipt":
                    result = result.replace(token, "0" * 32)
                return subprocess.CompletedProcess(command, 0, stdout=result)
            if "--ff-only" in command:
                self.events.append(("promotion", status(self.root)["busy"]))
            return original(command, *args, **kwargs)

        @contextmanager
        def lock(root, **kwargs):
            with integration_lock(root, **kwargs) as lease:
                if self.mutation == "candidate":
                    (self.root / "source.txt").write_text("raced after checks\n")
                yield lease

        with patch.object(batch, "write_report", return_value="a" * 64), patch.object(
            batch, "review_history", return_value=dict(missing_receipt_count=0)
        ), patch.object(batch, "integration_lock", lock), patch.object(
            batch.subprocess, "run", side_effect=run
        ):
            return batch.execute(
                self.root,
                [self.tip],
                branch="integrate/fixture",
                report="research/runtime/integration.json",
                reason="Reviewed candidate",
                validation_reason="Fixture validation",
                index_reason="Source links checked",
                message="checked",
                database="working/review.sqlite3",
                expected_master=self.base,
            )

    def test_only_checked_promotion_holds_lease(self):
        value = self.invoke()
        self.assertEqual(self.events, [("validation", False), ("promotion", True)])
        self.assertTrue(value["validation_outside_lock"])
        self.assertEqual(self.git(self.main, "rev-parse", "HEAD"), value["master"])
        self.assertFalse(status(self.root)["busy"])

    def test_changed_master_preserves_candidate_and_refuses_promotion(self):
        self.mutation = "master"
        with self.assertRaisesRegex(ValueError, "Master changed"):
            self.invoke()
        self.assertNotEqual(
            self.git(self.main, "rev-parse", "HEAD"),
            self.git(self.root, "rev-parse", "HEAD"),
        )
        self.assertNotIn(("promotion", True), self.events)
        self.assertFalse(status(self.root)["busy"])

    def test_changed_candidate_is_refused_under_short_lease(self):
        self.mutation = "candidate"
        with self.assertRaisesRegex(ValueError, "Candidate changed"):
            self.invoke()
        self.assertEqual(self.git(self.main, "rev-parse", "HEAD"), self.base)
        self.assertFalse(status(self.root)["busy"])

    def test_unchecked_successor_after_finish_is_not_promoted(self):
        self.mutation = "head"
        with self.assertRaisesRegex(ValueError, "Candidate changed after validation"):
            self.invoke()
        self.assertEqual(self.git(self.main, "rev-parse", "HEAD"), self.base)
        self.assertFalse(status(self.root)["busy"])

    def test_stale_finish_receipt_is_not_accepted(self):
        self.mutation = "receipt"
        with self.assertRaisesRegex(ValueError, "this invocation"):
            self.invoke()
        self.assertEqual(self.git(self.main, "rev-parse", "HEAD"), self.base)

    def test_preparation_refuses_existing_admission_before_changes(self):
        with integration_lock(self.root):
            with self.assertRaisesRegex(ValueError, "outside promotion admission"):
                self.invoke()
        self.assertEqual(self.events, [])
        self.assertFalse(status(self.root)["busy"])

    def test_failed_validation_never_requests_admission(self):
        self.validation_error = True
        with patch.object(
            batch, "integration_lock", side_effect=AssertionError("admission requested")
        ):
            with self.assertRaises(subprocess.CalledProcessError):
                self.invoke()
        self.assertEqual(self.events, [("validation", False)])
        self.assertEqual(self.git(self.main, "rev-parse", "HEAD"), self.base)
        self.assertFalse(status(self.root)["busy"])


if __name__ == "__main__":
    unittest.main()
