#!/usr/bin/env python3
"""Negative acceptance controls; synthetic reports do not establish live success."""
import copy
from pathlib import Path
import sys
import unittest

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


if __name__ == '__main__':unittest.main()
