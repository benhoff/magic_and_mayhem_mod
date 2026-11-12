#!/usr/bin/env python3
"""Synthetic admission/restore guards; these do not execute or certify a player."""
import copy,json,sys,tempfile,unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from rendering_pilot_ledger import Workspace,payload
from coverage_claims import fingerprint

class Reader:
 def __init__(self):
  self.reports={};self.source='f'*64
 def add(self,name,value):
  name='research/runtime/rendering-acceptance-pilot-results-20261031/'+name+'.json';self.reports[name]=value
  return dict(record=name,sha256=self.digest(name))
 def digest(self,name):return fingerprint(self.reports[name]) if name in self.reports else self.source
 def json(self,name):return copy.deepcopy(self.reports[name])

class Guards(unittest.TestCase):
 def setUp(self):
  self.reader=Reader();r=self.reader
  a=dict(kind='public_native_pilot_acquisition',success=True,sources_stable=True,original_manifest_verified_before_after=True,
   configuration=dict(software_flag=False,skip_intros=False),private_wine_stopped=True,retirement=dict(state='completed'),sources={'renderer/commands.hpp':r.source},original_pixels_used_as_native_inputs=False)
  ar=r.add('synthetic-acquisition',a)
  c=dict(a,kind='public_native_pilot_comparison',acquisition=ar,input_acknowledgements=dict(verified=True),original_menu_actions=dict(verified=True),asset_inputs_stable=True,
   comparison=dict(success=True,world_presentations=1,pixels_compared=1,mismatching_pixels=0,remaining_surfaces=0,comparisons=[dict(world=1,pixels=1,mismatching_pixels=0,actual_sha256=r.source,expected_sha256=r.source)],scope='synthetic'),
   observations=dict(world_swaps=1),original_reference=dict(continuous_from_owned_initial_history=True,native_player_pixels_read=False,original_destination_capture_read=False,worlds=1,draws=1))
  p=copy.deepcopy(a);p['configuration']['software_flag']=True
  self.value=dict(kind='EP03.pipeline',reason='Synthetic admission guard fixture',acquisition=ar,comparison=r.add('synthetic-comparison',c),software_probe=r.add('synthetic-probe',p))
  self.plan=json.loads((Path(__file__).resolve().parents[1]/'docs/native-rendering-closure.json').read_text())
 def test_positive_fixture_and_current_source_guard(self):
  self.assertEqual(payload(self.reader,self.value)['world_presentations'],1)
  with tempfile.TemporaryDirectory() as td:
   w=Workspace(td,self.plan)
   try:
    key=w.append(self.reader,self.value);self.assertEqual(w.append(self.reader,self.value),key)
    self.reader.source='e'*64
    with self.assertRaises(ValueError):w.append(self.reader,self.value)
   finally:w.close()
 def test_reject_incomplete_forged_and_mixed_phase_reports(self):
  mutations=[('success',1),('sources_stable',False),('original_manifest_verified_before_after',None),('original_pixels_used_as_native_inputs',True)]
  for field,value in mutations:
   with self.subTest(field=field):
    r=copy.deepcopy(self.reader);r.reports[self.value['acquisition']['record']][field]=value
    v=copy.deepcopy(self.value);v['acquisition']['sha256']=r.digest(v['acquisition']['record'])
    with self.assertRaises(ValueError):payload(r,v)
  for field,value in [('reason',''),('reason',None),('kind','supported-player')]:
   v=copy.deepcopy(self.value);v[field]=value
   with self.assertRaises(ValueError):payload(self.reader,v)
  r=copy.deepcopy(self.reader);r.reports[self.value['comparison']['record']]['sources']['renderer/commands.hpp']='e'*64
  v=copy.deepcopy(self.value);v['comparison']['sha256']=r.digest(v['comparison']['record'])
  with self.assertRaises(ValueError):payload(r,v)
 def test_reopen_and_restore_preserve_scope_and_reject_conflicts(self):
  with tempfile.TemporaryDirectory() as td,tempfile.TemporaryDirectory() as other:
   w=Workspace(td,self.plan);w.append(self.reader,self.value);snapshot=w.snapshot(self.reader);w.close()
   w=Workspace(td,self.plan);self.assertEqual(w.snapshot(self.reader),snapshot);w.close()
   w=Workspace(other,self.plan)
   try:
    w.restore(self.reader,snapshot);self.assertEqual(w.snapshot(self.reader),snapshot)
    for field,value in [('schema',True),('closes_full_requirement_rows',True),('decisions',{'supported-player':'accepted','original-recovery':'pending'})]:
     bad=copy.deepcopy(snapshot);bad[field]=value
     with self.assertRaises(ValueError):w.restore(self.reader,bad)
    bad=copy.deepcopy(snapshot);bad['events'][0]['observed_at']='2026-10-31T00:00:00'
    with self.assertRaises(ValueError):w.restore(self.reader,bad)
    bad=copy.deepcopy(snapshot);bad['events'][0]['observed_at']='2026-10-31T00:00:00+00:00'
    with self.assertRaises(ValueError):w.restore(self.reader,bad)
   finally:w.close()

if __name__=='__main__':unittest.main()
