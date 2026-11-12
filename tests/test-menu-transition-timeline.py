#!/usr/bin/env python3
import os,sqlite3,struct,subprocess,sys,tempfile,unittest
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools'))
from menu_transition_timeline import Recorder,host_rows,callback_rows,wire_words,SIZE
FIX=Path(os.environ['MNM_TIMELINE_TEST_INPUTS'])
def callbacks():
 rows=[]
 for n,e in [(1,2),(2,3)]:
  r=[0]*16;r[0]=n;r[1]=e;r[2]=123;r[3]=3;r[9]=2;rows.append(struct.pack('<16I',*r))
 return b'MNMMENU1'+struct.pack('<II',1,64)+b''.join(rows)
def wire():
 b=bytearray(73728);b[:16]=b'MNMMCMD5'+struct.pack('<II',5,73728);struct.pack_into('<6I',b,16,2,1,3,1,1,1);struct.pack_into('<8I',b,128,4,2,22,1,1,1,123,0);return b
class TimelineTests(unittest.TestCase):
 def test_actual_cpp_protocol_and_control(self):
  for name,control in [('normal-host.bin',False),('control-host.bin',True)]:
   h,r=host_rows((FIX/name).read_bytes());events=[x[1] for x in r];self.assertEqual(h[8],1);self.assertEqual(bool(h[11]),control);self.assertEqual(events.count('request_published'),1);self.assertEqual('destination_ready' in events,not control);self.assertEqual('read_suppressed' in events,control)
 def test_diagnostic_gate(self):self.assertFalse((FIX/'nominal-forbidden.bin').exists())
 def test_overflow_cannot_be_complete_proof(self):
  with self.assertRaises(ValueError):host_rows((FIX/'overflow-host.bin').read_bytes())
 def test_bad_header_and_record(self):
  b=bytearray((FIX/'normal-host.bin').read_bytes())
  for at,value in [(8,2),(20,513),(36,9),(48,1),(64,99),(124,1)]:
   c=b.copy();struct.pack_into('<I',c,at,value)
   with self.assertRaises(ValueError):host_rows(c)
 def test_partial_callbacks_and_unknown_time(self):
  data=callbacks();self.assertEqual(len(callback_rows(data)),2);self.assertEqual(len(callback_rows(data[:-1])),1)
  with self.assertRaises(ValueError):callback_rows(data[:-1],True)
 def test_stable_wire(self):
  data=wire();self.assertEqual(wire_words(data[:160])[10],1);struct.pack_into('<I',data,128,5);self.assertIsNone(wire_words(data[:160]))
 def test_recorder_roundtrip_and_replacement_refusal(self):
  with tempfile.TemporaryDirectory() as d:
   p=Path(d);(p/'menu-host-timeline.bin').write_bytes((FIX/'control-host.bin').read_bytes());(p/'events.bin').write_bytes(callbacks());(p/'channel.bin').write_bytes(wire());r=Recorder(p/'timeline.sqlite3');r.observe(p,True);summary=r.summary();self.assertTrue(summary['original_callback_pair_retained']);self.assertFalse(summary['original_callback_execution_timestamps_available']);self.assertEqual(summary['original_callback_receipt_intervals'][0][2],0)
   # Replacing a generation cannot silently continue the timeline.
   (p/'replacement').write_bytes(wire());(p/'replacement').replace(p/'channel.bin')
   with self.assertRaises(ValueError):r.observe(p)
   r.close()
   with sqlite3.connect(p/'timeline.sqlite3') as db:self.assertGreater(db.execute('SELECT COUNT(*) FROM host_events').fetchone()[0],0)
 def test_committed_rows_survive_abrupt_producer_exit(self):
  with tempfile.TemporaryDirectory() as d:
   p=Path(d)/'crash.sqlite3';code="import os,sys;from pathlib import Path;sys.path.insert(0,sys.argv[1]);from menu_transition_timeline import Recorder,host_rows;r=Recorder(sys.argv[2]);row=host_rows(Path(sys.argv[3]).read_bytes())[1][0];r.db.execute('INSERT INTO host_events VALUES('+','.join('?'*15)+')',row);os._exit(17)"
   c=subprocess.run([sys.executable,'-c',code,str(ROOT/'tools'),str(p),str(FIX/'normal-host.bin')]);self.assertEqual(c.returncode,17)
   with sqlite3.connect(p) as db:self.assertEqual(db.execute('SELECT COUNT(*) FROM host_events').fetchone()[0],1);self.assertEqual(db.execute('PRAGMA integrity_check').fetchone()[0],'ok')
if __name__=='__main__':unittest.main()
