#!/usr/bin/env python3
import importlib.util
from pathlib import Path
import sys
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
spec = importlib.util.spec_from_file_location('hashes', ROOT / 'tools/compare-drawing-copy-phase-hashes.py')
m = importlib.util.module_from_spec(spec); spec.loader.exec_module(m)


class HashDenominators(unittest.TestCase):
    def test_all_identities_and_failures_retained(self):
        old = [dict(ordinal=i, sequence=i * 3, canvas=2, width=800, height=600, original_sha256='a' * 64) for i in (1, 2)]
        actual = [dict(r, sha256='a' * 64) for r in old]
        actual[1]['sha256'] = 'b' * 64
        compared = m.compare_checkpoints(old, actual)
        self.assertEqual(2, len(compared))
        self.assertTrue(compared[0]['full_plane_hash_matches'])
        self.assertFalse(compared[1]['full_plane_hash_matches'])
        self.assertIsNone(compared[1]['different_pixels'])
        self.assertEqual(0, compared[0]['different_pixels'])
        for broken in (actual[:1], [actual[0], actual[0]], [dict(actual[0], canvas=5), actual[1]],
                       [dict(actual[0], sha256='?'), actual[1]]):
            with self.assertRaises(ValueError):
                m.compare_checkpoints(old, broken)
        with self.assertRaises(ValueError):
            m.compare_checkpoints([old[0], old[0]], actual)


if __name__ == '__main__':
    unittest.main()
