#!/usr/bin/env python3
from pathlib import Path
import struct,sys,tempfile,unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from drawing_source_completion import wait_for_source


def marker(n=120,failures=0):return b'MNMPDONE'+struct.pack('<6I',1,n,200,failures,100,20000)
class Process:
    def __init__(self,value=None):self.value=value
    def poll(self):return self.value


class Completion(unittest.TestCase):
    def test_advertised_budget_and_nonregular_marker_refuse(self):
        from drawing_source_completion import decode_completion
        for word,value in ((4,1048577),(6,10000),(7,536870913),(7,100)):
            raw=bytearray(marker());struct.pack_into('<I',raw,word*4,value)
            with self.assertRaises(ValueError):decode_completion(raw,120)
        with tempfile.TemporaryDirectory() as d:
            p=Path(d)/'actual';p.write_bytes(marker());link=Path(d)/'link';link.symlink_to(p)
            with self.assertRaises((OSError,ValueError)):wait_for_source(link,120,Process(),lambda:None)

    def test_nonfinite_and_boolean_timeout_refuse(self):
        with tempfile.TemporaryDirectory() as d:
            for timeout in (True,float('inf'),float('nan')):
                with self.assertRaises(ValueError):wait_for_source(Path(d)/'done',120,Process(),lambda:None,timeout=timeout)
    def test_heartbeat_until_exact_marker(self):
        with tempfile.TemporaryDirectory() as d:
            p=Path(d)/'done';ticks=[0];beats=[]
            def heartbeat():
                beats.append(1)
                if len(beats)==3:p.write_bytes(marker())
            x=wait_for_source(p,120,Process(),heartbeat,timeout=1,monotonic=lambda:ticks[0],sleep=lambda n:ticks.__setitem__(0,ticks[0]+n));self.assertEqual(len(beats),3);self.assertEqual(x['expected_queues'],120);self.assertFalse(x['semantic_outcomes_assumed'])
    def test_dead_and_timeout_refuse(self):
        with tempfile.TemporaryDirectory() as d:
            p=Path(d)/'done'
            with self.assertRaises(ValueError):wait_for_source(p,120,Process(0),lambda:None)
            tick=[0]
            with self.assertRaises(ValueError):wait_for_source(p,120,Process(),lambda:None,timeout=.1,monotonic=lambda:tick[0],sleep=lambda n:tick.__setitem__(0,tick[0]+n))
    def test_wrong_count_failure_and_corruption_refuse(self):
        with tempfile.TemporaryDirectory() as d:
            p=Path(d)/'done'
            for raw in (marker(16),marker(failures=1),marker()[:-1],b'BADMARK!'+marker()[8:]):
                p.write_bytes(raw)
                with self.assertRaises(ValueError):wait_for_source(p,120,Process(),lambda:None)


if __name__=='__main__':unittest.main()
