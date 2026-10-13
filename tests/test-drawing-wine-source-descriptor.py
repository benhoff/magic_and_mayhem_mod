import copy
from pathlib import Path
import struct,sys,unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from drawing_wine_source_descriptor import decode


def wire():
    header=[0]*16;header[2:8]=[1,160,0x40209ca7,16384,2048,2]
    sample=[0]*40;sample[:16]=[1,1,7,3,4,0,5,8,0,9,0,0,123,12,12,8]
    sample[16:26]=[116,0,0,0,0,1,40,40,1,3200];sample[26]=2
    end=[0]*40;end[:4]=[2,4,1,1]
    return header,sample,end


def encode(parts):
    h,r,t=parts;data=bytearray(struct.pack('<16I',*h)+struct.pack('<40I',*r)+struct.pack('<40I',*t));data[:8]=b'MNMWSD01';return data


class Descriptors(unittest.TestCase):
    def test_metadata_only_and_extra_queries(self):
        r=decode(encode(wire()));self.assertEqual(r['known_descriptors'],1);self.assertEqual(r['extra_public_queries'],2)
        self.assertFalse(r['source_pixels_read']);self.assertFalse(r['source_ownership_proved'])

    def test_bad_envelope_and_reserved_identity(self):
        data=encode(wire())
        for changed in (data[:-1],data+b'\0',b'INVALID!'+data[8:]):
            with self.assertRaises(ValueError):decode(changed)
        for field,value in ((0,2),(2,0),(4,0),(10,5),(26,3),(27,1)):
            p=wire();p[1][field]=value
            with self.assertRaises(ValueError):decode(encode(p))

    def test_success_requires_exact_generation_and_nonfailed_query(self):
        for field,value in ((6,0),(7,0),(9,0),(11,0x80000000),(14,11),(15,9),(26,1)):
            p=wire();p[1][field]=value
            with self.assertRaises(ValueError):decode(encode(p))

    def test_unknown_null_failed_and_changed_remain_explicit(self):
        for status in (1,2,3,4):
            p=wire();r=p[1];r[10]=status
            if status in (1,2):r[9]=0;r[11]=r[12]=0;r[16:26]=[0]*10;r[26]=status-1
            if status==3:r[11]=0x8876086c;r[16:26]=[0]*10
            if status==4:r[15]=0
            result=decode(encode(p));self.assertEqual(result['known_descriptors'],0)
            self.assertEqual(result['samples'][0]['status'],status)

    def test_terminal_cannot_hide_missing_samples(self):
        for field,value in ((0,3),(1,1),(2,0),(3,0),(4,1)):
            p=wire();p[2][field]=value
            with self.assertRaises(ValueError):decode(encode(p))


if __name__=='__main__':unittest.main()
