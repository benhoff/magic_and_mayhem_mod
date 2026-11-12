#!/usr/bin/env python3
"""Mixed, stale, truncated or incomplete cursor observations cannot join."""
from pathlib import Path
import struct
import sys
import tempfile
import unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from cursor_copy_state import decode,validate,CAPACITY


class Controls(unittest.TestCase):
    def setUp(self):
        self.h=[*struct.unpack('<2I',b'MNMCCS01'),1,128,320,CAPACITY,1,0,1,1,0x40209ca7,0x400000,0x300000,1,1,0]+[0]*16
        self.ch=[0]*32
        for k,v in {2:2,6:1,8:1,15:0x400000,16:0x300000}.items():self.ch[k]=v
        self.row=[0]*80
        for k,v in {0:1,1:0x58c488,2:0x18c488,3:7,4:22,5:11,6:22,7:11,8:4,9:3,
                    10:32768|1|2|16|512|1024|16384,11:1,12:0x10,14:1,15:1,16:640,17:480,18:16,
                    19:40,20:40,21:16,26:40,27:40,30:40,31:40,40:42,41:42,42:11,43:22,
                    44:100,45:0x1234,46:1,47:2,48:1,49:5,50:6,51:640,52:480,53:2,54:1,
                    57:40,58:40,61:3,62:1,64:1,67:640,68:480,69:1,72:640,73:480,74:1,75:9,76:10,77:1,78:1}.items():self.row[k]=v
        self.original=tuple(self.row[:44])

    def raw(self):
        return struct.pack('<32I',*self.h)+struct.pack('<80I',*self.row)+bytes((CAPACITY-1)*320)

    def check(self):return decode(self.raw(),tuple(self.ch),[self.original])

    def test_bound_observation_keeps_wire_and_native_positions_distinct(self):
        row=self.check()['observations'][0]
        self.assertEqual(row['wire_point'],[5,6]);self.assertEqual(row['native_desktop_cursor'],[9,10])
        self.assertTrue(row['wire_snapshot_stable']);self.assertEqual(row['source_generation'],3)
        self.assertFalse(self.check()['actual_getcursorpos_return_joined'])

    def test_unstable_publication_is_explicit_not_silently_coherent(self):
        self.row[53]=4;self.row[77]=0
        self.assertEqual(self.check()['unstable_wire_snapshots'],1)
        self.row[77]=1
        with self.assertRaisesRegex(ValueError,'snapshot'):self.check()

    def test_odd_sequence_and_unmapped_input_are_explicit(self):
        self.row[47]=self.row[53]=3;self.row[77]=0
        self.assertFalse(self.check()['observations'][0]['wire_snapshot_stable'])
        self.row[46]=0
        with self.assertRaisesRegex(ValueError,'snapshot'):self.check()
        self.row[47:54]=[0]*7
        self.assertFalse(self.check()['observations'][0]['wire_snapshot_stable'])

    def test_hresult_and_bad_geometry_remain_observed_failure(self):
        self.row[13]=0x88760096;self.row[14]=self.row[15]=0;self.row[24:28]=[640,512,680,552];self.row[40:44]=[0]*4
        self.original=tuple(self.row[:44]);result=self.check()
        self.assertEqual(result['failed_calls'],[1]);self.assertFalse(result['observations'][0]['source_inside'])

    def test_header_completeness_and_reserved_fields(self):
        old=self.h[:]
        for field,value in [(2,2),(3,64),(4,256),(5,1),(6,0),(7,1),(8,2),(9,0),(10,0),(16,1)]:
            self.h=old[:];self.h[field]=value
            with self.subTest(field=field),self.assertRaises(ValueError):self.check()

    def test_source_census_module_count_and_missing_state_refuse(self):
        old=self.ch[:]
        for field,value in [(2,1),(6,2),(8,2),(15,0x500000),(16,0x200000)]:
            self.ch=old[:];self.ch[field]=value
            with self.subTest(field=field),self.assertRaises(ValueError):self.check()
        self.ch=old;self.h[15]=1
        with self.assertRaises(ValueError):self.check()

    def test_exact_copy_binding_not_just_caller_or_count(self):
        old=self.row[:]
        for field in [0,1,2,3,8,9,12,13,24,28,40,43]:
            self.row=old[:];self.row[field]+=1
            with self.subTest(field=field),self.assertRaisesRegex(ValueError,'binding'):self.check()

    def test_state_reserved_flags_and_nonzero_tail(self):
        old=self.row[:]
        for field,value in [(46,3),(77,2),(78,2),(79,1)]:
            self.row=old[:];self.row[field]=value
            with self.subTest(field=field),self.assertRaises(ValueError):self.check()
        self.row=old;data=bytearray(self.raw());data[-1]=1
        with self.assertRaisesRegex(ValueError,'tail'):decode(data,tuple(self.ch),[self.original])

    def test_truncated_appended_and_duplicate_rows_refuse(self):
        for data in [self.raw()[:-1],self.raw()+b'X']:
            with self.assertRaises(ValueError):decode(data,tuple(self.ch),[self.original])
        self.h[6]=self.h[8]=2
        with self.assertRaisesRegex(ValueError,'coverage'):self.check()

    def test_no_selected_calls_does_not_claim_natural_reachability(self):
        self.h[6]=self.h[8]=0
        result=decode(struct.pack('<32I',*self.h)+bytes(CAPACITY*320),tuple(self.ch),[])
        self.assertEqual(result['selected_calls'],0);self.assertFalse(result['original_driver_equivalence'])

    def test_actual_strict_v2_reader_is_required(self):
        with tempfile.TemporaryDirectory() as temporary:
            capture=Path(temporary)/'lock-capture';capture.mkdir()
            h=[0]*32;h[:6]=[*struct.unpack('<2I',b'MNMCRT02'),2,128,176,65536]
            for k,v in {6:1,8:1,11:1,14:1,15:0x400000,16:0x300000}.items():h[k]=v
            route=capture/'copy-routes-v2.bin'
            route.write_bytes(struct.pack('<32I',*h)+struct.pack('<44I',*self.original)+bytes(65535*176))
            (capture/'cursor-copy-state-v1.bin').write_bytes(self.raw())
            self.assertEqual(validate(temporary)['selected_calls'],1)
            h[14]=0;route.write_bytes(struct.pack('<32I',*h)+struct.pack('<44I',*self.original)+bytes(65535*176))
            with self.assertRaises(ValueError):validate(temporary)


if __name__=='__main__':unittest.main(verbosity=2)
