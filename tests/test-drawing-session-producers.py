import importlib.util
from pathlib import Path
import struct
import unittest
spec=importlib.util.spec_from_file_location('producer',Path(__file__).resolve().parents[1]/'tools/inspect-canvas-producers.py');m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)


def raw_session():
    raw=b'MNMPRO05'+struct.pack('<14I',5,64,0x40209ca7,1048576,120,2,536870912,*([0]*7))
    marker=b'MNMPCS01'+struct.pack('<14I',1,64,0x40209ca7,0,0,64,0,7,1,120,0,0,0,0)
    return raw,marker


class Cases(unittest.TestCase):
    def test_closed_metadata_session(self):
        raw,marker=raw_session();h,r=m.decode_session(raw,marker);self.assertFalse(r)
        with self.assertRaisesRegex(ValueError,'external closure'):m.decode(raw)
    def test_marker_counts_and_reserves(self):
        raw,marker=raw_session()
        for index in (5,6,7,8,10,11,12):
            bad=bytearray(marker);struct.pack_into('<I',bad,index*4,999)
            with self.assertRaises(ValueError):m.decode_session(raw,bad)
    def test_legacy_cannot_be_relabelled(self):
        raw,marker=raw_session()
        with self.assertRaises(ValueError):m.decode_session(b'MNMPRO03'+raw[8:],marker)


if __name__=='__main__':unittest.main()
