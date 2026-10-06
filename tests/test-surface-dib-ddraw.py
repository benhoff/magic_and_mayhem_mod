#!/usr/bin/env python3
import copy,importlib.util,unittest
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('ddraw',ROOT/'tools/check-surface-dib-ddraw.py');d=importlib.util.module_from_spec(spec);spec.loader.exec_module(d)
class DdrawTests(unittest.TestCase):
 @classmethod
 def setUpClass(cls):
  _,cls.payload=d.load();p,_=d.gdi.sentinel.load();cls.assets=p['assets']
 def test_all_native_words_and_dc_samples(self):
  counts,_=d.compare(self.payload,self.assets);self.assertEqual(counts['cases'],72);self.assertEqual(counts['dc_samples'],216);self.assertEqual(counts['presentation_policy_differences'],70)
 def test_native_word_tampering_refused(self):
  p=copy.deepcopy(self.payload);p['rows'][0]['pixels'][0]^=1
  with self.assertRaisesRegex(ValueError,'native pixels differ'):d.compare(p,self.assets)
 def test_border_tampering_refused(self):
  p=copy.deepcopy(self.payload);r=next(r for r in p['rows'] if r['input']['shape']=='larger');r['pixels'][-1]^=1
  with self.assertRaisesRegex(ValueError,'native pixels differ'):d.compare(p,self.assets)
 def test_failed_getdc_output_tampering_refused(self):
  p=copy.deepcopy(self.payload);p['rows'][0]['observation']['repeated_out_state']=1
  with self.assertRaisesRegex(ValueError,'DC admission differs'):d.compare(p,self.assets)
 def test_original_lock_getdc_success_preserved(self):
  self.assertTrue(all(r['observation']['get_dc_while_locked']==0 for r in self.payload['rows']))
 def test_selected_bitmap_tampering_refused(self):
  p=copy.deepcopy(self.payload);p['rows'][0]['observation']['bitmap_bits']=32
  with self.assertRaisesRegex(ValueError,'Selected DC bitmap differs'):d.compare(p,self.assets)
 def test_dc_sample_tampering_refused(self):
  p=copy.deepcopy(self.payload);p['rows'][0]['observation']['dc_first_rgb']^=1
  with self.assertRaisesRegex(ValueError,'GetPixel sample differs'):d.compare(p,self.assets)
if __name__=='__main__':unittest.main()
