import sys,unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
import broader_picking_rebuild_final_gate as gate
from broader_player_picking_rebuild_matrix import specs
class Controls(unittest.TestCase):
 def test_fading_predicate_bytecode_preserved(self):self.assertIs(gate._FADING.__code__,gate.previous._FADING.__code__)
 def test_current_spec_binding_selected(self):self.assertIs(gate._FADING.__globals__['specs'],specs)
 def test_caller_predicate_bytecode_preserved(self):self.assertIs(gate._CALLER.__code__,gate.previous._CALLER.__code__)
 def test_caller_uses_current_fading_predicates(self):self.assertIs(gate._CALLER.__globals__['original_supplement'],gate._FADING)
 def test_prior_supplement_bytecode_preserved(self):self.assertIs(gate._PRIOR.__code__,gate.previous._PRIOR.__code__)
 def test_prior_supplement_uses_current_caller_predicates(self):self.assertIs(gate._PRIOR.__globals__['_CALLER'],gate._CALLER)
 def test_prior_witness_predicate_bytecode_preserved(self):self.assertIs(gate._WITNESS.__code__,gate.previous.supplement.__code__)
 def test_prior_witness_uses_current_prior(self):self.assertIs(gate._WITNESS.__globals__['_PRIOR'],gate._PRIOR)
 def test_same_context_predicates_preserved(self):self.assertIs(gate.same_context,gate.previous.same_context)
 def test_exact_guarded_identity_set_still_required(self):
  declared=[dict(family=f,case=dict(id=f+'-'+str(i)))for f,n in [('attachment',4),('font',4),('palette',4),('picking',2)]for i in range(n)];values=[dict(spec=s)for s in declared];rows=[dict(case=s['case']['id'])for s in declared]
  self.assertTrue(gate.previous.previous.previous.guarded_contexts(declared,values,rows));rows[-1]['case']='foreign'
  with self.assertRaises(ValueError):gate.previous.previous.previous.guarded_contexts(declared,values,rows)
if __name__=='__main__':unittest.main()
