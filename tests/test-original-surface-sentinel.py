#!/usr/bin/env python3
"""Meaningful trace/provenance and loader boundary regression checks, fully offline."""
import copy
import importlib.util
import json
import tempfile
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('sentinel', ROOT/'tools/check-original-surface-sentinel.py')
s = importlib.util.module_from_spec(spec)
spec.loader.exec_module(s)


class SentinelTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.payload, cls.meta = s.load()

    def row(self, **fields):
        return next(r for r in self.payload['rows'] if all(r['input'][k] == v for k, v in fields.items()))

    def test_all_original_traces(self):
        self.assertEqual(s.compare(self.payload)['cases'], self.meta['cases'])

    def test_sentinel_ignores_missing_asset_failure(self):
        out = self.row(entry=0x4a2f90, asset='missing')['original']
        self.assertEqual(out['return_value'], 0)
        self.assertEqual(len(out['events']), 3)
        self.assertEqual(self.row(entry=0x58d1a0, asset='missing')['original']['return_value'], 0)

    def test_null_global_still_loads_and_returns_success(self):
        out = self.row(entry=0x58d1a0, asset='indexed8', global_surface=0)['original']
        self.assertEqual(out['return_value'], 1)
        self.assertNotIn('get_dc', [e['kind'] for e in out['events']])
        self.assertEqual([e['bytes'] for e in out['events'] if e['kind'] == 'free'], [1064, 48])

    def test_driver_failures_do_not_change_loader_return(self):
        for name in ['get_dc', 'release_dc', 'dib_result']:
            value = 0x80004005 if name != 'dib_result' else 0xffffffff
            out = self.row(entry=0x58d1a0, asset='indexed8', global_surface=1, **{name: value})['original']
            self.assertEqual(out['return_value'], 1)
            self.assertIn('release_dc', [e['kind'] for e in out['events']])

    def test_truncated_and_allocation_failures_cleanup(self):
        for name in ['short-file', 'short-info', 'short-palette', 'short-pixels', 'bad-signature']:
            out = self.row(entry=0x58d1a0, asset=name)['original']
            self.assertEqual(out['return_value'], 0)
            self.assertIn('close', [e['kind'] for e in out['events']])
            self.assertNotIn('dib', [e['kind'] for e in out['events']])
        out = self.row(entry=0x58d1a0, allocation_failure=1)['original']
        self.assertEqual(out['return_value'], 0)
        self.assertNotIn('free', [e['kind'] for e in out['events']])

    def test_offset_gap_is_read_sequentially(self):
        row = self.row(entry=0x58d1a0, asset='offset-gap', global_surface=1)
        dib = next(e for e in row['original']['events'] if e['kind'] == 'dib')
        asset = s.base64.b64decode(self.payload['assets']['offset-gap'])
        self.assertEqual(dib['pixels_sha256'], s.hashlib.sha256(asset[1078:1078+48]).hexdigest())
        self.assertNotEqual(dib['pixels_sha256'], s.hashlib.sha256(asset[1086:]).hexdigest())

    def test_both_reloads_target_global_cursor(self):
        out = self.row(entry=0x58c4a0, source_reload=2, destination_reload=2,
                       source_restore=0, destination_restore=0, draw_results=[s.LOST, 0])['original']
        self.assertEqual([e['surface'] for e in out['events'] if e['kind'] == 'cursor_reload'], [3, 3])
        self.assertEqual([e['surface'] for e in out['events'] if e['kind'] == 'restore'], [1, 2])

    def test_tampered_destination_and_call_order_refused(self):
        for mutation in ['destination', 'order']:
            payload = copy.deepcopy(self.payload)
            row = next(r for r in payload['rows'] if r['input']['entry'] == 0x4a2f90 and r['input']['asset'] == 'indexed8' and r['input']['global_surface'])
            events = row['original']['events']
            if mutation == 'destination':
                events[0]['surface'] = 2
            else:
                events[0], events[1] = events[1], events[0]
            with self.assertRaisesRegex(ValueError, 'Original trace differs'):
                s.compare(payload)

    def test_loader_return_tampering_refused(self):
        payload = copy.deepcopy(self.payload)
        payload['rows'][0]['original']['return_value'] = 1
        with self.assertRaisesRegex(ValueError, 'Original trace differs'):
            s.compare(payload)

    def test_untrusted_catalog_refused(self):
        with tempfile.TemporaryDirectory() as folder:
            path = Path(folder)/'catalog.json'
            for fields in [dict(original_sha256='0'*64), dict(path='../original-sentinel.json.gz'), dict(oracle='native')]:
                path.write_text(json.dumps(dict(self.meta, **fields)))
                with self.assertRaises(ValueError):
                    s.load(path)


if __name__ == '__main__':
    unittest.main()
