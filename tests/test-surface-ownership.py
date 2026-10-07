#!/usr/bin/env python3
import copy,importlib.util,unittest
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
s=importlib.util.spec_from_file_location('ownership',ROOT/'tools/check-surface-ownership.py');m=importlib.util.module_from_spec(s);s.loader.exec_module(m)
class Oracle(unittest.TestCase):
 @classmethod
 def setUpClass(cls):cls.report,cls.rows=m.load()
 def corrupt(self,kind,column,value):
  rows=copy.deepcopy(self.rows);next(r for r in rows if r[0]==kind)[column]=value
  with self.assertRaises(ValueError):m.validate(self.report,rows)
 def test_complete(self):self.assertGreater(len(m.validate(self.report,self.rows)['frames']),20)
 def test_pixel(self):self.corrupt(33,5,999)
 def test_palette(self):self.corrupt(37,5,0xffffffff)
 def test_key(self):self.corrupt(34,4,999)
 def test_alias_identity(self):self.corrupt(31,4,0)
 def test_busy_flip(self):
  rows=copy.deepcopy(self.rows);next(r for r in rows if r[:3]==[40,31,3])[3]=0
  with self.assertRaises(ValueError):m.validate(self.report,rows)
 def test_lease_return(self):self.corrupt(45,3,0)
 def test_dc_flipback(self):self.corrupt(48,3,0)
 def test_reorder(self):
  rows=copy.deepcopy(self.rows);rows[0],rows[1]=rows[1],rows[0]
  with self.assertRaises(ValueError):m.validate(self.report,rows)
 def test_completion(self):
  with self.assertRaises(ValueError):m.validate(self.report,self.rows[:-1])
 def test_diagnostic_counts(self):
  rows=copy.deepcopy(self.rows)
  for r in rows:
   if r[0]==39:r[3]=99
  m.validate(self.report,rows)
if __name__=='__main__':unittest.main()
