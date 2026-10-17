import struct
import sys
import unittest
from pathlib import Path
sys.path.insert(0, str(Path(__file__).resolve().parents[1]/'tools'))
from drawing_source_reservations import decode


def fixture():
    h = [0]*16
    h[2:8] = [1, 128, 0x40209ca7, 131072, 2048, 4]
    raw = bytearray(struct.pack('<16I', *h));raw[:8] = b'MNMSRG01'
    for phase in range(1, 8):
        r = [0]*32;r[:4] = [phase, phase, 10, 100]
        if phase < 7:
            r[4:11] = [1, 9, 2, 3, 4, 0, 1]
            r[11:15] = [int(phase >= 2), 2*int(phase >= 3), 3*int(phase >= 4), phase]
        else:r[15] = 1
        r[17:19] = [1, int(phase >= 6)]
        raw += struct.pack('<32I', *r)
    return raw


class Reservations(unittest.TestCase):
    def test_complete(self):
        r = decode(fixture());self.assertEqual(r['reservations'], 1)
        self.assertFalse(r['physical_backing_owned'])
        self.assertFalse(r['unobserved_writers_closed'])

    def test_missing_or_reordered_fence(self):
        for phase in (3, 4, 5, 6):
            raw = fixture();del raw[64+(phase-1)*128:64+phase*128]
            with self.assertRaises(ValueError):decode(raw)

    def test_changed_identity(self):
        for phase in range(2, 7):
            for field in (4, 5, 6, 7, 8, 9, 10):
                raw = fixture();offset = 64+(phase-1)*128+field*4
                struct.pack_into('<I', raw, offset, 100)
                with self.assertRaises(ValueError):decode(raw)

    def test_changed_consumer(self):
        for phase, field in ((3, 11), (4, 12), (5, 13), (5, 2), (6, 13)):
            raw = fixture();struct.pack_into('<I', raw, 64+(phase-1)*128+field*4, 100)
            with self.assertRaises(ValueError):decode(raw)

    def test_refused_or_incomplete(self):
        raw = fixture();struct.pack_into('<I', raw, 64+5*128+4, 8)
        with self.assertRaises(ValueError):decode(raw)
        with self.assertRaises(ValueError):decode(fixture()[:-128])

    def test_reserved_and_terminal_counts(self):
        for phase, field in ((1, 19), (6, 18), (7, 17), (7, 4)):
            raw = fixture();struct.pack_into('<I', raw, 64+(phase-1)*128+field*4, 100)
            with self.assertRaises(ValueError):decode(raw)


if __name__ == '__main__':unittest.main()
