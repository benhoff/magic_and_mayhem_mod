#!/usr/bin/env python3
import copy
import json
from pathlib import Path
import sys
import tempfile
import unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from rts_controls_config import load, chord, DEFAULT
from rts_controls_launch import enabled_from_environment, authorize, stage

class Controls(unittest.TestCase):
    def test_defaults(self):
        settings, entries = load()
        self.assertEqual(settings['bindings']['wizard'],'F1')
        self.assertEqual(chord('Ctrl+Shift+F8'),(119,3))
        self.assertEqual(len(entries),20)

    def test_invalid_settings(self):
        original = json.loads(DEFAULT.read_text())
        cases = [('schema',True),('mouse_scroll_speed',0),('double_tap_ms',1001),
                 ('ctrl_additive_selection',1),('spell_slots',{'Alt+Q':26}),
                 ('bindings',{}),('spell_slots',{'F1':0}),('spell_slots',{'1':0})]
        with tempfile.TemporaryDirectory() as d:
            p=Path(d)/'settings.json'
            for key, value in cases:
                test=copy.deepcopy(original);test[key]=value;p.write_text(json.dumps(test))
                with self.subTest(key=key,value=value),self.assertRaises(ValueError):load(p)
            p.write_text('{"schema":1,"schema":1}')
            with self.assertRaises(ValueError):load(p)

    def test_bad_chords(self):
        for s in ('Ctrl+Ctrl+T','Meta+T','Ctrl+mouse1','Alt+','a',12):
            with self.subTest(chord=s),self.assertRaises(ValueError):chord(s)

    def test_explicit_activation(self):
        self.assertFalse(enabled_from_environment({}))
        self.assertFalse(enabled_from_environment({'MNM_RTS_CONTROLS':'0'}))
        self.assertTrue(enabled_from_environment({'MNM_RTS_CONTROLS':'1'}))
        with self.assertRaises(ValueError):enabled_from_environment({'MNM_RTS_CONTROLS':'yes'})
        environment={'MNM_RTS_CONTROLS_ACTIVE':'inherited'}
        authorize(Path('/tmp'),{},environment)
        self.assertNotIn('MNM_RTS_CONTROLS_ACTIVE',environment)

    def test_unsupported_before_mutation(self):
        with tempfile.TemporaryDirectory() as d:
            root=Path(d)
            with self.assertRaises(ValueError):stage(root,b'wrong',b'wrong',True)
            self.assertEqual(list(root.iterdir()),[])
            self.assertEqual(stage(root,b'wrong',b'unchanged',False),(b'unchanged',{}))

    def test_activation_rejects_tampering(self):
        import hashlib
        with tempfile.TemporaryDirectory() as d:
            root=Path(d);(root/'game').mkdir()
            dll=root/'game/MnmRtsControls.dll';dll.write_bytes(b'fake')
            config=root/'rts-controls.json';config.write_bytes(b'fake config')
            m={'rts_controls':True,'rts_controls_dll_sha256':hashlib.sha256(dll.read_bytes()).hexdigest(),
               'rts_controls_config_sha256':hashlib.sha256(config.read_bytes()).hexdigest()}
            env={};authorize(root,m,env);self.assertEqual(env['MNM_RTS_CONTROLS_ACTIVE'],'1')
            config.write_bytes(b'changed')
            with self.assertRaises(ValueError):authorize(root,m,env)
            self.assertNotIn('MNM_RTS_CONTROLS_ACTIVE',env)

    def test_redirected_game_rejected(self):
        with tempfile.TemporaryDirectory() as d:
            root=Path(d);(root/'elsewhere').mkdir();(root/'game').symlink_to(root/'elsewhere',target_is_directory=True)
            with self.assertRaises(ValueError):authorize(root,{'rts_controls':True},{})

    def test_passive_original_observation_rejects_controls(self):
        from unittest.mock import patch
        import rts_controls_launch
        observer=rts_controls_launch.module('rts_passive_guard','tools/prepare-menu-observer.py')
        with patch.dict('os.environ',{'MNM_RTS_CONTROLS':'1'}):
            with self.assertRaisesRegex(ValueError,'passive original-gameplay'):
                observer.prepare(original_gameplay=True,campaign_observe=True)

if __name__=='__main__':unittest.main()
