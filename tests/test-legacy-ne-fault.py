#!/usr/bin/env python3
"""NE extent and observed selector/PC accounting for the legacy diagnostic."""
import importlib.util
from pathlib import Path
import struct
import sys
import unittest

ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'tools'))
spec=importlib.util.spec_from_file_location('legacy_fault',ROOT/'tools/inspect-legacy-dibeng-fault.py')
fault=importlib.util.module_from_spec(spec);spec.loader.exec_module(fault)


class NeFaultTest(unittest.TestCase):
    def image(self):
        raw=bytearray(1024);raw[:2]=b'MZ';struct.pack_into('<I',raw,60,128);raw[128:130]=b'NE'
        for at,value in [(0x1c,2),(0x22,64),(0x32,4),(0x14,39),(0x16,2)]:struct.pack_into('<H',raw,128+at,value)
        struct.pack_into('<4H',raw,200,32,64,0x1d70,64);raw[512:576]=bytes(range(64));return raw

    def test_ne_segment_extent_and_entry(self):
        info,code=fault.ne_segment(self.image(),2)
        self.assertEqual((info['file_offset'],info['length'],info['entry_segment'],info['entry_offset']),(512,64,2,39))
        self.assertEqual(code,bytes(range(64)))

    def test_refuse_non_file_backed_iterated_or_truncated_segments(self):
        for at,value in [(200,0),(204,8),(202,4096)]:
            raw=self.image();struct.pack_into('<H',raw,at,value)
            with self.subTest(at=at),self.assertRaises(ValueError):fault.ne_segment(raw,2)
        for raw in [b'',self.image()[:400]]:
            with self.assertRaises(ValueError):fault.ne_segment(raw,2)

    def test_refuse_wrong_index_or_alignment(self):
        for index in [0,3]:
            with self.assertRaises(ValueError):fault.ne_segment(self.image(),index)
        raw=self.image();struct.pack_into('<H',raw,128+0x32,17)
        with self.assertRaises(ValueError):fault.ne_segment(raw,2)

    def test_context_needs_same_complete_register_record(self):
        log='00ec:trace:seh:dispatch_exception eip=000000ce esp=0000ffb4\n00ec:trace:seh:dispatch_exception eax=0000000b ebx=000003df\n00ec:trace:seh:dispatch_exception esi=000003af cs=0327 ss=01ef\n'
        self.assertEqual(fault.exception_contexts(log),[{'thread':'00ec','ip':206,'ax':11,'bx':991,'cs':807}])
        self.assertEqual(fault.exception_contexts(log.splitlines()[0]),[])
        self.assertEqual(fault.exception_contexts(log.replace('00ec:trace:seh:dispatch_exception eax=',
                                                             '00f4:trace:seh:dispatch_exception eax=')),[])

    def test_int31_mapping_does_not_claim_terminal_fault(self):
        log='00ec:trace:seh:dispatch_exception eip=000000ce\n00ec:trace:seh:dispatch_exception eax=0000000b ebx=000003df\n00ec:trace:seh:dispatch_exception cs=0327\n'
        contexts=fault.exception_contexts(log)
        result=fault.continuation_diagnostic(log,{},[{'context':contexts[0]}])
        self.assertTrue(result['descriptor_mapping_is_not_terminal_cause_proof'])
        self.assertIsNone(result['later_32bit_exception'])
        self.assertIsNone(result['readonly_observer_match'])

    def test_later_context_correlates_observer_by_pc(self):
        log='00ec:trace:int31:DOSVM_Int31Handler get descriptor (0x03df)\n00ec:trace:seh:dispatch_exception eip=7bcb6d6d\n00ec:trace:seh:dispatch_exception eax=ffffffff ebx=01262f60\n00ec:trace:seh:dispatch_exception cs=0023\n'
        record={'readonly_loader_exception_observations':[[88,0xc0000005,0x7bcb6d6d,1,0x1262f64,0x23],[89,28,0x11f0000,0x1000,2,0x40000]]}
        result=fault.continuation_diagnostic(log,record,[])
        self.assertEqual(result['observed_dpmi_dispatch'],[('00ec','get')])
        self.assertEqual(result['readonly_observer_match']['memory_query'][4],2)
        record['readonly_loader_exception_observations'][0][2]+=1
        self.assertIsNone(fault.continuation_diagnostic(log,record,[])['readonly_observer_match'])


if __name__=='__main__':unittest.main()
