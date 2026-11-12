#!/usr/bin/env python3
import copy,sys,unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from campaign_wizard_cast import probe_points,finishing_target,retry_finishing
class Cast(unittest.TestCase):
 def setUp(self):
  self.h=dict(slot=0,type=0,owner=0,active=1,health=120,mana=21*256,x=41,y=30,z=4);self.b=dict(slot=4,type=24,owner=1,active=1,health=30,x=42,y=26,z=4);self.a={0:self.h,4:self.b};self.first=dict(wizard_only=True,released=True,accepted_in_burst=4,target=dict(self.b));self.hit=(1,3,316,500,4,24,1,50,30,1,20,0,71,0x48ed24,0,0)
 def result(self,**kw):return finishing_target(self.h,self.a,self.first,[self.hit],600,316,**kw)
 def test_points_bounded_for_both_modes(self):
  for w,h in ((640,480),(800,600)):
   points=probe_points(self.h,self.b,(w,h));self.assertTrue(points);self.assertLessEqual(len(points),20);self.assertEqual(len(points),len(set(points)))
   self.assertTrue(all(48<=x<w-40 and 64<=y<h-90 for x,y in points))
 def test_other_actors_or_invalid_coordinates_do_not_supply_probes(self):
  for changed in (dict(owner=0),dict(type=10),dict(active=0),dict(health=0),dict(slot=True),dict(x=80)):
   self.assertEqual(probe_points(self.h,dict(self.b,**changed),(800,600)),[])
 def test_fresh_same_wizard_damage_allows_only_an_input_target(self):self.assertIs(self.result(),self.b)
 def test_first_volley_and_remaining_mana_required(self):
  for key,value in (('wizard_only',False),('released',False),('accepted_in_burst',0),('accepted_in_burst',True)):
   old=self.first[key];self.first[key]=value;self.assertIsNone(self.result());self.first[key]=old
  self.h['mana']=11*256;self.assertIsNone(self.result())
 def test_foreign_or_stale_damage_refuses(self):
  for index,value in ((2,317),(3,0xffff0000),(4,5),(5,10),(6,0),(8,50),(11,1),(12,14),(14,3),(15,1)):
   hit=list(self.hit);hit[index]=value
   self.assertIsNone(finishing_target(self.h,self.a,self.first,[tuple(hit)],600,316))
 def test_ambiguous_boss_and_reused_slot_refuse(self):
  self.a[5]=dict(self.b,slot=5);self.assertIsNone(self.result());del self.a[5];self.b['slot']=5;self.assertIsNone(self.result())
 def test_one_finishing_pause_only(self):self.assertIsNone(self.result(already=True))
 def test_health_distance_and_recorded_original_target_required(self):
  self.h['health']=39;self.assertIsNone(self.result());self.h['health']=120;self.b['y']=15;self.assertIsNone(self.result());self.b['y']=26;self.first['target']['slot']=5;self.assertIsNone(self.result())
 def test_retained640finishing_points_are_reached_early_without_click_credit(self):
  self.h.update(x=41,y=29);self.b.update(x=42,y=26)
  self.assertIn((480,154),probe_points(self.h,self.b,(640,480))[:10])
  self.assertEqual(probe_points(self.h,self.b,(640,480),[480,154])[0],(480,154))
  self.h.update(x=42,y=28);self.b.update(x=43,y=22)
  self.assertIn((480,120),probe_points(self.h,self.b,(640,480))[:8])
 def test_prior_proposals_remain_bounded_and_never_override_role_guards(self):
  expected=probe_points(self.h,self.b,(640,480))
  for invalid in ([True,100],[0,100],[640,100],[300,479],[300],None,'point'):
   self.assertEqual(probe_points(self.h,self.b,(640,480),invalid),expected)
  self.assertEqual(probe_points(self.h,dict(self.b,owner=0),(640,480),[480,154]),[])
  p=probe_points(self.h,self.b,(640,480),[480,154]);self.assertLessEqual(len(p),20);self.assertEqual(len(p),len(set(p)))
 def test_only_one_released_zero_cast_window_can_retry(self):
  r=dict(wizard_only=True,released=True,accepted_in_burst=0)
  self.assertTrue(retry_finishing(r,0))
  for count in (1,2,-1,False,None):self.assertFalse(retry_finishing(r,count))
  for change in (dict(released=False),dict(wizard_only=False),dict(accepted_in_burst=1),dict(accepted_in_burst=False)):
   self.assertFalse(retry_finishing(dict(r,**change),0))
 def test_finishing_damage_twenty_second_boundary_is_explicit(self):
  self.assertIs(finishing_target(self.h,self.a,self.first,[self.hit],20500,316),self.b)
  self.assertIsNone(finishing_target(self.h,self.a,self.first,[self.hit],20501,316))
if __name__=='__main__':unittest.main()
