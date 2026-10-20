"""Refuse incomplete or overstated portable surface validation."""
import copy
import importlib.util
import json
from pathlib import Path
import sys
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT/'tools'))
spec = importlib.util.spec_from_file_location('high_surfaces', ROOT/'tools/check-rendering-high-surfaces.py')
module = importlib.util.module_from_spec(spec)
spec.loader.exec_module(module)


class SurfaceValidation(unittest.TestCase):
    def setUp(self):
        self.catalog = json.loads(module.CATALOG.read_text())
        matrix = json.loads((ROOT/'research/runtime/native-surface-operation-matrix.json').read_text())
        self.required = {r['id'] for r in matrix['operations'] if r['requirement'] == 'required'}
        self.rows = [dict(id=k, returncode=0, success=True) for k in module.COMPONENTS]

    def test_complete_selected_cohorts_preserve_wider_acceptance(self):
        result = module.summarize(self.catalog, self.rows, self.required)
        self.assertTrue(result['required_fixture_cells_complete'])
        self.assertFalse(result['full_surface_acceptance_complete'])
        self.assertEqual(result['missing_required_fixture_cells'], [])
        self.assertTrue(result['remaining_contracts'])

    def test_old_copy_captures_cannot_claim_restore_or_palette(self):
        bad = copy.deepcopy(self.catalog)
        bad['components'][0]['matrix_ids'] += ['SURF.restore', 'SURF.palette']
        with self.assertRaisesRegex(ValueError, 'cannot establish'):
            module.validate_catalog(bad, self.required)

    def test_zero_exit_or_partial_report_does_not_hide_failure(self):
        self.rows[0].pop('success')
        result = module.summarize(self.catalog, self.rows, self.required)
        self.assertFalse(result['success'])
        self.assertFalse(result['required_fixture_cells_complete'])

    def test_missing_ownership_leaves_its_cells_unvalidated(self):
        self.rows[3].update(returncode=1, success=False)
        result = module.summarize(self.catalog, self.rows, self.required)
        self.assertEqual(set(result['missing_required_fixture_cells']),
                         {'SURF.flip', 'SURF.palette', 'SURF.restore', 'SURF.lifetime_updates'})

    def test_missing_duplicate_or_unreviewed_components_refuse(self):
        for rows in (self.rows[:-1], self.rows+[self.rows[0]]):
            with self.assertRaises(ValueError):
                module.summarize(self.catalog, rows, self.required)
        bad = copy.deepcopy(self.catalog)
        bad['components'][0]['id'] = 'unreviewed'
        with self.assertRaises(ValueError):
            module.validate_catalog(bad, self.required)


if __name__ == '__main__':
    unittest.main()
