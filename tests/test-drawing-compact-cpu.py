#!/usr/bin/env python3
"""Compact CPU admission cannot truncate typed graphics capture or fake data."""
import copy
import importlib.util
from pathlib import Path
import struct
import sys
import unittest
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools'))
from drawing_api_cohorts import cpu_journal_words
spec=importlib.util.spec_from_file_location('compact_builder',ROOT/'tools/build-drawing-api-census.py')
builder=importlib.util.module_from_spec(spec);spec.loader.exec_module(builder)


class CompactCpuTest(unittest.TestCase):
    def hook(self, slot=0x6903dc):
        return {'site':0x4cb64d,'target_mode':{'kind':'absolute_memory','address':slot},
                'call':{'bytes':(b'\xff\x15'+struct.pack('<I',slot)).hex()}}

    def manifest(self):
        return {'journal_version':1,'compact_cpu_journal':True,
                'call_boundary_role':'cpu_backend_dispatch','cohort':'core_api',
                'diagnostic_stack_words':8,'formal_arguments_recorded':False,
                'buffer_snapshots_recorded':False}

    def test_only_four_raster_globals_and_exact_crt_register_call_admitted(self):
        for slot in [0x689b78,0x689b7c,0x6c4e3c,0x6903dc]:
            builder.compact_cpu_hooks([self.hook(slot)],'core_api')
        builder.compact_cpu_hooks([{'site':0x59d98a,'target_mode':{'kind':'register','register':'eax'},'call':{'bytes':'ffd0'}}],'core_api')

    def test_non_cpu_slot_wrong_bytes_or_call_mode_refuse(self):
        for changes in [{'target_mode':{'kind':'absolute_memory','address':0x6903e0}}, {'call':{'bytes':'ff15e0036900'}}, {'target_mode':{'kind':'register_memory','register':'ecx','displacement':128}}]:
            h = self.hook();h.update(changes)
            with self.assertRaises(ValueError):builder.compact_cpu_hooks([h],'core_api')

    def test_arbitrary_register_callback_is_not_cpu_namespace(self):
        h = {'site':0x59d98b,'target_mode':{'kind':'register','register':'eax'},'call':{'bytes':'ffd0'}}
        with self.assertRaises(ValueError):builder.compact_cpu_hooks([h],'core_api')

    def test_typed_or_imported_metadata_cannot_be_truncated(self):
        for changes in [{'exact_import':{'dll':'gdi32.dll','name':'StretchDIBits'}}, {'details':{'name':'fake','formal_words':13,'origin':'synthetic_fixture','snapshots':[]}}]:
            h = self.hook();h.update(changes)
            with self.assertRaises(ValueError):builder.compact_cpu_hooks([h],'core_api')

    def test_compact_requires_core_and_nonempty_hooks(self):
        with self.assertRaises(ValueError):builder.compact_cpu_hooks([self.hook()],'api_only')
        with self.assertRaises(ValueError):builder.compact_cpu_hooks([],'core_api')

    def test_compact_explicit_eight_word_no_formal_no_buffer_wire(self):
        self.assertEqual(cpu_journal_words(self.manifest()),32)

    def test_compact_missing_or_false_scope_is_not_inferred(self):
        for key in ['compact_cpu_journal','diagnostic_stack_words','formal_arguments_recorded','buffer_snapshots_recorded']:
            m = self.manifest();m.pop(key)
            with self.assertRaises(ValueError):cpu_journal_words(m)

    def test_bool_version_or_word_count_and_typed_capture_flags_refuse(self):
        for key,value in [('journal_version',True),('diagnostic_stack_words',True),('diagnostic_stack_words',16),('formal_arguments_recorded',True),('buffer_snapshots_recorded',True)]:
            m = self.manifest();m[key]=value
            with self.assertRaises(ValueError):cpu_journal_words(m)

    def test_compact_role_or_api_only_mismatch_refuses(self):
        for key,value in [('cohort','api_only'),('call_boundary_role','graphics_api')]:
            m = self.manifest();m[key]=value
            with self.assertRaises(ValueError):cpu_journal_words(m)

    def test_detailed_cpu_profile_remains_explicit_version_two(self):
        m = {'journal_version':2,'call_boundary_role':'cpu_backend_dispatch','cohort':'core_api'}
        self.assertEqual(cpu_journal_words(m),200)
        m['compact_cpu_journal']=True
        with self.assertRaises(ValueError):cpu_journal_words(m)


if __name__ == '__main__':unittest.main()
