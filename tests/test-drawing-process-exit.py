from pathlib import Path
import struct,sys,unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from drawing_process_exit import decode
def sample(code=0):
 h=[0x444d4e4d,0x31305850,1,64,44,1000]+[0]*10
 a=[1,1,44,88,258,1,259,10,0]+[0]*7
 b=[2,2,44,88,0,1,code,20,0]+[0]*7
 return struct.pack('<48I',*(h+a+b))
class ProcessExit(unittest.TestCase):
 def test_actual_exit_code_and_still_active_value_after_signal(self):
  for code in [0,37,259]:self.assertEqual(decode(sample(code))['exit_code'],code)
 def test_missing_exit_is_partial_only(self):
  with self.assertRaises(ValueError):decode(sample()[:128])
  self.assertFalse(decode(sample()[:128],complete=False)['complete'])
 def test_identity_wait_query_and_reserved_refuse(self):
  for index,value in [(2,2),(4,0),(6,1),(16+4,0),(16+6,0),(32+3,89),(32+4,258),(32+5,0),(32+8,5),(32+15,1)]:
   data=bytearray(sample());struct.pack_into('<I',data,index*4,value)
   with self.assertRaises(ValueError):decode(data)
if __name__=='__main__':unittest.main()
