"""Current runtime final-ledger refusals; synthetic fixtures are not execution evidence."""
import copy,sys,unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from broader_runtime_final_gate import exact_ledger
class Ledger(unittest.TestCase):
 def setUp(self):
  old=[dict(case_id='old-'+str(i),reserved_games=2 if i<40 else 1,state='failed'if i==92 else'admitted',report='old-'+str(i),plan='old-plan',started_ns=i,validation='val',acquisition_sha256='hash')for i in range(93)]
  new=[dict(case_id='new-'+str(i),reserved_games=2 if i<8 else 1,state='admitted')for i in range(90)]
  self.rows={v['case_id']:copy.deepcopy(v)for v in old+new};self.values=[dict(spec=dict(case=dict(id=v['case_id'],reserved_original_games=v['reserved_games'])))for v in new];self.q=dict(old_requests=old)
 def test_complete_finite_ledger(self):exact_ledger(self.rows,self.values,self.q)
 def test_one_missing_context_refuses(self):
  with self.assertRaises(ValueError):exact_ledger(self.rows,self.values[:-1],self.q)
 def test_one_missing_historical_row_refuses(self):
  del self.rows['old-92']
  with self.assertRaises(ValueError):exact_ledger(self.rows,self.values,self.q)
 def test_extra_context_refuses(self):
  self.values.append(copy.deepcopy(self.values[-1]))
  with self.assertRaises(ValueError):exact_ledger(self.rows,self.values,self.q)
 def test_unknown_extra_row_refuses(self):
  self.rows['other']=dict(case_id='other',reserved_games=1)
  with self.assertRaises(ValueError):exact_ledger(self.rows,self.values,self.q)
 def test_historical_failed_state_cannot_promote(self):
  self.rows['old-92']['state']='admitted'
  with self.assertRaises(ValueError):exact_ledger(self.rows,self.values,self.q)
 def test_historical_hash_cannot_refresh(self):
  self.rows['old-0']['acquisition_sha256']='current'
  with self.assertRaises(ValueError):exact_ledger(self.rows,self.values,self.q)
 def test_historical_charge_cannot_refund(self):
  self.rows['old-92']['reserved_games']=0
  with self.assertRaises(ValueError):exact_ledger(self.rows,self.values,self.q)
 def test_duplicate_positive_identity_refuses(self):
  self.values[-1]['spec']['case']['id']=self.values[-2]['spec']['case']['id']
  with self.assertRaises(ValueError):exact_ledger(self.rows,self.values,self.q)
 def test_old_context_cannot_receive_current_credit(self):
  self.values[-1]['spec']['case']['id']='old-0'
  with self.assertRaises(ValueError):exact_ledger(self.rows,self.values,self.q)
if __name__=='__main__':unittest.main()
