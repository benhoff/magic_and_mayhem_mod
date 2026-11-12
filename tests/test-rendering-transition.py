#!/usr/bin/env python3
"""Adverse terminal-prefix controls never weaken normal END/source admission."""
from copy import deepcopy
from pathlib import Path
import importlib.util,json,struct,sys,unittest
import numpy as np
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from rendering_transition import terminal_commands,terminal_compare,fault_receipt,assert_epoch_partition,TERMINAL_KEYS,capture_path
from rendering_pilot import commands
from rendering_acceptance import requirements
from coverage_claims import fingerprint
from rendering_transition_budget import admit
from fixture_support import command_stream,u32 as word

def fixture():
 fields=[(1,word(1,2,2,16,0xf800,0x7e0,31)+bytes(8)),(19,word(1,1,0,0,0)),(6,word(1))]
 raw=command_stream(fields)
 row=dict(commands=3,mode=0,surface=1,world='1',sealed_world='1',width=2,height=2,frame_width=2,frame_height=2,left=0,top=0,image_width=2,image_height=2,scaling=2,reference_binding=7)
 pixels=bytes([0,0,8,255,0,0,16,255,0,0,24,255,0,0,33,255])
 observed=dict(failed=True,commands=raw+b'uncredited terminal suffix',paints={7:(row,pixels)},swaps=[7],exports={7:dict(row,lease='7')},command_rows=[])
 terminal=dict(schema=1,error='controlled terminal producer',state={k:True for k in TERMINAL_KEYS})
 return observed,{1:np.array([[1,2],[3,4]],dtype='uint32')},terminal

class Prefix(unittest.TestCase):
 def test_complete_visible_prefix_retains_uncredited_tail_without_end(self):
  o,w,t=fixture();r=terminal_compare(o,w,t);self.assertTrue(r['success']);self.assertEqual(r['pixels_compared'],4);self.assertEqual(r['uncredited_terminal_tail_bytes'],26);self.assertFalse(r['synthetic_end_added']);self.assertFalse(r['normal_completion_claimed']);self.assertEqual(r['reference_live_surfaces_before_declared_abort'],1)
 def test_normal_reader_still_refuses_missing_end(self):
  o,_,_=fixture()
  with self.assertRaises(ValueError):list(commands(o['commands']))
  self.assertEqual(len(list(terminal_commands(o['commands'],3))),3)
 def test_required_header_payload_sequence_end_refuse(self):
  o,_,_=fixture();raw=o['commands'][:-26]
  for kind in ('short','extent','gap','end','version'):
   b=bytearray(raw)
   if kind=='short':b=b[:-1]
   if kind=='extent':struct.pack_into('<I',b,24,0xffffffff)
   if kind=='gap':struct.pack_into('<I',b,20,2)
   if kind=='end':struct.pack_into('<I',b,16,8)
   if kind=='version':struct.pack_into('<I',b,8,4)
   with self.subTest(kind=kind),self.assertRaises(ValueError):list(terminal_commands(b,3))
 def test_pixels_and_original_inputs_are_independent(self):
  for changed in ('native','original'):
   o,w,t=fixture()
   if changed=='native':o['paints'][7]=(o['paints'][7][0],bytes(16))
   else:w[1][0,0]=22
   self.assertFalse(terminal_compare(o,w,t)['success'])
 def test_no_false_terminal_or_normal_reinterpretation(self):
  for key in [*TERMINAL_KEYS,'extra','normal','missing-error','boolean-schema']:
   o,w,t=fixture()
   if key in TERMINAL_KEYS:t['state'][key]=False
   elif key=='extra':t['state']['imagined']=True
   elif key=='normal':o['failed']=False
   elif key=='missing-error':t['error']=''
   else:t['schema']=True
   with self.subTest(key=key),self.assertRaises(ValueError):terminal_compare(o,w,t)
 def test_missing_paint_and_duplicated_cpu_reply_refuse(self):
  o,w,t=fixture();o['swaps']=[7,7];r=terminal_compare(o,w,t);self.assertEqual(len(r['comparisons']),2)
  o,w,t=fixture();o['command_rows']=[({'kind':'cpu-read','sequence':2},b'bad')]*2
  with self.assertRaises(ValueError):terminal_compare(o,w,t)

class Fault(unittest.TestCase):
 def fixture(self):
  a=bytearray(64);a[:8]=b'MNMRDC03';struct.pack_into('<8I',a,8,3,1048640,77,42,1,0,0,42);b=bytearray(a);struct.pack_into('<2I',b,24,3,5)
  return dict(success=True,schema=1,reason=5,producer=77,writer_claimed=False,cursor_or_payload_written=False,begin_host_ns='10',end_host_ns='11',before_header_hex=a.hex(),after_header_hex=b.hex())
 def test_actual_failure_wire_fields(self):self.assertEqual(fault_receipt(self.fixture(),77)['after'][6:8],[3,5])
 def test_identity_magic_state_cursors_reason_and_time_refuse(self):
  for kind in ('producer','magic','not-active','not-drained','reason','writes','clock','schema'):
   q=self.fixture()
   if kind=='producer':q['producer']=78
   elif kind in ('magic','not-active','not-drained'):
    b=bytearray.fromhex(q['before_header_hex'])
    if kind=='magic':b[0]^=1
    else:struct.pack_into('<I',b,24 if kind=='not-active' else 36,2)
    q['before_header_hex']=b.hex()
   elif kind=='reason':q['reason']=4
   elif kind=='writes':q['writer_claimed']=True
   elif kind=='clock':q['end_host_ns']='9'
   else:q['schema']=True
   with self.subTest(kind=kind),self.assertRaises(ValueError):fault_receipt(q,77)
 def test_epoch_names_separate_without_changing_legacy(self):
  a=capture_path('/old/world-continuous.bin','/capture');b=capture_path('/fresh/world-continuous.bin','/capture');self.assertNotEqual(a,b)
  with self.assertRaises(ValueError):capture_path('relative','/capture')

class Epochs(unittest.TestCase):
 def fixture(self):
  view=dict(swaps=[1,2],paints={1:({'lease':'1'},b''),2:({'lease':'1'},b'')})
  epochs=[dict(failed=True,opening=dict(host_us='10'),closing=dict(host_us='20'),binding=dict(producer=1,channel='/old'),swaps=[1]),dict(failed=False,opening=dict(host_us='30'),closing=dict(host_us='40'),binding=dict(producer=2,channel='/fresh'),swaps=[2])]
  return view,epochs
 def test_all_swaps_have_unique_epochs(self):
  v,e=self.fixture();self.assertTrue(assert_epoch_partition(v,e)['all_swaps_uniquely_bound'])
 def test_missing_duplicate_overlap_identity_and_class_refuse(self):
  for kind in ('missing','duplicate','overlap','producer','channel','class'):
   v,e=self.fixture()
   if kind=='missing':e[1]['swaps']=[]
   elif kind=='duplicate':e[1]['swaps']=[1,2]
   elif kind=='overlap':e[1]['opening']['host_us']='19'
   elif kind in ('producer','channel'):e[1]['binding'][kind]=e[0]['binding'][kind]
   else:e[1]['failed']=True
   with self.subTest(kind=kind),self.assertRaises(ValueError):assert_epoch_partition(v,e)

class Closeout(unittest.TestCase):
 def fixture(self):
  root=Path(__file__).resolve().parents[1];spec=importlib.util.spec_from_file_location('transition_close',root/'tools/close-rendering-transition.py');m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)
  plan=json.loads((root/'docs/native-rendering-closure.json').read_bytes());parent=dict(success=True,dossier_identity=fingerprint(requirements(plan)),requirements=[dict(v,state='pending') for v in requirements(plan)],passed_requirements=0)
  cases=[dict(software=f,size=s,identity=str((f,s)),verified=True,cleanup=dict(verified=True),physical_public_restart=True,old_rendering_accepted=False,pixels=10,mismatching_pixels=0,pair={},provider='fixture') for f in (False,True) for s in ([800,600],[640,480])]
  return m,plan,parent,cases
 def test_only_two_recovery_rows_change(self):
  m,p,parent,c=self.fixture();rows=m.close_rows(p,parent,c)
  self.assertEqual(sum(v['state']=='passed' for v in rows),2)
  self.assertEqual([v for v in rows if v['id']!='operation.transition-recovery'],[v for v in parent['requirements'] if v['id']!='operation.transition-recovery'])
 def test_missing_duplicate_domain_identity_and_failed_nominal_credit_refuse(self):
  for kind in ('missing','duplicate-domain','duplicate-identity','failed-credit','no-cleanup','no-restart','pixels'):
   m,p,parent,c=self.fixture()
   if kind=='missing':c.pop()
   elif kind=='duplicate-domain':c[1]['size']=c[0]['size']
   elif kind=='duplicate-identity':c[1]['identity']=c[0]['identity']
   elif kind=='failed-credit':c[0]['old_rendering_accepted']=True
   elif kind=='no-cleanup':c[0]['cleanup']['verified']=False
   elif kind=='no-restart':c[0]['physical_public_restart']=False
   else:c[0]['mismatching_pixels']=1
   with self.subTest(kind=kind),self.assertRaises(ValueError):m.close_rows(p,parent,c)

class Budget(unittest.TestCase):
 def case(self,name,repair=None,success=False,games=0,proved=False):return dict(report=name,repair_of=repair,result=dict(success=success,original_game_runs=games),proved=proved,comparison_failed=False)
 def test_two_zero_game_failures_remain_in_repair_ancestry(self):
  a=self.case('setup1');b=self.case('setup2','setup1');r=admit([a,b],'setup2',6,10);self.assertEqual(r['requests_before'],2);self.assertEqual(r['original_starts_before'],0);self.assertEqual(r['repair_ancestors'],['setup1','setup2'])
 def test_failed_ancestors_only_resolve_after_compared_repair(self):
  a=self.case('a',games=2);b=self.case('b','a',True,2,True);r=admit([a,b],None,6,10);self.assertEqual(r['resolved_ancestors'],['a'])
  b['proved']=False
  with self.assertRaises(ValueError):admit([a,b],None,6,10)
 def test_whole_next_two_start_case_must_fit(self):
  cases=[self.case(str(i),success=True,games=2,proved=True) for i in range(4)]
  self.assertEqual(admit(cases,None,6,10)['reserved_original_starts'],2)
  with self.assertRaises(ValueError):admit(cases,None,6,9)
 def test_unknown_count_duplicate_request_and_exhausted_request_refuse(self):
  for cases,count in [([self.case('x',games=None)],6),([self.case('x')]*2,6),([self.case('x',proved=True)],1)]:
   with self.assertRaises(ValueError):admit(cases,None,count,10)
if __name__=='__main__':unittest.main()
