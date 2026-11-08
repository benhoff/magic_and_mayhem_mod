#!/usr/bin/env python3
"""Adverse guards against false natural-defeat acceptance, using retained bytes."""
import copy
import json
import struct
import sys
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT/'tools'))
from coverage_evidence import Reader
from rendering_menu_acceptance import artifact
from rendering_defeat import INPUT, PARENT, LEAF, admit, defeat_route, cohort, close_rows
from rendering_pilot import sha


class Defeat(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        with Reader(ROOT) as r:
            n = INPUT+'matrix-defeat-default-640x480-07'
            cls.a = r.json(n+'.json'); cls.c = r.json(n+'-comparison.json')
            cls.pair = dict(acquisition=dict(record=n+'.json', sha256=r.digest(n+'.json')))
            cls.helper = json.loads(artifact(r, cls.a, 'progression-input-0.json'))
            cls.events = artifact(r, cls.a, 'events.bin')
            cls.gameplay = artifact(r, cls.a, 'gameplay-events.bin')
            cls.plan = r.json('docs/native-rendering-closure.json'); cls.parent = r.json(PARENT)

    def route(self, flow=None, helper=None, events=None, gameplay=None):
        return defeat_route(flow or self.a['flow'], helper or self.helper,
                            events or self.events, gameplay or self.gameplay)

    def test_original_death_report_and_pop(self):
        p = self.route()
        self.assertTrue(p['verified']); self.assertEqual(p['returned_main']['screen'], 3)

    def test_victory_cannot_supply_defeat(self):
        flow = copy.deepcopy(self.a['flow']); flow['states'][-2]['outcome'] = 1
        with self.assertRaises(ValueError): self.route(flow=flow)

    def test_quit_report_cannot_supply_defeat(self):
        flow = copy.deepcopy(self.a['flow']); flow['states'][-2]['texts'][19] = 'You quit the battle.'
        with self.assertRaises(ValueError): self.route(flow=flow)

    def test_stale_main_ack(self):
        flow = copy.deepcopy(self.a['flow']); flow['states'][-1]['ack'] -= 1
        with self.assertRaises(ValueError): self.route(flow=flow)

    def test_stale_main_generation(self):
        flow = copy.deepcopy(self.a['flow']); flow['states'][-1]['generation'] = flow['states'][-2]['generation']
        with self.assertRaises(ValueError): self.route(flow=flow)

    def test_realm_is_wrong_defeat_return(self):
        flow = copy.deepcopy(self.a['flow']); flow['states'][-1]['screen'] = 4
        with self.assertRaises(ValueError): self.route(flow=flow)

    def test_living_player(self):
        helper = copy.deepcopy(self.helper); helper['terminal_hero']['health'] = 1
        with self.assertRaises(ValueError): self.route(helper=helper)

    def test_forged_death_not_in_original_bytes(self):
        helper = copy.deepcopy(self.helper); helper['terminal_hero']['x'] += 1
        with self.assertRaises(ValueError): self.route(helper=helper)

    def test_forged_health_not_in_original_bytes(self):
        helper = copy.deepcopy(self.helper); helper['health_observations'][0]['health'] += 1
        with self.assertRaises(ValueError): self.route(helper=helper)

    def test_simulation_write(self):
        helper = copy.deepcopy(self.helper); helper['simulation_writes'] = True
        with self.assertRaises(ValueError): self.route(helper=helper)

    def test_checkpoint_resume(self):
        helper = copy.deepcopy(self.helper); helper['resume_checkpoint'] = True
        with self.assertRaises(ValueError): self.route(helper=helper)

    def test_casting_policy_is_different_input(self):
        helper = copy.deepcopy(self.helper); helper['casts'] = [{'id': 1}]
        with self.assertRaises(ValueError): self.route(helper=helper)

    def mutate_event(self, predicate, field, value):
        rows = [list(r) for r in struct.iter_unpack('<16I', self.events[16:])]
        row = next(r for r in rows if predicate(r)); row[field] = value
        return self.events[:16]+b''.join(struct.pack('<16I', *r) for r in rows)

    def test_missing_original_world_pop(self):
        raw = self.mutate_event(lambda r:r[1] == 28 and r[3] == 2, 1, 27)
        with self.assertRaises(ValueError): self.route(events=raw)

    def test_wrong_pop_owner(self):
        raw = self.mutate_event(lambda r:r[1] == 28 and r[3] == 2, 2, 999)
        with self.assertRaises(ValueError): self.route(events=raw)

    def test_missing_original_result_return(self):
        raw = self.mutate_event(lambda r:r[1] == 3 and r[3] == 6, 9, 20)
        with self.assertRaises(ValueError): self.route(events=raw)

    def test_raw_actor_owner_mismatch(self):
        raw = bytearray(self.gameplay); struct.pack_into('<I', raw, 16+2*4, 999)
        with self.assertRaises(ValueError): self.route(gameplay=bytes(raw))

    def test_nominal_profile(self):
        self.assertGreater(admit(self.a, self.c, self.pair, 'player-default', [640, 480]), 0)

    def test_wrong_actual_extent(self):
        with self.assertRaises(ValueError): admit(self.a, self.c, self.pair, 'player-default', [800, 600])

    def test_diagnostic_profile(self):
        a = copy.deepcopy(self.a); a['configuration']['diagnostic_only'] = True
        with self.assertRaises(ValueError): admit(a, self.c, self.pair, 'player-default', [640, 480])

    def test_truncated_source(self):
        a = copy.deepcopy(self.a); a['resident']['stage_profile']['capture']['truncated'] = True
        with self.assertRaises(ValueError): admit(a, self.c, self.pair, 'player-default', [640, 480])

    def test_dependent_oracle(self):
        c = copy.deepcopy(self.c); c['original_reference']['native_player_pixels_read'] = True
        with self.assertRaises(ValueError): admit(self.a, c, self.pair, 'player-default', [640, 480])

    @staticmethod
    def plane():
        body = b'abcd'
        h = dict(paint='1', lease='2', reference_binding=1, immutable_lease=False, world='1', width=1, height=1)
        o = dict(paints={1:(h, body)}, swaps=[1, 1])
        row = dict(paint=1, lease=2, reference_binding=1, immutable_lease=False, world=1, pixels=1,
                   mismatching_pixels=0, actual_sha256=sha(body), expected_sha256=sha(body))
        c = dict(success=True, comparisons=[dict(row), dict(row)], pixels_compared=2,
                 mismatching_pixels=0, remaining_surfaces=0, world_presentations=2)
        return o, c

    def test_repeated_swaps_are_real_occurrences(self):
        o, c = self.plane(); self.assertEqual(cohort(o, c)['native_swaps'], 2)

    def test_missing_swap(self):
        o, c = self.plane(); c['comparisons'].pop()
        with self.assertRaises(ValueError): cohort(o, c)

    def test_extra_swap(self):
        o, c = self.plane(); c['comparisons'].append(dict(c['comparisons'][0]))
        with self.assertRaises(ValueError): cohort(o, c)

    def test_changed_actual_gpu_pixels(self):
        o, c = self.plane(); o['paints'][1] = (o['paints'][1][0], b'xxxx')
        with self.assertRaises(ValueError): cohort(o, c)

    def test_original_pixel_mismatch(self):
        o, c = self.plane(); c['comparisons'][0]['expected_sha256'] = '0'*64
        with self.assertRaises(ValueError): cohort(o, c)

    def test_surviving_surface(self):
        o, c = self.plane(); c['remaining_surfaces'] = 1
        with self.assertRaises(ValueError): cohort(o, c)

    def cases(self):
        return [dict(verified=True, identity=str(i), domain=[p, s], pair=self.pair, provider='guard')
                for i, (p, s) in enumerate(( (p, s) for p in ('player-default','player-software')
                    for s in ([640,480],[800,600])))]

    def test_exact_two_rows_and_every_other_row_preserved(self):
        rows = close_rows(self.plan, self.parent, self.cases())
        self.assertEqual(sum(r['state']=='passed' for r in rows), 102)
        for old, new in zip(self.parent['requirements'], rows):
            if old['id'] != LEAF: self.assertEqual(old, new)

    def test_missing_domain(self):
        with self.assertRaises(ValueError): close_rows(self.plan, self.parent, self.cases()[:-1])

    def test_duplicate_identity(self):
        cases = self.cases(); cases[-1]['identity'] = cases[0]['identity']
        with self.assertRaises(ValueError): close_rows(self.plan, self.parent, cases)


if __name__ == '__main__':
    unittest.main()
