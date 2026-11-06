#!/usr/bin/env python3
"""Actual retained-source guards for finite indexed/history acceptance."""
import copy
import importlib.util
import json
from pathlib import Path
import struct
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from coverage_evidence import Reader
from rendering_menu_acceptance import artifact
from rendering_pilot import acquisition
from resident_capture import records
spec = importlib.util.spec_from_file_location('destination', ROOT / 'tools/close-rendering-destination-history.py')
m = importlib.util.module_from_spec(spec); spec.loader.exec_module(m)


class DestinationTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.reader = Reader(ROOT)
        cls.report = cls.reader.json(m.PREFIX + 'closeout-analysis-05.json')
        cls.case = cls.report['executions'][0]
        cls.a = cls.reader.json(cls.case['pair']['acquisition']['record'])
        cls.c = cls.reader.json(cls.case['pair']['comparison']['record'])
        parent = cls.reader.json(cls.report['accepted_parent']['binding']['record'])
        cls.parent = cls.reader.json(parent['inputs']['progress']['record'])
        cls.plan = cls.reader.json('docs/native-rendering-closure.json')
        streams = [artifact(cls.reader, cls.a, Path(v['path']).name) for v in cls.a['artifacts'] if '/capture/' in v['path'] and v['path'].endswith('.bin')]
        cls.observed = acquisition(streams, cls.a['session_identity'])
        with tempfile.TemporaryDirectory(dir='/tmp') as directory:
            path = Path(directory) / 'source.bin'; path.write_bytes(artifact(cls.reader, cls.a, 'worlds.bin'))
            for generation, chunk, raw, request in records(path):
                if generation == 2 and not chunk:
                    cls.rows = m.module('inspect-canvas-producers').decode(raw, complete=False)[1]
                    cls.request = request; break
        assert hasattr(cls, 'request')
        cls.entry, cls.rasters = m.checked_request(cls.rows, cls.request)

    @classmethod
    def tearDownClass(cls): cls.reader.close()

    def test_all_eight_actual_domains_and_only_four_new_rows_pass(self):
        self.assertEqual(m.close_rows(self.plan, self.parent, self.report['executions']), self.report['requirements'])
        self.assertEqual((self.report['passed_requirements'], self.report['pending_requirements']), (42, 114))
        self.assertEqual(self.report['newly_passed_requirements'], 4)
        self.assertEqual(self.report['source_generations'], 3713)
        self.assertEqual(self.report['world_presentations'], 5256)

    def test_accepted_rows_and_both_decisions_are_preserved(self):
        for before, after in zip(self.parent['requirements'], self.report['requirements']):
            if before['id'] in m.LEAVES: self.assertEqual((before['state'], after['state']), ('pending', 'passed'))
            else: self.assertEqual(before, after)
        self.assertEqual(self.parent['decisions'], self.report['decisions'])
        self.assertFalse(self.report['historical_failures_retired'])

    def test_missing_duplicate_wrong_size_and_unverified_domains_refuse(self):
        runs = self.report['executions']
        for kind in ('missing', 'duplicate', 'size', 'proof', 'identity'):
            bad = copy.deepcopy(runs)
            if kind == 'missing': bad.pop()
            elif kind == 'duplicate': bad[-1] = bad[0]
            elif kind == 'size': bad[-1]['domain'][1] = [800, 600]
            elif kind == 'identity': bad[-1]['identity'] = bad[0]['identity']
            else: bad[-1]['proof']['indexed_verified'] = False
            with self.subTest(kind=kind), self.assertRaises(ValueError): m.close_rows(self.plan, self.parent, bad)

    def test_actual_owned_request_descriptors_pass(self):
        self.assertEqual(m.crc32c(b'123456789'), 0xe3069283)
        entry, rows = m.checked_request(self.rows, self.request)
        self.assertEqual(len(rows), 1802); self.assertEqual(entry[3], 2)
        self.assertTrue(all(d[3] == m.source_hash(f, b) for f, b, d in rows))

    def test_descriptor_source_entry_ax_and_order_are_all_bound(self):
        for word in range(4):
            raw = bytearray(self.request); struct.pack_into('<I', raw, 64 + word * 4, struct.unpack_from('<I', raw, 64 + word * 4)[0] ^ 1)
            struct.pack_into('<I', raw, 48, m.fnv(raw[64:]))
            with self.subTest(word=word), self.assertRaises(ValueError): m.checked_request(self.rows, bytes(raw))

    def test_changed_palette_frame_or_origin_refuses(self):
        first = self.rasters[0][0][1]
        for kind in ('palette', 'frame', 'origin'):
            rows = list(self.rows); at = next(i for i, (f, b) in enumerate(rows) if f[1] == first)
            f, body = rows[at]; f = list(f); body = bytearray(body)
            if kind == 'palette': body[-1] ^= 1
            elif kind == 'frame': body[20] ^= 1
            else: f[8] ^= 1
            rows[at] = tuple(f), bytes(body)
            with self.subTest(kind=kind), self.assertRaises(ValueError): m.checked_request(rows, self.request)

    def test_guard_count_geometry_missing_and_reordered_rasters_refuse(self):
        for word in (6, 7, 8, 9, 13, 14, 15):
            raw = bytearray(self.request); struct.pack_into('<I', raw, word * 4, struct.unpack_from('<I', raw, word * 4)[0] ^ 1)
            with self.subTest(word=word), self.assertRaises(ValueError): m.checked_request(self.rows, bytes(raw))
        bad = [v for v in self.rows if v[0][1] != self.rasters[0][0][1]]
        with self.assertRaises(ValueError): m.checked_request(bad, self.request)

    def test_visible_opaque_clipped_and_shadow_samples_are_real(self):
        for case in self.report['executions']:
            proof = case['proof']
            for kind, witnesses in proof['visible_witnesses'].items():
                self.assertEqual(len(witnesses), 3)
                for row in witnesses:
                    w = row['witness']; self.assertGreater(w['visible_samples'], 0)
                    self.assertEqual(w['fields'][17], 1)
                    self.assertEqual(w['fields'][20], 512)
                    self.assertEqual(w['descriptor'][0], w['fields'][1])
                    self.assertEqual(w['fields'][14], 5 if kind == 'destination-shadow' else 0)

    def test_offscreen_or_unindexed_calls_do_not_supply_witnesses(self):
        f, body, descriptor = next(v for v in self.rasters if m.visible_witness(*v))
        fields = list(f); fields[8] = 100000
        self.assertIsNone(m.visible_witness(fields, body, descriptor))
        fields = list(f); fields[17] = 0
        self.assertIsNone(m.visible_witness(fields, body, descriptor))
        with self.assertRaises(ValueError): m.visible_witness(f, body[:-1], descriptor)

    def test_no_original_destinations_native_pixels_or_zero_seed_credit(self):
        for case in self.report['executions']:
            c = self.reader.json(case['pair']['comparison']['record'])
            ref = c['original_reference']
            self.assertTrue(ref['continuous_from_owned_initial_history'])
            self.assertFalse(ref['native_player_pixels_read']); self.assertFalse(ref['original_destination_capture_read'])
            self.assertFalse(c['first_presented_world']['complete_caller_preparation_proved'])
        self.assertEqual(self.report['original_game_runs'], 0); self.assertEqual(self.report['new_pixel_comparisons'], 0)
        self.assertFalse(self.report['new_native_code']); self.assertFalse(self.report['new_game_fixture'])

    def test_initial_creation_source_prefix_and_inline_history_are_accounted(self):
        for case in self.report['executions']:
            p = case['proof']
            self.assertEqual(p['source_chunks'], int(case['scenario'] == 'normal-quick'))
            for i, world in enumerate(p['source_worlds'], 1):
                self.assertEqual(world['generation'], i)
                self.assertGreater(world['source_incarnation']['created_at'], 0)
                self.assertGreater(world['source_incarnation']['incarnation'], 0)
                self.assertEqual([world['source_incarnation']['width'], world['source_incarnation']['height']], case['domain'][1])

    def test_dependent_reference_or_missing_generation_refuses(self):
        history = dict(worlds=self.case['proof']['source_worlds'], source_chunks=self.case['proof']['source_chunks'])
        for key, value in [('native_player_pixels_read', True), ('original_destination_capture_read', True), ('continuous_from_owned_initial_history', False)]:
            c = copy.deepcopy(self.c); c['original_reference'][key] = value
            with self.subTest(key=key), self.assertRaises(ValueError): m.output_join(self.a, c, self.observed, history)
        history['worlds'] = history['worlds'][:-1]
        with self.assertRaises(ValueError): m.output_join(self.a, self.c, self.observed, history)

    def test_missing_or_changed_actual_swap_refuses(self):
        history = dict(worlds=self.case['proof']['source_worlds'], source_chunks=self.case['proof']['source_chunks'])
        first = next(i for i in self.observed['swaps'] if int(self.observed['paints'][i][0]['world']) > 0)
        observed = dict(self.observed, swaps=[i for i in self.observed['swaps'] if i != first])
        with self.assertRaises(ValueError): m.output_join(self.a, self.c, observed, history)
        c = copy.deepcopy(self.c); next(v for v in c['comparison']['comparisons'] if v['world'] > 0)['actual_sha256'] = '0' * 64
        with self.assertRaises(ValueError): m.output_join(self.a, c, self.observed, history)

    def test_post_import_compositions_keep_all_comparisons_without_scene_credit(self):
        self.assertEqual(self.report['post_import_presentations'], 149)
        self.assertEqual(self.report['live_import_presentations'], 5107)
        self.assertEqual(self.report['retained_after_handle_release'], 3)
        for case in self.report['executions']:
            p = case['proof']; full = p['presented_worlds']
            self.assertEqual(len(full), p['live_import_presentations'] + p['post_import_presentations'])
            for row in full:
                if row['presentation_owner']:
                    self.assertLess(row['presentation_owner']['created_sequence'], row['presentation_owner']['deleted_sequence'])
                else: self.assertFalse(row['source_import_live'])
            represented = {r['world'] for r in full if r['source_import_live']}
            self.assertTrue(all(r['generation'] in represented for rows in p['visible_witnesses'].values() for r in rows))

    def test_current_native_and_reference_source_closures_are_preserved(self):
        self.assertTrue(all(self.reader.digest(n) == h for n, h in self.report['sources'].items()))


if __name__ == '__main__': unittest.main()
