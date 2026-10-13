#!/usr/bin/env python3
"""Storage guards: references, worktrees, symlinks and preflight boundary checks."""
import importlib.util
import json
from pathlib import Path
import subprocess
import tempfile
from types import SimpleNamespace
import unittest
from unittest.mock import Mock, patch

spec = importlib.util.spec_from_file_location('retention', Path(__file__).resolve().parents[1] / 'tools/working-retention.py')
retention = importlib.util.module_from_spec(spec)
spec.loader.exec_module(retention)


class Storage(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.root = Path(self.tmp.name)
        subprocess.run(['git', 'init', '-q', self.root], check=True)
        self.rules = {'schema': 1, 'max_working_gib': 200, 'minimum_free_gib': 100,
                      'recent_hours': 0, 'keep_latest_runs': 0, 'protected_names': []}

    def make(self, name):
        p = self.root / name
        p.mkdir(parents=True)
        (p / 'payload').write_bytes(b'x' * 4096)
        return p

    def test_exact_budget_and_reservation(self):
        g = retention.GIB
        self.assertEqual(retention.pressure(199*g, 101*g, self.rules, g), [])
        self.assertEqual(len(retention.pressure(199*g, 101*g, self.rules, 2*g)), 2)

    def test_free_space_only_reservation_boundary_ignores_working_size(self):
        rules = dict(self.rules, max_working_gib=None)
        g = retention.GIB
        self.assertEqual(retention.pressure(9999*g, 101*g, rules, g), [])
        self.assertEqual(len(retention.pressure(0, 101*g, rules, g+1)), 1)
        self.assertEqual(len(retention.pressure(0, 99*g, rules)), 1)

    def test_free_space_only_policy_accepts_null_or_absent_maximum(self):
        p = self.root / 'policy.json'
        for omit in (False, True):
            rules = dict(self.rules, max_working_gib=None)
            if omit:del rules['max_working_gib']
            p.write_text(json.dumps(rules))
            self.assertIsNone(retention.policy(p).get('max_working_gib'))

    def test_free_space_only_inventory_keeps_usage_and_reports_shortfall(self):
        self.make('working/tests/unit/run-old')
        with patch.object(retention.shutil, 'disk_usage', return_value=SimpleNamespace(free=99*retention.GIB)):
            report = retention.inventory(self.root, dict(self.rules, max_working_gib=None))
        self.assertGreater(report['working_allocated_bytes'], 0)
        self.assertIsNone(report['over_budget_bytes'])
        self.assertEqual(report['free_space_shortfall_bytes'], retention.GIB)
        self.assertEqual(len(report['pressure']), 1)

    def test_free_space_only_launch_skips_du_and_preserves_exit_status(self):
        p = self.root / 'policy.json'
        p.write_text(json.dumps(dict(self.rules, max_working_gib=None)))
        for free, expected in ((101, 7), (100, 2)):
            with patch.object(retention, 'ROOT', self.root), \
                    patch.object(retention.sys, 'argv', ['working-retention.py', '--policy', str(p), 'run', '--reserve-gib', '1', '--', 'capture']), \
                    patch.object(retention, 'usage') as scan, \
                    patch.object(retention.shutil, 'disk_usage', return_value=SimpleNamespace(free=free*retention.GIB)), \
                    patch.object(retention, 'guarded_run', return_value=7) as launch:
                self.assertEqual(retention.main(), expected)
                scan.assert_not_called()
                if expected == 7:launch.assert_called_once_with(['capture'], self.root, retention.policy(p))
                else:launch.assert_not_called()

    def test_auto_archive_keeps_launch_reservation_in_free_space_target(self):
        p = self.root / 'policy.json'
        p.write_text(json.dumps(dict(self.rules, max_working_gib=None, auto_archive=True)))
        maintain = Mock(return_value={'failures': []})
        with patch.object(retention, 'ROOT', self.root), \
                patch.object(retention.sys, 'argv', ['working-retention.py', '--policy', str(p), 'run', '--reserve-gib', '11', '--', 'capture']), \
                patch.object(retention, 'usage', return_value={str(self.root/'working'):9999*retention.GIB}), \
                patch.object(retention.shutil, 'disk_usage', side_effect=[SimpleNamespace(free=n*retention.GIB) for n in (110,112)]), \
                patch.dict('sys.modules', {'working_archive': SimpleNamespace(maintain=maintain)}), \
                patch.object(retention, 'guarded_run', return_value=0):
            self.assertEqual(retention.main(), 0)
        self.assertIsNone(maintain.call_args.args[1]['max_working_gib'])
        self.assertEqual(maintain.call_args.args[1]['minimum_free_gib'], 111)

    def test_disappearing_capture_reply_keeps_complete_root_measurement(self):
        working = self.make('working')
        result = subprocess.CompletedProcess(['du'], 1,
            f'4096\t{working}\0'.encode(),
            f"du: cannot access '{working}/capture/reply': No such file or directory\n".encode())
        with patch.object(retention.subprocess, 'run', return_value=result):
            self.assertEqual(retention.usage(working), {str(working): 4096})

    def test_inventory_refuses_other_errors_and_missing_root_total(self):
        working = self.make('working')
        missing = f"du: cannot access '{working}/reply': No such file or directory\n"
        for code, diagnostic in [(1, missing + 'du: cannot read directory: Permission denied\n'),
                                 (1, "du: cannot access '/outside/reply': No such file or directory\n"),
                                 (2, missing), (1, '')]:
            with self.subTest(code=code, diagnostic=diagnostic), patch.object(retention.subprocess, 'run',
                    return_value=subprocess.CompletedProcess(['du'], code,
                        f'4096\t{working}\0'.encode(), diagnostic.encode())):
                with self.assertRaises(subprocess.CalledProcessError):retention.usage(working)
        with patch.object(retention.subprocess, 'run',
                return_value=subprocess.CompletedProcess(['du'], 1, b'', missing.encode())):
            with self.assertRaises(ValueError):retention.usage(working)

    def test_referenced_run_and_worktree_protected_other_run_reviewable(self):
        p = self.make('working/experiments/scene/run-reference')
        self.make('working/experiments/scene/run-candidate')
        tree = self.make('working/branch')
        research = self.root / 'research'
        research.mkdir()
        (research / 'proof.json').write_text(json.dumps({'input': str(p / 'payload')}))
        subprocess.run(['git', '-C', self.root, 'add', 'research/proof.json'], check=True)
        with patch.object(retention, 'worktrees', return_value=[self.root, tree]):
            report = retention.inventory(self.root, self.rules)
        rows = {r['path']: r for r in report['units']}
        self.assertIn('tracked evidence/research reference', rows[p.relative_to(self.root).as_posix()]['reasons'])
        self.assertIn('registered Git worktree', rows['working/branch']['reasons'])
        self.assertEqual(rows['working/experiments/scene/run-candidate']['classification'], 'review_for_cleanup')
        self.assertTrue((p / 'payload').exists())

    def test_symlink_to_original_not_traversed_and_pin_respected(self):
        outside = self.make('original/media')
        run = self.make('working/tests/unit/run-pinned')
        (run / '.retain').touch()
        (self.root / 'working/alias').symlink_to(outside, target_is_directory=True)
        report = retention.inventory(self.root, self.rules)
        self.assertEqual(len(report['units']), 1)
        self.assertIn('.retain marker', report['units'][0]['reasons'])
        self.assertTrue((outside / 'payload').exists())

    def test_usage_refuses_symlink_root(self):
        p = self.make('real')
        alias = self.root / 'working'
        alias.symlink_to(p, target_is_directory=True)
        with self.assertRaises(ValueError):
            retention.usage(alias)

    def test_invalid_policy_rejected(self):
        p = self.root / 'policy.json'
        for update in [{'max_working_gib': -1}, {'max_working_gib': 0}, {'max_working_gib': True},
                       {'max_working_gib': 'none'}, {'max_working_gib': float('nan')},
                       {'minimum_free_gib': float('inf')}, {'protected_names': ['../original']},
                       {'keep_latest_runs': .5}, {'monitor_seconds': 0}, {'stop_grace_seconds': float('nan')}]:
            p.write_text(json.dumps(dict(self.rules, **update)))
            with self.assertRaises(ValueError):
                retention.policy(p)

    def test_guard_refuses_command_when_over_budget(self):
        p = self.root / 'policy.json'
        p.write_text(json.dumps(self.rules))
        with patch.object(retention, 'ROOT', self.root), \
                patch.object(retention.sys, 'argv', ['working-retention.py', '--policy', str(p), 'run', '--reserve-gib', '1', '--', 'unexpected-command']), \
                patch.object(retention, 'usage', return_value={str(self.root / 'working'): 201 * retention.GIB}), \
                patch.object(retention.shutil, 'disk_usage', return_value=SimpleNamespace(free=150 * retention.GIB)), \
                patch.object(retention.subprocess, 'Popen') as execute:
            self.assertEqual(retention.main(), 2)
            execute.assert_not_called()

    def test_running_guard_stops_group_after_free_space_drops(self):
        child = Mock(pid=987654)
        child.poll.return_value = None
        child.wait.side_effect = [subprocess.TimeoutExpired('capture', 2), 0, 0]
        with patch.object(retention.subprocess, 'Popen', return_value=child) as launch, \
                patch.object(retention.shutil, 'disk_usage', side_effect=[
                    SimpleNamespace(free=101 * retention.GIB), SimpleNamespace(free=99 * retention.GIB)]), \
                patch.object(retention.os, 'killpg') as kill:
            self.assertEqual(retention.guarded_run(['capture'], self.root, self.rules), 2)
        launch.assert_called_once_with(['capture'], cwd=self.root, start_new_session=True)
        self.assertEqual(kill.call_args_list, [unittest.mock.call(child.pid, retention.signal.SIGTERM),
                                              unittest.mock.call(child.pid, retention.signal.SIGKILL)])

    def test_guard_force_kills_when_term_times_out(self):
        child = Mock(pid=987654)
        child.poll.return_value = None
        child.wait.side_effect = [subprocess.TimeoutExpired('capture', 5), -9]
        with patch.object(retention.subprocess, 'Popen', return_value=child), \
                patch.object(retention.shutil, 'disk_usage', return_value=SimpleNamespace(free=99 * retention.GIB)), \
                patch.object(retention.os, 'killpg') as kill:
            self.assertEqual(retention.guarded_run(['capture'], self.root, self.rules), 2)
        kill.assert_any_call(child.pid, retention.signal.SIGKILL)
        self.assertEqual(child.wait.call_count, 2)

    def test_interrupt_and_failed_monitor_stop_owned_group(self):
        for failure in (KeyboardInterrupt(), OSError('measurement failed')):
            child = Mock(pid=987654)
            child.poll.return_value = None
            with patch.object(retention.subprocess, 'Popen', return_value=child), \
                    patch.object(retention.shutil, 'disk_usage', side_effect=failure), \
                    patch.object(retention.os, 'killpg') as kill:
                if isinstance(failure, KeyboardInterrupt):
                    self.assertEqual(retention.guarded_run(['capture'], self.root, self.rules), 130)
                else:
                    with self.assertRaises(OSError):
                        retention.guarded_run(['capture'], self.root, self.rules)
            kill.assert_any_call(child.pid, retention.signal.SIGTERM)

    def test_healthy_command_preserves_exit_status(self):
        self.make('working')
        with patch.object(retention.shutil, 'disk_usage', return_value=SimpleNamespace(free=100 * retention.GIB)), \
                patch.object(retention.os, 'killpg') as kill:
            self.assertEqual(retention.guarded_run([retention.sys.executable, '-c', 'raise SystemExit(7)'],
                                                 self.root, self.rules), 7)
        kill.assert_not_called()


if __name__ == '__main__':
    unittest.main()
