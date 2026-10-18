#!/usr/bin/env python3
"""Candidate scanner controls, independent of original executable artifacts."""
import importlib.util
from pathlib import Path
import struct
import unittest

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('branches', ROOT/'tools/inspect-pixel-route-branches.py')
module = importlib.util.module_from_spec(spec)
spec.loader.exec_module(module)


class Branches(unittest.TestCase):
    def test_all_short_encodings_both_directions(self):
        for op in (0xeb, *range(0x70, 0x80), *range(0xe0, 0xe4)):
            for delta in (-128, -1, 0, 127):
                with self.subTest(op=op, delta=delta):
                    row = next(module.branches(bytes((op, delta & 255)), 0x500000))
                    self.assertEqual(row['target'], 0x500002+delta)

    def test_near_call_jump_and_all_conditional_encodings(self):
        for op in (b'\xe8', b'\xe9', *(bytes((15, x)) for x in range(0x80, 0x90))):
            for delta in (-0x123456, 0x123456):
                code = op+struct.pack('<i', delta)
                row = next(module.branches(code, 0x500000))
                self.assertEqual(row['target'], 0x500000+len(code)+delta)

    def test_far_operands_preserve_unknown_selector(self):
        for op in (0x9a, 0xea):
            row = next(module.branches(bytes((op,))+struct.pack('<IH', 0x5532f0, 0x23), 0x400000))
            self.assertEqual((row['target'], row['selector']), (0x5532f0, 0x23))
            self.assertIs(row['runtime_linear_target_known'], False)

    def test_truncated_branches_are_not_decoded(self):
        for code in (b'\xeb', b'\xe0', b'\xe8\0\0\0', b'\x0f\x80\0\0\0', b'\xea\0\0\0\0\0'):
            self.assertEqual(list(module.branches(code, 0x500000)), [])

    def test_external_interior_target_and_exclusive_end(self):
        code = b'\xe9'+struct.pack('<i', 0x105-5)
        rows = module.scan(code, 0x500000, {'interior': (0x500100, 0x500110),
                                               'end': (0x500100, 0x500105)})
        self.assertEqual(rows['interior']['external_candidates'][0]['target'], 0x500105)
        self.assertEqual(rows['end']['external_candidates'], [])

    def test_internal_edges_do_not_create_external_callers(self):
        row = module.scan(b'\xeb\xfe', 0x500000, {'route': (0x500000, 0x500002)})['route']
        self.assertEqual(row['internal_candidates'], 1)
        self.assertEqual(row['external_candidates'], [])

    def test_overlapping_operand_interpretations_are_retained(self):
        # Actual normal alignment is ADD EBP,EBX / INC EAX.
        code = bytes.fromhex('03eb40')
        rows = module.scan(code, 0x54b5d4, {'route': (0x54b5f0, 0x54b7fb)})
        self.assertEqual(rows['route']['external_candidates'][0]['site'], 0x54b5d5)
        self.assertEqual(rows['route']['external_candidates'][0]['target'], 0x54b617)

    def test_wrapping_relative_target(self):
        self.assertEqual(next(module.branches(b'\xeb\x00', 0xfffffffe))['target'], 0)

    def test_camera_tail_is_inside_the_reviewed_range(self):
        self.assertEqual(module.WINDOWS['minimap/camera3'], (0x5532f0, 0x5536bc))
        rows = module.scan(bytes.fromhex('d1e089'), 0x553650,
                           {'camera': module.WINDOWS['minimap/camera3']})
        self.assertEqual(rows['camera']['external_candidates'], [])
        self.assertEqual(rows['camera']['internal_candidates'], 1)

    def test_unsupported_original_is_refused(self):
        with self.assertRaisesRegex(ValueError, 'Unsupported original'):
            module.inspect(b'Not the pinned game')


if __name__ == '__main__':
    unittest.main()
