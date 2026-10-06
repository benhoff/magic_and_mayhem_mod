#!/usr/bin/env python3
"""Fill provenance, clipping, color truncation and poisoned-output refusal."""
import gzip
import hashlib
import importlib.util
import json
from pathlib import Path
import tempfile
import unittest
ROOT=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('fills',ROOT/'tools/check-original-surface-fills.py');fills=importlib.util.module_from_spec(spec);spec.loader.exec_module(fills)
class Fills(unittest.TestCase):
    def test_original_pixels(self):
        r=fills.check();self.assertTrue(r['success']);self.assertGreater(r['high_color_truncation_cases'],0);self.assertGreater(r['pending_native_failure_cases'],0)
    def rejected(self,change,rows_change=None):
        with tempfile.TemporaryDirectory() as tmp:
            root=Path(tmp);m=json.loads(fills.DEFAULT.read_text());packed=(fills.DEFAULT.parent/m['path']).read_bytes()
            if rows_change:
                rows=json.loads(gzip.decompress(packed));rows_change(rows);raw=json.dumps(rows).encode();packed=gzip.compress(raw,mtime=0);m['sha256']=hashlib.sha256(packed).hexdigest();m['raw_sha256']=hashlib.sha256(raw).hexdigest()
            (root/m['path']).write_bytes(packed);change(m);path=root/'corpus.json';path.write_text(json.dumps(m))
            with self.assertRaises(ValueError):fills.check(path)
    def test_hash(self):self.rejected(lambda m:m.update(sha256='0'*64))
    def test_build(self):self.rejected(lambda m:m.update(original_sha256='0'*64))
    def test_path(self):self.rejected(lambda m:m.update(path='../escape.json.gz'))
    def test_poisoned_output(self):self.rejected(lambda m:None,lambda r:r[0]['destination_after'].__setitem__(0,r[0]['destination_after'][0]^1))
    def test_truncation(self):self.rejected(lambda m:None,lambda r:r[0]['trace'].__setitem__(13,0x10000))
    def test_clip_state(self):self.rejected(lambda m:None,lambda r:r[0]['clip_after'].__setitem__(0,64))
    def test_hresult(self):self.rejected(lambda m:None,lambda r:r[0].update(hresult=0x88760096))
if __name__=='__main__':unittest.main()
