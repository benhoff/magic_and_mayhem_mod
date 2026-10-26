#!/usr/bin/env python3
"""Accounting guards: narrow current proof cannot close unrelated contracts."""
from copy import deepcopy
from pathlib import Path
import sys
import unittest
sys.path.insert(0, str(Path(__file__).resolve().parents[1]/'tools'))
from coverage_claims import scenario_contract
from rendering_frontier import frontier


def fixture():
    behavior = dict(id='RS.colour', title='Bounded colour', kind='recovered', subsystem='rendering',
                    implementation=['renderer/colour.cpp'], scope='Selected branch only',
                    original=[dict(build='fixture', address=0x547d80, scope='One branch')],
                    status=dict(understanding='partial', implementation='scoped', comparison='recorded', integration='headless', replacement='none'),
                    remaining_gaps=['Other branches remain open'])
    a = dict(id='colour-a', level='headless', scope='First fixture', behaviors=['RS.colour'],
             tests=['tests/a.cpp'], evidence=['E.current'])
    b = dict(a, id='colour-b', scope='Second fixture')
    register = dict(behaviors=[behavior], scenarios=[a, b], dispatch_tables=[])
    current = {stage: dict(state='current', evidence=['E.current']) for stage in ('implementation', 'comparison', 'integration')}
    current['replacement'] = dict(state='none', evidence=[])
    audit = dict(errors=[], behaviors=[dict(id='RS.colour', evidence_freshness='stale', current_validation=current)])
    claims = [dict(behavior='RS.colour', scenarios={a['id']: scenario_contract(a)})]
    return register, audit, lambda eid: (claims, {'tests/a.cpp': 'fixture-sha'})


class FrontierTests(unittest.TestCase):
    def test_historical_staleness_does_not_erase_current_subset(self):
        r, a, load = fixture()
        row = frontier(r, a, load)['behaviors'][0]
        self.assertEqual(row['current_validation']['comparison']['state'], 'current')
        self.assertEqual(row['status']['understanding'], 'partial')
        self.assertIsNone(row['complete_branch_coverage'])
        self.assertEqual(row['original'], r['behaviors'][0]['original'])

    def test_current_behavior_is_not_every_scenario(self):
        r, a, load = fixture()
        scenarios = frontier(r, a, load)['behaviors'][0]['scenarios']
        self.assertEqual(scenarios[0]['current_evidence']['integration'], ['E.current'])
        self.assertEqual(scenarios[1]['current_evidence']['integration'], [])

    def test_scenario_test_sources_must_be_pinned(self):
        r, a, load = fixture()
        claims, _ = load('E.current')
        row = frontier(r, a, lambda _: (claims, {}))['behaviors'][0]
        self.assertEqual(row['scenarios'][0]['current_evidence']['integration'], [])

    def test_wrong_integration_boundary_is_not_current(self):
        r, a, _ = fixture()
        r['scenarios'][0]['level'] = 'live_equivalence'
        claims = [dict(behavior='RS.colour', scenarios={'colour-a': scenario_contract(r['scenarios'][0])})]
        row = frontier(r, a, lambda _: (claims, {'tests/a.cpp': 'sha'}))['behaviors'][0]
        self.assertEqual(row['scenarios'][0]['current_evidence']['integration'], [])

    def test_audit_errors_suppress_current_credit(self):
        r, a, _ = fixture()
        a['errors'] = ['Invalid original evidence']
        def forbidden(_):
            self.fail('Invalid audit must not load current claims')
        result = frontier(r, a, forbidden)
        self.assertFalse(result['audit_valid'])
        self.assertEqual(result['behaviors'][0]['current_validation']['comparison'], dict(state='unknown', evidence=[]))

    def test_native_policy_does_not_require_original_recovery(self):
        r, a, load = fixture()
        r['behaviors'][0]['kind'] = 'native_policy'
        r['behaviors'][0]['original'] = []
        a['behaviors'][0]['current_validation']['comparison'] = dict(state='none', evidence=[])
        row = frontier(r, a, load)['behaviors'][0]
        self.assertNotEqual(row['next_action'], 'obtain current original comparison')

    def test_static_discovery_is_not_automatically_a_missing_implementation(self):
        r, a, load = fixture()
        r['behaviors'][0]['status']['implementation'] = 'none'
        r['behaviors'][0]['implementation'] = []
        r['behaviors'][0]['scope'] = 'Static ownership witnesses; no implementation claim'
        row = frontier(r, a, load)['behaviors'][0]
        self.assertEqual(row['next_action'], 'review absent implementation claim')
        self.assertEqual(row['status']['implementation'], 'none')

    def test_recovered_implementation_needs_current_comparison(self):
        r, a, load = fixture()
        a['behaviors'][0]['current_validation']['comparison'] = dict(state='pending', evidence=[])
        self.assertEqual(frontier(r, a, load)['behaviors'][0]['next_action'], 'obtain current original comparison')

    def test_audio_is_excluded_and_unresolved_dispatch_retained(self):
        r, a, load = fixture()
        b = deepcopy(r['behaviors'][0]);b.update(id='AUDIO.other', subsystem='audio', implementation=['audio/pcm.cpp'])
        r['behaviors'].append(b)
        r['dispatch_tables'] = [dict(id='world-unresolved', complete=False, entries=[]), dict(id='world-complete', complete=True, entries=[])]
        result = frontier(r, a, load)
        self.assertEqual([b['id'] for b in result['behaviors']], ['RS.colour'])
        self.assertEqual([d['id'] for d in result['unresolved_dispatch']], ['world-unresolved'])
        self.assertIsNone(result['overall_completion_percent'])

    def test_one_behavior_cannot_close_a_shared_scenario(self):
        r, a, _ = fixture()
        other = deepcopy(r['behaviors'][0]);other['id'] = 'RS.other'
        outside = deepcopy(other);outside.update(id='AUDIO.other', subsystem='audio', implementation=['audio/pcm.cpp'])
        r['behaviors'] += [other, outside]
        r['scenarios'][0]['behaviors'] += ['RS.other', 'AUDIO.other']
        claims = [dict(behavior='RS.colour', scenarios={'colour-a': scenario_contract(r['scenarios'][0])})]
        result = frontier(r, a, lambda _: (claims, {'tests/a.cpp': 'sha'}))
        scenario = result['scenarios'][0]
        self.assertEqual(scenario['current_integrated_behaviors'], ['RS.colour'])
        self.assertEqual(scenario['behaviors_without_current_integration_proof'], ['RS.other'])
        self.assertEqual(scenario['outside_audit_behaviors'], ['AUDIO.other'])
        self.assertFalse(scenario['current_integration_for_all_selected'])


if __name__ == '__main__':
    unittest.main()
