#!/usr/bin/env python3
"""Six-row closeout must preserve every inherited definition and decision."""
from copy import deepcopy
import importlib.util
import json
from pathlib import Path
import sys
import unittest
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools'))
spec=importlib.util.spec_from_file_location('ui_close',ROOT/'tools/close-rendering-ui.py');m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)
from rendering_acceptance import requirements
from coverage_claims import fingerprint


class Tests(unittest.TestCase):
    def setUp(self):
        self.plan=json.loads((ROOT/'docs/native-rendering-closure.json').read_text());rows=requirements(self.plan);passed=0
        for r in rows:
            accepted=r['id'] not in m.LEAVES and passed<88
            r.update(state='passed' if accepted else 'pending',admitted_execution_evidence=[dict(execution='historical')] if accepted else [])
            passed+=accepted
        self.parent=dict(success=True,dossier_identity=fingerprint(requirements(self.plan)),requirements=rows,passed_requirements=88,pending_requirements=68,sources={'old':'hash'},scope='historical scope',decisions={'supported-player':'pending','original-recovery':'pending'})
        self.cases=[dict(identity=str(i),software=s,size=list(z),verified=True,execution={'case':i}) for i,(s,z) in enumerate((s,z) for s in (False,True) for z in ((800,600),(640,480)))]
    def test_exactly_six_preserves_parent(self):
        before=deepcopy(self.parent);rows=m.close_rows(self.plan,self.parent,self.cases)
        self.assertEqual(self.parent,before);self.assertEqual(sum(a!=b for a,b in zip(rows,before['requirements'])),6)
        self.assertEqual(sum(v['state']=='passed' for v in rows),94)
        for a,b in zip(rows,before['requirements']):
            if b['id'] not in m.LEAVES:self.assertEqual(a,b)
    def test_incomplete_foreign_duplicate_domains(self):
        bads=[self.cases[:3],self.cases+[self.cases[0]]]
        for field,value in [('identity','0'),('software',False),('size',[1024,768]),('verified',False)]:
            cases=deepcopy(self.cases);cases[-1][field]=value;bads.append(cases)
        for cases in bads:
            with self.subTest(cases=cases),self.assertRaises(ValueError):m.close_rows(self.plan,self.parent,cases)
    def test_unknown_missing_changed_rows(self):
        for change in ('missing','duplicate','metadata','counter','dossier','accepted-target'):
            p=deepcopy(self.parent)
            if change=='missing':p['requirements'].pop()
            elif change=='duplicate':p['requirements'][-1]=p['requirements'][0]
            elif change=='metadata':p['requirements'][0]['pending_reason']='altered'
            elif change=='counter':p['passed_requirements']=89
            elif change=='dossier':p['dossier_identity']='bad'
            else:next(v for v in p['requirements'] if v['id'] in m.LEAVES)['state']='passed'
            with self.subTest(change=change),self.assertRaises(ValueError):m.close_rows(self.plan,p,self.cases)


if __name__=='__main__':unittest.main()
