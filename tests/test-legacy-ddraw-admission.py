from pathlib import Path
import struct
import sys
import unittest
sys.path.insert(0, str(Path(__file__).resolve().parents[1]/'tools'))
from legacy_ddraw_admission import decode


class Admission(unittest.TestCase):
    def raw(self, stage):
        rows = [(90, 0, 0, 998 if stage == 1 else 0, 0 if stage == 1 else 0x100000, 0)]
        if stage > 1: rows.append((91, 0, 0, 127 if stage == 2 else 0, int(stage > 2), 0))
        if stage > 2: rows.append((92, 0, 0, 0x80004005 if stage == 3 else 0, int(stage > 3), 0))
        if stage > 3: rows += [(93, 0, 0, 0, 0, 8), (94, 0, 0, 0 if stage == 5 else 0x80004001, int(stage == 5), 0)]
        rows.append((99, 0, 0, 0, stage, 0))
        return b''.join(struct.pack('<6I', *r) for r in rows)
    def test_five_actual_outcome_classes(self):
        for stage in range(1, 6):
            with self.subTest(stage=stage):
                r = decode(self.raw(stage)); self.assertEqual(r['admission_stage'], stage)
                self.assertEqual(r['legacy_surface_available'], stage == 5)
        self.assertEqual(decode(self.raw(1))['loader_error'], 998)
    def test_null_surface_is_a_completed_negative(self):
        raw = bytearray(self.raw(4)); struct.pack_into('<I', raw, 4*24+12, 0)
        self.assertFalse(decode(raw)['legacy_surface_available'])
        struct.pack_into('<I', raw, 5*24+16, 5)
        with self.assertRaises(ValueError): decode(raw)
    def test_missing_duplicate_reordered_and_unknown_terminals_refuse(self):
        good = self.raw(5)
        for data in (b'', good[:-1], good[:-24], good+good[-24:], good[24:], good[:24]+good[48:72]+good[24:48]+good[72:]):
            with self.subTest(size=len(data)), self.assertRaises(ValueError): decode(data)
        for stage in (0, 6, 0xffffffff):
            bad = bytearray(good); struct.pack_into('<I', bad, len(bad)-8, stage)
            with self.assertRaises(ValueError): decode(bad)


if __name__ == '__main__': unittest.main()
