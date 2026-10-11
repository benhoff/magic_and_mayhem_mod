#!/usr/bin/env python3
"""Acceptance denominator, separate reference/parity axes and boundary refusals."""
import copy
import json
from pathlib import Path
import sys
import unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
import drawing_journeys as journeys
from drawing_corpus import ROOT

class JourneyTest(unittest.TestCase):
    def row(self, **changes):
        row={'id':'camera','scenario':'camera-motion','boundary':'presented_frame',
             'outcome':'matching','independent_reference':True,'contiguous':True,
             'actions_bound':True,'frames':4,'requirements':['pan'],'operations':[]}
        row.update(changes)
        return row

    def test_frozen_denominators(self):
        catalog=json.loads((ROOT/journeys.CATALOG).read_text())
        journeys.admit_suite(ROOT,catalog)
        self.assertEqual(len(catalog['requirements']),14)
        for key in ('requirements','operation_requirements'):
            changed=copy.deepcopy(catalog)
            if key=='requirements':changed[key]['camera-motion'].pop()
            else:changed[key].pop()
            with self.assertRaisesRegex(ValueError,'denominator'):
                journeys.admit_suite(ROOT,changed)

    def test_catalog_hash_and_duplicate_guards(self):
        catalog=json.loads((ROOT/journeys.CATALOG).read_text())
        changed=copy.deepcopy(catalog);changed['cases'][0]['catalog']['sha256']='0'*64
        with self.assertRaisesRegex(ValueError,'identity'):
            journeys.admit_suite(ROOT,changed)
        changed=copy.deepcopy(catalog);changed['cases'].append(changed['cases'][0])
        with self.assertRaisesRegex(ValueError,'Duplicate'):
            journeys.admit_suite(ROOT,changed)

    def test_component_pass_cannot_close_journey(self):
        for boundary in ('operation','world_canvas','composed_canvas'):
            result=journeys.coverage([self.row(boundary=boundary)])
            camera=next(s for s in result['scenarios'] if s['id']=='camera-motion')
            self.assertEqual(camera['reference_covered'],[])
            self.assertFalse(result['draw02_complete'])

    def test_temporal_action_and_original_guards(self):
        for changes in ({'contiguous':False},{'actions_bound':False},{'independent_reference':False},{'frames':0},{'frames':True}):
            result=journeys.coverage([self.row(**changes)])
            camera=next(s for s in result['scenarios'] if s['id']=='camera-motion')
            self.assertEqual(camera['reference_covered'],[])

    def test_reference_survives_native_failure(self):
        for outcome in ('mismatch','unsupported','error','pending'):
            result=journeys.coverage([self.row(outcome=outcome)])
            camera=next(s for s in result['scenarios'] if s['id']=='camera-motion')
            self.assertEqual(camera['reference_covered'],['pan'])
            self.assertEqual(camera['native_matching'],[])
            self.assertEqual(result['nonmatching'][0]['outcome'],outcome)

    def test_unknown_requirements_and_duplicate_observations_refuse(self):
        for row in (self.row(requirements=['invented']),self.row(operations=['invented']),self.row(scenario='invented'),self.row(requirements=['pan','pan'])):
            with self.assertRaises(ValueError):journeys.coverage([row])
        with self.assertRaisesRegex(ValueError,'Duplicate'):
            journeys.coverage([self.row(),self.row()])

    def test_full_gate_requires_every_journey_and_operation(self):
        rows=[self.row(id=s,scenario=s,requirements=list(journeys.REQUIREMENTS[s]),operations=list(journeys.OPERATIONS) if i==0 else []) for i,s in enumerate(journeys.SCENARIOS)]
        result=journeys.coverage(rows)
        self.assertTrue(result['reference_complete']);self.assertTrue(result['draw02_complete'])
        for i in range(len(rows)):
            changed=copy.deepcopy(rows);changed[i]['requirements'].pop()
            self.assertFalse(journeys.coverage(changed)['draw02_complete'])
        changed=copy.deepcopy(rows);changed[0]['operations'].pop()
        self.assertFalse(journeys.coverage(changed)['draw02_complete'])
        changed=copy.deepcopy(rows);changed[0]['outcome']='mismatch'
        self.assertTrue(journeys.coverage(changed)['reference_complete'])
        self.assertFalse(journeys.coverage(changed)['draw02_complete'])

    def test_native_error_preserves_admitted_reference_in_suite(self):
        import tempfile
        from unittest.mock import patch
        from drawing_corpus import load_tool,sha,BUILD
        checker=load_tool('check-drawing-journeys')
        with tempfile.TemporaryDirectory() as tmp:
            root=Path(tmp);(root/'research/runtime/coverage').mkdir(parents=True)
            (root/'research/runtime/coverage/register.json').write_text('{}')
            child={'scenario':'camera-motion','requirements':['pan'],'operations':[],'frames':[{},{}]}
            (root/'child.json').write_text(json.dumps(child))
            catalog={'schema':1,'source_sha256':BUILD,'requirements':{s:list(journeys.REQUIREMENTS[s]) for s in journeys.SCENARIOS},
                     'operation_requirements':list(journeys.OPERATIONS),'cases':[{'id':'camera','adapter':'session',
                     'catalog':{'path':'child.json','sha256':sha((root/'child.json').read_bytes())},'scope':'Synthetic accounting only'}]}
            (root/'suite.json').write_text(json.dumps(catalog))
            with patch('drawing_session.admit',return_value=([],[{},{}],[])),patch('drawing_session.check',side_effect=ValueError('backend failed')):
                result=checker.run(root,'suite.json',session=Path('/fake/backend'))
            camera=next(s for s in result['scenarios'] if s['id']=='camera-motion')
            self.assertEqual(camera['reference_covered'],['pan']);self.assertEqual(camera['native_matching'],[])
            self.assertTrue(result['cases'][0]['reference_admitted']);self.assertFalse(result['success'])
            with patch('drawing_session.admit',side_effect=ValueError('original hash mismatch')):
                result=checker.run(root,'suite.json',session=Path('/fake/backend'))
            self.assertFalse(result['cases'][0]['reference_admitted'])
            self.assertTrue(all(not s['reference_covered'] for s in result['scenarios']))

    def test_inventory_does_not_assert_execution(self):
        result=journeys.inventory(ROOT)
        self.assertFalse(result['reference_complete']);self.assertFalse(result['draw02_complete'])
        self.assertTrue(all(not s['reference_covered'] for s in result['scenarios']))

if __name__=='__main__':unittest.main()
