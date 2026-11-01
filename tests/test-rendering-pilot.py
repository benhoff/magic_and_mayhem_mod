#!/usr/bin/env python3
import copy,json,struct,sys,unittest,zlib
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from rendering_pilot import capture,acquisition,History,rgba,viewport,commands,counter
import numpy as np

def stream(role,events,limit=None):
 rows=[({'kind':'open','role':role,'session':'session','version':1},b''),*events];total=sum(len(b) for _,b in rows)
 if limit is not None:rows[0][0]['stored_limit_bytes']=limit
 rows.append((dict(kind='close',complete=True,error='',raw_bytes=str(total),prior_rows=len(rows)),b''));out=b'MNMACE01'
 for index,(row,body) in enumerate(rows):
  row=dict(row,index=index,host_us=str(1000+index));raw=json.dumps(row).encode();packed=struct.pack('>I',len(body))+zlib.compress(body) if body else b'';out+=struct.pack('<II',len(raw),len(packed))+raw+packed
 return out

class Admission(unittest.TestCase):
 def test_truncated_and_typed_identity(self):
  good=stream('commands',[(dict(kind='ended',commands=1),b'')]);self.assertEqual(len(capture(good)),3)
  with self.assertRaises(ValueError):capture(good[:-1])
  # Bool must never become numeric version1 during receipt read or restore.
  bad=good.replace(b'"version": 1',b'"version": t',1)
  with self.assertRaises((ValueError,json.JSONDecodeError)):capture(bad)
  with self.assertRaises(ValueError):counter('01')
  with self.assertRaises(ValueError):counter(True)
 def test_stored_capture_budgets_are_explicit_and_bounded(self):
  event=[(dict(kind='ended',commands=1),b'')]
  for limit in (256*1024**2,1024*1024**2):self.assertEqual(len(capture(stream('commands',event,limit))),3)
  for limit in (True,0,1024*1024**2+1,'1073741824'):
   with self.subTest(limit=limit),self.assertRaises(ValueError):capture(stream('commands',event,limit))
 def test_failure_footer(self):
  raw=stream('commands',[(dict(kind='ended',commands=1),b'')]);raw=raw.replace(b'"complete": true',b'"complete": null')
  with self.assertRaises(ValueError):capture(raw)
 def test_closed_role_set_required(self):
  with self.assertRaises(ValueError):acquisition([stream('commands',[(dict(kind='ended',commands=1),b'')])],'session')
 def test_gap_and_missing_end(self):
  header=b'MNMCMD05'+struct.pack('<II',5,16)
  with self.assertRaises(ValueError):list(commands(header+struct.pack('<III',8,2,0)))
  with self.assertRaises(ValueError):list(commands(header))

class PixelHistory(unittest.TestCase):
 def create(self,h,seq,sid,w,hh,words):h.apply(1,seq,struct.pack('<7I',sid,w,hh,16,0xf800,0x7e0,31)+np.array(words,dtype='<u2').tobytes())
 def test_keyed_self_snapshot_and_palette_independence(self):
  h=History({});self.create(h,1,1,4,1,[1,2,0,4]);h.apply(3,2,struct.pack('<10I',1,1,0,0,3,1,1,0,1,0))
  self.assertEqual(h.handles[1]['pixels'].tolist(),[[1,1,2,4]])
 def test_initialization_world_tags_and_masked_ui(self):
  world=np.array([[0xf800,0x07e0]],dtype='uint32');h=History({1:world});self.create(h,1,1,2,1,[0,0]);self.create(h,2,2,2,1,[0,0])
  h.apply(19,3,struct.pack('<5I',1,1,0,99,0));h.apply(3,4,struct.pack('<10I',1,2,0,0,2,1,0,0,0,0));h.apply(20,5,struct.pack('<5I',2,0,0,2,1)+struct.pack('<2H',31,123)+bytes([1,0]));h.apply(6,6,struct.pack('<I',2))
  image=h.render(dict(mode=1,sealed_world='1',world='1'));self.assertEqual(image.tolist(),[[[0,0,255,255],[0,255,0,255]]])
 def test_full_viewport_black_borders_and_scaling(self):
  frame=np.array([[[255,0,0,255],[0,255,0,255]]],dtype='uint8');row=dict(width=6,height=3,left=1,top=1,image_width=4,image_height=1,scaling=0)
  image=np.frombuffer(viewport(frame,row),dtype='u1').reshape(3,6,4);self.assertEqual(image[1,1:5].tolist(),[[255,0,0,255]]*2+[[0,255,0,255]]*2);self.assertTrue(np.all(image[0,:,:3]==0))
 def test_present_snapshot_survives_later_mutation(self):
  h=History({});self.create(h,1,1,1,1,[0xf800]);h.apply(6,2,struct.pack('<I',1))
  h.apply(2,3,struct.pack('<5IH',1,0,0,1,1,0x07e0))
  self.assertEqual(h.render(dict(mode=0,surface=1)).tolist(),[[[255,0,0,255]]])
  h.apply(6,4,struct.pack('<I',1));self.assertEqual(h.render(dict(mode=0,surface=1)).tolist(),[[[0,255,0,255]]])
 def test_oracle_diagnostics_refused(self):
  with self.assertRaises(ValueError):History({}).apply(5,1,b'')

if __name__=='__main__':unittest.main()
