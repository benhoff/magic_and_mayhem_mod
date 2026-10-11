#!/usr/bin/env python3
"""Mutation guards for CALL continuation relocation and strict API wire pairs."""
import copy
from pathlib import Path
import struct
import sys
import tempfile
import unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from drawing_api_journal import BUILD_SHA256, MAX_RECORDS, plan, trampoline, trace


def instruction(address, raw, mnemonic, operand=''):
    return {'address':address,'bytes':raw,'mnemonic':mnemonic,'operand':operand}


def fixtures(mode=None):
    mode = mode or {'kind':'register_memory','register':'edx','displacement':0x64}
    manifest = {'original_sha256':BUILD_SHA256,'hooks':[{'tag':0,'site':0x58b700,'target_mode':mode}]}
    before = [0]*32; before[:8] = [1,1,1,0,5,7,0x58b700,0x100000]
    before[9]=0x500000; before[14:18]=[2,0x500064,0x700000,1]
    before[18:26]=list(range(8));before[26:28]=[1,0x100000]
    if mode['kind']=='register':before[14:17]=[1,0,0x700000];before[10]=0x700000
    if mode['kind']=='direct_relative':before[14:17]=[4,0,mode['target']]
    if mode['kind']=='absolute_memory':before[15]=mode['address']
    after=before.copy();after[:3]=[2,2,1];after[7]+=20;after[10]=0x88760000;after[12]=123
    after[17:27]=[0]*10
    return manifest,[before,after]


def packed(manifest, rows, changes=None):
    header=bytearray(64);header[:8]=b'MNMDAPI1'
    struct.pack_into('<6I',header,8,1,64,128,0x400000,len(manifest['hooks']),MAX_RECORDS);header[32:]=bytes.fromhex(BUILD_SHA256)
    end=bytearray(64);end[:8]=b'MNMDAEND';struct.pack_into('<6I',end,8,1,len(rows),sum(r[1]==1 for r in rows),0,0,0)
    if changes:changes(header,end)
    return bytes(header)+b''.join(struct.pack('<32I',*row) for row in rows)+bytes(end)


class SpliceTests(unittest.TestCase):
    def test_absolute_call_returns_through_after_stub_before_continuation(self):
        code={0x401000:instruction(0x401000,'ff1508535c00','call','DWORD PTR ds:0x5c5308')}
        item=plan(0x401000,code);data,rest=trampoline(item,0x800000,0x900000)
        self.assertEqual(data[:6],bytes.fromhex('ff1508535c00'));self.assertEqual(rest,0x80000b)
        self.assertEqual((rest+struct.unpack_from('<i',data,7)[0])&0xffffffff,0x900000)
        self.assertEqual((0x800010+struct.unpack_from('<i',data,12)[0])&0xffffffff,0x401006)

    def test_rel32_call_keeps_actual_original_target(self):
        code={0x401000:instruction(0x401000,'e8fbffffff','call','0x401000')}
        data,rest=trampoline(plan(0x401000,code),0x800000,0x900000)
        self.assertEqual((0x800005+struct.unpack_from('<i',data,1)[0])&0xffffffff,0x401000)
        self.assertEqual(rest,0x80000a)

    def test_short_conditional_remainder_expands_without_changing_target(self):
        code={0x401000:instruction(0x401000,'ff5064','call','DWORD PTR [eax+0x64]'),
              0x401003:instruction(0x401003,'7505','jne','0x40100a')}
        item=plan(0x401000,code);data,rest=trampoline(item,0x800000,0x900000)
        self.assertEqual(rest,0x800008);self.assertEqual(data[8:10],b'\x0f\x85')
        self.assertEqual((rest+6+struct.unpack_from('<i',data,10)[0])&0xffffffff,0x40100a)
        self.assertEqual((0x800000+len(data)+struct.unpack_from('<i',data,len(data)-4)[0])&0xffffffff,0x401005)

    def test_copied_short_jump_and_return_are_preserved(self):
        code={0x401000:instruction(0x401000,'ffd0','call','eax'),
              0x401002:instruction(0x401002,'eb06','jmp','0x40100a'),
              0x401004:instruction(0x401004,'c3','ret')}
        data,rest=trampoline(plan(0x401000,code),0x800000,0x900000)
        self.assertEqual(data[7],0xe9);self.assertEqual(data[12],0xc3)
        self.assertEqual(rest,0x800007)
        self.assertEqual((rest+5+struct.unpack_from('<i',data,8)[0])&0xffffffff,0x40100a)

    def test_external_or_internal_incoming_interior_flow_refuses(self):
        code={0x401000:instruction(0x401000,'ffd0','call','eax'),
              0x401002:instruction(0x401002,'83c408','add','esp,0x8')}
        for address in (0x400ff0,0x401100):
            mutated=copy.deepcopy(code);mutated[address]=instruction(address,'eb10','jmp','0x401002')
            with self.assertRaises(ValueError):plan(0x401000,mutated)

    def test_overlap_refuses_adjacent_splice_is_allowed(self):
        code={0x401000:instruction(0x401000,'ff1508535c00','call','DWORD PTR ds:0x5c5308')}
        for occupied in [[(0x401000,0x401005)],[(0x401005,0x401010)],[(0x400fff,0x401001)]]:
            with self.assertRaises(ValueError):plan(0x401000,code,occupied)
        self.assertEqual(plan(0x401000,code,[(0x401006,0x401010)])['length'],6)

    def test_unsupported_loop_and_call_mode_refuse(self):
        code={0x401000:instruction(0x401000,'ffd0','call','eax'),
              0x401002:instruction(0x401002,'e204','loop','0x401008'),
              0x401004:instruction(0x401004,'90','nop')}
        with self.assertRaises(ValueError):plan(0x401000,code)
        code={0x401000:instruction(0x401000,'ff1488','call','DWORD PTR [eax+ecx*4]')}
        with self.assertRaises(ValueError):plan(0x401000,code)

    def test_control_transfer_operand_and_address_mismatches_refuse(self):
        code={0x401000:instruction(0x401000,'e8fbffffff','call','0x401001')}
        with self.assertRaises(ValueError):plan(0x401000,code)
        code[0x401000]=instruction(0x401001,'ff1508535c00','call','DWORD PTR ds:0x5c5308')
        with self.assertRaises(ValueError):plan(0x401000,code)

    def test_short_return_does_not_copy_into_the_next_function(self):
        code={0x401000:instruction(0x401000,'ffd0','call','eax'),
              0x401002:instruction(0x401002,'c3','ret'),
              0x401003:instruction(0x401003,'90','nop'),
              0x401004:instruction(0x401004,'90','nop')}
        with self.assertRaises(ValueError):plan(0x401000,code)


class JournalTests(unittest.TestCase):
    def decode(self, manifest, data):
        with tempfile.TemporaryDirectory() as directory:
            path=Path(directory)/'api.bin';path.write_bytes(data);return trace(path,manifest)

    def test_pair_retains_negative_hresult_lasterror_and_actual_cleanup(self):
        manifest,rows=fixtures();r=self.decode(manifest,packed(manifest,rows))
        self.assertTrue(r['journal_integrity']);self.assertEqual(r['events'],2)
        self.assertEqual(r['observed_cleanup'],[{'site':0x58b700,'bytes':20,'count':1}])
        self.assertEqual(r['outcomes'][0]['eax'],0x88760000);self.assertEqual(r['outcomes'][0]['last_error'],123)

    def test_all_target_modes_and_unreadable_target_remain_explicit(self):
        for mode in [{'kind':'register','register':'eax'},{'kind':'direct_relative','target':0x500000},{'kind':'absolute_memory','address':0x5c5014}]:
            m,rows=fixtures(mode);self.assertEqual(self.decode(m,packed(m,rows))['paired_calls'],1)
        m,rows=fixtures()
        for row in rows:row[14]=3;row[16]=0
        rows[0][17:26]=[0]*9
        r=self.decode(m,packed(m,rows));self.assertEqual(r['unreadable_target_calls'],1);self.assertEqual(r['unreadable_stack_calls'],1)

    def test_negative_actual_stack_delta_is_reported_without_guessed_abi(self):
        m,rows=fixtures();rows[1][7]=rows[0][7]-4
        self.assertEqual(self.decode(m,packed(m,rows))['observed_cleanup'][0]['bytes'],-4)

    def test_loss_open_io_reserved_and_count_terminal_mutations_refuse(self):
        m,rows=fixtures()
        for index in (3,4,5,6,7,8,15):
            def mutate(h,e,index=index):struct.pack_into('<I',e,index*4,99)
            with self.assertRaises(ValueError):self.decode(m,packed(m,rows,mutate))

    def test_truncation_magic_version_build_and_budget_refuse(self):
        m,rows=fixtures();data=packed(m,rows)
        for bad in [data[:-1],data[:-64],data+b'\0',b'badmagic'+data[8:]]:
            with self.assertRaises(ValueError):self.decode(m,bad)
        for index in (2,3,4,5,6,7,8,15):
            def mutate(h,e,index=index):struct.pack_into('<I',h,index*4,99)
            with self.assertRaises(ValueError):self.decode(m,packed(m,rows,mutate))

    def test_sequence_phase_thread_tag_site_stack_and_pair_mutations_refuse(self):
        m,rows=fixtures()
        for index,value in [(0,9),(2,9),(3,1),(4,15),(5,8),(6,0x58b701),(7,0),(14,1),(15,0),(16,0),(17,1),(18,1),(26,1),(27,0x100004)]:
            bad=copy.deepcopy(rows);bad[1][index]=value
            with self.assertRaises(ValueError):self.decode(m,packed(m,bad))

    def test_before_target_is_bound_to_preserved_register_or_iat(self):
        for mode,index in [({'kind':'register','register':'eax'},10),({'kind':'register_memory','register':'edx','displacement':0x64},9),({'kind':'absolute_memory','address':0x5c5014},15),({'kind':'direct_relative','target':0x500000},16)]:
            m,rows=fixtures(mode);rows[0][index]+=4
            with self.assertRaises(ValueError):self.decode(m,packed(m,rows))

    def test_nonzero_unreadable_and_post_stack_fields_refuse(self):
        m,rows=fixtures();rows[0][17]=0
        with self.assertRaises(ValueError):self.decode(m,packed(m,rows))
        m,rows=fixtures();rows[1][18]=123
        with self.assertRaises(ValueError):self.decode(m,packed(m,rows))

    def test_reentrant_same_site_pairs_are_lifo_per_thread(self):
        m,original=fixtures();a,b=original;inner=a.copy();inner[0]=2;inner[2]=2;inner[7]-=32;inner[27]=inner[7];inner[26]=2
        inner_after=inner.copy();inner_after[0]=3;inner_after[1]=2;inner_after[7]+=8;inner_after[17:26]=[0]*9;inner_after[26]=1
        b[0]=4;rows=[a,inner,inner_after,b]
        self.assertEqual(self.decode(m,packed(m,rows))['paired_calls'],2)
        bad=[a,inner,b,inner_after];bad[2]=bad[2].copy();bad[2][0]=3;bad[2][26]=1;bad[3]=bad[3].copy();bad[3][0]=4;bad[3][26]=0
        with self.assertRaises(ValueError):self.decode(m,packed(m,bad))

    def test_interleaved_threads_pair_independently(self):
        m,(a,b)=fixtures();c,d=copy.deepcopy((a,b));c[0]=2;c[2]=2;c[5]=9;c[26]=2
        b[0]=3;b[26]=1;d[0]=4;d[2]=2;d[5]=9
        self.assertEqual(self.decode(m,packed(m,[a,c,b,d]))['paired_calls'],2)

    def test_duplicate_or_boolean_manifest_tags_and_wrong_build_refuse(self):
        m,rows=fixtures()
        for bad in [dict(m,hooks=m['hooks']*2),dict(m,original_sha256='0'*64)]:
            with self.assertRaises(ValueError):self.decode(bad,packed(m,rows))
        m['hooks'][0]['tag']=False
        with self.assertRaises(ValueError):self.decode(m,packed(m,rows))

    def test_open_before_or_unknown_event_refuses(self):
        m,rows=fixtures()
        with self.assertRaises(ValueError):self.decode(m,packed(m,rows[:1]))
        rows[1][1]=3
        with self.assertRaises(ValueError):self.decode(m,packed(m,rows))


if __name__=='__main__':unittest.main()
