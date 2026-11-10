#!/usr/bin/env python3
"""Adversarial checks against actual retained boundary and route evidence."""
import copy
from pathlib import Path
import sys
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT/'tools'))
from coverage_evidence import Reader
import rendering_required_boundaries as boundary


class Controls(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.reader = Reader(ROOT)
        cls.parent = cls.reader.json(boundary.PARENT)
        cls.plan = cls.reader.json('docs/native-rendering-closure.json')
        cls.inputs = {k: cls.reader.json(v) for k, v in boundary.INPUTS.items()}
        cls.world_source = cls.reader.read_text('compat/legacy/world_frame.cpp')
        cls.policy = cls.reader.json(boundary.DOMAIN)

    @classmethod
    def tearDownClass(cls):
        cls.reader.close()

    def bad(self, key, change, check):
        value = copy.deepcopy(self.inputs[key])
        change(value)
        with self.assertRaises((ValueError, KeyError)):
            check(value)

    def test_actual_conjunction_keeps_gate_pending(self):
        result = boundary.assess(self.plan, self.parent, self.inputs, self.world_source, self.policy)
        self.assertFalse(result['gate_ready'])
        self.assertEqual(len(result['pending_scope_ids']), 5)
        self.assertTrue(result['history']['accepted_cohort_history_resolved'])
        self.assertEqual(len(result['actual_provider_routes']), 4)

    def test_parent_cannot_have_duplicate_rows(self):
        p = copy.deepcopy(self.parent); p['requirements'][0] = p['requirements'][1]
        with self.assertRaises(ValueError): boundary.basis(self.plan, p)

    def test_parent_count_cannot_promote_gate(self):
        p = copy.deepcopy(self.parent); p['passed_requirements'] = 150
        with self.assertRaises(ValueError): boundary.basis(self.plan, p)

    def test_controlled_overlap_obligation_remains(self):
        p = copy.deepcopy(self.parent); p['scope_amendments'][0]['remaining_obligation']['state'] = 'passed'
        with self.assertRaises(ValueError): boundary.basis(self.plan, p)

    def test_no_final_decision_by_review(self):
        p = copy.deepcopy(self.parent); p['decisions']['supported-player'] = 'passed'
        with self.assertRaises(ValueError): boundary.basis(self.plan, p)

    def test_empty_scan_cannot_prove_unreachable(self):
        self.bad('callers', lambda p: p.update(global_unreachable_proved=True),
                 lambda p: boundary.check_callers(p, self.inputs['routing']))

    def test_missing_route_does_not_shrink_denominator(self):
        self.bad('callers', lambda p: p['routes'].pop(),
                 lambda p: boundary.check_callers(p, self.inputs['routing']))

    def test_wrong_entry_needs_new_review(self):
        self.bad('callers', lambda p: p['routes'][0].update(start=1),
                 lambda p: boundary.check_callers(p, self.inputs['routing']))

    def test_local_dispatch_does_not_close_indirect_targets(self):
        self.bad('callers', lambda p: p['dispatch_witnesses'].update(other_dispatches_closed=True),
                 lambda p: boundary.check_callers(p, self.inputs['routing']))

    def test_safety_does_not_prove_natural_overlap(self):
        self.bad('routing', lambda p: p.update(natural_overlap_witness=True),
                 lambda p: boundary.check_callers(self.inputs['callers'], p))

    def test_facade_is_not_strict_player(self):
        self.bad('formats', lambda p: p.update(strict_player_executed=True),
                 lambda p: boundary.check_formats(p, self.world_source))

    def test_rgb555_storage_cannot_disappear(self):
        self.bad('formats', lambda p: p['format_rows'].pop(),
                 lambda p: boundary.check_formats(p, self.world_source))

    def test_surviving_format_owner_refuses(self):
        self.bad('formats', lambda p: p['format_rows'][2].update(remaining_surfaces=1),
                 lambda p: boundary.check_formats(p, self.world_source))

    def test_rgb555_world_policy_change_refuses(self):
        with self.assertRaises(ValueError): boundary.check_formats(self.inputs['formats'], '')

    def test_missing_history_domain_refuses(self):
        self.bad('history', lambda p: p['cases'].pop(), lambda p: boundary.check_history(p, self.parent))

    def test_history_is_not_current_player(self):
        self.bad('history', lambda p: p.update(current_whole_player_proved=True), lambda p: boundary.check_history(p, self.parent))

    def test_history_needs_ordered_imports(self):
        self.bad('history', lambda p: p['cases'][0]['command_history'].update(imports=0), lambda p: boundary.check_history(p, self.parent))

    def test_history_reference_cannot_be_absent(self):
        self.bad('history', lambda p: p['cases'][0]['history_index'].update(sha256=''), lambda p: boundary.check_history(p, self.parent))

    def test_progression_needs_four_real_domains(self):
        self.bad('progression', lambda p: p['cases'].pop(), boundary.check_progression)

    def test_later_map_not_save_reload(self):
        self.bad('progression', lambda p: p['cases'][0]['original']['later_entry'].update(region=1), boundary.check_progression)

    def test_results_need_natural_outcome(self):
        self.bad('progression', lambda p: p['cases'][0]['original']['semantic'].update(natural_outcome=0), boundary.check_progression)

    def test_retirement_must_precede_reacquisition(self):
        self.bad('progression', lambda p: p['cases'][0]['resources']['later_resources'].update(ordinal=1), boundary.check_progression)

    def test_pointer_reuse_is_not_epoch_identity(self):
        self.bad('progression', lambda p: p['cases'][0]['resources']['later_resources'].update(epoch=1), boundary.check_progression)

    def test_persistent_canvas_is_not_destroyed_on_map_exit(self):
        self.bad('progression', lambda p: p['cases'][0]['resources']['retirements'][0]['after'].update(canvas_interfaces=[0]*4), boundary.check_progression)

    def test_original_store_must_retire(self):
        self.bad('progression', lambda p: p['cases'][0]['resources']['retirements'][0]['after']['globals'].update({str(0x6de558):1}), boundary.check_progression)

    def test_native_generation_must_retire(self):
        self.bad('progression', lambda p: p['cases'][0]['native'].update(surviving_command_surfaces=1), boundary.check_progression)

    def test_native_delete_before_create(self):
        self.bad('progression', lambda p: p['cases'][0]['native']['previous_world'].update(deleted=2**31), boundary.check_progression)

    def test_duplicate_session_cannot_supply_two_domains(self):
        self.bad('progression', lambda p: p['cases'][1].update(identity=p['cases'][0]['identity']), boundary.check_progression)

    def test_boolean_is_not_a_natural_outcome(self):
        self.bad('progression', lambda p: p['cases'][0]['original']['semantic'].update(natural_outcome=True), boundary.check_progression)

    def test_boolean_is_not_an_epoch(self):
        self.bad('progression', lambda p: p['cases'][0]['resources']['retirements'][0].update(epoch=True), boundary.check_progression)

    def test_missing_raw_provider_observation(self):
        with self.assertRaises(ValueError):
            boundary.resource_journal(self.reader, dict(artifacts=[]), {})

    def test_oversized_raw_provider_observation(self):
        with self.assertRaises(ValueError):
            boundary.resource_journal(self.reader, dict(artifacts=[dict(path='a/map-resource-boundaries.bin', bytes=2**31)]), {})

    def test_wrong_raw_provider_digest(self):
        c = self.inputs['progression']['cases'][0]
        a = copy.deepcopy(self.reader.json(c['acquisition']['record']))
        next(v for v in a['artifacts'] if v['path'].endswith('/map-resource-boundaries.bin'))['sha256'] = '0'*64
        with self.assertRaises(ValueError): boundary.resource_journal(self.reader, a, c['resources'])

    def bad_policy(self, change):
        p = copy.deepcopy(self.policy); change(p)
        with self.assertRaises(ValueError): boundary.check_domain(p, self.plan)

    def test_no_silent_input_widening(self):
        self.bad_policy(lambda p: p.update(arbitrary_maps_saves_duration_supported=True))

    def test_assets_are_not_observed_map_support(self):
        self.bad_policy(lambda p: p.update(installed_assets_are_supported_maps=True))

    def test_no_journey_or_operation_waiver(self):
        self.bad_policy(lambda p: p.update(operations=20))

    def test_no_outcome_waiver(self):
        self.bad_policy(lambda p: p['required_outcomes'].remove('actual-later-map'))

    def test_no_technical_waiver_from_support_choice(self):
        self.bad_policy(lambda p: p.update(technical_unknowns_waived=True))

    def test_no_new_cohort_without_review(self):
        self.bad_policy(lambda p: p['evidence_cohort'].update(sha256='0'*64))

    def test_complete_analysis_is_read_only(self):
        before = copy.deepcopy(self.parent)
        report = boundary.analyze(self.reader)
        self.assertEqual(self.parent, before)
        self.assertEqual(report['preserved_acceptance_counts'], dict(passed=148, pending=8, total=156))
        self.assertTrue(report['support_promise_selected'])
        self.assertEqual(len(report['demonstrated_campaign_inputs']), 4)


if __name__ == '__main__':
    unittest.main()
