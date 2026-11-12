#!/usr/bin/env python3
"""A gap inventory cannot silently waive a requirement or manufacture a code defect."""
from copy import deepcopy
import contextlib
import importlib.util
import io
import json
from pathlib import Path
import sys
import tempfile
import unittest
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT/'tools'))
from rendering_gaps import validate


class GapTests(unittest.TestCase):
    def setUp(self):
        self.plan = json.loads((ROOT/'docs/native-rendering-closure.json').read_bytes())
        self.register = json.loads((ROOT/'research/runtime/coverage/register.json').read_bytes())
        self.gaps = self.plan['required_gap_register']

    def check(self):
        return validate(self.plan, self.register)

    def test_complete_inventory_does_not_assert_completion_or_missing_code(self):
        r = self.check()
        self.assertEqual(r['counts']['mandatory_assessments'], 78)
        self.assertEqual(r['counts']['journey_subrequirements'], 47)
        self.assertEqual(r['counts']['conditional_scope_decisions'], 8)
        self.assertEqual(r['mandatory_classifications'], dict(missing_code=0, missing_integration=0, missing_proof=78))
        for flag in ['engine_acceptance_assessed', 'engine_execution_performed', 'original_media_consumed',
                     'complete_replacement', 'execution_order_assessed']:
            self.assertIs(r[flag], False)
        self.assertEqual(r['status_promotions'], [])

    def test_missing_or_duplicate_journey_and_leaf_refuse(self):
        for location in ['journeys', 'subrequirements']:
            for duplicate in [False, True]:
                self.setUp()
                rows = self.gaps['journeys'] if location == 'journeys' else self.gaps['journeys'][0][location]
                if duplicate:rows.append(deepcopy(rows[0]))
                else:rows.pop()
                with self.subTest(location=location, duplicate=duplicate), self.assertRaises(ValueError):self.check()

    def test_operation_gate_and_scope_decision_cannot_disappear(self):
        for key in ['operations', 'gates', 'scope_decisions']:
            self.setUp();self.gaps[key].pop()
            with self.subTest(key=key), self.assertRaises(ValueError):self.check()

    def test_unlinked_or_duplicate_closing_task_refuses(self):
        self.gaps['operations'][0]['closing_task'] = 'missing'
        with self.assertRaisesRegex(ValueError, 'Unknown closing task'):self.check()
        self.setUp();self.gaps['task_briefs'].append(deepcopy(self.gaps['task_briefs'][0]))
        with self.assertRaisesRegex(ValueError, 'Duplicate'):self.check()

    def test_none_status_does_not_establish_missing_code_or_integration(self):
        for classification in ['missing_code', 'missing_integration']:
            self.setUp();self.gaps['operations'][0]['classification'] = classification
            with self.subTest(classification=classification), self.assertRaisesRegex(ValueError, 'concrete missing branch'):self.check()

    def test_static_closed_acceptance_refuses(self):
        for key in ['operations', 'gates', 'journeys']:
            self.setUp();self.gaps[key][0]['state'] = 'closed'
            with self.subTest(key=key), self.assertRaises(ValueError):self.check()

    def test_baseline_and_contract_cannot_be_rebound(self):
        self.gaps['baseline']['sha256'] = '0'*64
        with self.assertRaisesRegex(ValueError, 'Baseline'):self.check()
        self.setUp();self.plan['completion_contract']['configurations'][0]['lighting'] = 'original'
        with self.assertRaisesRegex(ValueError, 'contract binding'):self.check()

    def test_evidence_and_behavior_leads_must_exist(self):
        for key in ['behavior_leads', 'evidence_leads']:
            self.setUp();self.gaps['journeys'][0][key].append('MADE.UP')
            with self.subTest(key=key), self.assertRaisesRegex(ValueError, 'Unknown'):self.check()

    def test_both_target_configurations_remain(self):
        for key in ['journeys', 'operations']:
            self.setUp();self.gaps[key][0]['configurations'].pop()
            with self.subTest(key=key), self.assertRaisesRegex(ValueError, 'configuration omitted'):self.check()

    def test_scope_conditions_and_closure_cannot_be_waived(self):
        row = self.gaps['scope_decisions'][0]
        row['classification'] = 'optional'
        with self.assertRaisesRegex(ValueError, 'Conditional scope'):self.check()
        self.setUp();self.gaps['scope_decisions'][0]['required_when'] = 'Never required'
        with self.assertRaisesRegex(ValueError, 'Conditional scope'):self.check()
        self.setUp();row = self.gaps['scope_decisions'][0]
        next(b for b in self.gaps['task_briefs'] if b['id'] == row['closing_task'])['close_when'] = 'Ignore it'
        with self.assertRaisesRegex(ValueError, 'closure differs'):self.check()

    def test_diagnostic_cannot_become_a_completion_gate(self):
        self.gaps['diagnostics'][0]['required_for_completion'] = True
        with self.assertRaisesRegex(ValueError, 'Optional timing'):self.check()

    def test_ordering_requires_next_artifact(self):
        self.gaps['task_briefs'][0]['priority'] = 1
        with self.assertRaisesRegex(ValueError, 'unordered'):self.check()

    def test_gap_policy_cannot_weaken(self):
        self.gaps['assessment_policy']['source_freshness_is_not_scope_acceptance'] = False
        with self.assertRaisesRegex(ValueError, 'policy weakened'):self.check()

    def test_gaps_cli_never_invokes_original_media_commands(self):
        spec = importlib.util.spec_from_file_location('gaps_cli', ROOT/'tools/reconcile-rendering-dependencies.py')
        cli = importlib.util.module_from_spec(spec);spec.loader.exec_module(cli)
        with tempfile.TemporaryDirectory() as temporary:
            with patch.object(cli, 'ROOT', Path(temporary)), patch('rendering_gaps.review', return_value=self.check()), \
                 patch('evidence_local.local_evidence', side_effect=lambda _: contextlib.nullcontext()), \
                 patch.object(cli.subprocess, 'run', side_effect=AssertionError('Original command invoked')), \
                 patch.object(cli, 'write_report', return_value='fixture'), \
                 patch.object(sys, 'argv', ['reconcile', '--gaps-only', '--report', 'research/runtime/check.json']), \
                 patch('sys.stdout', new_callable=io.StringIO):
                self.assertEqual(cli.main(), 0)


if __name__ == '__main__':
    unittest.main()
