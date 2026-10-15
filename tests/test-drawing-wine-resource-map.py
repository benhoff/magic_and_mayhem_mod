#!/usr/bin/env python3
from pathlib import Path
import importlib.util,struct,sys,unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from drawing_wine_resource_mapping import decode


def wire():
    h=[0]*16;h[:2]=struct.unpack('<2I',b'MNMWRM01');h[2:7]=[1,128,0x40209ca7,131072,2048];a=[0]*32;a[:7]=[1,1,1,2,7,0,0];a[10]=1;b=a.copy();b[:2]=[2,2];b[24:28]=[1600,960000,2,1];t=[0]*32;t[:5]=[3,4,1,1,7];t[10]=2;t[27]=1;t[30]=1;return struct.pack('<16I',*h)+b''.join(struct.pack('<32I',*r) for r in [a,b,t])


class Maps(unittest.TestCase):
    def test_openinterval_preserved(self):
        x=decode(wire());self.assertEqual(x['open_map_intervals'],1);self.assertFalse(x['source_upload_identity_proved'])
    def test_identity_descriptor_and_reserve_refuse(self):
        for offset,value in ((64+128+10*4,3),(64+128+27*4,0),(64+31*4,1),(64+256+30*4,0)):
            raw=bytearray(wire());struct.pack_into('<I',raw,offset,value)
            with self.assertRaises(ValueError):decode(raw)
    def test_truncation_refuses(self):
        with self.assertRaises(ValueError):decode(wire()[:-1])


if __name__=='__main__':unittest.main()
