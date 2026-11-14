#!/usr/bin/env python3
"""Require exact prior checks and only named successor receipt substitutions."""
import sys,unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
import broader_strict_provenance_final_gate as g
import broader_player_strict_provenance_matrix as matrix
from broader_player_copy_matrix import current as original_current
class Preservation(unittest.TestCase):
 def test_receipts_only(self):self.assertEqual(g._restored,g._original_source)
 def test_prior_checks_identical(self):
  for n in ['project_rows','_FADING','_CALLER','_PRIOR','_WITNESS','_REBUILD','_UNCOVERED','_PAN','_PROJECT','_COLLECTION']:
   self.assertIs(getattr(g,n),getattr(g.previous,n))
 def test_same_supplement_instructions(self):
  old=g.previous.supplement.__code__;new=g.supplement.__code__
  for n in ['co_code','co_names','co_varnames','co_freevars','co_cellvars','co_argcount','co_kwonlyargcount','co_flags']:self.assertEqual(getattr(old,n),getattr(new,n))
 def test_same_supplement_constants_except_receipts(self):
  from types import CodeType
  def compare(a,b):
   for n in ['co_code','co_names','co_varnames','co_freevars','co_cellvars','co_argcount','co_kwonlyargcount','co_flags']:self.assertEqual(getattr(a,n),getattr(b,n))
   self.assertEqual(len(a.co_consts),len(b.co_consts))
   for x,y in zip(a.co_consts,b.co_consts):
    if isinstance(x,CodeType):self.assertIsInstance(y,CodeType);compare(x,y)
    else:self.assertEqual(g.RECEIPTS.get(x,x)if isinstance(x,str)else x,y)
  compare(g.previous.supplement.__code__,g.supplement.__code__)
 def test_complete_current_code_identical(self):self.assertIs(matrix._complete.__code__,original_current.__code__)
 def test_future_coordinator_only_validator_and_identity(self):
  R=Path(__file__).resolve().parents[1];old=(R/'tools/run-broader-strict-binding-strict-case.py').read_text();new=(R/'tools/run-broader-strict-provenance-case.py').read_text();self.assertEqual(new,old.replace('run-broader-strict-binding-strict-case.py','run-broader-strict-provenance-case.py').replace('tools/check-broader-strict-extent.py','tools/check-broader-strict-provenance.py'))
if __name__=='__main__':unittest.main()
