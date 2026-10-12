#!/usr/bin/env python3
from pathlib import Path
import struct
import sys
import unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from drawing_source_lease import decode


def wire():
    h=[0]*16;h[:2]=struct.unpack('<2I',b'MNMSLS01');h[2:6]=[1,128,0x40209ca7,4096]
    entry=[0]*32;entry[:4]=[1,1,1,7];entry[6:8]=[0x80000001,1];entry[16:19]=[64,1,19]
    returned=list(entry);returned[:2]=[2,2];returned[6]=0
    terminal=[0]*32;terminal[:3]=[3,4,1]
    return struct.pack('<16I',*h)+b''.join(struct.pack('<32I',*r) for r in [entry,returned,terminal])


class Lease(unittest.TestCase):
    def test_apiintervalnotasyncownership(self):
        x=decode(wire());self.assertEqual(x['accepted_leases'],1);self.assertFalse(x['async_egl_completion_covered'])
    def test_wrongidentity_failedrelease_reserve_refuse(self):
        for off,value in ((64+128+7*4,2),(64+128+6*4,4),(64+8*4,1),(64+7*4,0)):
            raw=bytearray(wire());struct.pack_into('<I',raw,off,value)
            with self.assertRaises(ValueError):decode(raw)
    def test_truncation_refuses(self):
        with self.assertRaises(ValueError):decode(wire()[:-1])


if __name__=='__main__':unittest.main()
