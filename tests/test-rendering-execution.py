#!/usr/bin/env python3
"""Execution plans preserve the acceptance pilot, required coverage and decision boundaries."""
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
from rendering_execution import validate


class ExecutionTests(unittest.TestCase):
    def setUp(self):
        self.plan = json.loads((ROOT/'docs/native-rendering-closure.json').read_bytes())
        self.execution = self.plan['execution_plan']
        self.tasks = {t['id']: t for t in self.execution['tasks']}

    def check(self):
        return validate(self.plan)

    def test_complete_plan_is_not_executed_acceptance(self):
        report = self.check()
        self.assertEqual(report['counts']['closing_briefs'], 36)
        self.assertEqual(report['counts']['required_tasks'], 11)
        self.assertEqual(report['initial_ready_tasks'], ['EP01'])
        self.assertEqual(report['dependency_waves'], [['EP01'], ['EP02'], ['EP03'],
            ['EP04', 'EP05', 'EP06'], ['EP07', 'EP08', 'EP09'], ['EP10'], ['EP11']])
        for key in ['task_execution_performed', 'engine_execution_performed', 'original_media_consumed',
                    'engine_acceptance_assessed', 'optimality_proved', 'complete_replacement']:
            self.assertIs(report[key], False)
        self.assertEqual(report['status_promotions'], [])

    def test_missing_or_duplicate_tasks_refuse(self):
        for duplicate in [False, True]:
            self.setUp()
            if duplicate:self.execution['tasks'].append(deepcopy(self.execution['tasks'][0]))
            else:self.execution['tasks'].pop()
            with self.subTest(duplicate=duplicate), self.assertRaisesRegex(ValueError, 'execution task'):self.check()

    def test_contract_and_gap_bindings_cannot_drift(self):
        for key in ['completion_contract', 'required_gap_register']:
            self.setUp();self.execution[key]['sha256'] = '0'*64
            with self.subTest(key=key), self.assertRaisesRegex(ValueError, 'input binding'):self.check()

    def test_cycle_unknown_and_self_dependencies_refuse(self):
        for deps in [['EP11'], ['invented'], ['EP01']]:
            self.setUp();self.tasks['EP01']['depends_on'] = deps
            with self.subTest(deps=deps), self.assertRaises(ValueError):self.check()

    def test_broad_runs_cannot_bypass_pilot(self):
        self.tasks['EP04']['depends_on'] = ['EP02']
        with self.assertRaisesRegex(ValueError, 'wait for the acceptance pilot'):self.check()

    def test_unrelated_scope_resolution_cannot_globally_block_pilot(self):
        self.tasks['EP07']['depends_on'] = ['EP02']
        self.tasks['EP03']['depends_on'].append('EP07')
        with self.assertRaisesRegex(ValueError, 'not global scope resolution'):self.check()

    def test_optional_diagnostic_cannot_block_completion(self):
        self.tasks['EP10']['depends_on'].append('EP12')
        with self.assertRaisesRegex(ValueError, 'Diagnostic dependency'):self.check()
        self.setUp();self.tasks['EP12']['required_for_completion'] = True
        with self.assertRaisesRegex(ValueError, 'Optional diagnostic'):self.check()

    def test_missing_and_multiply_owned_briefs_refuse(self):
        self.tasks['EP04']['closing_briefs'].pop()
        with self.assertRaisesRegex(ValueError, 'missing an execution owner'):self.check()
        self.setUp();self.tasks['EP04']['closing_briefs'].append(self.tasks['EP05']['closing_briefs'][0])
        with self.assertRaisesRegex(ValueError, 'multiply owned'):self.check()

    def test_static_task_state_or_evidence_cannot_close_work(self):
        for field, value in [('state', 'complete'), ('execution_evidence', ['invented'])]:
            self.setUp();self.tasks['EP03'][field] = value
            with self.subTest(field=field), self.assertRaisesRegex(ValueError, 'cannot claim execution'):self.check()

    def test_missing_action_or_deliverable_refuses(self):
        for field, value in [('action', ''), ('deliverables', []), ('on_failure', '')]:
            self.setUp();self.tasks['EP03'][field] = value
            with self.subTest(field=field), self.assertRaisesRegex(ValueError, 'lacks an action'):self.check()

    def test_final_join_and_recommended_order_must_cover_dependencies(self):
        self.tasks['EP10']['depends_on'].remove('EP09')
        with self.assertRaisesRegex(ValueError, 'omits required work'):self.check()
        self.setUp();self.execution['recommended_order'].reverse()
        with self.assertRaisesRegex(ValueError, 'contradicts dependencies'):self.check()

    def test_fault_testing_requires_current_normal_lifetime(self):
        self.tasks['EP09']['depends_on'] = ['EP03']
        with self.assertRaisesRegex(ValueError, 'healthy normal lifetime'):self.check()

    def test_session_packs_preserve_journeys_and_both_configurations(self):
        for field in ['journeys', 'configurations']:
            self.setUp()
            if field == 'journeys':
                for p in self.execution['session_packs']:p['journeys'] = [j for j in p['journeys'] if j != 'enabled-movies']
            else:self.execution['session_packs'][0]['configurations'].pop()
            with self.subTest(field=field), self.assertRaises(ValueError):self.check()

    def test_scope_rules_and_technical_ownership_remain(self):
        for field, value in [('blocking_condition', 'Never required'), ('technical_owner', 'user'), ('state', 'resolved')]:
            self.setUp();self.execution['scope_routes'][0][field] = value
            with self.subTest(field=field), self.assertRaisesRegex(ValueError, 'scope condition'):self.check()

    def test_product_choices_do_not_block_plan_or_silently_amend_target(self):
        for field, value in [('blocks_plan', True), ('contract_amendment_required_for_target_change', False)]:
            self.setUp();self.execution['product_choices'][0][field] = value
            with self.subTest(field=field), self.assertRaisesRegex(ValueError, 'Product choice'):self.check()

    def test_witness_rules_optimality_and_first_step_cannot_weaken(self):
        self.execution['reviewed_sources'].pop('tools/drawing_session.py')
        with self.assertRaisesRegex(ValueError, 'witness set'):self.check()
        self.setUp();self.execution['time_optimality_proved'] = True
        with self.assertRaisesRegex(ValueError, 'Unmeasured optimality'):self.check()
        self.setUp();self.execution['scheduling_rules']['reuse_requires_requirement_source_caller_config_scope'] = False
        with self.assertRaisesRegex(ValueError, 'Scheduling rule'):self.check()
        self.setUp();self.execution['first_action']['source_only_checks'].append(['launch-game'])
        with self.assertRaisesRegex(ValueError, 'First action'):self.check()

    def test_execution_plan_cli_never_invokes_original_or_game_commands(self):
        spec = importlib.util.spec_from_file_location('execution_cli', ROOT/'tools/reconcile-rendering-dependencies.py')
        cli = importlib.util.module_from_spec(spec);spec.loader.exec_module(cli)
        with tempfile.TemporaryDirectory() as temporary:
            with patch.object(cli, 'ROOT', Path(temporary)), patch('rendering_execution.review', return_value=self.check()), \
                 patch('evidence_local.local_evidence', side_effect=lambda _: contextlib.nullcontext()), \
                 patch.object(cli.subprocess, 'run', side_effect=AssertionError('Original/game command invoked')), \
                 patch.object(cli, 'write_report', return_value='fixture'), \
                 patch.object(sys, 'argv', ['reconcile', '--execution-plan-only', '--report', 'research/runtime/check.json']), \
                 patch('sys.stdout', new_callable=io.StringIO):
                self.assertEqual(cli.main(), 0)


if __name__ == '__main__':
    unittest.main()
