#!/usr/bin/env python3
import importlib.util
from pathlib import Path
import struct
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('aux', ROOT / 'tools/drawing_aux_save_readback.py')
aux = importlib.util.module_from_spec(spec)
spec.loader.exec_module(aux)


class GuardTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.header = b'MNMAUX01' + struct.pack('<14I', 1, 256, 0x40209ca7, 256, 30, 50, 5, 1, 40, 40, 3200, 0, 0, 0)
        self.row = [0] * 64
        self.row[:8] = [1, 1, 12, 4, 1, 0, 0, 11]
        self.row[9:12] = [1, 1, 1]
        self.row[15:24] = [1, 40, 40, 80, 16, 0xf800, 0x7e0, 31, 3200]
        for offset, canvas, width, height in [(24, 5, 40, 40), (40, 1, 800, 600)]:
            self.row[offset:offset + 3] = [64, 1, 19]
            self.row[offset + 4] = 33
            self.row[offset + 6:offset + 9] = [canvas, width, height]
            self.row[offset + 10] = 1
        self.row[56:62] = [19, 1, 33, 5, 1, 0]
        self.end = [0] * 64
        self.end[:10] = [2, 4, 2, 4, 1, 1, 1, 0, 0, 0]
        (self.root / 'aux-save-oracle-000001.565').write_bytes(bytes(3200))

    def run_data(self):
        (self.root / 'aux-save-readback.bin').write_bytes(self.header +
            struct.pack('<64I', *self.row) + struct.pack('<64I', *self.end))
        return aux.analyze(self.root)

    def refused(self, reason):
        self.row[8] = reason
        self.row[15] = self.row[23] = self.end[6] = 0
        self.end[7] = 1
        (self.root / 'aux-save-oracle-000001.565').unlink()

    def test_valid_mapped_view_is_not_backing_or_input(self):
        result = self.run_data()
        self.assertEqual(result['diagnostic_driver_calls'], 3)
        self.assertFalse(result['original_backing_owned'])
        self.assertFalse(result['original_pixels_used_as_native_inputs'])

    def test_successful_lock_requires_unlock(self):
        self.refused(4)
        self.row[11] = 0
        with self.assertRaisesRegex(ValueError, 'lacks extra Unlock'):
            self.run_data()

    def test_unlock_failure_cannot_claim_state_restored(self):
        self.refused(7)
        self.row[14] = 0x80004005
        self.assertFalse(self.run_data()['original_driver_state_restored'])

    def test_no_unlock_after_failed_lock(self):
        self.refused(3)
        self.row[13] = 0x887601ae
        with self.assertRaisesRegex(ValueError, 'failed predecessor'):
            self.run_data()

    def test_mid_call_close_refuses(self):
        self.end[9] = 1
        with self.assertRaisesRegex(ValueError, 'lifecycle'):
            self.run_data()

    def test_wrong_source_and_stopped_point_refuse(self):
        for index, value in [(46, 2), (26, 23)]:
            old = self.row[index]
            self.row[index] = value
            with self.assertRaisesRegex(ValueError, 'source point'):
                self.run_data()
            self.row[index] = old

    def test_bad_format_padding_extent_refuse(self):
        for index, value in [(18, 79), (19, 32), (20, 0x7c00), (23, 3198)]:
            old = self.row[index]
            self.row[index] = value
            with self.assertRaises(ValueError):
                self.run_data()
            self.row[index] = old

    def test_extra_or_short_oracle_refuse(self):
        path = self.root / 'aux-save-oracle-000002.565'
        path.write_bytes(bytes(3200))
        with self.assertRaisesRegex(ValueError, 'artifacts'):
            self.run_data()
        path.unlink()
        (self.root / 'aux-save-oracle-000001.565').write_bytes(bytes(3199))
        with self.assertRaisesRegex(ValueError, 'extent'):
            self.run_data()


if __name__ == '__main__':
    unittest.main()
