from pathlib import Path
import struct
import subprocess
import sys
import unittest
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools'))
from drawing_source_cursor_gate import decode

def fixture(refused=False):
 h=[0]*16;h[:2]=struct.unpack('<2I',b'MNMDGA01');h[2:6]=[1,192,0x40209ca7,8192]
 rows=[]
 for ordinal,phase in [(1,1),(2,2)]:
  r=[0]*48;r[:7]=[ordinal,phase,42,123,0 if refused else 7,0 if refused or phase==1 else 9,3 if refused else 0];r[16:19]=[1,128,0x40209ca7];r[19]=1 if refused else phase;r[20]=3 if refused else 0;r[21]=8 if refused else 7;r[22]=123;r[23:25]=[1,2];r[29]=0 if refused else (3 if phase==1 else 0);rows.append(r)
 r=[0]*48;r[:3]=[3,4,1];rows.append(r)
 return struct.pack('<16I',*h)+b''.join(struct.pack('<48I',*r) for r in rows)
class Guards(unittest.TestCase):
 def test_bound_partial(self):
  r=decode(fixture());self.assertEqual(r['completed'][0]['source_lease'],9);self.assertFalse(r['complete_reference'])
 def test_failed_attempt_id_is_retained_without_admission(self):
  r=decode(fixture(refused=True));self.assertEqual(r['completed'],[]);self.assertEqual(r['refused'][0]['token'],0);self.assertEqual(r['refused'][0]['attempted_token'],8);self.assertFalse(r['complete_reference'])
 def test_post_snapshot_refusal_has_no_reservation(self):
  data=bytearray(fixture(refused=True))
  for start in [64,64+192]:
   for word,value in [(6,17),(20,17),(21,0)]:struct.pack_into('<I',data,start+word*4,value)
  r=decode(data);self.assertEqual(r['completed'],[]);self.assertEqual(r['refused'][0]['attempted_token'],0);self.assertEqual(r['refused'][0]['reason'],17);self.assertFalse(r['complete_reference'])
 def test_attempt_identity_phase_reason_and_owned_refusal_refuse(self):
  for index,value in [(64+192+21*4,9),(64+192+23*4,3),(64+192+19*4,2),(64+192+20*4,4),(64+29*4,1)]:
   data=bytearray(fixture(refused=True));struct.pack_into('<I',data,index,value)
   with self.assertRaises(ValueError):decode(data)
 def test_completed_api_identity_cannot_be_reused(self):
  data=fixture();duplicate=bytearray(data[64:64+2*192]);struct.pack_into('<I',duplicate,0,3);struct.pack_into('<I',duplicate,192,4);terminal=bytearray(data[-192:]);struct.pack_into('<I',terminal,0,5)
  with self.assertRaises(ValueError):decode(data[:64+2*192]+duplicate+terminal)
 def test_owner_token_pending_and_release_fault_refuse(self):
  for index,value in [(64+192+3*4,124),(64+192+4*4,8),(64+192+29*4,1),(64+192+1*4,3)]:
   data=bytearray(fixture());struct.pack_into('<I',data,index,value)
   with self.assertRaises(ValueError):decode(data)
  with self.assertRaises(ValueError):decode(fixture()[:-192])
 def test_reserved_extent_refuse(self):
  for index in [6*4,64+7*4,64+32*4]:
   data=bytearray(fixture());struct.pack_into('<I',data,index,1)
   with self.assertRaises(ValueError):decode(data)
  with self.assertRaises(ValueError):decode(fixture()+b'\0')
 def test_frontend_control_and_conflicting_hook_refuse(self):
  base=[sys.executable,str(ROOT/'tools/capture-drawing-interactive.py'),'--source-game','/missing','--prefix-template','/missing','--report','/tmp/mnm-gate-refusal-test-unused.json','--cursor-source-gate']
  for extra in [[],['--control'],['--cursor-mutex']]:
   result=subprocess.run(base+extra,capture_output=True,text=True);self.assertNotEqual(result.returncode,0);self.assertIn('Cursor source gate requires',result.stderr)
if __name__=='__main__':unittest.main()
