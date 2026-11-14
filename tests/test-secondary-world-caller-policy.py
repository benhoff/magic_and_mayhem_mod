#!/usr/bin/env python3
import copy
import hashlib
import json
from pathlib import Path
import sys
import unittest
ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from secondary_world_caller_policy import (BASE, ADDITIONS, configuration, derive_header,
                                         compiler_overlay, selected_builder, original_bindings)


class Policy(unittest.TestCase):
    def setUp(self):
        self.policy = json.loads((ROOT / 'patches/world-secondary-black-callers-v1.json').read_text())
        self.base = (ROOT / BASE).read_bytes()
    def test_only_two_declared_pairs_added(self):
        header, receipt = derive_header(self.base, self.policy)
        at = receipt['insertion_offset']; inserted = bytes.fromhex(receipt['inserted_bytes'])
        self.assertEqual(header[:at] + header[at + len(inserted):], self.base)
        self.assertEqual(receipt['added_exact_pairs'], 2)
        self.assertEqual(receipt['derived_sha256'], hashlib.sha256(header).hexdigest())
    def test_altered_base_refuses(self):
        with self.assertRaises(ValueError): derive_header(self.base + b' ', self.policy)
    def test_undeclared_site_refuses(self):
        self.policy['additions'][0]['address'] += 1
        with self.assertRaises(ValueError): configuration(self.policy)
    def test_undeclared_tag_refuses(self):
        self.policy['additions'][0]['tag'] = 13
        with self.assertRaises(ValueError): configuration(self.policy)
    def test_changed_signature_refuses(self):
        self.policy['additions'][0]['signature'] = '00' * 10
        with self.assertRaises(ValueError): configuration(self.policy)
    def test_missing_pair_refuses(self):
        self.policy['additions'].pop()
        with self.assertRaises(ValueError): configuration(self.policy)
    def test_duplicate_pair_refuses(self):
        self.policy['additions'].append(copy.deepcopy(self.policy['additions'][0]))
        with self.assertRaises(ValueError): configuration(self.policy)
    def test_other_original_build_refuses(self):
        self.policy['original_sha256'] = '0' * 64
        with self.assertRaises(ValueError): configuration(self.policy)
    def test_boolean_schema_refuses(self):
        self.policy['schema'] = True
        with self.assertRaises(ValueError): configuration(self.policy)
    def test_extra_policy_field_refuses(self):
        self.policy['relax_owner_guard'] = True
        with self.assertRaises(ValueError): configuration(self.policy)
    def test_overlay_maps_exactly_one_input(self):
        overlay = compiler_overlay(ROOT / BASE, Path('/tmp/derived-caller-policy.h'))
        self.assertEqual(len(overlay['roots']), 1)
        self.assertEqual(overlay['roots'][0]['name'], str(ROOT / BASE))
    def test_overlay_cannot_overwrite_base(self):
        with self.assertRaises(ValueError): compiler_overlay(ROOT / BASE, ROOT / BASE)
    def test_actual_builder_only_two_selections(self):
        import ast
        source = (ROOT / 'tools/build-scene-observer.py').read_text()
        a = ast.parse(source); b = ast.parse(selected_builder(source))
        old = {n.targets[0].id: n.value for n in ast.walk(a) if isinstance(n, ast.Assign)
               and len(n.targets) == 1 and isinstance(n.targets[0], ast.Name) and n.targets[0].id in ('flags', 'output')}
        for n in ast.walk(b):
            if isinstance(n, ast.Assign) and len(n.targets) == 1 and isinstance(n.targets[0], ast.Name) and n.targets[0].id in old:
                n.value = copy.deepcopy(old[n.targets[0].id])
        self.assertEqual(ast.dump(a, include_attributes=False), ast.dump(b, include_attributes=False))
    def test_changed_builder_flag_shape_refuses(self):
        source = (ROOT / 'tools/build-scene-observer.py').read_text().replace("flags = ['clang'", "different = ['clang'")
        with self.assertRaises(ValueError): selected_builder(source)
    def test_original_binding_bytes_refuse(self):
        class Wrong:
            def read(self, pc, size): return b'\x00' * size
        with self.assertRaises(ValueError): original_bindings(Wrong(), self.policy)


if __name__ == '__main__': unittest.main()
