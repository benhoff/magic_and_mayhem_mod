#!/usr/bin/env python3
import struct
import sys
import unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from font_lifetime import decode,LIMIT,ROWS

def row(kind,ordinal,**fields):
    words=[0]*32;words[:4]=[128,ordinal,kind,17]
    for k,v in fields.items():words[int(k[1:])]=v
    return struct.pack('<32I',*words)

def journal():
    h=bytearray(64);h[:8]=b'MNMFLT01';struct.pack_into('<5I',h,8,1,64,0x40209ca7,LIMIT,ROWS)
    entry=bytearray(row(1,1,w5=8,w6=1,w8=1,w9=0x123,w13=1));struct.pack_into('<I',entry,0,384)
    load=row(2,2,w5=8,w6=1,w8=1,w9=0x123,w10=1,w12=1,w13=1,w14=1,w15=1)
    use=row(1,3,w5=9,w6=1,w7=1,w8=1,w9=0x456,w13=1)
    glyph=row(3,4,w5=9,w6=1,w7=1,w8=1,w9=0x4a5abd,w10=3,w13=65,w14=128,w15=40)
    returned=row(2,5,w5=9,w6=1,w7=1,w8=1,w9=0x456,w10=3)
    dtor=row(1,6,w5=1,w6=1,w7=1,w8=1,w9=0x789,w13=1)
    retired=row(2,7,w5=1,w6=1,w7=1,w8=1,w9=0x789,w10=6)
    body=bytes(h)+bytes(entry)+b'owned.sft\0'+bytes(246)+load+use+glyph+returned+dtor+retired
    return body+row(4,8,w6=7,w7=len(body),w8=1)

def direct_journal():
    raw=bytearray(journal());raw[:8]=b'MNMFLT02';struct.pack_into('<I',raw,8,2)
    glyph=list(struct.unpack_from('<32I',raw,704));glyph[0]=168;glyph[10]=glyph[13]=0;glyph[23]=2;glyph[24]=1
    frame=struct.pack('<I',40)+bytes(36)
    raw=raw[:704]+struct.pack('<32I',*glyph)+frame+raw[832:]
    struct.pack_into('<I',raw,len(raw)-128+28,len(raw)-128)
    return raw

class FontJournal(unittest.TestCase):
    def mutate(self,index,word,value):
        b=bytearray(journal());positions=[64,448,576,704,832,960,1088,1216]
        struct.pack_into('<I',b,positions[index]+word*4,value);return bytes(b)
    def test_positive(self):
        p=decode(journal());self.assertEqual(len(p['loads']),1);self.assertEqual(len(p['uses']),1)
        self.assertEqual(p['live_at_detach'],{});self.assertFalse(p['acceptance_ready'])
    def test_missing_terminal(self):
        with self.assertRaises(ValueError):decode(journal()[:-128])
    def test_missing_bytes(self):
        with self.assertRaises(ValueError):decode(journal()[:-1])
    def test_owner(self):
        with self.assertRaises(ValueError):decode(self.mutate(1,3,19))
    def test_return_identity(self):
        with self.assertRaises(ValueError):decode(self.mutate(1,9,0x124))
    def test_return_nesting(self):
        with self.assertRaises(ValueError):decode(self.mutate(4,10,1))
    def test_unloaded_glyph(self):
        with self.assertRaises(ValueError):decode(self.mutate(3,7,2))
    def test_wrong_glyph_parent(self):
        with self.assertRaises(ValueError):decode(self.mutate(3,10,1))
    def test_glyph_extent(self):
        with self.assertRaises(ValueError):decode(self.mutate(3,15,39))
    def test_terminal_loss(self):
        with self.assertRaises(ValueError):decode(self.mutate(7,5,1))
    def test_terminal_live_count(self):
        with self.assertRaises(ValueError):decode(self.mutate(7,10,1))
    def test_terminal_bytes(self):
        with self.assertRaises(ValueError):decode(self.mutate(7,7,2))
    def test_record_sequence(self):
        with self.assertRaises(ValueError):decode(self.mutate(3,1,2))
    def test_token_reuse(self):
        with self.assertRaises(ValueError):decode(self.mutate(1,13,2))
    def test_replacement_flag(self):
        with self.assertRaises(ValueError):decode(self.mutate(1,16,1))
    def test_reserved(self):
        with self.assertRaises(ValueError):decode(self.mutate(2,24,1))
    def test_v2_direct_source(self):
        p=decode(direct_journal());self.assertEqual(len(p['frame_snapshots']),1)
        self.assertTrue(p['uses'][0]['direct_range_binding'])
    def test_v2_snapshot_pointer_bytes(self):
        b=direct_journal();b[832+28]=1
        with self.assertRaises(ValueError):decode(b)
    def test_v2_snapshot_header_size(self):
        b=direct_journal();struct.pack_into('<I',b,832,41)
        with self.assertRaises(ValueError):decode(b)
    def test_v2_wrong_resource(self):
        b=direct_journal();struct.pack_into('<I',b,704+28,2)
        with self.assertRaises(ValueError):decode(b)
    def test_v2_wrong_role(self):
        b=direct_journal();struct.pack_into('<I',b,704+32,2)
        with self.assertRaises(ValueError):decode(b)
    def test_v2_reserved_entry(self):
        b=direct_journal();struct.pack_into('<I',b,64+96,1)
        with self.assertRaises(ValueError):decode(b)

if __name__=='__main__':unittest.main()
