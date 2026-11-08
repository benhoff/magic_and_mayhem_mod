#!/usr/bin/env python3
"""Fail-closed reader controls; synthetic wires never grant engine evidence."""
import importlib.util
from pathlib import Path
import struct
import unittest

ROOT=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('image_caller_state',ROOT/'tools/test-native-image-caller-state.py')
probe=importlib.util.module_from_spec(spec);spec.loader.exec_module(probe)

class ReaderGuards(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        raw=bytearray(struct.pack('<4I',0x494d4e4d,0x31305343,1,164))
        for index in range(19008):
            n=index;seed=n%12;n//=12;mode=n%9;n//=9;y=n%8-3;n//=8;x=n%11-4;n//=11
            a=[0x1000,0,0x200040,0xb1b2b3b4,0x120000,0xe1e2e3e4,0x51525354,0x71727374,0x202,0x200000,0xffffffc0 if mode==8 else 64,0,0,y&0xffffffff,0x4000,0x71357]
            b=a[:];b[2]=0x300040;b[9]=0x300000
            raw+=struct.pack('<41I',index+1,n,x&0xffffffff,y&0xffffffff,mode,seed,0x200000,0x300000,*a,*b,7)
        cls.wire=bytes(raw)

    def test_declared_domain_and_independent_destinations(self):
        r=probe.analyze(self.wire)
        self.assertTrue(r['equivalent']);self.assertEqual(r['counts']['complete_state_matches'],19008)

    def test_pixel_equality_does_not_hide_register_flags_or_stack_changes(self):
        for slot in [24+2,24+8,24+12]:
            raw=bytearray(self.wire);offset=16+slot*4
            struct.pack_into('<I',raw,offset,struct.unpack_from('<I',raw,offset)[0]^4)
            r=probe.analyze(raw)
            self.assertFalse(r['equivalent']);self.assertEqual(r['counts']['complete_destination_matches'],19008)

    def test_truncated_and_short_wires_refuse(self):
        for raw in [self.wire[:-1],self.wire[:16],self.wire[:-164]]:
            with self.assertRaises(ValueError):probe.analyze(raw)

    def test_duplicate_domain_refuses_even_with_full_count(self):
        raw=bytearray(self.wire);raw[16+164+4:16+164+24]=raw[20:40]
        with self.assertRaises(ValueError):probe.analyze(raw)

    def test_shared_buffers_refuse(self):
        raw=bytearray(self.wire);struct.pack_into('<I',raw,16+7*4,0x200000)
        with self.assertRaises(ValueError):probe.analyze(raw)

    def test_false_pixel_source_or_fx_check_never_equivalent(self):
        for bit in [1,2,4]:
            raw=bytearray(self.wire);struct.pack_into('<I',raw,16+40*4,7^bit)
            self.assertFalse(probe.analyze(raw)['equivalent'])

    def test_unknown_check_bits_refuse(self):
        raw=bytearray(self.wire);struct.pack_into('<I',raw,16+40*4,15)
        with self.assertRaises(ValueError):probe.analyze(raw)

    def test_shared_wrong_pitch_cannot_fake_the_declared_domain(self):
        raw=bytearray(self.wire);index=8*12
        for slot in [8+10,24+10]:struct.pack_into('<I',raw,16+index*164+slot*4,64)
        with self.assertRaises(ValueError):probe.analyze(raw)

if __name__=='__main__':unittest.main()
