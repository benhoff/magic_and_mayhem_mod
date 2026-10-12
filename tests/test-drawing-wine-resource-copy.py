import struct
import sys
import unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from drawing_wine_resource_copy import decode


def fixture():
    h=[0]*16;h[:2]=struct.unpack('<2I',b'MNMWCP01');h[2:6]=[1,192,0x40209ca7,131072]
    a=[0]*48;a[:7]=[1,1,1,18,7,1,10];a[10:23]=[1,0,0,2,0,0,0,0,0,0,0,7,1];a[23:29]=[0,0,800,600,0,1]
    b=a.copy();b[:2]=[2,2];b[6]=11
    t=[0]*48;t[:5]=[3,4,1,1,18]
    return bytearray(struct.pack('<16I',*h)+b''.join(struct.pack('<48I',*r) for r in (a,b,t))),dict(calls=[])


class ResourceCopy(unittest.TestCase):
    def test_exact_pair_retains_unknown_resources(self):
        r=decode(*fixture());self.assertEqual(r['unlinked_map_resources'],[1,2])
        self.assertFalse(r['async_completion_proved'])

    def test_changed_return_arguments_thread_or_prefix_refuse(self):
        for word,value in [(2,2),(3,19),(10,3),(16,1),(23,1),(31,0),(33,1)]:
            raw,res=fixture()
            if word==31:struct.pack_into('<I',raw,64+29*4,1);struct.pack_into('<I',raw,256+29*4,1)
            struct.pack_into('<I',raw,256+word*4,value)
            with self.subTest(word=word),self.assertRaises(ValueError):decode(raw,res)

    def test_null_and_unreadable_box_retained_with_failure_result(self):
        for status in (0,2):
            raw,res=fixture()
            for at in (64,256):struct.pack_into('<7I',raw,at+22*4,status,0,0,0,0,0,0)
            struct.pack_into('<I',raw,256+7*4,0x8876086c)
            self.assertEqual(decode(raw,res)['calls'][0]['box_status'],status)

    def test_incomplete_or_false_terminal_refuses(self):
        raw,res=fixture()
        with self.assertRaises(ValueError):decode(raw[:-1],res)
        struct.pack_into('<I',raw,448+3*4,0)
        with self.assertRaises(ValueError):decode(raw,res)


if __name__=='__main__':unittest.main()
