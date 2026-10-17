#!/usr/bin/env python3
"""Reject missing branches, no-op admission and relabeled live coverage."""
import copy
from pathlib import Path
import sys
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
from pixel_route_matrix import FAMILY_ROUTES, comparison_matrix


class MatrixTests(unittest.TestCase):
    def setUp(self):
        self.results = {f: dict(success=True, physical_words=100,
            route_coverage=[dict(route=r, native_by_format=[2, 3], changed_by_format=[1, 2]) for r in routes])
            for f, routes in FAMILY_ROUTES.items()}
        self.pending = ['raster/splice0', 'raster/splice1', 'pixel/pcx_words', 'minimap/camera3']

    def test_independent_comparison_does_not_grant_live_coverage(self):
        result = comparison_matrix(self.results, self.pending)
        self.assertEqual(result['compared_routes'], 38)
        self.assertEqual(result['remaining_live_routes'], sorted(self.pending))
        self.assertEqual(result['live_routes_added'], [])
        self.assertFalse(result['complete_replacement'])
        self.assertTrue(all(not row['live_exercised'] for row in result['routes']))

    def test_invalid_evidence(self):
        mutations = [
            lambda r: r.pop('raster'),
            lambda r: r['raster']['route_coverage'].pop(1),
            lambda r: r['raster']['route_coverage'].__setitem__(1, r['raster']['route_coverage'][0]),
            lambda r: r['pixel']['route_coverage'][0].update(changed_by_format=[0, 2]),
            lambda r: r['pixel']['route_coverage'][0].update(native_by_format=[2, 0]),
            lambda r: r['minimap']['route_coverage'][0].update(changed_by_format=[3, 2]),
            lambda r: r['minimap']['route_coverage'][0].update(native_by_format=[True, 3]),
            lambda r: r['colour_rect'].update(success=False),
            lambda r: r['raster'].update(physical_words=0),
        ]
        for mutate in mutations:
            with self.subTest(mutate=mutate):
                results = copy.deepcopy(self.results)
                mutate(results)
                with self.assertRaises(ValueError):
                    comparison_matrix(results, self.pending)

    def test_unreviewed_pending_routes(self):
        for pending in (['raster/splice0'] * 2, ['raster/splice99'], ['pixel/dib']):
            with self.subTest(pending=pending), self.assertRaises(ValueError):
                comparison_matrix(self.results, pending)


if __name__ == '__main__':
    unittest.main()
