#!/usr/bin/env python3
"""Overlap provenance, clipping, color truncation and poisoned-output refusal."""
import gzip
import hashlib
import importlib.util
import json
from pathlib import Path
import tempfile
import unittest
ROOT=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('overlap',ROOT/'tools/check-original-surface-overlap.py');overlap=importlib.util.module_from_spec(spec);spec.loader.exec_module(overlap)
class Overlap(unittest.TestCase):
    def test_original_pixels(self):
        r=overlap.check();self.assertTrue(r['success']);self.assertGreater(r['alias_cases'],0)
    def test_per_piece_snapshot(self):
        rows,_=overlap.load(overlap.DEFAULT);different=0
        for r in rows:
            out=r['destination_before'].copy()
            for (l,t,right,bottom),dx,dy in overlap.plan(r)[1]:
                for y in range(t,bottom):
                    for x in range(l,right):out[(dy+y-t)*8+dx+x-l]=r['source_before'][y*8+x]
            different+=out!=r['destination_after']
        self.assertEqual(different,8)
    def rejected(self,change,rows_change=None):
        with tempfile.TemporaryDirectory() as tmp:
            root=Path(tmp);m=json.loads(overlap.DEFAULT.read_text());packed=(overlap.DEFAULT.parent/m['path']).read_bytes()
            if rows_change:
                rows=json.loads(gzip.decompress(packed));rows_change(rows);raw=json.dumps(rows).encode();packed=gzip.compress(raw,mtime=0);m['sha256']=hashlib.sha256(packed).hexdigest();m['raw_sha256']=hashlib.sha256(raw).hexdigest()
            (root/m['path']).write_bytes(packed);change(m);path=root/'corpus.json';path.write_text(json.dumps(m))
            with self.assertRaises(ValueError):overlap.check(path)
    def test_hash(self):self.rejected(lambda m:m.update(sha256='0'*64))
    def test_build(self):self.rejected(lambda m:m.update(original_sha256='0'*64))
    def test_path(self):self.rejected(lambda m:m.update(path='../escape.json.gz'))
    def test_poisoned_output(self):self.rejected(lambda m:None,lambda r:r[0]['destination_after'].__setitem__(0,r[0]['destination_after'][0]^1))
    def test_identity(self):self.rejected(lambda m:None,lambda r:r[0]['identity'].__setitem__(1,0))
    def test_clip_state(self):self.rejected(lambda m:None,lambda r:r[0]['clip_after'].__setitem__(0,64))
    def test_hresult(self):self.rejected(lambda m:None,lambda r:r[0].update(hresult=0x88760096))
if __name__=='__main__':unittest.main()
