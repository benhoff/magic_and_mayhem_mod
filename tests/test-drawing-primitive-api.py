import copy
import importlib.util
from pathlib import Path
import struct
import sys
import unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from drawing_primitive_api import decode
from drawing_primitive_lifetimes import decode as lifetime_decode
spec=importlib.util.spec_from_file_location('lifetimes',Path(__file__).with_name('test-drawing-primitive-lifetimes.py'))
fixture=importlib.util.module_from_spec(spec);spec.loader.exec_module(fixture)


def inputs(active=True):
    h,r,p,c=fixture.fixture()
    life=lifetime_decode(fixture.raw(h,r),p,c,{2} if active else set())
    header=[0]*16;header[2:7]=[1,256,0x40209ca7,786433,192]
    rows=[]
    for phase in (1,2,3):
        row=[0]*64;row[:7]=[phase,phase,11,7,7,1,2]
        row[7]=0 if phase==1 else 9;row[8]=5 if phase==3 else 0
        point=[0]*48;point[:2]=[1,7];point[14]=40+phase*10
        if active:
            point[2:9]=[1,2,1,10,0x401234,0,0]
            point[11:13]=[2,1];point[16:40]=r[3][24:48]
        row[16:]=point;rows.append(row)
    terminal=[0]*64;terminal[:5]=[4,4,2,7,1];rows.append(terminal)
    api=[0]*32;api[:4]=[5,1,7,1];api[6]=7;api[8]=9;api[29]=11
    return header,rows,{11:api},life


def raw(header,rows):
    data=bytearray(struct.pack('<16I',*header));data[:8]=b'MNMPAPI1'
    return bytes(data)+b''.join(struct.pack('<64I',*r) for r in rows)


class PrimitiveApi(unittest.TestCase):
    def test_exact_identity_request_and_api_not_timing_candidates(self):
        h,r,a,l=inputs();result=decode(raw(h,r),a,l)
        self.assertEqual(result['calls'][0]['primitive'],2)
        self.assertEqual(result['active_primitive_calls'],1)
        self.assertTrue(result['exact_lifetimes_crosschecked'])
        self.assertFalse(result['source_bytes_immutable']);self.assertFalse(result['complete_presented_interval'])

    def test_outside_wrapper_is_explicit_and_unbound_execution_stays_unbound(self):
        h,r,a,l=inputs(False);result=decode(raw(h,r),a,l)
        self.assertEqual(result['active_primitive_calls'],0)
        self.assertFalse(decode(raw(h,r))['exact_lifetimes_crosschecked'])

    def test_point_identity_request_clock_and_phase_mutations_refuse(self):
        h,r,a,l=inputs()
        for row,word,value in ((0,19,99),(1,19,1),(0,20,99),(0,17,8),(0,32,97),(0,30,39),(2,30,81),(1,1,3),(0,29,1),(0,63,1),(0,23,2),(0,9,1),(1,7,8),(2,8,0)):
            rows=copy.deepcopy(r);rows[row][word]=value
            with self.subTest(row=row,word=word),self.assertRaises(ValueError):decode(raw(h,rows),a,l)

    def test_malformed_truncated_missing_or_duplicated_rows_refuse(self):
        h,r,a,l=inputs()
        for data in (b'bad',raw(h,r)[:-1],raw(h,r[:-1]),raw(h,r+r[-1:]),raw(h,[r[0],r[2],r[3]])):
            with self.assertRaises(ValueError):decode(data,a,l)
        for word in (2,3,4,5,6,7):
            header=h.copy();header[word]+=1
            with self.assertRaises(ValueError):decode(raw(header,r),a,l)

    def test_independent_completed_api_denominator_and_result_refuse(self):
        h,r,a,l=inputs()
        for word in (0,1,2,3,6,8):
            calls=copy.deepcopy(a);calls[11][word]+=1
            with self.subTest(word=word),self.assertRaises(ValueError):decode(raw(h,r),calls,l)
        calls=copy.deepcopy(a);calls[12]=calls[11].copy();calls[12][29]=12
        with self.assertRaises(ValueError):decode(raw(h,r),calls,l)

    def test_query_cannot_follow_retired_or_foreign_wrapper(self):
        h,r,a,l=inputs();l['selected_calls'][0]['thread']=8
        with self.assertRaises(ValueError):decode(raw(h,r),a,l)
        h,r,a,l=inputs();l['selected_calls'][0]['phases'][2]['qpc']=65
        with self.assertRaises(ValueError):decode(raw(h,r),a,l)


if __name__=='__main__':unittest.main()
