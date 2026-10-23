import copy
import hashlib
from pathlib import Path
import sys
import unittest
sys.path.insert(0, str(Path(__file__).resolve().parents[1]/'tools'))
from canvas_initial_history import analyze, unique


class InitialHistory(unittest.TestCase):
    def setUp(self):
        def row(sequence, op):
            r = [0]*24; r[0] = 96; r[1] = sequence; r[2] = op; r[3] = 1
            r[5:8] = [1, 1, 1]; return r
        self.records = [(row(i, op), b'') for i, op in enumerate((1, 5, 10, 3, 11), 1)]
        self.records[2][0][14] = self.records[4][0][14] = 1
        e = dict(ordinal=1, canvas=1, producer_prefix=1, phase=2, tag=2, cache_known=True,
                 extent=[1, 1, 1], cached_address=1, returned_address=1, lock_return_epoch=1)
        self.storage = dict(closed=True, calls=1, returns=1, observed_lock_return_epochs=1,
                            address_identities=1, events=[e])
        h = hashlib.sha256(b'\x34\x12').hexdigest()
        c = dict(ordinal=1, sequence=3, canvas=1, width=1, height=1, original_sha256=h,
                 native_sha256=h, different_pixels=0)
        n = dict(ordinal=1, sequence=3, canvas=1, width=1, height=1, sha256=h)
        self.replay = dict(success=True, sources_stable=True, native_exit=0, replay_order_changed=False,
                           original_pixels_used_as_native_inputs=False, comparisons=[c], native=dict(
                               success=True, remaining_surfaces=0, original_oracles_read=False,
                               original_pixels_used_as_native_inputs=False,
                               original_raster_results_used_as_native_inputs=False, live_replacement=False,
                               checkpoints=[n]))
    def run_case(self):
        return analyze(self.records, self.storage, self.replay, b'\x34\x12', b'\x34\x12')
    def test_closed_full_initial_plane_has_limited_claim(self):
        r = self.run_case(); self.assertEqual(r['initial_pixels_compared'], 1)
        self.assertEqual(r['last_observed_lock_return_epoch'], 1)
        self.assertFalse(r['physical_allocation_ownership_proved']); self.assertFalse(r['all_writers_observed'])
    def test_duplicate_checkpoint_cannot_hide_a_failure(self):
        self.replay['comparisons'].insert(0, dict(self.replay['comparisons'][0], different_pixels=1))
        with self.assertRaises(ValueError): self.run_case()
    def test_reordered_missing_and_forged_full_cohort_refuse(self):
        for key in ('sequence', 'canvas', 'width', 'height', 'ordinal', 'native_sha256', 'original_sha256', 'different_pixels'):
            saved = copy.deepcopy(self.replay)
            self.replay['comparisons'][0][key] = 'bad' if key.endswith('sha256') else 2
            with self.subTest(key=key), self.assertRaises((ValueError, KeyError)): self.run_case()
            self.replay = saved
        self.replay['native']['checkpoints'] = []
        with self.assertRaises(ValueError): self.run_case()
    def test_duplicate_native_and_boolean_identity_refuse(self):
        self.replay['native']['checkpoints'] *= 2
        with self.assertRaises(ValueError): self.run_case()
        for value in (True, 0, -1, 2**32):
            with self.subTest(value=value), self.assertRaises(ValueError): unique([{'id': value}], 'id')
    def test_intervening_writer_release_and_recreate_refuse(self):
        for op in (1, 2, 5, 9):
            self.records[3][0][2] = op
            with self.subTest(op=op), self.assertRaises(ValueError): self.run_case()
    def test_storage_open_or_unobserved_lock_refuses(self):
        self.storage['closed'] = False
        with self.assertRaises(ValueError): self.run_case()
        self.storage['closed'] = True; self.storage['events'] = []
        with self.assertRaises(ValueError): self.run_case()
    def test_original_plane_cannot_seed_or_replace_native_pixels(self):
        for key in ('original_oracles_read', 'original_pixels_used_as_native_inputs', 'original_raster_results_used_as_native_inputs', 'live_replacement'):
            self.replay['native'][key] = True
            with self.subTest(key=key), self.assertRaises(ValueError): self.run_case()
            self.replay['native'][key] = False
        with self.assertRaises(ValueError): analyze(self.records, self.storage, self.replay, b'\x34\x12', b'\x00\x00')


if __name__ == '__main__': unittest.main()
