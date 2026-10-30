#!/usr/bin/env python3
"""Completion contracts preserve required scope and cannot manufacture acceptance."""
from copy import deepcopy
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
from rendering_completion import validate


class CompletionTests(unittest.TestCase):
    def setUp(self):
        self.plan = json.loads((ROOT/'docs/native-rendering-closure.json').read_bytes())
        self.decisions = json.loads((ROOT/'docs/native-rendering-dependencies.json').read_bytes())
        self.suite = json.loads((ROOT/self.plan['scope']['journey_catalog']).read_bytes())
        # Synthetic baseline projection for pure contract guards. The real CLI
        # independently reads/verifies the actual SQLite payload and source tree.
        self.baseline = dict(kind='reviewed_rendering_baseline', success=True,
            static_review_only=True, engine_execution_performed=False,
            subject=dict(commit=self.plan['reviewed_commit']),
            baseline_scope=dict(exclusions=deepcopy(self.plan['scope']['exclusions'])),
            source_role_decisions=deepcopy(self.decisions['groups']))

    def check(self):
        return validate(self.plan, self.decisions, self.suite, self.baseline)

    def test_complete_contract_is_not_accepted_player(self):
        result = self.check()
        self.assertTrue(result['success'])
        self.assertEqual(result['counts']['journeys'], 14)
        self.assertEqual(result['counts']['operations'], 21)
        self.assertFalse(result['engine_acceptance_assessed'])
        self.assertFalse(result['complete_replacement'])
        self.assertFalse(result['engine_execution_performed'])
        self.assertEqual(result['status_promotions'], [])

    def test_missing_journey_subrequirement_or_operation_refuses(self):
        for kind in ('journey', 'operation'):
            with self.subTest(kind=kind):
                old = deepcopy(self.plan)
                if kind == 'journey':self.plan['journeys'][0]['requirements'].pop()
                else:self.plan['operations'].pop()
                with self.assertRaisesRegex(ValueError, 'requirements differ'):self.check()
                self.plan = old

    def test_catalog_and_scope_cannot_jointly_shrink(self):
        self.suite['requirements'].pop('map-transitions')
        self.plan['journeys'] = [j for j in self.plan['journeys'] if j['id'] != 'map-transitions']
        with self.assertRaisesRegex(ValueError, 'denominator'):self.check()

    def test_original_build_cannot_be_jointly_rebound(self):
        self.plan['original_sha256'] = self.suite['source_sha256'] = '0'*64
        with self.assertRaisesRegex(ValueError, 'Original build'):self.check()

    def test_no_fps_floor_and_no_waived_response_or_resource_bounds(self):
        for key, value in [('minimum_fps_required', True), ('response_deadlines_required', False),
                           ('resource_and_progress_checks_required', False)]:
            old = deepcopy(self.plan)
            self.plan['scope']['performance_policy'][key] = value
            with self.subTest(key=key), self.assertRaisesRegex(ValueError, 'bounds changed'):self.check()
            self.plan = old

    def test_missing_gate_or_unknown_decision_cannot_disappear(self):
        for key in ('scope_decisions',):
            old = deepcopy(self.plan)
            self.plan['completion_contract'][key].pop()
            with self.assertRaisesRegex(ValueError, 'scope decision'):self.check()
            self.plan = old
        self.plan['completion_contract']['acceptance']['gates'].pop()
        with self.assertRaisesRegex(ValueError, 'completion gate'):self.check()

    def test_unsupported_or_disabled_default_configuration_refuses(self):
        for key, value in [('world_format', 'RGB555'), ('movies', 'disabled'),
                           ('state', 'accepted'), ('arguments', ['--fullscreen'])]:
            old = deepcopy(self.plan)
            self.plan['completion_contract']['configurations'][0][key] = value
            with self.subTest(key=key), self.assertRaises(ValueError):self.check()
            self.plan = old

    def test_original_preparation_does_not_allow_original_pixels(self):
        self.plan['completion_contract']['supplier_policy']['original_pixel_fallback_allowed'] = True
        with self.assertRaisesRegex(ValueError, 'fallback policy'):self.check()

    def test_supplier_roles_and_exclusions_need_explicit_boundary_review(self):
        self.decisions['groups'][0]['role'] = 'retained_original_preparation_allowed'
        with self.assertRaisesRegex(ValueError, 'service roles'):self.check()
        self.setUp()
        self.plan['scope']['exclusions'].append('All unobserved routes')
        with self.assertRaisesRegex(ValueError, 'exclusions'):self.check()
        self.setUp()
        self.plan['completion_contract']['reviewed_sources'].pop('tools/native_only_contract.py')
        with self.assertRaisesRegex(ValueError, 'witness set'):self.check()

    def test_resolved_decisions_need_reason_evidence_and_boundary(self):
        row = self.plan['completion_contract']['scope_decisions'][0]
        row['state'] = 'resolved'
        with self.assertRaisesRegex(ValueError, 'exact boundary'):self.check()
        row.update(resolution_evidence=['TEST.evidence'], resolution_reason='Reviewed fixture boundary',
                   resolved_boundary='Fixture only')
        self.assertFalse(self.check()['complete_replacement'])

    def test_changed_baseline_or_static_acceptance_promotion_refuses(self):
        self.baseline['subject']['commit'] = '0'*40
        with self.assertRaisesRegex(ValueError, 'revision differs'):self.check()
        self.setUp()
        self.plan['completion_contract']['claim_state'] = 'accepted'
        with self.assertRaisesRegex(ValueError, 'cannot certify'):self.check()

    def test_contract_only_cli_never_verifies_or_reads_original_media(self):
        spec = importlib.util.spec_from_file_location('completion_cli', ROOT/'tools/reconcile-rendering-dependencies.py')
        cli = importlib.util.module_from_spec(spec);spec.loader.exec_module(cli)
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            with patch.object(cli, 'ROOT', root), patch('rendering_completion.review', return_value=self.check()), \
                 patch('evidence_local.local_evidence', side_effect=lambda _: __import__('contextlib').nullcontext()), \
                 patch.object(cli.subprocess, 'run', side_effect=AssertionError('Original command invoked')), \
                 patch.object(cli, 'write_report', return_value='fixture'), \
                 patch.object(sys, 'argv', ['reconcile', '--contract-only', '--report', 'research/runtime/check.json']), \
                 patch('sys.stdout', new_callable=io.StringIO):
                self.assertEqual(cli.main(), 0)


if __name__ == '__main__':
    unittest.main()
