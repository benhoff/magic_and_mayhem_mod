#!/usr/bin/env python3
"""Refuse widened boundary claims using actual retained execution data."""
import copy,json,sys,unittest
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools'))
from coverage_evidence import Reader
from rendering_menu_acceptance import artifact
from rendering_interaction import decode as input_decode
from required_boundary_observation import decode
import required_boundaries_gate as gate
from operations_gate import module

class Proxy:
 def __init__(self,r,reports=None,changed=None):self.reader=r;self.reports=reports or{};self.changed=changed or{};self.root=r.root
 def digest(self,n):return self.changed.get(n,self.reader.digest(n))
 def json(self,n):return copy.deepcopy(self.reports[n])if n in self.reports else self.reader.json(n)
 def read_bytes(self,n):return self.reader.read_bytes(n)

class Controls(unittest.TestCase):
 @classmethod
 def setUpClass(cls):
  cls.r=Reader(ROOT);cls.b=cls.r.json(gate.PREFIX+'closeout-bindings-02.json');cls.parent=cls.b['parent'];cls.plan=cls.r.json('docs/native-rendering-closure.json');cls.policy=cls.r.json(gate.POLICY)
  cls.a=gate.checked(cls.r,cls.b['fullscreen'][0][0]);cls.c=gate.checked(cls.r,cls.b['fullscreen'][0][1]);cls.input=input_decode(artifact(cls.r,cls.a,'frame-input.bin'))['rows'];cls.storage=decode(artifact(cls.r,cls.a,'required-boundaries-v1.bin'))
  cls.provider=gate.checked(cls.r,cls.b['providers'][0][0]);cls.pwire=decode(artifact(cls.r,cls.provider,'required-boundaries-v1.bin'))
  cls.proof=gate.analyze(cls.r,cls.parent,cls.b,cls.policy)
 @classmethod
 def tearDownClass(cls):cls.r.close()
 def test_actual_join(self):
  self.assertTrue(gate.validate_proof(self.proof));self.assertEqual(self.proof['total_new_compared_Worlds'],2831);self.assertEqual(self.proof['total_new_compared_pixels'],11903176720)
 def test_exact_two_cells(self):
  rows=gate.close_rows(self.parent,dict(record='proof',sha256='a'*64),self.proof)
  self.assertEqual(sum(r['state']=='passed'for r in rows),156);self.assertEqual([r for r in rows if r['id']!=gate.GATE],[r for r in self.parent['requirements']if r['id']!=gate.GATE])
 def test_all_eight_answered_originals_open(self):
  self.assertEqual(len(self.proof['decisions']),8);self.assertTrue(all(v['player_scope_state']=='resolved'and v['original_recovery_state']=='unresolved'for v in self.proof['decisions']))
 def test_budget_includes_refusals(self):self.assertEqual((self.proof['acquisition_budget']['requests'],self.proof['acquisition_budget']['original_starts']),(10,6))
 def test_actual_populated_both_profiles(self):self.assertTrue(all(v['lifetimes']['main']and v['lifetimes']['reports']and v['world_resources']['world_epochs']==2 for v in self.proof['providers']))
 def test_reference_hash(self):
  ref=copy.deepcopy(self.b['format']);ref['sha256']='0'*64
  with self.assertRaises(ValueError):gate.checked(self.r,ref)
 def test_reference_path_escape(self):
  with self.assertRaises(ValueError):gate.checked(self.r,dict(record='../escape',sha256='0'*64))
 def test_actual_dynamic_helpers_are_pinned(self):
  from coverage_claims import required_sources
  dynamic={'tools/'+v+'.py'for f in(gate.fullscreen_case,gate.provider_case,gate.basis)
      for v in f.__code__.co_consts if isinstance(v,str)and(ROOT/'tools'/(v+'.py')).is_file()}
  self.assertEqual(len(dynamic),3)
  b=next(v for v in self.r.json('research/runtime/coverage/register.json')['behaviors']if v['id']==gate.BID)
  self.assertTrue(dynamic<=gate.local_python_sources()and dynamic<=required_sources(b))
 def test_native_drift_is_not_host_correction(self):
  path='runtime/scene/required_boundaries.c'
  with self.assertRaises(ValueError):gate.fullscreen_case(Proxy(self.r,changed={path:'0'*64}),*self.b['fullscreen'][0],'player-default')
 def test_other_host_change_refused(self):
  with self.assertRaises(ValueError):gate.fullscreen_case(Proxy(self.r,changed={'tools/run-required-boundary-session.py':'0'*64}),*self.b['fullscreen'][0],'player-default')
 def test_missing_provider_profile(self):
  b=copy.deepcopy(self.b);b['providers'].pop()
  with self.assertRaises(ValueError):gate.analyze(self.r,self.parent,b,self.policy)
 def test_missing_fullscreen_profile(self):
  b=copy.deepcopy(self.b);b['fullscreen'].pop()
  with self.assertRaises(ValueError):gate.analyze(self.r,self.parent,b,self.policy)
 def test_provider_acquisition_not_pixels(self):
  a=copy.deepcopy(self.provider);a['player_accepted']=True;ref=self.b['providers'][0][0]
  with self.assertRaises(ValueError):gate.provider_case(Proxy(self.r,{ref['record']:a}),*self.b['providers'][0],'player-default')
 def test_provider_wrong_save(self):
  a=copy.deepcopy(self.provider);a['entry_checkpoint']['sha256']='0'*64;ref=self.b['providers'][0][0]
  with self.assertRaises(ValueError):gate.provider_case(Proxy(self.r,{ref['record']:a}),*self.b['providers'][0],'player-default')
 def test_provider_survivor(self):
  a=copy.deepcopy(self.provider);a['retirement']['graphics_retirement']['counts']['surfaces']=1;ref=self.b['providers'][0][0]
  with self.assertRaises(ValueError):gate.provider_case(Proxy(self.r,{ref['record']:a}),*self.b['providers'][0],'player-default')
 def test_format_control_missing555(self):
  ref=self.b['format'];q=gate.checked(self.r,ref);q['rows'].pop()
  with self.assertRaises(ValueError):gate.format_control(Proxy(self.r,{ref['record']:q}),ref)
 def test_format_control_live_promotion(self):
  ref=self.b['format'];q=gate.checked(self.r,ref);q['physical_driver_equivalence']=True
  with self.assertRaises(ValueError):gate.format_control(Proxy(self.r,{ref['record']:q}),ref)
 def test_caller_count_positive_not_fabricated(self):
  ref=self.b['caller_counts'][0];q=gate.checked(self.r,ref);q['natural_primary_witness']=True
  with self.assertRaises(ValueError):gate.caller_control(Proxy(self.r,{ref['record']:q}),self.b,self.proof['fullscreen'])
 def test_unreplayed_failure_not_repaired_witness(self):
  q=copy.deepcopy(self.proof);q['callers']['repaired_natural_witness']=True
  with self.assertRaises(ValueError):gate.validate_proof(q)
 def test_main_five_plus_null(self):
  self.assertTrue(any(r[56]==6 and all(r[57:62])and r[62]==0 for e,r in self.pwire['pairs']if e[2]==2))


def basis_test(change):
 def test(self):
  p=copy.deepcopy(self.parent);change(p)
  with self.assertRaises((ValueError,KeyError)):gate.basis(self.plan,p,self.b['parent'])
 return test
for name,change in [
 ('missing_row',lambda p:p['requirements'].pop()),('duplicate_row',lambda p:p['requirements'].__setitem__(1,p['requirements'][0])),
 ('changed_counts',lambda p:p.update(passed_requirements=156)),('final_player_verdict',lambda p:p['decisions'].update({'supported-player':'passed'})),
 ('whole_player',lambda p:p.update(current_whole_player_proved=True)),('combined_player',lambda p:p.update(current_combined_player_proved=True)),
 ('failure_retirement',lambda p:p.update(historical_failures_retired=True)),('scope_amendment',lambda p:p['scope_amendments'].clear()),
 ('accepted_leaf_lost',lambda p:p['requirements'][0].update(admitted_execution_evidence=[])),
 ('boundary_owner',lambda p:next(v for v in p['requirements']if v['id']==gate.GATE).update(owner_task='EP11'))]:setattr(Controls,'test_basis_'+name,basis_test(change))

def policy_test(path,value):
 def test(self):
  p=copy.deepcopy(self.policy);node=p
  for key in path[:-1]:node=node[key]
  node[path[-1]]=value
  with self.assertRaises(ValueError):gate.policy_review(p,self.plan)
 return test
for name,path,value in [
 ('arbitrary_inputs',['arbitrary_maps_saves_duration_supported'],True),('all_assets',['all_installed_assets_supported'],True),
 ('waived_technical_unknowns',['technical_unknowns_waived'],True),('lost_journey',['journeys'],13),('lost_operation',['operations'],20),
 ('lost_defeat',['supported_outcomes'],['victory','actual-later-map']),('padding',['configuration','padding'],[1,0]),
 ('alternate_owner',['configuration','DEBUG_UseMallocedBackBuffer'],True),('exclusive',['configuration','original_exclusive_fullscreen'],True),
 ('live555',['formats','live_RGB555_supported'],True),('555_unreachable',['formats','global_RGB555_unreachable'],True),
 ('hardware',['platform','hardware_renderer_proved'],True),('Win98',['platform','Win98_ME_proved'],True),
 ('SEH',['platform','Windows_SEH_dispatch_proved'],True),('supplier_erased',['supplier_boundary','original_helpers'],False),
 ('all_lifetimes',['supplier_boundary','all_exceptional_lifetimes_recovered'],True),
 ('routes_dropped',['natural_callers','installed_routes'],38),('natural_overlap',['natural_callers','natural_overlap_witness'],True),
 ('unreachable',['natural_callers','global_unreachable_proved'],True),('guard_disabled',['natural_callers','unknown_callers_fail_closed'],False)]:setattr(Controls,'test_policy_'+name,policy_test(path,value))

def storage_test(index,value):
 def test(self):
  d=copy.deepcopy(self.storage);row=list(d['worlds'][0]);row[index]=value;d['worlds'][0]=tuple(row)
  with self.assertRaises(ValueError):gate.storage_facts(d)
 return test
for name,index,value in [('alternate',24,1),('source',25,1),('pad_x',26,1),('pad_y',27,1),('enabled',28,1),('selector',29,1),('masks',30,0),('backing',34,0),('stride',35,0)]:setattr(Controls,'test_storage_'+name,storage_test(index,value))

def provider_test(tag,index,value):
 def test(self):
  d=copy.deepcopy(self.pwire);pairs=[];changed=False
  for e,r in d['pairs']:
   if e[2]==tag and not changed:r=list(r);r[index]=value;r=tuple(r);changed=True
   pairs.append((e,r))
  self.assertTrue(changed);d['pairs']=pairs
  with self.assertRaises(ValueError):module('check-required-provider-session').provider_lifetimes(d)
 return test
for name,tag,index,value in [('main_array',2,56,0),('main_callback_owner',2,83,0),('main_sixth_live',2,62,1),('main_release',4,52,1),('report_array',10,56,0),('report_callback',10,87,0),('report_release',12,54,1)]:setattr(Controls,'test_provider_'+name,provider_test(tag,index,value))

def fullscreen_test(change):
 def test(self):
  a=copy.deepcopy(self.a);change(a)
  with self.assertRaises(ValueError):gate.fullscreen_inputs(a,self.c,self.input)
 return test
for name,change in [('missing_transition',lambda a:a['required_boundaries']['transitions'].pop()),('missing_rebuild',lambda a:a['required_boundaries']['rebuilds'].pop()),('wrong_size',lambda a:a['required_boundaries']['transitions'][0].update(size=[640,480])),('unfocused',lambda a:a['required_boundaries']['transitions'][0]['geometry_after']['focus'].update(host_focused=False)),('nonce_changed',lambda a:a['required_boundaries']['rebuilds'][0]['original_after']['frame'].update(nonce='0')),('no_progress',lambda a:a['required_boundaries']['rebuilds'][0]['original_after']['frame'].update(generation=0))]:setattr(Controls,'test_fullscreen_'+name,fullscreen_test(change))


# These closures use actual rows with corrupted fresh-input observations.
def input_test(field,value):
 def test(self):
  rows=copy.deepcopy(self.input)
  for row in rows:
   if field=='health'and row.get('hit'):row['hit']['health']=value
   elif field=='poll'and row.get('cursor'):row['cursor']['poll']=value
   elif field=='input_sequence'and row.get('cursor'):row['cursor']['input_sequence']=value
  with self.assertRaises(ValueError):gate.fullscreen_inputs(self.a,self.c,rows)
 return test
for name,field in [('dead_hover','health'),('old_cursor','poll'),('old_input','input_sequence')]:setattr(Controls,'test_input_'+name,input_test(field,0))


def counter_test(index,value):
 def test(self):
  import struct
  raw=artifact(self.r,self.a,'native-pixel-producers.bin');words=list(struct.unpack('<580I',raw));words[index]=value
  with self.assertRaises(ValueError):gate.producer_routes(struct.pack('<580I',*words))
 return test
for name,index,value in [('version',1,1),('missing_route',4,0),('duplicate_install',4,2),('forwarded',6,1),('unexpected_route',4+31*3,1)]:setattr(Controls,'test_counter_'+name,counter_test(index,value))


def exit_test(tag,index,value,returned):
 def test(self):
  d=copy.deepcopy(self.pwire);pairs=[];changed=False
  for e,r in d['pairs']:
   if e[2]==tag and not changed:
    if returned:r=list(r);r[index]=value;r=tuple(r)
    else:e=list(e);e[index]=value;e=tuple(e)
    changed=True
   pairs.append((e,r))
  self.assertTrue(changed);d['pairs']=pairs
  with self.assertRaises(ValueError):gate.provider_exits(d)
 return test
for name,tag,index,value,returned in [('bitmap_absent',10,92,0,True),('bitmap_survives',13,92,1,True),('interface_survives',13,91,1,True),('report_initialized',13,9,1,True),('report_wrong_owner',13,6,1,True),('region_never_populated',20,40,0,True),('region_bitmap_survives',22,40,1,True),('region_callback_survives',22,54,1,True)]:setattr(Controls,'test_exit_'+name,exit_test(tag,index,value,returned))

if __name__=='__main__':unittest.main()
