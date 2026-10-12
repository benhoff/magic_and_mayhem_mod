import struct
import sys
import unittest
from pathlib import Path
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
from drawing_source_held_refusal import decode


def fixture():
    h = [0] * 16
    h[2:6] = [1, 192, 0x40209ca7, 1]
    r = [0] * 48
    r[:6] = [1, 3, 1, 20, 5, 40]
    r[8:12] = [0, 0, 799, 600]
    r[14:16] = [1, 16]
    raw = bytearray(struct.pack('<16I48I', *h, *r))
    raw[:8] = b'MNMSHR01'
    return raw


class Rejection(unittest.TestCase):
    def test_negative_geometry(self):
        result = decode(fixture())
        self.assertFalse(result['admitted'])
        self.assertEqual(result['rectangle'], [0, 0, 799, 600])
        self.assertFalse(result['source_ownership_proved'])

    def test_malformed_identity_header_and_extent(self):
        for at, value in [(12, 128), (44, 1), (68, 99), (76, 0), (120, 2)]:
            raw = fixture()
            struct.pack_into('<I', raw, at, value)
            with self.assertRaises(ValueError):
                decode(raw)
        with self.assertRaises(ValueError):
            decode(fixture()[:-1])

    def test_unreadable_geometry_is_unknown(self):
        raw = fixture()
        struct.pack_into('<5I', raw, 96, 0, 0, 0, 0, 0)
        struct.pack_into('<I', raw, 120, 0)
        self.assertIsNone(decode(raw)['rectangle'])
        struct.pack_into('<I', raw, 96, 1)
        with self.assertRaises(ValueError):
            decode(raw)


if __name__ == '__main__':
    unittest.main()
