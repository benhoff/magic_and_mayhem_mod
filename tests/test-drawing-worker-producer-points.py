#!/usr/bin/env python3
import struct
import sys
import unittest
from pathlib import Path
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
from drawing_worker_producer_points import decode


def fixture(reverse=False):
    header = [0] * 16
    header[:2] = struct.unpack('<2I', b'MNMWPP01')
    header[2:7] = [1, 128, 0x40209ca7, 131072, 64]
    legacy_header = [0] * 16
    legacy_header[:2] = struct.unpack('<2I', b'MNMWBK01')
    legacy_header[2:7] = [1, 192, 0x40209ca7, 131072, 42]
    legacy = []
    for worker in (1, 2):
        for phase in (1, 2):
            row = [0] * 48
            row[:5] = [len(legacy) + 1, phase, worker, 42, worker + 10]
            legacy.append(row)
    row = [0] * 48
    row[:5] = [5, 4, 1, 2, 11]
    legacy.append(row)
    rows = []
    order = [(1, 1), (1, 2), (2, 2), (2, 1)]
    if reverse:
        order = [(1, 2), (1, 1), (2, 1), (2, 2)]
    for phase, worker in order:
        row = [0] * 32
        row[:7] = [len(rows) + 1, phase, worker, worker + 10, 100, 0, 2 * worker - 1]
        row[8:11] = [64, 1, 3]
        row[11], row[22] = 10 if phase == 1 else 12, worker + 10
        rows.append(row)
    row = [0] * 32
    row[:8] = [5, 3, 2, 11, 100, 0, 2, 0]
    rows.append(row)
    return (struct.pack('<16I', *header) + b''.join(struct.pack('<32I', *r) for r in rows),
            struct.pack('<16I', *legacy_header) + b''.join(struct.pack('<48I', *r) for r in legacy))


class Points(unittest.TestCase):
    def test_interleaved_queries_keep_per_worker_bounds(self):
        for reverse in (False, True):
            result = decode(*fixture(reverse))
            self.assertEqual(result['producer_changes_during_worker'], 2)
            self.assertFalse(result['general_replay_order_proved'])

    def test_missing_pair_and_wrong_owner_legacy_or_reserved_refuse(self):
        data, legacy = fixture()
        for word, value in [(3, 99), (6, 99), (7, 1), (24, 1), (22, 99)]:
            bad = bytearray(data)
            struct.pack_into('<I', bad, 64 + word * 4, value)
            with self.assertRaises(ValueError):
                decode(bad, legacy)
        with self.assertRaises(ValueError):
            decode(data[:-128], legacy)

    def test_failed_getter_has_no_invented_metadata(self):
        data, legacy = fixture()
        bad = bytearray(data)
        struct.pack_into('<I', bad, 64 + 5 * 4, 2)
        with self.assertRaises(ValueError):
            decode(bad, legacy)
        for index in range(8, 24):
            struct.pack_into('<I', bad, 64 + index * 4, 0)
        self.assertEqual(decode(bad, legacy)['unavailable_pairs'], 1)

    def test_regressed_prefix_and_incomplete_terminal_refuse(self):
        data, legacy = fixture()
        for offset, value in [(64 + 3 * 128 + 11 * 4, 9), (64 + 4 * 128 + 7 * 4, 1)]:
            bad = bytearray(data)
            struct.pack_into('<I', bad, offset, value)
            with self.assertRaises(ValueError):
                decode(bad, legacy)

    def test_legacy_terminal_and_every_return_are_independently_required(self):
        data,legacy=fixture()
        for offset,value in [(64+192+2*4,2),(64+192+8*4,99),(64+4*192+3*4,99),(64+4*192+1*4,2)]:
            bad=bytearray(legacy);struct.pack_into('<I',bad,offset,value)
            with self.assertRaises(ValueError):decode(data,bad)

    def test_return_tick_wrap_is_allowed_but_reversal_is_not(self):
        data,legacy=fixture();raw=bytearray(data)
        struct.pack_into('<I',raw,64+4*4,0xfffffff0)
        struct.pack_into('<I',raw,64+3*128+4*4,16)
        self.assertEqual(len(decode(raw,legacy)['calls']),2)
        struct.pack_into('<I',raw,64+3*128+4*4,0xffffffef)
        with self.assertRaises(ValueError):decode(raw,legacy)


if __name__ == '__main__':
    unittest.main()
