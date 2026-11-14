import json,struct,sys,unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from broader_history_capture_contract import POLICY,header
class Headers(unittest.TestCase):
 def setUp(self):self.row={k:POLICY[k]for k in('version','policy','row_limit','raw_limit_bytes','stored_limit_bytes','queue_limit_bytes','queue_limit_rows','compression_workers')};self.row.update(index=0,kind='open',role='commands',session='exact')
 def raw(self):
  body=json.dumps(self.row).encode();return b'MNMACE01'+struct.pack('<II',len(body),0)+body
 def test_current_exact_header_accepted(self):self.assertEqual(header(self.raw(),'exact'),self.row)
 def test_legacy_or_changed_native_limits_refuse(self):
  for k in POLICY:
   if k not in self.row:continue
   old=self.row[k];self.row[k]='changed'
   with self.subTest(k=k):
    with self.assertRaises(ValueError):header(self.raw(),'exact')
   self.row[k]=old
 def test_foreign_session_refuses(self):
  with self.assertRaises(ValueError):header(self.raw(),'foreign')
 def test_truncated_header_refuses(self):
  for raw in(b'',self.raw()[:12],self.raw()[:-1]):
   with self.assertRaises(ValueError):header(raw,'exact')
if __name__=='__main__':unittest.main()
