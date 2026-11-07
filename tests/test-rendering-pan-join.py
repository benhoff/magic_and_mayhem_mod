#!/usr/bin/env python3
"""The final pan join must preserve the latest actual accepted inventory."""
from copy import deepcopy
import importlib.util
import os
import json
import struct
from pathlib import Path
import sys
import unittest
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools'))
from coverage_evidence import Reader
spec=importlib.util.spec_from_file_location('pan_join',ROOT/'tools/join-rendering-pan-acceptance.py');m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)

class Join(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        with Reader(ROOT) as r:
            cls.result=r.json(os.environ.get('MNM_PAN_ADMISSION_REPORT',m.PREFIX+'pan-closeout-02.json'))
            cls.parent=r.json(cls.result['parent']['record']);cls.analysis=r.json(cls.result['analysis']['record']);cls.plan=r.json('docs/native-rendering-closure.json')

    def test_actual_latest_parent_and_two_rows(self):
        rows=m.join(self.plan,self.parent,self.analysis)
        self.assertEqual(rows,self.result['requirements'])
        self.assertEqual(self.result['passed_requirements'],self.parent['passed_requirements']+2)
        self.assertEqual(self.result['decisions'],self.parent['decisions'])
        self.assertEqual(self.result['sources'],self.parent['sources'])

    def test_failed_or_incomplete_analysis_refuses(self):
        for key,value in [('success',False),('sources_stable',False),('pan_windows',7),('mismatching_pixels',1),('original_game_runs',1)]:
            with self.subTest(key=key):
                a=deepcopy(self.analysis);a[key]=value
                with self.assertRaises(ValueError):m.join(self.plan,self.parent,a)

    def test_inventory_regression_refuses(self):
        p=deepcopy(self.parent);p['requirements'].pop()
        with self.assertRaises(ValueError):m.join(self.plan,p,self.analysis)

    def test_actual_swap_clock_boundaries_have_no_blank_output(self):
        # The full analysis decoded and hash-checked every compressed paint.
        # Check the actual swap timestamps too, including repeated old paints
        # that straddle a paint-completion interval boundary.
        with Reader(ROOT) as r:
            for case in self.analysis['cases']:
                a=r.json(case['acquisition']['record']);c=r.json(case['comparison']['record'])
                paths=[v for v in a['artifacts'] if '/capture/viewport-' in v['path'] and v['path'].endswith('.bin')]
                self.assertEqual(len(paths),1)
                raw=r.read_bytes(paths[0]['record']);self.assertEqual(raw[:8],b'MNMACE01')
                at=8;paint=None;swaps=[]
                matches={(v['world'],v['paint']):v for v in c['comparison']['comparisons'] if v['world']>0}
                while at<len(raw):
                    size,pixels=struct.unpack_from('<II',raw,at);at+=8
                    row=json.loads(raw[at:at+size]);at+=size+pixels
                    if row['kind']=='paint':paint=row
                    elif row['kind']=='swap' and int(row['paint']):
                        self.assertEqual(paint['paint'],row['paint']);swaps.append((row,paint))
                self.assertEqual(at,len(raw))
                for mode,scale in [('sharp',0),('integer',2)]:
                    begin=next(x for x in a['camera_actions'] if x['action']==mode+'-pan' and x['phase']=='begin')
                    end=next(x for x in a['camera_actions'] if x['action']==mode+'-panned-away-2' and x['phase']=='settled')
                    selected=[(s,p) for s,p in swaps if int(begin['host_us'])<int(s['host_us'])<=int(end['host_us'])]
                    self.assertGreaterEqual(len(selected),3)
                    for swap,p in selected:
                        self.assertGreater(int(p['world']),0)
                        self.assertTrue(all(p[k] for k in ('native_gpu_frame','gpu_read_complete','pixels_captured')))
                        self.assertEqual(p['presentation_owner'],'gui');self.assertEqual(p['scaling'],scale)
                        self.assertEqual([p['frame_width'],p['frame_height']],case['domain'][1])
                        match=matches[(int(p['world']),int(p['paint']))]
                        self.assertEqual(match['expected_sha256'],match['actual_sha256'])
                        self.assertEqual(match['actual_sha256'],p['capture_body_sha256'])
                        self.assertEqual(match['mismatching_pixels'],0)

if __name__=='__main__':unittest.main()
