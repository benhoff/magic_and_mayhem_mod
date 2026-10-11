#!/usr/bin/env python3
"""Mutation guards for exact runtime bindings and nonsemantic classification."""
import copy
from pathlib import Path
import struct
import sys
import tempfile
import unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from drawing_api_journal import BUILD_SHA256,plan,trampoline
from drawing_api_bindings import decode,physical


def fixture():
    row={'address':0x58b700,'bytes':'ff1508535c00','mnemonic':'call','operand':'DWORD PTR ds:0x5c5308'}
    item=plan(row['address'],{row['address']:row});item['tag']=0
    manifest={'original_sha256':BUILD_SHA256,'hooks':[item]}
    code,rest=trampoline(item,0,0)
    h=[0]*32;h[2:14]=[1,128,48,1,0x400000,0x700000,0x10000,0x710000,0x10000,0x710100,0x720000,0x10000]
    header=bytearray(struct.pack('<32I',*h));header[:8]=b'MNMDAB01';header[64:96]=bytes.fromhex(BUILD_SHA256)
    binding=[0,item['site'],item['length'],item['call_length'],0x800000,len(code),0x800000+rest,0x700100,0x700200,0x800000+item['call_length'],item['site']+item['call_length'],0]
    end=bytearray(64);end[:8]=b'MNMDABEN';struct.pack_into('<2I',end,8,1,1)
    return manifest,header,binding,end


class BindingTests(unittest.TestCase):
    def read(self,manifest,header,row,end):
        with tempfile.TemporaryDirectory() as directory:
            path=Path(directory)/'bindings.bin';rows=row if isinstance(row[0],list) else [row]
            path.write_bytes(bytes(header)+b''.join(struct.pack('<12I',*item) for item in rows)+bytes(end));return decode(path,manifest)

    def test_exact_call_return_and_core_leave_without_logical_owner(self):
        m,h,r,e=fixture();b=self.read(m,h,r,e)
        self.assertTrue(b['integrity']);self.assertEqual(physical(0x800006,b),{'kind':'exact_api_trampoline_call_return','tag':0,'original_call_site':0x58b700,'original_call_return':0x58b706})
        self.assertEqual(physical(0x710100,b)['kind'],'exact_core_leave');self.assertIn('pending',physical(0x710100,b)['logical_parent'])

    def test_module_and_allocation_membership_does_not_assign_original_owner(self):
        m,h,r,e=fixture();b=self.read(m,h,r,e)
        self.assertEqual(physical(0x700123,b)['logical_owner'],'unassigned')
        self.assertEqual(physical(0x800005,b),{'kind':'api_trampoline_address','tag':0,'logical_owner':'unassigned'})
        for address in (0x400123,0x6fffff,0x730000,0x800010):self.assertEqual(physical(address,b)['kind'],'unassigned_address')

    def test_exact_row_fields_and_reserved_mutations_refuse(self):
        m,h,r,e=fixture()
        for index in range(12):
            if index in (7,8):continue # Other distinct in-module stub PCs are valid.
            bad=r.copy();bad[index]+=1
            with self.assertRaises(ValueError):self.read(m,h,bad,e)

    def test_stub_boundaries_and_alias_refuse(self):
        m,h,r,e=fixture()
        for entry,after in [(0x6fffff,0x700200),(0x710000,0x700200),(0x700100,0x700100),(0x700100,0x710000)]:
            bad=r.copy();bad[7:9]=[entry,after]
            with self.assertRaises(ValueError):self.read(m,h,bad,e)

    def test_allocation_overflow_or_module_overlap_refuse(self):
        m,h,r,e=fixture()
        for base in (0,0xfffffff8,0x700200,0x710200,0x720200):
            bad=r.copy();delta=base-r[4]
            for index in (4,6,9):bad[index]=(bad[index]+delta)&0xffffffff
            with self.assertRaises(ValueError):self.read(m,h,bad,e)

    def test_header_layout_build_reserved_and_count_mutations_refuse(self):
        m,h,r,e=fixture()
        for index in (2,3,4,5,6,14,15,16,23,24,31):
            bad=h.copy();struct.pack_into('<I',bad,4*index,99)
            with self.assertRaises(ValueError):self.read(m,bad,r,e)

    def test_module_null_size_overflow_overlap_and_leave_boundaries_refuse(self):
        m,h,r,e=fixture()
        for index,value in [(7,0),(8,0),(9,0),(10,0),(11,0x70ffff),(11,0x720000),(12,0),(13,0),(7,0xfffffff0),(9,0x700000)]:
            bad=h.copy();struct.pack_into('<I',bad,4*index,value)
            with self.assertRaises(ValueError):self.read(m,bad,r,e)

    def test_absent_modules_and_unknown_leave_are_explicit(self):
        m,h,r,e=fixture();struct.pack_into('<3I',h,36,0,0,0);struct.pack_into('<2I',h,48,0,0)
        b=self.read(m,h,r,e);self.assertEqual(b['core_leave'],0);self.assertEqual(b['modules']['core'],{'base':0,'size':0})
        m,h,r,e=fixture();struct.pack_into('<I',h,44,0);self.assertEqual(self.read(m,h,r,e)['core_leave'],0)

    def test_terminal_magic_version_count_reserved_and_truncation_refuse(self):
        m,h,r,e=fixture()
        for index in range(16):
            bad=e.copy();struct.pack_into('<I',bad,4*index,99)
            with self.assertRaises(ValueError):self.read(m,h,r,bad)
        with self.assertRaises(ValueError):self.read(m,h,r,e[:-1])

    def test_manifest_boolean_extent_signature_and_build_mutations_refuse(self):
        m,h,r,e=fixture()
        for key,value in [('tag',False),('site',True),('length',False),('call_length',False),('call_length',5),('signature','90'*6)]:
            bad=copy.deepcopy(m);bad['hooks'][0][key]=value
            with self.assertRaises(ValueError):self.read(bad,h,r,e)
        bad=copy.deepcopy(m);bad['original_sha256']='0'*64
        with self.assertRaises(ValueError):self.read(bad,h,r,e)

    def test_multiple_allocations_and_stubs_remain_unique(self):
        m,h,r,e=fixture();item=copy.deepcopy(m['hooks'][0]);item['tag']=1;item['site']+=0x100;item['call']['address']+=0x100;m['hooks'].append(item)
        struct.pack_into('<I',h,20,2);struct.pack_into('<I',e,12,2)
        second=r.copy();second[0]=1;second[1]+=0x100;second[10]+=0x100
        for index in (4,6,9):second[index]+=0x10000
        second[7]+=0x200;second[8]+=0x200
        self.assertEqual(len(self.read(m,h,[r,second],e)['hooks']),2)
        bad=second.copy();bad[7]=r[7]
        with self.assertRaises(ValueError):self.read(m,h,[r,bad],e)
        bad=second.copy()
        for index in (4,6,9):bad[index]-=0x10000-8
        with self.assertRaises(ValueError):self.read(m,h,[r,bad],e)

    def test_copied_instruction_address_mismatch_refuses(self):
        m,h,r,e=fixture();m['hooks'][0]['call']['address']+=1
        with self.assertRaises(ValueError):self.read(m,h,r,e)


if __name__=='__main__':unittest.main()
