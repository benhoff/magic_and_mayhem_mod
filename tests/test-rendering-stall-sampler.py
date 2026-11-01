#!/usr/bin/env python3
"""Exercise actual parent-GDB sampling, exit propagation and conservative timing."""
import hashlib
import json
import os
from pathlib import Path
import shutil
import struct
import subprocess
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT/'tools'))
from rendering_stall_sampler import CAPACITY, SIZE, OwnedRing, PendingAckWatch, run


def header(published=32, ack=0, session=1, state=1, reason=0, cancel=0):
    return b'MNMRDC03'+struct.pack('<14I', 3, SIZE, session, published,
                                  state, reason, cancel, ack, 0, 0, 0, 0, 0, 0)


class WatchTests(unittest.TestCase):
    def test_long_idle_is_not_pending_time_and_sampling_is_one_shot(self):
        w = PendingAckWatch()
        self.assertIsNone(w.observe(header(0), SIZE, 0))
        self.assertIsNone(w.observe(header(0), SIZE, 20))
        self.assertIsNone(w.observe(header(), SIZE, 20.1))
        self.assertIsNone(w.observe(header(), SIZE, 21.59))
        sample = w.observe(header(), SIZE, 21.61)
        self.assertAlmostEqual(sample['pending_seconds'], 1.51)
        self.assertIsNone(w.observe(header(), SIZE, 30))

    def test_ack_progress_and_idle_restart_the_interval(self):
        w = PendingAckWatch()
        w.observe(header(64), SIZE, 0)
        self.assertIsNone(w.observe(header(64, 32), SIZE, 1.4))
        self.assertIsNone(w.observe(header(64, 32), SIZE, 2.8))
        w.observe(header(64, 64), SIZE, 2.9)
        self.assertIsNone(w.observe(header(96, 64), SIZE, 10))
        self.assertIsNotNone(w.observe(header(96, 64), SIZE, 11.51))

    def test_publication_growth_does_not_renew_a_stalled_ack(self):
        w = PendingAckWatch()
        w.observe(header(), SIZE, 0)
        self.assertIsNotNone(w.observe(header(96), SIZE, 1.51))

    def test_unsigned_counter_wrap(self):
        w = PendingAckWatch()
        w.observe(header(16, 0xfffffff0), SIZE, 0)
        self.assertEqual(w.observe(header(16, 0xfffffff0), SIZE, 1.6)['unread_bytes'], 32)

    def test_inactive_cancelled_and_failed_rings_never_trigger(self):
        for values in (dict(state=0), dict(state=2), dict(state=3, reason=4), dict(cancel=1)):
            w = PendingAckWatch()
            self.assertIsNone(w.observe(header(**values), SIZE, 0))
            self.assertIsNone(w.observe(header(**values), SIZE, 10))

    def test_rejects_corruption_session_reuse_and_clock_reversal(self):
        for raw, size in [(header()[:-1], SIZE), (header(), SIZE-1),
                          (header(published=CAPACITY+1), SIZE), (header(session=0), SIZE),
                          (header(state=4), SIZE), (b'BADMAGIC'+header()[8:], SIZE)]:
            with self.assertRaises(ValueError):
                PendingAckWatch().observe(raw, size, 0)
        w = PendingAckWatch()
        w.observe(header(), SIZE, 1)
        with self.assertRaisesRegex(ValueError, 'session changed'):
            w.observe(header(session=2), SIZE, 2)
        with self.assertRaisesRegex(ValueError, 'monotonic'):
            w.observe(header(), SIZE, 0)
        for threshold in (0, 5, float('nan')):
            with self.assertRaises(ValueError):
                PendingAckWatch(threshold)

    def test_ambiguous_observation_restarts_interval(self):
        w = PendingAckWatch()
        w.observe(header(), SIZE, 0)
        w.reset_interval()
        self.assertIsNone(w.observe(header(), SIZE, 10))
        self.assertIsNotNone(w.observe(header(), SIZE, 11.6))


class OwnerTests(unittest.TestCase):
    def test_only_current_sole_admitted_generation_is_selected(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            owner = root/'working/tests/native-output-owner/run-owned/owner.json'
            owner.parent.mkdir(parents=True)
            owner.write_text('{}')
            ring = OwnedRing(root, owner)
            self.assertIsNone(ring.read())
            session = root/'working/experiments/menu-observer/run-current'
            session.mkdir(parents=True)
            path = session/'render-commands.bin'
            with path.open('wb') as f:
                f.write(header());f.truncate(SIZE)
            journal = owner.parent/'sessions';journal.mkdir()
            data = dict(session=str(session), native_only=dict(policy='native-only-rendering-v1'))
            (journal/'one.json').write_text('{')
            self.assertIsNone(ring.read())
            (journal/'one.json').write_text(json.dumps(data))
            self.assertEqual(ring.read(), (header(), SIZE))
            replacement = path.with_suffix('.new')
            replacement.write_bytes(path.read_bytes())
            replacement.replace(path)
            with self.assertRaisesRegex(ValueError, 'replaced'):
                ring.read()
            ring = OwnedRing(root, owner)
            self.assertEqual(ring.read(), (header(), SIZE))
            (journal/'two.json').write_text(json.dumps(data))
            with self.assertRaisesRegex(ValueError, 'exactly one'):
                ring.read()
            (journal/'two.json').unlink()
            data['session'] = str(root/'unowned/run-current')
            (journal/'one.json').write_text(json.dumps(data))
            with self.assertRaisesRegex(ValueError, 'identity'):
                ring.read()


class DiagnosticBoundaryTests(unittest.TestCase):
    def test_nominal_launch_cannot_enable_the_debugger(self):
        env = {k:v for k,v in os.environ.items() if not k.startswith('MNM_')}
        r = subprocess.run([sys.executable, '-B', ROOT/'tools/sample-rendering-stall.py',
                            '--binary', shutil.which('true'), '--repo', ROOT,
                            '--owner', ROOT/'missing-owner', '--output', ROOT/'working/pilot/refused'],
                           env=env, capture_output=True, text=True, timeout=5)
        self.assertEqual(r.returncode, 2)
        self.assertIn('explicitly diagnostic normal collector', r.stderr)
        self.assertFalse((ROOT/'working/pilot/refused').exists())


class ParentDebuggerTests(unittest.TestCase):
    def test_actual_parent_samples_once_continues_and_propagates_exits(self):
        self.assertIsNotNone(shutil.which('gdb'), 'Real parent-debugger control is required')
        parent = Path(os.environ['MNM_STALL_TEST_OUTPUT'])
        for case, expected, sampled in [('sample', 0, True), ('idle', 0, False),
                                        ('nonzero', 1, False), ('crash', 139, False), ('invalid', 125, False)]:
            out = parent/case
            with (parent/(case+'.log')).open('xb') as log:
                result = subprocess.run([sys.executable, '-B', __file__, '--child', case, str(out)],
                                        stdout=log, stderr=subprocess.STDOUT, timeout=12,
                                        env=dict(os.environ, DEBUGINFOD_URLS=''))
            self.assertEqual(result.returncode, expected, case)
            value = json.loads((out/'result.json').read_text())
            text = (parent/(case+'.log')).read_text()
            self.assertEqual(text.count('MNM_WATCHDOG_STACK_BEGIN'), int(sampled), text)
            self.assertEqual(text.count('MNM_WATCHDOG_STACK_END'), int(sampled), text)
            self.assertEqual(value['sample'] is not None, sampled)
            self.assertTrue(value['player_binary_stable'])
            self.assertFalse(value['deadline_or_ownership_changed'])
            if sampled:
                self.assertIn('clock_nanosleep', text)
                self.assertLess(value['sample']['pending_seconds'], 2.2)
                self.assertEqual(value['exit_code'], 0)
            if case == 'invalid':
                self.assertIn('Invalid V3', value['observation_error'])


def child(case, out):
    class FakeRing:
        path = Path('synthetic-ring')
        def read(self):
            if case == 'invalid':return header(published=CAPACITY+1), SIZE
            return header(published=32 if case == 'sample' else 0), SIZE
    import resource
    resource.setrlimit(resource.RLIMIT_CORE, (0, 0))
    binary = shutil.which('sh' if case == 'crash' else 'false' if case == 'nonzero' else 'sleep')
    arguments = ['-c', 'kill -SEGV $$'] if case == 'crash' else [] if case == 'nonzero' else ['2.3' if case == 'sample' else '.15']
    return run(binary, arguments, FakeRing(), out)


if __name__ == '__main__':
    if sys.argv[1:2] == ['--child']:
        raise SystemExit(child(sys.argv[2], Path(sys.argv[3])))
    unittest.main()
