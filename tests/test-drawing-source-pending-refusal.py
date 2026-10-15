import struct
import sys
import unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from drawing_source_pending_refusal import decode


def fixture():
    h=[0]*16;h[:2]=struct.unpack('<2I',b'MNMSPF01');h[2:10]=[1,64,0x40209ca7,128,2,55,13,18];h[13:15]=[2,1]
    a=[1,1,7,18,0,0x6ba01,13,4,2,13,1,0,1,1,0,0]
    b=[2,2,2,19,0,0x18bc00,2,0,0,0,0,0,0,0,0,0]
    return bytearray(struct.pack('<16I16I16I',*h,*a,*b))


class Pending(unittest.TestCase):
    def test_identity_is_not_overlap_or_admission(self):
        r=decode(fixture());self.assertEqual(r['pending_frames'],2)
        self.assertEqual(r['frames'][1]['object_canvas'],2)
        self.assertFalse(r['source_copy_admitted']);self.assertFalse(r['source_overlap_proved'])

    def test_count_source_flags_and_reserved_refuse(self):
        for offset,value in [(24,1),(52,1),(60,1),(64+10*4,0),(128+1*4,1),(128+13*4,2),(128+15*4,1)]:
            raw=fixture();struct.pack_into('<I',raw,offset,value)
            with self.assertRaises(ValueError):decode(raw)

    def test_unknown_record_extent_and_caller_refuse(self):
        for offset,value in [(128+7*4,1),(128+5*4,0x1c0000),(128+2*4,99)]:
            raw=fixture();struct.pack_into('<I',raw,offset,value)
            with self.assertRaises(ValueError):decode(raw)
        with self.assertRaises(ValueError):decode(fixture()[:-1])


if __name__=='__main__':unittest.main()
