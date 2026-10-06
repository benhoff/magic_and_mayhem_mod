#!/usr/bin/env python3
"""Provenance, complete-state and poisoned-output refusal checks."""
import gzip
import hashlib
import importlib.util
import json
from pathlib import Path
import tempfile
import unittest
ROOT=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('keys',ROOT/'tools/check-original-surface-keys.py');keys=importlib.util.module_from_spec(spec);spec.loader.exec_module(keys)
class Keys(unittest.TestCase):
    def test_original_pixels(self):
        r=keys.check();self.assertTrue(r['success']);self.assertGreater(r['mutation_second_calls'],0);self.assertGreater(r['pending_native_failure_cases'],0)
    def rejected(self,change,rows_change=None):
        with tempfile.TemporaryDirectory() as tmp:
            root=Path(tmp);m=json.loads(keys.DEFAULT.read_text());packed=(keys.DEFAULT.parent/m['path']).read_bytes()
            if rows_change:
                rows=json.loads(gzip.decompress(packed));rows_change(rows);raw=json.dumps(rows).encode();packed=gzip.compress(raw,mtime=0);m['sha256']=hashlib.sha256(packed).hexdigest();m['raw_sha256']=hashlib.sha256(raw).hexdigest()
            (root/m['path']).write_bytes(packed);change(m);path=root/'corpus.json';path.write_text(json.dumps(m))
            with self.assertRaises(ValueError):keys.check(path)
    def test_hash(self):self.rejected(lambda m:m.update(sha256='0'*64))
    def test_build(self):self.rejected(lambda m:m.update(original_sha256='0'*64))
    def test_path(self):self.rejected(lambda m:m.update(path='../escape.json.gz'))
    def test_poisoned_output(self):self.rejected(lambda m:None,lambda r:r[0]['destination_after'].__setitem__(0,r[0]['destination_after'][0]^1))
    def test_key_state(self):self.rejected(lambda m:None,lambda r:r[0]['key_before'].__setitem__(1,7))
    def test_mutation_continuity(self):self.rejected(lambda m:None,lambda r:next(x for x in r if x['input']['phase'])['destination_before'].__setitem__(0,0))
if __name__=='__main__':unittest.main()
