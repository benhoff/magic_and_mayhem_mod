#!/usr/bin/env python3
import copy
from pathlib import Path
import sys
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1]/'tools'))
from campaign_movement import validate_movement_probes


class MovementTests(unittest.TestCase):
    def fixture(self):
        rows = [[i, 1, 400, i*250, 0, 0, 0, 1, 200, x, y, 4, 1, 1, 0xffffffff, 12800]
                for i, (x, y) in enumerate([(10,71),(10,71),(9,74),(5,74)], 1)]
        probes = [dict(before_sequence=i, after_sequence=i+1, screen_delta=screen, tile_delta=delta)
                  for i, screen, delta in [(1,[140,-50],[0,0]),(2,[-140,50],[-1,3]),(3,[-140,-60],[-4,0])]]
        return rows, dict(player_owner=0, wizard_slot=0, movement_probes=probes)

    def test_idle_ground_does_not_prevent_valid_later_calibration(self):
        rows, inputs = self.fixture()
        result = validate_movement_probes(rows, inputs)
        self.assertTrue(result['verified'])
        self.assertEqual(result['idle_attempts'], 1)

    def test_attempts_or_fabricated_deltas_cannot_prove_movement(self):
        for mutate in [lambda r,i:i['movement_probes'][1].update(tile_delta=[99,99]),
                         lambda r,i:r[2].__setitem__(6,2), lambda r,i:r[2].__setitem__(5,14),
                         lambda r,i:r[2].__setitem__(7,0), lambda r,i:r[2].__setitem__(8,0),
                         lambda r,i:i['movement_probes'][1].update(after_sequence=99),
                         lambda r,i:i['movement_probes'][1].update(screen_delta=[999,0]),
                         lambda r,i:i['movement_probes'].extend(copy.deepcopy(i['movement_probes'])*3)]:
            rows, inputs = self.fixture(); mutate(rows, inputs)
            with self.assertRaises(ValueError): validate_movement_probes(rows, inputs)
        rows, inputs = self.fixture()
        for row in rows: row[9:11] = [10,71]
        for probe in inputs['movement_probes']: probe['tile_delta'] = [0,0]
        with self.assertRaises(ValueError): validate_movement_probes(rows, inputs)

    def test_no_navigation_does_not_claim_movement(self):
        self.assertFalse(validate_movement_probes([], {})['verified'])


if __name__ == '__main__': unittest.main()
