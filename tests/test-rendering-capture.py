#!/usr/bin/env python3
"""Campaign wire mutations, lazy replay and exact keyframe/body identity."""
import copy
import hashlib
import json
from pathlib import Path
import struct
import sys
import unittest
import zlib
import numpy as np
sys.path.insert(0, str(Path(__file__).resolve().parents[1]/'tools'))
from rendering_capture import capture, Plane, CAMPAIGN_POLICY, CAMPAIGN_RAW, CAMPAIGN_ROWS


def rows(count=130):
    output=[(dict(kind='open',role='viewport',version=2,session='one',policy=CAMPAIGN_POLICY,
                  raw_limit_bytes=CAMPAIGN_RAW,row_limit=CAMPAIGN_ROWS,keyframe_paints=128,
                  stored_limit_bytes=1024**3),b'')]
    previous=None;previous_index=-1;expected=[]
    for i in range(count):
        body=bytes((i+k)%256 for k in range(16));expected.append(body)
        full=i%128==0
        encoded=body if full else np.bitwise_xor(np.frombuffer(body,dtype='u1'),np.frombuffer(previous,dtype='u1')).tobytes()
        index=len(output)
        output.append((dict(kind='paint',width=2,height=2,capture_body_encoding='full-rgba-v1' if full else 'xor-previous-paint-v1',
                            capture_body_base=-1 if full else previous_index,capture_body_sha256=hashlib.sha256(body).hexdigest()),encoded))
        output.append((dict(kind='swap',paint=i+1),b''));previous,previous_index=body,index
    output.append((dict(kind='close',complete=True,error='',raw_bytes=str(16*count),prior_rows=len(output)),b''))
    return output,expected


def wire(values):
    out=bytearray(b'MNMACE01')
    for i,(metadata,body) in enumerate(values):
        metadata=dict(metadata,index=i,host_us=str(100+i))
        record=json.dumps(metadata).encode();packed=struct.pack('>I',len(body))+zlib.compress(body) if body else b''
        out+=struct.pack('<II',len(record),len(packed))+record+packed
    return bytes(out)


class CampaignCapture(unittest.TestCase):
    def test_every_plane_and_backward_keyframe_seek_match_exact_source(self):
        values,expected=rows();decoded=capture(wire(values));planes=[body for meta,body in decoded if meta['kind']=='paint']
        self.assertTrue(all(isinstance(p,Plane) for p in planes))
        self.assertEqual(planes[0].source.last,-1)
        for i in [129,0,128,127,3,*range(130)]:self.assertEqual(planes[i](),expected[i])
        self.assertEqual(len(planes[0].source.body),16)
        self.assertEqual(planes[0].source.keys,[0,128])

    def test_malformed_policy_cannot_expand_budgets_or_change_roles(self):
        values,_=rows(2)
        for key,bad in [('raw_limit_bytes',CAMPAIGN_RAW+1),('row_limit',True),('keyframe_paints',129),
                        ('version',True),('policy','other'),('role','commands'),('stored_limit_bytes',2*1024**3)]:
            mutated=copy.deepcopy(values);mutated[0][0][key]=bad
            with self.subTest(key=key),self.assertRaises(ValueError):capture(wire(mutated))

    def test_delta_requires_exact_previous_nonempty_paint_and_extent(self):
        values,_=rows(3)
        for key,bad in [('capture_body_base',2),('capture_body_base',True),('capture_body_sha256','0'*64),
                        ('capture_body_encoding','unknown'),('width',4),('height',1)]:
            mutated=copy.deepcopy(values);mutated[3][0][key]=bad
            with self.subTest(key=key,bad=bad),self.assertRaises(ValueError):capture(wire(mutated))

    def test_full_keyframe_and_regular_replay_bound_are_required(self):
        values,_=rows()
        for index in (1,257):
            mutated=copy.deepcopy(values);mutated[index][0]['capture_body_encoding']='xor-previous-paint-v1'
            mutated[index][0]['capture_body_base']=index-2
            with self.subTest(index=index),self.assertRaises(ValueError):capture(wire(mutated))

    def test_refusals_and_encoded_payload_outside_viewport_are_rejected(self):
        values,_=rows(2)
        for mutation in (lambda v:v[-1][0].update(complete=False,error='stored budget'),
                         lambda v:v[-1][0].update(raw_bytes='1'),
                         lambda v:v[1][0].update(kind='swap'),
                         lambda v:v[0][0].update(version=1),
                         lambda v:v[1][0].update(capture_body_base=0)):
            mutated=copy.deepcopy(values);mutation(mutated)
            with self.assertRaises(ValueError):capture(wire(mutated))
        with self.assertRaises(ValueError):capture(wire(values)[:-1])


if __name__=='__main__':unittest.main()
