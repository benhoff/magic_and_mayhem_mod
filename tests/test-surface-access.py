#!/usr/bin/env python3
import copy,importlib.util,unittest
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('a',ROOT/'tools/check-surface-access.py');a=importlib.util.module_from_spec(spec);spec.loader.exec_module(a)
class AccessTests(unittest.TestCase):
 @classmethod
 def setUpClass(cls):_,cls.data=a.load()
 def test_full_capture(self):self.assertEqual(a.compare(self.data)[0]['cases'],96)
 def mutate(self,sequence,index,field,value):
  d=copy.deepcopy(self.data);r=next(r for r in d['rows'] if r['input']['sequence']==sequence);r['steps'][index][field]=value
  with self.assertRaises(ValueError):a.compare(d)
 def test_lock_during_lock_busy(self):self.mutate('repeat-lock',1,'result',0)
 def test_dc_during_lock_allowed(self):self.mutate('lock-dc-release-unlock',1,'result',0x887601ae)
 def test_failed_getdc_preserves_output(self):self.mutate('repeat-dc',1,'dc_out_state',1)
 def test_busy_lock_clears_descriptor(self):
  d=copy.deepcopy(self.data);d['rows'][1]['steps'][1]['descriptor'][1]=0xabababab
  with self.assertRaisesRegex(ValueError,'clearing'):a.compare(d)
 def test_null_release_is_measured(self):self.mutate('dc-null-release',1,'result',0x80070057)
 def test_foreign_release_is_measured(self):self.mutate('dc-foreign-release',1,'result',0x80070057)
 def test_pixels_after_transition(self):
  d=copy.deepcopy(self.data);d['rows'][0]['pixels'][0]^=1
  with self.assertRaisesRegex(ValueError,'pixels'):a.compare(d)
 def test_complete_matrix(self):
  d=copy.deepcopy(self.data);d['rows'][0]['input']['operations'][0]=0
  with self.assertRaisesRegex(ValueError,'matrix'):a.compare(d)
 def test_unstable_failed_caps_is_not_equivalence(self):
  d=copy.deepcopy(self.data);d['rows'][1]['steps'][1]['descriptor'][26]^=0x1234;a.compare(d)
 def test_terminal_failure_cannot_invent_pixels(self):
  d=copy.deepcopy(self.data);r=next(r for r in d['rows'] if r['pixels'] is None);r['pixels']=[0]*48
  with self.assertRaisesRegex(ValueError,'cannot supply'):a.compare(d)
if __name__=='__main__':unittest.main()
