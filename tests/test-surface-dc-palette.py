#!/usr/bin/env python3
import copy,importlib.util,unittest
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('p','tools/check-surface-dc-palette.py');p=importlib.util.module_from_spec(spec);spec.loader.exec_module(p)
class DcPaletteTests(unittest.TestCase):
 @classmethod
 def setUpClass(cls):
  _,cls.payload=p.load();a,_=p.dc.gdi.sentinel.load();cls.assets=a['assets']
 def test_all_capture_outputs(self):
  c,_=p.compare(self.payload,self.assets);self.assertEqual(c['cases'],360);self.assertEqual(c['absent_palette_cases'],60);self.assertEqual(c['native_pixels'],14400)
 def changed(self,field,choose):
  data=copy.deepcopy(self.payload);row=next(r for r in data['rows'] if choose(r));row[field][0]^=1
  with self.assertRaises(ValueError):p.compare(data,self.assets)
 def test_native_mutation(self):self.changed('pixels',lambda r:r['input']['bits']==16)
 def test_palette_table_mutation(self):self.changed('color_table',lambda r:r['input']['palette_style']==1)
 def test_presented_color_mutation(self):self.changed('dc_rgb',lambda r:r['input']['bits']==32)
 def test_getpixel_clip_policy_mutation(self):self.changed('dc_clipped',lambda r:r['input']['gdiclip']==1 and r['input']['bits']==32)
 def test_directdraw_clipper_is_separate(self):
  d=copy.deepcopy(self.payload);r=next(r for r in d['rows'] if r['input']['ddclip']==3);r['observation']['clip_box_result']=1
  with self.assertRaisesRegex(ValueError,'clip box'):p.compare(d,self.assets)
 def test_empty_gdi_clip_retains_all_pixels(self):
  d=copy.deepcopy(self.payload);r=next(r for r in d['rows'] if r['input']['gdiclip']==3 and r['input']['bits']==32);r['pixels'][2]=0
  with self.assertRaisesRegex(ValueError,'raster differs'):p.compare(d,self.assets)
 def test_full_precision_nearest_model_refused(self):
  d=copy.deepcopy(self.payload);r=next(r for r in d['rows'] if r['input']['asset']=='rgb24' and r['input']['palette_style']==1 and r['input']['gdiclip']==0);_,_,rgb=p.dc.gdi.source_image(__import__('base64').b64decode(self.assets['rgb24']));r['pixels'][0]=p.nearest(rgb[0],p.palette(1))
  with self.assertRaisesRegex(ValueError,'raster differs'):p.compare(d,self.assets)
 def test_absent_palette_has_no_conversion_claim(self):
  d=copy.deepcopy(self.payload);r=d['rows'][0];r['pixels'][0]^=1;c,states=p.compare(d,self.assets);self.assertIsNone(states[0]['expected']);self.assertEqual(c['absent_palette_cases'],60)
 def test_absent_palette_untouched_border_checked(self):
  d=copy.deepcopy(self.payload);r=next(r for r in d['rows'] if r['input']['bits']==8 and not r['input']['palette_style'] and r['input']['gdiclip']==3);r['pixels'][0]^=1
  with self.assertRaisesRegex(ValueError,'untouched border'):p.compare(d,self.assets)
if __name__=='__main__':unittest.main()
