#!/usr/bin/env python3
"""Reject missing epoch timing and fake progress without new game execution."""
import importlib.util
from pathlib import Path
import sys
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT/'tools'))
spec = importlib.util.spec_from_file_location('broader_case', ROOT/'tools/check-broader-player-case.py')
m = importlib.util.module_from_spec(spec)
spec.loader.exec_module(m)


class EpochProgress(unittest.TestCase):
    def setUp(self):
        self.epoch = dict(resident=dict(completed='3', engine_fps=0,
            stage_profile=dict(samples_dropped=0,
                samples=[dict(generation=str(i), producer_wait_ack_ms=100,
                              native_consume_ms=50) for i in range(1, 4)],
                capture=dict(peak_queue_bytes=1024, queue_limit_bytes=256*1024**2,
                             queue_bytes=0, file_bytes=2048))))
        self.comparison = dict(world_domain=dict(worlds=[1, 2, 3]),
                               comparison=dict(world_presentations=5))

    def check(self):
        return m.epoch_progress(self.epoch, self.comparison)

    def test_finite_epoch_accepts_without_fps_floor(self):
        self.assertTrue(self.check()['verified'])
        self.assertIsNone(self.check()['fixed_fps_threshold'])

    def test_missing_deadline_measurement_refused(self):
        del self.epoch['resident']['stage_profile']['samples'][1]['producer_wait_ack_ms']
        with self.assertRaisesRegex(ValueError, 'measurement'):
            self.check()

    def test_nonfinite_deadline_measurement_refused(self):
        self.epoch['resident']['stage_profile']['samples'][1]['native_consume_ms'] = float('nan')
        with self.assertRaisesRegex(ValueError, 'measurement'):
            self.check()

    def test_missing_timing_generation_refused(self):
        self.epoch['resident']['stage_profile']['samples'].pop()
        with self.assertRaisesRegex(ValueError, 'history'):
            self.check()

    def test_original_deadline_is_not_widened(self):
        self.epoch['resident']['stage_profile']['samples'][0]['producer_wait_ack_ms'] = 120000
        with self.assertRaisesRegex(ValueError, 'deadline'):
            self.check()

    def test_pending_capture_queue_refused(self):
        self.epoch['resident']['stage_profile']['capture']['queue_bytes'] = 1
        with self.assertRaisesRegex(ValueError, 'bound'):
            self.check()

    def test_repaints_without_source_world_are_not_progress(self):
        self.comparison['world_domain']['worlds'] = []
        with self.assertRaisesRegex(ValueError, 'progress'):
            self.check()


if __name__ == '__main__':
    unittest.main()
