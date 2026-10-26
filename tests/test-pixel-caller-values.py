#!/usr/bin/env python3
"""Exercise real x86 decoding, candidate discovery and conservative refusals."""
import importlib.util
from pathlib import Path
import struct
import sys
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT/'tools'))
from original_x86 import Decoder
from pixel_caller_values import inspect_values

spec = importlib.util.spec_from_file_location('five_callers', ROOT/'tools/inspect-five-pixel-callers.py')
scanner = importlib.util.module_from_spec(spec)
spec.loader.exec_module(scanner)
BASE, TARGET = 0x401000, 0x54b5f0
WINDOWS = {'test': (TARGET, TARGET+32)}


class Values(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.decoder = Decoder()

    @classmethod
    def tearDownClass(cls):
        cls.decoder.close()

    def rows(self, code):
        rows, at = [], 0
        while at < len(code):
            row = scanner.normalized(self.decoder.decode(code[at:at+15], BASE+at))
            rows.append(row)
            at += row['size']
        return rows

    def scan(self, code, memory=None, roots=()):
        words = memory or {}
        return inspect_values(self.rows(code), WINDOWS, words.__getitem__, roots)

    def transfers(self, result):
        return [c for c in result['candidates'] if c['role']=='indirect_control_transfer']

    def test_split_rva_plus_image_base_indirect_call(self):
        code = b'\xb8'+struct.pack('<I', TARGET-0x400000)+b'\x05\x00\x00\x40\x00\xff\xd0'
        result = self.scan(code)
        self.assertEqual(self.transfers(result)[0]['target'], TARGET)
        self.assertEqual(len(self.transfers(result)[0]['trace']), 2)
        self.assertFalse(result['global_unreachable_proved'])
        self.assertEqual(result['confirmed_natural_callers'], [])

    def test_subtraction_and_wrapping(self):
        code = b'\xb8'+struct.pack('<I', (TARGET+0xff000000)&0xffffffff)+b'\x2d\x00\x00\x00\xff\xff\xe0'
        self.assertEqual(self.transfers(self.scan(code))[0]['target'], TARGET)

    def test_xor_encoded_pointer(self):
        code = b'\xb8'+struct.pack('<I', TARGET^0xa5a5a5a5)+b'\x35\xa5\xa5\xa5\xa5\xff\xd0'
        self.assertEqual(self.transfers(self.scan(code))[0]['target'], TARGET)

    def test_shift_or_construction(self):
        code = b'\xb8\x54\x00\x00\x00\xc1\xe0\x10\x0d\xf0\xb5\x00\x00\xff\xd0'
        self.assertEqual(self.transfers(self.scan(code))[0]['target'], TARGET)

    def test_register_copy_and_scaled_lea(self):
        code = b'\xb8\x00\x00\x50\x00\xbb\x7c\x2d\x01\x00\x8d\x0c\x98\x8b\xf1\xff\xd6'
        self.assertEqual(self.transfers(self.scan(code))[0]['target'], TARGET)

    def test_initial_image_memory_read_is_explicitly_not_runtime_proof(self):
        result = self.scan(b'\xa1\x00\x00\x60\x00\xff\xd0', {0x600000: TARGET})
        candidate = self.transfers(result)[0]
        self.assertEqual(candidate['initial_image_reads'], [0x600000])
        self.assertFalse(candidate['confirmed_original_caller'])
        self.assertFalse(result['indirect_transfers'][0]['runtime_target_recovered'])

    def test_indexed_indirect_memory_call(self):
        code = b'\xb8\x00\x00\x60\x00\xbb\x03\x00\x00\x00\xff\x14\x98'
        result = self.scan(code, {0x60000c: TARGET})
        self.assertEqual(self.transfers(result)[0]['target'], TARGET)

    def test_unresolved_index_stays_unresolved(self):
        result = self.scan(b'\xff\x14\x85\x00\x00\x60\x00', {0x600000: TARGET})
        self.assertIsNone(result['indirect_transfers'][0]['candidate_target'])
        self.assertEqual(result['candidates'], [])

    def test_missing_memory_is_not_zero(self):
        result = self.scan(b'\xa1\x00\x00\x60\x00\xff\xd0')
        self.assertIsNone(result['indirect_transfers'][0]['candidate_target'])

    def test_call_clears_registers_and_stack(self):
        code = b'\xb8'+struct.pack('<I', TARGET)+b'\x50\xe8\x01\x00\x00\x00\xff\xd0'
        self.assertEqual(self.transfers(self.scan(code)), [])

    def test_conditional_branch_and_join_do_not_propagate(self):
        code = b'\xb8'+struct.pack('<I', TARGET)+b'\x74\x00\xff\xd0'
        self.assertEqual(self.transfers(self.scan(code)), [])

    def test_known_incoming_root_clears_constants(self):
        code = b'\xb8'+struct.pack('<I', TARGET)+b'\xff\xd0'
        self.assertEqual(self.transfers(self.scan(code, roots=[BASE+5])), [])

    def test_instruction_gap_clears_constants(self):
        rows = self.rows(b'\xb8'+struct.pack('<I', TARGET)+b'\x90\xff\xd0')
        result = inspect_values([rows[0], rows[2]], WINDOWS, {}.__getitem__)
        self.assertEqual(self.transfers(result), [])

    def test_partial_register_mutation_kills_whole_register(self):
        code = b'\xb8'+struct.pack('<I', TARGET)+b'\xb4\x12\xff\xd0'
        self.assertEqual(self.transfers(self.scan(code)), [])

    def test_byte_memory_load_is_not_promoted_to_dword(self):
        code = b'\x8a\x05\x00\x00\x60\x00\xff\xd0'
        self.assertEqual(self.transfers(self.scan(code, {0x600000: TARGET})), [])

    def test_unsupported_instruction_discards_all_state(self):
        code = b'\xb8'+struct.pack('<I', TARGET)+b'\x0f\xa2\xff\xd0'
        result = self.scan(code)
        self.assertEqual(self.transfers(result), [])
        self.assertEqual(result['unsupported_instructions']['cpuid'], 1)

    def test_memory_store_is_an_alias_barrier(self):
        code = b'\xb8'+struct.pack('<I', TARGET)+b'\xc7\x03\x01\x00\x00\x00\xff\xd0'
        self.assertEqual(self.transfers(self.scan(code)), [])

    def test_store_of_computed_address_is_retained_as_candidate(self):
        code = b'\xb8'+struct.pack('<I', TARGET-4)+b'\x83\xc0\x04\xa3\x00\x00\x60\x00'
        result = self.scan(code)
        self.assertEqual([c['target'] for c in result['candidates'] if c['role']=='store'], [TARGET])

    def test_push_return_candidate_and_stack_rewrite_refusal(self):
        code = b'\x68'+struct.pack('<I', TARGET)+b'\xc3'
        self.assertEqual(self.scan(code)['candidates'][0]['role'], 'pushed_return_candidate')
        code = b'\x68'+struct.pack('<I', TARGET)+b'\x83\xc4\x04\xc3'
        self.assertEqual(self.scan(code)['candidates'], [])

    def test_target_inside_helper_is_not_an_external_caller(self):
        rows = self.rows(b'\xb8'+struct.pack('<I', TARGET)+b'\xff\xd0')
        for row in rows:
            row['address'] += TARGET-BASE
        result = inspect_values(rows, WINDOWS, {}.__getitem__)
        self.assertEqual(result['candidates'], [])

    def test_exclusive_end_is_not_a_target(self):
        code = b'\xb8'+struct.pack('<I', TARGET+32)+b'\xff\xd0'
        self.assertEqual(self.scan(code)['candidates'], [])

    def test_unknown_register_write_clears_previous_constant(self):
        code = b'\xb8'+struct.pack('<I', TARGET)+b'\x8b\xc3\xff\xd0'
        self.assertEqual(self.transfers(self.scan(code)), [])

    def test_pop_transfers_pushed_value(self):
        code = b'\x68'+struct.pack('<I', TARGET)+b'\x58\xff\xd0'
        self.assertEqual(self.transfers(self.scan(code))[0]['target'], TARGET)

    def test_signed_arithmetic_shift_and_masked_count(self):
        code = b'\xb8\xe0\x6b\xa9\x00\xc1\xf8\x21\xff\xd0'
        self.assertEqual(self.transfers(self.scan(code))[0]['target'], TARGET)

    def test_no_eval_or_segment_base_assumption(self):
        rows = [dict(address=BASE, bytes='90', mnemonic='mov', operand='eax, DWORD PTR fs:[0x600000]'),
                dict(address=BASE+1, bytes='ffd0', mnemonic='call', operand='eax')]
        self.assertEqual(self.transfers(inspect_values(rows, WINDOWS, {0x600000: TARGET}.__getitem__)), [])


class Graph(unittest.TestCase):
    # Reuse the actual decoder, not synthetic predecoded instruction claims.
    @classmethod
    def setUpClass(cls):
        cls.decoder = Decoder()

    @classmethod
    def tearDownClass(cls):
        cls.decoder.close()

    def graph(self, code, roots=(BASE,), limit=750000):
        class Image:
            def text(self, address):
                return BASE <= address < BASE+len(code)

            def section(self, address):
                return dict(address=BASE, stored=len(code))

            def read(self, address, size):
                return code[address-BASE:address-BASE+size]
        return scanner.decode_graph(Image(), self.decoder, set(roots), limit)

    def test_direct_jump_skips_unrooted_bytes(self):
        rows, graph = self.graph(bytes.fromhex('eb029090c3'))
        self.assertEqual([r['address'] for r in rows], [BASE, BASE+4])
        self.assertFalse(graph['roots_imply_natural_reachability'])

    def test_retained_extra_seed_does_not_claim_reachability(self):
        rows, graph = self.graph(bytes.fromhex('eb029090c3'), (BASE, BASE+2))
        self.assertEqual(len(rows), 4)
        self.assertFalse(graph['roots_imply_natural_reachability'])

    def test_conditional_traverses_both_sides(self):
        rows, _ = self.graph(bytes.fromhex('740190c3'))
        self.assertEqual(len(rows), 3)

    def test_self_loop_terminates(self):
        rows, _ = self.graph(bytes.fromhex('ebfe'))
        self.assertEqual(len(rows), 1)

    def test_indirect_jump_does_not_invent_fallthrough(self):
        rows, _ = self.graph(bytes.fromhex('ffe090c3'))
        self.assertEqual(len(rows), 1)

    def test_direct_call_retains_continuation_and_target(self):
        rows, _ = self.graph(bytes.fromhex('e801000000c3c3'))
        self.assertEqual([r['address'] for r in rows], [BASE, BASE+5, BASE+6])

    def test_decode_failure_retained(self):
        _, graph = self.graph(b'\x0f')
        self.assertEqual(graph['decode_failures'][0]['address'], BASE)

    def test_overlapping_seed_retained(self):
        _, graph = self.graph(bytes.fromhex('b890909090c3'), (BASE, BASE+1))
        self.assertIn([BASE, BASE+1], graph['overlapping_instruction_starts'])

    def test_instruction_budget_refuses_incomplete_success(self):
        with self.assertRaisesRegex(ValueError, 'budget exhausted'):
            self.graph(bytes.fromhex('909090c3'), limit=2)


if __name__ == '__main__':
    unittest.main()
