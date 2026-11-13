import copy,importlib.util,sys,unittest
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools'))
spec=importlib.util.spec_from_file_location('fault_coordinator',ROOT/'tools/run-broader-fault-case.py');m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)
def fixture():
 a=dict(proof=dict(epochs=[dict(resident=dict(completed='2',source_chunks='1'))for _ in range(2)]))
 failed=dict(mismatching_pixels=0,normal_completion_claimed=False,synthetic_end_added=False,required_prefix_commands=123,required_prefix_bytes=456,uncredited_terminal_tail_bytes=789,reference_live_surfaces_before_declared_abort=30)
 b=dict(success=False,error='Uncompared source/output epoch',sources_stable=True,comparison_processes_stopped=True,original_manifest_verified_before_after=True,epochs=[dict(original_reference=dict(worlds=2,source_chunks=1),comparison=failed),dict(original_reference=dict(worlds=2,source_chunks=1),comparison=dict(mismatching_pixels=4,remaining_surfaces=0))])
 return b,a
class BaselineGuards(unittest.TestCase):
 def test_failed_prefix_keeps_live_surfaces_and_uncredited_tail(self):b,a=fixture();m.raw_baseline_complete(b,a)
 def test_unknown_error_stops(self):
  b,a=fixture();b['error']='capture truncated'
  with self.assertRaises(ValueError):m.raw_baseline_complete(b,a)
 def test_missing_epoch_stops(self):
  b,a=fixture();b['epochs'].pop()
  with self.assertRaises(ValueError):m.raw_baseline_complete(b,a)
 def test_no_pixel_difference_cannot_hide_another_failure(self):
  b,a=fixture();b['epochs'][1]['comparison']['mismatching_pixels']=0
  with self.assertRaises(ValueError):m.raw_baseline_complete(b,a)
 def test_synthetic_end_stops(self):
  b,a=fixture();b['epochs'][0]['comparison']['synthetic_end_added']=True
  with self.assertRaises(ValueError):m.raw_baseline_complete(b,a)
 def test_missing_source_stops(self):
  b,a=fixture();b['epochs'][0]['original_reference']['worlds']=1
  with self.assertRaises(ValueError):m.raw_baseline_complete(b,a)
 def test_fresh_live_surfaces_stop(self):
  b,a=fixture();b['epochs'][1]['comparison']['remaining_surfaces']=1
  with self.assertRaises(ValueError):m.raw_baseline_complete(b,a)
if __name__=='__main__':unittest.main()
