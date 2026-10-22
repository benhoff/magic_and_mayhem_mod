#!/usr/bin/env python3
"""Reject false mutation timing from delayed producer return journaling."""
import copy
from pathlib import Path
import sys
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
from drawing_copy_return_execution import associate_return


def fixture():
    returned = dict(request=7, owner=44, producer_return_sequence=78,
                    destination=1, source=6, destination_extent_stride=[800, 600, 0],
                    destination_position=[400, 300], source_rectangle=[160, 80, 200, 120],
                    raw_key=31)
    row = [96, 78, 6, 1, 6, 800, 600, 0, 400, 300, 160, 80, 200, 120, 31, *([0] * 9)]
    commands = [dict(worker=47, worker_entry_record=40, worker_return_record=41),
                dict(worker=48, worker_entry_record=43, worker_return_record=44)]
    binding = dict(request=7, owner=44, producer_return_sequence=78, api_entry=115, commands=commands)
    current = dict(worker=52, worker_entry_record=55, worker_return_record=56)
    return row, returned, binding, current


class ReturnExecution(unittest.TestCase):
    def test_delayed_return_keeps_prior_worker_completion(self):
        result = associate_return(*fixture())
        self.assertEqual(result['relation'], 'commands-completed-before-worker-entry')
        self.assertEqual(result['last_worker_return_record'], 44)
        self.assertFalse(result['mutation_during_current_worker_proved'])

    def test_overlap_after_and_same_request_stay_distinct(self):
        for current, expected in [
                (dict(worker=52, worker_entry_record=42, worker_return_record=45), 'interleaved-or-overlapping-command-envelope'),
                (dict(worker=52, worker_entry_record=30, worker_return_record=31), 'commands-started-after-worker-return'),
                (dict(worker=48, worker_entry_record=43, worker_return_record=44), 'same-request-worker')]:
            data = list(fixture()); data[3] = current
            self.assertEqual(associate_return(*data)['relation'], expected)

    def test_changed_descriptor_sequence_owner_or_request_refuses(self):
        for mutate in [lambda x: x[0].__setitem__(4, 9), lambda x: x[0].__setitem__(1, 79),
                       lambda x: x[2].__setitem__('owner', 99), lambda x: x[2].__setitem__('request', 8),
                       lambda x: x[2].__setitem__('producer_return_sequence', 79)]:
            data = copy.deepcopy(list(fixture())); mutate(data)
            with self.assertRaises(ValueError): associate_return(*data)

    def test_missing_or_incomplete_workers_refuse(self):
        for mutate in [lambda x: x[2]['commands'].clear(),
                       lambda x: x[2]['commands'][0].__setitem__('worker_return_record', 40),
                       lambda x: x[3].__setitem__('worker_return_record', 55)]:
            data = copy.deepcopy(list(fixture())); mutate(data)
            with self.assertRaises(ValueError): associate_return(*data)


    def test_same_worker_cannot_claim_a_different_interval(self):
        data=list(fixture());data[3]=dict(worker=48,worker_entry_record=42,worker_return_record=45)
        with self.assertRaises(ValueError):associate_return(*data)

    def test_duplicate_negative_and_boolean_intervals_refuse(self):
        for mutate in [lambda x:x[2]['commands'].append(dict(x[2]['commands'][0])),
                       lambda x:x[3].__setitem__('worker_entry_record',-1),
                       lambda x:x[3].__setitem__('worker',True)]:
            data=copy.deepcopy(list(fixture()));mutate(data)
            with self.assertRaises(ValueError):associate_return(*data)

if __name__ == '__main__':
    unittest.main()
