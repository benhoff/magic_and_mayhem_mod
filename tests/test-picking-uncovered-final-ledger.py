import copy,importlib.util,sys,unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from broader_picking_uncovered_final_gate import exact_uncovered_ledger,FAILED
s=importlib.util.spec_from_file_location('prior_source_join_picking_ledger',Path(__file__).with_name('test-picking-rebuild-final-ledger.py'));prior=importlib.util.module_from_spec(s);s.loader.exec_module(prior)
class LedgerTests(prior.LedgerTests):
 def setUp(self):
  super().setUp();self.uncovered=dict(case_id=FAILED,reserved_games=1,state='failed',acquisition_sha256='software-overlap-negative');self.rows[FAILED]=copy.deepcopy(self.uncovered)
 def run_gate(self):exact_uncovered_ledger(self.rows,self.values,self.old,self.failed,self.zero,self.font,self.reference,self.schema,self.picking,self.index,self.witness,self.rebuild,self.uncovered)
 def test_uncovered_missing_refuses(self):
  self.rows.pop(FAILED)
  with self.assertRaises(ValueError):self.run_gate()
 def test_uncovered_refund_refuses(self):
  self.rows[FAILED]['reserved_games']=0
  with self.assertRaises(ValueError):self.run_gate()
 def test_uncovered_promotion_refuses(self):
  self.rows[FAILED]['state']='admitted'
  with self.assertRaises(ValueError):self.run_gate()
 def test_uncovered_hash_changed_refuses(self):
  self.rows[FAILED]['acquisition_sha256']='other'
  with self.assertRaises(ValueError):self.run_gate()
 def test_uncovered_hash_missing_refuses(self):
  self.uncovered['acquisition_sha256']=self.rows[FAILED]['acquisition_sha256']=None
  with self.assertRaises(ValueError):self.run_gate()
 def test_uncovered_positive_credit_refuses(self):
  self.values[-1]['spec']['case']['id']=FAILED
  with self.assertRaises(ValueError):self.run_gate()
if __name__=='__main__':unittest.main()
