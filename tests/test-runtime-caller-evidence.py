#!/usr/bin/env python3
"""Evidence payload identities must survive the legacy caller snapshot bridge."""
import hashlib,sys,unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from coverage_evidence import pointer
from broader_runtime_caller_evidence import payload_preimages

def sha(body):return hashlib.sha256(body).hexdigest()
class Payloads(unittest.TestCase):
 def setUp(self):self.body=b'{"original":"payload"}\n';self.capsule=pointer(sha(self.body),len(self.body));self.sources={'research/runtime/source.json':sha(self.body)};self.retained=[]
 def retain(self,value,body):self.retained.append(body);return dict(record='research/runtime/snapshot.bin',sha256=sha(body))
 def run_bind(self,**change):
  args=dict(existing={},sources=self.sources,physical=lambda n:self.capsule,payload=lambda n:self.body,retain=self.retain);args.update(change);return payload_preimages(**args)
 def test_original_payload_is_retained_and_pointer_identity_is_separate(self):
  result,refs=self.run_bind();self.assertEqual(self.retained,[self.body]);self.assertEqual(result[next(iter(self.sources))]['sha256'],sha(self.body));self.assertEqual(refs[next(iter(self.sources))]['physical_reference_sha256'],sha(self.capsule))
 def test_pointer_file_hash_cannot_replace_payload_hash(self):
  with self.assertRaises(ValueError):self.run_bind(sources={next(iter(self.sources)):sha(self.capsule)})
 def test_corrupt_or_truncated_payload_refused(self):
  for body in [self.body[:-1],b'changed']:
   with self.assertRaises(ValueError):self.run_bind(payload=lambda n:body)
 def test_wrong_capsule_size_refused(self):
  with self.assertRaises(ValueError):self.run_bind(physical=lambda n:pointer(sha(self.body),len(self.body)+1))
 def test_malformed_capsule_refused(self):
  with self.assertRaises(ValueError):self.run_bind(physical=lambda n:self.capsule+b'extra')
 def test_literal_source_must_match_and_is_not_duplicated(self):
  result,refs=self.run_bind(physical=lambda n:self.body);self.assertEqual((result,refs),({},{}));self.assertEqual(self.retained,[])
  with self.assertRaises(ValueError):self.run_bind(physical=lambda n:b'wrong')
 def test_tested_native_preimage_cannot_be_rebound(self):
  name=next(iter(self.sources))
  with self.assertRaises(ValueError):self.run_bind(existing={name:dict(sha256='0'*64)})
 def test_retainer_cannot_rewrite_snapshot_identity(self):
  with self.assertRaises(ValueError):self.run_bind(retain=lambda v,b:dict(record='research/runtime/wrong',sha256='0'*64))
 def test_existing_exact_native_preimage_is_kept(self):
  name=next(iter(self.sources));result,refs=self.run_bind(existing={name:dict(sha256=sha(self.body))},physical=lambda n:self.fail('Native preimage was reread as a pointer'));self.assertEqual((result,refs),({},{}))
if __name__=='__main__':unittest.main()
