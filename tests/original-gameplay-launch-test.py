#!/usr/bin/env python3
"""Execute the runner's actual admission block without launching Wine."""
import ast
import importlib.util
import os
from pathlib import Path
import tempfile
from types import SimpleNamespace
import unittest

ROOT = Path(__file__).resolve().parents[1]
tree = ast.parse((ROOT / 'tools/menu-game-runner.py').read_text())
start = next(i for i, n in enumerate(tree.body) if isinstance(n, ast.Assign)
             and any(isinstance(t, ast.Name) and t.id == 'original_gameplay' for t in n.targets))
end = next(i for i in range(start, len(tree.body)) if isinstance(tree.body[i], ast.If)
           and ast.unparse(tree.body[i].test) == "os.environ.get('MNM_MENU_NATIVE_RENDER') == '1'")
POLICY = compile(ast.Module(body=tree.body[start:end], type_ignores=[]), 'actual-runner-admission', 'exec')

class Admission(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.root = Path(self.tmp.name)
        self.meta = dict(original_gameplay=True, origin='menu_observation_only', campaign_observe=True,
                         native_render=False, native_draw=False, native_voices=False)
        self.env = dict(MNM_MENU_ORIGINAL_GAMEPLAY='1', MNM_MENU_NATIVE_RENDER='0',
                        MNM_MENU_CAMPAIGN_OBSERVE='Z:' + str(self.root / 'campaign.bin').replace('/', '\\'))

    def tearDown(self):
        self.tmp.cleanup()

    def execute(self):
        exec(POLICY, dict(metadata=self.meta, root=self.root,
                         native_draw=self.meta.get('native_draw', False),
                         os=SimpleNamespace(environ=self.env)))

    def test_original_outputs(self):
        self.execute()
        for key, name in [('MNM_MENU_GAMEPLAY_OBSERVE', 'gameplay-events.bin'),
                          ('MNM_MENU_SPELL_OBSERVE', 'spell-events.bin')]:
            self.assertEqual(self.env[key], 'Z:' + str(self.root / name).replace('/', '\\'))

    def test_profile_refusals(self):
        for key, value in [('original_gameplay', 1), ('original_gameplay', '1'),
                           ('origin', 'menu_action_bridge'), ('campaign_observe', False), ('campaign_observe', '1'),
                           ('native_render', True), ('native_draw', True), ('native_voices', True)]:
            with self.subTest(key=key, value=value):
                old = self.meta[key]; self.meta[key] = value
                with self.assertRaises(ValueError): self.execute()
                self.meta[key] = old

    def test_environment_refusals(self):
        for key, value in [('MNM_MENU_ORIGINAL_GAMEPLAY', '0'), ('MNM_MENU_CHANNEL', 'unapproved'),
                           ('MNM_MENU_NATIVE_RENDER', '1'), ('MNM_CAMPAIGN_CASTING_COMBAT', '1'),
                           ('MNM_MENU_CAMPAIGN_OBSERVE', 'redirected')]:
            with self.subTest(key=key):
                old = dict(self.env); self.env[key] = value
                with self.assertRaises(ValueError): self.execute()
                self.env = old

    def test_unstaged_request(self):
        self.meta['original_gameplay'] = False
        with self.assertRaises(ValueError): self.execute()

    def test_stale_and_redirected_outputs(self):
        for name in ['gameplay-events.bin', 'spell-events.bin']:
            p = self.root / name
            with self.subTest(name=name, kind='existing'):
                p.write_bytes(b'old evidence')
                with self.assertRaises(ValueError): self.execute()
                p.unlink()
            with self.subTest(name=name, kind='broken symlink'):
                p.symlink_to(self.root / 'missing')
                with self.assertRaises(ValueError): self.execute()
                p.unlink()

    def test_native_route_preserved(self):
        self.meta['original_gameplay'] = False
        self.meta['native_render'] = True
        self.env = dict(MNM_CAMPAIGN_CASTING_COMBAT='1')
        self.execute()
        self.assertIn('MNM_MENU_GAMEPLAY_OBSERVE', self.env)

    def test_prepare_refuses_before_build(self):
        spec = importlib.util.spec_from_file_location('prepare', ROOT / 'tools/prepare-menu-observer.py')
        m = importlib.util.module_from_spec(spec); spec.loader.exec_module(m)
        for flags in [dict(), dict(campaign_observe=True, actions=True),
                      dict(campaign_observe=True, native_render=True),
                      dict(campaign_observe=True, native_draw=True),
                      dict(campaign_observe=True, resident_world=True),
                      dict(campaign_observe=True, native_voices=True)]:
            with self.subTest(flags=flags):
                with self.assertRaises(ValueError): m.prepare(original_gameplay=True, **flags)

if __name__ == '__main__':
    unittest.main()
