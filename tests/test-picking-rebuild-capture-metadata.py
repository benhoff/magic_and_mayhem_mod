import copy,importlib.util,sys,unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from broader_picking_rebuild_capture_assessment import rebuild_joins
spec=importlib.util.spec_from_file_location('raw_join_controls',Path(__file__).with_name('test-picking-rebuild-join.py'));prior=importlib.util.module_from_spec(spec);spec.loader.exec_module(prior)
class MetadataControls(unittest.TestCase):
 def setUp(self):
  fixture=prior.Controls();fixture.setUp();self.a,self.o=fixture.assessment()
  self.o=copy.deepcopy(self.o)
  for r in self.a['proof']['display_rebuilds']:r['confirmed_paint']['lease']='5'
  for j in self.a['provider_adaptation']['picking_rebuild_source_join']['joins']:j['paint']['lease']='5'
  p=self.o['paints'][3][0];p.update(lease='5',reference_binding=3)
 def test_exact_derived_binding_passes(self):self.assertTrue(rebuild_joins(self.a,self.o))
 def test_input_is_not_mutated(self):
  old=copy.deepcopy(self.o);rebuild_joins(self.a,self.o);self.assertEqual(old,self.o)
 def test_wrong_binding_refuses(self):
  self.o['paints'][3][0]['reference_binding']=4
  with self.assertRaises(ValueError):rebuild_joins(self.a,self.o)
 def test_boolean_binding_refuses(self):
  self.o['paints'][3][0]['reference_binding']=True
  with self.assertRaises(ValueError):rebuild_joins(self.a,self.o)
 def test_missing_binding_refuses(self):
  del self.o['paints'][3][0]['reference_binding']
  with self.assertRaises(ValueError):rebuild_joins(self.a,self.o)
 def test_unknown_extra_metadata_still_refuses(self):
  self.o['paints'][3][0]['unexpected']=True
  with self.assertRaises(ValueError):rebuild_joins(self.a,self.o)
 def test_changed_actual_metadata_still_refuses(self):
  self.o['paints'][3][0]['frame_width']=640
  with self.assertRaises(ValueError):rebuild_joins(self.a,self.o)
 def test_zero_lease_refuses(self):
  self.o['paints'][3][0]['lease']='0'
  with self.assertRaises(ValueError):rebuild_joins(self.a,self.o)
 def test_raw_metadata_cannot_claim_derived_field(self):
  self.a['proof']['display_rebuilds'][0]['confirmed_paint']['reference_binding']=3
  with self.assertRaises(ValueError):rebuild_joins(self.a,self.o)
if __name__=='__main__':unittest.main()
