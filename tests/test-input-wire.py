import struct
import sys
from pathlib import Path
import unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from input_wire import valid_header,input_v1,input_v2


class Admission(unittest.TestCase):
    def test_legacy(self):
        self.assertTrue(valid_header(input_v1.initial_header(),input_v1.SIZE))

    def test_desktop(self):
        h=bytearray(input_v2.initial_header());struct.pack_into('<I',h,40,1)
        self.assertTrue(valid_header(h,input_v2.SIZE))
        for offset,value in [(8,3),(12,64),(40,0),(40,2),(44,1)]:
            bad=h.copy();struct.pack_into('<I',bad,offset,value)
            self.assertFalse(valid_header(bad,input_v2.SIZE),(offset,value))
        self.assertFalse(valid_header(h,64))
        self.assertFalse(valid_header(h[:40],input_v2.SIZE))


if __name__=='__main__':unittest.main()
