#!/usr/bin/env python3
"""Refuse incomplete, ambiguous or differing offline COPY proof claims."""
import copy
import hashlib
from pathlib import Path
import struct
import sys
import unittest
sys.path.insert(0, str(Path(__file__).resolve().parents[1]/'tools'))
from copy_tail_replay import trace, offline_observer, compare_records, compare_cpu


class Controls(unittest.TestCase):
    def row(self):
        return dict(sequence=2, words=[1, 2]+[0]*11, source_width=1, source_height=1,
                    target_width=1, target_height=1, pixels=1, mismatches=0,
                    source_sha256='a'*64, before_sha256='b'*64,
                    actual_sha256='c'*64, expected_sha256='c'*64)

    def history(self):
        words = [1, 2]+[0]*8
        return (b'MNMCMD05'+struct.pack('<II', 5, 16)
                +struct.pack('<III5I', 19, 1, 20, 2, 1, 0, 1, 0)
                +struct.pack('<III10I', 3, 2, 40, *words)+struct.pack('<III', 8, 3, 0))

    def routes(self):
        row = [0]*44
        row[0], row[2], row[15] = 9, 0x18c05d, 2
        row[40:44] = [2, 2, 1, 2]
        return [row]

    def test_attribution(self):
        result = trace(self.history(), [], self.routes(), [])
        self.assertEqual((result['copies'], result['missing_gpu_calls']), (1, 1))
        self.assertEqual(result['missing_copies'][0]['original_call'], 9)

    def test_missing_route(self):
        with self.assertRaises(ValueError):
            trace(self.history(), [], [], [])

    def test_overlapping_routes(self):
        with self.assertRaises(ValueError):
            trace(self.history(), [], self.routes()*2, [])

    def test_wrong_handles(self):
        routes = self.routes();routes[0][43] = 3
        with self.assertRaises(ValueError):
            trace(self.history(), [], routes, [])

    def test_foreign_captured_sequence(self):
        row = self.row();row['sequence'] = 99
        with self.assertRaises(ValueError):
            trace(self.history(), [row], self.routes(), [])

    def test_duplicate_capture(self):
        with self.assertRaises(ValueError):
            trace(self.history(), [self.row()]*2, self.routes(), [])

    def test_actual_capture_anchor(self):
        extra = self.row();extra['sequence'] = 9
        self.assertEqual(compare_records([self.row()], [self.row(), extra], 2)['additional_native_replay_checks'], 1)

    def test_every_anchor_field(self):
        for field in ('words', 'source_width', 'source_height', 'target_width', 'target_height',
                      'pixels', 'source_sha256', 'before_sha256', 'actual_sha256', 'expected_sha256'):
            row = copy.deepcopy(self.row());row[field] = 'different'
            with self.subTest(field=field), self.assertRaises(ValueError):
                compare_records([self.row()], [row], 1)

    def test_duplicate_replay(self):
        with self.assertRaises(ValueError):
            compare_records([], [self.row()]*2, 2)

    def test_incomplete_replay(self):
        with self.assertRaises(ValueError):
            compare_records([], [self.row()], 2)

    def test_tail_mismatch(self):
        row = self.row();row['mismatches'] = 1
        with self.assertRaises(ValueError):
            compare_records([], [row], 1)

    def test_cpu_reply_full_header_and_pixels(self):
        body = bytes(66);actual = dict(sequence=2, sha256=hashlib.sha256(body).hexdigest())
        rows = [(dict(kind='cpu-read', sequence=2), body)]
        self.assertEqual(compare_cpu(rows, [actual]), 1)
        for at in (0, 64):
            changed = bytearray(body);changed[at] = 1
            with self.assertRaises(ValueError):
                compare_cpu([(rows[0][0], bytes(changed))], [actual])

    def test_missing_cpu_reply(self):
        with self.assertRaises(ValueError):
            compare_cpu([(dict(kind='cpu-read', sequence=2), bytes(66))], [])

    def test_offline_patch_refuses_drift(self):
        root = Path(__file__).resolve().parents[1]
        source = (root/'renderer/command_copy_diagnostics.cpp').read_text()
        patched = offline_observer(source)
        self.assertIn('p.enqueued>=16384', patched)
        self.assertIn('wait.elapsed()<60000', patched)
        self.assertNotIn('p.enqueued>=16384', source)
        with self.assertRaises(ValueError):
            offline_observer(source.replace('p.enqueued>=8192', 'p.enqueued>=8200'))


if __name__ == '__main__':
    unittest.main()
