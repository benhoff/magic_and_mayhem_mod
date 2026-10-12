#!/usr/bin/env python3
"""Native menu audio admission and evidence guards, without original assets."""
import hashlib
import importlib.util
from pathlib import Path
import struct
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from native_audio_launch import SIZE, VERSION, authorize, validate_activity, validate_retirement


class AudioLaunchTest(unittest.TestCase):
    def setUp(self):
        self.directory = tempfile.TemporaryDirectory()
        self.addCleanup(self.directory.cleanup)
        self.root = Path(self.directory.name)
        (self.root / 'game').mkdir()
        self.dll = self.root / 'game/MnmAudio.dll'
        self.dll.write_bytes(b'fixture DLL')
        self.path = self.root / 'native-voices.bin'
        self.words = [0] * 32
        self.words[2:4] = [VERSION, SIZE]
        self.words[19:21] = [27, 1]
        self.words[31]=14
        self.metadata = dict(native_voices=True, native_render=True, origin='menu_action_bridge',
                             audio_dll_sha256=hashlib.sha256(self.dll.read_bytes()).hexdigest(),
                             audio_channel=str(self.path),native_extended_controls=True,native_output_recovery=True)
        self.write_channel()

    def write_channel(self):
        with self.path.open('wb') as output:
            output.write(b'MNMAUD01' + struct.pack('<30I', *self.words[2:]))
            output.truncate(SIZE)

    def test_ready_channel_and_inherited_environment(self):
        env = {'MNM_AUDIO_CHANNEL': 'unapproved inherited path'}
        authorize(self.root, self.metadata, env)
        self.assertEqual(env['MNM_AUDIO_CHANNEL'], 'Z:' + str(self.path).replace('/', '\\'))
        authorize(self.root, {}, env)
        self.assertNotIn('MNM_AUDIO_CHANNEL', env)

    def test_native_wave_capability_is_bound_to_staging(self):
        self.words[31]=15;self.write_channel()
        with self.assertRaises(ValueError):authorize(self.root,self.metadata,{})
        authorize(self.root,{**self.metadata,'native_wave_loading':True},{})
        self.words[31]=0x1000f;self.write_channel()
        with self.assertRaises(ValueError):authorize(self.root,{**self.metadata,'native_wave_loading':True},{})

    def test_native_factory_disables_wine_directsound(self):
        env={'WINEDLLOVERRIDES':'example=n','MNM_NATIVE_DISABLE_DSOUND':'1'}
        authorize(self.root,{**self.metadata,'native_audio_factory':True,'native_media':True},env)
        self.assertEqual(env['WINEDLLOVERRIDES'],'example=n;dsound=')
        with self.assertRaises(ValueError):authorize(self.root,{**self.metadata,'native_audio_factory':1},{})

    def test_header_failures(self):
        for word, value in [(2, 1), (3, 1), (20, 0)] + [(i, 1) for i in list(range(4, 19)) + list(range(21, 32))]:
            with self.subTest(word=word):
                before = self.words[word]
                self.words[word] = value
                self.write_channel()
                with self.assertRaises(ValueError):authorize(self.root, self.metadata, {})
                self.words[word] = before

    def test_staging_failures(self):
        for key, value in [('native_voices', 1), ('native_render', False), ('origin', 'menu_observation_only'),
                           ('native_extended_controls',False),('native_output_recovery',False),
                           ('audio_dll_sha256', 'wrong'), ('audio_channel', str(self.root / 'other.bin'))]:
            with self.subTest(key=key), self.assertRaises(ValueError):
                authorize(self.root, {**self.metadata, key: value}, {})
        self.path.write_bytes(b'MNMAUD01')
        with self.assertRaises(ValueError):authorize(self.root, self.metadata, {})

    def test_symlink_channel_refused(self):
        other = self.root / 'other.bin'
        self.path.rename(other)
        self.path.symlink_to(other)
        with self.assertRaises(ValueError):authorize(self.root, self.metadata, {})

    def test_native_staging_requires_render_actions(self):
        spec = importlib.util.spec_from_file_location('menu_stage', ROOT / 'tools/prepare-menu-observer.py')
        stage = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(stage)
        for actions, render in [(False, False), (True, False), (False, True)]:
            with self.assertRaises(ValueError):stage.prepare(actions=actions, native_render=render, native_voices=True)

    def test_retirement_requires_the_actual_final_channel(self):
        self.words[4] = self.words[16] = 42
        self.words[20] = 0
        self.words[21] = 1
        report = dict(native_selections=1, fallbacks=0, adapter_failures=0)
        self.write_channel()
        self.assertEqual(validate_retirement(self.path, report)['acknowledgement'], 42)
        for word, value in [(2, 1), (3, 1), (4, 0), (16, 41), (20, 1), (21, 2), (22, 1), (23, 1)]:
            with self.subTest(word=word):
                previous = self.words[word]
                self.words[word] = value
                self.write_channel()
                with self.assertRaises(ValueError):validate_retirement(self.path, report)
                self.words[word] = previous

    def test_activity_cannot_claim_silence_fallback_or_failed_output(self):
        report = {name: 1 for name in ('requests', 'creates', 'uploads', 'uploaded_bytes', 'plays', 'releases', 'native_selections')}
        report.update(fallbacks=0, adapter_failures=0, failed_requests=0, audible_output=True,
                      ready_before_stop=True, ready_after_stop=False, error='',output_available=True,
                      output_losses=0,output_recoveries=0,output_attempts=1,generated_frames=100,consumed_frames=50)
        self.assertEqual(validate_activity(report), report)
        changes = [(name, 0) for name in ('requests', 'creates', 'uploads', 'uploaded_bytes', 'plays', 'releases', 'native_selections')]
        changes += [(name, 1) for name in ('fallbacks', 'adapter_failures', 'failed_requests')]
        changes += [('output_available',False),('output_attempts',0),('consumed_frames',0),('consumed_frames',101),('output_recoveries',1)]
        changes += [('audible_output', False), ('ready_before_stop', False), ('ready_after_stop', True), ('error', 'device lost')]
        for name, value in changes:
            with self.subTest(name=name), self.assertRaises(ValueError):validate_activity({**report, name: value})


if __name__ == '__main__':unittest.main()
