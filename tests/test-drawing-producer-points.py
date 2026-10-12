#!/usr/bin/env python3
"""Point identities must match API rows and the exact live producer prefix."""
import importlib.util
from pathlib import Path
import struct
import sys
import unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from drawing_producer_points import decode_points
spec=importlib.util.spec_from_file_location('interval_fixture',Path(__file__).with_name('test-drawing-call-intervals.py'));m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)


def fixture():
    intervals,journal=m.fixture()
    calls=list(struct.iter_unpack('<32I',journal[64:-128]))
    producer=[0]*24;producer[0]=96;producer[1]=1;producer[2]=1;producer[3]=77;producer[5]=8;producer[6]=6
    records=[(tuple(producer),b'')]
    h=bytearray(64);h[:8]=b'MNMDSRC1';struct.pack_into('<4I',h,8,1,192,0x40209ca7,786433)
    rows=[]
    for c in calls:
        for phase in (1,2,3):
            r=[0]*48;r[0]=len(rows)+1;r[1]=phase;r[2]=c[29];r[4]=c[2];r[5]=c[3];r[6]=c[4];r[7]=c[6]
            if phase>=2:r[9]=c[8]
            if phase==3:r[3]=c[0];r[10]=c[12]
            point=[64,1,19,1,0,0,77,8,6,8,0,0,1,0,c[6],0]
            r[16:32]=point;r[32:48]=point;rows.append(r)
    r=[0]*48;r[0]=len(rows)+1;r[1]=4;r[2]=2;r[3]=3;r[7]=1;r[10]=1;rows.append(r)
    return bytes(h)+b''.join(struct.pack('<48I',*r) for r in rows),journal,intervals,records


class Points(unittest.TestCase):
    def test_matched_partial_metadata_cannot_claim_pixels_or_frames(self):
        r=decode_points(*fixture());self.assertEqual(r['frames_with_known_generations'],1)
        self.assertFalse(r['source_inputs_owned']);self.assertFalse(r['coherent_presented_frames']);self.assertFalse(r['complete_presented_journey'])
    def test_result_method_thread_frame_and_prefix_identity_refuse(self):
        raw,journal,intervals,records=fixture()
        for offset,value in ((64+16,7),(64+28,3),(256+36,0x80004005),(448+40,2),(64+19*4,2),(64+22*4,78),(64+23*4,7)):
            b=bytearray(raw);struct.pack_into('<I',b,offset,value)
            with self.subTest(offset=offset),self.assertRaises(ValueError):decode_points(b,journal,intervals,records)
    def test_pair_snapshot_mismatch_and_reserved_output_refuse(self):
        raw,journal,intervals,records=fixture()
        for offset,value in ((64+35*4,0),(64+46*4,8),(64+47*4,1),(64+11*4,1),(64+18*4,27)):
            b=bytearray(raw);struct.pack_into('<I',b,offset,value)
            with self.subTest(offset=offset),self.assertRaises(ValueError):decode_points(b,journal,intervals,records)
    def test_missing_duplicate_and_changed_terminal_refuse(self):
        raw,journal,intervals,records=fixture()
        for bad in (raw[:-192],raw+bytes(192),raw[:64]+raw[256:]):
            with self.assertRaises(ValueError):decode_points(bad,journal,intervals,records)
        b=bytearray(raw);struct.pack_into('<I',b,len(b)-192+36,1)
        with self.assertRaises(ValueError):decode_points(b,journal,intervals,records)
    def test_unknown_and_stopped_association_stay_explicit(self):
        raw,journal,intervals,records=fixture();b=bytearray(raw)
        for at in range(64,len(b)-192,192):
            for start in (16,32):
                struct.pack_into('<I',b,at+(start+2)*4,7)
                for i in range(6,12):struct.pack_into('<I',b,at+(start+i)*4,0)
        r=decode_points(b,journal,intervals,records);self.assertEqual(r['frames_with_known_generations'],0);self.assertEqual(r['frames_with_live_producer_points'],0)
    def test_released_canvas_cannot_be_bound_at_later_prefix(self):
        raw,journal,intervals,records=fixture();release=list(records[0][0]);release[1]=2;release[2]=2;records.append((tuple(release),b''));b=bytearray(raw)
        for at in range(64,len(b)-192,192):
            for start in (16,32):struct.pack_into('<I',b,at+(start+3)*4,2)
        with self.assertRaises(ValueError):decode_points(b,journal,intervals,records)

if __name__=='__main__':unittest.main()
