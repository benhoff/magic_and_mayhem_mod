#!/usr/bin/env python3
import copy,importlib.util,unittest
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];s=importlib.util.spec_from_file_location('b',ROOT/'tools/check-surface-borrowed-draw.py');b=importlib.util.module_from_spec(s);s.loader.exec_module(b)
class BorrowedDraw(unittest.TestCase):
 @classmethod
 def setUpClass(cls):_,cls.data=b.load()
 def test_matrix(self):self.assertEqual(b.compare(self.data)['cases'],180)
 def mutate(self,choose,field,value):
  d=copy.deepcopy(self.data);r=next(r for r in d['rows'] if choose(r['input']));r[field]=value
  with self.assertRaises(ValueError):b.compare(d)
 def test_locked_fill_succeeds(self):self.mutate(lambda c:c==dict(id=12,kind=0,held=2,clip=0),'hresult',0x887601ae)
 def test_dc_fill_succeeds(self):self.mutate(lambda c:c['kind']==0 and c['held']==4 and c['clip']==0,'hresult',0x887601ae)
 def test_empty_blt_is_not_busy(self):self.mutate(lambda c:c['kind']==1 and c['held']==1 and c['clip']==4,'hresult',0x887601ae)
 def test_fast_empty_clip_refuses(self):self.mutate(lambda c:c['kind']==3 and c['held']==0 and c['clip']==4,'hresult',0)
 def test_source_dc_copy_busy(self):self.mutate(lambda c:c['kind']==2 and c['held']==3 and c['clip']==0,'hresult',0)
 def test_key_preserves_destination(self):
  d=copy.deepcopy(self.data);r=next(r for r in d['rows'] if r['input']['kind']==2 and r['input']['held']==0 and r['input']['clip']==0);r['destination_after'][0]=0x8000
  with self.assertRaises(ValueError):b.compare(d)
 def test_complete_inputs(self):
  d=copy.deepcopy(self.data);d['rows'].pop()
  with self.assertRaises(ValueError):b.compare(d)
if __name__=='__main__':unittest.main()
