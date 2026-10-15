#!/usr/bin/env python3
import importlib.util,struct,unittest
from pathlib import Path
root=Path(__file__).resolve().parents[1]
s=importlib.util.spec_from_file_location('v4_producers',root/'tools/inspect-canvas-producers.py');m=importlib.util.module_from_spec(s);s.loader.exec_module(m)


def wire(queues=1):
    h=[0]*16;h[:2]=struct.unpack('<2I',b'MNMPRO04');h[2:9]=[4,64,0x40209ca7,1048576,queues,1,536870912];rows=[]
    for q in range(1,queues+1):
        for kind in (11,12):
            r=[0]*24;r[:3]=[96,len(rows)+1,kind];r[14]=q;rows.append(r)
    late=[0]*24;late[:3]=[96,len(rows)+1,5];late[3]=1;late[12:14]=[2,2];late[14]=0x07e0;rows.append(late)
    end=[0]*24;end[:3]=[96,len(rows)+1,27];end[14:18]=[queues,queues,queues,queues+1];end[19:23]=[300,0,7,3];rows.append(end)
    return struct.pack('<16I',*h)+b''.join(struct.pack('<24I',*r) for r in rows)


class Whole(unittest.TestCase):
    def test_postlude_and120_queues(self):
        for n in (1,16,120):
            h,rows=m.decode(wire(n));self.assertEqual(h[6],n);self.assertEqual(rows[-2][0][2],5);self.assertEqual(rows[-1][0][2],27)
    def test_partial_prefix_allowed_but_not_complete(self):
        partial=wire()[:-96];m.decode(partial,complete=False)
        with self.assertRaises(ValueError):m.decode(partial)
    def test_budget_flags_end_counter_owner_and_extra_data_refuse(self):
        for offset,value in ((20,262144),(28,0),(32,134217728),(24,121),(len(wire())-96+17*4,8),(len(wire())-96+21*4,0),(len(wire())-96+22*4,1)):
            raw=bytearray(wire());struct.pack_into('<I',raw,offset,value)
            with self.assertRaises(ValueError):m.decode(raw)
        with self.assertRaises(ValueError):m.decode(wire()+wire()[64:160])
    def test_v4_definition_count_limit(self):
        raw=wire();header=raw[:64];frame=bytearray(40);struct.pack_into('<I',frame,0,40)
        records=[]
        for i in range(4097):
            r=[0]*24;r[:3]=[136,i+1,25];r[14]=9;r[18]=r[19]=40;records.append(struct.pack('<24I',*r)+frame)
        m.decode(header+b''.join(records[:4096]),complete=False)
        with self.assertRaises(ValueError):m.decode(header+b''.join(records),complete=False)
    def test_legacy_headers_stay_strict(self):
        raw=bytearray(wire());raw[:8]=b'MNMPRO03';struct.pack_into('<I',raw,8,3)
        with self.assertRaises(ValueError):m.decode(raw)


if __name__=='__main__':unittest.main()
