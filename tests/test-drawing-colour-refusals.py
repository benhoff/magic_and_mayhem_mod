#!/usr/bin/env python3
import struct
import sys
import unittest
from pathlib import Path
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
from drawing_colour_refusals import decode


def fixture():
    raw = bytearray(192)
    raw[:8] = b'MNMCRF01'
    struct.pack_into('<5I', raw, 8, 1, 192, 4, 123, 1)
    struct.pack_into('<2I', raw, 72, 1, 1)
    struct.pack_into('<IiiHHHI', raw, 80, 1, 800, 600, 31, 63, 31, 0)
    struct.pack_into('<I', raw, 144, 7)
    return raw


class Refusals(unittest.TestCase):
    def test_policy_refusal_retains_inputs_and_failure(self):
        r = decode(fixture())
        self.assertEqual(r['reason'], 'trace_uniform_policy')
        self.assertEqual(r['shape']['channels'], [31, 63, 31])
        self.assertEqual(r['shape']['width'], 800)
        self.assertFalse(r['source_admission'])

    def test_unreadable_shape_is_not_fabricated(self):
        b = fixture(); struct.pack_into('<I', b, 16, 2)
        b[76:140] = bytes(64)
        self.assertIsNone(decode(b)['shape'])

    def test_malformed_records_refuse(self):
        for offset, value in ((8, 2), (12, 191), (16, 10), (24, 0), (72, 2),
                              (76, 2), (140 + 8, 1), (144, 0), (188, 1), (100, 0xffff0000)):
            b = fixture(); struct.pack_into('<I', b, offset, value)
            with self.subTest(offset=offset), self.assertRaises(ValueError): decode(b)
        for b in (b'', fixture()[:-1], fixture() + b'\0'):
            with self.assertRaises(ValueError): decode(b)


if __name__ == '__main__': unittest.main()
