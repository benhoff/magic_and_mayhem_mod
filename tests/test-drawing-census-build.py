#!/usr/bin/env python3
"""Interior probes must never silently become function-return hooks."""
import importlib.util
from pathlib import Path
import sys
import unittest
ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
spec = importlib.util.spec_from_file_location('census_build', ROOT / 'tools/build-drawing-census.py')
module = importlib.util.module_from_spec(spec); spec.loader.exec_module(module)


class SiteModeTests(unittest.TestCase):
    def setUp(self):
        self.inventory = {'functions': [{'entry': 0x401000, 'ranges': [[0x401000, 0x401100]]}]}
        self.route = {'observation_mode': 'site', 'original_anchor': 0x401000}

    def test_explicit_containing_anchor_and_exclusive_end(self):
        self.assertTrue(module.site_mode(self.route, 0x401010, self.inventory))
        for entry in (0x400fff, 0x401100):
            with self.subTest(entry=entry), self.assertRaises(ValueError):
                module.site_mode(self.route, entry, self.inventory)

    def test_missing_unknown_or_boolean_anchor_refuses(self):
        for anchor in (None, True, 0x400000, '0x401000'):
            with self.subTest(anchor=anchor), self.assertRaises(ValueError):
                module.site_mode({'observation_mode': 'site', 'original_anchor': anchor},
                                 0x401010, self.inventory)

    def test_unknown_mode_and_legacy_return_slot_refuse(self):
        for mode in ('auto', True, None, 1):
            with self.subTest(mode=mode), self.assertRaises(ValueError):
                module.site_mode({'observation_mode': mode}, 0x401010, self.inventory)
        with self.assertRaises(ValueError):
            module.site_mode({'observation_mode': 'entry'}, 0x58d3b0, self.inventory)
        self.assertTrue(module.site_mode({}, 0x58d3b0, self.inventory))
        self.assertFalse(module.site_mode({}, 0x401000, self.inventory))


if __name__ == '__main__': unittest.main()
