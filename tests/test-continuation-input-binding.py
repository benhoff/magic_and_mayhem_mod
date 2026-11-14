#!/usr/bin/env python3
import copy,sys,unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from broader_input_plan_inheritance import resolve,INCOMPLETE,PARENT
class Reader:
 def __init__(self):
  self.fixture=dict(record='fixture',sha256='a'*64);self.data={INCOMPLETE['record']:dict(success=True,previous_plan=copy.deepcopy(PARENT),kind='broader_current_player_drain_continuation_plan'),PARENT['record']:dict(success=True,owned_input=self.fixture),'fixture':dict(success=True)};self.hashes={INCOMPLETE['record']:INCOMPLETE['sha256'],PARENT['record']:PARENT['sha256'],'fixture':'a'*64}
 def digest(self,n):return self.hashes[n]
 def json(self,n):return copy.deepcopy(self.data[n])
class InputBinding(unittest.TestCase):
 def setUp(self):self.r=Reader()
 def test_exact_parent_supplies_same_input_without_mutation(self):
  before=copy.deepcopy(self.r.data);fixture,proof=resolve(self.r,INCOMPLETE);self.assertEqual(fixture,self.r.fixture);self.assertTrue(proof['inherited']);self.assertFalse(proof['immutable_reports_changed']);self.assertEqual(self.r.data,before)
 def test_explicit_input_is_used_directly(self):
  self.r.data[INCOMPLETE['record']]['owned_input']=self.r.fixture;fixture,proof=resolve(self.r,INCOMPLETE);self.assertEqual(fixture,self.r.fixture);self.assertFalse(proof['inherited'])
 def test_unknown_incomplete_plan_refused(self):
  ref=dict(record='unknown',sha256='b'*64);self.r.hashes['unknown']='b'*64;self.r.data['unknown']=self.r.data[INCOMPLETE['record']]
  with self.assertRaises(ValueError):resolve(self.r,ref)
 def test_changed_parent_reference_refused(self):
  self.r.data[INCOMPLETE['record']]['previous_plan']['sha256']='b'*64
  with self.assertRaises(ValueError):resolve(self.r,INCOMPLETE)
 def test_stale_parent_refused(self):
  self.r.hashes[PARENT['record']]='b'*64
  with self.assertRaises(ValueError):resolve(self.r,INCOMPLETE)
 def test_changed_fixture_refused(self):
  self.r.hashes['fixture']='b'*64
  with self.assertRaises(ValueError):resolve(self.r,INCOMPLETE)
 def test_failed_parent_refused(self):
  self.r.data[PARENT['record']]['success']=False
  with self.assertRaises(ValueError):resolve(self.r,INCOMPLETE)
if __name__=='__main__':unittest.main()
