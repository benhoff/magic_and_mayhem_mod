#!/usr/bin/env python3
"""Reject incomplete lifetime joins using actual retained executions and wire bytes."""
import copy
import importlib.util
import json
import os
from pathlib import Path
import struct
import sys
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from coverage_evidence import Reader
from rendering_menu_acceptance import artifact

spec = importlib.util.spec_from_file_location('retained_rows', ROOT / 'tools/close-rendering-retained-rows.py')
m = importlib.util.module_from_spec(spec)
spec.loader.exec_module(m)


class Checks(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        with Reader(ROOT) as r:
            cls.q = r.json(os.environ.get('MNM_RETAINED_ROW_RESULT', m.PREFIX + 'closeout-analysis-04.json'))
            assert cls.q['success']
            cls.parent = r.json(cls.q['parent']['record'])
            cls.plan = r.json('docs/native-rendering-closure.json')
            cls.case = next(v for v in cls.q['retirements'] if v['kind'] == 'public_native_normal_acquisition')
            a = r.json(cls.case['reference']['record'])
            names = ('native-only-session.json', 'native-output-admission.json', 'original-process-result.json',
                     'render-input.bin', 'render-commands.bin', 'render-commands.bin.control',
                     'native-cursor-lifetime.bin', 'native-draw-retirement.bin')
            data = [artifact(r, a, name) for name in names]
            cls.arguments = [a, *[json.loads(raw) for raw in data[:3]], *data[3:], cls.case['proof']['original_callbacks']]

    def damage_wire(self, mutate):
        arguments = copy.deepcopy(self.arguments)
        mutate(arguments)
        with self.assertRaises(ValueError):
            m.terminal_state(*arguments)

    def damage_proof(self, mutate):
        case = copy.deepcopy(self.case)
        mutate(case['proof'])
        with self.assertRaises(ValueError):
            m.check_retirement(case)

    @staticmethod
    def word(arguments, argument, index, value):
        raw = bytearray(arguments[argument])
        struct.pack_into('<I', raw, index * 4, value)
        arguments[argument] = bytes(raw)

    def test_actual_terminal_wires(self):
        self.assertTrue(m.terminal_state(*self.arguments)['verified'])

    def test_all_actual_retirements(self):
        for case in self.q['retirements']:
            m.check_retirement(case)

    def test_force_stop_is_not_normal(self):
        self.damage_wire(lambda a: a[1].update(force_stop_verified=True))

    def test_restart_is_not_normal(self):
        self.damage_wire(lambda a: a[1].update(restart_requested=True))

    def test_renderer_retirement_required(self):
        self.damage_wire(lambda a: a[1].update(renderer_retired=False))

    def test_shell_exit_code_alone_is_insufficient(self):
        self.damage_wire(lambda a: a[1].update(normal_quit=False))

    def test_foreign_session_refuses(self):
        self.damage_wire(lambda a: a[2].update(session='foreign'))

    def test_abnormal_original_exit_refuses(self):
        self.damage_wire(lambda a: a[3].update(original_exit_code=95))

    def test_foreign_private_prefix_refuses(self):
        self.damage_wire(lambda a: a[3].update(prefix='foreign'))

    def test_diagnostic_case_cannot_supply_normal_credit(self):
        self.damage_wire(lambda a: a[0]['configuration'].update(diagnostic_only=True))

    def test_original_quit_callback_required(self):
        self.damage_wire(lambda a: a[-1].pop())

    def test_repeated_quit_callback_refuses(self):
        self.damage_wire(lambda a: a[-1].append([3, 4]))

    def test_active_input_refuses(self):
        self.damage_wire(lambda a: self.word(a, 4, 5, 1))

    def test_unstable_input_refuses(self):
        self.damage_wire(lambda a: self.word(a, 4, 4, 3))

    def test_held_key_refuses(self):
        self.damage_wire(lambda a: self.word(a, 4, 16, 0x80000000))

    def test_undrained_ring_refuses(self):
        self.damage_wire(lambda a: self.word(a, 5, 9, 0))

    def test_unended_ring_refuses(self):
        self.damage_wire(lambda a: self.word(a, 5, 8, 0))

    def test_pending_stop_refuses(self):
        self.damage_wire(lambda a: self.word(a, 6, 8, 0))

    def test_foreign_stop_refuses(self):
        self.damage_wire(lambda a: self.word(a, 6, 4, 0))

    def test_surviving_control_tail_refuses(self):
        self.damage_wire(lambda a: self.word(a, 6, 16, 1))

    def test_pre_reclamation_graphics_owner_refuses(self):
        self.damage_wire(lambda a: self.word(a, 8, 6, 1))

    def test_unreturned_world_refuses(self):
        self.damage_wire(lambda a: a[0]['resident'].update(verified_returns='0'))

    def test_pending_world_refuses(self):
        self.damage_wire(lambda a: a[0]['resident'].update(pending=True))

    def test_unstopped_private_prefix_refuses(self):
        self.damage_wire(lambda a: a[0].update(private_wine_stopped=False))

    def test_lost_cursor_quiescence_refuses(self):
        self.damage_proof(lambda p: p['cursor'].update(active_clients=1))

    def test_cursor_job_imbalance_refuses(self):
        self.damage_proof(lambda p: p['cursor'].update(queue_accepted=p['cursor']['queue_accepted'] + 1))

    def test_cancelled_source_is_not_finished_success(self):
        self.damage_proof(lambda p: p['source_terminal'].update(finished=True))

    def test_native_owned_source_slot_refuses(self):
        self.damage_proof(lambda p: p['source_terminal'].update(native_owned_slots=1))

    def test_wrong_next_source_generation_refuses(self):
        self.damage_proof(lambda p: p['source_terminal'].update(next_unsubmitted_generation=1))

    def test_wrong_original_source_thread_refuses(self):
        self.damage_proof(lambda p: p['source_terminal'].update(original_thread=1))

    def test_missing_diagnostic_cannot_spread_to_other_collectors(self):
        self.damage_proof(lambda p: p['source_terminal'].update(header_only=True, close_caller_verified=False))

    def test_missing_import_incarnation_refuses(self):
        self.damage_proof(lambda p: p['command_lifetimes'].update(deleted_import_incarnations=0))

    def test_missing_recorded_command_lifetimes_refuses(self):
        self.damage_proof(lambda p: p.update(command_lifetimes=None))

    def test_omitting_accepted_case_cannot_close_gate(self):
        with self.assertRaises(ValueError):
            m.close_rows(self.plan, self.parent, self.q['cohort'], self.q['retirements'][:-1], self.q['creatures'])

    def test_changing_accepted_case_profile_refuses(self):
        retired = copy.deepcopy(self.q['retirements'])
        retired[0]['profile'] = 'foreign'
        with self.assertRaises(ValueError):
            m.close_rows(self.plan, self.parent, self.q['cohort'], retired, self.q['creatures'])

    def test_missing_software_frame_domain_refuses(self):
        with self.assertRaises(ValueError):
            m.close_rows(self.plan, self.parent, self.q['cohort'], self.q['retirements'], self.q['creatures'][:1])

    def test_metadata_only_software_proof_refuses(self):
        creatures = copy.deepcopy(self.q['creatures'])
        creatures[0]['actual_pixels_verified'] = False
        with self.assertRaises(ValueError):
            m.close_rows(self.plan, self.parent, self.q['cohort'], self.q['retirements'], creatures)

    def test_exact_three_rows_preserve_parent(self):
        rows = m.close_rows(self.plan, self.parent, self.q['cohort'], self.q['retirements'], self.q['creatures'])
        changed = {(after['configuration'], after['id']) for before, after in zip(self.parent['requirements'], rows) if before != after}
        self.assertEqual(changed, m.ROWS)
        self.assertEqual(sum(v['state'] == 'passed' for v in rows), self.parent['passed_requirements'] + 3)


if __name__ == '__main__':
    unittest.main()
