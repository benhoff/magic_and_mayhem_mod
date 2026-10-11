#!/usr/bin/env python3
"""V2 full argument, saved-output pointer and raw buffer integrity mutations."""
import copy
import importlib.util
from pathlib import Path
import struct
import sys
import tempfile
import unittest
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools'))
from drawing_api_details import trace,MAX_RECORDS,RECORD_BYTES
from drawing_api_journal import BUILD_SHA256
from drawing_api_contracts import UNKNOWN_WORDS
spec=importlib.util.spec_from_file_location('v1fixtures',ROOT/'tests/test-drawing-api-journal.py');v1=importlib.util.module_from_spec(spec);spec.loader.exec_module(v1)


def fixture(words=13,slots=None):
    m,base=v1.fixtures();m.update(journal_version=2,synthetic_fixture=True)
    m['hooks'][0]['details']={'name':'fixture','origin':'synthetic_fixture','formal_words':words,'snapshots':slots or []}
    n=16 if words==UNKNOWN_WORDS else words;rows=[r+[0]*168 for r in base]
    for r in rows:r[32:36]=[words,n,0,len(slots or [])]
    rows[0][34]=1;rows[0][36:36+n]=[0x1000+i*16 for i in range(n)]
    rows[0][18:18+min(n,8)]=rows[0][36:36+min(n,8)]
    if words!=UNKNOWN_WORDS:rows[1][7]=rows[0][7]+words*4
    for index,plan in enumerate(slots or []):
        for row in rows:
            if plan['events']&row[1]:
                offset=56+index*36;row[offset:offset+4]=[rows[0][36+plan['argument']],plan['bytes'],1,plan['bytes']]
                payload=bytes([row[1]])*plan['bytes']+bytes(128-plan['bytes']);row[offset+4:offset+36]=struct.unpack('<32I',payload)
    return m,rows


def packed(m,rows):
    h=bytearray(64);h[:8]=b'MNMDAPI2';struct.pack_into('<6I',h,8,2,64,RECORD_BYTES,0x400000,len(m['hooks']),MAX_RECORDS);h[32:]=bytes.fromhex(BUILD_SHA256)
    e=bytearray(64);e[:8]=b'MNMDAEND';struct.pack_into('<6I',e,8,2,len(rows),sum(r[1]==1 for r in rows),0,0,0)
    return bytes(h)+b''.join(struct.pack('<200I',*r) for r in rows)+bytes(e)


class DetailTests(unittest.TestCase):
    def read(self,m,rows):return self.raw(m,packed(m,rows))
    def raw(self,m,data):
        with tempfile.TemporaryDirectory() as directory:
            p=Path(directory)/'v2.bin';p.write_bytes(data);return trace(p,m)

    def test_thirteen_words_and_actual_fifty_two_byte_cleanup(self):
        m,rows=fixture();r=self.read(m,rows);self.assertEqual(r['observed_cleanup'][0]['bytes'],52)
        self.assertEqual(r['details']['argument_layouts'][0]['formal_words'],13)
        self.assertEqual(rows[0][48],0x10c0)

    def test_zero_and_unreviewed_layouts_remain_distinct(self):
        for words in [0,UNKNOWN_WORDS]:
            m,rows=fixture(words);r=self.read(m,rows);self.assertEqual(r['details']['argument_layouts'][0]['formal_words'],words)

    def test_saved_output_pointer_and_before_after_payload(self):
        plan={'argument':12,'bytes':8,'events':3,'name':'fixture buffer'};m,rows=fixture(slots=[plan]);r=self.read(m,rows)
        self.assertEqual([x['address'] for x in r['details']['buffer_snapshots']],[0x10c0,0x10c0])
        self.assertEqual([x['data'] for x in r['details']['buffer_snapshots']],['01'*8,'02'*8])

    def test_post_pointer_cannot_come_from_cleaned_stack(self):
        m,rows=fixture(slots=[{'argument':12,'bytes':4,'events':2,'name':'out'}]);rows[1][56]+=4
        with self.assertRaises(ValueError):self.read(m,rows)

    def test_layout_reserved_stack_availability_and_padding_mutations_refuse(self):
        m,rows=fixture()
        for event,index,value in [(0,32,12),(0,33,12),(0,34,2),(0,35,1),(0,49,1),(0,55,1),(1,34,1),(1,36,1),(1,7,rows[1][7]+4),(0,36,1)]:
            bad=copy.deepcopy(rows);bad[event][index]=value
            with self.assertRaises(ValueError):self.read(m,bad)

    def test_null_unreadable_and_unavailable_states_keep_no_payload(self):
        plan={'argument':1,'bytes':4,'events':3,'name':'buffer'}
        for state in [2,3,4]:
            m,rows=fixture(slots=[plan])
            if state==2:rows[0][19]=rows[0][37]=0
            if state==4:rows[0][34]=0;rows[0][36:52]=[0]*16
            for row in rows:row[56:92]=[0 if state in (2,4) else 0x1010,4,state,0]+[0]*32
            self.assertTrue(self.read(m,rows)['journal_integrity'])
            rows[1][60]=1
            with self.assertRaises(ValueError):self.read(m,rows)

    def test_snapshot_size_state_unused_bytes_and_event_mutations_refuse(self):
        m,rows=fixture(slots=[{'argument':1,'bytes':4,'events':2,'name':'out'}])
        for event,index,value in [(0,56,1),(1,57,8),(1,58,0),(1,59,8),(1,61,1),(1,92,1)]:
            bad=copy.deepcopy(rows);bad[event][index]=value
            with self.assertRaises(ValueError):self.read(m,bad)

    def test_receiver_candidate_is_saved_byte_equality_only(self):
        m,rows=fixture();rows[0][18]=rows[0][36]=0x600000
        for row in rows:row[52:55]=[0x600000,0x500000,1]
        self.assertEqual(self.read(m,rows)['details']['receiver_candidate_calls'],1)
        for event,index,value in [(0,52,0x600004),(0,53,0x500004),(0,54,2),(1,53,0x500004)]:
            bad=copy.deepcopy(rows);bad[event][index]=value
            with self.assertRaises(ValueError):self.read(m,bad)

    def test_base_pair_and_terminal_mutations_still_refuse(self):
        m,rows=fixture()
        for index in [0,2,3,4,5,6,14,15,26,27]:
            bad=copy.deepcopy(rows);bad[1][index]+=1
            with self.assertRaises(ValueError):self.read(m,bad)
        raw=packed(m,rows)
        for bad in [raw[:-1],raw[:-64],raw+b'\0',b'MNMDAPI1'+raw[8:]]:
            with self.assertRaises(ValueError):self.raw(m,bad)

    def test_original_metadata_cannot_assign_an_unsupported_offset_type(self):
        m,rows=fixture();m['synthetic_fixture']=False
        with self.assertRaises(ValueError):self.read(m,rows)


if __name__=='__main__':unittest.main()
