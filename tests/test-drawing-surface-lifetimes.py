#!/usr/bin/env python3
"""Only successful, temporally observed queries join lease identities."""
import importlib.util
from pathlib import Path
import struct
import sys
import unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from drawing_surface_lifetimes import analyze_lifetimes
spec=importlib.util.spec_from_file_location('interval_fixture',Path(__file__).with_name('test-drawing-call-intervals.py'));m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)


def lifetime_fixture(methods,objects,related=None,failures=(),timeline=None,frame_entry=1):
    timeline=timeline or tuple((i,p) for i in range(1,len(methods)+1) for p in (1,2,3))
    raw,journal=m.fixture(timeline,methods)
    raw=bytearray(raw);journal=bytearray(journal)
    if frame_entry!=1:
        for i in range(len(methods)):
            base=64+i*128
            struct.pack_into('<8I',journal,base+48,*([1,1,8,6,16,0,0xffffffff,0xffffffff] if i+1==frame_entry else [0]*8))
            struct.pack_into('<I',journal,base+120,int(i+1>=frame_entry))
    for i,obj in enumerate(objects):
        base=64+i*128;struct.pack_into('<I',journal,base+12,obj)
        rel=(related or {}).get(i+1,0);struct.pack_into('<I',journal,base+16,rel)
        if i+1 in failures:struct.pack_into('<I',journal,base+32,0x80004005)
    for at in range(64,len(raw)-64,64):
        row=list(struct.unpack_from('<16I',raw,at));entry,phase=row[2],row[1];c=struct.unpack_from('<32I',journal,64+(entry-1)*128)
        row[5]=c[3];row[6]=0 if methods[entry-1]==0 else c[4]
        if phase>=2:row[10]=c[8]
        if phase==3:row[11:13]=[c[12],c[0]]
        struct.pack_into('<16I',raw,at,*row)
    return bytes(raw),bytes(journal)


class Lifetimes(unittest.TestCase):
    def test_successful_query_pairs_lock_and_unlock_on_distinct_interfaces(self):
        r=analyze_lifetimes(*lifetime_fixture((5,0,25,32),(1,11,12,11),{2:12}))
        self.assertEqual(r['lease_gaps'],[]);self.assertEqual(r['remaining_leases'],[])
        self.assertIn([11,12],r['alias_groups']);self.assertEqual(r['leases'][0]['release_interface'],11)
        self.assertFalse(r['producer_inputs_owned']);self.assertFalse(r['all_backing_aliases_resolved'])
    def test_failed_query_and_attached_surface_never_join(self):
        for method,failures in ((0,(2,)),(12,())):
            r=analyze_lifetimes(*lifetime_fixture((5,method,25,32),(1,11,12,11),{2:12},failures))
            self.assertEqual(len(r['lease_gaps']),1);self.assertEqual(len(r['remaining_leases']),1)
            self.assertEqual(r['query_links'],[])
    def test_future_query_cannot_repair_an_earlier_unmatched_release(self):
        r=analyze_lifetimes(*lifetime_fixture((5,25,32,0),(1,12,11,11),{4:12}))
        self.assertEqual(r['lease_gaps'][0]['reason'],'unobserved-acquire')
        self.assertEqual(len(r['remaining_leases']),1)
    def test_query_during_a_lease_rekeys_existing_ownership(self):
        r=analyze_lifetimes(*lifetime_fixture((5,25,0,32),(1,12,11,11),{3:12}))
        self.assertEqual(r['lease_gaps'],[]);self.assertEqual(r['remaining_leases'],[])
    def test_failed_release_and_zero_refcount_leave_uncertainty_visible(self):
        r=analyze_lifetimes(*lifetime_fixture((5,25,32,2),(1,11,11,11),failures=(3,)))
        self.assertEqual(len(r['remaining_leases']),1)
        self.assertEqual(r['lease_gaps'][0]['reason'],'zero-reference-release-with-open-lease')
    def test_overlapping_lease_closed_before_shutdown_is_retained_at_witness(self):
        timeline=((1,1),(1,2),(1,3),(2,1),(2,2),(3,1),(3,2),(3,3),(2,3))
        r=analyze_lifetimes(*lifetime_fixture((25,5,32),(11,1,11),timeline=timeline,frame_entry=2))
        self.assertEqual(r['remaining_leases'],[]);f=r['frames'][0]
        self.assertEqual(f['open_lease_entries_at_entry'],[1]);self.assertEqual(f['open_lease_entries_at_ack'],[])
        self.assertEqual(f['overlapping_lease_entries'],[1])
    def test_relevant_source_or_destination_lease_is_reported(self):
        timeline=((1,1),(1,2),(1,3),(2,1),(2,2),(3,1),(3,2),(3,3),(2,3))
        r=analyze_lifetimes(*lifetime_fixture((25,5,32),(1,1,1),timeline=timeline,frame_entry=2))
        self.assertEqual(r['frames_with_source_or_destination_leases'],1)
        self.assertEqual(r['frames'][0]['relevant_lease_entries'],[1])
    def test_merging_two_open_leases_never_silently_removes_ambiguity(self):
        r=analyze_lifetimes(*lifetime_fixture((5,25,25,0,32),(1,11,12,11,11),{4:12}))
        self.assertEqual(r['lease_gaps'][0]['reason'],'alias-merge-with-two-leases')
        self.assertEqual(len(r['remaining_leases']),1)
    def test_recycled_interface_generations_are_not_inferred_aliases(self):
        r=analyze_lifetimes(*lifetime_fixture((5,25,32),(1,11,12)))
        self.assertEqual(len(r['lease_gaps']),1);self.assertEqual(len(r['remaining_leases']),1)
    def test_changed_interval_identity_refuses_before_lifetime_inference(self):
        raw,journal=lifetime_fixture((5,25,32),(1,11,11));b=bytearray(raw);struct.pack_into('<I',b,64+20,7)
        with self.assertRaises(ValueError):analyze_lifetimes(b,journal)

if __name__=='__main__':unittest.main()
