import copy,importlib.util,struct,sys,unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from broader_picking_uncovered_target import targets,bound_targets,choose,contains,opaque_rows,POLICY
from broader_picking_pose_target import targets as original,PosedRows
s=importlib.util.spec_from_file_location('exact_pose_fixture',Path(__file__).with_name('test-picking-pose-target.py'));prior=importlib.util.module_from_spec(s);s.loader.exec_module(prior)
class Controls(unittest.TestCase):
 def setUp(self):self.args=prior.case();self.args[3]['extent']=[800,600]
 def extra(self,anchor=(100,201),change=None):
  f,blob=self.args[0][0];f=list(f);f[1]=2;f[8:10]=anchor;blob=bytearray(blob)
  if change:change(blob)
  self.args[0].append((tuple(f),bytes(blob)))
 def test_clear_body_keeps_original_pixel(self):
  body,cell=targets(*self.args);old,foot=original(*self.args);self.assertEqual(body['point'],old['point']);self.assertEqual(cell,foot);self.assertEqual(body['target_policy'],POLICY)
 def test_covered_center_chooses_nearest_clear_opaque(self):
  self.extra();body,_=targets(*self.args);self.assertEqual(body['point'],[106,203]);self.assertFalse(body['hit_feedback_used']);self.assertFalse(body['original_hit_order_assumed'])
 def test_other_body_below_or_above_still_excluded(self):
  self.extra();a=targets(*self.args)[0];self.args[0].reverse();self.assertEqual(targets(*self.args)[0],a)
 def test_every_pixel_covered_refuses(self):
  self.extra(change=lambda b:struct.pack_into('<I',b,16,(-2)%2**32))
  with self.assertRaisesRegex(ValueError,'outside other'):targets(*self.args)
 def test_duplicate_player_emissions_allowed(self):
  self.args[0].append(self.args[0][0]);self.assertEqual(targets(*self.args)[0]['excluded_other_source_body_masks'],0)
 def test_duplicate_other_masks_counted_once(self):
  self.extra();self.args[0].append(self.args[0][-1]);self.assertEqual(targets(*self.args)[0]['excluded_other_source_body_masks'],1)
 def test_another_actor_same_body_identity_refuses(self):
  d=list(self.args[3]['draws'][0]);d[0]=1;self.args[3]['draws'].append(tuple(d))
  with self.assertRaisesRegex(ValueError,'shares tracked'):targets(*self.args)
 def test_hit_metadata_does_not_influence_selection(self):
  self.extra();before=targets(*self.args);self.args[3]['invented_hit_feedback']=dict(slot=99,point=[1,2]);self.assertEqual(targets(*self.args),before)
 def test_outside_viewport_refuses(self):
  self.args[3]['extent']=[10,10]
  with self.assertRaisesRegex(ValueError,'outside other'):targets(*self.args)
 def test_plain_records_bound_refused(self):
  with self.assertRaises(ValueError):bound_targets(*self.args[:3])
 def test_bound_same_generation(self):
  self.assertEqual(bound_targets(PosedRows(self.args[0],self.args[3]),*self.args[1:3]),targets(*self.args))
 def test_actor_pose_identity_still_required(self):
  self.args[2][0]=8
  with self.assertRaises(ValueError):targets(*self.args)
 def test_source_RLE_opaque_membership_half_open(self):
  shape=opaque_rows(self.args[0][0][1]);self.assertTrue(contains(shape,[100,200],[105,203]));self.assertFalse(contains(shape,[100,200],[104,203]));self.assertFalse(contains(shape,[100,200],[109,203]))
 def test_malformed_RLE_refuses(self):
  b=bytearray(self.args[0][0][1]);b[64]=255
  with self.assertRaises(ValueError):opaque_rows(bytes(b))
 def test_large_origin_refuses(self):
  b=bytearray(self.args[0][0][1]);struct.pack_into('<I',b,12,2**31)
  with self.assertRaisesRegex(ValueError,'no-wrap'):opaque_rows(bytes(b))
 def test_large_anchor_refuses(self):
  self.extra(anchor=(2**31,200))
  with self.assertRaisesRegex(ValueError,'no-wrap'):targets(*self.args)
if __name__=='__main__':unittest.main()
