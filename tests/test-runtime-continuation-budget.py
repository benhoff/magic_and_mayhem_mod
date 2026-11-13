"""Historical custody and stop-on-failure controls; synthetic rows grant no game credit."""
import copy,sys,unittest
from pathlib import Path
from unittest.mock import patch
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
import broader_runtime_continuation_budget as policy
class Budget(unittest.TestCase):
 def setUp(self):
  self.old=[dict(case_id='old-'+str(i),report='old-'+str(i),plan='old-plan',reserved_games=2 if i<40 else 1,state='failed'if i==92 else'admitted',started_ns=i,acquisition_sha256='hash',validation='val')for i in range(93)]
  self.positive={'bp29-case-'+str(i):2 if i<8 else 1 for i in range(90)};self.q=dict(old_requests=self.old,new_case_reservations=self.positive);self.plan=dict(success=True,previous_WP10_original_games=8,maximum_additional_original_games=231,minimum_final_additional_reservations=231,maximum_combined_original_games=239,maximum_combined_WP10_and_new_original_games=239)
  p=patch.object(policy,'review',return_value=self.q);p.start();self.addCleanup(p.stop)
 def rows(self):return copy.deepcopy(self.old)+[dict(case_id=k,report=k+'-report',plan='new-plan',reserved_games=v,state='admitted',started_ns=i+100,acquisition_sha256='hash',validation='val')for i,(k,v)in enumerate(self.positive.items())]
 def test_exact_ledger(self):self.assertEqual(policy.bound(None,self.plan),231);self.assertEqual(policy.predecessors(None,self.rows(),self.plan),231)
 def test_failed_historical_request_kept(self):self.assertEqual(policy.predecessors(None,copy.deepcopy(self.old),self.plan),133)
 def test_no_refund(self):
  rows=self.rows();rows[92]['reserved_games']=0
  with self.assertRaises(ValueError):policy.predecessors(None,rows,self.plan)
 def test_no_omitted_historical_row(self):
  with self.assertRaises(ValueError):policy.predecessors(None,self.rows()[1:],self.plan)
 def test_no_historical_promotion(self):
  rows=self.rows();rows[92]['state']='admitted'
  with self.assertRaises(ValueError):policy.predecessors(None,rows,self.plan)
 def test_failed_new_row_stops(self):
  rows=self.rows();rows[-1]['state']='failed'
  with self.assertRaises(ValueError):policy.predecessors(None,rows,self.plan)
 def test_incomplete_new_row_stops(self):
  rows=self.rows();rows[-1]['state']='acquired'
  with self.assertRaises(ValueError):policy.predecessors(None,rows,self.plan)
 def test_unknown_row_stops(self):
  rows=self.rows();rows[-1]['case_id']='unknown'
  with self.assertRaises(ValueError):policy.predecessors(None,rows,self.plan)
 def test_duplicate_row_stops(self):
  rows=self.rows();rows.append(copy.deepcopy(rows[-1]))
  with self.assertRaises(ValueError):policy.predecessors(None,rows,self.plan)
 def test_wrong_new_charge_stops(self):
  rows=self.rows();rows[-1]['reserved_games']=2
  with self.assertRaises(ValueError):policy.predecessors(None,rows,self.plan)
 def test_missing_hash_stops(self):
  rows=self.rows();rows[-1]['acquisition_sha256']=None
  with self.assertRaises(ValueError):policy.predecessors(None,rows,self.plan)
 def test_changed_ceiling_stops(self):
  self.plan['maximum_additional_original_games']=232
  with self.assertRaises(ValueError):policy.bound(None,self.plan)
if __name__=='__main__':unittest.main()
