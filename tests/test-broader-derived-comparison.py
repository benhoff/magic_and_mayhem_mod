#!/usr/bin/env python3
import importlib.util,sys,unittest
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools'));s=importlib.util.spec_from_file_location('derived_handoff',ROOT/'tools/retire-broader-derived-comparison.py');m=importlib.util.module_from_spec(s);s.loader.exec_module(m)
class TargetControls(unittest.TestCase):
 def test_both_exact_owned_namespaces(self):
  for policy,parent in m.PARENTS.items():self.assertEqual(m.target('research/current.json',dict(comparison_storage=dict(path=str(parent/'current'),temporary=True,policy=policy))),parent/'current')
 def test_unknown_non_temporary_foreign_and_wrong_identity_refused(self):
  policy,parent=next(iter(m.PARENTS.items()));good=dict(path=str(parent/'current'),temporary=True,policy=policy)
  for patch in[dict(temporary=False),dict(policy='other'),dict(path='/tmp/foreign/current'),dict(path=str(parent/'different'))]:
   with self.assertRaises(ValueError):m.target('research/current.json',dict(comparison_storage=good|patch))
if __name__=='__main__':unittest.main()
