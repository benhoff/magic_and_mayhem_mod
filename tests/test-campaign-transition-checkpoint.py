#!/usr/bin/env python3
"""Adversarial original transition-ready input checks; no original game."""
import copy,hashlib,importlib.util,struct,sys,unittest
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools'))
from campaign_checkpoint import HEADER,decode
from campaign_transition_checkpoint import retired_goal,transition_state,validate_transition_capture
spec=importlib.util.spec_from_file_location('checkpoint_fixtures',ROOT/'tests/test-campaign-checkpoint.py');fixtures=importlib.util.module_from_spec(spec);spec.loader.exec_module(fixtures)


def capture_fixture():
 flow,route,data,raw=fixtures.capture_fixture(True);seed=decode(data)[0];seed['screen']=13
 rows=[list(r) for r in struct.iter_unpack('<16I',data[16:])]
 for group in (1,2):
  at=group*4;hero=rows[at+1];hero[8]=117;hero[9:12]=[42,30,4];hero[15]=3*256
  dead=rows[at+3];rows[at+3]=dead[:5]+[0]*11
 goal=dict(sequence=1,slot=2,type=24,owner=1,active=0,health=0,x=42,y=26,z=4,behavior=8,action=14,target=0xffffffff,mana=0)
 observed=(1,1,99,300,2,24,1,0,0,42,26,4,8,14,0xffffffff,0)
 route.update(checkpoint_domain='transition-ready',retired_boss=goal)
 data=HEADER+b''.join(struct.pack('<16I',*r) for r in rows)
 return flow,route,data,raw,[observed],seed


class TransitionCheckpoint(unittest.TestCase):
 def test_actual_save_restore_and_retirement_are_input_only(self):
  q=validate_transition_capture(*capture_fixture());self.assertTrue(q['verified']);self.assertEqual(q['domain'],'transition-ready')
  self.assertEqual(q['wizard']['mana'],3*256);self.assertFalse(q['fresh_campaign_progression_accepted'])
 def test_live_wrong_role_or_invalid_retirement_pose_refuses(self):
  goal=capture_fixture()[1]['retired_boss']
  for change in (dict(active=1),dict(health=1),dict(type=10),dict(owner=0),dict(slot=True),dict(owner=True),dict(active=False),dict(x=80),dict(z=-1)):
   with self.subTest(change=change),self.assertRaises(ValueError):retired_goal(dict(goal,**change))
 def test_pose_must_bind_exact_original_observation(self):
  for change in (dict(x=43),dict(sequence=2),dict(mana=1)):
   args=list(capture_fixture());args[1]['retired_boss'].update(change)
   with self.subTest(change=change),self.assertRaises(ValueError):validate_transition_capture(*args)
 def test_new_original_save_and_reload_cannot_be_replaced_by_seed(self):
  args=list(capture_fixture());args[2]=args[2][:16+4*64]
  with self.assertRaises(ValueError):validate_transition_capture(*args)
 def test_original_save_bytes_and_seed_identity_remain_exact(self):
  args=list(capture_fixture());args[3]+=b'changed'
  with self.assertRaises(ValueError):validate_transition_capture(*args)
  args=list(capture_fixture());args[0]['states'][-1]['sha256']='0'*64
  with self.assertRaises(ValueError):validate_transition_capture(*args)
 def test_any_outcome_or_simulation_write_refuses_input_capture(self):
  for change in (dict(semantic_outcome_accepted=True),dict(simulation_writes=True),dict(checkpoint_ready=False),dict(checkpoint_domain='bridge')):
   args=list(capture_fixture());args[1].update(change)
   with self.subTest(change=change),self.assertRaises(ValueError):validate_transition_capture(*args)
 def test_seed_actor_fields_and_loaded_actor_fields_cannot_drift(self):
  args=list(capture_fixture());args[-1]['actors'][0]['health']-=1
  with self.assertRaises(ValueError):validate_transition_capture(*args)
  flow,route,data,raw,observed,seed=capture_fixture();rows=[list(r) for r in struct.iter_unpack('<16I',data[16:])];rows[9][8]-=1
  with self.assertRaises(ValueError):validate_transition_capture(flow,route,HEADER+b''.join(struct.pack('<16I',*r) for r in rows),raw,observed,seed)
 def test_dead_wizard_or_living_camulos_cannot_supply_transition_input(self):
  _,route,data,*_=capture_fixture();groups=decode(data);saved,loaded=groups[1:]
  for hp in (0,0xffffffff):
   a,b=copy.deepcopy(saved),copy.deepcopy(loaded);a['actors'][0]['health']=b['actors'][0]['health']=hp
   with self.assertRaises(ValueError):transition_state(a,b,route['retired_boss'])
  a,b=copy.deepcopy(saved),copy.deepcopy(loaded)
  for group in (a,b):group['actors'][2].update(type=24,owner=1,active=1,health=20)
  with self.assertRaises(ValueError):transition_state(a,b,route['retired_boss'])
 def test_far_or_invalid_saved_wizard_pose_refuses(self):
  _,route,data,*_=capture_fixture();saved,loaded=decode(data)[1:]
  for change in (dict(x=0,y=0),dict(x=80),dict(z=24)):
   a,b=copy.deepcopy(saved),copy.deepcopy(loaded)
   for group in (a,b):group['actors'][0].update(change)
   with self.subTest(change=change),self.assertRaises(ValueError):transition_state(a,b,route['retired_boss'])
 def test_nonrestored_ai_fields_remain_an_explicit_boundary(self):
  _,route,data,*_=capture_fixture();saved,loaded=decode(data)[1:];loaded['actors'][0]['behavior']=9
  _,storage,_=transition_state(saved,loaded,route['retired_boss']);self.assertTrue(storage['verified']);self.assertTrue(storage['other_observed_differences'])
 def test_exact_retained_input_fixtures_are_real_consumed_bytes(self):
  spec=importlib.util.spec_from_file_location('save_ready',ROOT/'tools/campaign-save-ready-input.py');driver=importlib.util.module_from_spec(spec);spec.loader.exec_module(driver)
  for name,h in driver.PINNED.items():self.assertEqual(hashlib.sha256((ROOT/'tools'/name).read_bytes()).hexdigest(),h)

if __name__=='__main__':unittest.main()
