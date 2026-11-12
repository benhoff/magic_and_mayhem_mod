#!/usr/bin/env python3
"""Selection IDs cannot silently become asset filenames or generated-world IDs."""
import importlib.util
from pathlib import Path
import sys
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT/'tools'))
spec = importlib.util.spec_from_file_location('execution_dossier', ROOT/'tools/collect-rendering-execution-research.py')
module = importlib.util.module_from_spec(spec)
spec.loader.exec_module(module)


class SelectionTests(unittest.TestCase):
    def test_acknowledged_row_has_no_asset_identity(self):
        row = module.selection_identity(dict(selected_map_row=0, selected_map_id='1', selected_map_name='A region'))
        self.assertFalse(row['filename_identity_established'])
        self.assertFalse(row['generated_world_identity_established'])

    def test_disagreeing_acknowledgement_refuses(self):
        with self.assertRaises(ValueError):
            module.selection_identity(dict(selected_map_row=0, selected_map_id='3'))

    def test_early_failure_without_selection_is_retained(self):
        self.assertEqual(module.selection_identity({})['selection_id'], '')


if __name__ == '__main__':
    unittest.main()
