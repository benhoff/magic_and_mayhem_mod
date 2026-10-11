#!/usr/bin/env python3
import copy
import struct
import sys
from pathlib import Path
import unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from drawing_physical import admit_witness,sha

def fixture():
 r=[0]*24
 for index,value in [(0,1),(1,1),(4,12),(5,1),(6,7),(8,1),(12,2),(13,1),(14,16),(15,0xf800),(16,0x7e0),(17,31),(19,1),(22,9)]:r[index]=value
 request=struct.pack('<24I',*r);physical=bytes([0,0,0,255,255,0,0,255]);oracle=struct.pack('<2H',0,0xf800)
 meta=dict(ordinal=1,attempt=1,width=2,height=1,source_sequence=12,queue=1,canvas=7,thread=9,action_record=0,stable_double_capture=True,request_sha256=sha(request),sha256=sha(physical))
 return request,b'MNMPACK1'+struct.pack('<II',1,1),physical,oracle,meta

class Guards(unittest.TestCase):
 def test_full_original_physical_witness(self):self.assertEqual(admit_witness(*fixture())['pixels'],2)
 def test_ack_has_exact_generation_and_attempt(self):
  f=list(fixture())
  for ack in (b'',b'MNMPACK1'+struct.pack('<II',2,1),b'MNMPACK1'+struct.pack('<II',1,2),f[1]+b'junk'):
   f[1]=ack
   with self.subTest(ack=ack),self.assertRaises(ValueError):admit_witness(*f)
 def test_failed_offscreen_or_unknown_format_refuses(self):
  for index,value in [(0,0),(1,3),(2,0x80004005),(8,0),(14,32),(19,0),(22,0)]:
   f=list(fixture());r=list(struct.unpack('<24I',f[0]));r[index]=value;f[0]=struct.pack('<24I',*r)
   with self.subTest(index=index),self.assertRaises(ValueError):admit_witness(*f)
 def test_typed_source_identity_and_stability(self):
  for key in ('ordinal','attempt','width','height','source_sequence','queue','canvas','thread','action_record'):
   f=list(fixture());f[4]=copy.deepcopy(f[4]);f[4][key]=True
   with self.subTest(key=key),self.assertRaises(ValueError):admit_witness(*f)
  f=list(fixture());f[4]['stable_double_capture']=1
  with self.assertRaises(ValueError):admit_witness(*f)
 def test_partial_pixels_hash_changes_and_presented_mismatch_refuse(self):
  for target in (2,3):
   f=list(fixture());f[target]=f[target][:-1]
   with self.subTest(target=target),self.assertRaises(ValueError):admit_witness(*f)
  f=list(fixture());f[2]=b'\x01'+f[2][1:];f[4]['sha256']=sha(f[2])
  with self.assertRaises(ValueError):admit_witness(*f)
  f=list(fixture());f[4]['request_sha256']='a'*64
  with self.assertRaises(ValueError):admit_witness(*f)

if __name__=='__main__':unittest.main()
