#!/usr/bin/env python3
"""Nominal observation cannot select faults, skip intros or change Qt actions."""
from pathlib import Path
import subprocess,sys,unittest
ROOT=Path(__file__).resolve().parents[1]
class CensusPolicy(unittest.TestCase):
 def test_diagnostic_or_non_normal_combinations_refuse_before_execution(self):
  for arguments in [[],['--scenario','pilot'],['--scenario','normal-quick','--operations-provenance'],['--scenario','normal-storage','--skip-intros'],['--scenario','normal-quick','--diagnostic-only'],['--scenario','normal-quick','--sample-stalls'],['--scenario','normal-quick','--stall-intro-host'],['--scenario','normal-quick','--stall-world-host']]:
   with self.subTest(arguments=arguments):
    run=subprocess.run([sys.executable,'-B',str(ROOT/'tools/run-rendering-acceptance-pilot.py'),'--report','research/runtime/rendering-normal-results-20261031/unexecuted-ep06-policy-test.json','--operation-census',*arguments],capture_output=True,text=True,timeout=10)
    self.assertEqual(run.returncode,2);self.assertIn('Operation census requires an unchanged nominal normal route',run.stderr)
if __name__=='__main__':unittest.main()
