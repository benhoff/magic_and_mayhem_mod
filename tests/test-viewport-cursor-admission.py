#!/usr/bin/env python3
"""A component or malformed inventory cannot promote the four requested rows."""
import copy
import importlib.util
import json
from pathlib import Path
import sys
import unittest
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'tools'))
from coverage_claims import fingerprint
from rendering_acceptance import requirements
from viewport_cursor import LEAVES,SIZES,edge_points
spec=importlib.util.spec_from_file_location('viewport_admission',ROOT/'tools/close-rendering-viewport-cursor.py')
admission=importlib.util.module_from_spec(spec);spec.loader.exec_module(admission)


class AdmissionGuards(unittest.TestCase):
    def setUp(self):
        self.plan=json.loads((ROOT/'docs/native-rendering-closure.json').read_text())
        definitions=requirements(self.plan)
        rows=[dict(r,state='pending' if r['id'] in LEAVES else 'passed',admitted_execution_evidence=[])
              for r in definitions]
        self.parent=dict(requirements=rows,dossier_identity=fingerprint(definitions),passed_requirements=152)
        self.cases=[]
        for software in (False,True):
            phases=[dict(size=size,windows=[dict(scaling=mode,label=label,verified=True,generations=[1,2,3])
                     for mode in ('sharp','integer') for label,_ in edge_points(size)]) for size in SIZES]
            proof=dict(verified=True,software=software,normal_quit=True,mismatching_pixels=0,
                       phases=phases,rebuilds=[dict(verified=True),dict(verified=True)])
            self.cases.append(dict(proof=proof,acquisition={},comparison={}))
    def test_exact_scope(self):
        rows=admission.close_rows(self.plan,self.parent,self.cases)
        changed=[(a['configuration'],a['id']) for a,b in zip(rows,self.parent['requirements']) if a!=b]
        self.assertEqual(set(changed),{(c,leaf) for c in ('player-default','player-software') for leaf in LEAVES})
    def test_rejects_one_profile(self):
        with self.assertRaises(ValueError):admission.close_rows(self.plan,self.parent,self.cases[:1])
    def test_rejects_missing_rebuild(self):
        self.cases[0]['proof']['rebuilds'].pop()
        with self.assertRaises(ValueError):admission.close_rows(self.plan,self.parent,self.cases)
    def test_rejects_missing_corner(self):
        self.cases[1]['proof']['phases'][1]['windows'].pop()
        with self.assertRaises(ValueError):admission.close_rows(self.plan,self.parent,self.cases)
    def test_rejects_invented_parent_total(self):
        self.parent['passed_requirements']=151
        with self.assertRaises(ValueError):admission.close_rows(self.plan,self.parent,self.cases)
    def test_rejects_foreign_inventory(self):
        self.parent['requirements'][0]['id']='invented'
        with self.assertRaises(ValueError):admission.close_rows(self.plan,self.parent,self.cases)
    def test_rejects_pixel_failure(self):
        self.cases[0]['proof']['mismatching_pixels']=1
        with self.assertRaises(ValueError):admission.close_rows(self.plan,self.parent,self.cases)
    def test_rejects_repromotion(self):
        row=next(v for v in self.parent['requirements'] if v['id'] in LEAVES)
        row['state']='passed';self.parent['passed_requirements']+=1
        with self.assertRaises(ValueError):admission.close_rows(self.plan,self.parent,self.cases)


if __name__=='__main__':unittest.main()
