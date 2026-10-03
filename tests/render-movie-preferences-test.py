#!/usr/bin/env python3
"""Check staged movie preferences and encrypted/plaintext precedence."""
import importlib.util
from pathlib import Path
import tempfile
import unittest

REPO=Path(__file__).resolve().parent.parent
spec=importlib.util.spec_from_file_location('render_stage',REPO/'tools/run-opengl-game.py')
stage=importlib.util.module_from_spec(spec);spec.loader.exec_module(stage)
encoder=stage.load('cfg_encoder','tools/encode-cfg.py');decoder=encoder.load_decoder(REPO)

class MoviePreferences(unittest.TestCase):
    def test_plain_and_encrypted(self):
        # Different contents verify both copies are edited independently.
        with tempfile.TemporaryDirectory() as directory:
            game=Path(directory);(game/'CFG/Encrypted').mkdir(parents=True)
            text=b'[OTHER]\r\nPlayFMV=TRUE\r\n[VIDEO]\r\nPlayFMV=TRUE ; intro\r\nPlayFMVOut=TRUE\r\nIsHighRes=TRUE\r\n'
            plain=game/'CFG/prefs.cfg';packed=game/'CFG/Encrypted/prefs.cfg'
            plain.write_bytes(text);packed.write_bytes(encoder.encode(text+b'; packed only\r\n',decoder)[0])
            records=stage.skip_movies(game)
            expected=text.replace(b'PlayFMV=TRUE ;',b'PlayFMV=FALSE ;').replace(b'PlayFMVOut=TRUE',b'PlayFMVOut=FALSE')
            self.assertEqual(plain.read_bytes(),expected)
            self.assertEqual(decoder.decode(packed.read_bytes())[1],expected+b'; packed only\r\n')
            self.assertEqual(len(records),2)
            self.assertTrue(all(item['before_sha256']!=item['after_sha256'] for item in records))
            stage.skip_movies(game)
            self.assertEqual(plain.read_bytes(),expected)

    def test_cd_music_both_formats(self):
        with tempfile.TemporaryDirectory() as directory:
            game=Path(directory);(game/'CFG/Encrypted').mkdir(parents=True)
            text=b'[VIDEO]\r\nPlayFMV=TRUE\r\nCDMusicEnabled=TRUE\r\n[SOUND]\r\nSoundEnabled=TRUE\r\nCDMusicEnabled=TRUE ; audio disc\r\n'
            plain=game/'CFG/prefs.cfg';packed=game/'CFG/Encrypted/prefs.cfg'
            plain.write_bytes(text);packed.write_bytes(encoder.encode(text,decoder)[0])
            records=stage.disable_cd_music(game)
            expected=text.replace(b'CDMusicEnabled=TRUE ; audio',b'CDMusicEnabled=FALSE ; audio')
            self.assertEqual(plain.read_bytes(),expected)
            self.assertEqual(decoder.decode(packed.read_bytes())[1],expected)
            self.assertEqual(len(records),2)

    def test_ambiguous_copy_prevents_all_writes(self):
        with tempfile.TemporaryDirectory() as directory:
            game=Path(directory);(game/'CFG/Encrypted').mkdir(parents=True)
            plain=game/'CFG/prefs.cfg';packed=game/'CFG/Encrypted/prefs.cfg'
            text=b'[VIDEO]\nPlayFMV=TRUE\nPlayFMVOut=TRUE\n'
            plain.write_bytes(text);bad=encoder.encode(text+b'PlayFMV=TRUE\n',decoder)[0];packed.write_bytes(bad)
            with self.assertRaises(ValueError):stage.skip_movies(game)
            self.assertEqual(plain.read_bytes(),text);self.assertEqual(packed.read_bytes(),bad)

if __name__=='__main__':unittest.main()
