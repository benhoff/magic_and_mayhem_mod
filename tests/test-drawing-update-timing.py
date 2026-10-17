#!/usr/bin/env python3
import struct
import sys
import unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from drawing_update_timing import decode


def fixture():
    h=[0]*16;h[:2]=struct.unpack('<2I',b'MNMDAPI1');h[2:9]=[1,128,0x40209ca7,262144,1024,1073741824,1]
    c=[0]*32;c[0:3]=[1,2,1];c[6]=7;c[8]=32768;c[9]=44;c[21]=27;c[29]=1
    end=[0]*32;end[:3]=[2,4,1];end[6]=7;end[21]=1
    journal=struct.pack('<16I',*h)+struct.pack('<32I',*c)+struct.pack('<32I',*end)
    h[:2]=struct.unpack('<2I',b'MNMUTIM1');h[2:9]=[1,64,0x40209ca7,786433,1000,0,0]
    rows=[]
    for phase in (1,2,3):
        r=[0]*16;r[:7]=[phase,phase,1,7,2,1,100+phase]
        if phase>=2:r[8:11]=[32768,44,27]
        if phase==3:r[11]=1
        rows.append(r)
    r=[0]*16;r[:7]=[4,4,1,2,0,0,105];rows.append(r)
    return struct.pack('<16I',*h)+b''.join(struct.pack('<16I',*r) for r in rows),journal


class Timing(unittest.TestCase):
    def test_consumed_input_identity_and_clock(self):
        raw,journal=fixture();r=decode(raw,journal)
        self.assertEqual(r['frequency'],1000);self.assertEqual(r['calls'][0]['arguments'][0],27)
        self.assertEqual(r['calls'][0]['return_qpc'],102);self.assertFalse(r['source_ownership_proved'])
    def test_missing_or_duplicate_call_phase_refuses(self):
        raw,journal=fixture()
        for data in (raw[:128]+raw[192:],raw[:128]+raw[64:128]+raw[128:]):
            with self.assertRaises(ValueError):decode(data,journal)
    def test_changed_header_identity_result_input_frame_reserved_refuse(self):
        raw,journal=fixture()
        for offset in (0,8,12,16,20,32,64+8,64+12,64+16,64+20,128+32,128+36,128+40,192+44,192+48,64+52):
            data=bytearray(raw);struct.pack_into('<I',data,offset,struct.unpack_from('<I',data,offset)[0]+1)
            with self.assertRaises(ValueError):decode(data,journal)
    def test_reversed_time_and_incomplete_terminal_refuse(self):
        raw,journal=fixture()
        for offset,value in ((128+24,90),(256+16,1),(256+24,102),(256+32,1)):
            data=bytearray(raw);struct.pack_into('<I',data,offset,value)
            with self.assertRaises(ValueError):decode(data,journal)
    def test_equal_qpc_ticks_allowed_without_inventing_subtick_order(self):
        raw,journal=fixture();data=bytearray(raw)
        for offset in (64+24,128+24,192+24):struct.pack_into('<I',data,offset,101)
        self.assertTrue(decode(data,journal)['all_admitted_calls_timestamped'])


if __name__=='__main__':unittest.main()
