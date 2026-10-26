#!/usr/bin/env python3
"""Positive observations cannot erase missing callbacks or admit corrupt rows."""
from pathlib import Path
import importlib.util,struct,sys,unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
import native_copy_routes as strict
import published_copy_routes as published
spec=importlib.util.spec_from_file_location('strict_fixtures',Path(__file__).with_name('test-native-copy-routes.py'));fixture=importlib.util.module_from_spec(spec);spec.loader.exec_module(fixture)

class Controls(unittest.TestCase):
    def setUp(self):
        # Reuse the existing independent wire fixture, not new reader output.
        fixture.Controls.setUp(self)
        self.path.unlink();self.path=self.capture/'copy-routes-v2.bin'
        self.h[1]=struct.unpack('<I',b'RT02')[0];self.h[2]=2;self.h[4]=176
        self.r.extend([7,7,1,2]);self.h[7]=self.h[9]=1;self.h[22]=2;self.write()
    def write(self):
        words=44 if len(self.r)==44 else 40
        self.path.write_bytes(struct.pack('<32I',*self.h)+struct.pack('<'+str(words)+'I',*self.r)+bytes((65536-1)*words*4))
        (self.capture/'gpu-copy-census-v1.bin').write_bytes(struct.pack('<32I',*self.gh)+struct.pack('<32I',*self.g)+bytes((8192-1)*128))
    def check(self):return published.validate_published(self.root)[2]
    def test_known_miss_is_positive_only_and_strict_still_refuses(self):
        r=self.check();self.assertFalse(r['application_observation_complete']);self.assertFalse(r['absence_claimed']);self.assertEqual(r['active_miss_reason_bits'],2)
        self.assertEqual(r['linked_gpu_calls'],1)
        with self.assertRaisesRegex(ValueError,'missed'):strict.validate(self.root)
    def test_complete_metadata_still_does_not_assert_absence(self):
        self.h[7]=self.h[9]=self.h[22]=0;self.write();r=self.check();self.assertTrue(r['application_observation_complete']);self.assertFalse(r['absence_claimed']);strict.validate(self.root)
    def test_unknown_inconsistent_or_missing_reason_refuses(self):
        old=self.h[:]
        for k,v in [(22,8),(22,0),(7,0),(9,0),(23,1)]:
            with self.subTest(k=k,v=v):
                self.h=old[:];self.h[k]=v;self.write()
                with self.assertRaises(ValueError):self.check()
    def test_unclosed_exhausted_or_dropped_rows_refuse(self):
        old=self.h[:]
        for k,v in [(6,0),(6,65537),(8,2),(14,0),(15,0),(16,0),(2,1)]:
            with self.subTest(k=k,v=v):
                self.h=old[:];self.h[k]=v;self.write()
                with self.assertRaises(ValueError):self.check()
    def test_counter_tail_and_selected_identity_corruption_refuses(self):
        self.h[19]=0;self.write()
        with self.assertRaises(ValueError):self.check()
        self.h[19]=1;old=self.r[:]
        for k,v in [(0,2),(2,0),(6,0),(10,self.r[10]^128),(33,2),(40,8),(43,5)]:
            with self.subTest(k=k):
                self.r=old[:];self.r[k]=v;self.write()
                with self.assertRaises(ValueError):self.check()
        self.r=old;self.write()
        with self.path.open('r+b') as f:f.seek(-1,2);f.write(b'X')
        with self.assertRaises(ValueError):self.check()
    def test_gpu_gaps_module_and_caller_mismatch_refuse(self):
        self.gh[7]=1;self.write()
        with self.assertRaises(ValueError):self.check()
        self.gh[7]=0;self.gh[19]+=1;self.write()
        with self.assertRaises(ValueError):self.check()
        self.gh[19]-=1;self.g[1]+=1;self.write()
        with self.assertRaises(ValueError):self.check()

if __name__=='__main__':unittest.main()
