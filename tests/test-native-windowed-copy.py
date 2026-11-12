#!/usr/bin/env python3
"""Refuse missing natural callers, wrong identities and unbound copy output."""
import copy
from pathlib import Path
import sys
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
from native_windowed_copy import launch_arguments, screen_spec, validate


class NaturalCopyProof(unittest.TestCase):
    def setUp(self):
        self.row = [0] * 44
        self.row[1:3] = [0x58c03d, 0x18c03d]
        self.row[10] = 4 | 32
        self.row[15] = 2
        self.row[23] = 17
        self.row[40:44] = [81, 83, 3, 4]
        self.output = dict(verified=True, application_calls=1)

    def check(self, rows=None, output=None, **identity):
        return validate([self.row] if rows is None else rows,
                        self.output if output is None else output,
                        **(dict(module_base=0x400000, module_extent=0x300000) | identity))

    def test_explicit_launch_and_ambient_rejection(self):
        self.assertEqual(launch_arguments({}), [])
        self.assertEqual(launch_arguments({'MNM_ORIGINAL_WINDOWED_DIAGNOSTIC': '0'}), [])
        self.assertEqual(launch_arguments({'MNM_ORIGINAL_WINDOWED_DIAGNOSTIC': '1'}), ['--', '-win'])
        for bad in ('', 'true', '1 -other', '2'):
            with self.assertRaises(ValueError):
                launch_arguments({'MNM_ORIGINAL_WINDOWED_DIAGNOSTIC': bad})

    def test_attached_clipper_output(self):
        proof = self.check()
        self.assertEqual((proof['calls'], proof['command_count']), (1, 3))
        self.assertEqual(proof['callers'], {'0x58c03d': 1})

    def test_original_windowed_desktop_precondition(self):
        self.assertEqual(screen_spec(True), '1280x1024x16')
        self.assertEqual(screen_spec(False), '1280x1024x24')

    def test_no_branch_and_cpu_only_are_not_gpu_proof(self):
        for field, value in ((10, 4), (15, 1)):
            with self.subTest(field=field):
                row = self.row[:]; row[field] = value
                with self.assertRaises(ValueError): self.check(rows=[row])

    def test_corrupt_natural_call_refuses(self):
        for field, value in ((11, 1), (10, 32), (23, 0), (1, 0x79000000),
                             (2, 0x18c03e), (40, 0), (41, 80), (42, 0),
                             (43, 0), (13, 0x88760096)):
            with self.subTest(field=field, value=value):
                row = self.row[:]; row[field] = value
                with self.assertRaises(ValueError): self.check(rows=[row])

    def test_missing_independent_output_refuses(self):
        for output in ({}, dict(verified=False, application_calls=1),
                       dict(verified=True, application_calls=2)):
            with self.assertRaises(ValueError): self.check(output=output)

    def test_unknown_module_refuses(self):
        for identity in (dict(module_base=0), dict(module_extent=0)):
            with self.assertRaises(ValueError): self.check(**identity)

    def test_observer_inputs_unchanged(self):
        before = copy.deepcopy((self.row, self.output))
        self.check()
        self.assertEqual((self.row, self.output), before)


if __name__ == '__main__': unittest.main()
