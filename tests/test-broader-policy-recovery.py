import importlib.util,sys,unittest
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools'))
spec=importlib.util.spec_from_file_location('policy_binding',ROOT/'tools/check-broader-policy-recovery.py');binding=importlib.util.module_from_spec(spec);spec.loader.exec_module(binding)
SOURCE=b'''from pathlib import Path
ROOT=Path('/owned')
def main(surface_loss=False):
 output_root=ROOT/('working/surface-loss/comparisons' if surface_loss else 'working/recovery-closeout08/comparisons')
 assert complete and mismatches==0
 return output_root, ROOT/'working/original-reference', deadline
'''
class PolicyBinding(unittest.TestCase):
 def test_exact_bindings_preserve_all_assertions_and_deadline(self):
  code,_=binding.bound_code(SOURCE,'fixture');n=dict(POLICY_OUTPUT_ROOT=Path('/ram'),POLICY_REFERENCE=Path('/policy'),complete=True,mismatches=0,deadline=300);exec(code,n)
  self.assertEqual(n['main'](),(Path('/ram'),Path('/policy'),300));n['mismatches']=1
  with self.assertRaises(AssertionError):n['main']()
  n['mismatches']=0;n['complete']=False
  with self.assertRaises(AssertionError):n['main']()
 def test_unknown_oracle_refuses(self):
  with self.assertRaises(ValueError):binding.bound_code(SOURCE.replace(b'working/original-reference',b'other'),'fixture')
 def test_duplicate_oracle_refuses(self):
  with self.assertRaises(ValueError):binding.bound_code(SOURCE+b"\nother=ROOT/'working/original-reference'\n",'fixture')
 def test_unknown_output_refuses(self):
  with self.assertRaises(ValueError):binding.bound_code(SOURCE.replace(b'working/surface-loss/comparisons',b'other'),'fixture')
 def test_actual_producer(self):binding.bound_code((ROOT/binding.SOURCE).read_bytes(),binding.SOURCE)
if __name__=='__main__':unittest.main()
