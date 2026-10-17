#!/usr/bin/env python3
"""Reject incomplete caller traces and independently corrupt copy witnesses."""
import copy
import hashlib
import importlib.util
import json
import struct
import tempfile
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('copy_diagnostics', ROOT/'tools/native_gpu_copy_diagnostics.py')
diagnostics = importlib.util.module_from_spec(spec); spec.loader.exec_module(diagnostics)


class Controls(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.root = Path(self.temporary.name)
        self.experiment = self.root/'experiment'; (self.experiment/'lock-capture').mkdir(parents=True)
        self.output = self.root/'pixels'; self.output.mkdir()
        self.sensor = self.experiment/'lock-capture/gpu-copy-census-v1.bin'
        self.header = [*struct.unpack('<2I', b'MNMGCN01'),1,128,128,8192,1,0,1,0,0,0,1,0,0,0,0,0,0x400000,0x260000,1,*([0]*11)]
        self.call = [1,0x401234,0x400000,0x1234,1,2,0,7,7,1,11,22,3,4,5,4,1,0,0,4,3,4,3,0,0,3,2,1,1,4,3,1]
        self.write_sensor()
        source = list(range(1,13)); before = list(range(100,112)); expected = before[:]
        for y in range(2):
            for x in range(3):
                pixel = source[y*4+x]
                if pixel != 2: expected[(y+1)*4+x+1] = pixel
        self.record = dict(sequence=7,words=[1,2,0,0,3,2,1,1,1,2,0,0,0],source_width=4,source_height=3,target_width=4,target_height=3,pixels=12,mismatches=0,files={})
        for kind,words in [('source',source),('before',before),('actual',expected),('expected',expected)]:
            self.write_pixels(kind,words)
        self.write_record()
        self.report = dict(success=True,ended=True,truncated=False,error='',copies_checked=1,original_pixels_used_as_native_inputs=False,diagnostic_native_readbacks=3,diagnostic_readback_bytes=144,pixels_compared=12,retained_pixel_cases=1)
        self.write_report()

    def write_sensor(self):
        self.sensor.write_bytes(struct.pack('<32I', *self.header) + struct.pack('<32I', *self.call) + bytes((8192-1)*128))

    def write_record(self):
        (self.output/'copies.jsonl').write_text(json.dumps(self.record)+'\n')

    def write_report(self):
        (self.output/'report.json').write_text(json.dumps(self.report)+'\n')

    def write_pixels(self,kind,words):
        name=kind+'.u32'; data=struct.pack('<'+'I'*len(words),*words)
        (self.output/name).write_bytes(data); self.record['files'][kind]=name
        self.record[kind+'_sha256']=hashlib.sha256(data).hexdigest()

    def test_exact_caller_and_external_keyed_comparison(self):
        r=diagnostics.validate(self.experiment,self.output)
        self.assertEqual((r['matched_calls'],r['keyed_calls'],r['retained_python_pixels']),(1,1,12))
        self.assertEqual(r['callers'],[dict(caller_rva='0x1234',flags=4,calls=1)])

    def test_census_completeness_and_identity(self):
        original=self.header[:]
        for field,value in [(2,2),(6,0),(7,1),(9,1),(12,0),(18,0),(20,0),(21,1)]:
            with self.subTest(field=field):
                self.header=original[:];self.header[field]=value;self.write_sensor()
                with self.assertRaises(ValueError):diagnostics.census(self.sensor)

    def test_changed_witness_hash(self):
        (self.output/'actual.u32').write_bytes(bytes(48))
        with self.assertRaisesRegex(ValueError,'Changed retained'):diagnostics.validate(self.experiment,self.output)

    def test_equal_but_wrong_actual_and_expected(self):
        self.write_pixels('actual',[0]*12);self.write_pixels('expected',[0]*12);self.write_record()
        with self.assertRaisesRegex(ValueError,'Independent retained'):diagnostics.validate(self.experiment,self.output)

    def test_missing_call_sequence(self):
        self.call[7]=self.call[8]=8;self.write_sensor()
        with self.assertRaisesRegex(ValueError,'lack native output'):diagnostics.validate(self.experiment,self.output)

    def test_wrong_resource_identity(self):
        self.call[4]=3;self.write_sensor()
        with self.assertRaisesRegex(ValueError,'Caller/copy command'):diagnostics.validate(self.experiment,self.output)

    def test_incomplete_host_and_duplicate_records(self):
        for field,value in [('success',False),('ended',False),('truncated',True),('copies_checked',2),('diagnostic_native_readbacks',2)]:
            with self.subTest(field=field):
                original=copy.deepcopy(self.report);self.report[field]=value;self.write_report()
                with self.assertRaises(ValueError):diagnostics.validate(self.experiment,self.output)
                self.report=original
        self.write_report();(self.output/'copies.jsonl').write_text((json.dumps(self.record)+'\n')*2)
        with self.assertRaises(ValueError):diagnostics.validate(self.experiment,self.output)

    def test_unsafe_witness_identity(self):
        self.record['files']['source']='../source.u32';self.write_record()
        with self.assertRaisesRegex(ValueError,'Unsafe copy'):diagnostics.validate(self.experiment,self.output)


if __name__=='__main__':unittest.main()
