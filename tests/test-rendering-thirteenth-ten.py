#!/usr/bin/env python3
"""Adversarial identity, owned-plane and exception-record regression controls."""
import copy,hashlib,importlib.util,struct,sys,unittest
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools'))
from drawing_comparison import index,plane
from drawing_held_egl_candidates import candidates

def load(name,path):
 spec=importlib.util.spec_from_file_location(name,ROOT/path);m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);return m

class Completion(unittest.TestCase):
 def test_terrain_presentation_policy_is_explicit_and_preserves_historical_floor(self):
  m=load('thirteenth_terrain_conversion','tools/test-terrain-preview.py');word=(4<<11)|(16<<5)|4
  self.assertEqual(m.rgba_words([word],'normalized-floor'),bytes([32,64,32,255]))
  self.assertEqual(m.rgba_words([word],'bit-replicated'),bytes([33,65,33,255]))
  with self.assertRaises(ValueError):m.rgba_words([word],'round-to-match')
 def test_legacy_held_report_defaults_are_v1_only_and_keep_every_other_field(self):
  from drawing_comparison import same_held_observation
  old=dict(schema=1,frames=[dict(sha256='a'*64,thread=7)])
  current=dict(schema=1,wire_version=1,frames=[dict(sha256='a'*64,thread=7,pending_admission=0)])
  self.assertTrue(same_held_observation(current,old));self.assertNotIn('pending_admission',old['frames'][0])
  for mutate in [lambda x:x.update(wire_version=2),lambda x:x['frames'][0].update(pending_admission=1),lambda x:x['frames'][0].update(sha256='b'*64),lambda x:x.update(unknown=True)]:
   changed=copy.deepcopy(current);mutate(changed);self.assertFalse(same_held_observation(changed,old))
 def test_identity_map_refuses_ambiguous_and_coerced_tokens(self):
  self.assertEqual(index([dict(id=1)],'id','fixture'),{1:dict(id=1)})
  for rows in ([dict(id=1),dict(id=1)],[dict(id=True)],[dict(id=1.0)],[dict(id=0)],[dict(id=2**32)]):
   with self.subTest(rows=rows),self.assertRaises(ValueError):index(rows,'id','fixture')
 def test_complete_plane_keeps_all_pixels_and_hash(self):
  data=struct.pack('<6H',0,1,0xffff,3,4,5);digest=hashlib.sha256(data).hexdigest()
  self.assertEqual(plane(data,3,2,digest),digest)
  for blob,w,h,sha in [(data[:-1],3,2,digest),(data,3.0,2,digest),(data,True,6,digest),(data,3,2,'0'*64),(data,3,2,True),(data,0,2,digest)]:
   with self.subTest(w=w,h=h,sha=sha),self.assertRaises(ValueError):plane(blob,w,h,sha)
 def test_both_prefix_readers_refuse_lossy_or_missing_identities(self):
  for i,name in enumerate(['compare-drawing-source-points.py','compare-drawing-held-egl.py']):
   m=load('thirteenth_prefix'+str(i),'tools/'+name);raw=bytes(64)+struct.pack('<I',96)+bytes(92)
   self.assertEqual(m.wire_prefix(raw,1),raw)
   for value in (0,-1,True,1.0,2,4000001):
    with self.subTest(name=name,value=value),self.assertRaises(ValueError):m.wire_prefix(raw,value)
 def test_source_cohort_keeps_missing_phase_and_unknown_prefix_as_exclusions(self):
  f=load('thirteenth_prediction_fixture','tests/test-drawing-source-prediction.py');m=f.m
  for points,reason in [([], 'incomplete-source-point-cohort'),([1,1,3],'incomplete-source-point-cohort'),([1,2,3],'unknown-source-prefix')]:
   observation,api,records=f.fixture()
   if not points:observation['frames'][0]['points']=[]
   else:
    for p,v in zip(observation['frames'][0]['points'],points):p['phase']=v;p['source'][3]=99
   selected,excluded=m.candidates(observation,api,records);self.assertFalse(selected);self.assertEqual(excluded[0]['reason'],reason)
 def test_source_duplicate_ordinal_is_not_silently_counted(self):
  f=load('thirteenth_prediction_duplicate','tests/test-drawing-source-prediction.py');o,a,r=f.fixture();o['frames'].append(copy.deepcopy(o['frames'][0]))
  with self.assertRaises(ValueError):f.m.candidates(o,a,r)
 def test_held_cohort_refuses_duplicate_binding_in_every_table(self):
  f=load('thirteenth_held_fixture','tests/test-drawing-held-egl-candidates.py')
  for section,key in [('api_source_worker_lease_observation','calls'),('wine_blit_packet_observation','bindings'),('wine_blit_worker_observation','calls'),('wine_submission_observation','calls'),('egl_physical_observation','frames'),('source_held_snapshot_observation','frames')]:
   x=f.reference();x[section][key].append(copy.deepcopy(x[section][key][0]))
   with self.subTest(section=section),self.assertRaises(ValueError):candidates(x,f.records())
 def test_held_post_snapshot_must_have_the_same_lease_owner(self):
  f=load('thirteenth_held_owner','tests/test-drawing-held-egl-candidates.py');x=f.reference();x['source_held_snapshot_observation']['frames'][1]['thread']=99
  selected,excluded=candidates(x,f.records());self.assertFalse(selected);self.assertEqual(excluded[0]['reason'],'changed-held-source-version')
 def test_ne_segment_ordinal_is_an_exact_integer(self):
  f=load('thirteenth_ne_fixture','tests/test-legacy-ne-fault.py');raw=f.NeFaultTest().image()
  for value in (True,1.0,'2'):
   with self.subTest(value=value),self.assertRaises(ValueError):f.fault.ne_segment(raw,value)
 def test_exception_fields_cannot_cross_same_thread_exception_boundaries(self):
  f=load('thirteenth_ne_context','tools/inspect-legacy-dibeng-fault.py')
  prefix='00ec:trace:seh:dispatch_exception '
  log=prefix+'eip=000000ce\n'+prefix+'eax=0000000b ebx=000003df\n'+prefix+'eip=000000de cs=0327\n'+prefix+'eax=0000000c ebx=000004df\n'+prefix+'cs=0327\n'
  self.assertEqual(f.exception_contexts(log),[dict(thread='00ec',ip=222,ax=12,bx=1247,cs=807)])
 def test_terminal_ne_observer_requires_matching_code_selector_and_complete_query(self):
  f=load('thirteenth_ne_terminal','tools/inspect-legacy-dibeng-fault.py');prefix='00ec:trace:seh:dispatch_exception '
  log=prefix+'eip=7bcb6d6d\n'+prefix+'eax=ffffffff ebx=01262f60\n'+prefix+'cs=0023\n'
  row=[88,0xc0000005,0x7bcb6d6d,1,0x1262f64,0x23];query=[89,28,0x11f0000,0x1000,2,0x40000]
  self.assertIsNotNone(f.continuation_diagnostic(log,dict(readonly_loader_exception_observations=[row,query]),[])['readonly_observer_match'])
  for rows in ([row[:-1]+[0x27],query],[row,[89]]):self.assertIsNone(f.continuation_diagnostic(log,dict(readonly_loader_exception_observations=rows),[])['readonly_observer_match'])
  self.assertIsNone(f.continuation_diagnostic(log.replace('7bcb6d6d','000000ce'),{},[])['later_32bit_exception'])

if __name__=='__main__':unittest.main()
