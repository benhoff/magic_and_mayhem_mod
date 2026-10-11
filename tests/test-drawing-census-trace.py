#!/usr/bin/env python3
"""Journal corruption, phase, interior and paired-return contract checks."""
import copy
from pathlib import Path
import struct
import sys
import tempfile
import unittest
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
from drawing_census import BUILD_SHA256, trace


class TraceTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory(); self.addCleanup(self.tmp.cleanup)
        self.path = Path(self.tmp.name) / 'trace.bin'
        self.manifest = {'original_sha256': BUILD_SHA256, 'hooks': [
            {'tag': 0, 'entry': 0x401100, 'site': False, 'route': 'test'},
            {'tag': 1, 'entry': 0x401200, 'site': True, 'route': 'interior'}]}
        self.inventory = {'source_sha256': BUILD_SHA256, 'functions': [
            {'entry': 0x401000, 'ranges': [[0x401000, 0x401005]]}]}
        self.callers = {'schema': 1, 'source_sha256': BUILD_SHA256, 'image_base': 0x400000,
                        'input_unchanged': True, 'callers': [{'call_va': 0x401000,
                        'return_pc': 0x401005, 'entry': 0x401100, 'bytes': 'e8fb000000',
                        'discovered_owners': [0x401000]}]}
        self.header = bytearray(b'MNMDCN01' + struct.pack('<6I', 1, 64, 128, 0x400000, 2, 4194304)
                                + bytes.fromhex(BUILD_SHA256))
        self.entry = [1, 1, 1, 0, 2, 7, 0x401100, 0x401005, 0x123400] + [0] * 23
        self.leave = list(self.entry); self.leave[:2] = [2, 2]; self.leave[11] = 0xffffffff
        self.end = bytearray(b'MNMDCEND' + struct.pack('<14I', 1, 2, 1, 0, 0, 0, *([0] * 8)))

    def decode(self, rows=None):
        rows = [self.entry, self.leave] if rows is None else rows
        self.path.write_bytes(self.header + b''.join(struct.pack('<32I', *r) for r in rows) + self.end)
        return trace(self.path, self.manifest, self.callers, self.inventory)

    def test_closed_pair_preserves_result_and_does_not_close_milestone(self):
        result = self.decode()
        self.assertTrue(result['journal_integrity']); self.assertFalse(result['draw01_complete'])
        self.assertEqual(result['result_counts'][0]['result'], 0xffffffff)
        self.assertEqual(result['entry_counts'][0]['caller_attribution'], 'exact_direct_call')

    def test_each_return_identity_and_reserved_field_is_checked(self):
        for index in [0, 2, 3, 4, 5, 6, 7, 8, 29, 30, 31]:
            bad = list(self.leave); bad[index] += 1
            with self.subTest(index=index), self.assertRaises(ValueError): self.decode([self.entry, bad])

    def test_loss_open_io_count_build_and_base_refuse(self):
        for offset in [12, 16, 20, 24, 28, 32]:
            saved = self.end[:]; struct.pack_into('<I', self.end, offset, 99)
            with self.subTest(offset=offset), self.assertRaises(ValueError): self.decode()
            self.end = saved
        self.header[20] ^= 1
        with self.assertRaises(ValueError): self.decode()
        self.header[20] ^= 1; self.header[32] ^= 1
        with self.assertRaises(ValueError): self.decode()

    def test_missing_terminal_and_truncated_record_refuse(self):
        self.decode(); data = self.path.read_bytes()
        for size in [0, 63, 64, len(data) - 1, len(data) - 64]:
            self.path.write_bytes(data[:size])
            with self.subTest(size=size), self.assertRaises(ValueError):
                trace(self.path, self.manifest, self.callers, self.inventory)

    def test_interior_site_has_no_invented_caller(self):
        row = list(self.entry); row[1:5] = [3, 1, 1, 14]; row[6] = 0x401200; row[7] = 0
        struct.pack_into('<I', self.end, 12, 1)
        result = self.decode([row]); self.assertEqual(result['site_counts'][0]['caller_attribution'],
                                                      'interior_site_no_return_caller')
        self.assertEqual((result['calls'], result['paired_calls'], result['site_events']), (1, 0, 1))
        self.assertEqual(result['site_eax_counts'], [{'phase': 14, 'entry': 0x401200, 'eax': 0, 'count': 1}])
        self.assertEqual(result['result_counts'], [])
        row[7] = 0x401005
        with self.assertRaises(ValueError): self.decode([row])

    def test_mixed_sites_keep_call_counter_and_results_distinct(self):
        site = list(self.entry); site[:5] = [2, 3, 2, 1, 2]
        site[6] = 0x401200; site[7] = 0; site[11] = 0x80004005
        leave = list(self.leave); leave[0] = 3
        struct.pack_into('<II', self.end, 12, 3, 2)
        result = self.decode([self.entry, site, leave])
        self.assertEqual((result['calls'], result['paired_calls'], result['site_events']), (2, 1, 1))
        self.assertEqual(result['site_eax_counts'][0]['eax'], 0x80004005)
        self.assertEqual(result['result_counts'][0]['result'], 0xffffffff)

    def test_unknown_and_unassigned_remain_explicit(self):
        self.entry[4] = self.leave[4] = 0; self.entry[7] = self.leave[7] = 0x410005
        result = self.decode(); self.assertEqual(result['unassigned_phase_calls'], 1)
        self.assertEqual(result['caller_pcs_without_discovered_owner'], [0x410005])
        self.assertEqual(result['entry_counts'][0]['caller_owners'], [])

    def test_manifest_and_static_caller_mismatch_refuse(self):
        self.manifest['hooks'].append(copy.deepcopy(self.manifest['hooks'][0]))
        with self.assertRaises(ValueError): self.decode()
        self.manifest['hooks'].pop(); self.callers['source_sha256'] = 'wrong'
        with self.assertRaises(ValueError): self.decode()


if __name__ == '__main__': unittest.main()
