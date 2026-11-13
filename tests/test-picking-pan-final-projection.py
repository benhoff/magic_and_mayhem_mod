import inspect,sys,unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
import broader_picking_pan_final_gate as gate
from broader_player_picking_pan_matrix import specs
class Controls(unittest.TestCase):
 def test_fading_code_preserved(self):self.assertIs(gate._FADING.__code__,gate.previous._FADING.__code__)
 def test_fading_current_specs(self):self.assertIs(gate._FADING.__globals__['specs'],specs)
 def test_caller_code_preserved(self):self.assertIs(gate._CALLER.__code__,gate.previous._CALLER.__code__)
 def test_caller_current_fading(self):self.assertIs(gate._CALLER.__globals__['original_supplement'],gate._FADING)
 def test_prior_code_preserved(self):self.assertIs(gate._PRIOR.__code__,gate.previous._PRIOR.__code__)
 def test_prior_current_caller(self):self.assertIs(gate._PRIOR.__globals__['_CALLER'],gate._CALLER)
 def test_witness_code_preserved(self):self.assertIs(gate._WITNESS.__code__,gate.previous._WITNESS.__code__)
 def test_witness_current_prior(self):self.assertIs(gate._WITNESS.__globals__['_PRIOR'],gate._PRIOR)
 def test_rebuild_exact_receipt_literals(self):self.assertEqual(gate.receipt_literal_review(gate.previous._REBUILD,gate._REBUILD),dict(pilot_record_literals=1,after_pilot_charge_literals=1))
 def test_rebuild_current_witness_and_pilot(self):self.assertIs(gate._REBUILD.__globals__['_WITNESS'],gate._WITNESS);self.assertEqual(gate._REBUILD.__globals__['PILOT'],gate.PILOT)
 def test_uncovered_exact_receipt_literals(self):self.assertEqual(gate.receipt_literal_review(gate.previous.supplement,gate._UNCOVERED),dict(pilot_record_literals=1,after_pilot_charge_literals=1))
 def test_uncovered_current_rebuild_and_pilot(self):self.assertIs(gate._UNCOVERED.__globals__['_REBUILD'],gate._REBUILD);self.assertEqual(gate._UNCOVERED.__globals__['PILOT'],gate.PILOT)
 def test_same_context_unchanged(self):self.assertIs(gate.same_context,gate.previous.same_context)
 def test_exact14guarded_identities(self):
  declared=[dict(family=f,case=dict(id=f+'-'+str(i)))for f,n in [('attachment',4),('font',4),('palette',4),('picking',2)]for i in range(n)];values=[dict(spec=s)for s in declared];rows=[dict(case=s['case']['id'])for s in declared];self.assertTrue(gate.guarded_contexts(declared,values,rows));rows[-1]['case']='foreign'
  with self.assertRaises(ValueError):gate.guarded_contexts(declared,values,rows)
 def test_source_receipt_changes_restore_exactly(self):
  rebuilt=gate.rebuilt_source().replace("pn=P+'picking-pan-single-pilot-01.json'","pn=P+'picking-uncovered-single-pilot-01.json'").replace("pilot['conservative_charges_after']==167","pilot['conservative_charges_after']==166");self.assertEqual(rebuilt,gate.previous.adapted_rebuild_source())
  uncovered=gate.uncovered_source().replace('picking-pan-single-pilot-01.json','picking-uncovered-single-pilot-01.json').replace('after_pilot_charges=167','after_pilot_charges=166');self.assertEqual(uncovered,inspect.getsource(gate.previous.supplement))
if __name__=='__main__':unittest.main()
