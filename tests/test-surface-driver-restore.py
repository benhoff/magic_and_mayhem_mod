#!/usr/bin/env python3
import copy,hashlib,importlib.util,json,struct,unittest
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
s=importlib.util.spec_from_file_location('restore',ROOT/'tools/check-surface-driver-restore.py');r=importlib.util.module_from_spec(s);s.loader.exec_module(r)
class DriverRestore(unittest.TestCase):
 def capture(self,bpp=32):return json.loads((ROOT/f'research/runtime/surface-driver-restore-{bpp}-capture-20261006.json').read_text())
 def fingerprint(self,c):c['outputs_sha256']=hashlib.sha256(b''.join(struct.pack('<6I',*x) for x in c['rows'])).hexdigest()
 def rejected(self,change):
  c=self.capture();change(c);self.fingerprint(c)
  with self.assertRaises(ValueError):r.validate(c,32)
 def test_captures(self):self.assertTrue(r.check()['success'])
 def test_completion_required(self):self.rejected(lambda c:c['rows'].pop())
 def test_lost_hresult(self):self.rejected(lambda c:next(x for x in c['rows'] if x[:3]==[2,1,1]).__setitem__(3,0))
 def test_restore_order(self):self.rejected(lambda c:c['rows'].reverse())
 def test_key_retention(self):self.rejected(lambda c:next(x for x in c['rows'] if x[:3]==[3,4,1]).__setitem__(4,0))
 def test_defined_pixels(self):self.rejected(lambda c:next(x for x in c['rows'] if x[:3]==[5,6,1]).__setitem__(5,0))
 def test_fingerprint(self):
  c=self.capture();c['outputs_sha256']='0'*64
  with self.assertRaises(ValueError):r.validate(c,32)
 def test_unspecified_restored_bytes(self):
  c=self.capture();values=[x for x in c['rows'] if x[:3]==[5,4,1]];values[0][5]^=1;h=2166136261
  for x in values:h=((h^x[5])*16777619)&0xffffffff
  next(x for x in c['rows'] if x[:3]==[6,4,1])[3]=h;self.fingerprint(c);self.assertGreater(r.validate(c,32),0)
if __name__=='__main__':unittest.main()
