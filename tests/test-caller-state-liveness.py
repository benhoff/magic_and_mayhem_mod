#!/usr/bin/env python3
"""Controls for conservative caller-state requirements, never pixel proof."""
import sys
from pathlib import Path
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
from caller_state_liveness import AccessDecoder, inspect


class Liveness(unittest.TestCase):
    def analyze(self, hex_bytes, registers, limit=1024, **options):
        raw = bytes.fromhex(hex_bytes)
        decoder = AccessDecoder()
        def decode(address):
            if not 0x1000 <= address < 0x1000 + len(raw):
                raise ValueError('Outside supplied code')
            return decoder.decode(raw[address-0x1000:], address)
        try:
            return inspect(0x1000, decode, registers=registers, limit=limit, **options)
        finally:
            decoder.close()

    def test_self_xor_kills_previous_value(self):
        r = self.analyze('31c0c3', dict(eax=0xffffffff, eflags=0x8d5))
        self.assertTrue(r['closed'])
        self.assertEqual(r['required'], {})

    def test_partial_register_store_requires_only_low_word(self):
        r = self.analyze('66890631c0c3', dict(eax=0xffffffff))
        self.assertTrue(r['closed'])
        self.assertEqual(r['required'], dict(eax=0xffff))

    def test_upper_register_survives_partial_write(self):
        r = self.analyze('6631c0c3', dict(eax=0xffffffff))
        self.assertFalse(r['closed'])
        self.assertEqual(r['escapes'][0]['registers'], dict(eax=0xffff0000))

    def test_inc_does_not_kill_carry(self):
        r = self.analyze('400f92c031c0c3', dict(eflags=0x8d5))
        self.assertTrue(r['closed'])
        self.assertEqual(r['required'], dict(eflags=1))

    def test_both_branch_successors_checked(self):
        r = self.analyze('740331c0c366890631c0c3', dict(eax=0xffffffff))
        self.assertTrue(r['closed'])
        self.assertEqual(r['required'], dict(eax=0xffff))

    def test_indirect_transfer_is_pending(self):
        r = self.analyze('ffe1', dict(edx=0xffffffff))
        self.assertFalse(r['closed'])
        self.assertEqual(r['escapes'][0]['reason'], 'indirect-transfer')

    def test_call_does_not_implicitly_discard_volatile_values(self):
        r = self.analyze('e800000000c3', dict(eax=0xffffffff, ecx=0xffffffff))
        self.assertFalse(r['closed'])
        self.assertEqual(r['escapes'][0]['registers'], dict(eax=0xffffffff, ecx=0xffffffff))

    def test_loop_has_fixed_point(self):
        r = self.analyze('ebfe', dict(edx=0xffffffff))
        # A closed loop has no return/unknown transfer, but that alone asserts
        # no termination, output comparison or equivalence.
        self.assertTrue(r['closed'])
        self.assertFalse(r['semantic_equivalence'])

    def test_decode_gap_remains_pending(self):
        r = self.analyze('90', dict(edx=0xffffffff))
        self.assertFalse(r['closed'])

    def test_work_bound_refuses_incomplete_analysis(self):
        with self.assertRaisesRegex(ValueError, 'bound'):
            self.analyze('909031d2', dict(edx=0xffffffff), limit=1)

    def test_sbb_self_uses_carry_without_old_destination(self):
        r = self.analyze('19c0c3', dict(eax=0xffffffff, eflags=1))
        self.assertTrue(r['closed'])
        self.assertEqual(r['required'], dict(eflags=1))

    def test_immediate_mask_does_not_require_discarded_bits(self):
        r = self.analyze('25ffff0000c3', dict(eax=0xffffffff))
        self.assertTrue(r['closed'])
        self.assertEqual(r['required'], dict(eax=0xffff))

    def test_direct_callee_read_and_return_context_are_checked(self):
        r = self.analyze('e801000000c3890631c0c3', dict(eax=0xffffffff), follow_calls=True)
        self.assertTrue(r['closed'])
        self.assertEqual(r['required'], dict(eax=0xffffffff))

    def test_direct_callee_overwrite_is_independently_checked(self):
        r = self.analyze('e801000000c331c0c3', dict(eax=0xffffffff), follow_calls=True)
        self.assertTrue(r['closed'])
        self.assertEqual(r['required'], {})

    def test_high_byte_immediate_mask_uses_parent_bit_positions(self):
        r = self.analyze('80e40f31c0c3', dict(eax=0xffffffff))
        self.assertTrue(r['closed'])
        self.assertEqual(r['required'], dict(eax=0xf00))

    def test_conditional_move_keeps_failed_condition_destination_live(self):
        r = self.analyze('0f44c2c3', dict(eax=0xffffffff))
        self.assertFalse(r['closed'])
        self.assertEqual(r['required'], dict(eax=0xffffffff))

    def test_conditional_exchange_cannot_kill_surviving_destination(self):
        r = self.analyze('0fb1d3c3', dict(ebx=0xffffffff))
        self.assertFalse(r['closed'])
        self.assertEqual(r['required'], dict(ebx=0xffffffff))

    def test_explicit_indirect_targets_all_checked(self):
        r = self.analyze('ffd1c331d2c3891631d2c3', dict(edx=0xffffffff),
                         follow_calls=True, indirect_targets={0x1000: (0x1003, 0x1006)})
        self.assertTrue(r['closed'])
        self.assertEqual(r['required'], dict(edx=0xffffffff))

    def test_explicit_preserved_unused_fact_is_conditional(self):
        r=self.analyze('ff150000200031c9c3',dict(ecx=0xffffffff),follow_calls=True,
            call_preserved_unused={0x1000:dict(ecx=0xffffffff)})
        self.assertTrue(r['closed']);self.assertEqual(r['required'],{})
        self.assertEqual(r['conditional_call_facts'],[dict(address=0x1000,preserved_unused=dict(ecx=0xffffffff))])

    def test_partial_callee_fact_does_not_cover_other_bits(self):
        r=self.analyze('ff150000200031c9c3',dict(ecx=0xffffffff),follow_calls=True,
            call_preserved_unused={0x1000:dict(ecx=0xffff)})
        self.assertFalse(r['closed']);self.assertEqual(r['escapes'][0]['registers'],dict(ecx=0xffff0000))

    def test_callee_fact_does_not_discard_surviving_register(self):
        r=self.analyze('ff1500002000c3',dict(ecx=0xffffffff),follow_calls=True,
            call_preserved_unused={0x1000:dict(ecx=0xffffffff)})
        self.assertFalse(r['closed']);self.assertEqual(r['escapes'][0]['registers'],dict(ecx=0xffffffff))

    def test_callee_fact_does_not_cover_flags(self):
        r=self.analyze('ff150000200031c9c3',dict(eflags=1),follow_calls=True,
            call_preserved_unused={0x1000:dict(ecx=0xffffffff)})
        self.assertFalse(r['closed']);self.assertEqual(r['escapes'][0]['registers'],dict(eflags=1))

    def test_target_operand_read_remains_required(self):
        r=self.analyze('ffd131c9c3',dict(ecx=0xffffffff),follow_calls=True,
            call_preserved_unused={0x1000:dict(ecx=0xffffffff)})
        self.assertEqual(r['required'],dict(ecx=0xffffffff))

    def test_callee_fact_needs_enabled_call_following(self):
        r=self.analyze('ff150000200031c9c3',dict(ecx=0xffffffff),
            call_preserved_unused={0x1000:dict(ecx=0xffffffff)})
        self.assertFalse(r['closed']);self.assertEqual(r['conditional_call_facts'],[])

    def test_invalid_callee_fact_refused(self):
        with self.assertRaises(ValueError):self.analyze('31c9c3',dict(ecx=0xffffffff),call_preserved_unused={0x1000:dict(ecx=0)})

    def test_unused_fact_cannot_change_non_call_flow(self):
        r=self.analyze('ffe1',dict(edx=0xffffffff),follow_calls=True,
            call_preserved_unused={0x1000:dict(edx=0xffffffff)})
        self.assertFalse(r['closed']);self.assertEqual(r['conditional_call_facts'],[])


if __name__ == '__main__':
    unittest.main()
