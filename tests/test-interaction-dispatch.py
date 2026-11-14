#!/usr/bin/env python3
import ast
import copy
from pathlib import Path
import sys
import unittest
ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from broader_interaction_dispatch import comparison_route, dispatch_source


class Dispatch(unittest.TestCase):
    def setUp(self):
        self.value = dict(success=False, sources_stable=True, comparison_processes_stopped=True,
            original_manifest_verified_before_after=True, error='Camera packet crosses a source chunk',
            comparison=dict(success=True, mismatching_pixels=0))
    def test_exact_complete_pixel_semantic_refusal(self): self.assertEqual(comparison_route(self.value), 'semantics')
    def test_source_drift_refuses(self):
        self.value['sources_stable'] = False
        with self.assertRaises(ValueError): comparison_route(self.value)
    def test_active_comparison_refuses(self):
        self.value['comparison_processes_stopped'] = False
        with self.assertRaises(ValueError): comparison_route(self.value)
    def test_missing_original_verification_refuses(self):
        self.value['original_manifest_verified_before_after'] = False
        with self.assertRaises(ValueError): comparison_route(self.value)
    def test_other_semantic_refusal_is_not_accepted(self):
        self.value['error'] = 'different functional failure'
        with self.assertRaises(ValueError): comparison_route(self.value)
    def test_pixel_disagreement_remains_negative(self):
        self.value['comparison'] = dict(success=False, mismatching_pixels=1)
        self.assertEqual(comparison_route(self.value), 'negative')
    def test_no_pixel_proof_refuses(self):
        del self.value['comparison']
        with self.assertRaises(ValueError): comparison_route(self.value)
    def test_complete_success_requires_semantic_proof(self):
        self.value.update(success=True, proof={'verified': True})
        self.assertEqual(comparison_route(self.value), 'complete')
        self.value['proof']['verified'] = False
        with self.assertRaises(ValueError): comparison_route(self.value)
    def test_all_original_coordinator_AST_except_comparison_command_is_identical(self):
        source = (ROOT / 'tools/run-broader-discovery-domain-case.py').read_text()
        original = ast.parse(source); selected = ast.parse(dispatch_source(source))
        calls = [v for v in ast.walk(selected) if isinstance(v, ast.Call) and isinstance(v.func, ast.Name)
                 and v.func.id == '_bound_comparison_command']
        self.assertEqual(len(calls), 1)
        class Restore(ast.NodeTransformer):
            def visit_Call(self, node):
                if isinstance(node.func, ast.Name) and node.func.id == '_bound_comparison_command':
                    return ast.parse("case['comparison_command']", mode='eval').body
                return self.generic_visit(node)
        self.assertEqual(ast.dump(original, include_attributes=False), ast.dump(Restore().visit(selected), include_attributes=False))
    def test_changed_original_dispatch_refuses(self):
        source = (ROOT / 'tools/run-broader-discovery-domain-case.py').read_text().replace("case['comparison_command']", "case['different_command']")
        with self.assertRaises(ValueError): dispatch_source(source)


if __name__ == '__main__': unittest.main()
