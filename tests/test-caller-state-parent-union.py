#!/usr/bin/env python3
"""Refuse unrelated row changes, lost inherited proofs and altered decisions."""
import copy,importlib.util,sys,unittest
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools'))
from coverage_evidence import Reader
spec=importlib.util.spec_from_file_location('caller_union',ROOT/'tools/merge-rendering-caller-state-acceptance.py');m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)
class Union(unittest.TestCase):
 def setUp(self):
  with Reader(ROOT)as r:self.plan=r.json('docs/native-rendering-closure.json');self.base=r.json('research/runtime/palette-master-merge-results-20261108/acceptance-01.json')
  self.owned=copy.deepcopy(self.base);self.parent=copy.deepcopy(self.base)
  with Reader(ROOT)as r:self.parent['definition_review']=r.json('research/runtime/viewport-cursor-results-20261108/master-admission-01.json')['definition_review']
  for row in self.owned['requirements']:
   if row['id']==m.LEAF:row['state']='passed'
  self.owned['passed_requirements']+=2;self.owned['pending_requirements']-=2
  for row in self.parent['requirements']:
   if row['id']in('journey.camera-motion.viewport-boundaries','journey.cursor-interaction.cursor-motion'):row['state']='passed'
  self.parent['passed_requirements']+=4;self.parent['pending_requirements']-=4
 def test_union(self):
  before=copy.deepcopy((self.base,self.owned,self.parent));rows=m.union(self.plan,self.base,self.owned,self.parent);self.assertEqual(sum(r['state']=='passed'for r in rows),124);self.assertEqual(before,(self.base,self.owned,self.parent))
 def test_unrelated_owned_change(self):
  self.owned['requirements'][0]['unexpected']='changed'
  with self.assertRaises(ValueError):m.union(self.plan,self.base,self.owned,self.parent)
 def test_lost_parent_acceptance(self):
  next(r for r in self.parent['requirements']if r['state']=='passed')['state']='pending';self.parent['passed_requirements']-=1;self.parent['pending_requirements']+=1
  with self.assertRaises(ValueError):m.union(self.plan,self.base,self.owned,self.parent)
 def test_parent_attachment_change(self):
  next(r for r in self.parent['requirements']if r['state']=='passed')['unexpected']='changed'
  with self.assertRaises(ValueError):m.union(self.plan,self.base,self.owned,self.parent)
 def test_final_decision(self):
  self.parent['decisions']['supported-player']='passed'
  with self.assertRaises(ValueError):m.union(self.plan,self.base,self.owned,self.parent)
 def test_duplicate_cell(self):
  self.parent['requirements'][0]=copy.deepcopy(self.parent['requirements'][1])
  with self.assertRaises(ValueError):m.union(self.plan,self.base,self.owned,self.parent)
 def test_wrong_counts(self):
  self.parent['passed_requirements']+=1
  with self.assertRaises(ValueError):m.union(self.plan,self.base,self.owned,self.parent)
 def test_missing_caller_cell(self):
  next(r for r in self.owned['requirements']if r['id']==m.LEAF)['state']='pending';self.owned['passed_requirements']-=1;self.owned['pending_requirements']+=1
  with self.assertRaises(ValueError):m.union(self.plan,self.base,self.owned,self.parent)
 def test_identity(self):
  self.parent['dossier_identity']='changed'
  with self.assertRaises(ValueError):m.union(self.plan,self.base,self.owned,self.parent)
 def test_annotation_review_required(self):
  self.parent.pop('definition_review')
  with self.assertRaises(ValueError):m.union(self.plan,self.base,self.owned,self.parent)
 def test_conditions_cannot_hide_in_annotation_review(self):
  self.plan['required_gap_register']['operations'][0]['remaining']+=' changed acceptance'
  self.parent['definition_review']['current_definition_sha256']=m.fingerprint(m.requirements(self.plan))
  with self.assertRaises(ValueError):m.union(self.plan,self.base,self.owned,self.parent)
 def test_annotation_deletion_refused(self):
  self.parent['definition_review']['only_added_candidate_annotations']['parent'][0]['before'].pop(0)
  with self.assertRaises(ValueError):m.union(self.plan,self.base,self.owned,self.parent)
if __name__=='__main__':unittest.main()
