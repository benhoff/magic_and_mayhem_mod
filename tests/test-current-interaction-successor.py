#!/usr/bin/env python3
import copy
import inspect
from pathlib import Path
import sys
import unittest
ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
import broader_player_current_interaction_matrix as binding
from broader_player_copy_matrix import current as original_current
from broader_player_runtime_caller_matrix import patched_source


class Reader:
    def __init__(self):
        self.phase = {'record': 'phase', 'sha256': 'phase-hash'}
        old = binding.P + binding.RECOVERED + '-coordinator-01.json'
        self.new = binding.P + binding.RECOVERED + '-coordinator-02.json'
        self.values = {
            old: dict(success=False, case=binding.RECOVERED, plan=self.phase),
            'execution': dict(success=True, original_game_runs=0, new_public_player_executions=0,
                new_request_reservations=0, request_rows_unchanged=True, original_acquisition_preserved=True,
                only_admission_fields_changed=True, validation={'record': 'validation'}, acquisition={'record': 'acquisition'}),
            self.new: dict(success=True, kind='broader_domain_case_coordinator', case=binding.RECOVERED,
                plan=self.phase, previous_failed_coordinator={'record': old, 'sha256': 'old-hash'},
                original_game_runs_added=0, new_acquisition_performed=False,
                retained_analysis_execution={'record': 'execution', 'sha256': 'execution-hash'},
                validation={'record': 'validation'}, acquisition={'record': 'acquisition'})}
        self.hashes = {'phase': 'phase-hash', old: 'old-hash', self.new: 'new-hash', 'execution': 'execution-hash'}
    def digest(self, path): return self.hashes[path]
    def json(self, path): return copy.deepcopy(self.values[path])


class Successor(unittest.TestCase):
    def setUp(self):
        self.reader = Reader()
        self.spec = dict(plan='phase', case={'id': binding.RECOVERED}, role='world', family='interaction')
    def resolve(self): return binding.coordinator_record(self.reader, self.spec)
    def test_exact_retained_successor(self): self.assertEqual(self.resolve(), self.reader.new)
    def test_replaced_failure_refuses(self):
        self.reader.values[binding.P + binding.RECOVERED + '-coordinator-01.json']['success'] = True
        with self.assertRaises(ValueError): self.resolve()
    def test_foreign_family_refuses(self):
        self.spec['family'] = 'hotspot'
        with self.assertRaises(ValueError): self.resolve()
    def test_extra_game_refuses(self):
        self.reader.values['execution']['original_game_runs'] = 1
        with self.assertRaises(ValueError): self.resolve()
    def test_extra_reservation_refuses(self):
        self.reader.values['execution']['new_request_reservations'] = 1
        with self.assertRaises(ValueError): self.resolve()
    def test_acquisition_changed_refuses(self):
        self.reader.values['execution']['original_acquisition_preserved'] = False
        with self.assertRaises(ValueError): self.resolve()
    def test_request_identity_change_refuses(self):
        self.reader.values['execution']['only_admission_fields_changed'] = False
        with self.assertRaises(ValueError): self.resolve()
    def test_unchanged_original_current_predicate_except_coordinator_expression(self):
        source = inspect.getsource(original_current)
        changed = patched_source(source)
        self.assertEqual(changed.replace('co_ref=ref(_caller_coordinator_record(r,spec))',
                                         "co_ref=ref(P+case['id']+'-coordinator-01.json')"), source)


if __name__ == '__main__': unittest.main()
