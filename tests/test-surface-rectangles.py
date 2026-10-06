#!/usr/bin/env python3
"""Offline comparison with retained original wrapper argument traces."""
import copy
import gzip
import hashlib
import importlib.util
import json
from pathlib import Path
import unittest

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('rectangles', ROOT/'tools/compare-surface-rectangles.py')
rectangles = importlib.util.module_from_spec(spec)
spec.loader.exec_module(rectangles)


class RectangleTraces(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        parent = ROOT/'tests/fixtures/surfaces'
        catalog = json.loads((parent/'rectangle-forwarding-corpus.json').read_text())
        packed = (parent/catalog['path']).read_bytes()
        if hashlib.sha256(packed).hexdigest() != catalog['sha256']:
            raise ValueError('Captured argument fixture changed')
        raw = gzip.decompress(packed)
        if hashlib.sha256(raw).hexdigest() != catalog['raw_sha256']:
            raise ValueError('Raw original fixture changed')
        cls.traces = json.loads(raw)
        if catalog['original_sha256'] != rectangles.BUILD_HASH or len(cls.traces) != catalog['cases']:
            raise ValueError('Original fixture identity/count mismatch')

    def test_all_original_arguments(self):
        self.assertEqual(len(self.traces), 966)
        for row in self.traces:
            self.assertEqual(rectangles.expected(row['input']), row['original'])

    def test_outside_and_empty_are_forwarded(self):
        selected = [row for row in self.traces if row['input']['label'].startswith(('outside-', 'empty-'))]
        self.assertEqual(len(selected), 360)
        for row in selected:
            self.assertEqual(row['original']['calls'], 1)
            self.assertEqual(row['original']['source'], row['input']['source'])

    def test_translation_mutation_boundary(self):
        for row in self.traces:
            case, original = row['input'], row['original']
            self.assertEqual(original['source_after'], case['source'])
            if case['entry'] != 0x58cbc0:
                self.assertEqual(original['destination_after'], case['destination'])

    def test_wrong_crop_or_flags_are_detected(self):
        row = next(row for row in self.traces if row['input']['label'] == 'left' and
                   row['input']['fullscreen'] and row['input']['entry'] == 0x58c360)
        cropped = copy.deepcopy(row['original'])
        cropped['destination'][0] = 0
        self.assertNotEqual(rectangles.expected(row['input']), cropped)
        changed = copy.deepcopy(row['original'])
        changed['flags'] ^= 0x10
        self.assertNotEqual(rectangles.expected(row['input']), changed)


if __name__ == '__main__':
    unittest.main()
