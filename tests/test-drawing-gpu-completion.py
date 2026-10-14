import struct
import sys
from pathlib import Path
import unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from drawing_gpu_completion import decode


def journal(status=2,wait=0x911c):
    h=[0]*16;h[2:7]=[1,128,0x40209ca7,131072,1000000000]
    data=bytearray(struct.pack('<16I',*h));data[:8]=b'MNMWGC01'
    r=[0]*32;r[:18]=[1,1,1,2,3,4,10,12,status,1,wait,2,7,1,1,1,1000000000,0]
    if status==1:r[9:11]=[0,0];r[12:16]=[0]*4
    t=[0]*32;t[:4]=[2,4,1,1];t[8]=int(status>2);t[9]=int(status==1);t[10]=int(status==2)
    return data+struct.pack('<32I',*r)+struct.pack('<32I',*t)


class Cases(unittest.TestCase):
    def test_both_signal_results(self):
        for wait in (0x911a,0x911c):self.assertEqual(decode(journal(wait=wait))['signaled'],1)
    def test_no_context_is_unwitnessed(self):
        r=decode(journal(status=1));self.assertEqual(r['unwitnessed'],1);self.assertFalse(r['all_contexts_completed'])
    def test_timeout_refuses(self):
        with self.assertRaises(ValueError):decode(journal(status=5,wait=0x911b))
        self.assertFalse(decode(journal(status=5,wait=0x911b),complete=False)['success'])
    def test_changed_status_refuses(self):
        with self.assertRaises(ValueError):decode(journal(wait=0x911b))
    def test_extra_reserve_refuses(self):
        d=journal();struct.pack_into('<I',d,64+72,1)
        with self.assertRaises(ValueError):decode(d)
    def test_unclosed_refuses(self):
        with self.assertRaises(ValueError):decode(journal()[:-128])


if __name__=='__main__':unittest.main()
