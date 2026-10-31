#!/usr/bin/env python3
"""Reject malformed/truncated original menu logs without inventing ownership."""
import importlib.util
from pathlib import Path
import struct
import sys
import unittest

ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'tools'))
spec=importlib.util.spec_from_file_location('research_join',ROOT/'tools/reconcile-rendering-execution-research.py')
module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
HEADER=b'MNMMENU1'+struct.pack('<2I',1,64)


class MenuEvidenceTests(unittest.TestCase):
    def test_unsupported_header_and_partial_record_refuse(self):
        for data in [b'',HEADER[:-1],HEADER+b'\0',b'MNMMENU1'+struct.pack('<2I',2,64)]:
            with self.assertRaises(ValueError):module.events(data)

    def test_sequence_gap_refuses(self):
        with self.assertRaises(ValueError):module.events(HEADER+struct.pack('<16I',2,*([0]*15)))

    def test_complete_tick_record_does_not_prove_router_ownership(self):
        value=module.events(HEADER+struct.pack('<16I',1,1,*([0]*14)))
        self.assertEqual(value['records'],1)
        self.assertFalse(value['complete_router_or_destructor_proof'])


if __name__=='__main__':unittest.main()
