#!/usr/bin/env python3
import struct
import sys
from pathlib import Path
import unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from drawing_x11_delivery import decode_delivery
def header():return b'MNMX1101'+struct.pack('<14I',1,96,65536,*([0]*11))
def row(phase=1,identity=1,time=10,result=0,pid=1):
 r=[0]*24;r[:9]=[phase,identity,1,pid,2,time,0,result,77];r[9]=100;return struct.pack('<24I',*r)
class Delivery(unittest.TestCase):
 def test_pair(self):
  d=decode_delivery(header()+row()+row(2,time=11,result=1));self.assertEqual(d['methods'],{'XPutImage':1});self.assertFalse(d['coherent_presented_frames']);self.assertEqual(d['calls'][0]['return_errno'],77)
 def test_pending_is_explicit(self):
  data=header()+row()+row(2,time=11)+row(identity=2,time=12)
  with self.assertRaises(ValueError):decode_delivery(data)
  self.assertEqual(decode_delivery(data,False)['pending'],[{'pid':1,'entry':2}])
 def test_interleaving_and_processes(self):
  data=header()+row()+row(pid=3)+row(2,pid=3)+row(2);self.assertEqual(decode_delivery(data)['processes'],2)
 def test_bad_envelopes(self):
  good=header()+row()+row(2)
  for data in (good[:63],good[:-1],b'BADMAGIC'+good[8:],good+row(2)):
   with self.assertRaises(ValueError):decode_delivery(data)
 def test_identity_time_and_reserved(self):
  for at,value in ((0,3),(4,0),(8,99),(12,0),(16,0),(28,1),(68,1)):
   item=bytearray(row());struct.pack_into('<I',item,at,value)
   with self.assertRaises(ValueError):decode_delivery(header()+item+row(2))
  with self.assertRaises(ValueError):decode_delivery(header()+row()+row(2,time=9))
  with self.assertRaises(ValueError):decode_delivery(header()+row(identity=2)+row(2,identity=2))
if __name__=='__main__':unittest.main()
