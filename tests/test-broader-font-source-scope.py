import unittest
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
class FrozenFontContinuationScope(unittest.TestCase):
 def test_world_coordinator_only_changes_request_driver_and_own_identity(self):
  before=(ROOT/'tools/run-broader-visible-domain-case.py').read_text();after=(ROOT/'tools/run-broader-font-domain-case.py').read_text()
  normalized=after.replace('tools/run-broader-font-source-case.py','tools/run-broader-visible-source-case.py').replace('tools/run-broader-font-domain-case.py','tools/run-broader-visible-domain-case.py');self.assertEqual(normalized,before)
 def test_selected_coordinator_only_changes_request_driver_and_own_identity(self):
  before=(ROOT/'tools/run-broader-selected-domain-case.py').read_text();after=(ROOT/'tools/run-broader-font-selected-domain-case.py').read_text()
  normalized=after.replace('tools/run-broader-font-source-case.py','tools/run-broader-visible-source-case.py').replace('tools/run-broader-font-selected-domain-case.py','tools/run-broader-selected-domain-case.py');self.assertEqual(normalized,before)
 def test_request_driver_preserves_the_original_state_machine(self):
  before=(ROOT/'tools/run-broader-visible-source-case.py').read_text();after=(ROOT/'tools/run-broader-font-source-case.py').read_text();self.assertEqual(after.replace('from broader_font_source_budget import','from broader_visible_source_budget import'),before)
if __name__=='__main__':unittest.main()
