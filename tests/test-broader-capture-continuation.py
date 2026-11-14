#!/usr/bin/env python3
import copy,importlib.util,sys,unittest
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools'))
from broader_capture_continuation import bound,predecessors
spec=importlib.util.spec_from_file_location('old_guard_fixture',ROOT/'tests/test-broader-guarded-continuation.py');old=importlib.util.module_from_spec(spec);spec.loader.exec_module(old)
class CaptureContinuation(unittest.TestCase):
 def setUp(self):
  base=old.GuardContinuation();base.setUp();ref=lambda n:dict(record=n,sha256='a'*64);self.r=base.reader;self.guardrow=base.row;self.r.data['old113']=base.plan
  self.a=dict(success=True,original_game_runs=2,private_wine_stopped=True,prefix_pool=dict(returned=True),proof=dict(epochs=[{},{}]));self.c=dict(success=False,error='Capture incomplete or truncated',comparison_processes_stopped=True,sources_stable=True,acquisition=ref('capture_a'))
  self.d=dict(success=True,case_accepted=False,acquisition=ref('capture_a'),failed_comparison=ref('capture_c'),missing_command_bytes=30089412,stopped_ring_capacity=1048576,missing_tail_recoverable_from_retained_ring=False,synthetic_END_or_replies_added=False,raw_capture_complete_flags_preserved=True,capture_limits_changed=False)
  self.h=dict(success=True,case_accepted=False,acquisition=ref('capture_a'),failed_comparison=ref('capture_c'),diagnosis=ref('capture_d'),reservations_refunded=False,Cold_handoffs=[dict(real_restore_verified=True,replicas_verified=2)for _ in range(3)],RAM_handoff=dict(real_restore_verified=True,replicas_verified=2))
  self.m=dict(success=True,total_completed_source_copies=58,source_checkouts_removed=0,result=dict(real_restore_verified=True,replicas_verified=2))
  self.q=dict(kind='broader_capture_refusal_continuation_review',success=True,original_game_runs=0,case_accepted=False,old_additional_ceiling=113,new_additional_ceiling=115,old_combined_ceiling=121,new_combined_ceiling=123,ceiling_extension=2,replacement_original_starts=2,reserved_failed_starts_retained=2,reservations_refunded=False,agent_set_limit=True,user_numeric_approval_claimed=False,native_sources_actions_deadlines_or_capture_limits_changed=False,automatic_repairs_remaining=0,reviewer_sources={'source.py':'a'*64},parent_global_plan=ref('old113'),maintenance_checkpoint=ref('maintenance'),failed_acquisition=ref('capture_a'),failed_comparison=ref('capture_c'),capture_diagnosis=ref('capture_d'),negative_custody=ref('capture_h'),failed_case_id='capture-negative',negative_predecessor_review=ref('ordinary_review'))
  ordinary=dict(kind='broader_player_negative_predecessor_review',success=True,case_accepted=False,original_game_runs=0,reserved_original_starts_retained=2,acquisition=ref('capture_a'),failed_comparison=ref('capture_c'),source_continuation_review=base.plan['source_continuation_review'],reviewer_sources={'source.py':'a'*64})
  self.r.data.update(capture_a=self.a,capture_c=self.c,capture_d=self.d,capture_h=self.h,maintenance=self.m,capture_review=self.q,ordinary_review=ordinary)
  self.p=base.plan|dict(maximum_additional_original_games=115,maximum_combined_original_games=123,maximum_combined_WP10_and_new_original_games=123,minimum_final_additional_reservations=115,capture_refusal_review=ref('capture_review'),preserved_negative_predecessors={'capture-negative':ref('ordinary_review')});self.row=dict(case_id='capture-negative',state='acquired',reserved_games=2,report='capture_a',acquisition_sha256='a'*64)
 def test_exact_extension_retains_both_negative_generations(self):self.assertEqual(bound(self.r,self.p),115);self.assertEqual(predecessors(self.r,[self.guardrow,self.row],self.p),4)
 def test_refund_or_input_capture_limit_changes_refused(self):
  for key in ['reservations_refunded','native_sources_actions_deadlines_or_capture_limits_changed','user_numeric_approval_claimed']:
   self.q[key]=True
   with self.assertRaises(ValueError):bound(self.r,self.p)
   self.q[key]=False
 def test_larger_inconsistent_or_spare_repair_budget_refused(self):
  for patch in [dict(maximum_additional_original_games=116),dict(maximum_combined_original_games=124),dict(minimum_final_additional_reservations=113),dict(maximum_manually_reviewed_additional_repairs=1)]:
   with self.assertRaises(ValueError):bound(self.r,self.p|patch)
 def test_missing_tail_cannot_be_synthesized_or_waived(self):
  for key in ['missing_tail_recoverable_from_retained_ring','synthetic_END_or_replies_added','capture_limits_changed']:
   self.d[key]=True
   with self.assertRaises(ValueError):bound(self.r,self.p)
   self.d[key]=False
 def test_incomplete_maintenance_or_custody_refused(self):
  self.m['total_completed_source_copies']=57
  with self.assertRaises(ValueError):bound(self.r,self.p)
  self.m['total_completed_source_copies']=58;self.h['RAM_handoff']['replicas_verified']=1
  with self.assertRaises(ValueError):bound(self.r,self.p)
 def test_unknown_missing_or_refunded_requests_refused(self):
  for rows in [[self.guardrow],[self.guardrow,self.row|dict(reserved_games=1)],[self.guardrow,self.row|dict(state='admitted')],[self.guardrow,self.row|dict(case_id='other')]]:
   with self.assertRaises(ValueError):predecessors(self.r,rows,self.p)
 def test_negative_comparison_never_becomes_accepted(self):
  self.c['success']=True
  with self.assertRaises(ValueError):bound(self.r,self.p)
if __name__=='__main__':unittest.main()
