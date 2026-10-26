import importlib.util
from pathlib import Path
import struct,unittest
s=importlib.util.spec_from_file_location('closure',Path(__file__).resolve().parents[1]/'tools/drawing_session_closure.py');m=importlib.util.module_from_spec(s);s.loader.exec_module(m)
def raw():
 h=[0]*16;h[:2]=struct.unpack('<2I',b'MNMPSC01');h[2:]=[1,64,0x40209ca7,128,1,23,501,12,3,64,0,0,120,0]
 return struct.pack('<16I',*h)+struct.pack('<8I',4,7,23,0,0x6ba01,0x200,6,0x1234)
class Cases(unittest.TestCase):
 def test_no_admission(self):
  d=m.decode_refusal(raw());self.assertFalse(d['source_closure_accepted']);self.assertEqual(d['frames'][0]['tag'],7)
 def test_extent(self):
  for b in [b'',raw()[:-1],raw()+b'\0',raw()+bytes(32)]:
   with self.assertRaises(ValueError):m.decode_refusal(b)
 def test_header(self):
  for at,v in [(2,2),(3,63),(4,1),(5,129),(6,2),(7,0),(13,2),(14,121),(15,2)]:
   b=bytearray(raw());struct.pack_into('<I',b,at*4,v)
   with self.assertRaises(ValueError):m.decode_refusal(b)
 def test_frame(self):
  for at,v in [(0,0),(0,129),(1,61),(2,0),(3,2),(4,0x1c0000),(5,0)]:
   b=bytearray(raw());struct.pack_into('<I',b,64+at*4,v)
   with self.assertRaises(ValueError):m.decode_refusal(b)
 def test_duplicate_slot(self):
  b=bytearray(raw());struct.pack_into('<I',b,24,2);b+=b[64:]
  with self.assertRaises(ValueError):m.decode_refusal(b)
 def test_reserved_frame_is_retained(self):
  b=bytearray(raw());struct.pack_into('<I',b,64+20,0xffffffff);d=m.decode_refusal(b);self.assertTrue(d['frames'][0]['reserved']);self.assertFalse(d['source_closure_accepted'])
if __name__=='__main__':unittest.main()
