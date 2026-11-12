#!/usr/bin/env python3
"""Live-reader guards and actual retained camera source/negative controls."""
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


def module(name):
    s = importlib.util.spec_from_file_location('test_' + name.replace('-', '_'), ROOT / 'tools' / (name + '.py'))
    m = importlib.util.module_from_spec(s); s.loader.exec_module(m); return m


matrix = module('close-rendering-camera-matrix')
collector = module('run-rendering-camera-acceptance')


class LiveReaderTests(unittest.TestCase):
    def wire(self, rows):
        out = bytearray(b'MNMACE01')
        for i, row in enumerate(rows):
            raw = json.dumps(dict(row, index=i)).encode(); out += struct.pack('<II', len(raw), 0) + raw
        return out

    def test_partial_append_never_invents_a_presentation(self):
        with tempfile.TemporaryDirectory() as d:
            path = Path(d) / 'viewport-1.bin'; reader = collector.LiveViewport(Path(d))
            raw = self.wire([dict(kind='open'), dict(kind='paint', paint='1', world='7'), dict(kind='swap', paint='1')])
            path.write_bytes(raw[:-1]); self.assertIsNone(reader.poll())
            path.write_bytes(raw); self.assertEqual(reader.poll()['world'], '7')
            self.assertEqual(reader.poll()['paint'], '1')

    def test_paint_without_swap_is_not_forward_progress(self):
        with tempfile.TemporaryDirectory() as d:
            path = Path(d) / 'viewport-1.bin'; path.write_bytes(self.wire([dict(kind='open'), dict(kind='paint', paint='1', world='7')]))
            self.assertIsNone(collector.LiveViewport(Path(d)).poll())

    def test_wrong_swap_generation_or_stream_identity_refuses(self):
        with tempfile.TemporaryDirectory() as d:
            path = Path(d) / 'viewport-1.bin'
            for rows in ([dict(kind='open'), dict(kind='swap', paint='1')],
                         [dict(kind='open'), dict(kind='paint', paint='1', world='7'), dict(kind='swap', paint='2')]):
                path.write_bytes(self.wire(rows))
                with self.assertRaises(ValueError): collector.LiveViewport(Path(d)).poll()
            path.write_bytes(b'FOREIGN!')
            with self.assertRaises(ValueError): collector.LiveViewport(Path(d)).poll()

    def test_duplicate_live_collector_refuses(self):
        with tempfile.TemporaryDirectory() as d:
            for n in (1, 2): (Path(d) / ('viewport-' + str(n) + '.bin')).touch()
            with self.assertRaises(ValueError): collector.LiveViewport(Path(d)).poll()


class RetainedCameraTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        from coverage_evidence import Reader
        cls.reader = Reader(ROOT)
        cls.report = cls.reader.json(matrix.PREFIX + 'retained-analysis-04.json')
        cls.a = cls.reader.json(cls.report['acquisition']['record'])
        cls.facets = cls.report['source_facets']
        from rendering_menu_acceptance import artifact
        from resident_capture import records
        decoder = matrix.module('inspect-canvas-producers')
        with tempfile.TemporaryDirectory(dir=ROOT / 'working') as directory:
            path = Path(directory) / 'worlds.bin'; path.write_bytes(artifact(cls.reader, cls.a, 'worlds.bin'))
            for generation, chunk, stream, request in records(path):
                if generation == 122 and not chunk:
                    cls.two_cell_rows = decoder.decode(stream, complete=False)[1]; break
        assert hasattr(cls, 'two_cell_rows'), 'Required retained two-Cell source packet missing'

    @classmethod
    def tearDownClass(cls): cls.reader.close()

    def refuses_actions(self, paints=None, markers=None):
        with self.assertRaises(ValueError):
            matrix.action_sources(self.a['camera_actions'] if markers is None else markers,
                                  self.facets['source_bound_paints'] if paints is None else paints)

    def test_actual_software_source_cycle_passes_without_viewport_credit(self):
        self.assertEqual(matrix.action_sources(self.a['camera_actions'], self.facets['source_bound_paints']), self.facets['actions'])
        self.assertEqual([v['last_camera']['view'] for v in self.facets['actions']], [0, 3, 2, 1, 0, 0])
        self.assertEqual(self.facets['cell_call_counts'], {'0': 1, '1': 120, '2': 94})
        self.assertFalse(self.facets['whole_viewport_accepted'])
        self.assertNotEqual(self.facets['actions'][-2]['last_camera']['center'], self.facets['actions'][-1]['last_camera']['center'])

    def test_all_retained_pixels_stay_negative_and_completely_classified(self):
        d = self.report['sharp_sampling']; c = self.reader.json(d['comparison']['record'])
        self.assertFalse(c['success']); self.assertFalse(c['comparison']['success'])
        self.assertFalse(d['whole_viewport_accepted'])
        self.assertEqual(len(d['rows']), 288)
        self.assertEqual([(v['paint'], v['world'], v['mismatching']) for v in d['rows']],
                         [(v['paint'], v['world'], v['mismatching_pixels']) for v in c['comparison']['comparisons']])
        self.assertEqual(d['total_mismatches'], 3819024)
        self.assertEqual(sum(v['non_tie_mismatches'] for v in d['rows']), 0)
        self.assertEqual(sum(v['not_adjacent_expected_texel'] for v in d['rows']), 0)

    def test_actual_marker_bindings_and_normal_pilot_quit(self):
        from campaign_input_observation import decode
        from rendering_menu_acceptance import artifact
        proof = matrix.module('rendering_gameplay_acceptance').bind_markers(self.a['camera_actions'], decode(artifact(self.reader, self.a, 'input-admission.bin')))
        self.assertTrue(proof['verified']); self.assertEqual(proof['markers'], 24)
        callbacks = [[v['menu_id'], v['argument']] for v in matrix.module('test-menu-observer').decode(artifact(self.reader, self.a, 'events.bin')) if v['event'] == 3]
        self.assertEqual(callbacks[-3:], [[26, 2], [22, 3], [3, 4]])
        self.assertTrue(self.a['private_wine_stopped']); self.assertTrue(self.a['prefix_pool']['returned'])

    def test_missing_host_requests_do_not_become_engine_refusals_or_normal_quits(self):
        self.assertEqual([(v['screen'], v['request'], v['acknowledged']) for v in self.report['input_misses']], [(25, 5, 5), (3, 2, 2)])
        for miss in self.report['input_misses']:
            self.assertTrue(miss['ready']); self.assertEqual(miss['status'], 1)
            self.assertFalse(miss['precise_mouse_delivery_cause_isolated']); self.assertFalse(miss['normal_quit'])

    def test_actual_attempt_counts_stop_without_promoting_acceptance(self):
        stop = self.reader.json(self.report['batch_stop']['record'])
        self.assertEqual(sum(v['original_game_runs'] for v in stop['attempts']), 3)
        self.assertEqual(sum(v['original_game_runs'] == 0 for v in stop['attempts']), 2)
        self.assertTrue(stop['stop_further_original_runs']); self.assertEqual(len(stop['attempts']), 5)
        self.assertEqual(self.report['newly_passed_requirements'], 0)
        self.assertEqual((self.report['passed_requirements'], self.report['pending_requirements']), (38, 118))
        self.assertFalse(self.report['full_orientation_matrix_passed']); self.assertFalse(self.report['historical_failures_retired'])

    def test_positive_gate_refuses_the_actual_sharp_negative(self):
        c = self.reader.json(self.report['sharp_sampling']['comparison']['record'])
        with self.assertRaises(ValueError): matrix.assess(self.reader, self.a, c, None)
        claimed_integer = copy.deepcopy(self.a); claimed_integer['configuration']['viewport_scaling'] = 'integer'
        with self.assertRaises(ValueError): matrix.assess(self.reader, claimed_integer, c, None)

    def test_stopped_collector_refuses_before_creating_execution_output(self):
        import subprocess
        report = matrix.PREFIX + 'stop-control-must-not-be-created.json'
        output = ROOT / 'working/camera-acceptance/stop-control-must-not-be-created'
        self.assertFalse(output.exists())
        result = subprocess.run([sys.executable, str(ROOT / 'tools/run-rendering-camera-acceptance.py'), '--report', report], capture_output=True, text=True)
        self.assertNotEqual(result.returncode, 0)
        self.assertIn('Camera batch stopped after physical input misses', result.stderr)
        self.assertFalse(output.exists()); self.assertFalse((ROOT / report).exists())

    def test_native_binary_and_compiler_sources_remain_unchanged(self):
        import hashlib
        plan = self.reader.json(matrix.PREFIX + 'input-plan-01.json')
        self.assertEqual(hashlib.sha256((ROOT / plan['player_binary']['path']).read_bytes()).hexdigest(), plan['player_binary']['sha256'])
        for name, digest in plan['compiler_sources'].items():
            self.assertEqual(self.reader.digest(name), digest, name)
        for miss in self.report['input_misses']:
            a = self.reader.json(miss['record']); self.assertTrue(a['sources_stable']); self.assertTrue(a['player_binary_stable'])

    def test_repeated_redraw_or_missing_context_refuses(self):
        for kind in ('redraw', 'context'):
            paints = copy.deepcopy(self.facets['source_bound_paints'])
            for p in paints:
                if kind == 'redraw': p['world'] = 1
                else: p['camera'] = None
            with self.subTest(kind=kind): self.refuses_actions(paints)

    def test_missing_marker_or_scene_effect_route_refuses(self):
        for kind in ('markers', 'shadow', 'displacement'):
            paints = copy.deepcopy(self.facets['source_bound_paints'])
            for p in paints:
                if p['camera']:
                    if kind == 'markers': p['camera']['creature_markers'] = 0
                    else: p['camera']['raster'] = [r for r in p['camera']['raster'] if r['backend'] != (10 if kind == 'shadow' else 9)]
            with self.subTest(kind=kind): self.refuses_actions(paints)

    def test_every_settled_orientation_is_required(self):
        for view in range(4):
            paints = copy.deepcopy(self.facets['source_bound_paints'])
            for p in paints:
                if p['camera'] and p['camera']['view'] == view: p['camera']['view'] = (view + 1) % 4
            with self.subTest(view=view): self.refuses_actions(paints)

    def test_refused_unfocused_reordered_markers_refuse(self):
        for kind in ('refused', 'unfocused', 'clock', 'phase'):
            rows = copy.deepcopy(self.a['camera_actions'])
            if kind == 'refused': rows[0]['accepted'] = False
            elif kind == 'unfocused': rows[0]['host_focus']['host_focused'] = False
            elif kind == 'clock': rows[-1]['host_us'] = rows[0]['host_us']
            else: rows[1]['phase'] = 'settled'
            with self.subTest(kind=kind): self.refuses_actions(markers=rows)

    def test_every_actual_cell_call_is_preserved_and_checked(self):
        rows = self.two_cell_rows
        cells = [f[1] for f, b in rows if f[2] == 24 and struct.unpack_from('<I', b)[0] == 0]
        self.assertEqual(len(cells), 2)
        result = matrix.context_many_cells(rows); self.assertEqual(result['cell_sequences'], cells)
        one = [(f, b) for f, b in rows if f[1] != cells[-1]]
        expected = matrix.module('close-rendering-source-camera').context(one)
        self.assertEqual(matrix.context_many_cells(one), dict(expected, cell_sequences=[cells[0]]))

    def test_second_cell_cannot_hide_wrong_view_center_surface_or_order(self):
        cell_indices = [i for i, (f, b) in enumerate(self.two_cell_rows) if f[2] == 24 and struct.unpack_from('<I', b)[0] == 0]
        for kind in ('view', 'center', 'surface', 'order'):
            rows = copy.deepcopy(self.two_cell_rows); index = cell_indices[-1]; f, b = rows[index]; f = list(f); b = bytearray(b)
            if kind in ('view', 'center'):
                word = 9 if kind == 'view' else 3; value = struct.unpack_from('<I', b, word * 4)[0]
                struct.pack_into('<I', b, word * 4, value + 1)
            elif kind == 'surface': f[3] += 1
            else: f[1] = 1
            rows[index] = tuple(f), bytes(b)
            with self.subTest(kind=kind), self.assertRaises(ValueError): matrix.context_many_cells(rows)

    def test_repeated_cell_support_does_not_admit_missing_camera_or_creature(self):
        for kind in (0, 1, 2):
            rows = [(f, b) for f, b in self.two_cell_rows if not (f[2] == 24 and struct.unpack_from('<I', b)[0] == kind)]
            with self.subTest(kind=kind), self.assertRaises(ValueError): matrix.context_many_cells(rows)


if __name__ == '__main__': unittest.main()
