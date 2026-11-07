#!/usr/bin/env python3
"""Adverse guards over actual retained animation and auxiliary output witnesses."""
import copy
import importlib.util
import os
from pathlib import Path
import sys
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from coverage_evidence import Reader

spec = importlib.util.spec_from_file_location('animation_auxiliary', ROOT / 'tools/close-rendering-animation-auxiliary.py')
m = importlib.util.module_from_spec(spec)
spec.loader.exec_module(m)


class Checks(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        with Reader(ROOT) as reader:
            cls.report = reader.json(os.environ.get('MNM_ANIMATION_AUXILIARY_REPORT', m.PREFIX + 'closeout-analysis-03.json'))
            assert cls.report['success']
            cls.runs = cls.report['executions']
            cls.parent = reader.json(cls.report['parent']['record'])
            cls.plan = reader.json('docs/native-rendering-closure.json')

    def rows(self, kind='animation:Creatures/Wizard1.spr'):
        rows = copy.deepcopy(self.runs[0]['witnesses'])
        chosen = [v for v in rows if v['kind'] == kind]
        return rows, chosen

    def damage(self, mutate, kind='animation:Creatures/Wizard1.spr'):
        rows, selected = self.rows(kind)
        mutate(selected)
        with self.assertRaises(ValueError):
            m.check_output(rows)

    def test_all_actual_source_output_contexts(self):
        for run in self.runs:
            m.check_output(run['witnesses'])

    def test_missing_route_refuses(self):
        with self.assertRaises(ValueError):
            m.check_output(self.runs[0]['witnesses'][:-1])

    def test_repeated_source_generation_refuses(self):
        self.damage(lambda v: v[1]['source'].update(generation=v[0]['source']['generation']))

    def test_repeated_encoded_frame_refuses(self):
        self.damage(lambda v: v[1]['source'].update(frame_sha256=v[0]['source']['frame_sha256']))

    def test_header_only_changes_cannot_be_animation(self):
        self.damage(lambda v: v[1]['source']['resource']['decoded'].update(bitmap_sha256=v[0]['source']['resource']['decoded']['bitmap_sha256']))

    def test_same_asset_frame_cannot_be_new_animation(self):
        self.damage(lambda v: v[1]['source']['resource'].update(frame=v[0]['source']['resource']['frame']))

    def test_resource_replacement_is_not_temporal_animation(self):
        self.damage(lambda v: v[1]['source']['resource']['asset'].update(sha256='0' * 64))

    def test_camera_motion_is_not_sprite_animation(self):
        self.damage(lambda v: v[1]['source']['camera'].update(center=[0, 0]))

    def test_uncovered_orientation_refuses(self):
        self.damage(lambda v: v[0]['source']['camera'].update(view=1))

    def test_new_source_incarnation_is_not_same_resource_clock(self):
        self.damage(lambda v: v[1]['source']['source_incarnation'].update(incarnation=999))

    def test_unrelated_asset_cannot_supply_creature_output(self):
        self.damage(lambda v: v[0]['source']['resource']['asset'].update(path='terrain.sft'))

    def test_empty_decoded_sprite_refuses(self):
        self.damage(lambda v: v[0]['source']['resource']['decoded'].update(opaque_pixels=0))

    def test_no_visible_change_refuses(self):
        self.damage(lambda v: v[1]['presentation'].update(changed_visible_words=0))

    def test_wrong_actual_pixels_refuse(self):
        self.damage(lambda v: v[0]['presentation'].update(actual_sha256='0' * 64))

    def test_missing_independent_original_output_refuses(self):
        self.damage(lambda v: v[0]['presentation'].update(independently_matched=False))

    def test_lost_lease_refuses(self):
        self.damage(lambda v: v[0]['presentation'].update(lease=0))

    def test_sparse_sample_substitution_refuses(self):
        self.damage(lambda v: v[0]['presentation'].update(mapped_words=1))

    def test_missing_source_coordinates_refuses(self):
        self.damage(lambda v: v[0]['presentation']['mapped_source_words'].pop())

    def test_wrong_original_result_refuses(self):
        self.damage(lambda v: v[0]['source']['descriptor'].__setitem__(2, 999))

    def test_wrong_original_entry_refuses(self):
        self.damage(lambda v: v[0]['source']['descriptor'].__setitem__(1, 0x57e540))

    def test_wrong_source_canvas_refuses(self):
        self.damage(lambda v: v[0]['source']['fields'].__setitem__(3, 999))

    def test_later_rectangle_cannot_receive_sprite_credit(self):
        def cover(values):
            row = values[0]
            width = row['source']['source_shape'][1]
            y, x = divmod(row['presentation']['mapped_source_words'][0], width)
            row['source']['primitive_occlusions'].append(dict(sequence=row['source']['sequence'] + 1,
                                                            area=[x, y, x + 1, y + 1]))
        self.damage(cover)

    def test_actual_queued_rectangles_remain_explicit(self):
        self.assertTrue(any(v['source']['primitive_occlusions'] for r in self.runs for v in r['witnesses']))

    def test_wrong_auxiliary_family_refuses(self):
        self.damage(lambda v: v[0]['source'].update(route='wave'), 'auxiliary:shadow')

    def test_wrong_shadow_backend_refuses(self):
        self.damage(lambda v: v[0]['source']['fields'].__setitem__(15, 7), 'auxiliary:shadow')

    def test_unowned_wave_policy_refuses(self):
        self.damage(lambda v: v[0]['source'].update(owned_reads=False), 'auxiliary:wave')

    def test_missing_wave_phase_refuses(self):
        self.damage(lambda v: v[0]['source']['offsets'].pop(), 'auxiliary:wave')

    def test_unbounded_wave_tap_refuses(self):
        self.damage(lambda v: v[0]['source']['offsets'].__setitem__(0, 999), 'auxiliary:wave')

    def test_missing_actual_domain_cannot_close_rows(self):
        with self.assertRaises(ValueError):
            m.close_rows(self.plan, self.parent, self.runs[:-1])

    def test_duplicate_actual_domain_cannot_close_rows(self):
        with self.assertRaises(ValueError):
            m.close_rows(self.plan, self.parent, self.runs[:3] + self.runs[:1])

    def test_exact_five_rows_preserve_all_parents(self):
        rows = m.close_rows(self.plan, self.parent, self.runs)
        changes = {(after['configuration'], after['id']) for before, after in zip(self.parent['requirements'], rows) if before != after}
        self.assertEqual(changes, m.ROWS)
        self.assertEqual(sum(v['state'] == 'passed' for v in rows), 67)

    def test_software_creature_leaf_remains_pending(self):
        rows = m.close_rows(self.plan, self.parent, self.runs)
        self.assertEqual(next(v for v in rows if v['configuration'] == 'player-software' and
                             v['id'] == 'journey.creatures-effects.creature-animation')['state'], 'pending')


if __name__ == '__main__':
    unittest.main()
