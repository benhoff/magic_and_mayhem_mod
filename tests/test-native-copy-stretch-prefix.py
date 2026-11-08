#!/usr/bin/env python3
"""A native stretch marker cannot admit legacy, failed or unbounded records."""
import importlib.util
from pathlib import Path
import struct
import unittest

root=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('copy_prefix',root/'tools/inspect-canvas-producers.py')
decoder=importlib.util.module_from_spec(spec);spec.loader.exec_module(decoder)


class Controls(unittest.TestCase):
    def row(self):
        r=[0]*24
        r[0:8]=[96,1,6,2,1,640,480,1280]
        r[8:17]=[320,0,0,0,320,480,31,313,480]
        r[22],r[23]=0x4c61d5,0x4e435331
        return r

    def stream(self,r):
        return b'MNMPRO03'+struct.pack('<14I',3,64,0x40209ca7,131072,32,*([0]*9))+struct.pack('<24I',*r)

    def test_observed_shrink_geometry_admitted(self):
        _,rows=decoder.decode(self.stream(self.row()),complete=False)
        self.assertEqual(rows[0][0][15:17],(313,480))

    def test_invalid_marker_geometry_result_and_operation(self):
        for k,v in [(15,0),(16,2049),(15,0xffffffff),(21,0x88760096),(23,7),(2,5),(18,1)]:
            with self.subTest(field=k,value=v):
                r=self.row();r[k]=v
                with self.assertRaises(ValueError):decoder.decode(self.stream(r),complete=False)

    def test_legacy_marker_refused(self):
        self.assertFalse(decoder.native_copy_metadata(self.row(),False))

    def test_original_native_rectangle_refusal_retained(self):
        r=self.row();r[23],r[21]=0x4e435231,0x88760096
        _,rows=decoder.decode(self.stream(r),complete=False)
        self.assertEqual(rows[0][0][23],0x4e435231)


if __name__=='__main__':unittest.main()
