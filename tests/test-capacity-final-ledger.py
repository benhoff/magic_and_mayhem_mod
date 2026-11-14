import copy,sys,unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from broader_capacity_final_gate import exact_capacity_ledger,FAILED
class LedgerTests(unittest.TestCase):
 def setUp(self):
  self.old=[dict(case_id='old-'+str(i),reserved_games=2 if i<40 else 1,state='admitted')for i in range(93)];self.failed=dict(case_id=FAILED,reserved_games=1,state='failed',acquisition_sha256='negative');self.values=[dict(spec=dict(case=dict(id='positive-'+str(i),reserved_original_games=2 if i<8 else 1)))for i in range(90)];self.rows={v['case_id']:copy.deepcopy(v)for v in self.old+[self.failed]};self.rows.update({v['spec']['case']['id']:dict(reserved_games=v['spec']['case']['reserved_original_games'],state='admitted')for v in self.values})
 def run_gate(self):exact_capacity_ledger(self.rows,self.values,self.old,self.failed)
 def test_complete(self):self.run_gate()
 def test_old231refuses(self):
  self.rows.pop(FAILED)
  with self.assertRaises(ValueError):self.run_gate()
 def test_refund_refuses(self):
  self.rows[FAILED]['reserved_games']=0
  with self.assertRaises(ValueError):self.run_gate()
 def test_negative_promoted_refuses(self):
  self.rows[FAILED]['state']='admitted'
  with self.assertRaises(ValueError):self.run_gate()
 def test_missing_positive_refuses(self):
  self.rows.pop('positive-0')
  with self.assertRaises(ValueError):self.run_gate()
 def test_extra_request_refuses(self):
  self.rows['spare']=dict(reserved_games=1)
  with self.assertRaises(ValueError):self.run_gate()
 def test_duplicate_positive_refuses(self):
  self.values[-1]=copy.deepcopy(self.values[-2])
  with self.assertRaises(ValueError):self.run_gate()
 def test_old_metadata_changed_refuses(self):
  self.rows['old-0']['state']='failed'
  with self.assertRaises(ValueError):self.run_gate()
 def test_negative_hash_changed_refuses(self):
  self.rows[FAILED]['acquisition_sha256']='changed'
  with self.assertRaises(ValueError):self.run_gate()
 def test_failure_receives_positive_credit_refuses(self):
  self.values[-1]['spec']['case']['id']=FAILED
  with self.assertRaises(ValueError):self.run_gate()
 def test_history_receives_positive_credit_refuses(self):
  self.values[-1]['spec']['case']['id']='old-0'
  with self.assertRaises(ValueError):self.run_gate()
 def test_start_domain_shrinks_refuses(self):
  self.values[0]['spec']['case']['reserved_original_games']=1
  with self.assertRaises(ValueError):self.run_gate()
if __name__=='__main__':unittest.main()
