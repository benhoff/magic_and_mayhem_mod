import copy,importlib.util,sys,unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from broader_picking_target_final_gate import exact_picking_target_ledger,PICKING_FAILED
s=importlib.util.spec_from_file_location('prior_palette_ledger_controls',Path(__file__).with_name('test-palette-schema-final-ledger.py'));prior=importlib.util.module_from_spec(s);s.loader.exec_module(prior)
class LedgerTests(prior.LedgerTests):
 def setUp(self):
  super().setUp();self.picking=dict(case_id=PICKING_FAILED,reserved_games=1,state='failed',acquisition_sha256='picking-negative');self.rows[PICKING_FAILED]=copy.deepcopy(self.picking)
 def run_gate(self):exact_picking_target_ledger(self.rows,self.values,self.old,self.failed,self.zero,self.font,self.reference,self.schema,self.picking)
 def test_picking_missing_refuses(self):
  self.rows.pop(PICKING_FAILED)
  with self.assertRaises(ValueError):self.run_gate()
 def test_picking_refund_refuses(self):
  self.rows[PICKING_FAILED]['reserved_games']=0
  with self.assertRaises(ValueError):self.run_gate()
 def test_picking_promotion_refuses(self):
  self.rows[PICKING_FAILED]['state']='admitted'
  with self.assertRaises(ValueError):self.run_gate()
 def test_picking_hash_changed_refuses(self):
  self.rows[PICKING_FAILED]['acquisition_sha256']='other'
  with self.assertRaises(ValueError):self.run_gate()
 def test_picking_hash_missing_refuses(self):
  self.picking['acquisition_sha256']=self.rows[PICKING_FAILED]['acquisition_sha256']=None
  with self.assertRaises(ValueError):self.run_gate()
 def test_picking_positive_credit_refuses(self):
  self.values[-1]['spec']['case']['id']=PICKING_FAILED
  with self.assertRaises(ValueError):self.run_gate()
if __name__=='__main__':unittest.main()
