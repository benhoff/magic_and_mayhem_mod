#!/usr/bin/env python3
"""Guard candidate classifications that could otherwise invent original stores."""
import importlib.util
from pathlib import Path
import sys
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT/'tools'))
spec = importlib.util.spec_from_file_location('execution_research', ROOT/'tools/research-rendering-execution.py')
research = importlib.util.module_from_spec(spec)
spec.loader.exec_module(research)


class ReferenceTests(unittest.TestCase):
    def classify(self, mnemonic, operand):
        return research.reference_access(dict(mnemonic=mnemonic, operand=operand), 0x6e1f68)

    def test_explicit_store_is_candidate_only(self):
        self.assertEqual(self.classify('mov', 'dword ptr [0x6e1f68], eax'), 'write_candidate')

    def test_read_is_not_store(self):
        self.assertEqual(self.classify('mov', 'eax, dword ptr [0x6e1f68]'), 'read_candidate')

    def test_comparison_does_not_write_its_left_memory(self):
        self.assertEqual(self.classify('cmp', 'dword ptr [0x6e1f68], 0'), 'read_candidate')

    def test_address_taking_is_not_memory_access(self):
        self.assertEqual(self.classify('lea', 'eax, [0x6e1f68]'), 'address_candidate')
        self.assertEqual(self.classify('push', '0x6e1f68'), 'literal_candidate')

    def test_indexed_displacement_is_not_an_absolute_global_store(self):
        self.assertEqual(self.classify('mov', 'dword ptr [eax + 0x6e1f68], ecx'), 'indexed_address_candidate')

    def test_literal_prefix_does_not_match_another_address(self):
        self.assertIsNone(self.classify('mov', 'eax, 0x6e1f680'))

    def test_exchange_memory_on_right_can_write(self):
        self.assertEqual(self.classify('xchg', 'eax, dword ptr [0x6e1f68]'), 'write_candidate')

    def test_unknown_access_is_retained(self):
        self.assertEqual(self.classify('fld', 'dword ptr [0x6e1f68]'), 'access_unclassified')


if __name__ == '__main__':
    unittest.main()
