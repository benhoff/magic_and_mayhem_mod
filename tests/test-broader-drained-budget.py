#!/usr/bin/env python3
import copy,importlib.util,sys,unittest
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools'))
spec=importlib.util.spec_from_file_location('parent_budget_controls',ROOT/'tests/test-broader-worker-source-budget.py');base=importlib.util.module_from_spec(spec);spec.loader.exec_module(base)
from broader_drained_source_budget import bound,predecessors
spec=importlib.util.spec_from_file_location('drained_recovery_adapter',ROOT/'tools/run-broader-drained-recovery.py');adapter=importlib.util.module_from_spec(spec);spec.loader.exec_module(adapter)
class Budget(unittest.TestCase):
 def setUp(self):
  parent=base.SourceBudget();parent.setUp();self.r=parent.r;ref=lambda n:dict(record=n,sha256='a'*64);self.r.data['parent-plan']=parent.p
  self.rows=copy.deepcopy(parent.rows)+[dict(case_id='bp13-positive-'+str(i),report='bp13-positive-'+str(i)+'.json',reserved_games=2,state='admitted',acquisition_sha256='a'*64)for i in range(5)]+[dict(case_id='bp13-transition-default800-01',report='bp13-transition-default800-01.json',reserved_games=2,state='failed',acquisition_sha256='a'*64)]
  self.q=dict(kind='broader_drained_admission_continuation_review',success=True,original_game_runs=0,old_additional_ceiling=128,new_additional_ceiling=130,old_combined_ceiling=136,new_combined_ceiling=138,retained_predecessor_reservations=42,remaining_positive_cases=85,remaining_positive_original_starts=88,reservations_refunded=False,agent_set_limit=True,user_numeric_approval_claimed=False,automatic_repair_starts=0,native_player_changed=False,response_deadlines_changed=False,parent_global_plan=ref('parent-plan'),reviewer_sources={'drain-review.py':'a'*64},controls=[ref('control')],predecessor_requests=copy.deepcopy(self.rows),remaining_case_ids=['bp13-new-'+str(i)for i in range(85)])
  self.q['negative_custody']=ref('negative-custody');self.r.data['negative-custody']=dict(success=True,case_accepted=False,reservations_refunded=False,request_row_preserved_exactly=copy.deepcopy(self.rows[-1]))
  self.r.data['drain-review']=self.q;self.p=parent.p|dict(drain_continuation_review=ref('drain-review'),maximum_additional_original_games=130,maximum_combined_original_games=138,maximum_combined_WP10_and_new_original_games=138,minimum_final_additional_reservations=130)
 def test_exact_bound_and_old_failure_charged(self):self.assertEqual(bound(self.r,self.p),130);self.assertEqual(predecessors(self.r,self.rows,self.p),42)
 def test_new_admitted_case_is_charged(self):self.assertEqual(predecessors(self.r,self.rows+[dict(case_id='bp13-new-0',report='bp13-new-0.json',reserved_games=1,state='admitted',acquisition_sha256='a'*64)],self.p),43)
 def test_missing_changed_refunded_duplicate_predecessor_refused(self):
  for rows in [self.rows[:-1],self.rows+[self.rows[0]],self.rows[:-1]+[self.rows[-1]|dict(reserved_games=1)],self.rows[:-1]+[self.rows[-1]|dict(state='admitted')]]:
   with self.assertRaises(ValueError):predecessors(self.r,rows,self.p)
 def test_unknown_unfinished_or_foreign_case_refused(self):
  row=dict(case_id='bp13-new-0',report='bp13-new-0.json',reserved_games=1,state='admitted',acquisition_sha256='a'*64)
  for patch in [dict(state='failed'),dict(state='requested'),dict(acquisition_sha256=None),dict(case_id='unknown'),dict(reserved_games=3)]:
   with self.assertRaises(ValueError):predecessors(self.r,self.rows+[row|patch],self.p)
 def test_no_native_waiver_deadline_refund_or_user_claim(self):
  for n in ['native_player_changed','response_deadlines_changed','reservations_refunded','user_numeric_approval_claimed']:
   self.q[n]=True
   with self.assertRaises(ValueError):bound(self.r,self.p)
   self.q[n]=False
 def test_parent_native_and_current_reviewer_drift_refused(self):
  for n in ['native.hpp','runtime.c','review.py','test.cpp','drain-review.py']:
   self.r.hashes[n]='b'*64
   with self.assertRaises(ValueError):bound(self.r,self.p)
   del self.r.hashes[n]
 def test_spare_or_changed_ceiling_refused(self):
  for patch in [dict(maximum_additional_original_games=132),dict(maximum_combined_original_games=140),dict(minimum_final_additional_reservations=129),dict(maximum_manually_reviewed_additional_repairs=1)]:
   with self.assertRaises(ValueError):bound(self.r,self.p|patch)
 def test_changed_future_denominator_refused(self):
  self.q['remaining_case_ids'].pop()
  with self.assertRaises(ValueError):predecessors(self.r,self.rows,self.p)
 def test_repair_flags_only_on_exact_original_collector(self):
  case=dict(repair_of=dict(record='negative.json'),repair_reason='Retained drain admission refusal')
  self.assertEqual(adapter.repair_arguments(['tools/run-rendering-transition.py','--plan','p'],case),['tools/run-rendering-transition.py','--plan','p','--repair-of','negative.json','--repair-reason','Retained drain admission refusal'])
  self.assertEqual(adapter.repair_arguments(['other.py'],case),['other.py']);self.assertEqual(adapter.repair_arguments(['tools/run-rendering-transition.py'],{}),['tools/run-rendering-transition.py'])
  with self.assertRaises(ValueError):adapter.repair_arguments(['tools/run-rendering-transition.py','--repair-of','x'],case)
if __name__=='__main__':unittest.main()
