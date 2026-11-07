#!/usr/bin/env python3
"""Actual marker admission guards; synthetic lineage controls earn no engine credit."""
from copy import deepcopy
import importlib.util
import os
from pathlib import Path
import struct
import sys
import unittest
import numpy as np
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools'))
from coverage_evidence import Reader
spec=importlib.util.spec_from_file_location('markers',ROOT/'tools/close-rendering-markers.py')
m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)

class ActualMarkers(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        with Reader(ROOT) as r:
            cls.report=r.json(os.environ['MNM_MARKERS_REPORT'])
            if not cls.report['success']:raise ValueError('Successful actual retained marker analysis required')
            cls.parent=r.json(cls.report['parent']['record']);cls.plan=r.json('docs/native-rendering-closure.json')
    def test_all_eighty_intervals(self):
        self.assertEqual(sum(len(c['marker_intervals']) for c in self.report['cases']),80)
        for c in self.report['cases']:m.validate_case(c)
    def test_incomplete_marker_evidence_refuses(self):
        mutations=[dict(verified=False),dict(normal_quit=False),dict(original_selection_acknowledged=False),
            dict(selection_state=2),dict(view=4),dict(source_generations=2),dict(whole_viewport_swaps=2),
            dict(compared_pixels=0),dict(mismatching_pixels=1),dict(matched_marker_pixels=0),
            dict(all_three_roles_presented=False),dict(cell_return_checked=False),dict(camera_origin_checked=False),
            dict(creature_dimension_refresh_checked=False),dict(owned_copy_lineage=False),
            dict(original_outer_border_disabled=False),dict(outer_border_render_claimed=True)]
        for c in self.report['cases']:
            for mode in ('sharp','integer'):
                original=next(v for v in c['marker_intervals'] if v['mode']==mode)
                for change in mutations:
                    bad=deepcopy(original);bad.update(change)
                    with self.subTest(domain=c['domain'],mode=mode,change=change),self.assertRaises(ValueError):m.validate_marker_interval(bad)
    def test_missing_or_duplicate_view_scale_state_refuses(self):
        for omit in (True,False):
            c=deepcopy(self.report['cases'][0])
            if omit:c['marker_intervals'].pop()
            else:c['marker_intervals'][-1]=deepcopy(c['marker_intervals'][0])
            with self.subTest(omit=omit),self.assertRaises(ValueError):m.validate_case(c)
        cases=deepcopy(self.report['cases']);cases[-1]=deepcopy(cases[0])
        with self.assertRaises(ValueError):m.close_rows(self.plan,self.parent,cases)
    def test_exact_two_pending_rows_preserve_parent(self):
        rows=m.close_rows(self.plan,self.parent,self.report['cases'])
        changes=[b for a,b in zip(self.parent['requirements'],rows) if a!=b]
        self.assertEqual({(v['configuration'],v['id']) for v in changes},{('player-default',m.LEAF),('player-software',m.LEAF)})
        self.assertEqual(len(changes),2);self.assertEqual(rows,self.report['requirements'])
        self.assertEqual(self.parent['decisions'],self.report['decisions']);self.assertEqual(self.parent['sources'],self.report['sources'])

class Lineage(unittest.TestCase):
    def fixture(self,wrong_upload=False,wrong_pixel=False,no_present=False,keyed=False):
        known={0:0x400,5:31,10:0xfc00};pixels=np.zeros((4,4),dtype='<u2')
        for i,v in known.items():pixels.ravel()[i]=v
        actual=np.zeros((4,4,4),dtype='u1');actual[:,:,3]=255
        for i,v in known.items():
            r,g,b=(v>>11)&31,(v>>5)&63,v&31;actual.reshape(16,4)[i]=[(r<<3)|(r>>2),(g<<2)|(g>>4),(b<<3)|(b>>2),255]
        if wrong_pixel:actual[0,0,0]^=1
        if wrong_upload:pixels.ravel()[0]^=1
        rows=[(1,struct.pack('<7I',10,4,4,16,0xf800,0x7e0,31)+bytes(32)),
              (1,struct.pack('<7I',20,4,4,16,0xf800,0x7e0,31)+bytes(32)),
              (2,struct.pack('<5I',10,0,0,4,4)+pixels.tobytes()),
              (3,struct.pack('<10I',10,20,0,0,4,4,0,0,int(keyed),0)),
              (19,struct.pack('<5I',10,1,0,1,0)),
              (18,b'') if no_present else (6,struct.pack('<I',20)),
              (7,struct.pack('<I',10)),(7,struct.pack('<I',20)),(8,b'')]
        raw=b'MNMCMD05'+struct.pack('<II',5,16)+b''.join(struct.pack('<3I',op,i,len(p))+p for i,(op,p) in enumerate(rows,1))
        paint=dict(commands=6,native_gpu_frame=True,mode=0,surface=20,width=4,height=4,image_width=4,image_height=4,
                   frame_width=4,frame_height=4,left=0,top=0,paint='1',world='1')
        return dict(commands=raw,swaps=[1],paints={1:(paint,actual.tobytes())}),{1:dict(shape=[4,4],known=known,owners={0:'camera',5:'cell',10:'creature'})}
    def test_independent_words_reach_actual_present(self):
        observed,wanted=self.fixture();r=m.presented_marker_words(observed,wanted)[1]
        self.assertEqual(r['matched_pixels'],3);self.assertEqual(r['actual_sha256'],r['expected_sha256'])
    def test_wrong_word_pixel_missing_present_and_keyed_copy_refuse(self):
        for change in ('wrong_upload','wrong_pixel','no_present','keyed'):
            with self.subTest(change=change),self.assertRaises(ValueError):m.presented_marker_words(*self.fixture(**{change:True}))

if __name__=='__main__':unittest.main()
