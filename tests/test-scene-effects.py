#!/usr/bin/env python3
"""Adverse admission checks over actual retained opaque scene transitions."""
from copy import deepcopy
import os
from pathlib import Path
import sys
import unittest
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT/'tools'))
from coverage_evidence import Reader
import scene_effects as m


class Checks(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        with Reader(ROOT) as r:
            cls.report = r.json(os.environ.get('MNM_SCENE_EFFECTS_REPORT', m.PREFIX+'analysis-03.json'))
            assert cls.report['success']
            cls.parent = r.json(cls.report['parent']['record'])
            cls.plan = r.json('docs/native-rendering-closure.json')

    def reject(self, change):
        value = deepcopy(self.report['cases'][0]['transition'])
        change(value)
        with self.assertRaises((ValueError, KeyError)):
            m.check_transition(value)

    def test_all_actual_domains(self):
        for c in self.report['cases']:
            m.check_transition(c['transition'])

    def test_body_animation_is_not_visibility(self):
        self.reject(lambda v: v['states'][1].update(payload_sha256='0'*64))

    def test_camera_motion_is_not_visibility(self):
        self.reject(lambda v: v['states'][1]['camera'].update(center=[0, 0]))

    def test_changed_bitmap_is_not_visibility(self):
        self.reject(lambda v: v['states'][1]['resource'].update(frame=9999))

    def test_changed_body_anchor_is_not_visibility(self):
        self.reject(lambda v: v['states'][1]['fields'].__setitem__(8, 0))

    def test_changed_clip_is_not_visibility(self):
        self.reject(lambda v: v['states'][1]['fields'].__setitem__(10, 99))

    def test_missing_frame_refuses(self):
        self.reject(lambda v: v['states'][1].update(generation=v['states'][0]['generation']+2))

    def test_repeated_generation_refuses(self):
        self.reject(lambda v: v['states'][1].update(generation=v['states'][0]['generation']))

    def test_changed_incarnation_refuses(self):
        self.reject(lambda v: v['states'][1]['source_incarnation'].update(incarnation=9999))

    def test_changed_mode_refuses(self):
        self.reject(lambda v: v['states'][0]['fields'].__setitem__(14, 1))

    def test_source_descriptor_mismatch_refuses(self):
        self.reject(lambda v: v['states'][0]['descriptor'].__setitem__(0, 0))

    def test_original_ax_mismatch_refuses(self):
        self.reject(lambda v: v['states'][0]['descriptor'].__setitem__(2, 999))

    def test_unknown_entry_refuses(self):
        value = deepcopy(self.report['cases'][0]['transition'])
        value['states'][0]['descriptor'][1] = 0
        with self.assertRaises((ValueError, KeyError)):
            m.check_transition(value)

    def test_blend_is_not_opaque_occlusion(self):
        self.reject(lambda v: v['states'][0]['winners'][0]['fields'].__setitem__(14, 1))

    def test_unknown_writer_refuses(self):
        value = deepcopy(self.report['cases'][0]['transition'])
        value['states'][0]['samples'][0]['winner'] = 0
        with self.assertRaises((ValueError, KeyError)):
            m.check_transition(value)

    def test_equal_colour_is_not_occlusion(self):
        def damage(v):
            s = next(s for state in v['states'] for s in state['samples'] if not s['visible'])
            s['original'] = s['underlying']
        self.reject(damage)

    def test_stationary_redraw_is_not_visibility(self):
        self.reject(lambda v: v['states'][1].update(samples=deepcopy(v['states'][0]['samples'])))

    def test_no_pixel_mapping_refuses(self):
        self.reject(lambda v: v['presentations'][0]['mapped_points'].pop())

    def test_fewer_actual_pixels_refuses(self):
        self.reject(lambda v: v['presentations'][0].update(viewport_pixels=0))

    def test_changed_native_pixels_refuse(self):
        self.reject(lambda v: v['presentations'][0].update(actual_sha256='0'*64))

    def test_lost_native_lease_refuses(self):
        self.reject(lambda v: v['presentations'][0].update(lease=0))

    def test_wrong_native_generation_refuses(self):
        self.reject(lambda v: v['presentations'][0].update(generation=0))

    def test_overwritten_rectangle_refuses(self):
        def damage(v):
            s = v['states'][0]
            y, x = divmod(v['points'][0], s['fields'][5])
            s['primitive_occlusions'].append(dict(sequence=max(w['fields'][1] for w in s['winners'])+1,
                                                  area=[x, y, x+1, y+1]))
        self.reject(damage)

    def test_missing_domain_refuses(self):
        with self.assertRaises(ValueError):
            m.close_rows(self.plan, self.parent, self.report['cases'][:-1])

    def test_reused_domain_refuses(self):
        with self.assertRaises(ValueError):
            m.close_rows(self.plan, self.parent, self.report['cases'][:3]+self.report['cases'][:1])

    def test_preserves_every_parent_and_only_two_rows(self):
        rows = m.close_rows(self.plan, self.parent, self.report['cases'])
        changed = {(a['configuration'], a['id']) for a, b in zip(rows, self.parent['requirements']) if a != b}
        self.assertEqual(changed, {(p, m.LEAF) for p in ('player-default', 'player-software')})
        for row in rows:
            if row['id'] in m.TARGETS[1:]:
                self.assertEqual(row['state'], 'pending')
        self.assertEqual(self.report['decisions'], self.parent['decisions'])

    def test_actual_progress_reader_keeps_four_semantic_rows_pending(self):
        import coverage_rendering_artifacts as ui
        import json
        definition = json.loads((ROOT/'apps/coverage-ui/tracks.json').read_text())
        with patch.object(ui, 'read_json', return_value=self.report):
            view = ui.rendering_artifacts_view(ROOT, definition)
        self.assertEqual(view['progress']['counts'], {'passed': 90, 'pending': 66})
        self.assertEqual(view['progress']['total'], 156)
        self.assertEqual(sum(r['state']=='pending' for r in self.report['task_rows']), 4)

    def test_join_refuses_unreviewed_analyzer_drift_and_case_relabelling(self):
        joiner = m.module('join-scene-effects')
        for kind in ('analyzer', 'domain', 'identity', 'provider', 'failed'):
            value = deepcopy(self.report)
            if kind == 'analyzer':
                value['analysis_sources']['tools/scene_effects.py'] = '0'*64
            elif kind == 'domain':
                value['cases'][0]['domain'][0] = 'player-software'
            elif kind == 'identity':
                value['cases'][0]['identity'] = '0'*32
            elif kind == 'provider':
                value['cases'][0]['provider'] = {}
            else:
                value['success'] = False
            with self.subTest(kind=kind), Reader(ROOT) as r:
                read = r.json
                with patch.object(r, 'json', side_effect=lambda n: value if n == 'candidate' else read(n)):
                    with self.assertRaises(ValueError):
                        joiner.join(r, 'candidate')


if __name__ == '__main__':
    unittest.main()
