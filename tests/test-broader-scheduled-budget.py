#!/usr/bin/env python3
"""The journal continuation retains every charge and admits no unknown retry."""
import copy,sys,unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from broader_scheduled_source_budget import bound,predecessors

class Reader:
 def __init__(self):
  old=[dict(case_id='historical-'+str(i),report='historical-'+str(i)+'.json',reserved_games=1 if i==0 else 2,state='failed'if i==0 else'admitted',acquisition_sha256='old')for i in range(25)]
  old += [dict(case_id='bp16-'+str(i),report='bp16-'+str(i)+'.json',reserved_games=2 if i<8 else 1,state='failed'if i==11 else'admitted',acquisition_sha256='old')for i in range(12)]
  old += [dict(case_id='publication-'+str(i),report='publication-'+str(i)+'.json',reserved_games=1,state='failed'if i==3 else'admitted',acquisition_sha256='old')for i in range(4)]
  old += [dict(case_id='capture-negative',report='capture-negative.json',reserved_games=1,state='acquired',acquisition_sha256='old')]
  old += [dict(case_id='counter-negative',report='counter-negative.json',reserved_games=1,state='failed',acquisition_sha256='old')]
  old += [dict(case_id='attachment-'+str(i),report='attachment-'+str(i)+'.json',reserved_games=1,state='admitted',acquisition_sha256='current')for i in range(4)]
  old += [dict(case_id='caller-refused',report='caller-refused.json',reserved_games=1,state='failed',acquisition_sha256=None)]
  old += [dict(case_id='caller-capacity-negative',report='caller-capacity-negative.json',reserved_games=1,state='failed',acquisition_sha256='old')]
  old += [dict(case_id='completed-'+str(i),report='completed-'+str(i)+'.json',reserved_games=1,state='admitted',acquisition_sha256='current')for i in range(12)]
  old += [dict(case_id='font-capacity-negative',report='font-capacity-negative.json',reserved_games=1,state='acquired',acquisition_sha256='old')]
  old += [dict(case_id='font-positive-'+str(i),report='font-positive-'+str(i)+'.json',reserved_games=1,state='admitted',acquisition_sha256='current')for i in range(4)]
  old += [dict(case_id='hotspot-positive',report='hotspot-positive.json',reserved_games=1,state='admitted',acquisition_sha256='current'),dict(case_id='hotspot-capacity-negative-current',report='hotspot-capacity-negative-current.json',reserved_games=1,state='acquired',acquisition_sha256='old')]
  old += [dict(case_id='scheduled-observer-negative',report='scheduled-observer-negative.json',reserved_games=1,state='acquired',acquisition_sha256='old')]
  self.hashes={'review':'review-sha','build':'build-sha','controls':'controls-sha','apps/current.cpp':'native-current','runtime/current.c':'runtime-current','tools/reviewer.py':'reviewer-current','tools/publisher.py':'publisher-current','binding':'binding-sha','apps/qt-shell/CMakeLists.txt':'config-current'}
  self.hashes['negative']='negative-sha'
  review=dict(hotspot_failure_custody=dict(record='negative',sha256='negative-sha'),hotspot_actions_or_caps_changed=False,kind='broader_copy_scheduling_continuation_review',success=True,original_game_runs=0,old_reserved_starts=101,retained_current_cases=35,remaining_positive_cases=55,remaining_positive_original_starts=55,new_additional_ceiling=156,new_combined_ceiling=164,reservations_refunded=False,agent_set_limit=True,user_numeric_approval_claimed=False,automatic_repair_starts=0,manually_reviewed_replacement_starts=9,changed_compiler_files=[],changed_runtime_files=[],game_response_deadlines_changed=False,reviewer_sources={'tools/reviewer.py':'reviewer-current'},current_build=dict(record='build',sha256='build-sha'),corrected_build_binding=dict(record='binding',sha256='binding-sha'),current_controls=[dict(record='controls',sha256='controls-sha')],old_requests=old,new_case_ids=['bp25-'+str(i)for i in range(55)])
  self.values={'review':review,'build':dict(success=True,sources_stable=True,compiler_sources={'apps/current.cpp':'native-current','apps/qt-shell/CMakeLists.txt':'config-current'},runtime_sources={'runtime/current.c':'runtime-current'}),'controls':dict(success=True,sources_stable=True,sources={'tools/publisher.py':'publisher-current','binding':'binding-sha','apps/qt-shell/CMakeLists.txt':'config-current'})}
  self.values['binding']=dict(success=True,new_native_builds=0,full_build_sources=dict(self.values['build']['compiler_sources']),player_build=dict(compiler_sources={'apps/current.cpp':'native-current'}),configuration_sources={'apps/qt-shell/CMakeLists.txt':'config-current'})
  self.values['negative']=dict(success=True,case_accepted=False,reservations_refunded=False,diagnosis=dict(diagnostic_copy_limit=8192,diagnostic_copies_checked=4328,missing_required_actual_COPY_observations=136),request_row_preserved_exactly=copy.deepcopy(old[-1]))
 def digest(self,n):return self.hashes[n]
 def json(self,n):return copy.deepcopy(self.values[n])
class Budget(unittest.TestCase):
 def setUp(self):
  self.r=Reader();self.plan=dict(copy_scheduling_review=dict(record='review',sha256='review-sha'),success=True,previous_WP10_original_games=8,maximum_additional_original_games=156,minimum_final_additional_reservations=156,maximum_combined_original_games=164,maximum_combined_WP10_and_new_original_games=164,maximum_manually_reviewed_additional_repairs=9);self.rows=copy.deepcopy(self.r.values['review']['old_requests'])
 def test_exact_old_failed_reservations_stay_charged(self):self.assertEqual(bound(self.r,self.plan),156);self.assertEqual(predecessors(self.r,self.rows,self.plan),101)
 def test_unknown_extra_failed_or_incomplete_new_case_refused(self):
  for cid,state,charge in [('unknown','admitted',1),('bp25-0','failed',1),('bp25-0','requested',1),('bp25-0','admitted',2)]:
   row=dict(case_id=cid,report=cid+'.json',reserved_games=charge,state=state,acquisition_sha256='new')
   with self.subTest(cid=cid,state=state,charge=charge):
    with self.assertRaises(ValueError):predecessors(self.r,self.rows+[row],self.plan)
 def test_one_exact_new_admission_adds_without_refund(self):
  row=dict(case_id='bp25-0',report='bp25-0.json',reserved_games=1,state='admitted',acquisition_sha256='new');self.assertEqual(predecessors(self.r,self.rows+[row],self.plan),102)
 def test_old_failed_row_relabel_or_removal_refused(self):
  for rows in [self.rows[1:],self.rows+ [copy.deepcopy(self.rows[0])]]:
   with self.assertRaises(ValueError):predecessors(self.r,rows,self.plan)
  self.rows[-1]['state']='admitted'
  with self.assertRaises(ValueError):predecessors(self.r,self.rows,self.plan)
 def test_native_or_runtime_change_cannot_be_waived(self):
  for n in ('apps/current.cpp','runtime/current.c'):
   old=self.r.hashes[n];self.r.hashes[n]='changed'
   with self.assertRaises(ValueError):bound(self.r,self.plan)
   self.r.hashes[n]=old
 def test_changed_control_or_reviewer_refused(self):
  for n in ('tools/publisher.py','tools/reviewer.py'):
   old=self.r.hashes[n];self.r.hashes[n]='changed'
   with self.assertRaises(ValueError):bound(self.r,self.plan)
   self.r.hashes[n]=old
 def test_ceiling_refund_or_automatic_repair_change_refused(self):
  for k,v in [('new_additional_ceiling',157),('reservations_refunded',True),('automatic_repair_starts',1),('user_numeric_approval_claimed',True)]:
   old=self.r.values['review'][k];self.r.values['review'][k]=v
   with self.assertRaises(ValueError):bound(self.r,self.plan)
   self.r.values['review'][k]=old
 def test_reused_future_case_identity_refused(self):
  self.r.values['review']['new_case_ids'][0]=self.rows[0]['case_id']
  with self.assertRaises(ValueError):bound(self.r,self.plan)
 def test_configuration_dependency_cannot_be_removed(self):
  self.r.values['binding']['configuration_sources'].clear()
  with self.assertRaises(ValueError):bound(self.r,self.plan)
 def test_hotspot_gap_or_refund_cannot_be_relabelled(self):
  for key,value in [('case_accepted',True),('reservations_refunded',True)]:
   old=self.r.values['negative'][key];self.r.values['negative'][key]=value
   with self.assertRaises(ValueError):bound(self.r,self.plan)
   self.r.values['negative'][key]=old
  self.r.values['negative']['diagnosis']['missing_required_actual_COPY_observations']=0
  with self.assertRaises(ValueError):bound(self.r,self.plan)
 def test_hotspot_actions_or_caps_cannot_change(self):
  self.r.values['review']['hotspot_actions_or_caps_changed']=True
  with self.assertRaises(ValueError):bound(self.r,self.plan)
if __name__=='__main__':unittest.main()
