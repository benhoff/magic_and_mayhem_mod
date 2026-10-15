#!/usr/bin/env python3
from pathlib import Path
import struct
import sys
import unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from drawing_wine_blit_packet import decode


def wire():
    h=[0]*16;h[:2]=struct.unpack('<2I',b'MNMWPK01');h[2:7]=[1,64,0x40209ca7,131072,2048]
    emit=[0]*16;emit[:5]=[1,1,1,17,7]
    entry=list(emit);entry[:2]=[2,2];entry[4]=8;entry[7]=1
    returned=list(entry);returned[:2]=[3,3]
    terminal=[0]*16;terminal[:5]=[4,4,1,1,7]
    return struct.pack('<16I',*h)+b''.join(struct.pack('<16I',*r) for r in [emit,entry,returned,terminal])


class Packet(unittest.TestCase):
    def test_exact_identity_not_source_ownership(self):
        x=decode(wire(),{17:{}},{1:{}});self.assertTrue(x['complete_packet_binding']);self.assertFalse(x['source_storage_owned'])
    def test_changed_submission_worker_thread_and_reserve_refuse(self):
        for offset,value in ((64+64+3*4,18),(64+128+7*4,2),(64+128+4*4,9),(64+9*4,1),(64+192+8*4,1)):
            raw=bytearray(wire());struct.pack_into('<I',raw,offset,value)
            with self.subTest(offset=offset),self.assertRaises(ValueError):decode(raw)
    def test_missing_crosslinks_and_truncation_refuse(self):
        with self.assertRaises(ValueError):decode(wire(),{},None)
        with self.assertRaises(ValueError):decode(wire(),None,{})
        with self.assertRaises(ValueError):decode(wire()[:-1])


if __name__=='__main__':unittest.main()
