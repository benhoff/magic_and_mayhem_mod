#!/usr/bin/env python3
"""Guard the finite owned-provider boundary; never turn zero counts global."""
from pathlib import Path
import sys
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1]/'tools'))
from native_player_overlap import CONTRACT, LAYOUT, contract, relation, summarize, verify_phase


def row(source=0x1000, target=0x2000, result=0, prepared=1):
    result_row = [0]*44
    result_row[4:6] = [target, source]
    result_row[13] = result; result_row[15] = prepared
    result_row[24:28] = [0, 0, 4, 4]
    result_row[28:32] = [1, 1, 5, 5]
    return result_row


class OwnedOverlapBoundary(unittest.TestCase):
    def test_all_twenty_five_interface_alias_pairs(self):
        for a in range(5):
            for b in range(5):
                self.assertEqual(relation(0x1000+12*a, 0x1000+12*b), 'same_owned_state')

    def test_alias_classification_is_symmetric(self):
        self.assertEqual(relation(0x1000, 0x1030), relation(0x1030, 0x1000))

    def test_distinct_allocations_all_interface_pairs(self):
        for a in range(5):
            for b in range(5):
                self.assertEqual(relation(0x1000+12*a, 0x1200+12*b), 'distinct_owned_states')

    def test_minimum_distinct_state_separation(self):
        self.assertEqual(relation(0x1000, 0x1000+236), 'distinct_owned_states')

    def test_misaligned_interfaces_refused(self):
        with self.assertRaises(ValueError): relation(0x1001, 0x2000)

    def test_impossible_alias_offset_refused(self):
        with self.assertRaises(ValueError): relation(0x1000, 0x1004)

    def test_impossible_interallocation_gap_refused(self):
        with self.assertRaises(ValueError): relation(0x1000, 0x1040)

    def test_null_interface_refused(self):
        with self.assertRaises(ValueError): relation(0, 0x2000)

    def test_non_pe32_pointer_refused(self):
        with self.assertRaises(ValueError): relation(2**32+0x1000, 0x2000)

    def test_wrapping_state_extent_refused(self):
        with self.assertRaises(ValueError): relation(0xfffffff0, 0x1000)

    def test_other_provider_layout_refused(self):
        with self.assertRaises(ValueError): relation(0x1000, 0x2000, dict(LAYOUT, interface_bytes=8))

    def test_boolean_address_is_not_an_interface(self):
        with self.assertRaises(ValueError): relation(True, 0x2000)

    def test_equal_coordinates_do_not_alias_distinct_storage(self):
        value = row(); value[28:32] = value[24:28]
        result = summarize([value])
        self.assertTrue(result['bounded_exclusion'])
        self.assertEqual(result['counts']['distinct_owned_state_copies'], 1)
        self.assertFalse(result['global_unreachability_proved'])

    def test_real_interface_alias_cannot_be_excluded(self):
        result = summarize([row(target=0x100c)])
        self.assertFalse(result['bounded_exclusion'])
        self.assertTrue(result['natural_overlap_witness'])
        self.assertFalse(result['independent_alias_output_proved'])

    def test_unknown_alias_geometry_stays_pending(self):
        result = summarize([row(target=0x100c, prepared=0)])
        self.assertFalse(result['bounded_exclusion'])
        self.assertEqual(result['counts']['shared_state_unknown_geometry'], 1)
        self.assertFalse(result['natural_overlap_witness'])

    def test_alias_nonoverlap_does_not_become_overlap(self):
        value = row(target=0x100c); value[28:32] = [4, 0, 8, 4]
        result = summarize([value])
        self.assertFalse(result['bounded_exclusion'])
        self.assertFalse(result['natural_overlap_witness'])

    def test_failed_calls_are_preserved_outside_success_boundary(self):
        failure = row(target=0x100c, result=0x88760096, prepared=0)
        result = summarize([row(), failure])
        self.assertEqual(result['counts']['failed_source_copies'], 1)
        self.assertEqual(result['failures'], [failure])
        self.assertEqual(result['counts']['successful_source_copies'], 1)
        self.assertFalse(result['natural_overlap_witness'])

    def test_nonzero_success_result_requires_review(self):
        with self.assertRaises(ValueError): summarize([row(result=1)])

    def test_fill_is_not_a_source_copy(self):
        result = summarize([row(source=0)])
        self.assertEqual(result['counts']['source_less'], 1)
        self.assertEqual(result['counts']['source_copies'], 0)
        self.assertFalse(result['bounded_exclusion'])

    def test_empty_capture_cannot_close_a_boundary(self):
        self.assertFalse(summarize([])['bounded_exclusion'])

    def test_only_failures_cannot_close_a_boundary(self):
        self.assertFalse(summarize([row(result=0x88760096)])['bounded_exclusion'])

    def test_wrong_wire_row_refused(self):
        with self.assertRaises(ValueError): summarize([row()[:-1]])

    def test_invalid_prepared_alias_geometry_refused(self):
        value = row(target=0x100c); value[26] = 0
        with self.assertRaises(ValueError): summarize([value])

    def test_changed_provider_contract_identity_refused(self):
        class Wrong:
            def digest(self, path):
                assert path == CONTRACT
                return '0'*64
        with self.assertRaises(ValueError): contract(Wrong())

    def actual_phase(self, mutate=None):
        from coverage_evidence import Reader
        with Reader(Path(__file__).resolve().parents[1]) as reader:
            value = reader.json(CONTRACT)
            phase = value['phases'][0]
            acquired = reader.json(phase['acquisition']['record'])
            if mutate:
                mutate(phase)
            return verify_phase(reader, acquired, phase)

    def test_actual_recorded_provider_phase_is_admitted(self):
        self.assertTrue(self.actual_phase()['lifetime_mode2_installed'])

    def test_another_provider_binary_is_refused(self):
        with self.assertRaises(ValueError):
            self.actual_phase(lambda phase: phase.update(native_draw_sha256='0'*64))

    def test_another_native_source_phase_is_refused(self):
        with self.assertRaises(ValueError):
            self.actual_phase(lambda phase: phase['provider_sources'].update({'runtime/native-draw/surfaces.h':'0'*64}))


if __name__ == '__main__':
    unittest.main()
