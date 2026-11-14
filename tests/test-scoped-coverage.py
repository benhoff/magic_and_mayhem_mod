#!/usr/bin/env python3
"""Real Git changes, provenance scope, staged isolation and immutable claims."""

import copy
import importlib.util
import json
from pathlib import Path
import subprocess
import sys
import unittest
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "tools"))
from coverage_scoped import capture, write_index, INDEX, View, dependencies
from coverage_gate import REGISTER, REVIEWS, BASELINE, check, changes, draft
from coverage_evidence import Reader

spec = importlib.util.spec_from_file_location(
    "scoped_fixtures", ROOT / "tests/test-coverage-gate.py"
)
fixtures = importlib.util.module_from_spec(spec)
spec.loader.exec_module(fixtures)


class Scoped(unittest.TestCase):
    def setUp(self):
        self.f = fixtures.GateTests()
        self.f.setUp()
        self.addCleanup(self.f.doCleanups)
        self.root = self.f.root
        self.f.json(REVIEWS, dict(schema=1, reviews=[]))
        self.f.json(BASELINE, self.f.before)
        self.git("init", "-qb", "master")
        self.git("config", "user.name", "Fixture")
        self.git("config", "user.email", "fixture@example.invalid")
        self.git("config", "core.hooksPath", "/dev/null")
        self.git("add", ".")
        self.git("commit", "-qm", "base")
        write_index(self.root)
        self.f.register = json.loads((self.root / REGISTER).read_bytes())
        self.git("add", "research")
        self.git("commit", "-qm", "provenance")

    def git(self, *args):
        return (
            subprocess.check_output(
                ["git", "-C", str(self.root), *args], stderr=subprocess.PIPE
            )
            .decode()
            .strip()
        )

    def update(self):
        self.f.sync()

    def test_docs_only_does_not_open_unrelated_evidence_or_inventory(self):
        self.f.write("docs/note.md", "ordinary prose\n")
        with patch(
            "coverage_scoped.dependency",
            side_effect=AssertionError("historical evidence decoded"),
        ):
            a, b, r, j, c = capture(self.root, "HEAD")
        self.assertEqual(r["checked_scope"]["behaviors"], [])
        self.assertEqual(r["checked_scope"]["evidence"], [])
        self.assertEqual([v["path"] for v in changes(a, b)["files"]], ["docs/note.md"])

    def test_affected_engine_change_keeps_stale_proof_and_requires_receipt(self):
        self.f.write("assets/model.cpp", "new engine code\n")
        self.update()
        a, b, r, j, c = capture(self.root, "HEAD")
        self.assertIn("MV.test", r["checked_scope"]["behaviors"])
        self.assertTrue(r["findings"]["stale_evidence"])
        self.assertTrue(check(a, b, j, r)["errors"])
        receipt = draft(changes(a, b))["reviews"][0]
        receipt["reason"] = "Reviewed bounded change"
        receipt["validation"]["reason"] = "Original comparison remains pending"
        self.assertEqual(
            check(a, b, dict(schema=1, reviews=[receipt]), r)["errors"], []
        )

    def test_staged_source_ignores_unstaged_fix(self):
        self.f.write("assets/model.cpp", "staged wrong census\n")
        self.git("add", "assets/model.cpp")
        self.update()
        with self.assertRaisesRegex(ValueError, "census drift"):
            capture(self.root, "HEAD", True)

    def test_index_cannot_hide_historical_dependency(self):
        view = View(self.root)
        try:
            reg = view.value(REGISTER)
            data = view.value(reg[INDEX]["path"])
        finally:
            view.close()
        from coverage_gate import digest

        empty = digest([])
        data["path_sets"][empty] = []
        data["evidence"]["old-original"]["path_set"] = empty
        self.f.json("research/bad-index.json", data)
        self.f.register[INDEX] = dict(
            path="research/bad-index.json", sha256=self.f.sha("research/bad-index.json")
        )
        self.update()
        with self.assertRaisesRegex(ValueError, "dependencies rewritten"):
            capture(self.root, "HEAD")

    def test_changed_evidence_bytes_are_verified(self):
        self.f.json("research/record.json", dict(success=False))
        with self.assertRaisesRegex(
            ValueError, "evidence.*hash mismatch|evidence assertion"
        ):
            capture(self.root, "HEAD")

    def test_duplicate_metadata_and_original_edits_refuse(self):
        self.f.register["evidence"] *= 2
        self.update()
        with self.assertRaisesRegex(ValueError, "Duplicate evidence"):
            capture(self.root, "HEAD")

    def test_forged_promotion_needs_current_execution(self):
        self.f.register["behaviors"][0]["status"]["implementation"] = "scoped"
        self.update()
        a, b, r, j, c = capture(self.root, "HEAD")
        receipt = draft(changes(a, b))["reviews"][0]
        receipt["reason"] = "Promotion attempted"
        receipt["validation"]["reason"] = "Pending proof"
        self.assertTrue(
            any(
                "promotion" in e
                for e in check(a, b, dict(schema=1, reviews=[receipt]), r)["errors"]
            )
        )

    def test_new_code_without_census_refuses(self):
        self.f.write("assets/new.cpp", "new unindexed code\n")
        with self.assertRaisesRegex(ValueError, "census drift"):
            capture(self.root, "HEAD")

    def test_unlinked_collector_provenance_expands_claim_scope(self):
        self.f.write("tools/collector.py", "old collector\n")
        self.f.record["sources"]["tools/collector.py"] = self.f.sha(
            "tools/collector.py"
        )
        self.f.json("research/record.json", self.f.record)
        self.f.register["evidence"][0]["sha256"] = self.f.sha("research/record.json")
        self.update()
        write_index(self.root)
        self.f.register = json.loads((self.root / REGISTER).read_bytes())
        self.git("add", "tools", "research")
        self.git("commit", "-qm", "collector provenance")
        self.f.write("tools/collector.py", "changed collector\n")
        self.update()
        a, b, r, j, c = capture(self.root, "HEAD")
        self.assertIn("MV.test", r["checked_scope"]["behaviors"])
        self.assertIn("MV.test", [v["id"] for v in changes(a, b)["behaviors"]])
        self.assertTrue(r["findings"]["stale_evidence"])

    def test_mode_only_change_needs_exact_mode_review(self):
        (self.root / "tests/model.py").chmod(0o755)
        a, b, r, j, c = capture(self.root, "HEAD")
        delta = changes(a, b)
        row = next(v for v in delta["files"] if v["path"] == "tests/model.py")
        self.assertEqual((row["before_mode"], row["after_mode"]), ("100644", "100755"))
        self.assertTrue(check(a, b, j, r)["errors"])
        receipt = draft(delta)["reviews"][0]
        receipt["reason"] = "Reviewed executable mode"
        receipt["validation"]["reason"] = "Source bytes are unchanged"
        self.assertEqual(
            check(a, b, dict(schema=1, reviews=[receipt]), r)["errors"], []
        )

    def test_scoped_cli_checks_actual_index_without_full_snapshot(self):
        self.f.write("docs/note.md", "prose\n")
        a, b, r, j, c = capture(self.root, "HEAD")
        receipt = draft(changes(a, b))["reviews"][0]
        receipt["reason"] = "Reviewed prose"
        receipt["validation"]["reason"] = "No contract change"
        self.f.json(REVIEWS, dict(schema=1, reviews=[receipt]))
        self.git("add", "docs", REVIEWS)
        cli = fixtures.cli
        from contextlib import redirect_stdout
        import io

        with patch.object(
            cli, "snapshot", side_effect=AssertionError("whole corpus scanned")
        ), patch.object(
            sys,
            "argv",
            ["gate", "--root", str(self.root), "--base", "HEAD", "--staged"],
        ), redirect_stdout(
            io.StringIO()
        ) as out:
            self.assertEqual(cli.main(), 0, out.getvalue())

    def test_legacy_relative_alias_is_preserved_but_detects_canonical_change(self):
        self.f.record["sources"]["assets/../assets/model.cpp"] = self.f.record[
            "sources"
        ]["assets/model.cpp"]
        self.f.json("research/record.json", self.f.record)
        self.f.register["evidence"][0]["sha256"] = self.f.sha("research/record.json")
        self.update()
        write_index(self.root)
        self.f.register = json.loads((self.root / REGISTER).read_bytes())
        self.git("add", "research")
        self.git("commit", "-qm", "historical alias")
        self.f.write("assets/model.cpp", "changed canonical source\n")
        self.update()
        a, b, r, j, c = capture(self.root, "HEAD")
        self.assertIn("MV.test", r["checked_scope"]["behaviors"])
        self.assertIn(
            "assets/../assets/model.cpp", b["evidence_sources"]["old-original"]
        )
        self.assertTrue(r["findings"]["stale_evidence"])

    def add_range(self):
        data = dict(
            source_sha256="a" * 64,
            success=True,
            sources=self.f.record["sources"],
            anchors={"0x100a": "90"},
        )
        self.f.json("research/range.json", data)
        ev = copy.deepcopy(self.f.register["evidence"][0])
        ev.update(
            id="range-proof",
            record="research/range.json",
            sha256=self.f.sha("research/range.json"),
        )
        ev.pop("claims_pointer", None)
        self.f.register["evidence"].append(ev)
        self.f.register["recovered_ranges"].append(
            dict(
                id="entry-gap",
                build="fixture",
                start=4106,
                end=4112,
                document="research/finding.md",
                evidence="range-proof",
                anchors_pointer="/anchors",
            )
        )
        self.update()
        write_index(self.root)
        self.f.register = json.loads((self.root / REGISTER).read_bytes())
        self.git("add", "research")
        self.git("commit", "-qm", "range proof")

    def test_unreferenced_range_does_not_pull_missing_subset_evidence(self):
        self.add_range()
        self.f.write("research/finding.md", "prose scope review\n")
        a, b, r, j, c = capture(self.root, "HEAD")
        self.assertNotIn("range-proof", r["checked_scope"]["evidence"])
        self.assertEqual(r["errors"], [])

    def test_referenced_range_closes_over_its_original_proof(self):
        self.add_range()
        self.f.register["behaviors"][0]["original"] = [
            dict(
                build="fixture",
                address=4106,
                scope="entry",
                recovered_range="entry-gap",
            )
        ]
        self.update()
        a, b, r, j, c = capture(self.root, "HEAD")
        self.assertIn("range-proof", r["checked_scope"]["evidence"])
        self.assertEqual(r["errors"], [])

    def test_native_api_mapping_keeps_its_build_without_behavior_address(self):
        from coverage_scoped import subset

        reg = copy.deepcopy(self.f.register)
        reg["behaviors"][0]["original"] = []
        reg["behaviors"][0]["evidence"] = []
        reg["api_bindings"] = [dict(build="fixture", behaviors=["MV.test"])]
        selected = subset(reg, {"MV.test"}, set())
        self.assertEqual([b["id"] for b in selected["builds"]], ["fixture"])

    def test_recorded_validation_cannot_use_stale_execution(self):
        self.f.write("assets/model.cpp", "new code\n")
        self.update()
        a, b, r, j, c = capture(self.root, "HEAD")
        receipt = draft(changes(a, b))["reviews"][0]
        receipt["reason"] = "Attempt stale credit"
        receipt["validation"] = dict(
            status="recorded", reason="Old run", evidence=["old-original"]
        )
        self.assertTrue(
            any(
                "stale" in e
                for e in check(a, b, dict(schema=1, reviews=[receipt]), r)["errors"]
            )
        )


if __name__ == "__main__":
    unittest.main()
