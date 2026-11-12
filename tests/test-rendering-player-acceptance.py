#!/usr/bin/env python3
"""Reject partial cohorts, source drift and unproved expanded requirement scope."""
import copy
import importlib.util
import json
from pathlib import Path
import sys
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT/'tools'))
from rendering_player_acceptance import first_presented, progress

spec = importlib.util.spec_from_file_location('normal_fixture', ROOT/'tests/test-rendering-normal-acceptance.py')
fixture = importlib.util.module_from_spec(spec)
spec.loader.exec_module(fixture)


class CohortTests(unittest.TestCase):
    def setUp(self):
        self.reader = fixture.Reader()
        self.plan = json.loads((ROOT/'docs/native-rendering-closure.json').read_text())
        self.cohort = []
        for configuration in ('player-default', 'player-software'):
            for scenario in ('normal-storage', 'normal-quick'):
                for size in ([800, 600], [640, 480]):
                    pair = self.reader.pair(scenario, len(self.cohort))
                    a = self.reader.data[pair['acquisition']['record']]
                    a['session_identity'] = f'{len(self.cohort)+1:032x}'
                    a['configuration']['software_flag'] = configuration == 'player-software'
                    a['resident']['renderer'] = 'llvmpipe (controlled)' if a['configuration']['software_flag'] else 'controlled GPU'
                    c = self.reader.data[pair['comparison']['record']]
                    c['world_domain']['extents'] = [size]
                    c['first_presented_world'] = dict(verified=True)
                    c['original_menu_actions'].update(
                        campaign_selection=dict(verified=True, callbacks=[[3, 0], [18, 65537], [18, 0]]),
                        display_rebuilds=dict(verified=True, dimensions=[[640, 480], [800, 600]] if size == [800, 600] else [[800, 600], [640, 480]], callback_sequences=[20, 30]))
                    c['cpu_delivery'] = dict(request_count=2, complete_reply_matches=2, counters=dict(completed=2), aggregate_completion_reconciles=True, terminal_reply_matches=True)
                    self.reader.refresh(pair)
                    self.cohort.append((configuration, pair))

    def test_only_finite_leaves_pass(self):
        result = progress(self.plan, self.reader, self.cohort)
        self.assertEqual((result['passed_requirements'], result['pending_requirements']), (16, 140))
        self.assertEqual(set(result['decisions'].values()), {'pending'})
        self.assertFalse(result['historical_failures_retired'])
        self.assertTrue(all(r['state'] == 'pending' for r in result['requirements']
                            if r['kind'] != 'journey' or r['requirement'] == 'first-composed-world'))

    def test_missing_duplicate_session_and_domain_refuse(self):
        with self.assertRaises(ValueError): progress(self.plan, self.reader, self.cohort[:-1])
        pair = self.cohort[-1][1]
        a = self.reader.data[pair['acquisition']['record']]
        a['session_identity'] = '0'*31+'1'
        self.reader.refresh(pair)
        with self.assertRaises(ValueError): progress(self.plan, self.reader, self.cohort)

    def test_unbound_callback_rebuild_and_cpu_completions_refuse(self):
        pair = self.cohort[0][1]
        c = self.reader.data[pair['comparison']['record']]
        original = copy.deepcopy(c)
        for mutate in (lambda x: x['original_menu_actions']['campaign_selection'].update(verified=False),
                       lambda x: x['original_menu_actions']['display_rebuilds'].update(dimensions=[[640, 480]]),
                       lambda x: x['original_menu_actions']['display_rebuilds'].update(callback_sequences=[20, 20]),
                       lambda x: x['cpu_delivery']['counters'].update(completed=1),
                       lambda x: x['cpu_delivery'].update(terminal_reply_matches=False),
                       lambda x: x['first_presented_world'].update(verified=False)):
            self.reader.data[pair['comparison']['record']] = c = copy.deepcopy(original)
            mutate(c); self.reader.refresh(pair)
            with self.assertRaises(ValueError): progress(self.plan, self.reader, self.cohort)

    def test_current_source_phase_required(self):
        self.reader.sources['apps/qt-shell/main.cpp'] = 'f'*64
        with self.assertRaises(ValueError): progress(self.plan, self.reader, self.cohort)

    def test_first_actual_swap_binds_coalesced_generation_and_full_plane(self):
        observed = dict(swaps=[1, 3], paints={1: (dict(world='0'), b''), 3: (dict(world='7', width=800, height=600), b'')})
        match = dict(paint=3, world=7, pixels=480000, mismatching_pixels=0, actual_sha256='a'*64, expected_sha256='a'*64)
        comparison = dict(comparisons=[match])
        reference = dict(continuous_from_owned_initial_history=True, worlds=7)
        result = first_presented(observed, comparison, reference)
        self.assertEqual(result['generation'], 7)
        self.assertFalse(result['complete_caller_preparation_proved'])
        for key, value in [('world', 6), ('pixels', 1), ('mismatching_pixels', 1), ('expected_sha256', 'b'*64)]:
            bad = dict(match, **{key: value})
            with self.assertRaises(ValueError): first_presented(observed, dict(comparisons=[bad]), reference)
        reference['worlds'] = 6
        with self.assertRaises(ValueError): first_presented(observed, comparison, reference)


if __name__ == '__main__': unittest.main()
