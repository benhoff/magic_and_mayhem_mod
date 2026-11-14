import copy,importlib.util,sys,unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from broader_picking_pan_final_gate import exact_pan_ledger,FAILED
s=importlib.util.spec_from_file_location('prior_uncovered_ledger',Path(__file__).with_name('test-picking-uncovered-final-ledger.py'));prior=importlib.util.module_from_spec(s);s.loader.exec_module(prior)
class LedgerTests(prior.LedgerTests):
 def setUp(self):
  super().setUp();self.pan=dict(case_id=FAILED,reserved_games=1,state='acquired',acquisition_sha256='nominal-pan-negative');self.rows[FAILED]=copy.deepcopy(self.pan)
 def run_gate(self):exact_pan_ledger(self.rows,self.values,self.old,self.failed,self.zero,self.font,self.reference,self.schema,self.picking,self.index,self.witness,self.rebuild,self.uncovered,self.pan)
 def test_pan_missing_refuses(self):
  self.rows.pop(FAILED)
  with self.assertRaises(ValueError):self.run_gate()
 def test_pan_refund_refuses(self):
  self.rows[FAILED]['reserved_games']=0
  with self.assertRaises(ValueError):self.run_gate()
 def test_pan_promotion_refuses(self):
  self.rows[FAILED]['state']='admitted'
  with self.assertRaises(ValueError):self.run_gate()
 def test_pan_hash_changed_refuses(self):
  self.rows[FAILED]['acquisition_sha256']='other'
  with self.assertRaises(ValueError):self.run_gate()
 def test_pan_hash_missing_refuses(self):
  self.pan['acquisition_sha256']=self.rows[FAILED]['acquisition_sha256']=None
  with self.assertRaises(ValueError):self.run_gate()
 def test_pan_positive_credit_refuses(self):
  self.values[-1]['spec']['case']['id']=FAILED
  with self.assertRaises(ValueError):self.run_gate()
if __name__=='__main__':unittest.main()
