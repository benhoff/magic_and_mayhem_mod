#!/usr/bin/env python3
import ast
import copy
from pathlib import Path
import sys
import unittest
ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from broader_interaction_analysis_binding import bind_plan, select_plan_source, FIELDS


class Binding(unittest.TestCase):
    def setUp(self):
        self.hashes = {'native.cpp': 'n', 'runtime.c': 'r', 'oracle.cpp': 'o', 'history.record': 'h', 'original.record': 'x'}
        self.binary = {'working/build/qt-shell/mnm-qt-shell': 'p',
                       'working/build/qt-shell/mnm-acceptance-world-history': 'h',
                       'working/original-reference': 'x'}
        self.phase = dict(family='interaction', compiler_sources={'native.cpp': 'n'}, runtime_sources={'runtime.c': 'r'},
            player_binary={'sha256': 'p'}, oracle=dict(native_sources={'oracle.cpp': 'o'}, binaries=[
                dict(path='working/build/qt-shell/mnm-acceptance-world-history', record='history.record', sha256='h'),
                dict(path='working/original-reference', record='original.record', sha256='x')]))
        self.build = dict(success=True, sources_stable=True, compiler_sources={'native.cpp': 'n'}, runtime_sources={'runtime.c': 'r'},
                          player={'sha256': 'p'}, **{'world-history': {'sha256': 'h'}, 'original-reference': {'sha256': 'x'}})
        self.acquired = dict(success=True, sources_stable=True, private_wine_stopped=True,
                            original_manifest_verified_before_after=True, sources={'native.cpp': 'n', 'runtime.c': 'r'},
                            player_binary={'sha256': 'p'})
        self.legacy = dict(compiler_sources={'old': 'c'}, runtime_sources={'old': 'r'}, player_binary={'sha256': 'old'},
                           oracle={'old': True}, bounds={'deadline': 30}, scenarios=[1, 2], actions=['unchanged'])

    def bind(self):
        return bind_plan(self.legacy, self.phase, self.acquired, self.build, self.hashes.__getitem__, self.binary.__getitem__)

    def test_exact_declared_fields_and_no_mutation(self):
        before = copy.deepcopy((self.legacy, self.phase, self.acquired, self.build))
        result = self.bind()
        self.assertEqual(result['bounds'], self.legacy['bounds'])
        self.assertEqual(result['actions'], self.legacy['actions'])
        self.assertEqual(result['scenarios'], self.legacy['scenarios'])
        self.assertEqual({k for k in result if result[k] != self.legacy[k]}, set(FIELDS))
        self.assertEqual(before, (self.legacy, self.phase, self.acquired, self.build))
        result['bounds']['deadline'] = 1
        self.assertEqual(self.legacy['bounds']['deadline'], 30)

    def test_reject_acquired_native_drift(self):
        self.acquired['sources']['native.cpp'] = 'changed'
        with self.assertRaisesRegex(ValueError, 'native closure'): self.bind()

    def test_reject_unstable_capture(self):
        self.acquired['private_wine_stopped'] = False
        with self.assertRaisesRegex(ValueError, 'Incomplete retained'): self.bind()

    def test_reject_runtime_build_mismatch(self):
        self.build['runtime_sources'] = {'runtime.c': 'other'}
        with self.assertRaisesRegex(ValueError, 'build closure'): self.bind()

    def test_reject_wrong_native_binary(self):
        self.binary['working/build/qt-shell/mnm-qt-shell'] = 'other'
        with self.assertRaisesRegex(ValueError, 'Native binary'): self.bind()

    def test_reject_oracle_source_drift(self):
        self.hashes['oracle.cpp'] = 'changed'
        with self.assertRaisesRegex(ValueError, 'oracle source'): self.bind()

    def test_reject_oracle_payload_drift(self):
        self.hashes['original.record'] = 'changed'
        with self.assertRaisesRegex(ValueError, 'Oracle executable'): self.bind()

    def test_reject_duplicate_oracle_executable(self):
        self.phase['oracle']['binaries'][1] = self.phase['oracle']['binaries'][0]
        with self.assertRaisesRegex(ValueError, 'Different oracle'): self.bind()

    def test_reject_foreign_family(self):
        self.phase['family'] = 'picking'
        with self.assertRaisesRegex(ValueError, 'family'): self.bind()

    def test_actual_analyser_has_only_one_changed_expression(self):
        source = (ROOT / 'tools/check-rendering-interaction.py').read_text()
        selected = 'research/runtime/new-analysis.json'
        original = ast.parse(source)
        changed = ast.parse(select_plan_source(source, selected))
        nodes = [n for n in ast.walk(changed) if isinstance(n, ast.Assign) and len(n.targets) == 1
                 and isinstance(n.targets[0], ast.Name) and n.targets[0].id == 'plan_name']
        self.assertEqual(len(nodes), 1)
        old = next(n for n in ast.walk(original) if isinstance(n, ast.Assign) and len(n.targets) == 1
                   and isinstance(n.targets[0], ast.Name) and n.targets[0].id == 'plan_name')
        self.assertEqual(nodes[0].value.value, selected)
        nodes[0].value = old.value
        self.assertEqual(ast.dump(original, include_attributes=False), ast.dump(changed, include_attributes=False))

    def test_reject_changed_legacy_selector(self):
        source = (ROOT / 'tools/check-rendering-interaction.py').read_text().replace('input-plan-08.json', 'different.json')
        with self.assertRaisesRegex(ValueError, 'selector changed'):
            select_plan_source(source, 'research/runtime/new-analysis.json')

    def test_reject_unsafe_analysis_path(self):
        with self.assertRaisesRegex(ValueError, 'Unsafe'):
            select_plan_source('', 'research/runtime/../old.json')


if __name__ == '__main__': unittest.main()
