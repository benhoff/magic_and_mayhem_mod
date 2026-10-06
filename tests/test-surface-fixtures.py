#!/usr/bin/env python3
"""Original corpus replay and refusal of corrupted or overstated evidence."""
import copy
import importlib.util
import json
from pathlib import Path
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('fixtures', ROOT/'tools/check-surface-fixtures.py')
fixtures = importlib.util.module_from_spec(spec)
spec.loader.exec_module(fixtures)


class SurfaceFixtures(unittest.TestCase):
    def test_original_cpu_corpus(self):
        report = fixtures.check(fixtures.DEFAULT)
        self.assertTrue(report['success'])
        self.assertFalse(report['milestone_complete'])
        self.assertEqual(len(report['cases']), 5)
        self.assertEqual(sorted(row['changed_pixels'] for row in report['cases']), [0, 6, 190, 190, 190])
        self.assertIn('SURF.restore', report['missing_required_matrix_ids'])
        self.assertIn('SURF.clip.rectangles', report['missing_required_matrix_ids'])

    def test_evidence_corruption(self):
        original = json.loads(fixtures.DEFAULT.read_text())['cases'][0]
        root = fixtures.DEFAULT.parent
        for field in ('capture', 'provenance'):
            case = copy.deepcopy(original)
            case[field]['sha256'] = '0'*64
            with self.assertRaises(ValueError):
                fixtures.read_case(root, case)
        for field, value in [('oracle', 'hook_owned_checkpoint'), ('expected_sha256', '0'*64),
                             ('caller_return_va', '0x0058c05d'), ('original_sha256', '0'*64)]:
            case = copy.deepcopy(original)
            case[field] = value
            with self.assertRaises(ValueError):
                fixtures.read_case(root, case)

    def test_path_escape(self):
        for path in ('../corpus.json', '/tmp/corpus.json'):
            with self.assertRaises(ValueError):
                fixtures.local(fixtures.DEFAULT.parent, path)

    def test_poisoned_expected_output_is_not_input(self):
        case = json.loads(fixtures.DEFAULT.read_text())['cases'][2]
        raw, capture = fixtures.read_case(fixtures.DEFAULT.parent, case)
        data = bytearray(raw)
        # Captured-after starts after source and destination-before payloads.
        offset = 128 + len(capture['source']) + len(capture['before'])
        data[offset] ^= 1
        poisoned = fixtures.replay.decode(data)
        native = fixtures.replay.replay(poisoned)
        self.assertEqual(native, fixtures.replay.replay(capture))
        self.assertFalse(fixtures.replay.compare(poisoned, native)['matching'])

    def test_reject_overstated_matrix_claim(self):
        corpus = json.loads(fixtures.DEFAULT.read_text())
        # No source/output files are opened before this invalid claim is refused.
        corpus['cases'][0]['matrix_ids'] = ['SURF.restore']
        with tempfile.TemporaryDirectory(dir=ROOT/'working', prefix='fixture-refusal-') as temp:
            path = Path(temp)/'corpus.json'
            path.write_text(json.dumps(corpus))
            with self.assertRaisesRegex(ValueError, 'cannot establish'):
                fixtures.check(path)


if __name__ == '__main__':
    unittest.main()
