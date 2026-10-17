#!/usr/bin/env python3
"""Guard actual hazards for the narrowly admitted offline COPY policy."""
import copy
from pathlib import Path
import struct
import sys
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
from drawing_copy_phase import select, transpose


def fixture():
    def row(seq, dst, src, width, height, pos, box, caller):
        r = [0] * 24
        r[:7] = [96, seq, 6, dst, src, width, height]
        r[8:14] = [*pos, *box]
        r[14], r[22] = 0xffffffff, caller
        return tuple(r), b''
    rows = [row(1, 1, 5, 800, 600, (400, 300), (0, 0, 40, 40), 0x4a4b01),
            row(2, 5, 1, 40, 40, (0, 0), (400, 300, 440, 340), 0x4a4c26),
            row(3, 1, 2, 800, 600, (0, 0), (0, 0, 800, 600), 0x46ba01)]
    returns = [dict(request=i, producer_return_sequence=seq) for i, seq in ((1, 1), (2, 3), (3, 2))]
    bindings = [dict(request=i, producer_entry_prefix=0, commands=[dict(worker_entry_record=lo, worker_return_record=lo + 1)])
                for i, lo in ((1, 1), (2, 3), (3, 5))]
    return rows, dict(entries=[], returns=returns), dict(bindings=bindings)


class PhaseGuards(unittest.TestCase):
    def rejected(self, rows, obs, bindings, reason):
        result = select(rows, obs, bindings)
        self.assertEqual([], result['selected'])
        self.assertEqual(reason, result['refused'][0]['reason'])

    def test_exact_bytes_and_sequences(self):
        rows, obs, bindings = fixture()
        chosen = select(rows, obs, bindings)
        self.assertEqual(1, len(chosen['selected']))
        raw = b'H' * 64 + b''.join(struct.pack('<24I', *r) for r, _ in rows)
        out = transpose(raw, chosen['selected'])
        self.assertEqual(raw[:160], out[:160])
        for at, original_at, seq in ((160, 256, 2), (256, 160, 3)):
            expected = bytearray(raw[original_at:original_at + 96])
            struct.pack_into('<I', expected, 4, seq)
            self.assertEqual(bytes(expected), out[at:at + 96])
        self.assertEqual(raw, transpose(raw, []))

    def test_missing_binding(self):
        rows, obs, b = fixture()
        b['bindings'].pop()
        self.rejected(rows, obs, b, 'missing-exact-request-binding')

    def test_overlapping_or_reverse_workers(self):
        for request, lo in ((1, 4), (3, 3)):
            rows, obs, b = fixture()
            c = b['bindings'][request - 1]['commands'][0]
            c['worker_entry_record'], c['worker_return_record'] = lo, lo + 1
            self.rejected(rows, obs, b, 'worker-triple-not-completely-ordered')

    def test_geometry_key_caller_and_payload(self):
        for index, field, value in ((0, 8, 761), (1, 14, 0), (2, 22, 0x123456), (1, 18, 4)):
            rows, obs, b = fixture()
            r = list(rows[index][0]); r[field] = value; rows[index] = tuple(r), b''
            self.rejected(rows, obs, b, 'unsupported-copy-triple')

    def test_cursor_movement_across_full_overwrite(self):
        rows, obs, b = fixture()
        r = list(rows[0][0]); r[8:10] = [393, 299]; rows[0] = tuple(r), b''
        self.assertEqual(1, len(select(rows, obs, b)['selected']))

    def test_world_and_other_draw_refused(self):
        for kind, reason in ((11, 'inside-active-world'), (5, 'other-observed-operation-in-full-interval')):
            rows, obs, b = fixture()
            marker = [0] * 24; marker[:4] = [96, 1, kind, 2]
            rows = [(tuple(marker), b'')] + [(tuple([r[0], r[1] + 1, *r[2:]]), payload) for r, payload in rows]
            obs['returns'] = [dict(q, producer_return_sequence=q['producer_return_sequence'] + 1) for q in obs['returns']]
            self.rejected(rows, obs, b, reason)

    def test_invalid_and_duplicate_transpositions(self):
        rows, obs, b = fixture(); item = select(rows, obs, b)['selected'][0]
        raw = b'H' * 64 + b''.join(struct.pack('<24I', *r) for r, _ in rows)
        for selections in ([dict(item, full_sequence=4)], [item, copy.deepcopy(item)]):
            with self.assertRaises(ValueError):
                transpose(raw, selections)
        with self.assertRaises(ValueError):
            transpose(raw[:-1], [item])


if __name__ == '__main__':
    unittest.main()
