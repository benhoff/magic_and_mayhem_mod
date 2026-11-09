#!/usr/bin/env python3
"""Caller expressions and temporally observed aliases cannot imply disjointness."""
import importlib.util
from pathlib import Path
import sys
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT/'tools'))
from copy_alias_callers import inspect


def module(name, path):
    spec = importlib.util.spec_from_file_location(name, path)
    result = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(result)
    return result


probe = module('alias_inspector', ROOT/'tools/inspect-copy-alias-callers.py')
lifetime = module('existing_lifetime_controls', ROOT/'tests/test-drawing-surface-lifetimes.py')


def code(operations):
    result = {}
    for address, mnemonic, operand in operations:
        raw = '90'
        if mnemonic == 'call':
            raw = 'e8' + (int(operand, 0) - address - 5).to_bytes(4, 'little', signed=True).hex()
        result[address] = dict(address=address, mnemonic=mnemonic, operand=operand, bytes=raw)
    return result


class CallerControls(unittest.TestCase):
    def test_equal_wrapper_expression_is_candidate_only(self):
        result = inspect(code([(0x100, 'push', 'ecx'), (0x101, 'call', '0x58bf40')]))
        self.assertEqual(result['calls'][0]['relationship'], 'same_wrapper_expression_candidate')
        self.assertFalse(result['natural_alias_proved'])
        self.assertFalse(result['global_absence_proved'])

    def test_distinct_wrapper_constants_never_prove_distinct_backing(self):
        result = inspect(code([(0x100, 'mov', 'ecx,0x6a49e0'), (0x101, 'push', '0x6a4fe0'),
                               (0x102, 'call', '0x58bf40')]))
        self.assertEqual(result['calls'][0]['relationship'], 'distinct_constant_wrappers_backing_unknown')
        self.assertFalse(result['calls'][0]['backing_disjoint'])

    def test_aliasing_memory_store_invalidates_cached_stack(self):
        result = inspect(code([(0x100, 'push', 'ecx'), (0x101, 'mov', 'DWORD PTR [eax],edx'),
                               (0x102, 'call', '0x58bf40')]))
        self.assertEqual(result['calls'][0]['relationship'], 'unresolved_wrapper_relationship')

    def test_branch_entry_and_unknown_callee_cannot_preserve_identity(self):
        for operations in [
            [(0x100, 'je', '0x103'), (0x101, 'push', 'ecx'), (0x102, 'nop', ''),
             (0x103, 'call', '0x58bf40')],
            [(0x100, 'push', 'ecx'), (0x101, 'call', '0x200'), (0x106, 'call', '0x58bf40')],
            [(0x100, 'push', 'ecx'), (0x101, 'xchg', 'eax,ecx'), (0x102, 'call', '0x58bf40')],
        ]:
            result = inspect(code(operations))
            self.assertEqual(result['calls'][0]['relationship'], 'unresolved_wrapper_relationship')

    def test_stack_address_normalization_and_push_value_order(self):
        result = inspect(code([(0x100, 'lea', 'ecx,[esp+0x10]'),
                               (0x101, 'push', 'eax'), (0x102, 'lea', 'eax,[esp+0x14]'),
                               (0x103, 'push', 'eax'), (0x104, 'call', '0x58bf40')]))
        self.assertEqual(result['calls'][0]['relationship'], 'same_wrapper_expression_candidate')

    def test_only_completed_prior_queries_join_original_copy_operands(self):
        for method, failures, expected in [(0, (), 1), (0, (1,), 0), (12, (), 0)]:
            intervals, journal = lifetime.lifetime_fixture((method, 5), (11, 12), {1: 12, 2: 11},
                                                          failures=failures, frame_entry=2)
            result = probe.original_aliases(journal, intervals)
            self.assertEqual(result['prior_query_alias'], expected)
            self.assertEqual(result['copies'], 1)
            self.assertFalse(result['output_compared'])
            self.assertFalse(result['global_absence_proved'])

    def test_query_after_copy_entry_cannot_retroactively_establish_alias(self):
        timeline = ((1, 1), (2, 1), (2, 2), (1, 2), (2, 3), (1, 3))
        intervals, journal = lifetime.lifetime_fixture((5, 0), (12, 11), {1: 11, 2: 12}, timeline=timeline)
        result = probe.original_aliases(journal, intervals)
        self.assertEqual(result['prior_query_alias'], 0)
        self.assertEqual(result['successful_queries'], 1)
        self.assertEqual(result['unknown_backing_relationships'], 1)

    def test_corrupt_original_interval_refuses(self):
        intervals, journal = lifetime.lifetime_fixture((5,), (11,), {1: 11})
        with self.assertRaises(ValueError):
            probe.original_aliases(journal, intervals[:-64])


if __name__ == '__main__':
    unittest.main()
