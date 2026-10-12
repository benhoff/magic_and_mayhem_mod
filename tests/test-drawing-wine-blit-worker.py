#!/usr/bin/env python3
from pathlib import Path
import sys
import struct
import unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from drawing_wine_blit_worker import decode


def fixture():
    h=[0]*16;h[:2]=struct.unpack('<2I',b'MNMWLF01');h[2:7]=[1,96,0x40209ca7,131072,2048]
    terminal=[0]*24;terminal[:5]=[1,4,1,0,7]
    lifetime=struct.pack('<16I',*h)+struct.pack('<24I',*terminal)
    h=[0]*16;h[:2]=struct.unpack('<2I',b'MNMWBK01');h[2:7]=[1,192,0x40209ca7,131072,42]
    entry=[0]*48;entry[:5]=[1,1,1,42,7];entry[8]=1;entry[10]=2
    returned=list(entry);returned[:3]=[2,2,1];returned[6]=0xcafe
    terminal=[0]*48;terminal[:5]=[3,4,1,1,7]
    return struct.pack('<16I',*h)+b''.join(struct.pack('<48I',*r) for r in (entry,returned,terminal)),lifetime


class Worker(unittest.TestCase):
    def test_unknown_generation_stays_explicit(self):
        x=decode(*fixture());self.assertEqual(x['unknown_destination_generations'],1);self.assertFalse(x['source_submission_identity_bound'])
    def test_opcode_generation_prefix_and_return_fields_refuse(self):
        raw,lifetime=fixture()
        for offset,value in ((64+3*4,41),(64+9*4,1),(64+7*4,99),(64+192+8*4,2),(64+34*4,1)):
            bad=bytearray(raw);struct.pack_into('<I',bad,offset,value)
            with self.subTest(offset=offset),self.assertRaises(ValueError):decode(bad,lifetime)
    def test_missing_terminal_and_truncation_refuse(self):
        raw,lifetime=fixture()
        for bad in (raw[:-1],raw[:-192]):
            with self.assertRaises(ValueError):decode(bad,lifetime)


if __name__=='__main__':unittest.main()
