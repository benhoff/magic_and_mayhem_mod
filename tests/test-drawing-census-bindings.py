#!/usr/bin/env python3
"""Reject imprecise core allocation identities and copied-byte attribution."""
import copy
from pathlib import Path
import struct
import sys
import tempfile
import unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from drawing_census_bindings import BUILD_SHA256,decode,physical,relocated


def fixture(opcode=0xe8):
    item={'tag':0,'entry':0x500100,'length':5,'pop':8,'site':False,'signature':(bytes([opcode])+struct.pack('<i',0x500900-0x500105)).hex(),'relocations':[0]}
    manifest={'schema':1,'original_sha256':BUILD_SHA256,'hooks':[item]}
    header=bytearray(128);header[:8]=b'MNMDCB01';struct.pack_into('<8I',header,8,1,128,96,1,0x400000,0x700000,0x10000,0x700100);header[64:96]=bytes.fromhex(BUILD_SHA256)
    row=bytearray(96);struct.pack_into('<12I',row,0,0,item['entry'],5,8,0,0x800000,10,0x700200,1,0,0,0);row[48:53]=bytes.fromhex(item['signature']);row[64:74]=relocated(item,0x800000)
    end=bytearray(64);end[:8]=b'MNMDCBEN';struct.pack_into('<2I',end,8,1,1)
    return manifest,header,row,end


class CoreBindingTests(unittest.TestCase):
    def read(self,m,h,r,e):
        with tempfile.TemporaryDirectory() as directory:
            p=Path(directory)/'core-bindings.bin';p.write_bytes(h+r+e);return decode(p,m)

    def test_exact_relocated_call_maps_only_its_actual_return(self):
        m,h,r,e=fixture();b=self.read(m,h,r,e);call=physical(0x800005,b)
        self.assertEqual(call['kind'],'exact_core_trampoline_call_return');self.assertEqual(call['original_call_return'],0x500105);self.assertEqual(call['target'],0x500900)
        for pc in [0x800000,0x800004,0x800006,0x800009]:self.assertEqual(physical(pc,b)['logical_owner'],'unassigned')
        self.assertEqual(physical(0x80000a,b)['kind'],'unassigned_address')

    def test_relative_jump_is_never_a_call_return(self):
        m,h,r,e=fixture(0xe9);b=self.read(m,h,r,e);self.assertEqual(b['hooks'][0]['relocated_calls'],[]);self.assertEqual(physical(0x800005,b)['kind'],'core_trampoline_address')

    def test_exact_leave_is_pending_logical_tail_review(self):
        b=self.read(*fixture());self.assertIn('pending',physical(0x700100,b)['logical_parent']);self.assertEqual(physical(0x700101,b)['logical_owner'],'unassigned')
        self.assertEqual(physical(0x500105,b)['kind'],'unassigned_address')

    def test_all_header_layout_build_reserved_and_terminal_fields(self):
        m,h,r,e=fixture()
        for index in [0,1,2,3,4,5,6,*range(10,32)]:
            bad=h.copy();struct.pack_into('<I',bad,index*4,99)
            with self.subTest(header=index),self.assertRaises(ValueError):self.read(m,bad,r,e)
        for index in range(16):
            bad=e.copy();struct.pack_into('<I',bad,index*4,99)
            with self.subTest(end=index),self.assertRaises(ValueError):self.read(m,h,r,bad)
        with self.assertRaises(ValueError):self.read(m,h,r,e[:-1])
        with self.assertRaises(ValueError):self.read(m,h,r,e+b'\0')

    def test_row_exact_identity_and_reserved_offsets(self):
        m,h,r,e=fixture()
        for index in [0,1,2,3,4,6,8,9,10,11,22,23]:
            bad=r.copy();struct.pack_into('<I',bad,index*4,99)
            with self.subTest(index=index),self.assertRaises(ValueError):self.read(m,h,bad,e)

    def test_original_relocated_and_padding_bytes_are_exact(self):
        m,h,r,e=fixture()
        for at in range(48,88):
            bad=r.copy();bad[at]^=1
            with self.subTest(at=at),self.assertRaises(ValueError):self.read(m,h,bad,e)

    def test_module_bounds_and_named_leave_refuse(self):
        m,h,r,e=fixture()
        for index,value in [(7,0),(8,0),(7,0xfffffff0),(9,0),(9,0x6fffff),(9,0x710000)]:
            bad=h.copy();struct.pack_into('<I',bad,index*4,value)
            with self.assertRaises(ValueError):self.read(m,bad,r,e)

    def test_stub_needs_unique_in_module_address(self):
        m,h,r,e=fixture()
        for stub in [0,0x6fffff,0x710000,0x700100]:
            bad=r.copy();struct.pack_into('<I',bad,28,stub)
            with self.assertRaises(ValueError):self.read(m,h,bad,e)

    def test_allocation_rejects_overflow_null_and_module_overlap(self):
        m,h,r,e=fixture()
        for base in [0,0xfffffff8,0x700000,0x70fff8]:
            bad=r.copy();struct.pack_into('<I',bad,20,base);bad[64:88]=relocated(m['hooks'][0],base).ljust(24,b'\0')
            with self.assertRaises(ValueError):self.read(m,h,bad,e)

    def test_multiple_allocations_cannot_alias_stubs_entries_or_ranges(self):
        m,h,r,e=fixture();other=copy.deepcopy(m['hooks'][0]);other['tag']=1;other['entry']+=0x100;m['hooks'].append(other);struct.pack_into('<I',h,20,2);struct.pack_into('<I',e,12,2)
        second=r.copy();struct.pack_into('<I',second,0,1);struct.pack_into('<I',second,4,other['entry']);struct.pack_into('<I',second,20,0x810000);struct.pack_into('<I',second,28,0x700300);second[64:88]=relocated(other,0x810000).ljust(24,b'\0')
        self.assertEqual(len(self.read(m,h,r+second,e)['hooks']),2)
        bad=second.copy();struct.pack_into('<I',bad,28,0x700200)
        with self.assertRaises(ValueError):self.read(m,h,r+bad,e)
        for base in [0x800000,0x800009]:
            bad=second.copy();struct.pack_into('<I',bad,20,base);bad[64:88]=relocated(other,base).ljust(24,b'\0')
            with self.assertRaises(ValueError):self.read(m,h,r+bad,e)
        m['hooks'][1]['entry']=m['hooks'][0]['entry'];struct.pack_into('<I',second,4,m['hooks'][0]['entry']);second[64:88]=relocated(m['hooks'][1],0x810000).ljust(24,b'\0')
        with self.assertRaises(ValueError):self.read(m,h,r+second,e)

    def test_manifest_malformed_types_offsets_sites_and_signatures(self):
        m,h,r,e=fixture()
        changes=[('tag',False),('entry',True),('length',True),('pop',True),('pop',3),('site',1),('site',True),('signature','90'*5),('signature','90'*4),('relocations',[True]),('relocations',[1]),('relocations',[0,0]),('relocations',[])]
        for key,value in changes:
            bad=copy.deepcopy(m);bad['hooks'][0][key]=value
            with self.subTest(key=key,value=value),self.assertRaises(ValueError):self.read(bad,h,r,e)
        for key,value in [('schema',2),('original_sha256','0'*64),('hooks',[])]:
            bad=copy.deepcopy(m);bad[key]=value
            with self.assertRaises(ValueError):self.read(bad,h,r,e)
        with self.assertRaises(ValueError):relocated({**m['hooks'][0],'length':10,'signature':'e800000000e900000000','relocations':[0,4]},0x800000)

    def test_site_zero_pop_remains_unpaired(self):
        m,h,r,e=fixture();m['hooks'][0]['site']=True;m['hooks'][0]['pop']=0;struct.pack_into('<2I',r,12,0,1);self.assertTrue(self.read(m,h,r,e)['hooks'][0]['site'])

    def test_invalid_physical_addresses_refuse(self):
        b=self.read(*fixture())
        for pc in [-1,0x100000000,True,None,'0x800005']:
            with self.assertRaises(ValueError):physical(pc,b)


if __name__=='__main__':unittest.main()
