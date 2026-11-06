#!/usr/bin/env python3
"""Negative controls on retained camera source packets and actual native swaps."""
import copy
import importlib.util
import os
from pathlib import Path
import struct
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
spec = importlib.util.spec_from_file_location('source_camera', ROOT / 'tools/close-rendering-source-camera.py')
camera = importlib.util.module_from_spec(spec)
spec.loader.exec_module(camera)


@unittest.skipUnless(os.environ.get('MNM_SOURCE_CAMERA_RESULT'), 'Supply retained source-camera evidence for actual-case controls')
class SourceCameraTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.reader = camera.Reader(ROOT)
        cls.result = cls.reader.json(os.environ['MNM_SOURCE_CAMERA_RESULT'])
        cls.a, cls.c = (cls.reader.json(cls.result['pair'][k]['record']) for k in ('acquisition', 'comparison'))
        cls.markers = cls.a['flow']['gameplay_witness']
        cls.paints = cls.result['compared_paints']
        decoder = camera.module('inspect-canvas-producers')
        with tempfile.TemporaryDirectory(dir=ROOT / 'working') as directory:
            p = Path(directory) / 'worlds.bin'
            p.write_bytes(camera.artifact(cls.reader, cls.a, 'worlds.bin'))
            for generation, chunk, stream, request in camera.records(p):
                if generation == 112 and not chunk:
                    cls.rows = decoder.decode(stream, complete=False)[1]
                    break

    @classmethod
    def tearDownClass(cls):
        cls.reader.close()

    def refuses(self, paints=None, markers=None):
        with self.assertRaises(ValueError):
            camera.action_bindings(self.markers if markers is None else markers,
                                   self.paints if paints is None else paints)

    def change_overlay(self, word, value, kind=2):
        rows = copy.deepcopy(self.rows)
        for i, (fields, body) in enumerate(rows):
            if fields[2] == 24 and struct.unpack_from('<I', body)[0] == kind:
                changed = bytearray(body)
                struct.pack_into('<I', changed, word * 4, value)
                rows[i] = fields, bytes(changed)
        return rows

    def test_actual_source_and_six_actions_pass(self):
        self.assertEqual(camera.context(self.rows), self.result['source_contexts'][111]['camera'])
        self.assertEqual(camera.action_bindings(self.markers, self.paints), self.result['actions'])
        self.assertEqual([r['last_camera']['view'] for r in self.result['actions']], [0, 3, 2, 1, 0, 0])

    def test_missing_camera_cell_creature_or_terrain_refuses(self):
        for kind in (0, 1, 2, 3):
            rows = [(f, b) for f, b in self.rows if not (f[2] == 22 if kind == 3
                    else f[2] == 24 and struct.unpack_from('<I', b)[0] == kind)]
            with self.subTest(kind=kind), self.assertRaises(ValueError):
                camera.context(rows)

    def test_disagreeing_source_orientations_refuse(self):
        for kind in range(3):
            with self.subTest(kind=kind), self.assertRaises(ValueError):
                camera.context(self.change_overlay(9, 2, kind))

    def test_disagreeing_source_centers_refuse(self):
        for kind in range(3):
            with self.subTest(kind=kind), self.assertRaises(ValueError):
                camera.context(self.change_overlay(3, 34, kind))

    def test_four_selectors_are_not_four_camera_bodies(self):
        expected = {0: 0, 1: 2, 2: 0, 3: 1}
        self.assertEqual({r['view']: r['camera_body'] for v in self.result['source_contexts']
                          if (r := v['camera']) is not None}, expected)
        rows = copy.deepcopy(self.rows)
        i = next(i for i, (f, b) in enumerate(rows) if f[2] == 24 and struct.unpack_from('<I', b)[0] == 2)
        f, b = rows[i]
        f = list(f)
        f[17] = 3
        rows[i] = tuple(f), b
        with self.assertRaises(ValueError):
            camera.context(rows)

    def test_same_view_on_other_surface_refuses(self):
        rows = copy.deepcopy(self.rows)
        i = next(i for i, (f, _) in enumerate(rows) if f[2] == 22)
        f, b = rows[i]
        f = list(f)
        f[3] += 1
        rows[i] = tuple(f), b
        with self.assertRaises(ValueError):
            camera.context(rows)

    def test_camera_marker_order_is_required(self):
        rows = copy.deepcopy(self.rows)
        indices = [i for i, (f, _) in enumerate(rows) if f[2] == 24]
        rows[indices[0]], rows[indices[1]] = rows[indices[1]], rows[indices[0]]
        with self.assertRaises(ValueError):
            camera.context(rows)

    def test_no_raster_or_repeated_redraw_is_not_scene_progress(self):
        with self.assertRaises(ValueError):
            camera.context([(f, b) for f, b in self.rows if f[2] != 9])
        paints = copy.deepcopy(self.paints)
        for p in paints:
            p['world'] = 112
        self.refuses(paints)

    def test_source_context_cannot_be_replaced_by_periodic_scalar(self):
        paints = copy.deepcopy(self.paints)
        for p in paints:
            p['camera'] = None
        self.refuses(paints)

    def test_each_rotation_requires_the_own_settled_source_view(self):
        for action in camera.ACTIONS[1:5]:
            paints = copy.deepcopy(self.paints)
            action_row = next(v for v in self.result['actions'] if v['action'] == action)
            ids = {v['paint'] for v in action_row['settled_paints']}
            for p in paints:
                if p['paint'] in ids:
                    p['camera']['view'] = (p['camera']['view'] + 1) % 4
            with self.subTest(action=action):
                self.refuses(paints)

    def test_missing_action_output_does_not_borrow_another_window(self):
        w = camera.windows(self.markers)['rotate-2']
        self.refuses([p for p in self.paints if not int(w['released']['host_us']) < p['host_us'] <= int(w['settled']['host_us'])])

    def test_empty_markers_or_missing_shadow_displacement_refuse(self):
        for field in ('creature_markers', 'raster'):
            paints = copy.deepcopy(self.paints)
            for p in paints:
                if p['camera']:
                    p['camera'][field] = 0 if field == 'creature_markers' else [r for r in p['camera'][field] if r['backend'] != 9]
            with self.subTest(field=field):
                self.refuses(paints)

    def test_pan_requires_changed_source_map_center(self):
        paints = copy.deepcopy(self.paints)
        for p in paints:
            if p['camera']:
                p['camera']['center'] = [33, 65]
        self.refuses(paints)

    def test_refused_or_reordered_physical_markers_refuse(self):
        markers = copy.deepcopy(self.markers)
        markers[0]['accepted'] = False
        self.refuses(markers=markers)
        markers = copy.deepcopy(self.markers)
        markers[-1]['host_us'] = markers[0]['host_us']
        self.refuses(markers=markers)

    def test_every_swap_occurrence_keeps_source_and_comparison_identity(self):
        paints = self.result['compared_paints']
        self.assertEqual(len(paints), self.c['comparison']['world_presentations'])
        self.assertTrue(all(p['import_sequence'] <= p['command_sequence'] for p in paints))
        expected = [v for v in self.c['comparison']['comparisons'] if v['world'] > 0]
        self.assertEqual(sum(p['pixels'] for p in paints), sum(v['pixels'] for v in expected))
        self.assertEqual([(p['paint'], p['world'], p['rgba_sha256']) for p in paints],
                         [(p['paint'], p['world'], p['expected_sha256']) for p in expected])

    def test_no_full_requirement_or_scalar_identity_promotion(self):
        accepted = self.reader.json(self.result['accepted_parent']['binding']['record'])
        previous = self.reader.json(accepted['inputs']['progress']['record'])
        self.assertEqual(self.result['requirements'], previous['requirements'])
        self.assertEqual(self.result['decisions'], previous['decisions'])
        self.assertEqual(self.result['newly_passed_requirements'], 0)
        self.assertFalse(self.result['exact_periodic_scalar_to_source_frame_binding_proved'])
        self.assertFalse(self.result['full_camera_picking_hud_requirement_proved'])
        self.assertEqual(self.result['passed_requirements'], 38)

    def output_args(self):
        cls = type(self)
        if not hasattr(cls, 'observed'):
            streams = [camera.artifact(self.reader, self.a, Path(v['path']).name)
                       for v in self.a['artifacts'] if '/capture/' in v['path'] and v['path'].endswith('.bin')]
            cls.observed = camera.acquisition(streams, self.a['session_identity'])
            cls.lifetimes = camera.module('close-rendering-world-exit').imported_lifetimes(cls.observed['commands'])
        return [copy.deepcopy(self.a), copy.deepcopy(self.c),
                dict(self.observed, swaps=list(self.observed['swaps']), paints=dict(self.observed['paints'])),
                {v['generation']: v for v in self.result['source_contexts']}, self.lifetimes]

    def test_swaps_cannot_be_dropped_or_duplicated(self):
        for delta in (-1, 1):
            args = self.output_args()
            if delta < 0:
                args[2]['swaps'].pop()
            else:
                args[2]['swaps'].append(args[2]['swaps'][-1])
            with self.subTest(delta=delta), self.assertRaises(ValueError):
                camera.output_bindings(*args)

    def test_changed_comparison_source_or_retained_pixels_refuse(self):
        for key, value in [('world', 0xffffffff), ('lease', 0), ('actual_sha256', '0' * 64), ('pixels', 1)]:
            args = self.output_args()
            match = next(v for v in args[1]['comparison']['comparisons'] if v['world'] > 0)
            match[key] = value
            with self.subTest(key=key), self.assertRaises(ValueError):
                camera.output_bindings(*args)

    def test_non_gpu_or_unowned_presentation_refuses(self):
        for key, value in [('native_gpu_frame', False), ('gpu_read_complete', False), ('presentation_owner', 'worker'), ('commands', 0)]:
            args = self.output_args()
            ordinal = next(p for p in args[2]['swaps'] if int(args[2]['paints'][p][0]['world']) > 0)
            row, body = args[2]['paints'][ordinal]
            args[2]['paints'][ordinal] = dict(row, **{key: value}), body
            with self.subTest(key=key), self.assertRaises(ValueError):
                camera.output_bindings(*args)

    def test_dependent_history_or_unreturned_sources_refuse(self):
        for flag in ('continuous_from_owned_initial_history', 'native_player_pixels_read', 'original_destination_capture_read'):
            args = self.output_args()
            args[1]['original_reference'][flag] = flag != 'continuous_from_owned_initial_history'
            with self.subTest(flag=flag), self.assertRaises(ValueError):
                camera.output_bindings(*args)
        args = self.output_args()
        args[0]['resident']['verified_returns'] = '0'
        with self.assertRaises(ValueError):
            camera.output_bindings(*args)


if __name__ == '__main__':
    unittest.main()
