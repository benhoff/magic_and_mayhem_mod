#!/usr/bin/env python3
"""Negative acceptance controls; synthetic reports do not establish live success."""
import copy
from pathlib import Path
import sys
import unittest
import subprocess

sys.path.insert(0, str(Path(__file__).resolve().parents[1]/'tools'))
from test_native_restart_support import validate_flow


class RestartAcceptance(unittest.TestCase):
    def fixture(self):
        names = ['main', 'handoff', 'live-world', 'producer-failure-injected', 'terminal-cleanup',
                 'terminal-stable', 'hard-restart-click', 'main', 'handoff', 'live-world',
                 'fresh-session-confirmed', 'normal-main-quit', 'producer-finished']
        events = [dict(step=name, milliseconds=i*1000) for i, name in enumerate(names)]
        for index, cycle in [(2, 0), (9, 1)]:
            events[index].update(cycle=cycle, root='session-'+str(cycle), identity=42+cycle,
                                 launcher_pid=100+cycle, renderer='test', source_generation=3, frames=5, paints=4)
        events[3]['reason'] = 5
        events[4].update(error='injected', state={k: True for k in ('terminal', 'owners_retired', 'timers_stopped',
                         'frame_cleared', 'input_detached', 'input_suspended', 'restart_available', 'retry_disabled', 'fallback_disabled')})
        events[5]['milliseconds_observed'] = 500
        events[10]['old_launcher_gone'] = True
        events[-1].update(normal_quit_accepted=True, stage=11)
        return dict(success=True, error='', events=events)

    def test_accept_complete(self):
        self.assertTrue(validate_flow(self.fixture())['success'])

    def test_reject_missing_each_transition(self):
        for index in range(13):
            flow = self.fixture();del flow['events'][index]
            with self.subTest(index=index), self.assertRaises(ValueError):validate_flow(flow)

    def test_reject_incomplete_or_reused_generation(self):
        mutations = [(9, 'root', 'session-0'), (9, 'identity', 42), (9, 'launcher_pid', 100),
                     (9, 'source_generation', 0), (9, 'paints', 0), (10, 'old_launcher_gone', False),
                     (5, 'milliseconds_observed', 499), (3, 'reason', 0), (12, 'normal_quit_accepted', False)]
        for index, key, value in mutations:
            flow = self.fixture();flow['events'][index][key] = value
            with self.subTest(key=key), self.assertRaises(ValueError):validate_flow(flow)

    def test_reject_any_unclean_terminal_state(self):
        initial = self.fixture()
        for key in initial['events'][4]['state']:
            flow = copy.deepcopy(initial);flow['events'][4]['state'][key] = False
            with self.subTest(key=key), self.assertRaises(ValueError):validate_flow(flow)

    def worker(self):
        flow=self.fixture()
        for event in flow['events']:
            if event['step']=='live-world':
                event.update(render_owner='dedicated-thread',command_stream_version=5,transport_version=3,cpu_materializations=0)
        return flow

    def test_worker_v5_observations_are_explicit(self):
        result=validate_flow(self.worker(),expected_owner='dedicated-thread',expected_stream_version=5)
        self.assertEqual(result['command_stream_version'],5)
        self.assertEqual(result['transport_version'],3)
        self.assertFalse(result['live_pending_cpu_read_fault_proved'])

    def test_requested_worker_does_not_authorize_other_or_missing_observations(self):
        for index in [2,9]:
            for key,value in [('render_owner','gui-resident'),('render_owner',None),('command_stream_version',3),
                              ('command_stream_version','5'),('transport_version',5),('transport_version',True),
                              ('cpu_materializations',-1),('cpu_materializations',True),('cpu_materializations',None)]:
                flow=self.worker();flow['events'][index][key]=value
                with self.subTest(index=index,key=key,value=value),self.assertRaises(ValueError):
                    validate_flow(flow,expected_owner='dedicated-thread',expected_stream_version=5)

    def test_legacy_report_cannot_supply_explicit_worker_proof(self):
        with self.assertRaises(ValueError):validate_flow(self.fixture(),expected_owner='dedicated-thread',expected_stream_version=5)

    def test_unknown_expected_profile_refuses(self):
        for version in [True,0,6,'5']:
            with self.subTest(version=version),self.assertRaises(ValueError):
                validate_flow(self.worker(),expected_owner='dedicated-thread',expected_stream_version=version)

    def test_worker_cli_refuses_unrelated_journeys_before_inputs(self):
        root=Path(__file__).resolve().parents[1]
        for extra in [[],['--native-only'],['--restart']]:
            p=subprocess.run([sys.executable,str(root/'tools/test-native-owned-campaign.py'),'--independent-world',
                '--source-game','missing','--claims','missing','--report','missing',*extra],capture_output=True,text=True,timeout=15)
            self.assertEqual(p.returncode,2)
            self.assertIn('Independent World requires',p.stderr)


if __name__ == '__main__':unittest.main()
