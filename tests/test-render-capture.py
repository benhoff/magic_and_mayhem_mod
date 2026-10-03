#!/usr/bin/env python3
"""Native-pixel replay semantics and malformed evidence rejection."""
import importlib.util
from pathlib import Path
import struct
import unittest

REPO = Path(__file__).resolve().parent.parent
spec = importlib.util.spec_from_file_location('render_replay', REPO/'tools/replay-render-capture.py')
replay = importlib.util.module_from_spec(spec); spec.loader.exec_module(replay)


def fixture(bits=16, keyed=True, fast=False):
    h = [0]*32
    h[2:10] = [1, 2 if fast else 1, 0x58c279, (1 if fast else 0x8000) if keyed else 0,
               int(keyed), 1 if keyed else 0, 1 if keyed else 0, 0]
    h[10:18] = [0, 0, 2, 2, 1, 1, 3, 3]
    h[18:26] = [2, 2, 4, 3, bits, *((0xf800, 0x7e0, 0x1f) if bits == 16 else
                                   (0, 0, 0) if bits == 8 else (0xff0000, 0xff00, 0xff))]
    size = bits//8
    h[26:29] = [4*size, 12*size, 1024 if bits == 8 else 0]
    source = [1, 7, 8, 1]; before = [9]*12; after = before.copy()
    after[5:7] = [9 if keyed else 1, 7]
    after[9:11] = [8, 9 if keyed else 1]
    encode = lambda values: b''.join(v.to_bytes(size, 'little') for v in values)
    # Deliberately identical palette colors; index identity must survive replay.
    palette = bytes([45, 90, 180, 0])*256 if bits == 8 else b''
    return b'MNMBLT01' + struct.pack('<30I', *h[2:]) + encode(source) + encode(before) + encode(after) + palette*2


class CaptureTest(unittest.TestCase):
    def test_native_copy_and_exact_key(self):
        for bits in (8, 16, 24, 32):
            for keyed in (False, True):
                for fast in (False, True):
                    with self.subTest(bits=bits, keyed=keyed, fast=fast):
                        c = replay.decode(fixture(bits, keyed, fast))
                        self.assertEqual(replay.replay(c), c['after'])
                        self.assertTrue(replay.compare(c, c['after'])['matching'])

    def test_subrect_uses_surface_stride(self):
        data = bytearray(fixture(keyed=False))
        struct.pack_into('<4i', data, 40, 1, 0, 2, 2)
        struct.pack_into('<4i', data, 56, 2, 0, 3, 2)
        c = replay.decode(data)
        expected = [9, 9, 7, 9, 9, 9, 1, 9, 9, 9, 9, 9]
        self.assertEqual(replay.replay(c), struct.pack('<12H', *expected))

    def test_mismatch_outside_rectangle_is_reported(self):
        c = replay.decode(fixture())
        actual = bytearray(replay.replay(c)); actual[0] ^= 1
        report = replay.compare(c, actual)
        self.assertFalse(report['matching'])
        self.assertEqual(report['mismatching_pixels'], 1)
        self.assertEqual((report['first_mismatch']['x'], report['first_mismatch']['y']), (0, 0))

    def test_truncation_and_trailing_payload(self):
        data = fixture()
        for broken in (b'', data[:127], data[:-1], data+b'\0', b'BADMAGIC'+data[8:]):
            with self.assertRaises(ValueError): replay.decode(broken)

    def test_reject_unsupported_contracts(self):
        # Fields are independently malformed; none may silently replay as a copy.
        for offset, value in ((8, 2), (12, 9), (20, 0x200), (24, 0), (28, 3), (32, 2), (32, 0x10000),
                              (36, 0x80000000), (40, 0xffffffff), (48, 1), (72, 257),
                              (80, 2049), (88, 15), (92, 0xf801), (96, 0xf800),
                              (104, 1), (112, 1024), (124, 1)):
            with self.subTest(offset=offset):
                broken = bytearray(fixture()); struct.pack_into('<I', broken, offset, value)
                with self.assertRaises(ValueError): replay.decode(broken)

    def test_ppm_previews(self):
        c = replay.decode(fixture(8))
        self.assertEqual(replay.ppm(c, c['source'], True), b'P6\n2 2\n255\n'+bytes([45, 90, 180])*4)
        c = replay.decode(fixture())
        self.assertTrue(replay.ppm(c, c['before']).startswith(b'P6\n4 3\n255\n'))

    def test_events_are_bounded_and_sequenced(self):
        header = b'MNMDRW01'+struct.pack('<2I', 1, 64)
        event = struct.pack('<16I', 1, 4, 0x58b6a0, *([0]*13))
        report = replay.summarize_events(header+event)
        self.assertEqual(report['counts'], {'Lock': 1})
        self.assertEqual(report['callers'][0]['return_va'], '0x0058b6a0')
        for data in (header+event[:-1], header+event*2, header+event*2049):
            with self.assertRaises(ValueError): replay.summarize_events(data)


if __name__ == '__main__': unittest.main()
