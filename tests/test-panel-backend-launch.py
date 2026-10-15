#!/usr/bin/env python3
"""Exercise native panel session admission without game media or Wine."""
import hashlib
from pathlib import Path
import sys
import tempfile
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1]/'tools'))
from panel_backend_launch import authorize, enabled_from_environment, stage


class PanelLaunchTests(unittest.TestCase):
    def setUp(self):
        self.storage = tempfile.TemporaryDirectory()
        self.addCleanup(self.storage.cleanup)
        self.root = Path(self.storage.name)
        (self.root/'game').mkdir()
        self.dll = self.root/'game/MnmPanelWriters.dll'
        self.dll.write_bytes(b'owned fixture DLL')
        self.metadata = dict(native_panel_takeover=True, native_draw=True, native_render=True,
                             origin='menu_action_bridge', panel_dll_sha256=hashlib.sha256(self.dll.read_bytes()).hexdigest())

    def test_explicit_selection(self):
        self.assertFalse(enabled_from_environment({}))
        self.assertFalse(enabled_from_environment({'MNM_PANEL_TAKEOVER':'0'}))
        self.assertTrue(enabled_from_environment({'MNM_PANEL_TAKEOVER':'1'}))
        for value in ('', '11', 'takeover', 1, True):
            with self.assertRaises(ValueError):
                enabled_from_environment({'MNM_PANEL_TAKEOVER':value})

    def test_disabled_clears_inherited_mode(self):
        env = {'MNM_PANEL_TAKEOVER':'1'}
        authorize(self.root, {}, env)
        self.assertNotIn('MNM_PANEL_TAKEOVER', env)
        self.assertEqual(stage(self.root, b'', b'unchanged', False), (b'unchanged', {}))

    def test_exact_owned_session(self):
        env = {'MNM_PANEL_TAKEOVER':'bad inherited mode'}
        authorize(self.root, self.metadata, env)
        self.assertEqual(env['MNM_PANEL_TAKEOVER'], '1')

    def test_invalid_policy_and_required_hosting(self):
        for key, value in (('native_panel_takeover',1), ('native_panel_takeover','1'),
                           ('native_draw',False), ('native_draw',1), ('native_render',False),
                           ('origin','menu_observation_only')):
            env = {'MNM_PANEL_TAKEOVER':'1'}
            with self.assertRaises(ValueError):
                authorize(self.root, {**self.metadata, key:value}, env)
            self.assertNotIn('MNM_PANEL_TAKEOVER', env)

    def test_changed_or_unbound_dll(self):
        for metadata in ({**self.metadata, 'panel_dll_sha256':'0'*64},
                         {k:v for k,v in self.metadata.items() if k!='panel_dll_sha256'}):
            with self.assertRaises(ValueError):
                authorize(self.root, metadata, {})
        self.dll.write_bytes(b'changed')
        with self.assertRaises(ValueError):
            authorize(self.root, self.metadata, {})

    def test_symlinked_dll_refused(self):
        destination = self.root/'other.dll'
        destination.write_bytes(self.dll.read_bytes())
        self.dll.unlink();self.dll.symlink_to(destination)
        with self.assertRaises(ValueError):
            authorize(self.root, self.metadata, {})

    def test_symlinked_game_refused(self):
        game = self.root/'game';game.rename(self.root/'other-game');game.symlink_to(self.root/'other-game')
        with self.assertRaises(ValueError):
            authorize(self.root, self.metadata, {})

    def test_staging_rejects_unpinned_source(self):
        with self.assertRaises(ValueError):
            stage(self.root, b'not the original', b'', True)
        with self.assertRaises(ValueError):
            stage(self.root, b'', b'', '1')


if __name__ == '__main__':
    unittest.main()
