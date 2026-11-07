#!/usr/bin/env python3
import copy,importlib.util,os,sys,unittest
from pathlib import Path
import numpy as np
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools'))
from coverage_evidence import Reader
s=importlib.util.spec_from_file_location('operations',ROOT/'tools/close-rendering-operations.py');m=importlib.util.module_from_spec(s);s.loader.exec_module(m)
class Checks(unittest.TestCase):
 @classmethod
 def setUpClass(cls):
  with Reader(ROOT) as r:
   cls.q=r.json(os.environ.get('MNM_OPERATIONS_RESULT',m.PREFIX+'closeout-analysis-final-01.json'));assert cls.q['success'];cls.parent=r.json(cls.q['parent']['record']);cls.plan=r.json('docs/native-rendering-closure.json');cls.c=cls.q['cases'][0];cls.w=cls.c['witnesses'][0]
   a=r.json(cls.c['pair']['acquisition']['record']);v=next(v for v in a['artifacts'] if '/capture/' in v['path'] and Path(v['path']).name.startswith('commands') and v['path'].endswith('.bin'))
   cls.actual_reply=next(body for row,body in m.capture(r.read_bytes(v['record'])) if row['kind']=='cpu-read')
   cls.reply_expected=np.frombuffer(cls.actual_reply[64:],dtype='<u2').copy().reshape(-1,1)
 def bad(self,change):
  c=copy.deepcopy(self.c);change(c)
  with self.assertRaises(ValueError):m.check_case(c)
 def damage(self,change):
  w=copy.deepcopy(self.w);change(w)
  with self.assertRaises(ValueError):m.check_witness(w)
 def test_all_actual_cases(self):
  for c in self.q['cases']:m.check_case(c)
 def test_actual_reply_denominator(self):self.assertEqual(sum(len(c['reads']) for c in self.q['cases']),212)
 def test_actual_ordered_writer_denominator(self):self.assertEqual(sum(len(c['witnesses']) for c in self.q['cases']),12)
 def test_missing_domain_refuses(self):
  with self.assertRaises(ValueError):m.close_rows(self.plan,self.parent,self.q['cases'][:-1])
 def test_duplicate_domain_refuses(self):
  c=copy.deepcopy(self.q['cases']);c[0]=c[1]
  with self.assertRaises(ValueError):m.close_rows(self.plan,self.parent,c)
 def test_repeated_identity_refuses(self):
  c=copy.deepcopy(self.q['cases']);c[0]['identity']=c[1]['identity']
  with self.assertRaises(ValueError):m.close_rows(self.plan,self.parent,c)
 def test_missing_read_verification(self):self.bad(lambda c:c['reads'][0].update(verified=False))
 def test_reply_before_resource_creation(self):self.bad(lambda c:c['reads'][0].update(created=c['reads'][0]['sequence']))
 def test_duplicate_reply_ticket(self):self.bad(lambda c:c['reads'][0].update(ticket=c['reads'][1]['ticket']))
 def test_zero_source_generation(self):self.bad(lambda c:c['reads'][0].update(world_generation=0))
 def test_repeated_reply_cannot_supply_changes(self):self.bad(lambda c:[r.update(sha256='0'*64) for r in c['reads']])
 def test_no_masked_cpu_writes(self):self.bad(lambda c:c.update(masked_updates=0))
 def test_missing_writer_witness(self):self.bad(lambda c:c['witnesses'].pop())
 def test_write_must_precede_gpu_sink(self):self.damage(lambda w:w.update(sink_sequence=w['write_sequence']))
 def test_actual_earlier_reply_required(self):self.damage(lambda w:w.update(earlier_reply_ticket=0))
 def test_writer_resource_lifetime_required(self):self.damage(lambda w:w.update(source_created=w['write_sequence']))
 def test_gpu_source_resource_lifetime_required(self):self.damage(lambda w:w.update(observed_created=w['sink_sequence']))
 def test_changed_writer_words_required(self):self.damage(lambda w:w.update(changed_words=0))
 def test_retained_history_required(self):self.damage(lambda w:w.update(equal_history_words=0))
 def test_nonzero_retained_history_required(self):self.damage(lambda w:w.update(equal_nonzero_history_words=0))
 def test_retained_history_hash_required(self):self.damage(lambda w:w.update(equal_after_sha256='0'*64))
 def test_writer_must_reach_actual_gpu_source(self):self.damage(lambda w:w.update(surviving_written_words=0))
 def test_actual_word_mismatch_refuses(self):self.damage(lambda w:w.update(actual_source_sha256='0'*64))
 def test_complete_gpu_source_verification_required(self):self.damage(lambda w:w.update(full_gpu_source_verified=False))
 def test_repeated_writer_refuses(self):self.bad(lambda c:c['witnesses'].__setitem__(0,c['witnesses'][1]))
 def test_wrong_actual_reply_word(self):
  b=bytearray(self.actual_reply);b[-1]^=1
  with self.assertRaises(ValueError):m.reply_pixels(self.reply_expected,bytes(b),self.actual_reply[:64])
 def test_wrong_actual_reply_header(self):
  b=bytearray(self.actual_reply);b[8]^=1
  with self.assertRaises(ValueError):m.reply_pixels(self.reply_expected,bytes(b),self.actual_reply[:64])
 def test_truncated_actual_reply(self):
  with self.assertRaises(ValueError):m.reply_pixels(self.reply_expected,self.actual_reply[:-2],self.actual_reply[:64])
 def test_cpu_upload_outside_storage(self):
  with self.assertRaises(ValueError):m.expected_write(np.zeros((4,4),dtype='uint16'),np.ones((2,2),dtype='uint16'),np.ones((2,2),dtype=bool),3,3)
 def test_complete_gpu_destination_required(self):self.damage(lambda w:w.update(full_gpu_destination_verified=False))
 def test_failed_copy_caller_refuses(self):self.damage(lambda w:w['copy_caller'].__setitem__(13,0x88760096))
 def test_repeated_gpu_source_refuses(self):self.bad(lambda c:c['witnesses'][0].update(actual_source_sha256=c['witnesses'][1]['actual_source_sha256'],expected_source_sha256=c['witnesses'][1]['expected_source_sha256']))
 def test_counter_mismatch_refuses(self):self.bad(lambda c:c['wire_counts'].update(completed=0))
 def test_unknown_cpu_writer_refuses(self):self.bad(lambda c:c['witnesses'][0].update(upload_kind='unknown'))
 def test_only_two_rows_change(self):
  rows=m.close_rows(self.plan,self.parent,self.q['cases']);self.assertEqual(rows,self.q['requirements']);changes=[(a['configuration'],a['id']) for a,b in zip(rows,self.parent['requirements']) if a!=b];self.assertEqual(set(changes),{(p,m.LEAF) for p in ['player-default','player-software']});self.assertEqual(self.q['decisions'],self.parent['decisions'])
if __name__=='__main__':unittest.main()
