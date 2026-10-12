#!/usr/bin/env python3
"""Causal overlap, pairing, CPU leases and exact API cross-binding guards."""
import struct
import sys
import unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from drawing_call_intervals import decode_intervals


def fixture(timeline=((1,1),(1,2),(1,3),(2,1),(2,2),(2,3)),methods=(5,7)):
    h=bytearray(64);h[:8]=b'MNMDAPI1';struct.pack_into('<7I',h,8,1,128,0x40209ca7,262144,1024,1073741824,3)
    calls=[]
    for i,m in enumerate(methods,1):
        r=[0]*32;r[0]=i;r[1]=1;r[2]=m;r[3]=1;r[4]=2 if m in (5,7) else 0;r[6]=i;r[29]=i
        if i==1:r[12]=1;r[13]=1;r[14]=8;r[15]=6;r[16]=16;r[18]=r[19]=0xffffffff
        r[30]=1;calls.append(r)
    end=[0]*32;end[0]=len(calls)+1;end[1]=4;end[2]=2;end[6]=1;end[21]=len(calls);end[22]=1;end[25]=192
    journal=bytes(h)+b''.join(struct.pack('<32I',*r) for r in calls+[end])
    h=bytearray(64);h[:8]=b'MNMDINT1';struct.pack_into('<4I',h,8,1,64,0x40209ca7,786433)
    events=[];pending=0
    for entry,phase in timeline:
        c=calls[entry-1];r=[0]*16;r[0]=len(events)+1;r[1]=phase;r[2]=entry;r[3:7]=c[1:5];r[7]=c[6];r[8]=c[5]
        if phase==1:pending+=1
        if phase>=2:r[10]=c[8]
        if phase==3:r[11]=c[12];r[12]=c[0]
        r[13]=pending;events.append(r)
        if phase==3:pending-=1
    end=[0]*16;end[0]=len(events)+1;end[1]=4;end[2]=2;end[7]=1;end[10:14]=[len(calls)+1,1,len(calls),pending]
    return bytes(h)+b''.join(struct.pack('<16I',*r) for r in events+[end]),journal


class IntervalTest(unittest.TestCase):
    def test_sequential_calls_are_scoped_not_owned(self):
        r=decode_intervals(*fixture())
        self.assertEqual(r['frames_with_competing_mutations'],0)
        self.assertFalse(r['producer_inputs_owned']);self.assertFalse(r['complete_presented_journey'])
    def test_other_thread_can_write_after_return_before_physical_ack(self):
        r=decode_intervals(*fixture(((1,1),(1,2),(2,1),(2,2),(2,3),(1,3))))
        self.assertEqual(r['frames'][0]['competing_entries'],[2]);self.assertEqual(r['frames_with_other_thread_mutations'],1)
        self.assertEqual(r['max_pending'],2)
    def test_overlap_during_original_call_is_detected(self):
        r=decode_intervals(*fixture(((1,1),(2,1),(1,2),(2,2),(2,3),(1,3))))
        self.assertEqual(r['frames_with_competing_mutations'],1)
    def test_changed_identity_result_pending_frame_and_terminal_refuse(self):
        raw,journal=fixture()
        for offset,value in ((64+20,2),(128+40,0x80004005),(64+52,0),(192+44,2),(192+48,2),(448+48,3),(448+52,1)):
            b=bytearray(raw);struct.pack_into('<I',b,offset,value)
            with self.subTest(offset=offset),self.assertRaises(ValueError):decode_intervals(b,journal)
    def test_missing_duplicate_or_reordered_points_refuse(self):
        for timeline in (((1,1),(1,3),(2,1),(2,2),(2,3)),((1,1),(1,2),(1,2),(1,3),(2,1),(2,2),(2,3)),((1,1),(1,2),(1,3),(2,1))):
            with self.assertRaises(ValueError):decode_intervals(*fixture(timeline))
    def test_impossible_entry_order_and_journal_completion_before_prior_return_refuse(self):
        for timeline in (((2,1),(2,2),(2,3),(1,1),(1,2),(1,3)),((1,1),(2,1),(2,2),(2,3),(1,2),(1,3))):
            with self.assertRaises(ValueError):decode_intervals(*fixture(timeline))
    def test_cpu_lease_is_visible_until_unlock_and_failed_unlock_keeps_it(self):
        # First entry remains the physical Blt; following entries own CPU leases.
        timeline=tuple((i,p) for i in (1,2,3) for p in (1,2,3))
        raw,journal=fixture(timeline,(5,25,32));r=decode_intervals(raw,journal)
        self.assertEqual(r['remaining_leases'],[]);self.assertEqual(len(r['lease_events']),2)
        b=bytearray(journal);struct.pack_into('<I',b,64+2*128+32,0x80004005)
        e=bytearray(raw)
        for event in (8,9):struct.pack_into('<I',e,64+(event-1)*64+40,0x80004005)
        r=decode_intervals(e,b);self.assertEqual(r['remaining_leases'],[dict(object=1,kind='cpu',entry=2)])
    def test_unknown_version_extra_bytes_and_nonphysical_journal_refuse(self):
        raw,journal=fixture()
        for b in (raw[:-1],raw+bytes(64),raw[:8]+struct.pack('<I',2)+raw[12:]):
            with self.assertRaises(ValueError):decode_intervals(b,journal)
        b=bytearray(journal);struct.pack_into('<I',b,32,1)
        with self.assertRaises(ValueError):decode_intervals(raw,b)

if __name__=='__main__':unittest.main()
