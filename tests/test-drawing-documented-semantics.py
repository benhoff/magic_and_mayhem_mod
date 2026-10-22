#!/usr/bin/env python3
"""Guard against false toggle/pause admission from incomplete original observations."""
import copy
from pathlib import Path
import sys
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
from drawing_documented_semantics import observe_toggles, observe_pauses


def toggle_fixture():
    calls, actions = [], []
    for i, (before, after) in enumerate([(0, 1), (1, 0), (0, 1)]):
        x = 100 + i
        actions.append(dict(kind='click', button=1, x=x, y=100, ordinal=i+1,
                            **({'modifiers': ['Shift']} if i else {})))

        def sample(value, hit=1):
            t = dict(actor_valid=True, actor_active=1, actor_health=400,
                     pool_id=1, capacity=125, slot_plus_one=1, actor_type=0,
                     actor_owner=0, actor_flag_68=value)
            return dict(t, actor_state=hit, tracked_actor=t, raw_latch_count=value,
                        mode=value, tick=(len(calls)+1)*300)

        calls.append(dict(call=len(calls)+1, mask=1, mouse_record=[x, 100],
                          entry=sample(before), returned=sample(after)))
        calls.append(dict(call=len(calls)+1, mask=2, mouse_record=[x, 100],
                          entry=sample(after), returned=sample(after)))
        calls.append(dict(call=len(calls)+1, mask=16, mouse_record=[500, 300],
                          entry=sample(after, 0), returned=sample(after, 0)))
    return dict(wire_version=3, dropped=0, foreign_thread_calls=0, pending=0, calls=calls), actions


def pause_fixture():
    samples, counter = [], 0
    for tick in range(0, 16001, 250):
        mode = int(3250 <= tick < 6250 or 9250 <= tick < 12250)
        if not mode:
            counter += 1
        samples.append(dict(tick=tick, mode=mode, top_screen=2, validity=63,
                            world_counter=counter, accepted_sequence=len(samples), ani_fields=[1, 0, 0]))
    rows = []
    for i, (tick, code) in enumerate(zip((3000, 6000, 9000, 12000), (32, 32, 45, 46)), 1):
        r = [0]*32
        r[0], r[1], r[2], r[5], r[6], r[10], r[11], r[21] = i, 2, 4, 0x401000, 7, tick, tick, code
        rows.append(tuple(r))
    actions = [dict(kind='tap', key=k) for k in ('space', 'space', 'Insert', 'Delete')]
    return dict(owner_thread=7, samples=samples), rows, actions


class Guards(unittest.TestCase):
    def test_bounded_toggle_cycle(self):
        r, a = toggle_fixture()
        self.assertTrue(observe_toggles(r, a)['bounded_select_clear_reselect_observed'])

    def test_stale_hit_identity_and_release_refuse(self):
        for mutation in ('hit', 'identity', 'release'):
            r, a = toggle_fixture()
            if mutation == 'hit':
                r['calls'][3]['entry']['actor_state'] = 0
            elif mutation == 'identity':
                r['calls'][3]['returned']['tracked_actor']['pool_id'] = 2
            else:
                r['calls'][4]['returned']['raw_latch_count'] = 1
            with self.assertRaises(ValueError):
                observe_toggles(r, a)

    def test_missing_away_and_ambiguous_coordinates_refuse(self):
        for mutation in ('away', 'coordinates'):
            r, a = toggle_fixture()
            if mutation == 'away':
                r['calls'][5]['returned']['actor_state'] = 1
            else:
                r['calls'].append(copy.deepcopy(r['calls'][3]))
            with self.assertRaises(ValueError):
                observe_toggles(r, a)

    def test_bounded_pause_with_active_controls(self):
        s, r, a = pause_fixture()
        result = observe_pauses(s, r, a)
        self.assertEqual(len(result['pause_intervals']), 2)
        self.assertFalse(result['all_simulation_or_animation_pause_proved'])

    def test_advancing_paused_counter_and_no_render_refuse(self):
        for mutation in ('counter', 'render', 'active-control'):
            s, r, a = pause_fixture()
            for ordinal, sample in enumerate(s['samples']):
                if mutation == 'counter' and sample['mode']:
                    sample['world_counter'] = ordinal
                elif mutation == 'render':
                    sample['accepted_sequence'] = 0
                elif mutation == 'active-control':
                    sample['world_counter'] = 0
            with self.assertRaises(ValueError):
                observe_pauses(s, r, a)

    def test_missing_key_wrong_owner_and_unbounded_transition_refuse(self):
        for mutation in ('missing', 'owner', 'time'):
            s, r, a = pause_fixture()
            row = list(r[0])
            if mutation == 'missing':
                row[21] = 99
            elif mutation == 'owner':
                row[6] = 8
            else:
                row[10] = row[11] = 2000
            r[0] = tuple(row)
            with self.assertRaises(ValueError):
                observe_pauses(s, r, a)


    def test_toggle_does_not_bridge_invalid_intermediate_tracking(self):
        for field,value in [('actor_valid',False),('actor_flag_68',0),('actor_health',0)]:
            r,a=toggle_fixture();broken=copy.deepcopy(r['calls'][2]);broken['returned']['tracked_actor'][field]=value
            broken['call']=3
            for c in r['calls'][2:]:c['call']+=1
            r['calls'].insert(2,broken)
            with self.assertRaises(ValueError):observe_toggles(r,a)
        r,a=toggle_fixture();a[1]['ordinal']=1
        with self.assertRaises(ValueError):observe_toggles(r,a)

    def test_pause_cannot_bridge_invalid_samples_or_counter_reversal(self):
        for field,value in [('validity',0),('top_screen',17),('accepted_sequence',0)]:
            s,r,a=pause_fixture();sample=next(x for x in s['samples'] if x['tick']==4000);sample[field]=value
            with self.assertRaises(ValueError):observe_pauses(s,r,a)
        s,r,a=pause_fixture();r.insert(1,r[0])
        with self.assertRaises(ValueError):observe_pauses(s,r,a)

if __name__ == '__main__':
    unittest.main()
