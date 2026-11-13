#!/usr/bin/env python3
"""Exact retained hotspot analysis bindings; fixtures grant no game acceptance."""
import copy,json,sys,tempfile,unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from broader_player_hotspot_analysis_matrix import analysis_binding,P,BINARY

class Binding(unittest.TestCase):
 def setUp(self):
  self.tmp=tempfile.TemporaryDirectory();self.addCleanup(self.tmp.cleanup);self.root=Path(self.tmp.name)
  self.cid='bp27-world-hotspot-default800-01';self.spec=dict(family='hotspot',role='world',plan='phase',case=dict(id=self.cid,report='capture'))
  self.row=dict(case_id=self.cid,report='capture',plan='pinned phase',reserved_games=1,started_ns=123,acquisition_sha256='a',state='admitted',validation='oldvalidation')
  self.name=P+self.cid+'-analysis-revalidation-01.json';path=self.root/self.name;path.parent.mkdir(parents=True);path.touch()
  ref=lambda n:dict(record=n,sha256='hash:'+n)
  self.oldco=P+self.cid+'-coordinator-01.json'
  self.b=dict(success=True,kind='broader_retained_hotspot_analysis_revalidation',case=self.cid,original_game_runs=0,new_native_executions=0,new_pixel_comparisons=1,Native_ELF_sha256=BINARY,request_identity={k:self.row[k]for k in('case_id','report','plan','reserved_games','started_ns','acquisition_sha256')},ledger_validation='oldvalidation',acquisition=ref('capture'),plan=ref('phase'),source_phase_waivers=[],original_coordinator=ref(self.oldco),original_coordinator_success=True,original_validation=ref('oldvalidation'),raw_capture_and_Native_inputs_unchanged=True,historical_failed_comparison_preserved=True,sources={'analyzer':'hash:analyzer'},current_validation=ref('validation2'),current_coordinator=ref('coordinator2'),current_handoff=ref('handoff2'))
  self.data={self.name:self.b,self.oldco:dict(success=True,validation=ref('oldvalidation')),'validation2':dict(success=True),'coordinator2':dict(success=True),'handoff2':dict(success=True)}
  outer=self
  class Reader:
   def digest(self,n):return 'hash:'+n
   def json(self,n):return outer.data[n]
  self.r=Reader()
 def check(self):return analysis_binding(self.root,self.r,self.spec,self.row)
 def test_exact_admitted_history_can_select_new_analysis(self):self.assertEqual(self.check()['value']['current_validation']['record'],'validation2')
 def test_no_binding_keeps_existing_path(self):
  (self.root/self.name).unlink();self.assertIsNone(self.check())
 def test_other_family_refuses(self):
  self.spec['family']='font'
  with self.assertRaises(ValueError):self.check()
 def test_new_original_start_refuses(self):
  self.b['original_game_runs']=1
  with self.assertRaises(ValueError):self.check()
 def test_new_native_execution_refuses(self):
  self.b['new_native_executions']=1
  with self.assertRaises(ValueError):self.check()
 def test_different_player_refuses(self):
  self.b['Native_ELF_sha256']='other'
  with self.assertRaises(ValueError):self.check()
 def test_acquisition_substitution_refuses(self):
  self.b['acquisition']['sha256']='other'
  with self.assertRaises(ValueError):self.check()
 def test_reservation_or_start_identity_cannot_change(self):
  for key in ['reserved_games','started_ns','acquisition_sha256','plan']:
   with self.subTest(key=key):
    old=self.b['request_identity'][key];self.b['request_identity'][key]='other'
    with self.assertRaises(ValueError):self.check()
    self.b['request_identity'][key]=old
 def test_original_validation_pointer_cannot_be_rewritten(self):
  self.row['validation']='other'
  with self.assertRaises(ValueError):self.check()
 def test_historical_negative_must_remain_preserved(self):
  self.b['historical_failed_comparison_preserved']=False
  with self.assertRaises(ValueError):self.check()
 def test_native_source_waiver_refuses(self):
  self.b['source_phase_waivers']=['renderer']
  with self.assertRaises(ValueError):self.check()
 def test_stale_analysis_source_refuses(self):
  self.b['sources']['analyzer']='old'
  with self.assertRaises(ValueError):self.check()
 def test_new_failed_validation_refuses(self):
  self.data['validation2']['success']=False
  with self.assertRaises(ValueError):self.check()
 def test_old_coordinator_identity_cannot_change(self):
  self.b['original_coordinator']['sha256']='other'
  with self.assertRaises(ValueError):self.check()
 def test_acquired_request_can_finish_without_rerun_or_ledger_rewrite(self):
  self.cid='bp27-world-hotspot-default640-01';self.spec['case']['id']=self.cid
  name=P+self.cid+'-analysis-revalidation-01.json';p=self.root/name;p.touch()
  self.row['case_id']=self.cid;self.row['validation']='validation2'
  self.b['case']=self.cid;self.b['request_identity']['case_id']=self.cid
  self.b['ledger_validation']='validation2';self.b['original_validation']=None;self.b['original_coordinator_success']=False
  co=P+self.cid+'-coordinator-01.json';self.b['original_coordinator']=dict(record=co,sha256='hash:'+co)
  self.data[co]=dict(success=False);self.data[name]=self.b
  self.assertIsNotNone(self.check())
if __name__=='__main__':unittest.main()
