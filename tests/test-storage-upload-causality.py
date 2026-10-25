#!/usr/bin/env python3
import copy
from hashlib import sha256
from pathlib import Path
import struct
import sys
import tempfile
import unittest

import numpy as np

sys.path.insert(0, str(Path(__file__).resolve().parents[1]/'tools'))
from storage_upload_causality import bind, compare, decode, read_point, rgb565, samples


def fixture():
    w = [0]*46
    w[:19] = [1,192,10,11,1,1,1,1,22,33,4,3,4,5,80,80,44,1,2]
    w[20], w[22] = 100, 200
    w[24:40] = [55,4,5,0,0,4,5,0,0,24,2,0,0,0,22,0]
    w[40:46] = [4,5,1,0,0,2]
    words = np.arange(12,dtype=np.uint16).reshape(3,4)+0x1234
    plane = np.zeros((5,4),dtype='<u4')
    rgb = rgb565(words).astype(np.uint32)
    plane[2:] = rgb[:,:,0]<<16 | rgb[:,:,1]<<8 | rgb[:,:,2]
    data = plane.tobytes()
    row = decode(b'MNMESH01'+struct.pack('<46I',*w))
    row.update(data=data,pixels_sha256=sha256(data).hexdigest())
    return w,row,words


class Causality(unittest.TestCase):
    def test_full_submitted_plane_explains_wrong_output(self):
        _,s,words=fixture()
        raw=np.frombuffer(s['data'],dtype='<u4').reshape(5,4)[:3]
        physical=np.stack([(raw>>16)&255,(raw>>8)&255,raw&255],axis=2)
        result=compare(s,words,words,physical)
        self.assertTrue(result['after_nominal_destination'])
        self.assertEqual(result['submitted_physical_mismatches'],0)
        self.assertEqual(result['geometry_delta_destination_mismatches'],0)
        self.assertGreater(result['destination_physical_mismatches'],0)

    def test_correct_output_is_not_misclassified(self):
        _,s,words=fixture()
        result=compare(s,words,words,rgb565(words))
        self.assertFalse(result['after_nominal_destination'])

    def test_upstream_disagreement_prevents_attribution(self):
        _,s,words=fixture()
        result=compare(s,words,words^0xffff,np.zeros((3,4,3),dtype=np.uint8))
        self.assertFalse(result['after_nominal_destination'])

    def test_changed_sample_refuses(self):
        _,s,words=fixture();s['data']=b'X'+s['data'][1:]
        with self.assertRaisesRegex(ValueError,'changed'):
            compare(s,words,words,rgb565(words))

    def test_malformed_metadata_and_stability_refuse(self):
        w,_,_=fixture()
        for index,value in ((0,2),(1,191),(2,0),(4,17),(5,99),(14,84),(15,79),(17,0),(19,1),(42,0),(45,1)):
            bad=w.copy();bad[index]=value
            with self.subTest(index=index),self.assertRaises(ValueError):
                decode(b'MNMESH01'+struct.pack('<46I',*bad))
        with self.assertRaises(ValueError):decode(b'MNMESH01')

    def test_exact_forwarded_packet_required(self):
        _,s,_=fixture()
        p=dict(drawable=55,total_width=4,total_height=5,source_x=0,source_y=0,source_width=4,
               source_height=5,destination_x=0,destination_y=0,depth=24,format=2,send_event=0,
               left_pad=0,data_bytes=0,shm_segment=22,shm_offset=0,method=1,cookie_sequence=44,
               begin_ns=90,end_ns=210)
        row=dict(pid=10,thread=11,width=4,height=3,xcb_images=[p])
        self.assertIs(bind(s,row),p)
        for field in ('cookie_sequence','shm_segment','source_y'):
            bad=copy.deepcopy(row);bad['xcb_images'][0][field]+=1
            with self.subTest(field=field),self.assertRaises(ValueError):bind(s,bad)
        for variant in ([],[p,p]):
            with self.assertRaises(ValueError):bind(s,dict(row,xcb_images=variant))

    def test_frame_identity_and_interval_refuse(self):
        _,s,_=fixture()
        with self.assertRaisesRegex(ValueError,'identity'):
            bind(s,dict(pid=10,thread=12,width=4,height=3,xcb_images=[]))

    def test_complete_point_does_not_close_a_process_cohort(self):
        w,s,_=fixture()
        with tempfile.TemporaryDirectory() as name:
            p=Path(name)/'shm-source-10-000001.bin'
            p.write_bytes(b'MNMESH01'+struct.pack('<46I',*w))
            p.with_suffix('.bgra').write_bytes(s['data'])
            self.assertEqual(read_point(p)['pixels_sha256'],s['pixels_sha256'])
            with self.assertRaises(FileNotFoundError):samples(Path(name))


if __name__=='__main__':unittest.main()
