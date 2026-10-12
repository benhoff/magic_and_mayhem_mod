#!/usr/bin/env python3
"""Reject denominator drift and attempts to convert planning into evidence."""
import copy
import json
from pathlib import Path
import sys
import unittest
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
from drawing_acceptance import PLAN, admit, inventory
from drawing_corpus import ROOT


class AcceptanceTest(unittest.TestCase):
    def setUp(self):
        self.plan = json.loads((ROOT / PLAN).read_text())

    def test_complete_inventory_has_no_execution_claim(self):
        result = inventory()
        self.assertEqual((result['journey_count'], result['journey_requirement_count'], result['operation_count']), (14, 47, 21))
        for key in ('execution_performed', 'reference_complete', 'draw02_complete'):
            self.assertIs(result[key], False)

    def test_every_requirement_is_mandatory(self):
        for scenario, rows in self.plan['journeys'].items():
            for i in range(len(rows)):
                changed = copy.deepcopy(self.plan)
                changed['journeys'][scenario].pop(i)
                with self.assertRaises(ValueError):
                    admit(ROOT, changed)
        for name in self.plan['operations']:
            changed = copy.deepcopy(self.plan)
            del changed['operations'][name]
            with self.assertRaises(ValueError):
                admit(ROOT, changed)

    def test_observations_inputs_actions_and_dependencies_are_required(self):
        for field in ('actions', 'observe', 'owned_inputs', 'dependencies'):
            for value in ([], [''], ['same', 'same'], 'untyped'):
                changed = copy.deepcopy(self.plan)
                changed['journeys']['cursor-interaction'][0][field] = value
                with self.assertRaises(ValueError):
                    admit(ROOT, changed)

    def test_success_and_evidence_labels_refuse(self):
        for field in ('matching', 'reference_complete', 'evidence', 'draw02_complete'):
            changed = copy.deepcopy(self.plan)
            changed['operations']['surface-loss-restore'][field] = True
            with self.assertRaises(ValueError):
                admit(ROOT, changed)

    def test_order_build_and_identity_drift_refuse(self):
        for mutation in ('order', 'build', 'identity', 'schema'):
            changed = copy.deepcopy(self.plan)
            if mutation == 'order':
                changed['journeys']['camera-motion'].reverse()
            elif mutation == 'build':
                changed['source_sha256'] = '0' * 64
            elif mutation == 'identity':
                changed['operations']['copy-keyed']['id'] = 'copy-opaque'
            else:
                changed['schema'] = True
            with self.assertRaises(ValueError):
                admit(ROOT, changed)


if __name__ == '__main__':
    unittest.main()
