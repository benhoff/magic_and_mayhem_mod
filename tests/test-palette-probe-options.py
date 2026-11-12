#!/usr/bin/env python3
"""Real argparse preflight for the exact planned acquisition, with no game."""
import importlib.util,json,subprocess,sys,unittest
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools'))
spec=importlib.util.spec_from_file_location('palette_probe',ROOT/'tools/run-palette-observation-probe.py');probe=importlib.util.module_from_spec(spec);spec.loader.exec_module(probe)
class Controls(unittest.TestCase):
 def run_options(self,args):return subprocess.run([*probe.probe_command(),*args,'--validate-only'],capture_output=True,timeout=10)
 def test_exact_planned_command_validates_without_display_or_original(self):
  q=self.run_options([]);self.assertEqual(q.returncode,0,q.stderr);self.assertEqual(json.loads(q.stdout)['original_game_runs'],0)
 def test_old_incompatible_budget_option_is_rejected(self):
  q=self.run_options(['--capture-seconds','60']);self.assertEqual(q.returncode,2);self.assertIn(b'requires finite original-active World',q.stderr)
 def test_palette_requires_closed_producer(self):
  q=subprocess.run([sys.executable,ROOT/'tools/capture-scene-game.py','--palette-observation','--validate-only'],capture_output=True,timeout=10);self.assertEqual(q.returncode,2);self.assertIn(b'Palette observation requires',q.stderr)
if __name__=='__main__':unittest.main()
