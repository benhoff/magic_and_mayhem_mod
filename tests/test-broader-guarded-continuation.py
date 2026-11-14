#!/usr/bin/env python3
import copy,sys,unittest
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools'))
from broader_guarded_continuation import bound,predecessors
class Reader:
 def __init__(self,data):self.data=data
 def json(self,n):return self.data[n]
 def digest(self,n):return 'a'*64
class GuardContinuation(unittest.TestCase):
 def setUp(self):
  ref=lambda n:dict(record=n,sha256='a'*64)
  self.old=dict(success=True,previous_WP10_original_games=8,maximum_additional_original_games=112,source_continuation_review=ref('source'),maximum_combined_original_games=120)
  self.negative=dict(kind='broader_guard_stopped_unreported_acquisition',success=False,delegate_success=False,case_accepted=False,original_game_runs=2,charged_original_starts=2,actual_start_count_complete=True,observed_session_roots=['one','two'],private_wine_stopped=True,missing_unrecoverable_collector_observations=['a','b','c','d'],Cold_handoffs=[dict(real_restore_verified=True,replicas_verified=2)for _ in range(3)],RAM_handoff=dict(real_restore_verified=True,replicas_verified=2))
  self.review=dict(kind='broader_guard_stopped_continuation_review',success=True,original_game_runs=0,case_accepted=False,old_additional_ceiling=112,new_additional_ceiling=113,old_combined_ceiling=120,new_combined_ceiling=121,ceiling_extension=1,replacement_original_starts=2,prior_remaining_unallocated_starts=1,minimum_final_additional_reservations=113,reserved_failed_starts_retained=2,observed_failed_original_starts=2,reservations_refunded=False,native_inputs_or_actions_changed=False,user_numeric_approval_claimed=False,reviewer_sources={'source.py':'a'*64},failed_acquisition=ref('negative'),parent_global_plan=ref('old'),failed_case_id='stopped')
  self.reader=Reader(dict(review=self.review,negative=self.negative,old=self.old,source=dict(success=True,original_game_runs=0,reserved_original_starts_retained=12,previous_WP10_starts_retained=8,old_cases_current_runtime_credit=False)))
  self.plan=self.old|dict(maximum_additional_original_games=113,maximum_combined_original_games=121,maximum_combined_WP10_and_new_original_games=121,minimum_final_additional_reservations=113,maximum_manually_reviewed_additional_repairs=0,guard_stopped_review=ref('review'),preserved_setup_refusals={})
  self.row=dict(case_id='stopped',state='failed',reserved_games=2,report='negative',acquisition_sha256='a'*64)
 def test_smallest_extension_and_full_charge(self):self.assertEqual(bound(self.reader,self.plan),113);self.assertEqual(predecessors(self.reader,[self.row],self.plan),2)
 def test_larger_or_inconsistent_ceiling_refused(self):
  for patch in[dict(maximum_additional_original_games=114),dict(maximum_combined_original_games=122),dict(minimum_final_additional_reservations=112),dict(maximum_manually_reviewed_additional_repairs=1)]:
   with self.assertRaises(ValueError):bound(self.reader,self.plan|patch)
 def test_failure_never_becomes_a_pass(self):
  self.negative['success']=True
  with self.assertRaises(ValueError):bound(self.reader,self.plan)
 def test_refund_or_changed_input_refused(self):
  for key in('reservations_refunded','native_inputs_or_actions_changed','user_numeric_approval_claimed'):
   self.review[key]=True
   with self.assertRaises(ValueError):bound(self.reader,self.plan)
   self.review[key]=False
 def test_missing_or_incomplete_raw_custody_refused(self):
  self.negative['RAM_handoff']['replicas_verified']=1
  with self.assertRaises(ValueError):bound(self.reader,self.plan)
 def test_foreign_unknown_and_refunded_rows_refused(self):
  for row in[self.row|dict(reserved_games=1),self.row|dict(state='admitted'),self.row|dict(acquisition_sha256='b'*64),self.row|dict(case_id='unknown')]:
   with self.assertRaises(ValueError):predecessors(self.reader,[row],self.plan)
 def test_missing_reviewed_request_refused(self):
  with self.assertRaises(ValueError):predecessors(self.reader,[],self.plan)
if __name__=='__main__':unittest.main()
