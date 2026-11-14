"""Include module initialization and constant dynamic controller dependencies."""
import sys,tempfile,unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from broader_player_sources import python_sources

class Closure(unittest.TestCase):
    def test_import_dynamic_conditional_and_loader_sources(self):
        with tempfile.TemporaryDirectory()as d:
            root=Path(d);(root/'tools').mkdir();(root/'protocols/python').mkdir(parents=True)
            files={'tools/start.py':"import shared\nfrom wire import X\nloader.module('normal-cleanup')\nfilename='low.py'if X else'high.py'\n",'tools/shared.py':'import nested\n','tools/nested.py':'VALUE=1\n','tools/normal-cleanup.py':'import nested\n','tools/low.py':'','tools/high.py':'','protocols/python/wire.py':'X=1\n','tools/unrelated.py':''}
            for n,body in files.items():(root/n).write_text(body)
            self.assertEqual(python_sources(root,['tools/start.py']),set(files)-{'tools/unrelated.py'})
    def test_missing_or_escaping_seed_refuses(self):
        with tempfile.TemporaryDirectory()as d:
            for seed in('missing.py','../outside.py'):
                with self.subTest(seed=seed),self.assertRaises(ValueError):python_sources(Path(d),[seed])
    def test_current_world_loader_includes_real_cleanup(self):
        root=Path(__file__).resolve().parents[1]
        sources=python_sources(root,['tools/run-broader-world.py','tools/run-rendering-acceptance-pilot.py'])
        self.assertIn('tools/broader_world_launch.py',sources)
        self.assertIn('tools/run-native-owned.py',sources)
        self.assertIn('tools/test-native-campaign.py',sources)

if __name__=='__main__':unittest.main()
