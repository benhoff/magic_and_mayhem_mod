#!/usr/bin/env python3
from pathlib import Path
import struct
import sys
import unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from drawing_wine_buffer import decode


def fixture():
    h=[0]*16;h[:2]=struct.unpack('<2I',b'MNMWLF01');h[2:7]=[1,96,0x40209ca7,131072,2048]
    entry=[0]*24;entry[:5]=[1,1,1,3,7];returned=list(entry);returned[:3]=[2,2,1];returned[10:12]=[1,1]
    terminal=[0]*24;terminal[:5]=[3,4,1,1,7];terminal[11]=1
    lifetime=struct.pack('<16I',*h)+b''.join(struct.pack('<24I',*r) for r in (entry,returned,terminal))
    h=[0]*16;h[:2]=struct.unpack('<2I',b'MNMWBF01');h[2:6]=[1,96,0x40209ca7,131072]
    entry=[0]*24;entry[:5]=[1,1,1,1,7];entry[10:12]=[1,1];entry[15]=2
    returned=list(entry);returned[:3]=[2,2,1];returned[8]=1;returned[13]=2;returned[16]=2
    terminal=[0]*24;terminal[:5]=[3,4,1,1,7]
    return struct.pack('<16I',*h)+b''.join(struct.pack('<24I',*r) for r in (entry,returned,terminal)),lifetime


class Buffer(unittest.TestCase):
    def test_unknown_return_origin_keeps_generation_zero(self):
        x=decode(*fixture());self.assertEqual(x['unknown_texture_returns'],1);self.assertFalse(x['source_storage_owned'])
    def test_parent_generation_prefix_and_return_identity_refuse(self):
        raw,lifetime=fixture()
        for off,value in ((64+11*4,2),(64+15*4,99),(64+96+14*4,1),(64+96+10*4,3),(64+96+8*4,0)):
            bad=bytearray(raw);struct.pack_into('<I',bad,off,value)
            with self.subTest(off=off),self.assertRaises(ValueError):decode(bad,lifetime)
    def test_truncation_and_reserved_refuse(self):
        raw,lifetime=fixture()
        with self.assertRaises(ValueError):decode(raw[:-1],lifetime)
        bad=bytearray(raw);struct.pack_into('<I',bad,64+17*4,1)
        with self.assertRaises(ValueError):decode(bad,lifetime)


if __name__=='__main__':unittest.main()
