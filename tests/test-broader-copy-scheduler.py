#!/usr/bin/env python3
"""Owned-child scheduling cannot affect unrelated launches or waive evidence."""
import hashlib
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import types
import unittest
from unittest.mock import Mock, patch
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
from broader_copy_diagnostic_scheduler import CopyDiagnosticScheduler, POLICY
from broader_hotspot_scheduled_launch import WorldBinding, BaseWorldBinding


class Scheduler(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory(); self.addCleanup(self.tmp.cleanup)
        self.root = Path(self.tmp.name); (self.root / 'working').mkdir()
        self.binary = self.root / 'Native'; self.binary.write_bytes(b'exact-native')
        self.sha = hashlib.sha256(self.binary.read_bytes()).hexdigest()
        self.cpu = min(os.sched_getaffinity(0))
        self.policy = dict(policy=POLICY, cpu=self.cpu, main_nice=19, child_only=True)
        self.scheduler = CopyDiagnosticScheduler(self.root, self.binary, self.sha, self.policy)
        self.directory = self.root / 'working' / 'owned'
        self.environment = dict(MNM_NATIVE_COPY_DIAGNOSTIC_SCOPE='all', MNM_NATIVE_COPY_DIAGNOSTICS=str(self.directory), untouched='kept')

    def test_unrelated_launch_passes_exact_arguments(self):
        parent = Mock(); command = ['/unrelated/helper', 'argument']
        self.scheduler.wrap(parent)(command, env={'unrelated': 'same'}, stdout=17)
        parent.assert_called_once_with(command, env={'unrelated': 'same'}, stdout=17)
        self.assertEqual(self.scheduler.records, [])

    def test_binary_drift_refuses_before_launch(self):
        self.binary.write_bytes(b'changed'); parent = Mock()
        with self.assertRaisesRegex(ValueError, 'identity changed'):
            self.scheduler.wrap(parent)([str(self.binary)], env=self.environment)
        parent.assert_not_called()

    def test_foreign_diagnostic_directory_refuses(self):
        self.environment['MNM_NATIVE_COPY_DIAGNOSTICS'] = str(self.root / 'foreign')
        parent = Mock()
        with self.assertRaisesRegex(ValueError, 'not owned'):
            self.scheduler.wrap(parent)([str(self.binary)], env=self.environment)
        parent.assert_not_called()

    def test_existing_marker_refuses_before_launch(self):
        self.directory.mkdir(); (self.directory / 'copies.jsonl').touch(); parent = Mock()
        with self.assertRaisesRegex(ValueError, 'already exists'):
            self.scheduler.wrap(parent)([str(self.binary)], env=self.environment)
        parent.assert_not_called()

    def test_partial_selection_refuses(self):
        self.environment['MNM_NATIVE_COPY_DIAGNOSTIC_SCOPE'] = 'distinct'
        with self.assertRaisesRegex(ValueError, 'all-copy'):
            self.scheduler.wrap(Mock())([str(self.binary)], env=self.environment)

    def test_shell_launch_refuses(self):
        with self.assertRaisesRegex(ValueError, 'direct argv'):
            self.scheduler.wrap(Mock())([str(self.binary)], env=self.environment, shell=True)

    def test_unknown_policy_or_priority_refuses(self):
        for change in (dict(policy='other'), dict(main_nice=-1), dict(cpu=-1), dict(child_only=False)):
            with self.subTest(change=change), self.assertRaises(ValueError):
                CopyDiagnosticScheduler(self.root, self.binary, self.sha, self.policy | change)

    def test_exact_launch_keeps_environment_and_sets_only_child_nice(self):
        self.directory.mkdir(); child = Mock(pid=123); child.poll.return_value = None
        def spawn(command, **kwargs):
            self.assertEqual(command, ['taskset', '--cpu-list', str(self.cpu), str(self.binary), 'same-arg'])
            self.assertIs(kwargs['env'], self.environment)
            (self.directory / 'copies.jsonl').touch(); return child
        threads = {'123': dict(nice=0, affinity=[self.cpu]), '124': dict(nice=0, affinity=[self.cpu]), '125': dict(nice=19, affinity=sorted(os.sched_getaffinity(0)))}
        with patch.object(self.scheduler, '_identity'), patch.object(self.scheduler, '_snapshot', return_value=threads), patch('broader_copy_diagnostic_scheduler.os.getpriority', side_effect=[0, 19]), patch('broader_copy_diagnostic_scheduler.os.sched_getaffinity', return_value={self.cpu}), patch('broader_copy_diagnostic_scheduler.os.setpriority') as priority:
            self.assertIs(self.scheduler.wrap(spawn)([str(self.binary), 'same-arg'], env=self.environment), child)
            self.assertTrue(self.scheduler.ready.wait(2)); receipt = self.scheduler.receipt()
            priority.assert_called_once_with(os.PRIO_PROCESS, 123, 19)
        self.assertFalse(receipt['parent_scheduling_changed'])
        self.assertEqual(receipt['launches'][0]['threads_before'], threads)
        with self.assertRaisesRegex(ValueError, 'only one'):
            self.scheduler.wrap(Mock())([str(self.binary)], env=self.environment)

    def test_identity_refusal_does_not_change_priority(self):
        self.directory.mkdir(); marker = self.directory / 'copies.jsonl'; marker.touch()
        child = Mock(pid=123); child.poll.return_value = None; record = {}
        with patch.object(self.scheduler, '_identity', side_effect=ValueError('foreign child')), patch('broader_copy_diagnostic_scheduler.os.setpriority') as priority:
            self.scheduler._observe(child, marker, record)
            priority.assert_not_called()
        self.assertFalse(record['applied']); self.assertIn('foreign child', record['error'])

    def test_exited_child_lacks_policy_receipt(self):
        child = Mock(pid=123); child.poll.return_value = 0; record = {}
        self.scheduler._observe(child, self.directory / 'copies.jsonl', record)
        self.assertFalse(record['applied'])
        with self.assertRaisesRegex(ValueError, 'successful owned-child'):
            self.scheduler.receipt()

    def test_binding_sits_below_parent_and_restores_original_popen(self):
        binding = WorldBinding.__new__(WorldBinding); original = Mock(); wrapped = Mock()
        binding.original_popen = original; binding.scheduling = Mock(); binding.scheduling.wrap.return_value = wrapped
        with patch.object(BaseWorldBinding, 'install') as install:
            binding.install(); install.assert_called_once(); self.assertIs(binding.popen, wrapped)
        before = subprocess.Popen
        try:
            with patch.object(BaseWorldBinding, 'restore', side_effect=ValueError('cleanup')):
                with self.assertRaisesRegex(ValueError, 'cleanup'): binding.restore()
            self.assertIs(subprocess.Popen, original); self.assertIs(binding.popen, original)
        finally: subprocess.Popen = before

    def test_collector_body_only_changes_binding_import(self):
        root = Path(__file__).resolve().parents[1]
        before = (root / 'tools/run-broader-hotspot-visible-world.py').read_text()
        after = (root / 'tools/run-broader-hotspot-scheduled-world.py').read_text()
        self.assertEqual(after.replace('from broader_hotspot_scheduled_launch import WorldBinding', 'from broader_hotspot_visible_launch import WorldBinding'), before)


if __name__ == '__main__': unittest.main()
