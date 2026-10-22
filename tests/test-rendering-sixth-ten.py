"""Exact resource-prefix event lookup and strict portable identity regressions."""
import importlib.util
from pathlib import Path
import struct
import sys
import tracemalloc
import unittest
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools'))
from drawing_resource_index import ResourceIndex
from drawing_wine_resource_mapping import decode as resources
from drawing_worker_context import channel
from drawing_wine_retained_map_source import decode as retained


def fixture(name,function,*args):
 spec=importlib.util.spec_from_file_location(name,ROOT/('tests/test-drawing-'+name+'.py'));m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);return getattr(m,function)(*args)

def mutate(raw,word,value):
 result=bytearray(raw);struct.pack_into('<I',result,word*4,value);return result

class SixthTen(unittest.TestCase):
 def test_retained_prefix_at_map_and_unmap_boundaries(self):
  q,raw,sub,life=fixture('wine-retained-map-source','fixture')
  resources(raw);index=ResourceIndex(raw)
  self.assertEqual(index.candidates(3,1,1,0),[])
  self.assertEqual(index.candidates(4,1,1,0)[0]['open'],1)
  self.assertEqual(index.candidates(5,1,1,0)[0]['open'],1)
  self.assertEqual(index.candidates(6,1,1,0)[0]['open'],0)
  for prefix in (-1,8):
   with self.assertRaises(ValueError):index.candidates(prefix,1,1,0)
  self.assertEqual(index.candidates(6,1,0,0),[])
  self.assertEqual(retained(q,raw,sub,life)['matched'],1)

 def test_retained_full_pipeline_has_linear_prefix_memory(self):
  q,raw,sub,life=fixture('wine-retained-map-source','fixture',True)
  # 200 independent map intervals and 1600 exact query prefixes defeat a full
  # dict snapshot at every requested prefix while requiring exact source joins.
  rows=list(struct.iter_unpack('<32I',raw[64:-128]));rows=[list(r) for r in rows]
  call=3;interval=1
  for resource in range(10,210):
   call+=1;interval+=1;e=[0]*32;e[:7]=[len(rows)+1,1,call,2,18,0,0];e[10]=resource
   rows.append(e);r=list(e);r[:2]=[len(rows)+1,2];r[24:28]=[2,2,500,interval];rows.append(r)
  # Distinct successful getter calls create many retained prefixes without
  # changing the candidate of the source texture being queried.
  starts=[]
  for n in range(1600):
   call+=1;e=[0]*32;e[:7]=[len(rows)+1,1,call,1,18,0,0];e[10:12]=[1000,0];rows.append(e);r=list(e);r[:2]=[len(rows)+1,2];r[8]=1;r[14]=2;rows.append(r);starts.append(r[0])
  t=[0]*32;t[:5]=[len(rows)+1,4,1,call,18];t[10]=2048;t[27]=interval;t[30]=200;rows.append(t)
  raw=raw[:64]+b''.join(struct.pack('<32I',*r) for r in rows)
  # Each synthetic source entry uses the same live source and its own exact
  # observer identity; the map remains closed throughout all prefixes.
  s0=list(struct.unpack_from('<48I',sub,64));s1=list(struct.unpack_from('<48I',sub,256));sr=[];qr=[]
  query=list(struct.unpack_from('<48I',q,64))
  for n,prefix in enumerate(starts,1):
   for phase,template in [(1,s0),(2,s1)]:
    row=list(template);row[0]=len(sr)+1;row[1]=phase;row[2]=n;sr.append(row)
   row=list(query);row[0]=n;row[2]=n;row[16:18]=[prefix,prefix];qr.append(row)
  st=list(struct.unpack_from('<48I',sub,len(sub)-192));st[0]=len(sr)+1;st[3]=len(starts);sr.append(st)
  qt=list(struct.unpack_from('<48I',q,len(q)-192));qt[0]=len(qr)+1;qt[4]=len(starts);qr.append(qt)
  sub=sub[:64]+b''.join(struct.pack('<48I',*r) for r in sr);q=q[:64]+b''.join(struct.pack('<48I',*r) for r in qr)
  tracemalloc.start()
  try:value=retained(q,raw,sub,life);_,peak=tracemalloc.get_traced_memory()
  finally:tracemalloc.stop()
  self.assertEqual(value['matched'],1600);self.assertLess(peak,16*1024*1024)

 def test_resource_parent_depth_and_identity_agree(self):
  raw=fixture('wine-resource-map','wire')
  for entry,depth in ((1,0),(0,1),(1,33)):
   invalid=bytearray(raw)
   for at in (64,192):struct.pack_into('<2I',invalid,at+5*4,entry,depth)
   with self.assertRaises(ValueError):resources(invalid)

 def test_unknown_context_cannot_retain_submission(self):
  words=fixture('worker-context','context');words[7]=0;words[11]=1
  with self.assertRaises(ValueError):channel(tuple(words))
  words[8]=0;self.assertEqual(channel(tuple(words))['command'],0)

if __name__=='__main__':unittest.main()
