#!/usr/bin/env python3
"""Refusal and scope guards for independently observed primary publication."""
import copy
from pathlib import Path
import struct
import sys
import tempfile
import unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from drawing_publication import analyze


class Publication(unittest.TestCase):
    def setUp(self):
        self.tmp=tempfile.TemporaryDirectory();self.addCleanup(self.tmp.cleanup);self.root=Path(self.tmp.name)
        self.records=[]
        for kind in (1,5,10,11,12,6):
            r=[0]*24;r[0]=96;r[1]=len(self.records)+1;r[2]=kind;r[3]=1;r[5]=r[6]=r[7]=1
            if kind==10:r[14]=1
            if kind in (11,12):r[14]=1
            self.records.append(r)
        self.row=[1,1,0,42,5,1,1,1,1,0x1000000,0,0,1,1,16,0xf800,0x7e0,31,2,1,0,1,0,0]
        self.save()
    def save(self):
        header=[0]*16;header[:7]=[int.from_bytes(b'MNMP','little'),int.from_bytes(b'RO01','little'),1,64,0x40209ca7,65536,1]
        data=struct.pack('<16I',*header)+b''.join(struct.pack('<24I',*r) for r in self.records)
        (self.root/'canvas-producers.bin').write_bytes(data)
        (self.root/'canvas-producers.done').write_bytes(b'MNMPDONE'+struct.pack('<6I',1,1,len(self.records),sum(r[2]==13 for r in self.records),1,len(data)))
        (self.root/'producer-oracle-0001.565').write_bytes(b'\x34\x12')
        (self.root/'publication-oracle-0001.565').write_bytes(b'\x34\x12')
        (self.root/'drawing-publication-0000.bin').write_bytes(b'MNMDP001'+struct.pack('<6I',1,96,0x40209ca7,65536,1024,0)+struct.pack('<24I',*self.row))
    def test_scope_does_not_close_journeys(self):
        r=analyze(self.root);self.assertFalse(r['all_primary_routes_observed']);self.assertFalse(r['actions_bound']);self.assertEqual(r['readback_drops'],0);self.assertEqual(r['calls'][0]['last_error'],42)
    def test_changed_guarded_flags_refuse(self):
        self.row[9]=0;self.save()
        with self.assertRaises(ValueError):analyze(self.root)
    def test_reordered_producer_boundary_refuses(self):
        self.row[4]=6;self.save()
        with self.assertRaises(ValueError):analyze(self.root)
    def test_short_full_plane_refuses(self):
        (self.root/'publication-oracle-0001.565').write_bytes(b'\x12')
        with self.assertRaises(ValueError):analyze(self.root)
    def test_failed_primary_copy_cannot_terminate(self):
        self.row[2]=0x887601c2;self.row[19]=self.row[21]=0;self.save()
        with self.assertRaises(ValueError):analyze(self.root)
    def test_readback_failure_is_visible(self):
        self.row[10]=0x8876021c;self.row[11]=0xffffffff;self.row[19]=self.row[21]=0;self.row[20]=1;self.save()
        r=analyze(self.root);self.assertEqual(r['frames'],[]);self.assertEqual(r['readback_drops'],1)
    def test_unlock_failure_is_visible(self):
        self.row[11]=0x8876021c;self.row[20]=1;self.save();self.assertEqual(analyze(self.root)['readback_drops'],1)
    def test_loss_counter_cannot_be_erased(self):
        self.row[10]=0x8876021c;self.row[19]=self.row[21]=0;self.save()
        with self.assertRaises(ValueError):analyze(self.root)
    def test_unknown_format_refuses(self):
        self.row[15]=0x7c00;self.save()
        with self.assertRaises(ValueError):analyze(self.root)
    def test_final_offscreen_call_refuses(self):
        self.row[8]=0;self.row[19]=self.row[21]=0;self.save()
        with self.assertRaises(ValueError):analyze(self.root)
    def test_unbalanced_world_refuses(self):
        self.records[4][2]=4;self.save()
        with self.assertRaises(ValueError):analyze(self.root)
    def test_trace_truncation_refuses(self):
        p=self.root/'drawing-publication-0000.bin';p.write_bytes(p.read_bytes()[:-1])
        with self.assertRaises(ValueError):analyze(self.root)

if __name__=='__main__':unittest.main()
