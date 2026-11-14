import copy,sys,unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from broader_nominal_schemas import caller_preimages
class Schema(unittest.TestCase):
 def test_empty_list_only_changes_shape(self):
  before=dict(source_preimages=[],runtime_sources={'exact':'hash'},other=[1]);old=copy.deepcopy(before);after,changed=caller_preimages(before);self.assertTrue(changed);self.assertEqual(after,dict(old,source_preimages={}));self.assertEqual(old,before)
 def test_mapping_preserved(self):
  value=dict(source_preimages={'exact':{'record':'original','sha256':'pinned'}});self.assertEqual(caller_preimages(value),(value,False))
 def test_absent_field_uses_original_default(self):self.assertEqual(caller_preimages({}),({},False))
 def test_nonempty_sequence_refuses(self):
  with self.assertRaises(ValueError):caller_preimages(dict(source_preimages=[{'record':'unknown'}]))
 def test_other_shapes_refuse(self):
  for v in(None,'',0,False):
   with self.assertRaises(ValueError):caller_preimages(dict(source_preimages=v))
if __name__=='__main__':unittest.main()
