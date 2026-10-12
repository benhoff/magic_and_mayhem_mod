#!/usr/bin/env python3
from pathlib import Path
import struct
import sys
import unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from drawing_source_byte_witness import decode


def wire():
    h=[0]*16;h[:2]=struct.unpack('<2I',b'MNMSBY01');h[2:8]=[1,192,0x40209ca7,4096,1024,1073741824];rows=[]
    for phase in [1,2]:
        r=[0]*48;r[:4]=[phase,phase,1,7];r[7]=2;rows.append(r)
    t=[0]*48;t[:3]=[3,4,1];rows.append(t)
    return struct.pack('<16I',*h)+b''.join(struct.pack('<48I',*r) for r in rows)


class Witness(unittest.TestCase):
    def test_refusals_remain_external_oracle_scope(self):
        x=decode(wire());self.assertEqual(x['refused_points'],2);self.assertFalse(x['source_storage_owned'])
    def test_plane_for_refusal_and_duplicate_phase_refuse(self):
        for off,value in ((64+8*4,1),(64+192+4,1),(64+10*4,1)):
            raw=bytearray(wire());struct.pack_into('<I',raw,off,value)
            with self.assertRaises(ValueError):decode(raw)
    def test_truncation_refuses(self):
        with self.assertRaises(ValueError):decode(wire()[:-1])


if __name__=='__main__':unittest.main()
