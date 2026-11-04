#!/usr/bin/env python3
"""Strict size-profile admission for retained CPU-reader analysis."""
import importlib.util
from pathlib import Path
import sys
import unittest

ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'tools'))
spec=importlib.util.spec_from_file_location('reader_analysis',ROOT/'tools/analyze-cpu-reader-journal.py')
analysis=importlib.util.module_from_spec(spec);spec.loader.exec_module(analysis)

class WorldProfile(unittest.TestCase):
    def test_legacy_default_remains_640_without_new_metadata(self):
        for acquisition in ({},{'flow':{}},{'flow':{'initial_resolution':0}}):
            self.assertEqual(analysis.expected_world_extent(acquisition),(640,480))

    def test_explicit_profiles_match_the_declared_original_modes(self):
        for mode,size,extent in [(0,'800x600',(800,600)),(1,'640x480',(640,480))]:
            self.assertEqual(analysis.expected_world_extent({'flow':{'initial_resolution':mode}},size),extent)

    def test_a_different_declared_mode_refuses(self):
        for mode,size in [(0,'640x480'),(1,'800x600')]:
            with self.assertRaisesRegex(ValueError,'differs from acquisition'):
                analysis.expected_world_extent({'flow':{'initial_resolution':mode}},size)

    def test_missing_or_untyped_mode_cannot_authorize_a_profile(self):
        for mode in [None,True,False,'0','1',2,-1]:
            for size in ['640x480','800x600']:
                with self.assertRaises(ValueError):
                    analysis.expected_world_extent({'flow':{'initial_resolution':mode}},size)

    def test_unlisted_extents_refuse(self):
        for size in ['1024x768','800X600','',True]:
            with self.assertRaisesRegex(ValueError,'Unsupported'):
                analysis.expected_world_extent({'flow':{'initial_resolution':0}},size)

if __name__=='__main__':unittest.main()
