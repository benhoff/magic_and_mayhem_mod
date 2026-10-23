#!/usr/bin/env python3
"""Strict bounded accepted-store census decoding, independent of game timing."""
from pathlib import Path
import struct
import sys
import unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from native_cpu_store_profile import decode

class ProfileTest(unittest.TestCase):
    def record(self, flags=0):
        header=b'MNMSTP01'+struct.pack('<8I',1,40,40,64,1,flags,6,0)
        row=struct.pack('<5I',0x401234,6,12,2,3)+bytes.fromhex('668907')+bytes(13)+struct.pack('<I',0)
        return header+row
    def test_complete_counts_and_exact_instruction(self):
        x=decode(self.record());self.assertTrue(x['complete']);self.assertEqual((x['stores'],x['bytes']),(6,12))
        self.assertEqual(x['sites'][0]['instruction'],'668907')
    def test_drops_caps_and_conflicts_are_incomplete(self):
        for flag in [1,2,4,8,15]:self.assertFalse(decode(self.record(flag))['complete'])
    def test_header_extent_and_reserved_fields_refuse(self):
        raw=self.record()
        for bad in [raw[:-1],raw+b'\0',b'BADMAGIC'+raw[8:],raw[:36]+struct.pack('<I',1)+raw[40:]]:
            with self.assertRaises(ValueError):decode(bad)
    def test_forged_counts_width_code_and_sites_refuse(self):
        raw=self.record()
        for offset,value in [(24,65),(28,16),(32,5),(44,0),(48,13),(52,3),(56,16),(76,1)]:
            bad=bytearray(raw);struct.pack_into('<I',bad,offset,value)
            with self.subTest(offset=offset):
                with self.assertRaises(ValueError):decode(bytes(bad))
        bad=bytearray(raw);bad[63]=1
        with self.assertRaises(ValueError):decode(bytes(bad))

if __name__=='__main__':unittest.main()
