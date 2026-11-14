#!/usr/bin/env python3
"""Only the reviewed caller analysis arguments may be substituted."""
import copy,sys,unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from broader_runtime_caller_commands import analysis_command

def fixture():
 old=['python3','tools/compare-broader-caller.py','--acquisition','first-acq','--report','first-out','--claims','old-claims','--requirements','old-requirements','--model-build','exact-model','--history-build','missing-history']
 new=list(old);new[1]='tools/compare-broader-runtime-caller-evidence.py'
 for flag,value in [('--claims','new-claims'),('--requirements','new-requirements'),('--history-build','retained-history')]:new[new.index(flag)+1]=value
 command=list(old);command[3]='other-acq';command[5]='other-out';return dict(report='other-acq',comparison='other-out',comparison_command=command),dict(original_comparison_command=old,comparison_command=new)
class Commands(unittest.TestCase):
 def test_only_script_output_and_three_reviewed_inputs_change(self):
  case,review=fixture();before=copy.deepcopy((case,review));cmd=analysis_command(case,review,'fresh-output');self.assertEqual(cmd[3],'other-acq');self.assertEqual(cmd[5],'fresh-output');self.assertEqual(cmd[cmd.index('--model-build')+1],'exact-model');self.assertEqual((case,review),before)
 def test_foreign_or_missing_script_refused(self):
  case,review=fixture();case['comparison_command'][1]='wrong'
  with self.assertRaises(ValueError):analysis_command(case,review,'out')
 def test_duplicate_analysis_flag_refused(self):
  case,review=fixture();case['comparison_command']+=['--claims','extra']
  with self.assertRaises(ValueError):analysis_command(case,review,'out')
 def test_foreign_acquisition_or_output_refused(self):
  for flag in ['--acquisition','--report']:
   case,review=fixture();case['comparison_command'][case['comparison_command'].index(flag)+1]='foreign'
   with self.assertRaises(ValueError):analysis_command(case,review,'out')
 def test_unreviewed_predecessor_input_refused(self):
  for flag in ['--claims','--requirements','--history-build']:
   case,review=fixture();case['comparison_command'][case['comparison_command'].index(flag)+1]='unreviewed'
   with self.assertRaises(ValueError):analysis_command(case,review,'out')
if __name__=='__main__':unittest.main()
