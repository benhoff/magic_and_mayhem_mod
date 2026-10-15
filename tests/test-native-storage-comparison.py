#!/usr/bin/env python3
"""Independent raster comparison must retain mismatches and baseline limits."""
import importlib.util
from pathlib import Path
import sys
import unittest
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'tools'))
spec=importlib.util.spec_from_file_location('comparison',ROOT/'tools/test-native-storage-comparison.py')
m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)
class ComparisonTests(unittest.TestCase):
    def test_changed_pixel_fails_exact_comparison(self):
        expected=bytes(960000);actual=b'\x01\x00'+expected[2:]
        result=m.compare_words(actual,expected,expected,expected)
        self.assertEqual(result['mismatched_words'],1)
        self.assertEqual(result['two_baseline_equal_mismatches'],1)
    def test_inherited_pixels_do_not_become_independent(self):
        zero=bytes(960000);poison=b'\xa5\x5a'*480000
        result=m.compare_words(zero,zero,zero,poison)
        self.assertEqual(result['two_baseline_equal_words'],0)
        self.assertEqual(result['two_baseline_different_words'],480000)
    def test_truncated_frame_refused(self):
        with self.assertRaises(ValueError):m.compare_words(b'',b'',b'',b'')
if __name__=='__main__':unittest.main()
