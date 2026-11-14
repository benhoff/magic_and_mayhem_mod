#!/usr/bin/env python3
"""Current native provenance never follows from a historical host-only join."""
import tempfile
from pathlib import Path
import sys
import unittest
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT/'tools'))
from broader_player_acceptance import fingerprints, native_phase


class Provenance(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.names = {'runtime/factory.c': 'runtime-current', 'renderer/draw.cpp': 'draw-current'}
        for name in self.names:
            p = self.root/name
            p.parent.mkdir(parents=True, exist_ok=True)
            p.touch()
        self.reader = type('Reader', (), {'digest': lambda _, n: self.names[n]})()
        self.acquired = dict(sources=dict(self.names), player_binary=dict(sha256='current-binary'))
        self.guard = patch('broader_player_acceptance.native_runtime_source_paths',
                           return_value={'runtime/factory.c'})
        self.guard.start()
        self.addCleanup(self.guard.stop)

    def inspect(self):
        return native_phase(self.root, self.reader, self.acquired, 'current-binary')

    def test_complete_source_is_only_a_candidate(self):
        result = self.inspect()
        self.assertTrue(result['current_native_source_candidate'])
        self.assertFalse(result['current_application_acceptance_established'])

    def test_unrecorded_runtime_file_is_not_inferred(self):
        del self.acquired['sources']['runtime/factory.c']
        result = self.inspect()
        self.assertFalse(result['current_native_source_candidate'])
        self.assertEqual(result['missing_runtime_sources'], ['runtime/factory.c'])

    def test_inactive_native_branch_still_requires_full_hash(self):
        self.acquired['sources']['renderer/draw.cpp'] = 'old-native-byte-hash'
        result = self.inspect()
        self.assertFalse(result['current_native_source_candidate'])
        self.assertEqual(result['changed_native_sources']['renderer/draw.cpp']['current'], 'draw-current')

    def test_different_executable_is_not_current_tested_binary(self):
        self.acquired['player_binary']['sha256'] = 'historical-binary'
        self.assertFalse(self.inspect()['current_native_source_candidate'])

    def test_conflicting_compiler_preimage_refused(self):
        self.acquired['compiler_sources'] = {'runtime/factory.c': 'different'}
        with self.assertRaisesRegex(ValueError, 'Conflicting'):
            fingerprints(self.acquired)

    def test_empty_provenance_cannot_be_current(self):
        self.acquired['sources'] = {}
        self.assertFalse(self.inspect()['current_native_source_candidate'])


if __name__ == '__main__':
    unittest.main()
