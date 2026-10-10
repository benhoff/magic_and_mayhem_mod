#!/usr/bin/env python3
"""Strict staged render-config edits preserve game-speed and unrelated bytes."""
import sys
import tempfile
import unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from native_render_config import without_draw_skipping,stage_native_draw_cadence

TEXT=b'[VIDEO]\r\nMaxFramesPerSec=17\r\n[DEBUG]\r\nMaxFramesPerSec = 20\r\nSkipFrameEvery=1 ; draw\r\nSkipXFrames=1\r\nMaxSkipXFrames=1\r\n[OTHER]\r\nSkipFrameEvery=5\r\n'


class NativeConfigTests(unittest.TestCase):
    def test_only_render_skipping_changes_and_edit_is_idempotent(self):
        changed=without_draw_skipping(TEXT)
        expected=TEXT.replace(b'SkipFrameEvery=1',b'SkipFrameEvery=0').replace(b'SkipXFrames=1',b'SkipXFrames=0')
        self.assertEqual(changed,expected)
        self.assertEqual(without_draw_skipping(changed),changed)

    def test_missing_duplicate_wrong_section_and_malformed_settings_refuse(self):
        for text in [TEXT.replace(b'SkipXFrames=1\r\n',b''),TEXT.replace(b'[DEBUG]',b'[WRONG]'),
                     TEXT.replace(b'SkipXFrames=1',b'SkipXFrames=no'),TEXT.replace(b'SkipXFrames=1',b'SkipXFrames=1\r\nSkipXFrames=2')]:
            with self.assertRaises(ValueError):without_draw_skipping(text)

    def test_both_precedence_copies_validated_before_any_write(self):
        with tempfile.TemporaryDirectory() as temporary:
            game=Path(temporary);(game/'CFG/Encrypted').mkdir(parents=True)
            plain=game/'CFG/chaos.cfg';packed=game/'CFG/Encrypted/chaos.cfg'
            plain.write_bytes(TEXT);packed.write_bytes(b'packed:'+TEXT.replace(b'[DEBUG]',b'[WRONG]'))
            encode=lambda b:b'packed:'+b;decode=lambda b:b.removeprefix(b'packed:')
            with self.assertRaises(ValueError):stage_native_draw_cadence(game,encode,decode)
            self.assertEqual(plain.read_bytes(),TEXT)
            packed.write_bytes(encode(TEXT));report=stage_native_draw_cadence(game,encode,decode)
            self.assertEqual(decode(packed.read_bytes()),plain.read_bytes())
            self.assertEqual(len(report['files']),2)
            for file in report['files']:self.assertNotEqual(file['before_sha256'],file['after_sha256'])
            with self.assertRaises(ValueError):stage_native_draw_cadence(game/'original',encode,decode)


if __name__=='__main__':unittest.main()
