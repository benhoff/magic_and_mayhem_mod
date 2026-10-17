#!/usr/bin/env python3
"""Require unique owner/interval/descriptor identities before command association."""
import copy
from pathlib import Path
import sys
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
from drawing_copy_bindings import associate


def fixture():
    entries, returns, calls, points, submissions, packets = [], [], {}, {}, [], []
    for request, owner, returned in [(1, 11, 13), (2, 22, 12)]:
        entry = dict(request=request, owner=owner, producer_prefix=9 + request,
                     destination=1, source=2, destination_extent_stride=[800, 600, 0],
                     destination_position=[0, 0], source_rectangle=[0, 0, 800, 600],
                     raw_key=0xffffffff)
        entries.append(entry)
        returns.append(dict(entry, producer_return_sequence=returned))
        call = [0] * 32
        call[2], call[6], call[21] = 7, owner, 16
        call[24:28] = [0, 0, 800, 600]
        calls[request] = call
        phases = {}
        for phase in (1, 2, 3):
            point = [0] * 48
            point[7] = point[30] = point[46] = owner
            point[18] = point[34] = 19
            point[19] = point[35] = 9 + request
            point[22], point[38] = 1, 2
            point[23:25] = [800, 600]
            phases[phase] = point
        points[request] = phases
        submissions.append(dict(call=request, api_entry=request, thread=owner, hresult=0))
        packets.append(dict(command=request, worker=request, submission=request, emit_thread=owner,
                            worker_entry_record=3 * request - 1, worker_return_record=3 * request))
    observation = dict(entries=entries, returns=returns[::-1])
    return [observation, calls, points, submissions, packets]


class Bindings(unittest.TestCase):
    def test_return_reversal_keeps_separate_worker_order(self):
        result = associate(*fixture())
        self.assertTrue(result['all_requests_bound'])
        self.assertEqual(result['adjacent_return_reversals'], [
            dict(entry_order=[1, 2], return_order=[2, 1], worker_order=[1, 2])])
        self.assertFalse(result['general_replay_order_proved'])

    def test_owner_geometry_key_and_closed_boundary_refuse(self):
        for mutation in [lambda x: x[1][1].__setitem__(6, 99),
                         lambda x: x[1][1].__setitem__(24, 1),
                         lambda x: x[1][1].__setitem__(21, 17),
                         lambda x: x[2][1][3].__setitem__(19, 13)]:
            data = fixture()
            mutation(data)
            result = associate(*data)
            self.assertEqual(result['unbound_requests'], [1])
            self.assertIn(1, result['excluded_api_entries'])
            self.assertFalse(result['all_requests_bound'])

    def test_missing_phase_refuses(self):
        data = fixture()
        del data[2][1][2]
        with self.assertRaises(ValueError):
            associate(*data)

    def test_duplicate_or_ambiguous_binding_refuses(self):
        data = fixture()
        data[1][3] = copy.deepcopy(data[1][1])
        data[2][3] = copy.deepcopy(data[2][1])
        with self.assertRaises(ValueError):
            associate(*data)
        data = fixture()
        data[0]['entries'].append(dict(data[0]['entries'][0], request=3))
        data[0]['returns'].append(dict(data[0]['returns'][1], request=3))
        with self.assertRaises(ValueError):
            associate(*data)

    def test_missing_changed_or_shared_worker_identity_refuses(self):
        for mutation in [lambda x: x[4].pop(0),
                         lambda x: x[4][0].__setitem__('emit_thread', 99),
                         lambda x: x[4][1].__setitem__('worker', 1),
                         lambda x: x[3][0].__setitem__('hresult', 1)]:
            data = fixture()
            mutation(data)
            with self.assertRaises(ValueError):
                associate(*data)

    def test_interleaved_commands_do_not_claim_total_order(self):
        data = fixture()
        data[4][0]['worker_return_record'] = 6
        result = associate(*data)
        self.assertIsNone(result['adjacent_return_reversals'][0]['worker_order'])


if __name__ == '__main__':
    unittest.main()
