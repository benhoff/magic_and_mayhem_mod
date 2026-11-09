#!/usr/bin/env python3
"""Reject incomplete domains and accidental acceptance outside caller-visible state."""
import copy,importlib.util,sys,unittest
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools'))
spec=importlib.util.spec_from_file_location('caller_admission',ROOT/'tools/close-rendering-caller-state.py');m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)
from coverage_evidence import Reader
class Admission(unittest.TestCase):
 def setUp(self):
  self.cases=[dict(identity=str((s,d)),software=s,size=d,plan_sha256='same',native_phase_sha256='same',verified=True,state_cases=1,pixels=1,mismatches=0,normal_retirement=True,source_accounted=True,independent_expected_bytes=True,supplied_contexts=False,acquisition={},comparison={},provider='test')for s in(False,True)for d in((640,480),(800,600))]
 def test_complete_domains(self):m.validate_cases(self.cases)
 def test_missing_domain(self):
  with self.assertRaises(ValueError):m.validate_cases(self.cases[:-1])
 def test_reused_session(self):
  self.cases[-1]['identity']=self.cases[0]['identity']
  with self.assertRaises(ValueError):m.validate_cases(self.cases)
 def test_duplicate_resolution(self):
  self.cases[-1]['size']=(640,480)
  with self.assertRaises(ValueError):m.validate_cases(self.cases)
 def test_source_phase_mixture(self):
  self.cases[-1]['native_phase_sha256']='other'
  with self.assertRaises(ValueError):m.validate_cases(self.cases)
 def test_each_conjunct_required(self):
  for key,value in [('verified',False),('state_cases',0),('pixels',0),('mismatches',1),('normal_retirement',False),('source_accounted',False),('independent_expected_bytes',False),('supplied_contexts',True)]:
   with self.subTest(key=key):
    cases=copy.deepcopy(self.cases);cases[0][key]=value
    with self.assertRaises(ValueError):m.validate_cases(cases)
 def test_only_two_rows_change(self):
  with Reader(ROOT)as r:plan=r.json('docs/native-rendering-closure.json');parent=r.json('research/runtime/palette-master-merge-results-20261108/acceptance-01.json')
  before=copy.deepcopy(parent);rows=m.close_rows(plan,parent,self.cases);self.assertEqual(before,parent)
  changed=[(a,b)for a,b in zip(parent['requirements'],rows)if a!=b];self.assertEqual(len(changed),2);self.assertTrue(all(a['id']==m.LEAF and b['state']=='passed'for a,b in changed));self.assertEqual(parent['decisions'],before['decisions'])
 def test_parent_count_guard(self):
  with Reader(ROOT)as r:plan=r.json('docs/native-rendering-closure.json');parent=r.json('research/runtime/palette-master-merge-results-20261108/acceptance-01.json')
  parent['passed_requirements']+=1
  with self.assertRaises(ValueError):m.close_rows(plan,parent,self.cases)
 def test_parent_identity_guard(self):
  with Reader(ROOT)as r:plan=r.json('docs/native-rendering-closure.json');parent=r.json('research/runtime/palette-master-merge-results-20261108/acceptance-01.json')
  parent['dossier_identity']='other'
  with self.assertRaises(ValueError):m.close_rows(plan,parent,self.cases)
if __name__=='__main__':unittest.main()
