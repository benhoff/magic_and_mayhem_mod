#!/usr/bin/env python3
import sys
from pathlib import Path
import unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from drawing_delivery_timing import analyze_timing,nanoseconds
def peer(a=10,b=11,c=20,d=21):return dict(success=True,frames=[dict(ordinal=1,entry=77,x11_target='24',root_x=-4,root_y=7,clock_domain='linux_CLOCK_MONOTONIC',first_capture=dict(begin_ns=str(a),end_ns=str(b)),second_capture=dict(begin_ns=str(c),end_ns=str(d)),settling_samples=[])])
def delivery(a=30,b=40):return dict(success=True,calls=[dict(method=11,pid=3,entry=1,enter_ns=a,return_ns=b)])
class Timing(unittest.TestCase):
 def test_delivery_identity_clock_and_pair_denominator_are_strict(self):
  for field,value in [('pid',True),('entry',0),('enter_ns',1.5),('return_ns',True),('return_ns',1<<64),('return_ns',29)]:
   d=delivery();d['calls'][0][field]=value
   with self.subTest(field=field,value=value),self.assertRaises(ValueError):analyze_timing(peer(),d)
  d=delivery();d['calls'].append(dict(d['calls'][0]))
  with self.assertRaises(ValueError):analyze_timing(peer(),d)
  p=peer();p['frames'].append(dict(p['frames'][0],ordinal=2,first_capture=dict(begin_ns='50',end_ns='51'),second_capture=dict(begin_ns='60',end_ns='61')))
  with self.assertRaises(ValueError):analyze_timing(p,delivery())
 def test_before_first_swap(self):
  r=analyze_timing(peer(),delivery());self.assertEqual(r['physical_pairs_before_first_egl'],1);self.assertFalse(r['source_submission_identity_bound'])
  self.assertEqual(r['frames'][0]['root_x'],-4);self.assertEqual(r['frames'][0]['x11_target'],'24')
 def test_pending_calls_and_untyped_physical_placement_refuse(self):
  d=delivery();d['pending']=[dict(pid=3,entry=2)]
  with self.assertRaises(ValueError):analyze_timing(peer(),d)
  for key,value in [('root_x',True),('root_y',2**31),('x11_target','0'),('x11_target','01')]:
   p=peer();p['frames'][0][key]=value
   with self.assertRaises(ValueError):analyze_timing(p,delivery())
 def test_overlap_and_after(self):
  self.assertEqual(analyze_timing(peer(),delivery(15,30))['pairs_overlapping_egl'],1)
  self.assertTrue(analyze_timing(peer(50,51,60,61),delivery())['frames'][0]['after_first_egl_return'])
 def test_exact_uint64(self):
  self.assertEqual(nanoseconds('18446744073709551615'),(1<<64)-1)
  for value in ('18446744073709551616','01','-1','1e3',10):
   with self.assertRaises(ValueError):nanoseconds(value)
 def test_bad_clock_order_identity(self):
  for p in (peer(12,11),peer(10,25,20,21)):
   with self.assertRaises(ValueError):analyze_timing(p,delivery())
  p=peer();p['frames'][0]['clock_domain']='Win ticks'
  with self.assertRaises(ValueError):analyze_timing(p,delivery())
 def test_settling_before_retained_pair(self):
  p=peer(30,31,40,41);p['frames'][0]['settling_samples']=[dict(ordinal=1,sample=1,capture=dict(begin_ns='10',end_ns='11'))];self.assertEqual(analyze_timing(p,delivery())['frames'][0]['settling_samples'],1)
  p['frames'][0]['settling_samples'][0]['capture']['end_ns']='35'
  with self.assertRaises(ValueError):analyze_timing(p,delivery())
 def test_latest_completed_return_with_overlapping_calls(self):
  d=delivery(10,40);d['calls'].append(dict(method=11,pid=3,entry=2,enter_ns=20,return_ns=30))
  r=analyze_timing(peer(50,51,60,61),d);self.assertEqual(r['frames'][0]['last_completed_egl_before_read'],dict(pid=3,entry=1))
if __name__=='__main__':unittest.main()
