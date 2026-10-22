"""Fail closed for incomplete/changed real-driver cohorts."""
import importlib.util
from pathlib import Path
import struct
import unittest

spec=importlib.util.spec_from_file_location('driver',Path(__file__).resolve().parents[1]/'tools/original_canvas_driver.py')
driver=importlib.util.module_from_spec(spec);spec.loader.exec_module(driver)


class DriverTest(unittest.TestCase):
    def test_incomplete(self):
        for raw in (b'',b'x',struct.pack('<6I',99,99,0,0,4,0)):
            with self.assertRaises(ValueError):driver.analyze(raw)

    def test_retained_discovery_trace_and_mutations(self):
        # Literal real-driver event types/results; leases have fixed 48-word cells.
        rows=[]
        def add(k,p,i,h=0,a=0,b=0):rows.append([k,p,i,h,a,b])
        def lock(p,i,d):
            add(42,p,i,0,1,16);add(44,p,i,0,0,123);add(45,p,i,0,48,d)
        for op in range(2):
            for i in range(2):
                add(40,10*op,i,0,0x10004040 if i else 0x840,i+1)
                add(41,10*op,i,0,0x1000400,0x1234);lock(10*op,i,1)
            for i in range(2):
                p=10*op+1;add(46,p,i,driver.LOST if i else 0,op)
                if op:
                    if i:add(42,p,i,driver.LOST,1);add(43,p,i)
                    lock(p,i,0)
                add(41,p,i,0,0x1000400,0xabcd)
                if i and not op:add(42,p,i,driver.LOST,1);add(43,p,i)
                lock(p,i,1)
            for i in range(2):add(47,10*op+2,i,0,0,1-i)
        add(99,99,0,0,4)
        def encode(rs):return b''.join(struct.pack('<6I',*r) for r in rs)
        self.assertEqual(driver.analyze(encode(rows))['cohorts'],4)
        for n,row in enumerate(rows):
            changed=[r[:] for r in rows];changed[n][3]^=1
            with self.subTest(event=n):
                with self.assertRaises(ValueError):driver.analyze(encode(changed))
        with self.assertRaises(ValueError):driver.analyze(encode(rows[:-2]+rows[-1:]))


if __name__=='__main__':unittest.main()
