#!/usr/bin/env python3
import copy,importlib.util,unittest
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('rgb',ROOT/'tools/check-surface-dib-ddraw-rgb.py');d=importlib.util.module_from_spec(spec);spec.loader.exec_module(d)
class FullDcTests(unittest.TestCase):
 @classmethod
 def setUpClass(cls):
  _,cls.payload=d.load();p,_=d.dc.gdi.sentinel.load();cls.assets=p['assets']
 def test_all_independent_dc_colors(self):
  c,_=d.compare(self.payload,self.assets);self.assertEqual(c['cases'],72);self.assertEqual(c['rgb565_channel_levels'],[32,64,32]);self.assertGreater(c['floor_rgb_differences'],0)
 def test_dc_rgb_tampering_refused(self):
  p=copy.deepcopy(self.payload);p['rows'][0]['dc_pixels'][0]^=1
  with self.assertRaisesRegex(ValueError,'Full DC RGB differs'):d.compare(p,self.assets)
 def test_old_floor_expansion_refused(self):
  p=copy.deepcopy(self.payload)
  r=next(r for r in p['rows'] if r['input']['bits']==16 and any(d.dc.expand565(v)!=d.dc.replicated565(v) for v in r['pixels']))
  r['dc_pixels']=[d.dc.colorref(d.dc.expand565(v)) for v in r['pixels']]
  with self.assertRaisesRegex(ValueError,'Full DC RGB differs'):d.compare(p,self.assets)
if __name__=='__main__':unittest.main()
