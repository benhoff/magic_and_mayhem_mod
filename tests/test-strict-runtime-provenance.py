#!/usr/bin/env python3
"""Reject conflicting runtime joins and preserve acquisition source maps."""
import copy,sys,unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from broader_strict_provenance import merged_sources,HEADER
class Sources(unittest.TestCase):
 def setUp(self):
  self.a={'sources':{'apps/player.cpp':'player'}};self.p={'runtime_sources':{HEADER:'header','runtime/scene/world.c':'world'}};self.b=copy.deepcopy(self.p)
 def test_exact_join(self):self.assertEqual(merged_sources(self.a,self.p,self.b),{'apps/player.cpp':'player',HEADER:'header','runtime/scene/world.c':'world'})
 def test_acquisition_immutable(self):
  before=copy.deepcopy((self.a,self.p,self.b));merged_sources(self.a,self.p,self.b);self.assertEqual((self.a,self.p,self.b),before)
 def test_conflicting_recorded_header(self):
  self.a['sources'][HEADER]='wrong'
  with self.assertRaises(ValueError):merged_sources(self.a,self.p,self.b)
 def test_conflicting_recorded_other_runtime(self):
  self.a['sources']['runtime/scene/world.c']='wrong'
  with self.assertRaises(ValueError):merged_sources(self.a,self.p,self.b)
 def test_build_header_mismatch(self):
  self.b['runtime_sources'][HEADER]='wrong'
  with self.assertRaises(ValueError):merged_sources(self.a,self.p,self.b)
 def test_phase_omits_runtime_input(self):
  del self.p['runtime_sources'][HEADER]
  with self.assertRaises(ValueError):merged_sources(self.a,self.p,self.b)
 def test_build_omits_runtime_input(self):
  del self.b['runtime_sources'][HEADER]
  with self.assertRaises(ValueError):merged_sources(self.a,self.p,self.b)
 def test_matching_existing_entry(self):
  self.a['sources'][HEADER]='header';self.assertEqual(merged_sources(self.a,self.p,self.b)[HEADER],'header')
if __name__=='__main__':unittest.main()
