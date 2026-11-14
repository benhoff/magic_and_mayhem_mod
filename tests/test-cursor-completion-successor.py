#!/usr/bin/env python3
"""Only the retained software800 case may select its completion successor."""
from pathlib import Path
import inspect,sys,unittest
from unittest.mock import patch
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
import broader_player_cursor_completion_matrix as selected
import broader_player_derived_matrix as derived
import broader_player_secondary_matrix as secondary
from broader_player_runtime_caller_matrix import patched_source
from broader_player_copy_matrix import current as base
class Tests(unittest.TestCase):
 def test_full_derived_code_preserved(self):self.assertIs(selected.selected_current.__code__,derived.current.__code__)
 def test_full_secondary_code_preserved(self):self.assertIs(selected.selected_secondary.__code__,secondary.current.__code__)
 def test_other_case_delegates_unchanged(self):
  with patch.object(selected.previous,'current',return_value=42)as f:
   self.assertEqual(selected.current(None,None,{'case':{'id':'other'}},{}),42);f.assert_called_once()
 def test_exact_case_uses_full_selected_chain(self):
  with patch.object(selected,'selected_current',return_value=43)as f:
   self.assertEqual(selected.current(None,None,{'case':{'id':selected.RECOVERED}},{}),43);f.assert_called_once()
 def test_foreign_case_selector_refuses(self):
  with self.assertRaises(ValueError):selected.coordinator_record(None,{'case':{'id':'other'},'family':'interaction','role':'world'})
 def test_foreign_family_selector_refuses(self):
  with self.assertRaises(ValueError):selected.coordinator_record(None,{'case':{'id':selected.RECOVERED},'family':'ui','role':'world'})
 def test_foreign_role_selector_refuses(self):
  with self.assertRaises(ValueError):selected.coordinator_record(None,{'case':{'id':selected.RECOVERED},'family':'interaction','role':'menu'})
 def test_final_code_preserved(self):
  from broader_player_cursor_completion_final_matrix import current
  from broader_player_discovery_final_matrix import current as original
  self.assertIs(current.__code__,original.__code__)
if __name__=='__main__':unittest.main()
