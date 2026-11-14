#!/usr/bin/env python3
import copy,sys,unittest
from pathlib import Path
from types import SimpleNamespace
from unittest.mock import patch
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools'))
import broader_interaction_recent_current as m
from broader_interaction_recent_sources import POLICY
class Current(unittest.TestCase):
 def setUp(self):
  self.phase=dict(family='interaction',recent_interaction_sources=dict(policy=POLICY));self.rows=[dict(ordinal=i,kind=1,thread=1,frame=dict(generation=i,nonce='1'),history=[1,2,3],position=[4,5,6],camera=[0]*7,mode=0)for i in range(1,13)]
  self.a=dict(success=True,sources_stable=True,actual_provider_verified=True,provider_adaptation=dict(recent_interaction_sources=dict(policy=POLICY,maximum_age_frames=2,selected_opaque_pixel_formula_unchanged=True,prefix_dependencies_decoded_and_hashed=True,materialized_only_requested_World=True,action_schedule_changed=False,old_15second_readiness_deadline_preserved=True,hit_feedback_search=False,Native_runtime_sources_changed=False)),artifacts=[dict(path='owned/frame-input.bin',record='frames')],actions=[])
  for i in range(1,13):self.a['actions'].extend([dict(action='source-sprite-target',frame_input_row=i,source_generation=i,anchor=[1,2],point=[2,3],recent_source_check=dict(policy=POLICY,source_row=i,checked_row=i,checked_generation=i,age_frames=0,unchanged_fields=['history','position','camera','mode'],hit_feedback_used=False)),dict(action='pointer',logical=[2,3],world=str(i))])
  self.reader=SimpleNamespace(read_bytes=lambda _:b'frames')
 def assess(self):
  with patch.object(m,'decode',return_value=dict(rows=self.rows)):return m.verify_recent(self.reader,self.phase,self.a)
 def refuses(self):
  with self.assertRaises((ValueError,KeyError)):self.assess()
 def test_complete_actual_targets(self):self.assertEqual(len(self.assess()['targets']),12)
 def test_failed_acquisition_refuses(self):self.a['success']=False;self.refuses()
 def test_unverified_provider_refuses(self):self.a['actual_provider_verified']=False;self.refuses()
 def test_missing_target_refuses(self):del self.a['actions'][-2:];self.refuses()
 def test_changed_action_schedule_refuses(self):self.a['provider_adaptation']['recent_interaction_sources']['action_schedule_changed']=True;self.refuses()
 def test_hit_search_refuses(self):self.a['actions'][0]['recent_source_check']['hit_feedback_used']=True;self.refuses()
 def test_source_nonce_change_refuses(self):self.a['actions'][0]['recent_source_check'].update(checked_row=2,checked_generation=2,age_frames=1);self.rows[1]['frame']['nonce']='2';self.refuses()
 def test_changed_anchor_refuses(self):self.a['actions'][0]['anchor']=[2,3];self.refuses()
 def test_changed_pose_refuses(self):self.a['actions'][0]['recent_source_check'].update(checked_row=2,checked_generation=2,age_frames=1);self.rows[1]['history']=[2,3,4];self.refuses()
 def test_old_source_snapshot_refuses(self):self.a['actions'][0]['recent_source_check'].update(checked_row=5,checked_generation=5,age_frames=4);self.refuses()
 def test_delayed_physical_pointer_refuses(self):self.a['actions'][1]['world']='55';self.refuses()
 def test_wrong_physical_pixel_refuses(self):self.a['actions'][1]['logical']=[3,3];self.refuses()
 def test_missing_next_pointer_refuses(self):del self.a['actions'][1];self.refuses()
 def test_callback_instead_of_World_row_refuses(self):self.rows[0]['kind']=2;self.refuses()
if __name__=='__main__':unittest.main()
