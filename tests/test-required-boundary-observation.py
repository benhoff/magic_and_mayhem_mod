#!/usr/bin/env python3
"""Loss/identity/torn-stream controls for the bounded guest diagnostic."""
from pathlib import Path
import sys,struct,unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from required_boundary_observation import decode

def fixture():
 h=[0]*16;h[:8]=[*struct.unpack('<2I',b'MNMBND01'),1,64,512,8192,0x40209ca7,23]
 rows=[]
 for event in (1,2,3,4):
  r=[0]*128;r[0]=len(rows)+1;r[1]=event;r[3]=7;r[5]=0x401234
  if event in (1,2):r[2]=2;r[6]=0x123000
  if event==2:r[11]=1
  if event==3:r[6:10]=[0x456000,800,600,800];r[11]=1
  if event==4:r[5:8]=[3,1,1]
  rows.append(r)
 return h,rows

def pack(h,r):return struct.pack('<16I',*h)+b''.join(struct.pack('<128I',*v) for v in r)
class Controls(unittest.TestCase):
 def test_valid(self):
  d=decode(pack(*fixture()));self.assertEqual(len(d['pairs']),1);self.assertEqual(d['worlds_observed'],1)
 def test_refusals(self):
  changes=[(0,0,9),(1,2,23),(1,11,9),(2,5,0),(2,6,9),(2,11,2),
           (3,6,0),(3,7,801),(3,9,799),(4,2,1),(4,3,8),(4,5,4),(4,7,2),(4,8,1)]
  for row,field,value in changes:
   with self.subTest(row=row,field=field):
    h,r=fixture();(h if row==0 else r[row-1])[field]=value
    with self.assertRaises(ValueError):decode(pack(h,r))
 def test_truncation(self):
  raw=pack(*fixture())
  for at in (0,8,63,64,len(raw)-1,len(raw)-512):
   with self.subTest(at=at):
    with self.assertRaises(ValueError):decode(raw[:at])
 def test_orphan(self):
  h,r=fixture();r[1][11]=2
  with self.assertRaises(ValueError):decode(pack(h,r))
 def test_entry_only(self):
  h,r=fixture();r[0][2]=18;r.pop(1)
  for i,row in enumerate(r,1):row[0]=i
  r[-1][5]=2
  self.assertEqual(len(decode(pack(h,r))['pairs']),0)
if __name__=='__main__':unittest.main()
