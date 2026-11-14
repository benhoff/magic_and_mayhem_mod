#!/usr/bin/env python3
"""Exact setup-charge extension controls; synthetic rows grant no game credit."""
import copy,sys,unittest
from pathlib import Path
from unittest.mock import patch
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
import broader_setup_continuation_budget as policy
class Budget(unittest.TestCase):
 def setUp(self):
  ref=lambda n:dict(record=n,sha256='hash:'+n)
  self.old=[dict(case_id='old-'+str(i),report='old-'+str(i)+'-report',plan='oldplan',reserved_games=2 if i<32 else 1,state='admitted',started_ns=i,acquisition_sha256='oldhash',validation='oldval')for i in range(71)]
  self.closed=dict(case_id='bp27-world-attachment-default640-01',report='failed-report',plan='oldplan',reserved_games=1,state='failed',started_ns=71,acquisition_sha256=None,validation=None)
  ids=['bp27-world-attachment-'+s+'-01'for s in['default640','default800','software640','software800']]+['new-'+str(i)for i in range(86)]
  positive={n:2 if 4<=i<12 else 1 for i,n in enumerate(ids)}
  self.base=dict(old_requests=self.old,new_case_reservations=positive)
  self.q=dict(success=True,kind='B06_reviewed_zero_start_setup_continuation',original_game_runs=0,Native_sources_unchanged=True,old_source_current_credit=False,reservations_refunded=False,automatic_retries=False,base_plan=ref('base'),old_requests=copy.deepcopy(self.old),closed_setup_certificate=ref('failure'),closed_setup_requests=[copy.deepcopy(self.closed)],positive_case_reservations={n.replace('bp27-world-attachment-','bp28-world-attachment-'):v for n,v in positive.items()},maximum_additional_charges=202,maximum_combined_with_WP10=210,additional_charge_for_closed_setup=1,preserved_current_contexts=18,preserved_current_actual_starts=26,reviewer_sources={'policy':'hash:policy'})
  failure=dict(success=True,original_game_runs=0,new_native_executions=0,failed_request=copy.deepcopy(self.closed),failed_request_must_not_be_rewritten_or_refunded=True,actual_start_proof=dict(before_player_or_Original_launch=True))
  self.data={'review':self.q,'base':dict(success=True),'failure':failure};outer=self
  class Reader:
   def digest(self,n):return 'hash:'+n
   def json(self,n):return outer.data[n]
  self.r=Reader();self.plan=dict(success=True,setup_continuation_review=ref('review'),previous_WP10_original_games=8,maximum_additional_original_games=202,minimum_final_additional_reservations=202,maximum_combined_original_games=210,maximum_combined_WP10_and_new_original_games=210)
  self.mock=patch.object(policy,'original_review',return_value=self.base);self.mock.start();self.addCleanup(self.mock.stop)
 def rows(self):
  return copy.deepcopy(self.old+[self.closed])+[dict(case_id=n,report=n+'-report',plan='newplan',reserved_games=v,state='admitted',started_ns=100+i,acquisition_sha256='newhash',validation='newval')for i,(n,v)in enumerate(self.q['positive_case_reservations'].items())]
 def test_exact_positive_matrix_plus_charged_failure_totals202(self):self.assertEqual(policy.bound(self.r,self.plan),202);self.assertEqual(policy.predecessors(self.r,self.rows(),self.plan),202)
 def test_closed_setup_row_cannot_disappear(self):
  with self.assertRaises(ValueError):policy.predecessors(self.r,[r for r in self.rows()if r['case_id']!=self.closed['case_id']],self.plan)
 def test_closed_charge_cannot_be_refunded(self):
  rows=self.rows();rows[71]['reserved_games']=0
  with self.assertRaises(ValueError):policy.predecessors(self.r,rows,self.plan)
 def test_failed_setup_cannot_be_rewritten_admitted(self):
  rows=self.rows();rows[71]['state']='admitted'
  with self.assertRaises(ValueError):policy.predecessors(self.r,rows,self.plan)
 def test_unproved_actual_start_does_not_qualify(self):
  self.data['failure']['original_game_runs']=1
  with self.assertRaises(ValueError):policy.bound(self.r,self.plan)
 def test_only_attachment_identities_may_change(self):
  q=self.q['positive_case_reservations'];q['other']=q.pop('new-0')
  with self.assertRaises(ValueError):policy.bound(self.r,self.plan)
 def test_one_extra_positive_case_refuses(self):
  self.q['positive_case_reservations']['extra']=1
  with self.assertRaises(ValueError):policy.bound(self.r,self.plan)
 def test_original71_history_cannot_change(self):
  self.q['old_requests'][0]['plan']='other'
  with self.assertRaises(ValueError):policy.bound(self.r,self.plan)
 def test_failed_new_case_stops_next_acquisition(self):
  rows=self.rows();rows[-1]['state']='failed'
  with self.assertRaises(ValueError):policy.predecessors(self.r,rows,self.plan)
 def test_unknown_request_refuses(self):
  rows=self.rows();rows[-1]['case_id']='unknown'
  with self.assertRaises(ValueError):policy.predecessors(self.r,rows,self.plan)
 def test_duplicate_request_refuses(self):
  rows=self.rows();rows.append(copy.deepcopy(rows[-1]))
  with self.assertRaises(ValueError):policy.predecessors(self.r,rows,self.plan)
 def test201_or203_ceiling_refuses(self):
  for n in[201,203]:
   self.plan['maximum_additional_original_games']=n
   with self.assertRaises(ValueError):policy.bound(self.r,self.plan)
 def test_stale_reviewer_source_refuses(self):
  self.q['reviewer_sources']['policy']='old'
  with self.assertRaises(ValueError):policy.bound(self.r,self.plan)
 def test_automatic_retry_policy_refuses(self):
  self.q['automatic_retries']=True
  with self.assertRaises(ValueError):policy.bound(self.r,self.plan)
if __name__=='__main__':unittest.main()
