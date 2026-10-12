#!/usr/bin/env python3
import copy
import json
from pathlib import Path
import struct
import sys
import tempfile
import unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from drawing_api_session import sha
from drawing_egl_physical import analyze_egl_physical
def fixture(root):
 rows=[]
 for phase,time in [(1,10),(2,12)]:
  r=[0]*24;r[:9]=[phase,1,11,3,4,time,0,phase==2,77];r[9:12]=[1,2,1];rows.append(struct.pack('<24I',*r))
 (root/'x11-delivery.bin').write_bytes(b'MNMX1101'+struct.pack('<14I',3,96,65536,*([0]*11))+b''.join(rows))
 r=[0]*32;r[2:18]=[1,128,1,3,4,1,11,1,2,1,1,77,800,600,17,0];request=b'MNMEGL01'+struct.pack('<30I',*r[2:]);(root/'egl-physical-request-000001.bin').write_bytes(request)
 (root/'egl-physical-ack-000001.bin').write_bytes(b'MNMEACK1'+struct.pack('<2I',1,1));(root/'egl-physical.done').write_bytes(b'MNMEGD01'+struct.pack('<6I',1,3,1,1,1,0))
 plane=b'\0\0\0\xff'*(800*600);(root/'egl-physical-oracle-000001.rgba').write_bytes(plane)
 frame=dict(ordinal=1,pid=3,thread=4,entry=1,method=11,display=1,surface=2,alias=1,width=800,height=600,clock_domain='linux_CLOCK_MONOTONIC',first_capture=dict(begin_ns='20',end_ns='21'),second_capture=dict(begin_ns='26',end_ns='27'),path='egl-physical-oracle-000001.rgba',sha256=sha(plane),request_sha256=sha(request),stable_double_capture=True,settling_samples=[])
 return dict(success=True,original_egl_caller_fenced=True,native_rendering=False,boundary='observed_post_egl_return_physical',all_global_presentation_routes=False,source_inputs_bound=False,all_physical_changes_captured=False,frames=[frame],transient_bytes=0,transient_byte_budget=1024*1024)
class Binding(unittest.TestCase):
 def setUp(self):self.tmp=tempfile.TemporaryDirectory();self.root=Path(self.tmp.name);self.peer=fixture(self.root)
 def tearDown(self):self.tmp.cleanup()
 def run_peer(self,peer=None):
  p=self.root/'peer.json';p.write_text(json.dumps(peer or self.peer));return analyze_egl_physical(self.root,p)
 def test_exact_full_plane(self):
  r=self.run_peer();self.assertEqual(r['pixels'],480000);self.assertFalse(r['source_inputs_bound']);self.assertFalse(r['scanout_established'])
 def test_changed_plane(self):
  p=self.root/'egl-physical-oracle-000001.rgba';data=bytearray(p.read_bytes());data[0]=1;p.write_bytes(data)
  with self.assertRaises(ValueError):self.run_peer()
 def test_changed_ack_and_extra_request(self):
  (self.root/'egl-physical-ack-000001.bin').write_bytes(b'MNMEACK1'+struct.pack('<2I',1,2))
  with self.assertRaises(ValueError):self.run_peer()
  (self.root/'egl-physical-ack-000001.bin').write_bytes(b'MNMEACK1'+struct.pack('<2I',1,1));(self.root/'egl-physical-request-000002.bin').write_bytes(b'0')
  with self.assertRaises(ValueError):self.run_peer()
 def test_scope_identity_and_time(self):
  for field,value in [('source_inputs_bound',True),('scanout_established',True)]:
   peer=copy.deepcopy(self.peer);peer[field]=value
   with self.assertRaises(ValueError):self.run_peer(peer)
  for field,value in [('thread',5),('surface',3),('clock_domain','Win ticks'),('first_capture',dict(begin_ns='15',end_ns='16'))]:
   peer=copy.deepcopy(self.peer);peer['frames'][0][field]=value
   with self.assertRaises(ValueError):self.run_peer(peer)
 def test_bad_close_marker(self):
  (self.root/'egl-physical.done').write_bytes(b'MNMEGD01'+struct.pack('<6I',1,3,2,1,1,0))
  with self.assertRaises(ValueError):self.run_peer()
if __name__=='__main__':unittest.main()
