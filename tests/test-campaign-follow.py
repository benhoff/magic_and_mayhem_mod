#!/usr/bin/env python3
"""Private follow-observation integrity and fresh identity admission controls."""
from pathlib import Path
import struct
import sys
import tempfile
import unittest

ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'tools'))
from campaign_follow import HEADER,read_rows,followed_wizard
from campaign_route import follow_point


class FollowTests(unittest.TestCase):
    def setUp(self):
        self.row=[1,100,7,3,14,0,1,0,0,0,1,176,50,23,57,6]
        self.source=dict(slot=3,type=14,owner=0,active=1,health=50)
        self.wizard=dict(slot=0,type=0,owner=0,active=1,health=176)

    def parse(self,body,complete=False):
        with tempfile.TemporaryDirectory() as directory:
            path=Path(directory)/'follow.bin';path.write_bytes(body)
            return read_rows(path,complete)

    def test_exact_record_and_signed_coordinates(self):
        self.row[13]=0xffffffff
        rows=self.parse(HEADER+struct.pack('<16I',*self.row),True)
        observed=followed_wizard(rows,self.source,self.wizard)
        self.assertEqual(observed['reference'],0)
        self.assertEqual(observed['position'],[-1,57,6])

    def test_partial_live_tail_requires_complete_retained_record(self):
        data=HEADER+struct.pack('<16I',*self.row)+b'partial'
        self.assertEqual(len(self.parse(data)),1)
        with self.assertRaises(ValueError):self.parse(data,True)
        with self.assertRaises(ValueError):self.parse(b'bad')

    def test_sequence_capacity_and_observer_source_refusals(self):
        for at,value in [(0,2),(2,0),(4,0),(5,1),(6,0)]:
            row=list(self.row);row[at]=value
            with self.subTest(at=at),self.assertRaises(ValueError):self.parse(HEADER+struct.pack('<16I',*row))
        data=HEADER+b''.join(struct.pack('<16I',i+1,*self.row[1:]) for i in range(4097))
        with self.assertRaises(ValueError):self.parse(data)

    def test_fresh_reference_requires_both_living_exact_identities(self):
        self.assertIsNone(followed_wizard([self.row],self.source,self.wizard,after=1))
        for at,value in [(3,4),(4,0),(5,1),(6,0),(7,2),(8,14),(9,1),(10,0),(11,0),(11,0xffffffff),(12,0)]:
            row=list(self.row);row[at]=value
            with self.subTest(at=at,value=value):self.assertIsNone(followed_wizard([row],self.source,self.wizard))
        for key in ['source','wizard']:
            for field,value in [('active',0),('health',0),('health',-1)]:
                source=dict(self.source);wizard=dict(self.wizard)
                (source if key=='source' else wizard)[field]=value
                with self.subTest(key=key,field=field,value=value):self.assertIsNone(followed_wizard([self.row],source,wizard))

    def test_newer_source_reference_cannot_inherit_old_success(self):
        later=list(self.row);later[0]=2;later[7]=0xffffffff
        self.assertIsNone(followed_wizard([self.row,later],self.source,self.wizard))
        unrelated=list(later);unrelated[3]=9
        self.assertIsNotNone(followed_wizard([self.row,unrelated],self.source,self.wizard))

    def test_captured_relative_positions_target_wizard_from_centred_follower(self):
        source=dict(self.source,x=23,y=58,z=5);wizard=dict(self.wizard,x=23,y=57,z=6)
        self.assertEqual(follow_point(source,wizard),(432,228))
        self.assertEqual(follow_point(source,wizard,(640,480)),(352,168))
        self.assertEqual(follow_point(source,wizard,body_offset=22),(432,246))
        self.assertEqual(follow_point(source,wizard,body_offset=54),(432,214))
        self.assertIsNone(follow_point(source,dict(wizard,x=10000)))

    def test_body_candidates_refuse_unbounded_offsets_and_top_edge(self):
        source=dict(self.source,x=23,y=58,z=5);wizard=dict(self.wizard,x=23,y=57,z=6)
        for offset in (0,100,-1):
            with self.subTest(offset=offset),self.assertRaises(ValueError):follow_point(source,wizard,body_offset=offset)
        self.assertIsNone(follow_point(source,dict(wizard,z=17),body_offset=54))

    def test_follow_projection_refuses_self_foreign_or_dead_identities(self):
        source=dict(self.source,x=23,y=58,z=5);wizard=dict(self.wizard,x=23,y=57,z=6)
        for changes in [dict(slot=0),dict(type=0),dict(owner=1),dict(active=0),dict(health=0)]:
            with self.subTest(changes=changes),self.assertRaises(ValueError):follow_point(dict(source,**changes),wizard)


if __name__=='__main__':unittest.main()
