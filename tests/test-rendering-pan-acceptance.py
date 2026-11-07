#!/usr/bin/env python3
"""Mutate retained actual pan facts and require the frozen row gate to refuse."""
from copy import deepcopy
import importlib.util
import os
from pathlib import Path
import sys
import unittest
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools'))
from coverage_evidence import Reader
spec=importlib.util.spec_from_file_location('pan_acceptance',ROOT/'tools/close-rendering-pan-acceptance.py');m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)

class Pan(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        with Reader(ROOT) as r:
            cls.report=r.json(os.environ.get('MNM_PAN_REPORT',m.PREFIX+'pan-analysis-01.json'))
            if not cls.report['success']:raise ValueError('Successful retained analysis required')
            cls.parent=r.json(cls.report['parent']['record']);cls.plan=r.json('docs/native-rendering-closure.json')

    def test_all_eight_actual_pans(self):
        self.assertEqual(len(self.report['cases']),4)
        for c in self.report['cases']:
            for p in c['pans']:m.validate_pan(p)

    def test_evidence_mutations_refuse(self):
        mutations=[lambda p:p.update(verified=False),lambda p:p.update(physical_pan=False),
            lambda p:p.update(normal_quit=False),lambda p:p['after'].update(center=p['before']['center']),
            lambda p:p['after'].update(view=1),lambda p:p['object_hit'].update(slot=1),
            lambda p:p['object_hit'].update(active=False),lambda p:p['object_hit'].update(health=0),
            lambda p:p.update(motion_callback_returned=False),lambda p:p.update(selection_callback_returned=False),
            lambda p:p.update(source_only_target=False),lambda p:p.update(picking_source_after_pan=False),
            lambda p:p.update(minimap_camera_matches_world=False),lambda p:p.update(minimap_layout_unchanged=False),
            lambda p:p.update(whole_viewport_swaps=2),lambda p:p.update(compared_pixels=0),
            lambda p:p.update(mismatching_pixels=1),lambda p:p.update(source_generations=2)]
        for i,mutate in enumerate(mutations):
            with self.subTest(i=i):
                p=deepcopy(self.report['cases'][0]['pans'][0]);mutate(p)
                with self.assertRaises(ValueError):m.validate_pan(p)

    def test_domain_or_scale_omission_refuses(self):
        for missing in ('domain','duplicate','scale'):
            with self.subTest(missing=missing):
                cases=deepcopy(self.report['cases'])
                if missing=='domain':cases.pop()
                elif missing=='duplicate':cases[-1]=deepcopy(cases[0])
                else:cases[0]['pans'].pop()
                with self.assertRaises(ValueError):m.close_rows(self.plan,self.parent,cases)

    def test_exact_two_rows_preserve_every_parent(self):
        rows=m.close_rows(self.plan,self.parent,self.report['cases'])
        changes=[b for a,b in zip(self.parent['requirements'],rows) if a!=b]
        self.assertEqual({(r['configuration'],r['id']) for r in changes},{('player-default',m.LEAF),('player-software',m.LEAF)})
        self.assertEqual(len(changes),2)
        self.assertEqual(rows,self.report['requirements'])
        self.assertEqual(self.parent['decisions'],self.report['decisions'])
        self.assertEqual(self.parent['sources'],self.report['sources'])

if __name__=='__main__':unittest.main()
