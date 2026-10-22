from pathlib import Path
import copy
import importlib.util
import struct
import sys
import unittest
ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
spec = importlib.util.spec_from_file_location('analysis', ROOT / 'tools/analyze-drawing-actor-latches.py')
analysis = importlib.util.module_from_spec(spec); spec.loader.exec_module(analysis)


def fixture():
    h = [0] * 16; h[:2] = struct.unpack('<2I', b'MNMDAPI1')
    h[2:9] = [1, 128, 0x40209ca7, 262144, 1024, 1073741824, 2]
    row = [0] * 32; row[:3] = [1, 2, 3]; row[6] = 18; row[8] = 1
    row[11] = 80; row[27:30] = [450, 350, 1]; row[31] = 1
    end = [0] * 32; end[:3] = [2, 4, 1]; end[6] = 18; end[21] = 1
    api = struct.pack('<16I', *h) + struct.pack('<32I', *row) + struct.pack('<32I', *end)
    actor = dict(slot=1, actor_type=27, owner=0, active=1, health=100, raw_68=1, raw_70=0)
    base = dict(ordinal=1, tick=101, thread=18, screen=2, kind=1, call=1, phase_ordinal=1, flags=11, pool_id=1, capacity=3, active_slots=[0, 1, 2], raw_68_slots=[], raw_70_slots=[], raw_count=0, raw_mode=0, hit_state=1, hit_slot=1, first_latched_actor=None, input_point=None)
    after = dict(base, ordinal=2, tick=102, kind=2, phase_ordinal=2, raw_68_slots=[1], raw_count=1, raw_mode=1, first_latched_actor=actor)
    point = dict(flags=3, owner_thread=18, record_prefix=1, entry_prefix=1, after_frame=0, drops=False, gaps=False, pending=False, cursor_record=1, cursor_entry=1, cursor_tick=80, cursor=[450, 350], current_cursor_admissible=True)
    away = dict(after, ordinal=3, tick=103, kind=0, call=0, phase_ordinal=0, hit_state=0, hit_slot=None, input_point=point)
    before_state = dict(capacity=3,actor_type=27,actor_owner=0,phase_ordinal=1, tick=101, actor_state=1, actor_flag_68=0, slot_plus_one=2, pool_id=1, raw_latch_count=0)
    after_state = dict(before_state, phase_ordinal=2, tick=102, actor_flag_68=1, raw_latch_count=1)
    region = dict(owner=18, calls=[dict(call=1, mask=1, mouse_record=[400, 265] + [0] * 8, entry=before_state, returned=after_state)])
    return dict(owner=18, samples=[base, after, away]), region, api


class Binding(unittest.TestCase):
    def test_actual_phase_and_cursor_joins_are_scoped(self):
        r = analysis.analyze(*fixture())
        self.assertTrue(r['bounded_hover_independent_latch_observed'])
        self.assertEqual(r['observations'][0]['hover_away_witnesses'][0]['cursor'], [450, 350])
        for key in ('continuous_unsampled_state_proved', 'actor_generation_proved', 'complete_selection_proved', 'source_frame_parity_proved'):
            self.assertFalse(r[key])

    def test_invalid_pool_and_changed_actor_stop_without_bridging(self):
        for field, value in [('pool_id', 2), ('flags', 2), ('raw_68_slots', [])]:
            latches, region, api = fixture()
            middle = dict(latches['samples'][2], ordinal=3, input_point=None)
            middle[field] = value
            latches['samples'][2]['ordinal'] = 4
            latches['samples'].insert(2, middle)
            self.assertFalse(analysis.analyze(latches, region, api)['bounded_hover_independent_latch_observed'])
        latches, region, api = fixture()
        latches['samples'][2]['first_latched_actor'] = dict(latches['samples'][2]['first_latched_actor'], owner=1)
        self.assertFalse(analysis.analyze(latches, region, api)['bounded_hover_independent_latch_observed'])

    def test_stale_cursor_and_unmatched_counter_do_not_admit_witness(self):
        latches, region, api = fixture(); latches['samples'][2]['tick'] = 300
        self.assertFalse(analysis.analyze(latches, region, api)['bounded_hover_independent_latch_observed'])
        latches, region, api = fixture(); region['calls'][0]['returned']['raw_latch_count'] = 2
        self.assertEqual(analysis.analyze(latches, region, api)['observations'], [])

    def test_spoofed_cursor_and_phase_are_rejected(self):
        latches, region, api = fixture(); latches['samples'][2]['input_point']['cursor'] = [451, 350]
        with self.assertRaises(ValueError): analysis.analyze(latches, region, api)
        latches, region, api = fixture(); latches['samples'][1]['phase_ordinal'] = 4
        with self.assertRaises(ValueError): analysis.analyze(latches, region, api)


    def test_duplicate_ordinals_and_capacity_or_flag_changes_do_not_bridge(self):
        l,r,a=fixture();l['samples'][2]['ordinal']=2
        with self.assertRaises(ValueError):analysis.analyze(l,r,a)
        for change in ('capacity','mode','flag'):
            l,r,a=fixture();middle=copy.deepcopy(l['samples'][2]);middle.update(input_point=None)
            if change=='capacity':middle['capacity']=4
            elif change=='mode':middle['raw_mode']=0
            else:middle['first_latched_actor']['raw_68']=2
            l['samples'][2]['ordinal']=4;l['samples'].insert(2,middle)
            self.assertFalse(analysis.analyze(l,r,a)['bounded_hover_independent_latch_observed'])

    def test_region_type_owner_and_latched_actor_must_agree(self):
        for field,value in [('actor_owner',1),('actor_type',28),('capacity',4)]:
            l,r,a=fixture();r['calls'][0]['returned'][field]=value
            self.assertEqual(analysis.analyze(l,r,a)['observations'],[])

if __name__ == '__main__': unittest.main()
