#!/usr/bin/env python3
"""Keep interval, owner and numerical identity gaps distinct from ownership."""
from pathlib import Path
import sys
import unittest
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
from drawing_storage_query_returns import bind


def fixture():
    entry = dict(ordinal=1, phase=1, call=1, tag=2, thread=18, cache_known=True,
                 canvas=7, cached_address=1, returned_address=0,
                 lock_return_epoch=0, producer_prefix=10)
    returned = dict(entry, ordinal=3, phase=2, cached_address=2,
                    returned_address=2, lock_return_epoch=1, producer_prefix=11)
    query = dict(ordinal=2, query=1, route=1, input_address=2,
                 metadata=[64, 2, 131, 10, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 18, 1])
    return dict(closed=True, events=[entry, returned], queries=[query])


class Queries(unittest.TestCase):
    def test_pending_query_equal_completed_return_is_observation_only(self):
        result = bind(fixture())
        self.assertEqual(result['bindings'][0]['canvas'], 7)
        self.assertEqual(result['bindings'][0]['lock_return_epoch'], 1)
        self.assertEqual(result['unbound'], [])
        self.assertFalse(result['source_storage_owned'])

    def test_legacy_and_null_identity_remain_unbound(self):
        for value in (None, 0):
            data = fixture()
            if value is None:
                del data['queries'][0]['input_address']
            else:
                data['queries'][0]['input_address'] = value
            result = bind(data)
            self.assertEqual(result['bindings'], [])
            self.assertEqual(result['queries_accounted'], 1)
            self.assertIs(result['unbound'][0]['input_address'], value)

    def test_wrong_owner_changed_canvas_and_unequal_address_stay_unbound(self):
        for mutation in [lambda d: d['queries'][0]['metadata'].__setitem__(14, 19),
                         lambda d: d['events'][1].__setitem__('canvas', 8),
                         lambda d: d['queries'][0].__setitem__('input_address', 3)]:
            data = fixture()
            mutation(data)
            self.assertEqual(len(bind(data)['unbound']), 1)

    def test_ambiguous_lock_and_outside_interval_never_match(self):
        data = fixture()
        second = dict(data['events'][0], ordinal=2, call=2)
        returned = dict(data['events'][1], ordinal=5, call=2)
        data['events'].extend([second, returned])
        data['queries'][0]['ordinal'] = 4
        data['events'][1]['ordinal'] = 6
        self.assertEqual(bind(data)['unbound'][0]['reason'], 'ambiguous-same-thread-lock')
        data = fixture()
        data['queries'][0]['ordinal'] = 4
        self.assertEqual(bind(data)['unbound'][0]['reason'], 'no-same-thread-lock')

    def test_stopped_failed_nonmap_and_ambiguous_points_stay_visible(self):
        for mutation in [lambda d: d['queries'][0]['metadata'].__setitem__(2, 7),
                         lambda d: d['queries'][0]['metadata'].__setitem__(5, 1),
                         lambda d: d['queries'][0].__setitem__('route', 2),
                         lambda d: d['queries'][0]['metadata'].__setitem__(2, 11)]:
            data = fixture()
            mutation(data)
            self.assertEqual(bind(data)['bindings'], [])
            self.assertEqual(len(bind(data)['unbound']), 1)

    def test_open_trace_and_inconsistent_return_cache_refuse(self):
        data = fixture()
        data['closed'] = False
        with self.assertRaises(ValueError):
            bind(data)
        data = fixture()
        data['events'][1]['cached_address'] = 3
        with self.assertRaises(ValueError):
            bind(data)


if __name__ == '__main__':
    unittest.main()
