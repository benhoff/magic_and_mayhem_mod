#!/usr/bin/env python3
"""Mutation guards for exact CALL bytes, extent, ownership and import evidence."""
from pathlib import Path
import sys
import unittest
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
from drawing_api_sites import call_mode, boundary_rows, selected_ranges, instruction_scope, virtual_candidate


def row(raw, operand, address=0x401000):
    return {'address': address, 'bytes': raw, 'mnemonic': 'call', 'operand': operand}


class CallBoundaryTests(unittest.TestCase):
    def test_direct_bytes_and_signed_target_are_checked(self):
        self.assertEqual(call_mode(row('e8fbffffff', '0x401000'))['target'], 0x401000)
        for raw, operand in [('e8fbffffff', '0x401001'), ('e8fbffff', '0x401000')]:
            with self.assertRaises(ValueError): call_mode(row(raw, operand))

    def test_register_modes_and_register_mismatch(self):
        for raw, register in [('ffd0', 'eax'), ('ffd1', 'ecx'), ('ffd7', 'edi')]:
            self.assertEqual(call_mode(row(raw, register)), {'kind': 'register', 'register': register})
            with self.assertRaises(ValueError): call_mode(row(raw, 'edx'))
        with self.assertRaises(ValueError): call_mode(row('ffd090', 'eax'))

    def test_base_memory_and_signed_displacements_are_exact(self):
        cases = [('ff11', 'DWORD PTR [ecx]', 'ecx', 0),
                 ('ff5164', 'DWORD PTR [ecx+0x64]', 'ecx', 100),
                 ('ff52f8', 'DWORD PTR [edx-0x8]', 'edx', -8),
                 ('ff9280000000', 'DWORD PTR [edx+0x80]', 'edx', 128)]
        for raw, operand, register, offset in cases:
            with self.subTest(raw=raw):
                self.assertEqual(call_mode(row(raw, operand)), {'kind': 'register_memory', 'register': register, 'displacement': offset})
                with self.assertRaises(ValueError): call_mode(row(raw, operand.replace(register, 'edi')))
        with self.assertRaises(ValueError): call_mode(row('ff51', 'DWORD PTR [ecx+0x64]'))

    def test_absolute_slot_is_not_the_runtime_target(self):
        r = row('ff1508535c00', 'DWORD PTR ds:0x5c5308')
        self.assertEqual(call_mode(r), {'kind': 'absolute_memory', 'address': 0x5c5308})
        with self.assertRaises(ValueError): call_mode(row(r['bytes'], 'DWORD PTR ds:0x5c5309'))

    def test_indexed_and_prefixed_forms_remain_unresolved(self):
        for raw, operand in [('ff1488', 'DWORD PTR [eax+ecx*4]'), ('2effd0', 'cs call eax')]:
            self.assertEqual(call_mode(row(raw, operand))['kind'], 'unresolved_indirect')
        for raw in ['ff14', 'ff9488', 'ff5488']:
            with self.assertRaises(ValueError): call_mode(row(raw, 'DWORD PTR [eax+ecx*4]'))

    def test_exact_import_and_empty_owner_are_preserved(self):
        code = {0x401000: row('ff1508535c00', 'DWORD PTR ds:0x5c5308')}
        selections = [{'anchor': None, 'ranges': [[0x401000, 0x401006]], 'kind': 'unassigned_selected_window', 'route': 'window'}]
        inventory = {'functions': [{'entry': 0x400000, 'ranges': [[0x400000, 0x400100]]}],
                     'imports': [{'iat_va': 0x5c5308, 'dll': 'jpeg.dll', 'name': 'decoder'}]}
        result = boundary_rows(code, selections, inventory)[0]
        self.assertEqual(result['return_pc'], 0x401006); self.assertEqual(result['discovered_owners'], [])
        self.assertEqual(result['exact_import']['name'], 'decoder')
        inventory['imports'][0]['iat_va'] += 1
        self.assertIsNone(boundary_rows(code, selections, inventory)[0]['exact_import'])

    def test_call_crossing_selected_range_refuses(self):
        code = {0x401000: row('ff1508535c00', 'DWORD PTR ds:0x5c5308')}
        with self.assertRaises(ValueError):
            boundary_rows(code, [{'ranges': [[0x401000, 0x401005]]}], {'functions': [], 'imports': []})

    def test_missing_discovered_range_is_not_replaced_by_nearest(self):
        catalog = {'routes': [{'entry': 0x58b660, 'id': 'lock'}, {'entry': 0x58d3b0, 'id': 'window'}]}
        inventory = {'functions': [{'entry': 0x58b650, 'ranges': [[0x58b650, 0x58b800]]}]}
        with self.assertRaises(ValueError): selected_ranges(catalog, inventory)
        inventory['functions'][0]['entry'] = 0x58b660
        selection = selected_ranges(catalog, inventory)
        self.assertEqual(selection[0]['anchor'], 0x58b660)
        self.assertIsNone(selection[-1]['anchor'])

    def test_virtual_slot_candidate_is_opt_in_and_untyped(self):
        code={0x401000:row('ff9180000000','DWORD PTR [ecx+0x80]')};inventory={'functions':[],'imports':[]}
        self.assertEqual(boundary_rows(code,[],inventory),[])
        candidate=boundary_rows(code,[],inventory,True)[0]
        self.assertEqual(candidate['semantic_contract'],'unreviewed');self.assertEqual(candidate['discovered_owners'],[])
        self.assertEqual(candidate['selections'][0]['ranges'],[[0x401000,0x401006]])
        self.assertEqual(candidate['selections'][0]['kind'],'virtual_slot128_candidate')

    def test_candidate_rejects_other_slots_modes_and_sib(self):
        for raw in ('ff917f000000','ff9100010000','ff5180','ff1580000000','ff948880000000','2eff9180000000'):
            self.assertFalse(virtual_candidate(row(raw,'unused')))

    def test_candidate_does_not_duplicate_selected_site(self):
        code={0x401000:row('ff9180000000','DWORD PTR [ecx+0x80]')};inventory={'functions':[],'imports':[]}
        result=boundary_rows(code,[{'ranges':[[0x401000,0x401006]],'kind':'window'}],inventory,True)
        self.assertEqual(len(result),1);self.assertEqual(result[0]['selections'][0]['kind'],'window')

    def test_exact_graphics_imports_are_scanned_outside_wrappers(self):
        code = {0x401000: row('ff1508535c00', 'DWORD PTR ds:0x5c5308')}
        inventory = {'functions': [], 'imports': [{'iat_va': 0x5c5308, 'dll': 'GDI32.dll', 'name': 'SelectObject'}]}
        result = boundary_rows(code, [], inventory)[0]
        self.assertEqual(result['selections'][0]['kind'], 'exact_graphics_import_call')
        self.assertEqual(result['discovered_owners'], [])
        inventory['imports'][0]['dll'] = 'KERNEL32.dll'
        self.assertEqual(boundary_rows(code, [], inventory), [])

    def test_reachable_selection_keeps_both_arms_without_claiming_function(self):
        code = {1: {'bytes': '7402', 'mnemonic': 'je', 'operand': '0x5'},
                3: {'bytes': '90', 'mnemonic': 'nop', 'operand': ''},
                4: {'bytes': 'c3', 'mnemonic': 'ret', 'operand': ''},
                5: {'bytes': 'c3', 'mnemonic': 'ret', 'operand': ''}}
        graph = instruction_scope(1, code, set())
        self.assertEqual(graph['instructions'], 4)
        self.assertEqual(graph['ranges'], [[1, 6]])
        self.assertEqual(len(graph['exits']), 2)

    def test_registered_tail_and_indirect_exits_stay_explicit(self):
        code = {1: {'bytes': 'eb02', 'mnemonic': 'jmp', 'operand': '0x5'}}
        graph = instruction_scope(1, code, {5})
        self.assertEqual(graph['exits'], [{'kind': 'registered_entry_boundary', 'target': 5}])
        code[1] = {'bytes': 'ffe0', 'mnemonic': 'jmp', 'operand': 'eax'}
        self.assertEqual(instruction_scope(1, code, set())['exits'][0]['kind'], 'unresolved_branch')

    def test_missing_and_budgeted_graphs_refuse(self):
        code = {1: {'bytes': '90', 'mnemonic': 'nop', 'operand': ''}}
        with self.assertRaises(ValueError): instruction_scope(1, code, set())
        code[2] = {'bytes': 'c3', 'mnemonic': 'ret', 'operand': ''}
        with self.assertRaises(ValueError): instruction_scope(1, code, set(), budget=1)


if __name__ == '__main__': unittest.main()
