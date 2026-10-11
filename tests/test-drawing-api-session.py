#!/usr/bin/env python3
import struct,sys,unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from drawing_api_session import decode

def fixture():
    h=bytearray(64);h[:8]=b'MNMDAPI1';struct.pack_into('<6I',h,8,1,128,0x40209ca7,262144,1024,1073741824)
    a=[0]*32;a[0]=1;a[1]=1;a[2]=5;a[3]=1;a[6]=5;a[12]=1;a[13]=1;a[14]=8;a[15]=6;a[16]=16;a[29]=1;a[30]=1
    t=[0]*32;t[0]=2;t[1]=4;t[2]=2;t[6]=5;t[21]=1;t[22]=1;t[25]=96
    return bytes(h)+struct.pack('<32I',*a)+struct.pack('<32I',*t)
class TestSession(unittest.TestCase):
    def test_valid_partial_boundary(self):
        r=decode(fixture(),lambda n:bytes(96));self.assertEqual(r['pixels_observed'],48);self.assertFalse(r['all_global_presentation_routes_observed']);self.assertFalse(r['semantic_input_outcomes_complete'])
    def test_metadata_mode_never_admits_pixel_readback(self):
        b=bytearray(fixture());struct.pack_into('<I',b,32,1)
        with self.assertRaises(ValueError):decode(b)
        struct.pack_into('<I',b,64+48,0);struct.pack_into('<I',b,64+52,2);struct.pack_into('<I',b,64+120,0)
        struct.pack_into('<I',b,192+88,0);struct.pack_into('<I',b,192+100,0)
        result=decode(b);self.assertFalse(result['driver_readbacks_enabled']);self.assertEqual(result['boundary'],'api_results_without_driver_readbacks')
        struct.pack_into('<I',b,32,0)
        with self.assertRaises(ValueError):decode(b)
    def test_input_only_mode_refuses_surface_calls(self):
        b=bytearray(fixture());struct.pack_into('<I',b,32,2)
        with self.assertRaises(ValueError):decode(b)
    def test_undefined_record_kind_refuses(self):
        b=bytearray(fixture());struct.pack_into('<I',b,64+4,6)
        with self.assertRaises(ValueError):decode(b)
    def test_terminal_required(self):
        with self.assertRaises(ValueError):decode(fixture()[:-128])
    def test_typed_frame_method_results_loss_order(self):
        for offset,value in [(64,3),(64+8,17),(64+32,0x80004005),(64+48,2),(64+52,0),(64+64,32),(64+72,0x80004005),(192+92,1),(192+96,1),(192+100,0)]:
            b=bytearray(fixture());struct.pack_into('<I',b,offset,value)
            with self.subTest(offset=offset,value=value),self.assertRaises(ValueError):decode(b)
    def test_full_plane_required(self):
        with self.assertRaises(ValueError):decode(fixture(),lambda n:bytes(94))
    def test_budget_is_not_complete_session(self):
        b=bytearray(fixture());struct.pack_into('<I',b,200,3)
        with self.assertRaises(ValueError):decode(b)
if __name__=='__main__':unittest.main()
