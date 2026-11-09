#!/usr/bin/env python3
"""Adverse input/source/paint causality and complete-domain guards."""
import copy
import struct
from pathlib import Path
import sys
import unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from viewport_cursor import cursor_ready, edge_points, physical_point, complete_domains, hidden_copy_witness


class ProofGuards(unittest.TestCase):
    def setUp(self):
        self.baseline = dict(frame=dict(generation=10,nonce='abc'),tick=100,
                             cursor=dict(poll=4))
        self.row = dict(kind=1,frame=dict(generation=11,nonce='abc'),tick=140,ordinal=13,
                        cursor=dict(point=[0,0],poll=5,tick=120),cursor_type=6,
                        saved_cursor=[1,-19,0],camera=[0,4,5,0,0,0,0])
        self.paint = dict(world='11',paint='9',native_gpu_frame=True,gpu_read_complete=True,
                          pixels_captured=True,presentation_owner='gui')
    def test_exact_completed_poll(self):
        self.assertEqual(cursor_ready([self.row],self.paint,[0,0],self.baseline)['poll'],5)
    def test_rejects_stale_poll(self):
        self.row['cursor']['poll']=4
        self.assertIsNone(cursor_ready([self.row],self.paint,[0,0],self.baseline))
    def test_rejects_unpresented_output(self):
        self.paint['gpu_read_complete']=False
        self.assertIsNone(cursor_ready([self.row],self.paint,[0,0],self.baseline))
    def test_rejects_foreign_source(self):
        self.row['frame']['nonce']='other'
        self.assertIsNone(cursor_ready([self.row],self.paint,[0,0],self.baseline))
    def test_rejects_late_poll(self):
        self.row['tick']=1000
        self.assertIsNone(cursor_ready([self.row],self.paint,[0,0],self.baseline))
    def test_rejects_poll_before_action(self):
        self.row['cursor']['tick']=99
        self.assertIsNone(cursor_ready([self.row],self.paint,[0,0],self.baseline))
    def test_rejects_different_pointer(self):
        self.assertIsNone(cursor_ready([self.row],self.paint,[1,0],self.baseline))
    def test_rejects_old_paint(self):
        self.paint['world']='10'
        self.assertIsNone(cursor_ready([self.row],self.paint,[0,0],self.baseline))
    def test_complete_edges_both_sizes(self):
        for size in ([640,480],[800,600]):
            points=dict(edge_points(size));w,h=size
            self.assertEqual({tuple(v) for k,v in points.items() if '-' in k},
                             {(0,0),(w-1,0),(0,h-1),(w-1,h-1)})
    def test_physical_letterbox_and_edge_mapping(self):
        paint=dict(width=1000,height=700,frame_width=800,frame_height=600,
                   left=100,top=50,image_width=800,image_height=600)
        pointer=dict(viewport=[5,9,1000,700],screen=[904,658],logical=[799,599])
        self.assertEqual(physical_point(pointer,paint,[800,600]),[799,599])
        pointer['screen'][0]+=1
        with self.assertRaises(ValueError): physical_point(pointer,paint,[800,600])
    def test_component_or_one_profile_cannot_close_rows(self):
        for cases in ([],[dict(software=False)]):
            with self.assertRaises(ValueError): complete_domains(cases)

    def test_hidden_copy_observation_refuses_visible_and_foreign_records(self):
        row=[0x484d4e4d,0x31305043,1,128,0x40209ca7,0x5770fa,
             640,480,640,480,640,512,680,552,640,512,15,1,16,16,3,4]+[0]*10
        raw=struct.pack('<32I',*row)
        self.assertTrue(hidden_copy_witness(raw)['verified'])
        for index,value in [(5,0x5770fb),(6,800),(16,14),(20,0),(22,1)]:
            changed=list(row);changed[index]=value
            with self.assertRaises(ValueError):hidden_copy_witness(struct.pack('<32I',*changed))
        row[10:16]=[600,400,640,440,600,400]
        with self.assertRaises(ValueError):hidden_copy_witness(struct.pack('<32I',*row))
        with self.assertRaises(ValueError):hidden_copy_witness(raw[:-1])


if __name__ == '__main__': unittest.main()
