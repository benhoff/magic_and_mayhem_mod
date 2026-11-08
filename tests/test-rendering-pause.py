#!/usr/bin/env python3
"""Reject false pause credit, stalled hosts, leaked keys and media ownership."""
from copy import deepcopy
from pathlib import Path
import sys,unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from rendering_pause import validate_dwell

def fixture():
 return [dict(host_ns=1_000_000_000+i*100_000_000,original_world_return=10,
   input=dict(active=0,held=[],keys=[0]*256,sequence=2+i*2),
   world=dict(nonce='1234',pending_world_jobs=0),
   media=dict(operation=1,status=3,request=2,response=2,heartbeat=i),
   music=dict(ready=1,failed=0,losses=0,heartbeat=i)) for i in range(31)]
class Pause(unittest.TestCase):
 def test_selected_dwell_only(self):
  r=validate_dwell(fixture());self.assertTrue(r['verified']);self.assertFalse(r['music_playback_frozen_claimed']);self.assertFalse(r['all_simulation_animation_pause_claimed'])
 def test_bad_ownership_refuses(self):
  mutations=[lambda r:r.update(original_world_return=11),lambda r:r['world'].update(nonce='1235'),
   lambda r:r['world'].update(pending_world_jobs=1),lambda r:r['input'].update(active=1),
   lambda r:r['input'].update(held=[38]),lambda r:r['input']['keys'].__setitem__(38,1),
   lambda r:r['media'].update(status=2),lambda r:r['media'].update(response=1),
   lambda r:r['music'].update(ready=0),lambda r:r['music'].update(failed=1),lambda r:r['music'].update(losses=1)]
  for i,mutate in enumerate(mutations):
   rows=fixture();mutate(rows[15])
   with self.subTest(i=i),self.assertRaises(ValueError):validate_dwell(rows)
 def test_sparse_clock_and_stalled_hosts_refuse(self):
  for what in ('short','reordered','input','media','music'):
   rows=fixture()
   if what=='short':rows=rows[:29]
   elif what=='reordered':rows[12]['host_ns']=rows[11]['host_ns']
   else:
    key='sequence' if what=='input' else 'heartbeat'
    for r in rows:r[what][key]=0
   with self.subTest(what=what),self.assertRaises(ValueError):validate_dwell(rows)
class Matrix(unittest.TestCase):
 @classmethod
 def setUpClass(cls):
  import importlib.util
  from coverage_evidence import Reader
  root=Path(__file__).resolve().parents[1]
  spec=importlib.util.spec_from_file_location('close_pause',root/'tools/close-rendering-pause.py');cls.module=importlib.util.module_from_spec(spec);spec.loader.exec_module(cls.module)
  with Reader(root) as r:
   cls.plan=r.json('docs/native-rendering-closure.json');cls.parent=r.json('research/runtime/rendering-fill-operations-results-20261107/closeout-integrated-05.json')
 @staticmethod
 def cases():
  return [dict(software=f,size=z,identity=str((f,z)),verified=True,cleanup=dict(verified=True),pixels=100,mismatching_pixels=0,pair={},provider='synthetic',
    dwells=[dict(verified=True,samples=31,duration_ns=3000000000,compared_resumed_paints=3,all_simulation_animation_pause_claimed=False,music_playback_frozen_claimed=False)]*2)
   for f in (False,True) for z in ([800,600],[640,480])]
 def test_only_frozen_pause_rows_change(self):
  rows=self.module.close_rows(self.plan,self.parent,self.cases())
  changed=[(a,b) for a,b in zip(self.parent['requirements'],rows) if a!=b]
  self.assertEqual(len(changed),2)
  self.assertTrue(all(a['id']=='journey.pause-preferences.pause' and a['state']=='pending' and b['state']=='passed' for a,b in changed))
 def test_missing_duplicate_bad_matrix_refuses(self):
  for what in ('missing','duplicate','extent','interval','cleanup','pixels','sparse','duration','resume','overclaim'):
   c=deepcopy(self.cases())
   if what=='missing':c.pop()
   elif what=='duplicate':c[1]['identity']=c[0]['identity']
   elif what=='extent':c[0]['size']=[801,600]
   elif what=='interval':c[0]['dwells'].pop()
   elif what=='cleanup':c[0]['cleanup']['verified']=False
   elif what=='pixels':c[0]['mismatching_pixels']=1
   elif what=='sparse':c[0]['dwells'][0]['samples']=29
   elif what=='duration':c[0]['dwells'][0]['duration_ns']=1999999999
   elif what=='resume':c[0]['dwells'][0]['compared_resumed_paints']=0
   else:c[0]['dwells'][0]['music_playback_frozen_claimed']=True
   with self.subTest(what=what),self.assertRaises(ValueError):self.module.close_rows(self.plan,self.parent,c)
if __name__=='__main__':unittest.main()
