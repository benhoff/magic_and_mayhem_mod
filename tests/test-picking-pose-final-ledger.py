import copy,importlib.util,sys,unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from broader_picking_pose_final_gate import exact_pose_ledger,FAILED
s=importlib.util.spec_from_file_location('prior_inverse_picking_ledger',Path(__file__).with_name('test-picking-target-final-ledger.py'));prior=importlib.util.module_from_spec(s);s.loader.exec_module(prior)
class LedgerTests(prior.LedgerTests):
 def setUp(self):
  super().setUp();self.index=dict(case_id=FAILED,reserved_games=1,state='failed',acquisition_sha256='index-negative');self.rows[FAILED]=copy.deepcopy(self.index)
 def run_gate(self):exact_pose_ledger(self.rows,self.values,self.old,self.failed,self.zero,self.font,self.reference,self.schema,self.picking,self.index)
 def test_index_missing_refuses(self):
  self.rows.pop(FAILED)
  with self.assertRaises(ValueError):self.run_gate()
 def test_index_refund_refuses(self):
  self.rows[FAILED]['reserved_games']=0
  with self.assertRaises(ValueError):self.run_gate()
 def test_index_promotion_refuses(self):
  self.rows[FAILED]['state']='admitted'
  with self.assertRaises(ValueError):self.run_gate()
 def test_index_hash_changed_refuses(self):
  self.rows[FAILED]['acquisition_sha256']='other'
  with self.assertRaises(ValueError):self.run_gate()
 def test_index_hash_missing_refuses(self):
  self.index['acquisition_sha256']=self.rows[FAILED]['acquisition_sha256']=None
  with self.assertRaises(ValueError):self.run_gate()
 def test_index_positive_credit_refuses(self):
  self.values[-1]['spec']['case']['id']=FAILED
  with self.assertRaises(ValueError):self.run_gate()
if __name__=='__main__':unittest.main()
