#!/usr/bin/env python3
import copy,importlib.util,unittest
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('r',ROOT/'tools/check-surface-dc-reuse.py');r=importlib.util.module_from_spec(spec);spec.loader.exec_module(r)
class ReuseTests(unittest.TestCase):
 @classmethod
 def setUpClass(cls):
  _,cls.data=r.load();a,_=r.p.dc.gdi.sentinel.load();cls.assets=a['assets']
 def test_all_phases(self):
  c,_=r.compare(self.data,self.assets);self.assertEqual(c['cases'],192);self.assertEqual(c['phases'],576);self.assertEqual(c['native_pixels'],27648);self.assertGreater(c['default_palette_phases'],0)
 def mutate(self,choose,action):
  d=copy.deepcopy(self.data);row=next(x for x in d['rows'] if choose(x['input']));action(row)
  with self.assertRaises(ValueError):r.compare(d,self.assets)
 def test_default_context_tampering(self):
  d=copy.deepcopy(self.data);d['default_dc_palette'][64]^=1
  with self.assertRaisesRegex(ValueError,'default context'):r.compare(d,self.assets)
 def test_binding_during_lease_is_deferred(self):self.mutate(lambda c:c['initial_palette']==1 and c['action']==5,lambda x:x['phases'][0]['table_draw'].__setitem__(64,r.p.palette(3)[64]))
 def test_direct_table_edit_does_not_persist(self):self.mutate(lambda c:c['action']==4,lambda x:x['phases'][1]['table_before'].__setitem__(64,r.changed_colors()[0]))
 def test_clip_does_not_persist(self):self.mutate(lambda c:c['clip']==1,lambda x:x['phases'][1]['observation'].__setitem__('clip_left',2))
 def test_palette_detach_uses_default(self):self.mutate(lambda c:c['initial_palette']==1 and c['action']==3,lambda x:x['phases'][1].__setitem__('table_before',r.p.palette(1)))
 def test_second_lease_pixels_mutation(self):self.mutate(lambda c:c['bits']==8,lambda x:x['phases'][1]['pixels'].__setitem__(0,x['phases'][1]['pixels'][0]^1))
 def test_dc_rgb_mutation(self):self.mutate(lambda c:c['bits']==16,lambda x:x['phases'][2]['dc_rgb'].__setitem__(0,x['phases'][2]['dc_rgb'][0]^1))
 def test_complete_case_matrix_required(self):
  d=copy.deepcopy(self.data);d['rows'][1]['input']['action']=6
  with self.assertRaisesRegex(ValueError,'case matrix'):r.compare(d,self.assets)
if __name__=='__main__':unittest.main()
