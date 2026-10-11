#!/usr/bin/env python3
"""Exact drawing CALL identity and ownership refusal tests; no original media."""
import copy
from pathlib import Path
import sys
import unittest
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
from drawing_census import calls_for, direct_call, instructions, validate_callers, splice_conflicts


class CensusTests(unittest.TestCase):
    def setUp(self):
        self.row = {'address': 0x401000, 'bytes': 'e8fb000000', 'mnemonic': 'call', 'operand': '0x401100'}
        self.inventory = {'functions': [{'entry': 0x400ff0, 'ranges': [[0x400ff0, 0x401005]]}]}
        self.record = {'schema': 1, 'source_sha256': 'build', 'image_base': 0x400000, 'input_unchanged': True,
                       'callers': [{'call_va': 0x401000, 'return_pc': 0x401005, 'entry': 0x401100,
                                    'bytes': 'e8fb000000', 'discovered_owners': [0x400ff0]}]}

    def test_relative_bytes_and_operand_must_agree(self):
        self.assertEqual(direct_call(self.row), 0x401100)
        self.row['operand'] = '0x401101'
        with self.assertRaises(ValueError): direct_call(self.row)

    def test_indirect_calls_do_not_acquire_a_target(self):
        self.row.update(bytes='ffd0', operand='eax')
        self.assertIsNone(direct_call(self.row))

    def test_discovered_ranges_and_gaps_preserved(self):
        rows = {0x401000: self.row}
        self.assertEqual(calls_for(rows, {0x401100}, self.inventory)[0]['discovered_owners'], [0x400ff0])
        self.inventory['functions'][0]['ranges'][0][1] = 0x401000
        self.assertEqual(calls_for(rows, {0x401100}, self.inventory)[0]['discovered_owners'], [])

    def test_interior_branch_and_call_refuse_entry_splices(self):
        rows = instructions('  58eddd: eb d2  jmp 0x58edb1\n'
                            '  401000: e8 ac dd 18 00  call 0x58edb1\n')
        self.assertEqual(len(splice_conflicts(0x58edb0, 6, rows)), 2)
        self.assertEqual(splice_conflicts(0x58edb1, 6, rows), [])
        self.assertEqual(splice_conflicts(0x58edb0, 1, rows), [])

    def test_parser_preserves_exact_call(self):
        rows = instructions('  401000:\te8 fb 00 00 00       \tcall   0x401100\n')
        self.assertEqual(rows[0x401000], self.row)

    def test_build_entry_pc_bytes_duplicate_and_input_identity_refuse(self):
        self.assertIn(0x401005, validate_callers(self.record, 'build', {0x401100}))
        mutations = [lambda r: r.update(source_sha256='other'), lambda r: r.update(image_base=0x500000),
                     lambda r: r.update(input_unchanged=False), lambda r: r['callers'][0].update(return_pc=0x401006),
                     lambda r: r['callers'][0].update(bytes='e8fc000000'),
                     lambda r: r['callers'].append(copy.deepcopy(r['callers'][0]))]
        for change in mutations:
            r = copy.deepcopy(self.record); change(r)
            with self.subTest(r=r), self.assertRaises(ValueError): validate_callers(r, 'build', {0x401100})
        with self.assertRaises(ValueError): validate_callers(self.record, 'build', set())


if __name__ == '__main__': unittest.main()
