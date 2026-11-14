import copy,sys,unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from broader_picking_target_final_gate import guarded_contexts,_PRIOR,_CALLER,previous
class Controls(unittest.TestCase):
 def setUp(self):
  self.declared=[dict(family=f,case=dict(id=f+'-'+str(i)))for f,n in [('attachment',4),('font',4),('palette',4),('picking',2)]for i in range(n)]
  self.values=[dict(spec=copy.deepcopy(s))for s in self.declared];self.repaired=[dict(case=s['case']['id'])for s in self.declared]
 def check(self):return guarded_contexts(self.declared,self.values,self.repaired)
 def test_exact14_contexts(self):self.assertTrue(self.check())
 def test_missing_original_context(self):
  self.declared.pop()
  with self.assertRaises(ValueError):self.check()
 def test_invented16_contexts(self):
  self.declared.extend([dict(family='picking',case=dict(id='invented-'+str(i)))for i in range(2)])
  with self.assertRaises(ValueError):self.check()
 def test_missing_actual_proof(self):
  self.repaired.pop()
  with self.assertRaises(ValueError):self.check()
 def test_duplicate_actual_proof(self):
  self.repaired[-1]=copy.deepcopy(self.repaired[-2])
  with self.assertRaises(ValueError):self.check()
 def test_foreign_actual_proof(self):
  self.repaired[-1]['case']='foreign'
  with self.assertRaises(ValueError):self.check()
 def test_missing_positive_value(self):
  self.values.pop()
  with self.assertRaises(ValueError):self.check()
 def test_prior_predicate_bytecode_retained(self):
  self.assertIs(_PRIOR.__code__,previous.supplement.__code__);self.assertIs(_CALLER.__code__,previous._CALLER.__code__)
if __name__=='__main__':unittest.main()
