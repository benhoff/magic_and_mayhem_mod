#!/usr/bin/env python3
"""Refuse incomplete or mismatched copy histories while retaining reversed returns."""
from pathlib import Path
import struct
import sys
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
from drawing_copy_order import decode


def journal(mutate=None):
    h = bytearray(64)
    h[:8] = b'MNMDCQ01'
    struct.pack_into('<4I', h, 8, 1, 128, 0x40209ca7, 65536)
    rows = []
    for phase, request, owner, prefix, returned, active in [(1, 1, 11, 10, 0, 1), (1, 2, 22, 11, 0, 2), (2, 2, 22, 12, 12, 1), (2, 1, 11, 13, 13, 0)]:
        r = [0]*32
        r[:8] = [len(rows)+1, phase, request, owner, 7, 100, prefix, 1]
        r[8:20] = [1, 2, 800, 600, 0, 0, 0, 0, 0, 800, 600, 0xffffffff]
        r[20], r[21], r[24] = 0x1234, returned, active
        rows.append(r)
    r = [0]*32
    r[0], r[1], r[2], r[6], r[7] = 5, 3, 2, 13, 1
    rows.append(r)
    if mutate:
        mutate(rows)
    return bytes(h) + b''.join(struct.pack('<32I', *r) for r in rows)


class Order(unittest.TestCase):
    def test_reversed_returns_retained(self):
        r = decode(journal())
        self.assertEqual([e['request'] for e in r['entries']], [1, 2])
        self.assertEqual([e['request'] for e in r['returns']], [2, 1])
        self.assertFalse(r['driver_order_proved'])

    def test_changed_descriptor_owner_and_bad_return_prefix_refuse(self):
        for index, value in [(8, 99), (3, 99), (21, 10)]:
            with self.assertRaises(ValueError):
                decode(journal(lambda rows: rows[3].__setitem__(index, value)))

    def test_duplicate_entry_and_incomplete_terminal_refuse(self):
        for mutation in [lambda r: r[1].__setitem__(2, 1), lambda r: r[-1].__setitem__(24, 1), lambda r: r.pop(3)]:
            with self.assertRaises(ValueError):
                decode(journal(mutation))

    def test_reserved_and_count_corruption_refuse(self):
        for index in (22, 25, 24):
            with self.assertRaises(ValueError):
                decode(journal(lambda rows: rows[0].__setitem__(index, 9)))


if __name__ == '__main__':
    unittest.main()
