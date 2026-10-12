import struct
import sys
import unittest
from pathlib import Path
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
from drawing_wine_map_storage_point import decode


def fixture():
    h=[0]*16; h[2:6]=[1,96,0x40209ca7,131072]
    p=[0]*24; p[:8]=[1,2,5,9,1,18,10,1]
    p[8:24]=[64,1,19,70,4,0,8,800,600,800,11,11,0,0,18,0]
    t=[0]*24; t[:7]=[2,4,1,1,0,18,12]
    raw=bytearray(struct.pack('<16I24I24I',*h,*p,*t)); raw[:8]=b'MNMWSP01'
    calls=dict(calls=[dict(call=5,method=2,result=0,object=9,map_interval=1,thread=18,
                           row_pitch=1600,slice_pitch=960000,texture=12,texture_generation=3)])
    return raw,calls


class StoragePoint(unittest.TestCase):
    def test_v2_query_identity_and_stopped_without_query(self):
        raw,calls=fixture();struct.pack_into('<I',raw,100,2);struct.pack_into('<I',raw,156,7)
        self.assertEqual(decode(raw,calls)['matched'],1)
        struct.pack_into('<I',raw,156,0)
        with self.assertRaises(ValueError):decode(raw,calls)
        struct.pack_into('<I',raw,104,7);struct.pack_into('<6I',raw,120,0,0,0,0,0,0)
        self.assertEqual(decode(raw,calls)['unknown'],1)

    def test_exact_live_point_is_bounded(self):
        raw,calls=fixture(); r=decode(raw,calls)
        self.assertEqual(r['matched'],1)
        self.assertEqual(r['points'][0]['canvas'],8)
        self.assertFalse(r['source_upload_identity_proved'])
        self.assertFalse(r['source_ownership_proved'])

    def test_wrong_interval_thread_extent_or_cache_state(self):
        for word,value in [(4,2),(5,19),(10,23),(13,1),(17,799),(23,1)]:
            raw,calls=fixture();struct.pack_into('<I',raw,64+word*4,value)
            with self.assertRaises(ValueError):decode(raw,calls)

    def test_unknown_and_ambiguous_cache_are_retained(self):
        for flags in (3,7,11,35,131):
            raw,calls=fixture();struct.pack_into('<I',raw,104,flags)
            struct.pack_into('<6I',raw,120,0,0,0,0,0,0)
            r=decode(raw,calls);self.assertEqual(r['unknown'],1)
            self.assertEqual(r['points'][0]['canvas'],0)

    def test_pending_transition_cannot_also_match(self):
        raw,calls=fixture();struct.pack_into('<I',raw,104,147)
        with self.assertRaises(ValueError):decode(raw,calls)

    def test_missing_failed_or_extra_map_point_refuses(self):
        raw,calls=fixture();calls['calls'][0]['result']=0x80000000
        with self.assertRaises(ValueError):decode(raw,calls)
        raw,calls=fixture()
        with self.assertRaises(ValueError):decode(raw[:-1],calls)
        t=[1,4,1,0,0,18,12]+[0]*17
        with self.assertRaises(ValueError):decode(raw[:64]+struct.pack('<24I',*t),calls)


if __name__=='__main__':unittest.main()
