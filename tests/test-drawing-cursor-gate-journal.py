import importlib.util
from pathlib import Path
import struct
import unittest
ROOT=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('gate',ROOT/'tools/drawing_cursor_gate_journal.py');gate=importlib.util.module_from_spec(spec);spec.loader.exec_module(gate)
def sample():
 h=[0]*16;h[:2]=struct.unpack('<2I',b'MNMDGT01');h[2:9]=[1,192,0x40209ca7,8192,1,0,3]
 r=[0]*48;r[0:3]=[1,9,123];return struct.pack('<16I',*h)+struct.pack('<48I',*r)
class JournalGuards(unittest.TestCase):
 def test_partial_snapshot_never_complete(self):self.assertFalse(gate.decode(sample())['complete'])
 def test_extent_refuses(self):
  for payload in [sample()[:-1],sample()+b'\0']:
   with self.assertRaises(ValueError):gate.decode(payload)
 def test_reserved_and_identity_binding(self):
  data=bytearray(sample());struct.pack_into('<I',data,64+32*4,1)
  with self.assertRaises(ValueError):gate.decode(data)
 def test_point_token_mismatch(self):
  data=bytearray(sample());struct.pack_into('<3I',data,64+16*4,1,128,0x40209ca7);struct.pack_into('<I',data,64+21*4,9)
  with self.assertRaises(ValueError):gate.decode(data)
 def test_loss_stays_visible(self):
  data=bytearray(sample());struct.pack_into('<I',data,7*4,1);r=gate.decode(data);self.assertEqual(r['dropped'],1);self.assertFalse(r['complete'])
if __name__=='__main__':unittest.main()
