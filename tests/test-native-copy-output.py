#!/usr/bin/env python3
"""Refuse missing CPU-keyed draws, unbound callers and incomplete all-copy proof."""
import copy
import importlib.util
import json
from pathlib import Path
import struct
import sys
import unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
import native_copy_output as output
spec=importlib.util.spec_from_file_location('copy_pixel_fixture',Path(__file__).with_name('test-native-gpu-copy-diagnostics.py'))
fixture=importlib.util.module_from_spec(spec);spec.loader.exec_module(fixture)


class CallerOutput(unittest.TestCase):
    def setUp(self):
        self.base=fixture.Controls('test_exact_caller_and_external_keyed_comparison');self.base.setUp();self.addCleanup(self.base.doCleanups)
        b=self.base;capture=b.experiment/'lock-capture';self.path=capture/'copy-routes-v2.bin'
        self.header=[*struct.unpack('<2I',b'MNMCRT02'),2,128,176,65536,2,0,2,0,1,1,0,0,1,0x400000,0x260000,0,0,2,1,2,*([0]*10)]
        g=b.call;gpu=[0]*44
        for k,v in {0:1,1:g[1],2:g[3],3:7,4:g[11],5:g[10],6:g[11],7:g[10],8:g[13],9:g[12],10:32768|1|2|4|16|64|16384,11:1,12:1,14:1,15:2,16:4,17:3,18:16,19:4,20:3,21:16,22:2,26:3,27:2,28:1,29:1,30:4,31:3,33:1,38:1,40:7,41:7,42:1,43:2}.items():gpu[k]=v
        cpu=gpu[:];cpu[0]=2;cpu[1]=0x401998;cpu[2]=0x1998;cpu[4:8]=[55,44,55,44];cpu[10]&=~4;cpu[15]=1;cpu[33]=0;cpu[40:44]=[8,8,3,4]
        self.rows=[gpu,cpu]
        b.record['source_world_ancestry']=True;self.cpu=copy.deepcopy(b.record);self.cpu['sequence']=8;self.cpu['source_world_ancestry']=False;self.cpu['words'][:2]=[3,4]
        b.report.update(selection='all-copies',copies_seen=2,copies_checked=2,diagnostic_native_readbacks=6,diagnostic_readback_bytes=288,pixels_compared=24,retained_pixel_cases=2)
        self.write()

    def write(self):
        b=self.base
        self.path.write_bytes(struct.pack('<32I',*self.header)+b''.join(struct.pack('<44I',*r) for r in self.rows)+bytes((65536-len(self.rows))*176))
        (b.output/'copies.jsonl').write_text(json.dumps(b.record)+'\n'+json.dumps(self.cpu)+'\n');b.write_report()

    def validate(self):return output.validate(self.base.experiment,self.base.output)

    def test_actual_cpu_keyed_and_gpu_bind_full_destinations(self):
        r=self.validate();self.assertEqual((r['cpu_keyed_calls'],r['gpu_copy_calls'],r['caller_bound_commands']),(1,1,2));self.assertEqual(r['post_retirement_calls_excluded'],2)

    def test_legacy_or_world_only_cannot_claim_all_output(self):
        for selection in ('world-ancestry','',None):
            with self.subTest(selection=selection):
                self.base.report['selection']=selection;self.write()
                with self.assertRaisesRegex(ValueError,'All-copy selection'):self.validate()

    def test_cpu_missing_duplicate_or_wrong_binding_is_refused(self):
        original=self.rows[1][:]
        for fields in ([0,0,0,0],[7,7,1,2],[8,8,9,4],[8,9,3,4],[9,9,3,4]):
            with self.subTest(binding=fields):
                self.rows[1]=original[:];self.rows[1][40:44]=fields;self.write()
                with self.assertRaises(ValueError):self.validate()

    def test_original_key_geometry_and_extent_cannot_disagree(self):
        original=self.rows[1][:]
        for key in (16,19,22,24,26,28):
            with self.subTest(field=key):
                self.rows[1]=original[:];self.rows[1][key]+=1;self.write()
                with self.assertRaisesRegex(ValueError,'CPU caller'):self.validate()

    def test_unobserved_native_copy_or_missing_retained_keyed_case_refused(self):
        self.base.report['copies_seen']=3;self.write()
        with self.assertRaisesRegex(ValueError,'every native COPY'):self.validate()
        self.base.report['copies_seen']=2;del self.cpu['files'];self.base.report['retained_pixel_cases']=1;self.write()
        with self.assertRaisesRegex(ValueError,'CPU-keyed pixel witness'):self.validate()

    def test_failed_active_call_cannot_supply_output_credit(self):
        self.rows[1][13]=0x88760096;self.write()
        with self.assertRaisesRegex(ValueError,'Active copy failed'):self.validate()

    def test_gpu_binding_remains_exact(self):
        self.rows[0][40:42]=[8,8];self.write()
        with self.assertRaisesRegex(ValueError,'GPU command binding'):self.validate()

    def test_gpu_original_arguments_cannot_disagree_with_self_consistent_pixels(self):
        original=self.rows[0][:]
        for field in (16,19,22,24,26,28):
            with self.subTest(field=field):
                self.rows[0]=original[:];self.rows[0][field]+=1;self.write()
                with self.assertRaisesRegex(ValueError,'GPU caller'):self.validate()

    def test_keyed_source_metadata_must_be_known(self):
        self.rows[1][10]&=~64;self.write()
        with self.assertRaisesRegex(ValueError,'known source key'):self.validate()

    def test_foreign_cpu_callers_do_not_establish_game_reachability(self):
        self.rows[1][1]=0x90000000;self.rows[1][2]=0xffffffff;self.write()
        with self.assertRaisesRegex(ValueError,'original-module callers'):self.validate()

    def test_nonzero_unused_tail_is_not_complete(self):
        with self.path.open('r+b') as stream:stream.seek(-1,2);stream.write(b'x')
        with self.assertRaisesRegex(ValueError,'unused tail'):self.validate()


if __name__=='__main__':unittest.main()
