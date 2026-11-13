#!/usr/bin/env python3
"""Strict orchestration must match retained execution and review pins."""
import copy,sys,unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from broader_player_viewport_inputs_matrix import coordinator_sources
N='tools/run-broader-viewport-strict-case.py'
T='tools/retire-broader-strict.py'
class Reader:
 def __init__(self):
  self.hashes={N:'coordinator-current',T:'retention-current','plan':'plan-current','review':'review-current'}
  self.values={'plan':dict(success=True,viewport_source_review=dict(record='review',sha256='review-current')),'review':dict(success=True,reviewer_sources={N:'coordinator-current'})}
 def digest(self,n):return self.hashes[n]
 def json(self,n):return copy.deepcopy(self.values[n])
class Pins(unittest.TestCase):
 def setUp(self):
  self.r=Reader();self.phase=dict(broader_plan=dict(record='plan',sha256='plan-current'),sources={N:'coordinator-current',T:'retention-current'});self.co=dict(kind='broader_strict_case_coordinator')
 def inspect(self,role='strict'):return coordinator_sources(self.r,self.phase,self.co,role)
 def test_exact_plan_and_review_supply_strict_provenance(self):self.assertEqual(self.inspect(),{N:'coordinator-current',T:'retention-current'})
 def test_changed_coordinator_or_retention_refused(self):
  for name in(N,T):
   with self.subTest(name=name):
    self.r.hashes[name]='changed'
    with self.assertRaises(ValueError):self.inspect()
    self.r.hashes[name]=self.phase['sources'][name]
 def test_review_disagreement_refused(self):
  self.r.values['review']['reviewer_sources'][N]='old-coordinator'
  with self.assertRaises(ValueError):self.inspect()
 def test_missing_either_plan_pin_refused(self):
  for name in(N,T):
   with self.subTest(name=name):
    value=self.phase['sources'].pop(name)
    with self.assertRaises(ValueError):self.inspect()
    self.phase['sources'][name]=value
 def test_conflicting_result_pin_refused(self):
  self.co['source']=dict(path=N,sha256='different')
  with self.assertRaises(ValueError):self.inspect()
 def test_changed_plan_identity_refused(self):
  self.r.hashes['plan']='changed'
  with self.assertRaises(ValueError):self.inspect()
 def test_other_roles_cannot_borrow_strict_plan_pins(self):
  with self.assertRaises(ValueError):self.inspect('world')
 def test_other_role_exact_recorded_source_still_verified(self):
  self.co['source']=dict(path=N,sha256='coordinator-current');self.assertEqual(self.inspect('world'),{N:'coordinator-current'})
  self.r.hashes[N]='changed'
  with self.assertRaises(ValueError):self.inspect('world')
if __name__=='__main__':unittest.main()
