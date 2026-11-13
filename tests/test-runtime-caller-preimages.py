#!/usr/bin/env python3
"""Reject incomplete, unsafe, rebound or corrupt native caller source roots."""
import copy,hashlib,json,sys,unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from broader_runtime_caller_preimages import resolve

def digest(body):return hashlib.sha256(body).hexdigest()
def fixture():
 bodies={'runtime/example.c':b'original source','renderer/empty.hpp':b''};sources={n:digest(v)for n,v in bodies.items()};manifest=dict(schema=1,sources={n:dict(sha256=digest(v),size=len(v))for n,v in bodies.items()});body=(json.dumps(manifest,sort_keys=True)+'\n').encode();ref=dict(sha256=digest(body),size=len(body));return bodies,sources,manifest,body,ref

class Roots(unittest.TestCase):
 def setUp(self):self.bodies,self.sources,self.manifest,self.body,self.ref=fixture();self.retained=[]
 def read(self,value):return next(v for v in self.bodies.values()if digest(v)==value['sha256'])
 def retain(self,value,body):self.retained.append(body);return dict(record='research/runtime/test/'+value['sha256']+'.bin',sha256=digest(body))
 def run_resolve(self,**overrides):
  args=dict(reference=self.ref,expected_reference=self.ref,body=self.body,expected_sources=self.sources,read=self.read,retain=self.retain);args.update(overrides);return resolve(**args)
 def repack(self):self.body=(json.dumps(self.manifest,sort_keys=True)+'\n').encode();self.ref=dict(sha256=digest(self.body),size=len(self.body))
 def test_all_exact_sources_and_empty_file_are_retained_without_input_mutation(self):
  before=copy.deepcopy((self.ref,self.sources,self.manifest));result=self.run_resolve();self.assertEqual(set(result),set(self.sources));self.assertEqual(sorted(self.retained),sorted(self.bodies.values()));self.assertEqual((self.ref,self.sources,self.manifest),before)
 def test_wrong_build_root_refused(self):
  with self.assertRaises(ValueError):self.run_resolve(expected_reference=dict(sha256='0'*64,size=len(self.body)))
 def test_corrupt_root_bytes_or_size_refused(self):
  with self.assertRaises(ValueError):self.run_resolve(body=self.body+b' ')
  wrong=dict(self.ref,size=len(self.body)+1)
  with self.assertRaises(ValueError):self.run_resolve(reference=wrong,expected_reference=wrong)
 def test_non_object_or_unknown_root_schema_refused(self):
  for value in [[],None,dict(schema=2,sources=self.manifest['sources']),dict(self.manifest,extra=True)]:
   body=json.dumps(value).encode();ref=dict(sha256=digest(body),size=len(body))
   with self.assertRaises(ValueError):self.run_resolve(reference=ref,expected_reference=ref,body=body)
 def test_omitted_or_extra_native_source_refused(self):
  for expected in [dict(list(self.sources.items())[:1]),dict(self.sources,extra='a'*64),{}]:
   with self.assertRaises(ValueError):self.run_resolve(expected_sources=expected)
 def test_original_source_digest_cannot_be_rebound(self):
  self.manifest['sources']['runtime/example.c']['sha256']='f'*64;self.repack()
  with self.assertRaises(ValueError):self.run_resolve()
 def test_truncated_or_changed_source_bytes_refused(self):
  for body in [b'',b'changed']:
   with self.assertRaises(ValueError):self.run_resolve(read=lambda value:body)
 def test_non_integer_negative_or_wrong_member_size_refused(self):
  for size in [True,-1,1.0,999]:
   self.setUp();self.manifest['sources']['runtime/example.c']['size']=size;self.repack()
   with self.assertRaises(ValueError):self.run_resolve()
 def test_unknown_or_non_object_member_refused(self):
  for value in [None,[],dict(self.manifest['sources']['runtime/example.c'],extra=True)]:
   self.setUp();self.manifest['sources']['runtime/example.c']=value;self.repack()
   with self.assertRaises(ValueError):self.run_resolve()
 def test_escaping_original_working_and_noncanonical_paths_refused(self):
  for name in ['../escape','/absolute','original/Chaos.exe','working/source.c','.git/config','runtime/../source.c','runtime//source.c','.']:
   self.setUp();value=self.manifest['sources'].pop('runtime/example.c');self.manifest['sources'][name]=value;self.sources[name]=self.sources.pop('runtime/example.c');self.repack()
   with self.assertRaises(ValueError):self.run_resolve()
 def test_retainer_cannot_change_identity_or_point_outside_research(self):
  for value in [dict(record='working/fake',sha256='0'*64),dict(record='research/runtime/a',sha256='0'*64),dict(record='research/runtime/../fake',sha256='0'*64)]:
   with self.assertRaises(ValueError):self.run_resolve(retain=lambda entry,body:value)
if __name__=='__main__':unittest.main()
