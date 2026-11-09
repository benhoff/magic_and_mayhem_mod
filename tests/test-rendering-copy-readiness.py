#!/usr/bin/env python3
"""Unknown/refused or merely observed COPY branches cannot earn output credit."""
from pathlib import Path
import sys
import unittest
sys.path.insert(0, str(Path(__file__).resolve().parents[1]/'tools'))
from rendering_copy_readiness import classify, summarize


class Controls(unittest.TestCase):
    def row(self, alias=False):
        row = [0]*44
        row[5], row[10], row[15] = 1, 128 if alias else 0, 1
        row[24:32] = [0, 0, 10, 10, 2, 2, 12, 12]
        return row

    def test_distinct_overlapping_coordinates_are_not_aliases(self):
        self.assertFalse(classify(self.row())['overlapping'])

    def test_alias_overlap_and_disjoint_alias(self):
        row = self.row(True)
        self.assertTrue(classify(row)['overlapping'])
        row[28:32] = [10, 10, 20, 20]
        self.assertFalse(classify(row)['overlapping'])

    def test_stretch_is_independent_of_translation(self):
        row = self.row()
        self.assertFalse(classify(row)['stretched'])
        row[30] += 1
        self.assertTrue(classify(row)['stretched'])

    def test_refused_geometry_is_unknown(self):
        row = self.row(True)
        row[15], row[13] = 0, 0x88760096
        row[30] += 1
        q = summarize([row])
        self.assertEqual(q['counts']['unknown_geometry'], 1)
        self.assertEqual(q['counts']['stretched'], 0)
        self.assertEqual(q['counts']['failed_tracked_alias'], 1)

    def test_signed_coordinates_and_wide_difference(self):
        row = self.row()
        row[24:32] = [0x80000000, 0xffffffff, 0x7fffffff, 1, 0x80000000, 0xffffffff, 0x7fffffff, 1]
        self.assertFalse(classify(row)['stretched'])

    def test_nonpositive_prepared_geometry_refuses(self):
        row = self.row()
        row[26] = 0
        with self.assertRaisesRegex(ValueError, 'nonpositive'):
            classify(row)

    def test_fill_is_not_a_copy(self):
        row = self.row()
        row[5] = 0
        self.assertEqual(summarize([row])['counts']['source_less_calls'], 1)

    def test_positive_metadata_is_not_output_or_acceptance(self):
        row = self.row(True)
        row[30] += 1
        q = summarize([row])
        self.assertEqual(q['counts']['successful_prepared_stretch'], 1)
        self.assertEqual(q['counts']['successful_prepared_overlap'], 1)
        self.assertFalse(q['acceptance_ready'])
        self.assertFalse(q['output_verified'])
        self.assertFalse(q['natural_reachability_proved'])

    def test_wrong_wire_version_refuses(self):
        with self.assertRaisesRegex(ValueError, 'Version2'):
            classify(self.row()[:40])


if __name__ == '__main__':
    unittest.main()
