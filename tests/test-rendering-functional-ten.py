#!/usr/bin/env python3
"""Adverse admission checks use the actual retained outputs and public runs."""
from pathlib import Path
import copy,importlib.util,io,json,struct,sys,tarfile,unittest
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools'))
from coverage_evidence import Reader
s=importlib.util.spec_from_file_location('functional',ROOT/'tools/close-rendering-functional-ten.py');m=importlib.util.module_from_spec(s);s.loader.exec_module(m)

class FunctionalChecks(unittest.TestCase):
 @classmethod
 def setUpClass(cls):
  with Reader(ROOT) as r:
   cls.partial=r.json(m.PREFIX+'partial-analysis-01.json')['cases']
   native=r.json(m.PREFIX+'native-partial-controls-04.json');cls.native={}
   with tarfile.open(fileobj=io.BytesIO(r.read_bytes(native['raw_record'])),mode='r:gz') as archive:
    for case in m.module('test-native-gpu-copy-plan').cpu_cases():
     def data(name):return archive.extractfile('controls/'+case['name']+'/'+name).read()
     cls.native[case['name']]=(case,data('output.bin'),data('native.bin'))
   cls.acquired=[r.json(m.PREFIX+n) for n in ('default-results-acquisition-03.json','software-results-acquisition-01.json')]
   cls.returns=[m.results_case(r,m.PREFIX+n) for n in ('default-results-acquisition-03.json','software-results-acquisition-01.json')]
   cls.callbacks=[v['proof']['original_callbacks'] for v in cls.returns]
   cls.parent=r.json('research/runtime/rendering-movie-skip-results-20261106/closeout-analysis-01.json');cls.plan=r.json('docs/native-rendering-closure.json')
 def damaged(self,name,word=0):
  case,raw,native=self.native[name];changed=bytearray(native);changed[word*2]^=1
  with self.assertRaises(ValueError):m.native_case_expected(case,raw,bytes(changed))
 def test_all_eight_natural_partial_domains(self):
  self.assertEqual(len(self.partial),8)
  self.assertEqual(len({(v['domain'][0],v['scenario'],tuple(v['domain'][1])) for v in self.partial}),8)
  for c in self.partial:
   self.assertEqual(len({v['generation'] for v in c['witnesses']}),3)
   for v in c['witnesses']:
    self.assertGreater(v['changed_words'],0);self.assertGreater(v['preserved_nonzero_words'],0)
    self.assertEqual(v['before_exterior_sha256'],v['after_exterior_sha256']);self.assertEqual(v['actual_sha256'],v['expected_sha256'])
 def test_all_actual_native_outputs_have_independent_words(self):
  self.assertEqual(len(self.native),15)
  for case,raw,native in self.native.values():self.assertTrue(m.native_case_expected(case,raw,native)['verified'])
 def test_changed_exterior_word_refuses(self):self.damaged('clip-single')
 def test_keyed_skipped_word_refuses(self):self.damaged('keyed',5)
 def test_empty_clip_write_refuses(self):self.damaged('clip-empty')
 def test_first_piece_refusal_write_refuses(self):self.damaged('clip-first-source-failure')
 def test_late_failure_keeps_valid_prefix(self):
  case,raw,native=self.native['clip-late-source-failure'];p=m.native_case_expected(case,raw,native)
  self.assertEqual((p['result'],p['pieces']),(0x88760096,1));self.assertGreater(p['written_words'],0);self.assertGreater(p['preserved_words'],0)
 def test_dropping_committed_prefix_refuses(self):
  case,raw,native=self.native['clip-late-source-failure'];raw=bytearray(raw);struct.pack_into('<I',raw,24,0);struct.pack_into('<I',raw,56,0)
  with self.assertRaises(ValueError):m.native_case_expected(case,bytes(raw),native)
 def test_wrong_prefix_failure_result_refuses(self):
  case,raw,native=self.native['clip-late-source-failure'];raw=bytearray(raw);struct.pack_into('<I',raw,20,0)
  with self.assertRaises(ValueError):m.native_case_expected(case,bytes(raw),native)
 def test_zero_plan_ordinary_copy_still_writes(self):
  case,raw,native=self.native['disjoint'];p=m.native_case_expected(case,raw,native)
  self.assertEqual(p['pieces'],0);self.assertEqual(p['effective_pieces'],1);self.assertEqual(p['written_words'],117)
 def test_both_actual_public_returns(self):
  self.assertEqual({a['configuration']['software_flag'] for a in self.acquired},{False,True})
  for a,c in zip(self.acquired,self.callbacks):self.assertTrue(m.assess_return(a,c)['verified'])
 def test_results_ack_gap_refuses(self):
  a=copy.deepcopy(self.acquired[0]);a['states'][9]['ack']-=1
  with self.assertRaises(ValueError):m.assess_return(a,self.callbacks[0])
 def test_results_handoff_loss_refuses(self):
  a=copy.deepcopy(self.acquired[0]);next(v for v in a['states'] if v['screen']==26)['handoff']=0
  with self.assertRaises(ValueError):m.assess_return(a,self.callbacks[0])
 def test_forced_stop_is_not_normal_completion(self):
  a=copy.deepcopy(self.acquired[0]);a['forced_private_prefix_stop']=True
  with self.assertRaises(ValueError):m.assess_return(a,self.callbacks[0])
 def test_surviving_graphics_owner_refuses(self):
  a=copy.deepcopy(self.acquired[0]);a['retirement']['graphics_retirement']['counts']['surfaces']=1
  with self.assertRaises(ValueError):m.assess_return(a,self.callbacks[0])
 def test_abort_cannot_become_natural_victory(self):
  a=copy.deepcopy(self.acquired[0]);a['proof']['natural_victory_defeat_proved']=True
  with self.assertRaises(ValueError):m.assess_return(a,self.callbacks[0])
 def test_missing_profile_does_not_close(self):
  with self.assertRaises(ValueError):m.close_rows(self.plan,self.parent,self.partial,self.returns[:1])
 def test_missing_partial_domain_does_not_close(self):
  with self.assertRaises(ValueError):m.close_rows(self.plan,self.parent,self.partial[:-1],self.returns)
 def test_four_new_rows_and_every_parent_preserved(self):
  rows=m.close_rows(self.plan,self.parent,self.partial,self.returns);self.assertEqual(sum(v['state']=='passed' for v in rows),56)
  for old,new in zip(self.parent['requirements'],rows):
   if old['id'] not in ('operation.partial-write','journey.battle-results.return-to-menu'):self.assertEqual(old,new)

if __name__=='__main__':unittest.main()
