#!/usr/bin/env python3
import copy,importlib.util,unittest
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('gdi',ROOT/'tools/check-surface-dib-gdi.py');gdi=importlib.util.module_from_spec(spec);spec.loader.exec_module(gdi)
class GdiTests(unittest.TestCase):
 @classmethod
 def setUpClass(cls):
  _,cls.payload=gdi.load();p,_=gdi.sentinel.load();cls.assets=p['assets']
 def test_all_captured_pixels(self):self.assertEqual(gdi.compare(self.payload,self.assets)[0]['cases'],18)
 def test_pixel_mutation_refused(self):
  p=copy.deepcopy(self.payload);p['rows'][0]['pixels'][0]^=1
  with self.assertRaisesRegex(ValueError,'GDI RGB pixels differ'):gdi.compare(p,self.assets)
 def test_unwritten_border_mutation_refused(self):
  p=copy.deepcopy(self.payload);p['rows'][2]['pixels'][-1]=0
  with self.assertRaisesRegex(ValueError,'GDI RGB pixels differ'):gdi.compare(p,self.assets)
 def test_usage_mutation_refused(self):
  p=copy.deepcopy(self.payload);p['rows'][0]['input']['usage']=1
  with self.assertRaisesRegex(ValueError,'Original usage differs'):gdi.compare(p,self.assets)
 def test_failed_gdi_result_refused(self):
  p=copy.deepcopy(self.payload);p['rows'][0]['result']=0
  with self.assertRaisesRegex(ValueError,'GDI drawing failed'):gdi.compare(p,self.assets)
 def test_unused_high_byte_is_not_alpha_contract(self):
  p=copy.deepcopy(self.payload);p['rows'][0]['pixels'][0]^=0xff000000
  self.assertEqual(gdi.compare(p,self.assets)[0]['cases'],18)
if __name__=='__main__':unittest.main()
