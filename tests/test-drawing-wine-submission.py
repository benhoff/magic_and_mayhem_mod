#!/usr/bin/env python3
import sys
from pathlib import Path
import struct
import unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from drawing_wine_submission import decode


def wire():
    h=[0]*16;h[:2]=struct.unpack('<2I',b'MNMWIN01');h[2:8]=[1,192,0x40209ca7,131072,32,2048]
    entry=[0]*48;entry[:8]=[1,1,1,1,7,42,2,9];entry[10:16]=[1,2,4,5,6,3];entry[16:24]=[0xfffffffe,3,8,9,1,2,11,12]
    returned=list(entry);returned[:3]=[2,2,1];returned[8:10]=[0x80004005,0xcafe]
    terminal=[0]*48;terminal[:5]=[3,4,1,1,7];terminal[10]=2
    return struct.pack('<16I',*h)+b''.join(struct.pack('<48I',*r) for r in [entry,returned,terminal])


class Submission(unittest.TestCase):
    def test_paired_arguments_and_unbound_scope(self):
        x=decode(wire());self.assertEqual(x['calls'][0]['api_entry'],42);self.assertFalse(x['source_submission_identity_bound']);self.assertFalse(x['egl_temporal_assignment_performed'])
    def test_shared_allocator_may_include_non_submission_objects(self):
        raw=bytearray(wire());struct.pack_into('<I',raw,64+384+10*4,3);self.assertTrue(decode(raw)['success'])
    def test_truncation_and_header_refuse(self):
        for raw in (wire()[:-1],wire()[:-192],b'BAD'+wire()[3:]):
            with self.assertRaises(ValueError):decode(raw)
    def test_changed_return_and_reserved_fields_refuse(self):
        for offset,value in ((64+192+10*4,9),(64+27*4,1),(64+15*4,4),(64+192+2*4,2),(64+2*4,2)):
            raw=bytearray(wire());struct.pack_into('<I',raw,offset,value)
            with self.subTest(offset=offset),self.assertRaises(ValueError):decode(raw)
    def test_parent_thread_and_terminal_count_refuse(self):
        call=[0]*32;call[1]=1;call[6]=7;decode(wire(),{42:call})
        call[6]=8
        with self.assertRaises(ValueError):decode(wire(),{42:call})
        raw=bytearray(wire());struct.pack_into('<I',raw,64+384+10*4,1)
        with self.assertRaises(ValueError):decode(raw)


if __name__=='__main__':unittest.main()
