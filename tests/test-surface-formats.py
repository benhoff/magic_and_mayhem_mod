#!/usr/bin/env python3
"""Retained driver oracle rejects altered inputs, output branches and padding."""
import copy,importlib.util,unittest
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
s=importlib.util.spec_from_file_location('formats',ROOT/'tools/check-surface-formats.py');m=importlib.util.module_from_spec(s);s.loader.exec_module(m)
class Oracle(unittest.TestCase):
 @classmethod
 def setUpClass(cls):cls.report,cls.rows=m.load()
 def changed(self,predicate,mutate):
  rows=copy.deepcopy(self.rows);row=next(r for r in rows if predicate(r));mutate(row)
  with self.assertRaises(ValueError):m.validate(self.report,rows)
 def test_complete(self):self.assertEqual(m.validate(self.report,self.rows)['admitted_draws'],1400)
 def test_missing(self):
  with self.assertRaises(ValueError):m.validate(self.report,self.rows[:-1])
 def test_input(self):self.changed(lambda r:True,lambda r:r['input'].__setitem__('key_mode',99))
 def test_create(self):self.changed(lambda r:r['input']['layout']==2,lambda r:r['create'].__setitem__(0,0))
 def test_bind(self):self.changed(lambda r:r['input']['layout']==1,lambda r:r['set_descriptor'].__setitem__(0,1))
 def test_pointer(self):self.changed(lambda r:not any(r['create']),lambda r:r['source_before'].__setitem__('pointer_matches',0))
 def test_pitch(self):self.changed(lambda r:not any(r['create']),lambda r:r['source_before']['descriptor'].__setitem__(4,7))
 def test_native_key(self):self.changed(lambda r:r['input']['format']==4 and r['input']['kind']==2 and r['input']['key_mode']==3 and r['input']['clip']==0 and r['input']['layout']==0,lambda r:r['destination_after']['pixels'].__setitem__(0,r['source_before']['pixels'][0]))
 def test_fill_mask(self):self.changed(lambda r:r['input']['format']==1 and r['input']['kind']==0 and r['input']['clip']==0 and r['input']['layout']==0,lambda r:r['destination_after']['pixels'].__setitem__(0,0xf197))
 def test_key_output(self):self.changed(lambda r:r['input']['key_mode']==2 and not any(r['create']),lambda r:r['key_range'].__setitem__(0,0))
 def test_fast_clip(self):self.changed(lambda r:r['input']['kind']==3 and r['input']['clip']==1 and r['input']['layout']==0,lambda r:r.__setitem__('hresult',0))
 def test_guard(self):self.changed(lambda r:r['input']['layout']==1 and not any(r['create']),lambda r:r.__setitem__('destination_storage','00'+r['destination_storage'][2:]))
 def test_source(self):self.changed(lambda r:r['input']['layout']==0,lambda r:r['source_after']['pixels'].__setitem__(0,0))
if __name__=='__main__':unittest.main()
