import struct
import sys
import unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from drawing_storage_transitions import decode,bind_producer,bind_queries


def fixture():
    h=[0]*16;h[:2]=struct.unpack('<2I',b'MNMCST02');h[2:7]=[1,96,0x40209ca7,131072,512]
    a=[0]*24;a[:10]=[1,1,1,2,18,0x58b660-0x400000,10,0,0,7]
    a[10:20]=[1,800,600,800,3,2,3,800,600,1600]
    b=a.copy();b[:2]=[2,2];b[20:22]=[1,1]
    t=[0]*24;t[:10]=[3,4,1,1,0,10,0,0,3,1]
    return bytearray(struct.pack('<16I',*h)+b''.join(struct.pack('<24I',*r) for r in [a,b,t]))


def query_fixture():
    raw=fixture();struct.pack_into('<I',raw,8,3)
    m=[64,2,19,10,0,0,7,800,600,800,0,0,0,0,18,1]
    q=[3,3,1,1,1,1,1,0]+m
    t=list(struct.unpack_from('<24I',raw,256));t[0]=4;t[10]=1
    return raw[:256]+struct.pack('<24I24I',*q,*t)


def candidate_fixture():
    raw=bytearray(query_fixture());struct.pack_into('<I',raw,8,4)
    q=list(struct.unpack_from('<24I',raw,256));q[4:7]=[0,0,0];q[10]=3;q[14:20]=[0]*6
    c=[4,5,1,7,1,31,16,1,800,600,800,1,0x1000,4,0x20000,0x1000,4,0x20000,4,10,0,18,1,0]
    t=list(struct.unpack_from('<24I',raw,352));t[0]=5;t[8]=4;t[11]=1
    return raw[:256]+struct.pack('<24I24I24I',*q,*c,*t)


class StorageTransitions(unittest.TestCase):
    def test_v5_known_input_matches_cache_and_legacy_reserved_remains_strict(self):
        raw=bytearray(query_fixture());struct.pack_into('<I',raw,8,5);struct.pack_into('<I',raw,256+7*4,1)
        self.assertEqual(decode(raw)['queries'][0]['input_address'],1)
        struct.pack_into('<I',raw,8,4)
        with self.assertRaises(ValueError):decode(raw)
        struct.pack_into('<I',raw,8,5);struct.pack_into('<I',raw,256+7*4,2)
        with self.assertRaises(ValueError):decode(raw)

    def test_v5_pending_unknown_input_stays_separate_from_epoch_and_canvas(self):
        raw=bytearray(query_fixture());struct.pack_into('<I',raw,8,5)
        for word,value in [(4,0),(5,0),(6,0),(7,4),(10,131)]:struct.pack_into('<I',raw,256+word*4,value)
        for word in range(14,20):struct.pack_into('<I',raw,256+word*4,0)
        struct.pack_into('<I',raw,352+8*4,4)
        result=decode(raw);self.assertEqual(result['queries'][0]['input_address'],4)
        self.assertEqual(result['queries'][0]['epoch'],0);self.assertEqual(result['queries'][0]['metadata'][6],0)
        for value in (513,5):
            bad=bytearray(raw);struct.pack_into('<I',bad,256+7*4,value)
            with self.assertRaises(ValueError):decode(bad)
        struct.pack_into('<I',raw,256+7*4,0);struct.pack_into('<I',raw,352+8*4,3)
        self.assertEqual(decode(raw)['queries'][0]['input_address'],0)

    def test_v5_candidate_input_must_equal_its_query(self):
        raw=bytearray(candidate_fixture());struct.pack_into('<I',raw,8,5);struct.pack_into('<I',raw,256+7*4,4)
        result=decode(raw);self.assertEqual(result['candidates'][0]['input_address'],result['queries'][0]['input_address'])
        struct.pack_into('<I',raw,352+18*4,3)
        with self.assertRaises(ValueError):decode(raw)

    def test_candidate_offset_and_public_region_are_observation_only(self):
        r=decode(candidate_fixture());self.assertEqual(r['candidates'][0]['delta'],16)
        self.assertFalse(r['candidates'][0]['physical_alias_proved'])
        self.assertEqual(r['query_epochs_known'],0)

    def test_candidate_wrong_query_epoch_span_region_or_budget_refuses(self):
        for word,value in [(2,2),(4,17),(5,29),(6,960000),(7,0),(11,513),(19,11),(22,2),(23,1)]:
            raw=bytearray(candidate_fixture());struct.pack_into('<I',raw,352+word*4,value)
            with self.subTest(word=word),self.assertRaises(ValueError):decode(raw)

    def test_external_query_identity_is_exact_and_omissions_are_visible(self):
        r=decode(query_fixture());m=r['queries'][0]['metadata']
        points=dict(points=[dict(metadata=m)])
        self.assertEqual(bind_queries(r,points,{}, {})['known_epoch_queries'],1)
        self.assertEqual(bind_queries(r,{}, {}, {})['unlinked_query_ids'],[1])
        wrong=m.copy();wrong[6]=8
        with self.assertRaises(ValueError):bind_queries(r,dict(points=[dict(metadata=wrong)]),{}, {})

    def test_query_binds_exact_observed_epoch(self):
        r=decode(query_fixture());self.assertEqual(r['query_epochs_known'],1)
        self.assertEqual(r['queries'][0]['epoch'],1)
        self.assertFalse(r['queries'][0]['allocation_ownership_proved'])

    def test_query_cannot_invent_epoch_identity_or_live_state(self):
        for word,value in [(2,2),(3,3),(4,2),(5,2),(6,2),(7,1),(9,1),(10,23),(14,8),(23,2)]:
            raw=bytearray(query_fixture());struct.pack_into('<I',raw,256+word*4,value)
            with self.subTest(word=word),self.assertRaises(ValueError):decode(raw)

    def test_changed_address_retains_unknown_epoch(self):
        raw=bytearray(query_fixture())
        for word,value in [(4,0),(5,4),(6,3)]:struct.pack_into('<I',raw,256+word*4,value)
        struct.pack_into('<I',raw,352+8*4,4)
        r=decode(raw);self.assertEqual(r['query_epochs_known'],0)
        self.assertEqual(r['queries'][0]['status'],3)

    def test_explicit_relocated_caller_requires_version_and_exact_lock_site(self):
        raw=fixture();struct.pack_into('<I',raw,8,2)
        for at in (64,160):
            struct.pack_into('<I',raw,at+5*4,0x18d329)
            struct.pack_into('<I',raw,at+22*4,1)
        r=decode(raw);self.assertTrue(all(e['caller_relocated'] for e in r['events']))
        struct.pack_into('<I',raw,8,1)
        with self.assertRaises(ValueError):decode(raw)
        struct.pack_into('<I',raw,8,2);struct.pack_into('<I',raw,64+5*4,0x18d328)
        with self.assertRaises(ValueError):decode(raw)

    def test_caller_mapping_cannot_change_between_phases(self):
        raw=fixture();struct.pack_into('<I',raw,8,2)
        for at in (64,160):struct.pack_into('<I',raw,at+5*4,0x18d329)
        struct.pack_into('<I',raw,64+22*4,1)
        with self.assertRaises(ValueError):decode(raw)

    def test_cross_stream_generation_prefix_and_return_address(self):
        create=[0]*24;create[1:4]=[1,1,7];create[5:7]=[800,600]
        records=[(tuple(create),b'')]
        observed=decode(fixture())
        for e in observed['events']:e['producer_prefix']=1
        self.assertEqual(bind_producer(observed,records)['known_cache_points'],2)
        observed['events'][0]['producer_prefix']=0
        with self.assertRaises(ValueError):bind_producer(observed,records)
        observed['events'][0]['producer_prefix']=1;observed['events'][1]['returned_address']=2
        with self.assertRaises(ValueError):bind_producer(observed,records)

    def test_closed_lock_return_is_observation_only(self):
        r=decode(fixture());self.assertEqual(r['observed_lock_return_epochs'],1)
        self.assertTrue(r['closed']);self.assertFalse(r['allocation_ownership_proved'])
        self.assertFalse(r['all_lock_calls_observed'])

    def test_null_return_still_advances_observed_epoch(self):
        raw=fixture();struct.pack_into('<I',raw,64+96+21*4,0)
        self.assertEqual(decode(raw)['observed_lock_return_epochs'],1)

    def test_wrong_return_identity_epoch_order_or_budget(self):
        for word,value in [(2,2),(3,1),(4,19),(5,0),(6,9),(10,513),(20,2),(22,1)]:
            raw=fixture();struct.pack_into('<I',raw,64+96+word*4,value)
            with self.subTest(word=word),self.assertRaises(ValueError):decode(raw)

    def test_open_call_is_retained_and_cannot_claim_closed(self):
        raw=fixture();t=[0]*24;t[:10]=[2,4,1,0,1,10,0,0,3,0]
        r=decode(raw[:160]+struct.pack('<24I',*t));self.assertFalse(r['closed'])
        self.assertEqual(r['open_calls'],[1])
        t[4]=0
        with self.assertRaises(ValueError):decode(raw[:160]+struct.pack('<24I',*t))

    def test_non_lock_cannot_export_raw_result(self):
        raw=fixture()
        for at in [64,160]:struct.pack_into('<I',raw,at+3*4,0)
        with self.assertRaises(ValueError):decode(raw)

    def test_missing_trailing_or_address_identity_gap(self):
        raw=fixture()
        with self.assertRaises(ValueError):decode(raw[:-1])
        with self.assertRaises(ValueError):decode(raw[:160])
        struct.pack_into('<I',raw,64+192+8*4,4)
        with self.assertRaises(ValueError):decode(raw)


if __name__=='__main__':unittest.main()
