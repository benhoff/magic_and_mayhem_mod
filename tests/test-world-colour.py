#!/usr/bin/env python3
import importlib.util
from pathlib import Path
import struct
import tempfile
import unittest
ROOT=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('colour',ROOT/'tools/inspect-world-colour.py');module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
class ColourTest(unittest.TestCase):
    def stream(self):
        words=[1,64,160,1,8,4,8,0x45ff,3,0,0x800000,0x597086,0,0]
        records=[(0xfffffffe,0,0,2166136261,0,0,0,0),(2,0x5003c9,2,123,1,1,3,2),(0xffffffff,0,2,123,1,1,3,2)]
        return bytearray(b'MNMCOL1\0'+struct.pack('<14I',*words)+b''.join(struct.pack('<8I',*row) for row in records))
    def inspect(self,raw):
        with tempfile.TemporaryDirectory() as directory:
            path=Path(directory)/'probe.bin';path.write_bytes(raw);return module.inspect(path,0x45ff)
    def test_change_has_explicit_attribution_boundary(self):
        r=self.inspect(self.stream());self.assertEqual(len(r['changed_intervals']),1);self.assertFalse(r['writer_identified']);self.assertFalse(r['live_replacement'])
    def test_reject_truncation_and_unknown_or_failed_header(self):
        for offset,value in [(8,2),(12,32),(16,159),(24,0),(28,2049),(32,7),(36,65536),(40,0),(44,1),(60,1),(64,1),(96,14),(128,2),(100,1),(104,33),(116,4),(112,7)]:
            raw=self.stream();struct.pack_into('<I',raw,offset,value)
            with self.subTest(offset=offset),self.assertRaises(ValueError):self.inspect(raw)
        with self.assertRaises(ValueError):self.inspect(self.stream()[:-1])
if __name__=='__main__':unittest.main()
