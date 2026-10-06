#!/usr/bin/env python3
import base64,copy,importlib.util,unittest
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('p','tools/check-surface-dc-palette-identity.py');p=importlib.util.module_from_spec(spec);spec.loader.exec_module(p)
class PaletteIdentityTests(unittest.TestCase):
 @classmethod
 def setUpClass(cls):_,cls.payload=p.load()
 def test_all_identity_outputs(self):
  c,_=p.compare(self.payload);self.assertEqual(c['cases'],150);self.assertEqual(c['native_pixels'],7200)
 def test_duplicate_indices_preserved(self):
  data=copy.deepcopy(self.payload);r=next(r for r in data['rows'] if r['input']['asset']=='cube-duplicate' and r['input']['palette_style']==3 and r['input']['gdiclip']==0);self.assertEqual(r['pixels'][1],255);r['pixels'][1]=0
  with self.assertRaisesRegex(ValueError,'raster differs'):p.compare(data)
 def test_duplicate_translation_uses_first_entry(self):
  data=copy.deepcopy(self.payload);r=next(r for r in data['rows'] if r['input']['asset']=='gray-duplicate' and r['input']['palette_style']==3 and r['input']['gdiclip']==0);self.assertEqual(r['pixels'][1],0);r['pixels'][1]=255
  with self.assertRaisesRegex(ValueError,'raster differs'):p.compare(data)
 def test_boundary_conversion_and_clipping_independent(self):
  data=copy.deepcopy(self.payload);r=next(r for r in data['rows'] if r['input']['asset']=='rgb24-boundaries' and r['input']['palette_style']==2 and r['input']['gdiclip']==1);r['pixels'][0]^=1
  with self.assertRaisesRegex(ValueError,'raster differs'):p.compare(data)
 def test_exact_source_table_tampering_refused(self):
  data=copy.deepcopy(self.payload);r=next(r for r in data['rows'] if r['input']['palette_style']==4);r['color_table'][5]^=1
  with self.assertRaisesRegex(ValueError,'color table'):p.compare(data)
if __name__=='__main__':unittest.main()
