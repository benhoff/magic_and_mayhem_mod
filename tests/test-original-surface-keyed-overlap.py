#!/usr/bin/env python3
import copy,importlib.util,unittest
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
s=importlib.util.spec_from_file_location('k',ROOT/'tools/check-original-surface-keyed-overlap.py');k=importlib.util.module_from_spec(s);s.loader.exec_module(k)
class KeyedOverlap(unittest.TestCase):
 @classmethod
 def setUpClass(cls):cls.rows,_=k.load()
 def test_complete_outputs(self):self.assertEqual(k.compare(self.rows)['cases'],2304)
 def test_corrupt_pixels(self):
  rows=copy.deepcopy(self.rows);rows[0]['destination_after'][0]^=1
  with self.assertRaises(ValueError):k.compare(rows)
 def test_corrupt_result(self):
  rows=copy.deepcopy(self.rows);rows[0]['hresult']^=1
  with self.assertRaises(ValueError):k.compare(rows)
 def test_key_not_opaque(self):
  rows=copy.deepcopy(self.rows);r=next(r for r in rows if r['input']['pattern']==0 and r['input']['phase']==0 and r['hresult']==0 and r['input']['label']=='right');r['destination_after']=k.opaque.cpu(r)[1]
  with self.assertRaises(ValueError):k.compare(rows)
 def test_clip_order_matters(self):
  different=0
  for r in self.rows:
   h,pieces=k.opaque.plan(r);out=list(r['destination_before']);frozen=r['source_before'];key=r['input']['key']
   for (l,t,right,bottom),dx,dy in pieces:
    for y in range(t,bottom):
     for x in range(l,right):
      if frozen[y*8+x]!=key:out[(dy+y-t)*8+dx+x-l]=frozen[y*8+x]
   different+=out!=r['destination_after']
  self.assertGreater(different,0)
 def test_complete_matrix(self):
  rows=copy.deepcopy(self.rows);rows.pop()
  with self.assertRaises(ValueError):k.compare(rows)
if __name__=='__main__':unittest.main()
