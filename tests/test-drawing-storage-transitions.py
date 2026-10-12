import struct
import sys
import unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from drawing_storage_transitions import decode,bind_producer


def fixture():
    h=[0]*16;h[:2]=struct.unpack('<2I',b'MNMCST02');h[2:7]=[1,96,0x40209ca7,131072,512]
    a=[0]*24;a[:10]=[1,1,1,2,18,0x58b660-0x400000,10,0,0,7]
    a[10:20]=[1,800,600,800,3,2,3,800,600,1600]
    b=a.copy();b[:2]=[2,2];b[20:22]=[1,1]
    t=[0]*24;t[:10]=[3,4,1,1,0,10,0,0,3,1]
    return bytearray(struct.pack('<16I',*h)+b''.join(struct.pack('<24I',*r) for r in [a,b,t]))


class StorageTransitions(unittest.TestCase):
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
