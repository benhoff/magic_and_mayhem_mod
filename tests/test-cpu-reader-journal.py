#!/usr/bin/env python3
from copy import deepcopy
from pathlib import Path
import struct
import importlib.util
import subprocess
import sys
import unittest
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools'))
from cpu_reader_journal import SIZE,CAPACITY,decode,chains,resolve_callers,pe_identity,wire_reads


def fixture():
    h=[0]*32;h[:2]=struct.unpack('<2I',b'MNMCRJ01');h[2:6]=[1,128,128,CAPACITY]
    h[10:19]=[1,99,0x400000,0x310000,0x10000000,0x10000,100,300,1]
    h[19:26]=[2,1,1,2,0,0,7]
    def row(event,context,reason,**fields):
        r=[0]*32;r[1:8]=[event,316,110,99,context,reason,0x58c000]
        r[8:14]=[0x400000,0x18c000,0x310000,0x40209ca7,123,512];r[14]=0x10000
        for k,v in fields.items():r[int(k[1:])]=v
        return r
    begin=row(1,1,1)
    request=row(2,1,1,w15=7,w16=9,w18=1,w19=640,w20=480,w21=5,w23=6,w28=0x20000,w29=1280,w30=9)
    complete=deepcopy(request);complete[1]=3;complete[16]=10;complete[27]=256
    acquired=row(7,1,1,w15=7,w16=11,w19=640,w20=480,w25=0xabcd,w27=1024,w31=1)
    returned=row(4,1,1,w25=0xabcd)
    end=row(6,1,1)
    release_begin=row(1,2,6)
    release=row(5,2,6,w15=7,w16=12,w19=640,w20=480,w25=0xabcd,w27=1536,w30=11,w31=1)
    release_end=row(6,2,6)
    rows=[begin,request,complete,acquired,returned,end,release_begin,release,release_end]
    h[6]=h[7]=len(rows)
    for i,r in enumerate(rows):r[0]=i+1
    wire={1:dict(sequence=42,words=[7,1,0,99,640,480,5,0,6,0])}
    return h,rows,wire


def raw(h,rows):return struct.pack('<32I',*h)+b''.join(struct.pack('<32I',*r) for r in rows)+b'\0'*(SIZE-128-len(rows)*128)


class JournalTests(unittest.TestCase):
    def test_wire_join_rejects_changed_reply_identity_and_duplicate(self):
        spec=importlib.util.spec_from_file_location('binding_fixture',ROOT/'tests/test-storage-cpu-read-binding.py')
        f=importlib.util.module_from_spec(spec);spec.loader.exec_module(f)
        rows,count,_,_,body=f.fixture()
        joined,counts,_=wire_reads(f.cap(rows),'fixture',count,body)
        self.assertEqual(joined[1]['words'],[1,1,0,77,2,2,1,0,9,0])
        self.assertEqual(counts['completed'],1)
        for offset in range(0,64,4):
            changed=deepcopy(rows);data=bytearray(body);data[offset]^=1
            changed[1]=('cpu-read',dict(sequence=3),bytes(data))
            with self.subTest(offset=offset),self.assertRaises(ValueError):wire_reads(f.cap(changed),'fixture',count,body)
        with self.assertRaises(ValueError):wire_reads(f.cap([*rows[:2],rows[1],*rows[2:]]),'fixture',count,body)
        with self.assertRaises(ValueError):wire_reads(f.cap(rows),'wrong',count,body)
        with self.assertRaises(ValueError):wire_reads(f.cap(rows),'fixture',count,body[:-1]+b'\1')

    def test_complete_chain_and_zero_epoch(self):
        h,r,w=fixture();a,b=decode(raw(h,r));result=chains(a,b,w)
        self.assertTrue(result['complete_observed_chain']);self.assertEqual(result['dc_borrows'],1)
        self.assertFalse(result['requests'][0]['original_upstream_caller_proved'])

    def test_drop_overflow_uncommitted_and_gate_refuse(self):
        for index,value in ((8,1),(9,1),(7,8),(25,3),(10,0),(23,1)):
            h,r,_=fixture();h[index]=value
            with self.subTest(index=index),self.assertRaises(ValueError):decode(raw(h,r))

    def test_partial_tail_and_reserved_fields_refuse(self):
        h,r,_=fixture();data=bytearray(raw(h,r));data[-1]=1
        with self.assertRaises(ValueError):decode(data)
        with self.assertRaises(ValueError):decode(data[:-1])
        h[31]=1
        with self.assertRaises(ValueError):decode(raw(h,r))

    def test_changed_request_completion_or_context_refuse(self):
        for row,index,value in ((1,23,7),(2,28,0x30000),(2,30,8),(2,27,0),(1,30,8),(2,5,2),(0,31,99)):
            h,r,w=fixture();r[row][index]=value
            with self.subTest(row=row,index=index),self.assertRaises(ValueError):chains(*decode(raw(h,r)),w)

    def test_duplicate_ticket_and_missing_reply_refuse(self):
        h,r,w=fixture();r[2][1]=2
        with self.assertRaises(ValueError):chains(*decode(raw(h,r)),w)
        h,r,w=fixture()
        with self.assertRaises(ValueError):chains(*decode(raw(h,r)),{})

    def test_wrong_dc_and_failed_release_cannot_retire(self):
        for index,value in ((25,0xabce),(26,0x80004005)):
            h,r,w=fixture();r[7][index]=value
            with self.assertRaises(ValueError):chains(*decode(raw(h,r)),w)

    def test_absent_checkpoint_is_an_explicit_gap(self):
        h,r,w=fixture();r[7][27]=1024;result=chains(*decode(raw(h,r)),w)
        self.assertFalse(result['complete_observed_chain'])
        self.assertEqual(result['missing_links'][0]['kind'],'release_checkpoint_or_owner_unproved')

    def test_tick_wrap_and_changed_module_rva(self):
        h,r,w=fixture();h[16]=0xfffffff0;h[17]=20
        for row in r:row[3]=1
        chains(*decode(raw(h,r)),w)
        r[0][9]+=1
        with self.assertRaises(ValueError):decode(raw(h,r))

    def test_ambiguous_modules_are_never_guessed(self):
        c=dict(context=1,rva='0x100',extent=4096,timestamp=1,header_fnv=2,header_bytes=512)
        modules=[dict(identity=[4096,1,2,512],sha256='a',name='one.dll'),dict(identity=[4096,1,2,512],sha256='b',name='two.dll')]
        self.assertFalse(resolve_callers([c],modules)[0]['module_resolved'])
        self.assertEqual(resolve_callers([c],modules[:1])[0]['module'],'one.dll')

    def test_pe_header_bound_and_unknown_image_refuse(self):
        for data in (b'',b'MZ'+b'\0'*62):
            with self.assertRaises(ValueError):pe_identity(data)

    def test_launcher_rejects_conflicting_routes_before_build(self):
        variants=[[],['--software','--skip-intros'],['--software','--copy-callers'],['--software','--operation-census'],['--software','--sample-stalls'],['--software','--stall-world-host']]
        for extra in variants:
            p=subprocess.run([sys.executable,'-B',str(ROOT/'tools/run-rendering-acceptance-pilot.py'),
                '--cpu-reader-journal','--scenario','normal-storage','--report','research/runtime/rendering-normal-results-20261031/not-launched-reader-journal.json',*extra],capture_output=True,text=True,timeout=10)
            self.assertEqual(p.returncode,2,p.stdout+p.stderr)


if __name__=='__main__':unittest.main()
