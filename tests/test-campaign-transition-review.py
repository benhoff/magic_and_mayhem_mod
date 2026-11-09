#!/usr/bin/env python3
"""Reject stale, substituted and unowned campaign transition witnesses."""
import copy
import json
from pathlib import Path
import struct
import sys
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT/'tools'))
from campaign_transition_review import (INPUT, CASES, COMPRESSED_LIMIT, DECODED_LIMIT,
    bound_artifact, command_bytes, coverage_frontier, native_handoff, read_reference,
    referenced_output, semantic_handoff, nominal_candidate,callbacks_match)
from coverage_evidence import Reader
from rendering_menu_acceptance import artifact
from rendering_pilot import sha


class Transition(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        with Reader(ROOT) as reader:
            cls.a = reader.json(INPUT+CASES[0][0]+'.json')
            cls.c = reader.json(INPUT+CASES[0][1]+'.json')
            cls.events = artifact(reader, cls.a, 'events.bin')

    def test_historical_new_game_only_allows_absent_null_load_metadata(self):
        old={'verified':True,'callback':[1,2,3]};current=dict(old,initial_original_load=None)
        self.assertTrue(callbacks_match(current,old,{}))
        self.assertFalse(callbacks_match(dict(current,callback=[1,2,4]),old,{}))
        self.assertFalse(callbacks_match(dict(current,initial_original_load={'verified':True}),old,{}))
        self.assertFalse(callbacks_match(current,old,{'entry_mode':'original-checkpoint'}))

    def test_actual_original_exit_and_next_region(self):
        result = semantic_handoff(self.a['flow'], self.events)
        self.assertTrue(result['verified'])
        self.assertEqual(result['later_entry']['region'], 2)

    def flow_refusal(self, mutate):
        flow = copy.deepcopy(self.a['flow'])
        mutate(flow)
        with self.assertRaises(ValueError):
            semantic_handoff(flow, self.events)

    def test_reload_cannot_supply_progression(self):
        self.flow_refusal(lambda f: f['states'][7].update(event='load'))

    def test_same_map_refused(self):
        self.flow_refusal(lambda f: f['worlds'][1].update(region=1))

    def test_defeat_refused(self):
        self.flow_refusal(lambda f: f['states'][4].update(outcome=0))

    def test_quit_report_refused(self):
        self.flow_refusal(lambda f: f['states'][4]['texts'].__setitem__(19, 'You quit the battle.'))

    def test_stale_bridge_ack(self):
        self.flow_refusal(lambda f: f['states'][8].update(ack=8))

    def test_stale_bridge_generation(self):
        self.flow_refusal(lambda f: f['states'][8].update(generation=f['states'][7]['generation']))

    def test_wrong_travel_location(self):
        self.flow_refusal(lambda f: f['states'][7].update(location=1))

    def test_stale_next_world(self):
        self.flow_refusal(lambda f: f['worlds'][1].update(after_world=1))

    def test_later_presentation_probe_can_skip_a_generated_frame(self):
        flow=copy.deepcopy(self.a['flow']);flow['worlds'][1]['world']+=1
        self.assertTrue(semantic_handoff(flow,self.events)['verified'])
        flow['worlds'][1]['after_world']=flow['worlds'][1]['world']
        with self.assertRaises(ValueError):semantic_handoff(flow,self.events)

    def test_wrong_frame_mode(self):
        self.flow_refusal(lambda f: f['worlds'][1].update(width=800))

    def mutate_events(self, predicate, field, value):
        rows = [list(r) for r in struct.iter_unpack('<16I', self.events[16:])]
        next(r for r in rows if predicate(r))[field] = value
        return self.events[:16]+b''.join(struct.pack('<16I', *r) for r in rows)

    def test_wrong_exit_thread(self):
        raw = self.mutate_events(lambda r: r[1] == 28, 2, 999)
        with self.assertRaises(ValueError): semantic_handoff(self.a['flow'], raw)

    def test_still_active_old_world(self):
        raw = self.mutate_events(lambda r: r[1] == 28, 6, 1)
        with self.assertRaises(ValueError): semantic_handoff(self.a['flow'], raw)

    def test_missing_next_region_callback(self):
        raw = self.mutate_events(lambda r: r[0] == 34, 3, 17)
        with self.assertRaises(ValueError): semantic_handoff(self.a['flow'], raw)

    def test_truncated_original_history(self):
        with self.assertRaises(ValueError): semantic_handoff(self.a['flow'], self.events[:-1])

    @staticmethod
    def wire(ops):
        return b'MNMCMD05'+struct.pack('<II', 5, 16)+b''.join(
            struct.pack('<III', op, i, len(payload))+payload for i, (op, payload) in enumerate(ops, 1))

    @staticmethod
    def operations():
        create = lambda sid: struct.pack('<7I', sid, 640, 480, 16, 0xf800, 0x7e0, 31)
        imported = lambda sid, g: struct.pack('<5I', sid, g, 0, 10, 0)
        return [(1, create(1)), (19, imported(1, 1)), (7, struct.pack('<I', 1)),
                (1, create(2)), (19, imported(2, 2)), (7, struct.pack('<I', 2)), (8, b'')]

    def test_native_old_surface_retires_before_new_creation(self):
        p = native_handoff(self.wire(self.operations()), 2, [640, 480])
        self.assertLess(p['previous_world']['deleted'], p['first_later_world']['created'])
        self.assertEqual(p['surviving_command_surfaces'], 0)

    def test_native_sampled_baseline_retires_before_observed_later_frame(self):
        ops=self.operations()[:-1]
        ops.extend([(1,struct.pack('<7I',3,640,480,16,0xf800,0x7e0,31)),(19,struct.pack('<5I',3,3,0,10,0)),(7,struct.pack('<I',3)),(8,b'')])
        proof=native_handoff(self.wire(ops),3,[640,480],prior_generation=1)
        self.assertEqual(proof['previous_world']['generation'],1)
        self.assertEqual(proof['observed_later_generation'],3)
        for baseline in (0,3,True):
            with self.assertRaises(ValueError):native_handoff(self.wire(ops),3,[640,480],prior_generation=baseline)
        held=ops.pop(2);ops.insert(7,held)
        with self.assertRaises(ValueError):native_handoff(self.wire(ops),3,[640,480],prior_generation=1)

    def test_old_surface_survives_next_map(self):
        ops = self.operations(); old_delete = ops.pop(2); ops.insert(4, old_delete)
        with self.assertRaises(ValueError): native_handoff(self.wire(ops), 2, [640, 480])

    def test_original_generation_gap(self):
        ops = self.operations(); ops[4] = (19, struct.pack('<5I', 2, 3, 0, 10, 0))
        with self.assertRaises(ValueError): native_handoff(self.wire(ops), 2, [640, 480])

    def test_mixed_source_nonce(self):
        ops = self.operations(); ops[4] = (19, struct.pack('<5I', 2, 2, 0, 11, 0))
        with self.assertRaises(ValueError): native_handoff(self.wire(ops), 2, [640, 480])

    def test_missing_native_retirement(self):
        ops = self.operations(); ops.pop(5)
        with self.assertRaises(ValueError): native_handoff(self.wire(ops), 2, [640, 480])

    def test_missing_command_end(self):
        with self.assertRaises(ValueError): native_handoff(self.wire(self.operations()[:-1]), 2, [640, 480])

    def test_actual_extent_cannot_be_substituted(self):
        with self.assertRaises(ValueError): native_handoff(self.wire(self.operations()), 2, [800, 600])

    def test_referenced_independent_output(self):
        self.assertTrue(referenced_output(self.c, self.a['flow'],
                        self.c['captured_command_stream_sha256'])['verified'])

    def test_changed_command_comparison(self):
        with self.assertRaises(ValueError): referenced_output(self.c, self.a['flow'], '0'*64)

    def test_dependent_oracle_refused(self):
        c = copy.deepcopy(self.c); c['original_reference']['native_player_pixels_read'] = True
        with self.assertRaises(ValueError): referenced_output(c, self.a['flow'], c['captured_command_stream_sha256'])

    def test_missing_transition_output(self):
        c = copy.deepcopy(self.c); c['transition_worlds'][1]['swaps'] = []
        with self.assertRaises(ValueError): referenced_output(c, self.a['flow'], c['captured_command_stream_sha256'])

    def test_mismatching_retained_pixels(self):
        c = copy.deepcopy(self.c); c['comparison']['comparisons'][0]['actual_sha256'] = '0'*64
        with self.assertRaises(ValueError): referenced_output(c, self.a['flow'], c['captured_command_stream_sha256'])

    def test_diagnostic_domain_never_fills_nominal_gap(self):
        cases = [dict(configuration='player-default', extent=[800, 600], nominal_candidate=False),
                 dict(configuration='player-software', extent=[640, 480], nominal_candidate=True)]
        for row in coverage_frontier(cases):
            self.assertEqual(row['state'], 'pending')
            self.assertEqual(len(row['missing_nominal_domains']), 3)

    def test_nominal_candidate_retains_process_and_compiler_requirements(self):
        self.assertTrue(nominal_candidate(self.a, self.c))
        for key in ('compiler_closure_stable', 'player_binary_stable'):
            a = dict(self.a); a.pop(key)
            self.assertFalse(nominal_candidate(a, self.c))
        c = dict(self.c); c.pop('comparison_processes_stopped')
        self.assertFalse(nominal_candidate(self.a, c))

    def test_older_output_does_not_invent_process_retirement(self):
        c = dict(self.c); c.pop('comparison_processes_stopped')
        p = referenced_output(c, self.a['flow'], c['captured_command_stream_sha256'])
        self.assertIsNone(p['historical_comparison_processes_stopped'])

    def test_compressed_read_cap_before_decoder(self):
        with self.assertRaises(ValueError): command_bytes(b'MNMACE01'+b'\0'*COMPRESSED_LIMIT, 's')

    def test_decompression_cap_before_decoder(self):
        raw = b'MNMACE01'+struct.pack('<II', 2, 4)+b'{}'+struct.pack('>I', DECODED_LIMIT+1)
        with self.assertRaisesRegex(ValueError, 'decoded RAM budget'): command_bytes(raw, 's')

    def test_changed_report_reference(self):
        class Fake:
            def read_bytes(self, path): return b'{}'
        with self.assertRaises(ValueError): read_reference(Fake(), dict(record='x', sha256='0'*64))

    def test_artifact_cap_before_read(self):
        class Fake:
            def read_bytes(self, path): raise AssertionError('Oversized input was read')
        acquired = dict(artifacts=[dict(path='a/events.bin', bytes=1000)])
        with self.assertRaises(ValueError): bound_artifact(Fake(), acquired, 'events.bin', 100)


if __name__ == '__main__':
    unittest.main()
