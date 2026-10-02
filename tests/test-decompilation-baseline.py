#!/usr/bin/env python3
import importlib.util
import json
from pathlib import Path
import shutil
import tempfile
import unittest

REPO = Path(__file__).resolve().parent.parent
spec = importlib.util.spec_from_file_location("baseline", REPO / "tools/verify-decompilation-baseline.py")
tool = importlib.util.module_from_spec(spec)
spec.loader.exec_module(tool)


class BaselineTests(unittest.TestCase):
    def test_tracked_checksums(self):
        self.assertEqual(tool.verify(tool.BASELINE), (14, 0))

    def test_binary_bytes(self):
        artifacts, chunks = tool.verify(tool.BASELINE, REPO / "working/game-nocd/Chaos.exe")
        self.assertEqual(artifacts, 14)
        self.assertGreater(chunks, 1000)

    def test_tampered_copy_is_rejected(self):
        with tempfile.TemporaryDirectory() as temporary:
            snapshot = Path(temporary) / "snapshot"
            shutil.copytree(tool.BASELINE, snapshot)
            (snapshot / "raw/00512800.c").write_text("modified baseline")
            with self.assertRaisesRegex(ValueError, "checksum mismatch"):
                tool.verify(snapshot)

    def test_manifest_escape_is_rejected(self):
        with tempfile.TemporaryDirectory() as temporary:
            snapshot = Path(temporary)
            (snapshot / "manifest.json").write_text(json.dumps({"artifacts": {"../escape": "none"}}))
            with self.assertRaisesRegex(ValueError, "escapes baseline"):
                tool.verify(snapshot)

    def test_other_build_is_rejected(self):
        with self.assertRaisesRegex(ValueError, "Unsupported executable"):
            tool.verify(tool.BASELINE, REPO / "working/game-clean/Chaos.exe")


if __name__ == "__main__":
    unittest.main()
