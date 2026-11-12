#!/usr/bin/env python3
"""Reject corrupt, foreign, incomplete and fabricated original UI observations."""
from pathlib import Path
import struct
import sys
import unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from rendering_ui_acceptance import decode,decode_keys
from rendering_ui_proof import complete_witness,sealed_worlds,restored_after_motion


def row(kind=1,ordinal=1,call=0,generation=1):
    w=[0]*128;w[:6]=[ordinal,kind,call,100+ordinal,316,generation]
    w[50]=12;w[56:64]=[1,32,0x40209ca7,generation,0,123,0,316]
    return w


def wire(*rows):return b'MNMUIA01'+struct.pack('<2I',1,512)+b''.join(struct.pack('<128I',*r) for r in rows)


class Tests(unittest.TestCase):
    def test_restore_follows_original_motion_not_host_delivery(self):
        rows=[dict(frame=dict(generation=i),tooltip=dict(visible=int(i==1))) for i in range(1,5)]
        motion=dict(kind=3,thread=316,mask=32,mouse=[40,100],world_return=1)
        m,restored,transition=restored_after_motion(rows,[motion],0,4,316,{1,2,3,4})
        self.assertEqual(restored,rows[1:]);self.assertEqual(transition,rows[:1]);self.assertEqual(m,motion)
        for events,shown in [([],{1,2,3,4}),([dict(motion,thread=7)],{1,2,3,4}),([dict(motion,mouse=[730,530])],{1,2,3,4}),([motion],{2,3})]:
            with self.assertRaises(ValueError):restored_after_motion(rows,events,0,4,316,shown)
    def test_terminal_state_cannot_overwrite_last_live_generation(self):
        live=dict(kind=1,world_return=7,frame=dict(generation=7),health=192)
        retired=dict(live,world_return=8,health=0)
        self.assertEqual(sealed_worlds(dict(rows=[live,retired])),{7:live})
        with self.assertRaises(ValueError):sealed_worlds(dict(rows=[live,live]))
    def test_sealed_world(self):
        d=decode(wire(row()));self.assertEqual(d['rows'][0]['frame']['generation'],1);self.assertEqual(d['owner'],316)
    def test_closed_actual_callback(self):
        self.assertEqual(decode(wire(row(2,1,1),row(3,2,1)))['calls'],1)
    def test_truncation(self):
        with self.assertRaises(ValueError):decode(wire(row())[:-1])
    def test_no_terminal_pending_callback(self):
        with self.assertRaises(ValueError):decode(wire(row(2,1,1)))
        self.assertEqual(decode(wire(row(2,1,1)),allow_pending=True)['calls'],1)
    def test_adverse_fields(self):
        for index,value in [(0,2),(1,8),(4,0),(6,1),(18,1),(20,2),(21,2),(36,1),(45,1),(46,1),(47,256),(50,0x8000000c),(56,2),(58,0),(59,0),(61,0),(63,17),(64,65)]:
            with self.subTest(index=index):
                r=row();r[index]=value
                with self.assertRaises(ValueError):decode(wire(r))
    def test_frame_return_must_match(self):
        r=row();r[5]=2;r[16]=2
        with self.assertRaises(ValueError):decode(wire(r))
    def test_terminal_transfer_receives_no_sealed_world_credit(self):
        r=row();r[5]=2;r[16]=6
        d=decode(wire(r))['rows'][0]
        self.assertNotEqual(d['world_returns'],d['frame']['generation'])
        r[5]=3
        with self.assertRaises(ValueError):decode(wire(r))
    def test_stopped_campaign_transfers_are_retained_without_output_credit(self):
        rs=[row(1,i) for i in (1,2,3)]
        for i,r in enumerate(rs):r[5]=i+1;r[7]=1;r[16]=(2,6,4)[i];r[15]=1;r[50]|=32
        def make(rows):return b'MNMUIA01'+struct.pack('<2I',3,512)+b''.join(struct.pack('<128I',*r) for r in rows)
        d=complete_witness(make(rs));self.assertEqual(len(d['rows']),1)
        self.assertEqual([r['top'] for r in d['terminal_transfers']],[6,4]);self.assertTrue(all(not r['output_credit'] for r in d['terminal_transfers']))
        for index,value in [(0,2),(1,4),(2,1),(4,9),(5,4),(7,2),(10,1),(12,2),(15,0),(16,2),(50,0x8000002c),(59,2),(61,0),(63,7)]:
            with self.subTest(index=index):
                bad=[list(r) for r in rs];bad[-1][index]=value
                with self.assertRaises(ValueError):complete_witness(make(bad))
        with self.assertRaises(ValueError):complete_witness(make(rs[:-1]))
    def test_foreign_owner(self):
        r=row(1,2);r[4]=r[63]=317
        with self.assertRaises(ValueError):decode(wire(row(),r))
    def test_generation_regression(self):
        with self.assertRaises(ValueError):decode(wire(row(generation=2),row(1,2,generation=1)))
    def test_clock_regression(self):
        r=row(1,2);r[3]=99
        with self.assertRaises(ValueError):decode(wire(row(),r))
    def test_return_without_entry(self):
        with self.assertRaises(ValueError):decode(wire(row(3,1,1)))
    def test_wrong_callback_family(self):
        with self.assertRaises(ValueError):decode(wire(row(2,1,1),row(6,2,1)))
    def test_repeated_callback(self):
        with self.assertRaises(ValueError):decode(wire(row(2,1,1),row(2,2,1)))
    def test_three_distinct_original_fields(self):
        r=row();r[7]=42;r[10]=1;r[18]=2;r[50]|=32
        raw=bytearray(wire(r));struct.pack_into('<I',raw,8,3);d=decode(raw)['rows'][0]
        self.assertEqual((d['mode'],d['input_mode'],d['accepted_sequence']),(1,2,42))
        r[10]=42
        with self.assertRaises(ValueError):decode(bytes(raw[:16])+struct.pack('<128I',*r))
    def test_legacy_values_cannot_supply_pause(self):
        r=row();r[10]=123;r[18]=2;r[6]=120;r[50]|=2;r[43]=30*256
        raw=bytearray(wire(r));struct.pack_into('<I',raw,8,2);d=decode(raw)['rows'][0]
        self.assertIsNone(d['mode']);self.assertEqual(d['accepted_sequence'],123);self.assertEqual(d['wizard']['mana'],30*256)
    def test_original_key_transitions(self):
        h=b'MNMUIK01'+struct.pack('<14I',1,48,2,64,*([0]*10))
        a=[1,100,316,32,0x8001,1,2,0x46bf32,0,0,0,0];b=[2,101,316,32,0,1,4,0x46bf32,0,0,0,0]
        raw=h+struct.pack('<24I',*a,*b);self.assertEqual(len(decode_keys(raw)['rows']),2)
        for index,value in [(0,3),(2,0),(3,39),(4,0),(5,2),(6,3),(7,0),(8,1)]:
            with self.subTest(index=index):
                bad=list(a);bad[index]=value
                with self.assertRaises(ValueError):decode_keys(h+struct.pack('<24I',*bad,*b))
        with self.assertRaises(ValueError):decode_keys(raw[:-1])


if __name__=='__main__':unittest.main()
