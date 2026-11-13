#!/usr/bin/env python3
"""No broad source waiver follows from byte-compatible journal publication."""
import copy,sys,unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from broader_publication_applicability import reviewed_sources,PUBLISHER
from coverage_claims import fingerprint

class Reader:
 def __init__(self,sources,identity):
  self.hashes={PUBLISHER:'current-publisher','apps/player.cpp':'native-current','review':'review-sha','old.bin':'old-publisher','build':'build-sha','controls':'controls-sha','tools/helper.py':'helper-current'}
  binding=dict(identity,validation_sources_sha256=fingerprint(sources),coordinator_sources_sha256=fingerprint(sources))
  others=[dict(binding,case_id='other-'+str(i))for i in range(2)]
  self.values={'review':dict(success=True,kind='broader_completed_journal_publication_applicability',retained_current_cases=11,case_bindings=[binding,*others],native_files_waived=[],old_source_bytes_preserved=True,all_retained_complete_journals_republish_identically=True,game_response_deadlines_changed=False,sources={'tools/helper.py':'helper-current'},old_publisher=dict(record='old.bin',sha256='old-publisher'),current_publisher_sha256='current-publisher',current_build=dict(record='build',sha256='build-sha'),publication_controls=dict(record='controls',sha256='controls-sha')),'build':dict(success=True,sources_stable=True,compiler_sources={'apps/player.cpp':'native-current'},runtime_sources={}),'controls':dict(success=True,sources_stable=True,publication_controls=dict(tests=8),sources={'tools/helper.py':'helper-current'})}
 def digest(self,n):return self.hashes[n]
 def json(self,n):return copy.deepcopy(self.values[n])
class Applicability(unittest.TestCase):
 def setUp(self):
  self.sources={PUBLISHER:'old-publisher','apps/player.cpp':'native-current'};self.identity=dict(case_id='reviewed',acquisition=dict(record='a',sha256='a-sha'),validation=dict(record='v',sha256='v-sha'));self.reader=Reader(self.sources,self.identity);self.ref=dict(record='review',sha256='review-sha')
 def inspect(self):return reviewed_sources(self.reader,self.sources,self.identity,self.ref,purpose='validation')
 def test_exact_review_returns_new_map_and_keeps_old_hash(self):
  current,receipt=self.inspect();self.assertEqual(current[PUBLISHER],'current-publisher');self.assertEqual(self.sources[PUBLISHER],'old-publisher');self.assertEqual(receipt['native_files_waived'],[])
 def test_native_difference_is_always_refused(self):
  self.reader.hashes['apps/player.cpp']='changed'
  with self.assertRaises(ValueError):self.inspect()
 def test_foreign_case_or_record_is_refused(self):
  for key,value in [('case_id','foreign'),('validation',dict(record='different',sha256='different'))]:
   old=self.identity[key];self.identity[key]=value
   with self.assertRaises(ValueError):self.inspect()
   self.identity[key]=old
 def test_different_complete_source_map_cannot_borrow_receipt(self):
  self.sources.pop('apps/player.cpp')
  with self.assertRaises(ValueError):self.inspect()
 def test_another_publisher_change_is_not_inferred_compatible(self):
  self.reader.hashes[PUBLISHER]='changed-again'
  with self.assertRaises(ValueError):self.inspect()
 def test_changed_control_or_review_source_is_refused(self):
  self.reader.hashes['tools/helper.py']='changed'
  with self.assertRaises(ValueError):self.inspect()
 def test_wrong_purpose_and_refreshed_historical_hash_are_refused(self):
  with self.assertRaises(ValueError):reviewed_sources(self.reader,self.sources,self.identity,self.ref,purpose='native')
  self.sources[PUBLISHER]='different-old'
  with self.assertRaises(ValueError):self.inspect()
 def test_current_map_requires_no_historical_exception(self):
  self.sources[PUBLISHER]='current-publisher';current,review=self.inspect();self.assertIsNone(review);self.assertEqual(current,self.sources);self.assertIsNot(current,self.sources)
if __name__=='__main__':unittest.main()
