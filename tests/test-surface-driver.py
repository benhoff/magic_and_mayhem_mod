#!/usr/bin/env python3
"""Offline model checked against independent Wine DirectDraw boundary captures."""
import copy
import gzip
import hashlib
import json
from pathlib import Path
import unittest
ROOT=Path(__file__).resolve().parents[1]
INVALID_RECT=0x88760096
NO_LIST=0x887600cd
BUSY=0x887601ae
FAST_CLIP=0x8876023e

def load():
    parent=ROOT/'tests/fixtures/surfaces';meta=json.loads((parent/'driver-clipping-surface2-corpus.json').read_text())
    p=parent/meta['path'];assert p.parent==parent and p.name==meta['path']
    packed=p.read_bytes();assert hashlib.sha256(packed).hexdigest()==meta['sha256']
    raw=gzip.decompress(packed);assert hashlib.sha256(raw).hexdigest()==meta['raw_sha256']
    records=json.loads(raw);assert len(records)==meta['cases'];return records

def prediction(c):
    # Bounded 1:1 geometry and flags only. No GPU/native-renderer code is used.
    source=[0x1000+i*73 for i in range(48)];before=[0xa000+i for i in range(48)]
    s=c['source'];d=c['destination'];w,h=s[2]-s[0],s[3]-s[1]
    if c['fast']:d=[d[0],d[1],d[0]+w,d[1]+h]
    def inside(r):return 0<=r[0]<r[2]<=8 and 0<=r[1]<r[3]<=6
    if c['fast']:
        # Destination admission precedes clipper rejection; source validation follows.
        if w<0 or h<0 or d[0]<0 or d[1]<0 or d[2]>8 or d[3]>6:return INVALID_RECT,before
        if c['clip']:return FAST_CLIP,before
        if not inside(s) or not inside(d):return INVALID_RECT,before
    else:
        if w<=0 or h<=0 or d[0]>=d[2] or d[1]>=d[3]:return INVALID_RECT,before
        if c['clip']==3:return NO_LIST,before
        if not c['clip'] and (not inside(s) or not inside(d)):return INVALID_RECT,before
    if c['held']:return BUSY,before
    if not c['fast'] and c['flags']&0x8000:return 0x80070057,before
    if not c['fast'] and c['flags']&0x80000000:return 0x80004001,before
    regions={0:[[0,0,8,6]],1:[[2,1,6,5]],2:[[0,0,3,2],[5,3,8,6]],4:[]}[c['clip']]
    result=before.copy()
    for r in regions:
        clipped=[max(d[0],r[0]),max(d[1],r[1]),min(d[2],r[2]),min(d[3],r[3])]
        if clipped[0]>=clipped[2] or clipped[1]>=clipped[3]:continue
        mapped=[s[0]+clipped[0]-d[0],s[1]+clipped[1]-d[1],s[0]+clipped[2]-d[0],s[1]+clipped[3]-d[1]]
        if not inside(mapped):return INVALID_RECT,result
        r,m=clipped,mapped
        for y in range(r[1],r[3]):
            for x in range(r[0],r[2]):result[y*8+x]=source[(m[1]+y-r[1])*8+m[0]+x-r[0]]
    return 0,result

def compare(record):
    h,p=prediction(record['input'])
    return record['hresult']==h and record['destination_after']==p

class Driver(unittest.TestCase):
    def test_all_driver_outputs(self):
        for r in load():
            with self.subTest(case=r['input']):self.assertTrue(compare(r))
    def test_capture_integrity_and_input_preservation(self):
        for r in load():
            c=r['input']
            self.assertEqual(r['input_after'],[c[k] for k in ['id','fast','clip','flags','held']]+c['source']+c['destination'])
            self.assertEqual(r['source_before'],[0x1000+i*73 for i in range(48)])
            self.assertEqual(r['source_after'],r['source_before'])
            self.assertEqual(r['destination_before'],[0xa000+i for i in range(48)])
            for d in [r['source_desc'],r['destination_desc']]:
                self.assertEqual([d[2],d[3],d[21],d[22],d[23],d[24]],[6,8,16,0xf800,0x7e0,31])
            if c['clip']==3:self.assertEqual(r['clip_result'],NO_LIST)
            elif c['clip']:
                self.assertEqual(r['clip_result'],0)
                self.assertEqual(r['clip_data'][:4],[32,1,{1:1,2:2,4:0}[c['clip']],{1:16,2:32,4:0}[c['clip']]])
    def test_wait_flag_pairs(self):
        pairs={}
        for r in load():
            c=r['input']
            if c['held'] or c['label'] in ('missing-source-key','invalid-flags'):continue
            key=(c['fast'],c['clip'],c['label']);pairs.setdefault(key,[]).append(r)
        self.assertGreater(len(pairs),100)
        for pair in pairs.values():
            self.assertEqual(len(pair),2)
            self.assertEqual(pair[0]['hresult'],pair[1]['hresult'])
            self.assertEqual(pair[0]['destination_after'],pair[1]['destination_after'])
    def test_detects_wrong_clipping_and_error_policy(self):
        records=load()
        r=copy.deepcopy(next(r for r in records if r['input']['label']=='full' and r['input']['clip']==2 and not r['input']['fast']))
        r['destination_after']=r['source_before'];self.assertFalse(compare(r))
        r=copy.deepcopy(next(r for r in records if r['hresult']==INVALID_RECT))
        r['hresult']=0;self.assertFalse(compare(r))

if __name__=='__main__':unittest.main()
