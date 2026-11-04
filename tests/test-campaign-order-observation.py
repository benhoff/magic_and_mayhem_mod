#!/usr/bin/env python3
import struct
import sys
from pathlib import Path
import unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from campaign_order_observation import decode


def sample(kind,call=0):
    r=[0]*64;r[2]=42;r[3]=kind;r[4]=35;r[5]=call
    r[8:12]=[5,4,1,0];r[12]=8
    r[28:34]=[1,0,20,0,20,4096]
    return r


def wire(*rows,version=1):
    data=b'MNMCOA01'+struct.pack('<II',version,272 if version==3 else 256)
    for i,r in enumerate(rows,1):
        r=r.copy();r[0]=i;r[1]=100+i
        data+=struct.pack('<68I' if version==3 else '<64I',*r)
    return data


class Orders(unittest.TestCase):
    def pair(self):
        a=sample(2,1);b=sample(3,1);b[4]|=4
        record=struct.pack('<IIIBhhhBB',0,5,21,3,23,58,6,0,0)
        b[30]+=21;b[31]+=1;b[32]+=21;b[34:36]=[21,1]
        b[36:60]=struct.unpack('<24I',record+bytes(96-len(record)))
        return a,b

    def test_selection_and_submission_are_separate_from_delivery(self):
        result=decode(wire(sample(1),*self.pair()))
        self.assertEqual(result['rows'][0]['primary'],[3])
        self.assertEqual(result['rows'][-1]['records'][0]['destination'],[23,58,6])
        self.assertFalse(result['command_delivery_proved'])
        self.assertEqual(result['pending_callbacks'],[])

    def test_prefix_can_end_inside_callback(self):
        self.assertEqual(decode(wire(sample(2,1)))['pending_callbacks'],[1])

    def test_unchanged_buffer_is_valid_but_not_submission(self):
        a=sample(2,1);b=sample(3,1);b[4]|=4
        self.assertEqual(decode(wire(a,b))['rows'][-1]['records'],[])

    def test_loss_foreign_oversized_and_reserved_refused(self):
        for flag in (8,16,64,0x80000000):
            r=sample(1);r[4]|=flag
            with self.subTest(flag=flag),self.assertRaises(ValueError):decode(wire(r))

    def test_invalid_selection_refused(self):
        for index,value in ((8,3),(9,0),(10,2),(20,128)):
            r=sample(1);r[index]=value
            with self.subTest(index=index),self.assertRaises(ValueError):decode(wire(r))

    def test_unpaired_or_lost_callback_refused(self):
        for rows in ((sample(3,1),),(sample(2,2),),(sample(2,1),sample(3,2))):
            with self.subTest(rows=rows),self.assertRaises(ValueError):decode(wire(*rows))

    def test_false_append_counters_lengths_count_padding_refused(self):
        for index,value in ((30,20),(31,2),(32,20),(34,20),(35,2),(38,10),(59,1)):
            a,b=self.pair();b[index]=value
            with self.subTest(index=index),self.assertRaises(ValueError):decode(wire(a,b))

    def test_partial_record_and_owner_change_refused(self):
        a,b=self.pair()
        with self.assertRaises(ValueError):decode(wire(a,b)[:-1])
        b[2]=43
        with self.assertRaises(ValueError):decode(wire(a,b))

    def test_v2_candidate_slot_is_distinct_from_selection(self):
        r=sample(1);r[4]|=64;r[63]=1
        result=decode(wire(r,version=2));row=result['rows'][0]
        self.assertEqual(result['wire_version'],2)
        self.assertEqual(row['primary'],[3])
        self.assertTrue(row['candidate_valid'])
        self.assertEqual(row['candidate_slot_plus_one'],1)
        with self.assertRaises(ValueError):decode(wire(r))

    def test_v2_null_and_unvalidated_candidate_are_distinct(self):
        r=sample(1)
        self.assertFalse(decode(wire(r,version=2))['rows'][0]['candidate_valid'])
        r[4]|=64
        self.assertTrue(decode(wire(r,version=2))['rows'][0]['candidate_valid'])

    def test_v2_candidate_requires_valid_pool_and_slot_bounds(self):
        for flag,slot in ((35,1),(34|64,1),(35|64,6)):
            r=sample(1);r[4]=flag;r[63]=slot
            with self.subTest(flag=flag,slot=slot),self.assertRaises(ValueError):decode(wire(r,version=2))

    def test_v3_keeps_candidate_and_exact_world_return_token(self):
        a=sample(1)+[10,0,0,0];a[4]|=64;a[63]=1
        b=a.copy();b[64]=11
        rows=decode(wire(a,b,version=3))['rows']
        self.assertEqual([r['world_return_ordinal'] for r in rows],[10,11])
        self.assertEqual(rows[-1]['candidate_slot_plus_one'],1)

    def test_v3_backward_return_token_and_reserved_words_refuse(self):
        a=sample(1)+[10,0,0,0];b=a.copy();b[64]=9
        with self.assertRaises(ValueError):decode(wire(a,b,version=3))
        for index in (65,66,67):
            bad=a.copy();bad[index]=1
            with self.subTest(index=index),self.assertRaises(ValueError):decode(wire(bad,version=3))

    def test_v3_return_token_wrap_is_forward(self):
        a=sample(1)+[0xffffffff,0,0,0];b=a.copy();b[64]=0
        self.assertEqual(decode(wire(a,b,version=3))['rows'][-1]['world_return_ordinal'],0)


if __name__=='__main__':unittest.main()
