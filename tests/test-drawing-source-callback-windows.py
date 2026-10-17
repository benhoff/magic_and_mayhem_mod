#!/usr/bin/env python3
import struct
import sys
import unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from drawing_source_callback_windows import decode


def fixture():
    h=[0]*16;h[:2]=struct.unpack('<2I',b'MNMDWIN1');h[2:6]=[1,64,0x40209ca7,512]
    cadence=[];points=[]
    for sequence,prefix,queues in ((1,0,0),(2,2,1),(3,3,1),(4,5,2)):
        c=[0]*24;c[:3]=[sequence,2,7];c[5]=sequence-1;c[6]=sequence;cadence.append(c)
        r=[0]*16;r[:9]=[sequence,1,sequence,sequence,sequence-1,0,7,prefix,queues];points.append(r)
    t=[0]*16;t[:2]=[5,4];t[6:9]=[7,6,2];points.append(t)
    producer=[]
    for op,q in ((11,1),(12,1),(3,0),(11,2),(12,2),(27,0)):
        r=[0]*24;r[2]=op;r[14]=q;producer.append((r,b''))
    return struct.pack('<16I',*h)+b''.join(struct.pack('<16I',*r) for r in points),cadence,producer


class SourceWindows(unittest.TestCase):
    def test_queue_free_window_has_exact_two_prefixes(self):
        raw,rows,records=fixture();r=decode(raw,rows,records,2)
        self.assertEqual([p['queue_prefix'] for p in r['points']],[0,1,1,2]);self.assertFalse(r['original_source_atomicity_proved'])
    def test_pending_frames_remain_explicit(self):
        raw,rows,records=fixture();raw=bytearray(raw);struct.pack_into('<I',raw,64+64+40,1)
        self.assertEqual(decode(raw,rows,records,2)['callbacks_with_pending_frames'],1)
    def test_identity_prefix_queue_failures_and_reserved_refuse(self):
        raw,rows,records=fixture()
        for offset in (0,8,12,16,20,24,64+8,64+12,64+16,64+20,64+24,64+32,64+36,64+44,64+3*64+28,64+4*64+28):
            data=bytearray(raw);struct.pack_into('<I',data,offset,struct.unpack_from('<I',data,offset)[0]+1)
            with self.assertRaises(ValueError):decode(data,rows,records,2)
    def test_missing_terminal_or_boundary_refuses(self):
        raw,rows,records=fixture()
        for data in (raw[:-1],raw[:-64],raw[:128]+raw[192:]):
            with self.assertRaises(ValueError):decode(data,rows,records,2)


if __name__=='__main__':unittest.main()
