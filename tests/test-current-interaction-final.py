#!/usr/bin/env python3
import copy
from pathlib import Path
import sys
import unittest
ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from broader_player_current_interaction_final_matrix import bind_current, original_final_current
from broader_current_interaction_final_gate import reviewed_interaction, CONJUNCTION, RECOVERED


class Reader:
    def __init__(self):
        self.values = {CONJUNCTION: dict(success=True, kind='reviewed_current_interaction_retained_analysis_conjunction',
            case=RECOVERED, qualified_current_contexts=45, request_rows=152, conservative_charges=201,
            original_game_runs=0, new_native_executions=0, new_request_reservations=0,
            Native_runtime_compiler_player_controller_sources_changed=False, source_waivers=[],
            old_reports_or_hashes_overwritten=False, historical_failures_retired=False,
            total_positive_contexts=90, total_positive_original_starts=98, final_request_rows=197,
            final_conservative_charges=246, source_chunks_credited_as_Worlds=0,
            source_chunks_credited_as_action_witnesses=0, pixels=3202419000, mismatching_pixels=0,
            controls=[{'record': n, 'sha256': n} for n in ['c12', 'c16', 'c8']], validation={'record': 'v'},
            acquisition={'record': 'a'}, coordinator={'record': 'co2'}, typed_bookkeeping={'record': 'typed', 'sha256': 'typed'}),
            'c12': dict(success=True, tests=12, sources={'source': 'source'}),
            'c16': dict(success=True, tests=16, sources={'source': 'source'}),
            'c8': dict(success=True, tests=8, sources={'source': 'source'}),
            'typed': dict(success=True, results=[dict(replicas_verified=2, logical_restore_verified=True,
                                                    real_restore_performed=True) for _ in range(2)])}
        self.changed = set()
    def digest(self, path): return 'changed' if path in self.changed else path
    def json(self, path): return copy.deepcopy(self.values[path])


class Final(unittest.TestCase):
    def setUp(self):
        self.reader = Reader()
        self.values = [dict(spec={'case': {'id': RECOVERED}}, validation={'record': 'v'}, acquisition={'record': 'a'}, coordinator={'record': 'co2'})]
    def check(self): return reviewed_interaction(self.reader, self.values)
    def test_exact_retained_binding(self): self.assertEqual(self.check()['record'], CONJUNCTION)
    def test_changed_controls_refuse(self):
        self.reader.changed.add('source')
        with self.assertRaises(ValueError): self.check()
    def test_missing_case_refuses(self):
        with self.assertRaises(ValueError): reviewed_interaction(self.reader, [])
    def test_wrong_coordinator_refuses(self):
        self.values[0]['coordinator'] = {'record': 'co1'}
        with self.assertRaises(ValueError): self.check()
    def test_extra_game_refuses(self):
        self.reader.values[CONJUNCTION]['original_game_runs'] = 1
        with self.assertRaises(ValueError): self.check()
    def test_changed_denominator_refuses(self):
        self.reader.values[CONJUNCTION]['total_positive_contexts'] = 89
        with self.assertRaises(ValueError): self.check()
    def test_source_chunk_World_credit_refuses(self):
        self.reader.values[CONJUNCTION]['source_chunks_credited_as_Worlds'] = 1
        with self.assertRaises(ValueError): self.check()
    def test_unrestored_database_refuses(self):
        self.reader.values['typed']['results'][0]['real_restore_performed'] = False
        with self.assertRaises(ValueError): self.check()
    def test_original_final_predicate_code_is_identical(self):
        bound = bind_current(lambda *args: None)
        self.assertIs(bound.__code__, original_final_current.__code__)
    def test_native_phase_still_required(self):
        value = dict(phase={'current_native_source_candidate': False}, current_sources={}, publication_applicability=[])
        with self.assertRaises(ValueError): bind_current(lambda *args: value)(None, self.reader, None, None)
    def test_full_current_sources_still_required(self):
        value = dict(phase={'current_native_source_candidate': True}, current_sources={'x': 'old'}, publication_applicability=[])
        with self.assertRaises(ValueError): bind_current(lambda *args: value)(None, self.reader, None, None)
    def test_no_source_waiver_still_required(self):
        value = dict(phase={'current_native_source_candidate': True}, current_sources={}, publication_applicability=[1])
        with self.assertRaises(ValueError): bind_current(lambda *args: value)(None, self.reader, None, None)



class ReaderImports(unittest.TestCase):
    def test_actual_reader_imports_only(self):
        import ast
        from broader_current_interaction_final_runner import selected_source, READERS
        for name in READERS.values():
            original = ast.parse((ROOT / 'tools' / name).read_text())
            selected = ast.parse(selected_source((ROOT / 'tools' / name).read_text()))
            for node in ast.walk(selected):
                if isinstance(node, ast.ImportFrom):
                    if node.module == 'broader_player_current_interaction_final_matrix':
                        node.module = 'broader_player_discovery_final_matrix'
                    elif node.module == 'broader_current_interaction_final_gate':
                        node.module = 'broader_discovery_final_gate'
            self.assertEqual(ast.dump(original, include_attributes=False), ast.dump(selected, include_attributes=False))
    def test_internal_supplement_import_selected(self):
        from broader_current_interaction_final_runner import selected_source
        for name in ['check-broader-player-acceptance-discovery.py', 'check-broader-discovery-failure-relevance.py']:
            self.assertIn('from broader_current_interaction_final_gate import supplement', selected_source((ROOT / 'tools' / name).read_text()))

if __name__ == '__main__': unittest.main()
