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
    def test_final_publication_must_follow_world_return(self):
        self.row[4]=4;self.save() # The queue has entered but has not returned.
        with self.assertRaises(ValueError):analyze(self.root)
    def test_world_return_cannot_precede_entry(self):
        self.records[3][2],self.records[4][2]=12,11;self.save()
        with self.assertRaises(ValueError):analyze(self.root)
    def test_trace_truncation_refuses(self):
        p=self.root/'drawing-publication-0000.bin';p.write_bytes(p.read_bytes()[:-1])
        with self.assertRaises(ValueError):analyze(self.root)



class AppliedActions(unittest.TestCase):
    save=Publication.save
    def setUp(self):
        Publication.setUp(self)
        self.actions=[[1,1,1,1,3,1,0,0xffffffff,1,0,1,77,0,0,0,0],
                      [2,2,1,1,3,1,0,1,2,0,1,77,0,0,0,0]]
        self.row[22]=77;self.row[23]=2;self.save_actions()
    def save_actions(self):
        Publication.save(self)
        p=self.root/'drawing-publication-0000.bin';data=bytearray(p.read_bytes());data[:8]=b'MNMDP002';struct.pack_into('<I',data,8,2);p.write_bytes(data)
        (self.root/'drawing-actions-0000.bin').write_bytes(b'MNMDAC01'+struct.pack('<6I',1,64,0x40209ca7,8192,0,0)+b''.join(struct.pack('<16I',*r) for r in self.actions))
    def test_actual_entry_return_identity(self):
        a=analyze(self.root)['applied_actions'];self.assertEqual(a['successful_requests'],1);self.assertEqual(a['pending_requests'],[]);self.assertEqual(a['requests'][0]['entry_next_frame'],1)
    def test_changed_return_argument_refuses(self):
        self.actions[1][6]=2;self.save_actions()
        with self.assertRaises(ValueError):analyze(self.root)
    def test_unfinished_dispatch_remains_pending(self):
        self.actions=self.actions[:1];self.row[23]=1;self.save_actions();a=analyze(self.root)['applied_actions'];self.assertEqual(a['pending_requests'],[1]);self.assertEqual(a['successful_requests'],0)
    def test_rejected_dispatch_does_not_become_applied(self):
        self.actions[1][7]=4;self.save_actions();self.assertEqual(analyze(self.root)['applied_actions']['successful_requests'],0)
    def test_forged_phase_boundary_refuses(self):
        self.row[23]=0;self.save_actions()
        with self.assertRaises(ValueError):analyze(self.root)
    def test_wrong_action_thread_refuses(self):
        self.actions[1][11]=78;self.save_actions()
        with self.assertRaises(ValueError):analyze(self.root)
    def test_action_plane_ordinal_cannot_be_guessed(self):
        self.actions[0][10]=2;self.save_actions()
        with self.assertRaises(ValueError):analyze(self.root)
    def test_action_trace_truncation_refuses(self):
        p=self.root/'drawing-actions-0000.bin';p.write_bytes(p.read_bytes()[:-1])
        with self.assertRaises(ValueError):analyze(self.root)
    def test_absent_action_trace_refuses_v2(self):
        (self.root/'drawing-actions-0000.bin').unlink()
        with self.assertRaises(OSError):analyze(self.root)

if __name__=='__main__':unittest.main()
