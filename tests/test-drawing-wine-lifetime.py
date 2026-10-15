#!/usr/bin/env python3
from pathlib import Path
import struct
import sys
import unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from drawing_wine_lifetime import decode


def wire():
    h=[0]*16;h[:2]=struct.unpack('<2I',b'MNMWLF01');h[2:7]=[1,96,0x40209ca7,131072,2048]
    create=[0]*24;create[:5]=[1,1,1,1,7]
    created=list(create);created[:3]=[2,2,1];created[10:12]=[1,1]
    release=[0]*24;release[:5]=[3,1,2,2,7];release[10:12]=[1,1]
    retired=list(release);retired[:3]=[4,2,2];retired[12]=2
    terminal=[0]*24;terminal[:5]=[5,4,1,2,7];terminal[11]=1
    return struct.pack('<16I',*h)+b''.join(struct.pack('<24I',*r) for r in [create,created,release,retired,terminal])


class Lifetime(unittest.TestCase):
    def test_create_release_closure_keeps_scope(self):
        x=decode(wire());self.assertEqual(x['observed_generations'],1);self.assertEqual(x['remaining_observed_objects'],0);self.assertFalse(x['all_internal_lifetimes_observed'])
    def test_generation_and_result_flags_refuse(self):
        for offset,value in ((64+96+11*4,2),(64+192+11*4,0),(64+288+12*4,0),(64+384+11*4,2)):
            raw=bytearray(wire());struct.pack_into('<I',raw,offset,value)
            with self.subTest(offset=offset),self.assertRaises(ValueError):decode(raw)
    def test_truncation_reserve_and_failed_create_refuse(self):
        with self.assertRaises(ValueError):decode(wire()[:-1])
        for offset,value in ((64+13*4,1),(64+96+8*4,0x80004005)):
            raw=bytearray(wire());struct.pack_into('<I',raw,offset,value)
            with self.assertRaises(ValueError):decode(raw)


if __name__=='__main__':unittest.main()
