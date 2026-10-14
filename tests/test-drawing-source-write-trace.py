#!/usr/bin/env python3
from pathlib import Path
import struct
import sys
import unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from drawing_source_write_trace import snapshot, marker, fault


def wire():
    h = [0]*16; h[:2] = struct.unpack('<2I',b'MNMSWT01')
    h[2:15] = [1,64,0x40209ca7,1,7,9,2,5,128,960000,962560,1,128]
    r = [0]*16; r[:10] = [1,1,7,255,1,128,800,600,800,960000]; r[12:15] = [0x1000,4,0x20000]
    return struct.pack('<16I',*h)+struct.pack('<16I',*r)


def fault_wire(offset=128,phase=2):
    r = [0]*32; r[:2] = struct.unpack('<2I',b'MNMSWF01')
    r[2:16] = [1,128,1,7,10,9,offset,int(128 <= offset < 960128),phase,0xc0000005,1,2,962560,128]
    r[16:24] = [7,0x1000,0x14c,0,8192,0,0x1000000,16]; r[24:28] = [0x1234]*4
    r[28:30] = [5,2]
    return struct.pack('<32I',*r)


class Trace(unittest.TestCase):
    def test_source_shape_and_explicit_limits(self):
        t = snapshot(wire()); self.assertEqual(t['canvases'][0]['start_relative_to_page'],128)
        self.assertFalse(t['all_backing_aliases_accounted']); self.assertFalse(t['original_source_atomicity_proved'])
    def test_header_extent_and_reserved_refuse(self):
        for offset,value in ((2*4,2),(10*4,4096),(12*4,960000),(13*4,129),(15*4,1)):
            data=bytearray(wire());struct.pack_into('<I',data,offset,value)
            with self.assertRaises(ValueError): snapshot(data)
        with self.assertRaises(ValueError): snapshot(wire()[:-1])
    def test_overlap_flags_are_recomputed(self):
        for value in (255^1,255^2,255^4,255^8):
            data=bytearray(wire());struct.pack_into('<I',data,64+3*4,value)
            with self.assertRaises(ValueError): snapshot(data)
    def test_duplicate_cache_and_missing_source_refuse(self):
        data=bytearray(wire()+wire()[64:]);struct.pack_into('<I',data,13*4,2);struct.pack_into('<I',data,128,2)
        with self.assertRaises(ValueError): snapshot(data)
        data=bytearray(wire());struct.pack_into('<I',data,6*4,8)
        with self.assertRaises(ValueError): snapshot(data)
    def test_invalid_extent_and_query_metadata_refuse(self):
        for offset,value in ((64+9*4,1),(64+6*4,801),(64+12*4,0x10000),(64+15*4,1)):
            data=bytearray(wire());struct.pack_into('<I',data,offset,value)
            with self.assertRaises(ValueError): snapshot(data)
    def test_marker_identity_and_phase_refuse(self):
        t=snapshot(wire());data=b'MNMSWA01'+struct.pack('<6I',1,32,1,7,9,2)
        marker(data,t,2)
        with self.assertRaises(ValueError): marker(data,t,3)
        with self.assertRaises(ValueError): marker(data[:-1],t,2)
    def test_fault_writer_and_page_halo(self):
        t=snapshot(wire());f=fault(fault_wire(),t)
        self.assertTrue(f['foreign_thread']);self.assertEqual(f['recorded_pixel_aliases'],[7]);self.assertFalse(f['write_resumed'])
        f=fault(fault_wire(960128),t);self.assertFalse(f['target_is_source_pixel']);self.assertEqual(f['recorded_pixel_aliases'],[])
        f=fault(fault_wire(64),t);self.assertFalse(f['target_is_source_pixel'])
    def test_arming_and_retiring_faults_stay_explicit(self):
        t=snapshot(wire())
        for phase in (1,3):self.assertEqual(fault(fault_wire(phase=phase),t)['phase'],phase)
    def test_fault_identity_image_and_code_refuse(self):
        t=snapshot(wire())
        for offset,value in ((4*4,2),(8*4,962560),(9*4,0),(10*4,4),(12*4,0),(17*4,8192),(18*4,0x8664),(23*4,15),(30*4,1)):
            data=bytearray(fault_wire());struct.pack_into('<I',data,offset,value)
            with self.assertRaises(ValueError):fault(data,t)
    def test_unknown_writer_is_not_attributed(self):
        t=snapshot(wire());data=bytearray(fault_wire())
        for i in range(16,28):struct.pack_into('<I',data,i*4,0)
        f=fault(data,t);self.assertIsNone(f['instruction_rva']);self.assertIsNone(f['code_hex']);self.assertFalse(f['pe_identity_known'])


if __name__ == '__main__': unittest.main()
