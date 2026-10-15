#!/usr/bin/env python3
import struct
import sys
from pathlib import Path
import unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from drawing_egl_physical import decode_request
def request():
 r=[0]*32;r[2:18]=[1,128,1,3,4,5,11,1,2,1,1,77,800,600,10,0];return b'MNMEGL01'+struct.pack('<30I',*r[2:])
class Physical(unittest.TestCase):
 def test_identity(self):self.assertEqual(decode_request(request())[4:8],(1,3,4,5))
 def test_envelope(self):
  for raw in (request()[:-1],b'BADMAGIC'+request()[8:],request()+b'0'):
   with self.assertRaises(ValueError):decode_request(raw)
 def test_bad_id_extent_and_reserved(self):
  for field,value in ((2,2),(3,96),(4,0),(4,1025),(5,0),(6,0),(7,0),(8,10),(9,0),(10,257),(11,17),(12,0),(14,801),(18,1),(31,1)):
   raw=bytearray(request());struct.pack_into('<I',raw,field*4,value)
   with self.assertRaises(ValueError):decode_request(raw)
if __name__=='__main__':unittest.main()
