import struct,sys,unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from drawing_egl_swap_completion import decode,decode_blit,decode_window,decode_surface,decode_viewport_policy
class Cases(unittest.TestCase):
 def valid(self):
  r=[0]*32;r[2:14]=[1,128,1,2,3,640,480,0x911a,1,1,1,0];r[14]=1;r[16]=2;r[18:22]=[0,0,640,480];return b'MNMESGC1'+struct.pack('<30I',*r[2:])
 def test_complete_metadata(self):
  d=decode(self.valid());self.assertEqual(d['viewport'],[0,0,640,480]);self.assertTrue(d['context_unchanged'])
 def test_missing_failed_or_incomplete_fence(self):
  for index,value in [(2,2),(3,127),(4,0),(7,2049),(9,0x911b),(10,0),(11,0),(12,0),(13,1),(31,1),(16,0)]:
   raw=bytearray(self.valid());struct.pack_into('<I',raw,index*4,value)
   with self.assertRaises(ValueError):decode(raw)
 def test_blit_metadata(self):
  raw=b'MNMEGBT1'+struct.pack('<22I',1,96,1,2,3,1,0,0,640,480,0,0,640,480,0x4000,0x2600,1,0,7,640,480,0x8ce0)
  d=decode_blit(raw);self.assertEqual(d['source_box'],[0,0,640,480]);self.assertEqual(d['read_width'],640)
  with self.assertRaises(ValueError):decode_blit(raw[:-1])
 def test_window_geometry_and_unknown_association(self):
  r=[0]*24;r[2:9]=[1,96,1,2,3,2,1];r[9:16]=[7,1,0,120,640,480,2];raw=b'MNMEW001'+struct.pack('<22I',*r[2:]);d=decode_window(raw);self.assertEqual(d['root_y'],120);self.assertTrue(d['known'])
  r[8:]=[0]*16;d=decode_window(b'MNMEW001'+struct.pack('<22I',*r[2:]));self.assertFalse(d['known'])
  with self.assertRaises(ValueError):decode_window(raw[:-1])
 def test_pre_swap_surface_extent(self):
  r=[0]*16;r[2:9]=[1,64,1,2,3,640,480];raw=b'MNMESBF1'+struct.pack('<14I',*r[2:]);self.assertEqual(decode_surface(raw)['width'],640)
  for index,value in [(2,2),(3,63),(4,0),(7,2049),(9,1)]:
   bad=bytearray(raw);struct.pack_into('<I',bad,index*4,value)
   with self.assertRaises(ValueError):decode_surface(bad)
 def test_policy_requires_applied_and_restored_state(self):
  r=[0]*16;r[2:]=[1,64,1,2,3,1,0,0,800,600,640,480,1,0];raw=b'MNMEVP01'+struct.pack('<14I',*r[2:]);self.assertEqual(decode_viewport_policy(raw)['temporary'],[0,0,640,480])
  for index,value in [(7,0),(7,16385),(12,2049),(14,0),(15,1)]:
   bad=bytearray(raw);struct.pack_into('<I',bad,index*4,value)
   with self.assertRaises(ValueError):decode_viewport_policy(bad)
 def test_wrong_extent(self):
  for raw in [self.valid()[:-1],self.valid()+b'\0',b'BADMAGIC'+self.valid()[8:]]:
   with self.assertRaises(ValueError):decode(raw)
if __name__=='__main__':unittest.main()
