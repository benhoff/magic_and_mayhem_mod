import copy,sys,unittest
from pathlib import Path
from unittest.mock import patch
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from broader_picking_pan_assessment import pan_receipts
from broader_picking_pan_ack import POLICY
class Controls(unittest.TestCase):
 def setUp(self):
  before=dict(kind=1,ordinal=1,thread=7,frame=dict(generation=5,nonce='123'),camera=[0,23,55,6]);after=dict(kind=1,ordinal=2,thread=7,frame=dict(generation=6,nonce='123'),camera=[0,22,54,6]);self.rows=[before,after]
  names=[p+'-'+m+'-pan'for p in ('high-initial','low','high-returned')for m in ('sharp','integer')]
  attempts=[dict(action=n,before=before,acknowledged_after=after,pressed_monotonic=0,acknowledged_monotonic=.35,released_monotonic=.36,deadline_monotonic=1.02,minimum_hold_seconds=.22,hold_plus_settle_budget_seconds=1.02,physical_release_accepted=True)for n in names]
  policy=dict(policy=POLICY,physical_key_identities_and_order_unchanged=True,hold_release_policy_changed=True,minimum_hold_seconds=.22,maximum_hold_plus_settle_seconds=1.02,settlement_uses_remaining_existing_budget=True,source_generation_nonce_thread_required=True,changed_camera_same_view_required=True,original_five_World_thirty_second_progress_wait_unchanged=True,original_pan_analysis_predicate_unchanged=True,journey_instances=1,attempts=attempts)
  self.a=dict(provider_adaptation=dict(picking_source_pan_ack=policy),camera_actions=[dict(action=n,phase=p,world='11'if p=='settled'else'6')for n in names for p in ['begin','released','settled']]);self.p=policy
 def check(self):
  with patch('broader_picking_pan_assessment.artifact',return_value=b'controlled'),patch('broader_picking_pan_assessment.decode',return_value=dict(rows=self.rows)):return pan_receipts(None,self.a)
 def test_all_six_exact_acks_pass(self):self.assertTrue(self.check())
 def test_missing_pan_refuses(self):
  self.p['attempts'].pop()
  with self.assertRaises(ValueError):self.check()
 def test_changed_row_refuses(self):
  self.p['attempts'][0]['acknowledged_after']=dict(self.rows[1],camera=[0,21,53,6])
  with self.assertRaises(ValueError):self.check()
 def test_late_release_refuses(self):
  self.p['attempts'][0]['released_monotonic']=1.03
  with self.assertRaises(ValueError):self.check()
 def test_early_ack_refuses(self):
  self.p['attempts'][0]['acknowledged_monotonic']=.21
  with self.assertRaises(ValueError):self.check()
 def test_larger_deadline_refuses(self):
  self.p['attempts'][0]['deadline_monotonic']=1.2
  with self.assertRaises(ValueError):self.check()
 def test_release_refusal_refuses(self):
  self.p['attempts'][0]['physical_release_accepted']=False
  with self.assertRaises(ValueError):self.check()
 def test_missing_five_World_progress_refuses(self):
  self.a['camera_actions'][2]['world']='10'
  with self.assertRaises(ValueError):self.check()
if __name__=='__main__':unittest.main()
