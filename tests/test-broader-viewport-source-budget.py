#!/usr/bin/env python3
import copy,sys,unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from broader_viewport_source_budget import bound,predecessors
class Reader:
 def __init__(self):self.data={};self.hashes={}
 def digest(self,n):return self.hashes.get(n,'a'*64)
 def json(self,n):return self.data[n]
class SourceBudget(unittest.TestCase):
 def setUp(self):
  self.r=Reader();ref=lambda n:dict(record=n,sha256='a'*64)
  self.rows=[dict(case_id='old-'+str(i),report='old-'+str(i)+'.json',reserved_games=2,state='failed'if i==12 else'acquired',acquisition_sha256='a'*64)for i in range(25)]
  self.rows[-1]['reserved_games']=1
  self.q=dict(kind='broader_viewport_source_continuation_review',success=True,original_game_runs=0,old_source_current_credit=False,old_reserved_starts=49,old_additional_ceiling=126,new_additional_ceiling=147,old_combined_ceiling=134,new_combined_ceiling=155,fresh_positive_cases=90,fresh_positive_original_starts=98,reservations_refunded=False,agent_set_limit=True,user_numeric_approval_claimed=False,automatic_repair_starts=0,queue_limits_increased=False,game_response_deadlines_changed=False,reviewer_sources={'review.py':'a'*64},current_build=ref('build'),changed_compiler_files=['apps/qt-shell/acceptance_capture.hpp'],changed_runtime_files=[],current_controls=[ref('control')],old_requests=copy.deepcopy(self.rows))
  self.r.data.update(review=self.q,build=dict(success=True,sources_stable=True,old_B04_preserved=True,compiler_sources={'native.hpp':'a'*64},runtime_sources={'runtime.c':'a'*64}),control=dict(success=True,sources_stable=True,sources={'test.cpp':'a'*64}))
  self.p=dict(viewport_source_review=ref('review'),success=True,maximum_additional_original_games=147,maximum_combined_original_games=155,maximum_combined_WP10_and_new_original_games=155,minimum_final_additional_reservations=147,maximum_manually_reviewed_additional_repairs=0,previous_WP10_original_games=8)
 def test_frozen_old_rows_keep_all_charges(self):self.assertEqual(bound(self.r,self.p),147);self.assertEqual(predecessors(self.r,self.rows,self.p),49)
 def test_current_only_after_admission(self):
  row=dict(case_id='bp16-loss-default640-01',report='bp16-loss-default640-01.json',reserved_games=2,state='admitted',acquisition_sha256='a'*64)
  self.assertEqual(predecessors(self.r,self.rows+[row],self.p),51)
  for patch in [dict(state='failed'),dict(state='requested'),dict(case_id='bp11-new'),dict(acquisition_sha256=None),dict(reserved_games=3)]:
   with self.assertRaises(ValueError):predecessors(self.r,self.rows+[row|patch],self.p)
 def test_old_missing_refunded_changed_duplicate_refused(self):
  for rows in [self.rows[:-1],self.rows+[self.rows[0]],self.rows[:-1]+[self.rows[-1]|dict(reserved_games=2)],self.rows[:-1]+[self.rows[-1]|dict(state='admitted')]]:
   with self.assertRaises(ValueError):predecessors(self.r,rows,self.p)
 def test_no_source_waiver_refund_or_queue_bump(self):
  for k in ['old_source_current_credit','reservations_refunded','queue_limits_increased','user_numeric_approval_claimed','game_response_deadlines_changed']:
   self.q[k]=True
   with self.assertRaises(ValueError):bound(self.r,self.p)
   self.q[k]=False
 def test_stale_native_runtime_control_or_reviewer_refused(self):
  for n in ['native.hpp','runtime.c','test.cpp','review.py']:
   self.r.hashes[n]='b'*64
   with self.assertRaises(ValueError):bound(self.r,self.p)
   del self.r.hashes[n]
 def test_larger_budget_or_spare_repair_refused(self):
  for patch in [dict(maximum_additional_original_games=148),dict(maximum_combined_original_games=156),dict(minimum_final_additional_reservations=146),dict(maximum_manually_reviewed_additional_repairs=1)]:
   with self.assertRaises(ValueError):bound(self.r,self.p|patch)
 def test_unreviewed_cpp_delta_refused(self):
  self.q['changed_runtime_files']=['runtime.c']
  with self.assertRaises(ValueError):bound(self.r,self.p)
if __name__=='__main__':unittest.main()
