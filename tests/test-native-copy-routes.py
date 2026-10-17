#!/usr/bin/env python3
"""Incomplete or misbound route observations must never prove branch absence."""
import copy
from pathlib import Path
import struct
import sys
import tempfile
import unittest

sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
import native_copy_routes as routes


class Controls(unittest.TestCase):
    def setUp(self):
        self.tmp=tempfile.TemporaryDirectory();self.addCleanup(self.tmp.cleanup)
        self.root=Path(self.tmp.name);self.capture=self.root/'lock-capture';self.capture.mkdir()
        self.path=self.capture/'copy-routes-v1.bin'
        self.h=[*struct.unpack('<2I',b'MNMCRT01'),1,128,160,65536,1,0,1,0,1,0,0,0,1,0x400000,0x260000,0,0,1,1,*([0]*11)]
        self.r=[0]*40
        for k,v in {0:1,1:0x401234,2:0x1234,3:7,4:22,5:11,6:22,7:11,8:4,9:3,10:32768|1|2|4|16|64|16384,11:1,12:1,14:1,15:2,16:4,17:3,18:16,19:4,20:3,21:16,22:2,26:3,27:2,28:1,29:1,30:4,31:3,33:1,38:1}.items():self.r[k]=v
        self.gh=[*struct.unpack('<2I',b'MNMGCN01'),1,128,128,8192,1,0,1,0,0,0,1,0,0,0,0,0,0x400000,0x260000,1,*([0]*11)]
        self.g=[1,0x401234,0x400000,0x1234,1,2,0,7,7,1,11,22,3,4,5,4,1,0,0,4,3,4,3,0,0,3,2,1,1,4,3,1]
        self.write()

    def write(self):
        self.path.write_bytes(struct.pack('<32I',*self.h)+struct.pack('<40I',*self.r)+bytes((65536-1)*160))
        (self.capture/'gpu-copy-census-v1.bin').write_bytes(struct.pack('<32I',*self.gh)+struct.pack('<32I',*self.g)+bytes((8192-1)*128))

    def test_closed_gpu_keyed_join(self):
        self.h[21]=2;self.write()
        result=routes.validate(self.root)
        self.assertEqual(result['linked_gpu_calls'],1)
        self.assertEqual(result['branches']['keyed']['prepared_gpu_calls'],1)
        self.assertEqual(result['post_retirement_calls_excluded'],2)

    def test_cpu_keyed_is_not_gpu_coverage(self):
        self.r[10]&=~4;self.r[15]=1;self.r[33]=0
        self.h[10]=self.h[20]=0;self.h[11]=1
        self.gh[6]=self.gh[8]=self.gh[12]=0;self.write()
        (self.capture/'gpu-copy-census-v1.bin').write_bytes(struct.pack('<32I',*self.gh)+bytes(8192*128))
        result=routes.validate(self.root)
        self.assertEqual((result['branches']['keyed']['calls'],result['branches']['keyed']['gpu_source_calls']),(1,0))

    def test_incomplete_census(self):
        old=self.h[:]
        for k,v in [(6,0),(7,1),(8,2),(9,1),(14,0),(15,0),(22,1)]:
            with self.subTest(field=k):
                self.h=old[:];self.h[k]=v;self.write()
                with self.assertRaises(ValueError):routes.decode(self.path)

    def test_bad_alias_and_clipper_state(self):
        old=self.r[:]
        for mask in (128,256,32,16):
            with self.subTest(mask=mask):
                self.r=old[:];self.r[10]^=mask;self.write()
                with self.assertRaises(ValueError):routes.decode(self.path)

    def test_changed_gpu_caller(self):
        self.g[1]+=1;self.g[3]+=1;self.write()
        with self.assertRaisesRegex(ValueError,'GPU caller'):routes.validate(self.root)

    def test_changed_gpu_history(self):
        self.r[10]&=~4;self.write()
        with self.assertRaisesRegex(ValueError,'stale CPU source'):routes.decode(self.path)

    def test_module_mismatch(self):
        self.gh[19]+=1;self.write()
        with self.assertRaisesRegex(ValueError,'module mismatch'):routes.validate(self.root)

    def test_missing_gpu_link(self):
        self.r[33]=2;self.write()
        with self.assertRaisesRegex(ValueError,'missing GPU row'):routes.validate(self.root)

    def test_gpu_branch_disagreement(self):
        self.g[15]=0;self.gh[12]=0;self.write()
        with self.assertRaisesRegex(ValueError,'GPU caller'):routes.validate(self.root)

    def test_tail_and_counter_corruption(self):
        self.h[19]=0;self.write()
        with self.assertRaisesRegex(ValueError,'counters'):routes.decode(self.path)
        self.h[19]=1;self.write()
        with self.path.open('r+b') as f:f.seek(-1,2);f.write(b'X')
        with self.assertRaisesRegex(ValueError,'unused tail'):routes.decode(self.path)


if __name__=='__main__':unittest.main()
