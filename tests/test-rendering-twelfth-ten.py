#!/usr/bin/env python3
"""Adversarial diagnostic identity, histogram and source-prefix controls."""
import copy,importlib.util,math,struct,sys,tempfile,unittest
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools'))
from profile_native_resident_helpers import completed_prefix_worlds,distribution,summarize
from drawing_physical_generation import validate_capture

def module(name,path):
 spec=importlib.util.spec_from_file_location(name,ROOT/path);m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);return m

def prefix():
 return dict(sources_unchanged=True,private_wine_stopped=True,original_manifest_verified_after=True,resident_world=dict(completed='64',imports='64',verified_returns='64',world_readbacks=0,native_readbacks=0,rgba_readbacks=0,world_pixel_reply_bytes=0,stage_profile=dict(samples_dropped=0,gpu_queries_dropped=0,capture_truncated=False,samples=[dict(generation=str(i+1),producer_return_ms=0) for i in range(64)],capture=dict(accepted_worlds=65,written_worlds=64,truncated=False,error=''))))

class Completion(unittest.TestCase):
 def test_native_outcome_reader_keeps_complete_pretty_document(self):
  from rendering_twelfth_controls import last_json
  self.assertEqual(last_json('driver note\n{"progress":1}\n'+__import__('json').dumps(dict(success=True,cases=[dict(result=1)]),indent=2)+'\ntrailing driver note\n'),dict(success=True,cases=[dict(result=1)]))
  with self.assertRaises(ValueError):last_json('driver error: {bad}\n')
 def test_completed_prefix_uses_exact_integer_counters(self):
  self.assertEqual(completed_prefix_worlds(prefix()),64)
  for value in (64.9,64.0,True,' 64','+64','64.0','064','６４'):
   p=prefix();p['resident_world']['completed']=value
   with self.subTest(value=value),self.assertRaises(ValueError):completed_prefix_worlds(p)
 def test_completed_prefix_rejects_coerced_generation_and_timing(self):
  for field,value in [('generation',1.9),('producer_return_ms',float('nan')),('producer_return_ms',float('inf')),('producer_return_ms',-1),('producer_return_ms',True)]:
   p=prefix();p['resident_world']['stage_profile']['samples'][0][field]=value
   with self.subTest(field=field,value=value),self.assertRaises(ValueError):completed_prefix_worlds(p)
 def test_capture_counts_remain_balanced_and_bounded(self):
  for accepted,written in [(63,64),(65,66),(8193,64),(65,64.5)]:
   p=prefix();p['resident_world']['stage_profile']['capture'].update(accepted_worlds=accepted,written_worlds=written)
   with self.subTest(accepted=accepted,written=written),self.assertRaises(ValueError):completed_prefix_worlds(p)
 def test_distribution_keeps_negative_residual_but_refuses_nonfinite(self):
  self.assertEqual(distribution([-2,0,2])['mean'],0)
  for value in (float('nan'),float('inf'),True,'1',None):
   with self.subTest(value=value),self.assertRaises(ValueError):distribution([0,value])
 def test_summary_generation_denominator_is_exact_and_ordered(self):
  row=dict(generation='64',decode_ms=1,handoff_ms=2,native_consume_ms=3)
  self.assertEqual(summarize([row])['native_decode_handoff_consume_ms']['mean'],6)
  for rows in ([dict(row,generation=64.9)],[row,row],[dict(row,generation='65'),row]):
   with self.subTest(rows=rows),self.assertRaises(ValueError):summarize(rows)
 def test_physical_marker_requires_opaque_integer_receipt(self):
  m=module('twelfth_physical','tests/test-drawing-physical-generation.py');item,body,marker=m.sample();item['capture_height']=35.0
  with self.assertRaises(ValueError):validate_capture(item,body,marker,4,3,3)
  item,body,marker=m.sample();marker=marker[:3]+b'\0'+marker[4:]
  import hashlib
  item['marker_sha256']=hashlib.sha256(marker).hexdigest();item['capture_sha256']=hashlib.sha256(body+marker).hexdigest()
  with self.assertRaises(ValueError):validate_capture(item,body,marker,4,3,3)
 def test_checkpoint_typed_identity_and_denominator(self):
  m=module('twelfth_hashes','tools/compare-drawing-copy-phase-hashes.py');old=dict(ordinal=1,sequence=1,canvas=2,width=4,height=3,original_sha256='a'*64);new=dict(old,sha256='a'*64)
  self.assertTrue(m.compare_checkpoints([old],[new])[0]['full_plane_hash_matches'])
  for key,value in [('ordinal',True),('sequence',1.0),('canvas',0),('width',1.5),('height',2049),('ordinal',2**32)]:
   for original in (False,True):
    with self.subTest(key=key,value=value,original=original),self.assertRaises(ValueError):m.compare_checkpoints([dict(old,**{key:value})] if original else [old],[new] if original else [dict(new,**{key:value})])
  for digest in (None,True,'?', 'A'*64):
   with self.subTest(digest=digest),self.assertRaises(ValueError):m.compare_checkpoints([old],[dict(new,sha256=digest)])
 def test_scene_first_difference_must_be_a_real_critical_byte(self):
  m=module('twelfth_scene','tests/test-scene-production-decoder.py');data,_=m.fixture()
  with tempfile.TemporaryDirectory() as d:
   p=Path(d)/'trace.bin'
   for field,value in [(25,99),(26,1),(27,256)]:
    bad=bytearray(data);struct.pack_into('<I',bad,64+field*4,value);p.write_bytes(bad)
    with self.subTest(field=field),self.assertRaises(ValueError):m.decode(p)
 def test_scene_histogram_matches_sum_and_maximum(self):
  m=module('twelfth_budget','tests/test-scene-production-decoder.py');data,_=m.fixture(6)
  with tempfile.TemporaryDirectory() as d:
   p=Path(d)/'trace.bin'
   for field,value in [(19,59999),(21,49),(24,1)]:
    bad=bytearray(data);struct.pack_into('<I',bad,64+field*4,value);p.write_bytes(bad)
    with self.subTest(field=field),self.assertRaises(ValueError):m.decode(p)
   # Saturated bin127 represents long intervals; it must retain the actual max.
   valid=bytearray(data)
   for field,value in [(18,2),(19,1050),(20,2),(21,1000),(24+50,1),(24+127,1)]:struct.pack_into('<I',valid,64+field*4,value)
   p.write_bytes(valid);self.assertTrue(m.summarize(p)['protected_state_invariance'])
   struct.pack_into('<I',valid,64+19*4,1049);p.write_bytes(valid)
   with self.assertRaises(ValueError):m.decode(p)

if __name__=='__main__':unittest.main()
