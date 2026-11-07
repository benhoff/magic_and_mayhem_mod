import copy
import importlib.util
import os
from pathlib import Path
import sys
import unittest

ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools'))
from coverage_evidence import Reader
s=importlib.util.spec_from_file_location('full_fill',ROOT/'tools/close-rendering-full-fill.py')
m=importlib.util.module_from_spec(s);s.loader.exec_module(m)

class Checks(unittest.TestCase):
 @classmethod
 def setUpClass(cls):
  with Reader(ROOT) as r:
   cls.q=r.json(os.environ.get('MNM_FULL_FILL_RESULT',m.PREFIX+'closeout-analysis-02.json'))
   assert cls.q['success'];cls.parent=r.json(cls.q['parent']['record']);cls.plan=r.json('docs/native-rendering-closure.json')
   cls.c=cls.q['cases'][0];cls.w=cls.c['witnesses'][0]
   a=r.json(cls.c['pair']['acquisition']['record']);ref=next(v for v in a['artifacts'] if '/capture/' in v['path'] and Path(v['path']).name.startswith('commands') and v['path'].endswith('.bin'))
   raw=b''.join(b for h,b in m.capture(r.read_bytes(ref['record'])) if h['kind']=='fragment')
   cls.update=next(p for op,seq,p in m.commands(raw) if seq==cls.w['update_sequence'])
 def bad(self,change):
  w=copy.deepcopy(self.w);change(w)
  with self.assertRaises(ValueError):m.check_witness(w)
 def bad_case(self,change):
  c=copy.deepcopy(self.c);change(c)
  with self.assertRaises(ValueError):m.check_case(c)
 def test_all_actual_domains(self):
  for c in self.q['cases']:m.check_case(c)
  self.assertEqual(sum(len(c['witnesses']) for c in self.q['cases']),12)
 def test_actual_update_argument_color(self):
  m.constant_update(self.update,*self.w['source']['fields'][5:7],self.w['value'])
 def test_actual_update_word_corruption(self):
  b=bytearray(self.update);b[-1]^=1
  with self.assertRaises(ValueError):m.constant_update(bytes(b),*self.w['source']['fields'][5:7],self.w['value'])
 def test_truncated_actual_update(self):
  with self.assertRaises(ValueError):m.constant_update(self.update[:-2],*self.w['source']['fields'][5:7],self.w['value'])
 def test_changed_color_argument(self):
  with self.assertRaises(ValueError):m.constant_update(self.update,*self.w['source']['fields'][5:7],self.w['value']^1)
 def test_wrong_full_extent(self):
  w,h=self.w['source']['fields'][5:7]
  with self.assertRaises(ValueError):m.constant_update(self.update,w-1,h,self.w['value'])
 def test_no_initialization_policy_credit(self):self.bad(lambda w:w['source']['fields'].__setitem__(22,0x4e495a31))
 def test_partial_fill_cannot_supply_full_row(self):self.bad(lambda w:w['source']['fields'].__setitem__(12,w['source']['fields'][12]-1))
 def test_wrong_original_entry(self):self.bad(lambda w:w['caller'].__setitem__(1,0x58bb0d))
 def test_failed_fill_caller(self):self.bad(lambda w:w['caller'].__setitem__(13,0x88760096))
 def test_wrong_source_return(self):self.bad(lambda w:w['source']['fields'].__setitem__(21,1))
 def test_attached_clipper_cannot_supply_plain_fill(self):self.bad(lambda w:w['caller'].__setitem__(10,w['caller'][10]|32))
 def test_unknown_fill_flags(self):self.bad(lambda w:w['caller'].__setitem__(12,0x80000000))
 def test_outside_original_source_caller(self):self.bad(lambda w:w['source']['fields'].__setitem__(22,0xffffffff))
 def test_invocation_after_world_required(self):self.bad(lambda w:w.update(generation=0))
 def test_update_after_creation(self):self.bad(lambda w:w.update(created=w['update_sequence']))
 def test_gpu_use_before_retirement(self):self.bad(lambda w:w.update(deleted=w['sink_sequence']))
 def test_ordered_gpu_sink(self):self.bad(lambda w:w.update(sink_sequence=w['update_sequence']))
 def test_lower_copy_fence(self):self.bad(lambda w:w.update(lower_fence=w['update_sequence']))
 def test_upper_copy_fence(self):self.bad(lambda w:w.update(upper_fence=w['update_sequence']))
 def test_meaningful_changed_output(self):self.bad(lambda w:w.update(changed_words=0))
 def test_surviving_fill_words(self):self.bad(lambda w:w.update(surviving_fill_words=0))
 def test_native_update_verified(self):self.bad(lambda w:w.update(native_update_matches_argument=False))
 def test_gpu_source_verified(self):self.bad(lambda w:w.update(complete_gpu_source_verified=False))
 def test_gpu_destination_verified(self):self.bad(lambda w:w.update(complete_gpu_destination_verified=False))
 def test_gpu_source_hash_matches(self):self.bad(lambda w:w.update(actual_source_sha256='0'*64))
 def test_copy_caller_success(self):self.bad(lambda w:w['copy_caller'].__setitem__(13,0x88760096))
 def test_physical_identity_observation(self):self.bad(lambda w:w.update(physical_identity_observations=0))
 def test_world_must_be_live(self):self.bad(lambda w:w.update(world_surface=0))
 def test_world_must_precede_fill(self):self.bad(lambda w:w.update(world_created=w['update_sequence']))
 def test_world_cannot_retire_before_gpu_output(self):self.bad(lambda w:w.update(world_deleted=w['sink_sequence']))
 def test_same_world_at_gpu_output(self):self.bad(lambda w:w.update(sink_world_surface=w['world_surface']+1))
 def test_same_world_generation_at_gpu_output(self):self.bad(lambda w:w.update(sink_generation=w['generation']+1))
 def test_missing_source_chronology(self):self.bad_case(lambda c:c.update(joined_source_caller_calls=0))
 def test_unpaired_tail_preserved(self):self.bad_case(lambda c:c['unpaired_tail_fills'].pop())
 def test_repeated_witness(self):self.bad_case(lambda c:c['witnesses'].__setitem__(0,c['witnesses'][1]))
 def test_missing_profile_domain(self):
  with self.assertRaises(ValueError):m.close_rows(self.plan,self.parent,self.q['cases'][:-1])
 def test_duplicate_identity(self):
  cases=copy.deepcopy(self.q['cases']);cases[0]['identity']=cases[1]['identity']
  with self.assertRaises(ValueError):m.close_rows(self.plan,self.parent,cases)
 def test_only_two_rows_change(self):
  rows=m.close_rows(self.plan,self.parent,self.q['cases']);self.assertEqual(rows,self.q['requirements'])
  self.assertEqual({(a['configuration'],a['id']) for a,b in zip(rows,self.parent['requirements']) if a!=b},{(p,m.LEAF) for p in ['player-default','player-software']})
  self.assertEqual(self.q['decisions'],self.parent['decisions'])

if __name__=='__main__':unittest.main()
