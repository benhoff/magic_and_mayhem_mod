"""Reuse only a completed original comparison; never waive an unknown failure."""
import importlib.util,sys,unittest
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools'))
def module(name,path):
 spec=importlib.util.spec_from_file_location(name,ROOT/path);m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);return m
m=module('minimal_fault','tools/run-broader-worker-fault-minimal-case.py');old=module('fault_fixture','tests/test-broader-fault-coordinator.py')
class ComparisonChoice(unittest.TestCase):
 def test_complete_original_match_needs_no_policy_reexecution(self):
  b,a=old.fixture();b['success']=True;b['epochs'][1]['comparison']['mismatching_pixels']=0
  self.assertTrue(m.raw_original_satisfies(b,a))
 def test_contradictory_success_is_not_a_raw_match(self):
  b,a=old.fixture();b['success']=True
  with self.assertRaises(ValueError):m.raw_original_satisfies(b,a)
 def test_pixel_only_difference_requires_separate_policy(self):
  b,a=old.fixture();self.assertFalse(m.raw_original_satisfies(b,a))
 def test_incomplete_original_match_still_refused(self):
  for key in ['sources_stable','comparison_processes_stopped','original_manifest_verified_before_after']:
   b,a=old.fixture();b['success']=True;b[key]=False
   with self.assertRaises(ValueError):m.raw_original_satisfies(b,a)
 def test_unknown_failure_never_routes_to_policy_waiver(self):
  b,a=old.fixture();b['error']='Capture incomplete or truncated'
  with self.assertRaises(ValueError):m.raw_original_satisfies(b,a)
 def test_failed_prefix_and_fresh_retirement_remain_exact(self):
  for index,key,value in [(0,'synthetic_end_added',True),(0,'normal_completion_claimed',True),(1,'remaining_surfaces',1)]:
   b,a=old.fixture();b['epochs'][index]['comparison'][key]=value
   with self.assertRaises(ValueError):m.raw_original_satisfies(b,a)
if __name__=='__main__':unittest.main()
