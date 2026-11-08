#!/usr/bin/env python3
"""Versioned command history admits the observed long trace and retains hard caps."""
from pathlib import Path
import sys,json,struct,unittest,copy
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from rendering_capture import capture,COMMAND_HISTORY_POLICY,COMMAND_HISTORY_ROWS,COMMAND_HISTORY_RAW

def wire(count=2,changes=None,complete=True):
 first=dict(kind='open',role='commands',version=4,session='command-controls',policy=COMMAND_HISTORY_POLICY,
  raw_limit_bytes=COMMAND_HISTORY_RAW,row_limit=COMMAND_HISTORY_ROWS,stored_limit_bytes=1024**3,queue_limit_bytes=256*1024**2,queue_limit_rows=4096)
 first.update(changes or {});out=bytearray(b'MNMACE01')
 def add(meta,index):
  row=json.dumps(dict(meta,index=index,host_us=str(index+1)),separators=(',',':')).encode();out.extend(struct.pack('<II',len(row),0));out.extend(row)
 add(first,0)
 for i in range(count):add(dict(kind='fragment',ordinal=i),i+1)
 add(dict(kind='close',complete=complete,error='' if complete else 'Acceptance capture raw/row budget exceeded',prior_rows=count+1,raw_bytes='0'),count+1)
 return bytes(out)
class CommandCapture(unittest.TestCase):
 def test_complete_history_past_old_row_limit(self):
  rows=capture(wire(17000));self.assertEqual(len(rows),17002);self.assertEqual(rows[-2][0]['ordinal'],16999)
 def test_old_version_cannot_borrow_larger_declared_limit(self):
  with self.assertRaisesRegex(ValueError,'row budget'):capture(wire(17000,dict(version=1)))
 def test_explicit_new_row_cap_and_failure_remain_failures(self):
  with self.assertRaisesRegex(ValueError,'row budget'):capture(wire(COMMAND_HISTORY_ROWS-1))
  with self.assertRaisesRegex(ValueError,'incomplete or truncated'):capture(wire(3,complete=False))
 def test_malformed_policy_role_or_budget_is_refused(self):
  for key,value in [('role','viewport'),('policy','other'),('row_limit',True),('row_limit',65537),('raw_limit_bytes',COMMAND_HISTORY_RAW+1),('stored_limit_bytes',2*1024**3),('queue_limit_rows',True),('queue_limit_rows',4097),('queue_limit_bytes',256*1024**2+1)]:
   with self.subTest(key=key,value=value),self.assertRaisesRegex(ValueError,'command history capture policy'):capture(wire(2,{key:value}))
if __name__=='__main__':unittest.main()
