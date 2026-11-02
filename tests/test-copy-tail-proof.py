#!/usr/bin/env python3
"""Negative joins and missing-output boundaries for offline COPY-tail proof."""
from copy import deepcopy
from pathlib import Path
import io
import sys
import unittest
sys.path.insert(0, str(Path(__file__).resolve().parents[1]/'tools'))
from copy_tail_proof import attribute, check_replay, summary, write_case
import struct


def fixture():
    decoded, routes, live = [], [], []
    for i in range(1, 4):
        w = [1, 2, 0, 0, 2, 2, 0, 0, int(i == 3), 0]
        decoded.append((3, i, struct.pack('<10I', *w)))
        r = [0]*44; r[0] = i; r[2] = 0x18c279; r[15] = 1
        r[11] = 1; r[12] = int(i == 3)
        r[24:32] = [0, 0, 2, 2, 0, 0, 2, 2]
        r[16:18] = r[19:21] = [2, 2]; r[40:44] = [i, i, 1, 2]; routes.append(r)
        if i == 1:live.append(dict(sequence=i, words=w+[0, 0, 0], mismatches=0, actual_sha256='a', expected_sha256='a'))
    diagnostic = dict(selection='all-copies', ended=True, success=False, truncated=True,
                      error='', original_pixels_used_as_native_inputs=False, copies_checked=1, copies_seen=3)
    return decoded, routes, live, diagnostic


class TailTests(unittest.TestCase):
    def test_two_outputs_are_replay_candidates_not_recovered_live_outputs(self):
        tail = attribute(*fixture(), [])
        self.assertEqual(summary(tail)['unchecked_commands'], 2)
        self.assertTrue(all(x['original_live_output_available'] is False for x in tail))
        self.assertTrue(all(x['phase_bracket']['exact_menu_action_proved'] is False for x in tail))

    def test_duplicate_original_ownership_refuses(self):
        d, r, l, c = fixture(); r.append(r[-1])
        with self.assertRaisesRegex(ValueError, 'Overlapping'):attribute(d, r, l, c, [])

    def test_changed_live_geometry_refuses(self):
        d, r, l, c = fixture(); l[0]['words'][6] = 1
        with self.assertRaisesRegex(ValueError, 'prefix'):attribute(d, r, l, c, [])

    def test_unowned_and_wrong_opcode_refuse(self):
        d, r, l, c = fixture()
        with self.assertRaisesRegex(ValueError, 'Unowned'):attribute(d, r[:-1], l, c, [])
        d[2] = (2, 3, b'')
        with self.assertRaisesRegex(ValueError, 'another opcode'):attribute(d, r, l, c, [])

    def test_changed_caller_resource_key_or_geometry_refuses(self):
        for column, value in ((42, 9), (22, 31), (28, 1)):
            d, r, l, c = fixture(); r[-1][column] = value
            with self.assertRaisesRegex(ValueError, 'disagreement'):attribute(d, r, l, c, [])

    def test_non_suffix_omissions_refuse(self):
        d, r, l, c = fixture(); l[0]['sequence'] = 2
        with self.assertRaisesRegex(ValueError, 'contiguous'):attribute(d, r, l, c, [])

    def test_fresh_nominal_or_uncapped_diagnostic_cannot_be_relabelled(self):
        for key, value in [('success', True), ('truncated', False), ('selection', 'world-ancestry')]:
            d, r, l, c = fixture(); c[key] = value
            with self.assertRaises(ValueError):attribute(d, r, l, c, [])

    def test_extent_and_fixture_budget_refuse(self):
        item = attribute(*fixture(), [])[0]
        with self.assertRaisesRegex(ValueError, 'extent'):write_case(io.BytesIO(), item, b'', b'')

    def test_output_mismatch_and_duplicate_sequence_refuse(self):
        tail = attribute(*fixture(), [])
        expected = [dict(sequence=t['sequence'], source_sha256='s', before_sha256='b', expected_sha256='e') for t in tail]
        actual = [dict(sequence=t['sequence'], width=2, height=2, source_sha256='s', before_sha256='b', actual_sha256='e') for t in tail]
        self.assertTrue(all(x['original_live_output_recovered'] is False for x in check_replay(tail, expected, actual)))
        wrong = deepcopy(actual); wrong[1]['actual_sha256'] = 'b'
        with self.assertRaisesRegex(ValueError, 'destination mismatch'):check_replay(tail, expected, wrong)
        wrong = deepcopy(actual); wrong[1]['sequence'] = wrong[0]['sequence']
        with self.assertRaisesRegex(ValueError, 'sequence'):check_replay(tail, expected, wrong)


if __name__ == '__main__':unittest.main()
