#!/usr/bin/env python3
"""Check independent presentation expectations and isolated input selection."""
import os
from pathlib import Path
import sys
import unittest
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
from terrain_validation_inputs import terrain_executable
from terrain_validation_pixels import rgb565_rgba


class TerrainValidationTest(unittest.TestCase):
    def test_presentation_matches_repeated_channel_bits_for_every_word(self):
        def expand(value, width):
            bits = format(value, '0' + str(width) + 'b')
            return int((bits * 2)[:8], 2)
        expected = bytes(channel for value in range(65536)
                         for channel in (expand(value >> 11, 5),
                                         expand((value >> 5) & 63, 6),
                                         expand(value & 31, 5), 255))
        self.assertEqual(rgb565_rgba(range(65536)), expected)

    def test_previous_floor_model_is_detected_without_changing_native_words(self):
        word = (4 << 11) | (16 << 5) | 4
        words = [0, word, 65535]
        self.assertEqual(rgb565_rgba(words), bytes((0, 0, 0, 255, 33, 65, 33, 255,
                                                   255, 255, 255, 255)))
        self.assertNotEqual(rgb565_rgba([word]), bytes((4 * 255 // 31,
                                                       16 * 255 // 63,
                                                       4 * 255 // 31, 255)))
        self.assertEqual(words, [0, word, 65535])

    def test_missing_override_keeps_existing_launcher_selection(self):
        with patch.dict(os.environ, {}, clear=True):
            self.assertEqual(terrain_executable(Path('/checkout')),
                             Path('/checkout/working/game-nocd/Chaos.exe'))

    def test_explicit_input_does_not_replace_launcher_path(self):
        with patch.dict(os.environ, {'MNM_TERRAIN_EXECUTABLE': '/tmp/owned-reference.exe'}):
            self.assertEqual(terrain_executable(Path('/checkout')),
                             Path('/tmp/owned-reference.exe'))


if __name__ == '__main__':
    unittest.main()
