#!/usr/bin/env python3
"""Exact selected CPU CALL/store identities, no inferred target/owner."""
import copy
from pathlib import Path
import struct
import sys
import unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from drawing_cpu_sites import boundaries,assignments,SLOTS


class CpuSiteTests(unittest.TestCase):
    def setUp(self):self.inventory={'functions':[{'entry':0x400100,'ranges':[[0x400100,0x400200]]}]}
    def call(self,slot=0x6903dc,site=0x400120):return {'address':site,'bytes':(b'\xff\x15'+struct.pack('<I',slot)).hex(),'mnemonic':'call','operand':'DWORD PTR ds:'+hex(slot)}
    def write(self,slot=0x6903dc,target=0x596cb8,site=0x400140):return {'address':site,'bytes':(b'\xc7\x05'+struct.pack('<2I',slot,target)).hex(),'mnemonic':'mov','operand':'DWORD PTR ds:'+hex(slot)+','+hex(target)}

    def test_all_four_slots_have_exact_bytes_and_return_extent(self):
        for slot in SLOTS:
            row=self.call(slot);r=boundaries({row['address']:row},self.inventory)[0];self.assertEqual(r['target_mode'],{'kind':'absolute_memory','address':slot});self.assertEqual(r['return_pc'],row['address']+6);self.assertIsNone(r['exact_import']);self.assertEqual(r['semantic_contract'],'unreviewed');self.assertNotIn('actual_target',r)

    def test_unselected_global_import_or_slot_neighbor_does_not_join_census(self):
        for slot in [0x6903db,0x6903dd,0x5c50a0,0,0xffffffff]:
            row=self.call(slot);self.assertEqual(boundaries({row['address']:row},self.inventory),[])

    def test_prefixed_sib_relative_virtual_and_wrong_call_extents_are_excluded(self):
        for raw in ['66ff15dc036900','ff942480000000','e800000000','ff9080000000','ff15dc03690090','ffd0']:
            row=self.call();row['bytes']=raw;self.assertEqual(boundaries({row['address']:row},self.inventory),[])

    def test_absolute_operand_bytes_cannot_disagree(self):
        row=self.call();row['operand']='DWORD PTR ds:0x689b78'
        with self.assertRaises(ValueError):boundaries({row['address']:row},self.inventory)

    def test_exact_crt_callback_site_is_untyped_register_call(self):
        row={'address':0x59d98a,'bytes':'ffd0','mnemonic':'call','operand':'eax'};r=boundaries({row['address']:row},self.inventory)[0];self.assertEqual(r['return_pc'],0x59d98c);self.assertEqual(r['target_mode'],{'kind':'register','register':'eax'});self.assertEqual(r['discovered_owners'],[]);self.assertNotIn('target',r['target_mode'])
        for site in [0x59d989,0x59d98b]:self.assertEqual(boundaries({site:{**row,'address':site}},self.inventory),[])

    def test_callback_register_operand_and_bytes_must_agree(self):
        row={'address':0x59d98a,'bytes':'ffd0','mnemonic':'call','operand':'ecx'}
        with self.assertRaises(ValueError):boundaries({row['address']:row},self.inventory)
        row.update(bytes='ffd1');self.assertEqual(boundaries({row['address']:row},self.inventory),[])

    def test_immediate_assignments_preserve_exact_target_without_executed_branch(self):
        for slot in SLOTS:
            row=self.write(slot);r=assignments({row['address']:row},self.inventory)[0];self.assertEqual((r['slot'],r['target']),(slot,0x596cb8));self.assertNotIn('executed',r);self.assertIn('alias',r['scope'])

    def test_immediate_store_operand_target_slot_and_mnemonic_mismatch_refuse(self):
        for change in [{'operand':'DWORD PTR ds:0x689b78,0x596cb8'},{'operand':'DWORD PTR ds:0x6903dc,0x597086'},{'mnemonic':'call'}]:
            row=self.write();row.update(change)
            with self.assertRaises(ValueError):assignments({row['address']:row},self.inventory)

    def test_register_absolute_indirect_alias_and_prefixed_stores_are_not_invented(self):
        for raw in ['a3dc036900','8905dc036900','8900','66c705dc036900b86c5900','c705dc036900b86c59']:
            row=self.write();row['bytes']=raw;self.assertEqual(assignments({row['address']:row},self.inventory),[])

    def test_unselected_slot_assignments_are_excluded(self):
        row=self.write(0x6903dd);self.assertEqual(assignments({row['address']:row},self.inventory),[])

    def test_discovered_ranges_are_preserved_without_nearest_owner(self):
        a=self.call(site=0x4001ff);b=self.call(site=0x400200);r=boundaries({a['address']:a,b['address']:b},self.inventory);self.assertEqual(r[0]['discovered_owners'],[0x400100]);self.assertEqual(r[1]['discovered_owners'],[])
        row=self.write(site=0x400200);self.assertEqual(assignments({row['address']:row},self.inventory)[0]['discovered_owners'],[])

    def test_census_order_is_stable_and_call_selection_is_only_instruction_extent(self):
        a=self.call(site=0x400180);b=self.call(site=0x400120);r=boundaries({a['address']:a,b['address']:b},self.inventory);self.assertEqual([x['site'] for x in r],[0x400120,0x400180]);self.assertEqual(r[0]['selections'][0]['ranges'],[[0x400120,0x400126]]);self.assertIsNone(r[0]['selections'][0]['anchor'])


if __name__=='__main__':unittest.main()
