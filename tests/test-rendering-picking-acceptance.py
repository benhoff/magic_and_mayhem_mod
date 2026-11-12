#!/usr/bin/env python3
"""Prospective row guards; synthetic admission facts earn no native acceptance."""
from copy import deepcopy
import importlib.util
import os
from pathlib import Path
import sys
import unittest
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'tools'))
from coverage_evidence import Reader
spec=importlib.util.spec_from_file_location('picking_closeout',ROOT/'tools/close-rendering-picking-acceptance.py')
m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)
PREFIX='research/runtime/rendering-picking-resize-results-20261107/'


class Admission(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        with Reader(ROOT) as r:
            cls.plan=r.json('docs/native-rendering-closure.json')
            cls.parent=r.json('research/runtime/rendering-retained-results-20261107/closeout-analysis-04.json')
            cls.compared=dict(success=True,sources_stable=True,comparison_processes_stopped=True,comparison=dict(success=True,mismatching_pixels=0),acquisition=dict(record='synthetic-only',sha256='0'*64),proof=dict(verified=True,windows=48,object_windows=24,cell_windows=24,displayed_source_witnesses=144,live_display_rebuilds=2,same_world_continuity=False))
            cls.binding=dict(record='synthetic-only',sha256='0'*64)

    def test_only_default_picking_changes(self):
        rows=m.close_rows(self.plan,self.parent,self.compared,self.binding)
        changed=[(a,b) for a,b in zip(self.parent['requirements'],rows) if a!=b]
        self.assertEqual(len(changed),1)
        self.assertEqual((changed[0][1]['configuration'],changed[0][1]['id']),('player-default',m.LEAF))
        self.assertEqual(changed[0][1]['state'],'passed')
        self.assertEqual(sum(r['state']=='passed' for r in rows),self.parent['passed_requirements']+1)

    def test_missing_or_failed_proof_refuses(self):
        mutations=[lambda c:c.update(success=False),lambda c:c.update(sources_stable=False),
            lambda c:c.update(comparison_processes_stopped=False),
            lambda c:c['comparison'].update(success=False),lambda c:c['comparison'].update(mismatching_pixels=1),
            lambda c:c['proof'].update(verified=False),lambda c:c['proof'].update(windows=47),
            lambda c:c['proof'].update(object_windows=23),lambda c:c['proof'].update(cell_windows=23),
            lambda c:c['proof'].update(displayed_source_witnesses=143),
            lambda c:c['proof'].update(live_display_rebuilds=1),
            lambda c:c['proof'].update(same_world_continuity=True)]
        for index,mutate in enumerate(mutations):
            with self.subTest(index=index):
                c=deepcopy(self.compared);mutate(c)
                with self.assertRaises(ValueError):m.close_rows(self.plan,self.parent,c,self.binding)

    def test_inventory_and_count_corruption_refuses(self):
        for kind in ('duplicate','count','identity','already-passed'):
            with self.subTest(kind=kind):
                p=deepcopy(self.parent)
                if kind=='duplicate':p['requirements'][1]=deepcopy(p['requirements'][0])
                elif kind=='count':p['passed_requirements']+=1
                elif kind=='identity':p['dossier_identity']='0'*64
                else:
                    next(r for r in p['requirements'] if (r['configuration'],r['id'])==('player-default',m.LEAF))['state']='passed'
                    p['passed_requirements']+=1
                with self.assertRaises(ValueError):m.close_rows(self.plan,p,self.compared,self.binding)


if __name__=='__main__':unittest.main()
