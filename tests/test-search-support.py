#!/usr/bin/env python3
"""Verify extracted evidence and reject unsupported inputs before writing."""
import hashlib
import importlib.util
import json
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch

REPO = Path(__file__).resolve().parent.parent
spec = importlib.util.spec_from_file_location("support", REPO / "tools/export-search-support.py")
support = importlib.util.module_from_spec(spec)
spec.loader.exec_module(support)


class SupportTests(unittest.TestCase):
    def test_hash_rejection_before_output(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            executable = root / "bad.exe"
            executable.write_bytes(b"unsupported")
            with patch.object(support, "REPO", root):
                with self.assertRaisesRegex(ValueError, "Unsupported executable"):
                    support.export(executable)
            self.assertFalse((root / "working").exists())

    def test_file_backed_tables_and_complete_exports(self):
        executable = REPO / "working/game-nocd/Chaos.exe"
        with tempfile.TemporaryDirectory() as temporary, patch.object(support, "REPO", Path(temporary)):
            output = support.export(executable)
            manifest = json.loads((output / "manifest.json").read_text())
            self.assertTrue(manifest["input_unchanged"])
            self.assertEqual(manifest["scalar_dispatch"],[0x5058a4,0x5058c2,0x5058c2,0x5058a4,0x5058a4])
            self.assertEqual(manifest["scalar_slope_factor"],76.80000305175781)
            self.assertEqual(manifest["directions"], [7,0,1,6,0,2,5,4,3])
            planar = [[0,-1],[1,-1],[1,0],[1,1],[0,1],[-1,1],[-1,0],[-1,-1]]
            expected = [xy + [0] for xy in planar]
            for z in (1, -1):
                expected += [[0,0,z]] + [xy + [z] for xy in planar]
            self.assertEqual(manifest["neighbor_offsets"], expected)
            self.assertEqual(manifest["local_caller_offsets"], [[0,0,0]] + expected)
            self.assertEqual(manifest["six_link_tables"], {
                "x": [-1,1,0,0,0,0], "y": [0,0,-1,1,0,0], "z": [0,0,0,0,-1,1],
                "masks": [8192,2048,1024,4096,512,256], "costs": [2,2,2,2,1,3]})
            self.assertEqual(manifest["six_link_float_step_hex"], "0000803f")
            self.assertEqual(manifest["six_link_float_step"], 1.0)
            self.assertEqual(manifest["ranges"]["creature_acceptance"], [0x514360, 0x51462c])
            self.assertEqual(manifest["ranges"]["creature_occupancy"], [0x4f5a40, 0x4f5aed])
            self.assertIn("creature_acceptance.asm", manifest["artifacts"])
            self.assertIn("creature_occupancy.asm", manifest["artifacts"])
            self.assertEqual(manifest["ranges"]["movement_test"], [0x4f3990, 0x4f41a0])
            self.assertIn("movement_test.asm", manifest["artifacts"])
            self.assertEqual(manifest["ranges"]["cell_test"], [0x4f47d0, 0x4f48f6])
            self.assertIn("cell_test.asm", manifest["artifacts"])
            self.assertEqual(manifest["ranges"]["occupancy_test"], [0x4f3550, 0x4f35d0])
            self.assertIn("occupancy_test.asm", manifest["artifacts"])
            self.assertEqual(manifest["ranges"]["validity_test"], [0x4f46b0, 0x4f47c4])
            self.assertIn("validity_test.asm", manifest["artifacts"])
            self.assertEqual(manifest["ranges"]["cell_validity_test"], [0x4f3440, 0x4f354c])
            self.assertIn("cell_validity_test.asm", manifest["artifacts"])
            self.assertEqual(manifest["ranges"]["record_query"], [0x4f4330, 0x4f44db])
            self.assertIn("record_query.asm", manifest["artifacts"])
            self.assertEqual(manifest["ranges"]["cell_support"], [0x4f3320, 0x4f3439])
            self.assertIn("cell_support.asm", manifest["artifacts"])
            self.assertEqual(manifest["ranges"]["boundary_test"], [0x4f41a0, 0x4f431c])
            self.assertEqual(manifest["boundary_dispatch"], [0x4f41be, 0x4f41cf, 0x4f4232, 0x4f42c5])
            self.assertIn("boundary_test.asm", manifest["artifacts"])
            self.assertEqual(manifest["ranges"]["any_terrain"], [0x4f44e0, 0x4f45ae])
            self.assertEqual(manifest["ranges"]["tall_terrain"], [0x4f45b0, 0x4f46a9])
            self.assertIn("any_terrain.asm", manifest["artifacts"])
            self.assertIn("tall_terrain.asm", manifest["artifacts"])
            for name, start, end in [("creature_scalar",0x5205b0,0x520620),
                                     ("base_scalar",0x505840,0x505909),
                                     ("adjust_scalar",0x505920,0x505a96),
                                     ("scaled_metric",0x4eaca0,0x4eacb1)]:
                self.assertEqual(manifest["ranges"][name],[start,end])
                self.assertIn(name + ".asm",manifest["artifacts"])
            for name, expected_hash in manifest["artifacts"].items():
                data = (output / name).read_bytes()
                self.assertEqual(hashlib.sha256(data).hexdigest(), expected_hash)
                self.assertNotIn(b".byte", data, name) # no sliced instructions
            # A second export must leave the first evidence directory intact.
            self.assertNotEqual(output, support.export(executable))
            self.assertTrue((output / "manifest.json").exists())


if __name__ == "__main__":
    unittest.main()
