#!/usr/bin/env python3
from pathlib import Path
import struct
import sys
import unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from drawing_worker_context import channel,decode


def context():
    x=[0]*16;x[:2]=struct.unpack('<2I',b'MNMWCX01');x[2:11]=[1,64,2,1,7,8,9,10,11];return x


def wire():
    h=[0]*16;h[:2]=struct.unpack('<2I',b'MNMEWC01');h[2:5]=[1,128,131072]
    rows=[]
    for phase in [1,2]:
        r=[0]*32;r[:6]=[phase,phase,1,2,3,phase];r[8:24]=context();rows.append(r)
    return struct.pack('<16I',*h)+b''.join(struct.pack('<32I',*r) for r in rows)


class Context(unittest.TestCase):
    def test_stable_context_not_ownership_or_thread_mapping(self):
        x=decode(wire());self.assertEqual(x['stable_active_contexts'],1);self.assertFalse(x['win_unix_thread_relation_validated']);self.assertFalse(x['source_storage_owned'])
    def test_odd_inactive_and_reserved_refuse(self):
        for index,value in [(4,3),(5,0),(12,1),(11,1)]:
            x=context();x[index]=value
            with self.subTest(index=index),self.assertRaises(ValueError):channel(tuple(x))
    def test_changed_and_missing_context_retained(self):
        raw=bytearray(wire());struct.pack_into('<I',raw,64+128+(8+4)*4,4);self.assertEqual(decode(raw)['stable_active_contexts'],0)
        raw=bytearray(wire());struct.pack_into('<I',raw,64+7*4,1);raw[64+32:64+96]=bytes(64);self.assertEqual(decode(raw)['missing_or_unstable_contexts'],1)
    def test_truncated_and_duplicate_phase_refuse(self):
        with self.assertRaises(ValueError):decode(wire()[:-1])
        raw=bytearray(wire());struct.pack_into('<I',raw,64+128+4,1)
        with self.assertRaises(ValueError):decode(raw)


if __name__=='__main__':unittest.main()
