"""Conservative finite-admission controls; no Original or Native execution."""
import copy,sys,unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from broader_copy_source_budget import bound,predecessors
class Reader:
 def __init__(self):
  self.values={};self.hashes={}
 def add(self,n,v):self.values[n]=v;self.hashes[n]='hash-'+n;return dict(record=n,sha256=self.hashes[n])
 def digest(self,n):return self.hashes[n]
 def json(self,n):return self.values[n]
class Budget(unittest.TestCase):
 def setUp(self):
  r=self.r=Reader();r.hashes['source']='sourcehash';old=[dict(case_id='old'+str(i),report='old'+str(i)+'-acquisition',reserved_games=2 if i<32 else 1,state='failed'if i<4 else'admitted',acquisition_sha256='payload')for i in range(71)];new={'new'+str(i):2 if i<8 else 1 for i in range(90)};build=r.add('build',dict(success=True,sources_stable=True,compiler_sources={'source':'sourcehash'},runtime_sources={}));control=r.add('control',dict(success=True,sources_stable=True,sources={'source':'sourcehash'}));review=dict(kind='broader_COPY_source_cohort_review',success=True,original_game_runs=0,old_reserved_starts=103,fresh_positive_cases=90,fresh_positive_original_starts=98,new_additional_ceiling=201,new_combined_ceiling=209,old_source_current_credit=False,reservations_refunded=False,automatic_repair_starts=0,game_response_deadlines_changed=False,reviewer_sources={'source':'sourcehash'},current_controls=[control],current_build=build,old_requests=old,new_case_reservations=new);self.review=review;self.plan=dict(COPY_source_review=r.add('review',review),success=True,previous_WP10_original_games=8,maximum_additional_original_games=201,minimum_final_additional_reservations=201,maximum_combined_original_games=209,maximum_combined_WP10_and_new_original_games=209);self.old=old
 def row(self,i=0):return dict(case_id='new'+str(i),report='new'+str(i)+'-acquisition',reserved_games=2 if i<8 else 1,state='admitted',acquisition_sha256='newpayload')
 def test_exact_complete(self):self.assertEqual(bound(self.r,self.plan),201);self.assertEqual(predecessors(self.r,self.old+[self.row(i)for i in range(90)],self.plan),201)
 def test_no_refund_historical_negative(self):
  rows=copy.deepcopy(self.old);rows[0]['reserved_games']=0
  with self.assertRaises(ValueError):predecessors(self.r,rows,self.plan)
 def test_no_historical_rewrite(self):
  rows=copy.deepcopy(self.old);rows[0]['state']='admitted'
  with self.assertRaises(ValueError):predecessors(self.r,rows,self.plan)
 def test_no_missing_old(self):
  with self.assertRaises(ValueError):predecessors(self.r,self.old[1:],self.plan)
 def test_no_retry_after_bad_new(self):
  for state in ['requested','acquired','failed']:
   row=self.row();row['state']=state
   with self.subTest(state=state),self.assertRaises(ValueError):predecessors(self.r,self.old+[row],self.plan)
 def test_no_unknown_or_duplicate(self):
  for rows in [self.old+[dict(self.row(),case_id='unknown')],self.old+[self.row(),self.row()]]:
   with self.assertRaises(ValueError):predecessors(self.r,rows,self.plan)
 def test_no_short_charge_fault(self):
  with self.assertRaises(ValueError):predecessors(self.r,self.old+[dict(self.row(),reserved_games=1)],self.plan)
 def test_native_drift_refused(self):
  self.r.hashes['source']='changed'
  with self.assertRaises(ValueError):bound(self.r,self.plan)
 def test_identity_drift_refused(self):
  self.plan['COPY_source_review']['sha256']='foreign'
  with self.assertRaises(ValueError):bound(self.r,self.plan)
 def test_no_ceiling_expansion(self):
  self.plan['maximum_additional_original_games']=202
  with self.assertRaises(ValueError):bound(self.r,self.plan)
 def test_no_old_source_credit(self):
  self.review['old_source_current_credit']=True
  with self.assertRaises(ValueError):bound(self.r,self.plan)
if __name__=='__main__':unittest.main()
