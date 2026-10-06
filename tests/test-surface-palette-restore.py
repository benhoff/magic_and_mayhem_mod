#!/usr/bin/env python3
import importlib.util,unittest
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
s=importlib.util.spec_from_file_location('palette_restore',ROOT/'tools/check-surface-palette-restore.py');r=importlib.util.module_from_spec(s);s.loader.exec_module(r)
class PaletteRestore(unittest.TestCase):
 def rejected(self,change):
  capture,rows=r.load();change(rows)
  with self.assertRaises(ValueError):r.validate(capture,rows)
 def test_capture(self):self.assertTrue(r.check()['success'])
 def test_completion(self):self.rejected(lambda rows:rows.pop())
 def test_lost_getpalette(self):self.rejected(lambda rows:next(x for x in rows if x[:3]==[21,1,1]).__setitem__(3,0))
 def test_retained_identity(self):self.rejected(lambda rows:next(x for x in rows if x[:3]==[21,4,1]).__setitem__(4,0))
 def test_retained_entry(self):self.rejected(lambda rows:next(x for x in rows if x[:3]==[22,4,1] and x[4]==37).__setitem__(5,0))
 def test_shared_update_isolation(self):self.rejected(lambda rows:next(x for x in rows if x[:3]==[22,2,2] and x[4]==37).__setitem__(5,r.color(37,0,1)))
 def test_reload_indices(self):self.rejected(lambda rows:next(x for x in rows if x[:3]==[5,6,1]).__setitem__(5,0))
 def test_palette_rebind(self):self.rejected(lambda rows:next(x for x in rows if x[:3]==[24,9,1]).__setitem__(4,0))
 def test_unspecified_restored_bytes(self):
  capture,rows=r.load();values=[x for x in rows if x[:3]==[5,4,1]];values[0][5]^=1;h=2166136261
  for x in values:h=((h^x[5])*16777619)&0xffffffff
  next(x for x in rows if x[:3]==[6,4,1])[3]=h;self.assertTrue(r.validate(capture,rows))
if __name__=='__main__':unittest.main()
