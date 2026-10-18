from pathlib import Path
import struct
import sys
import tempfile
import unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from raster_finite_counts import decode


class Counts(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.path = Path(self.temp.name)/'counts.bin'
        self.words = [1,208,23,0]+[0]*46
        self.words[4+14*2:6+14*2] = [7,9]
        self.native = {'splice'+str(i):100 for i in range(23)}

    def result(self):
        self.path.write_bytes(b'MNMFDSP1'+struct.pack('<50I',*self.words))
        return decode(self.path,self.native)

    def test_branch_observation_is_separate_from_equivalence(self):
        d = self.result()
        self.assertEqual((d['calls'],d['retained_pixels']),(7,9))
        self.assertFalse(d['original_outside_allocation_equivalence'])

    def test_missing_truncated_or_wrong_magic_is_not_zero_traffic(self):
        with self.assertRaises(FileNotFoundError):decode(self.path,self.native)
        for data in (b'',b'MNMFDSP1',b'BADMAGIC'+bytes(200)):
            self.path.write_bytes(data)
            with self.assertRaises(ValueError):decode(self.path,self.native)

    def test_wrong_version_extent_routes_or_overflow_refuses(self):
        for i in range(4):
            original = self.words[i]; self.words[i] += 1
            with self.assertRaises(ValueError):self.result()
            self.words[i] = original

    def test_pixels_and_calls_must_correspond(self):
        for pair in ([0,9],[7,0],[7,6]):
            self.words[32:34] = pair
            with self.assertRaises(ValueError):self.result()

    def test_cannot_exceed_actual_native_calls(self):
        self.native['splice14'] = 6
        with self.assertRaises(ValueError):self.result()


if __name__=='__main__':unittest.main()
