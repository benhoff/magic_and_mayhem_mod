#!/usr/bin/env python3
"""Guard code identity and bounded fallback dispatch for stopped UI analysis."""
import importlib.util,sys,unittest
from pathlib import Path
from unittest.mock import Mock,patch
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
import broader_terminal_pause as paused
import broader_player_terminal_matrix as matrix
import broader_player_derived_matrix as derived
import broader_player_secondary_matrix as secondary
from broader_terminal_ui_identity import MESSAGE,bind_pause
class Tests(unittest.TestCase):
    def test_unchanged_current_code(self):self.assertIs(matrix.current.__code__,derived.current.__code__)
    def test_unchanged_secondary_code(self):self.assertIs(matrix.secondary_current.__code__,secondary.current.__code__)
    def test_unchanged_pause_code(self):self.assertIs(bind_pause(paused.original,Mock()).__code__,paused.original.__code__)
    def test_reviewed_observation_controls_required(self):
        r=Mock();r.json.return_value={'success':False}
        with patch.object(paused,'from_acquisition')as f:
            with self.assertRaises(ValueError):paused.verify_pause(r,{})
            f.assert_not_called()
    def test_terminal_failure_preserved(self):
        r=Mock();r.json.side_effect=[{'success':True,'tests':20,'sources':{}},{'success':True,'tests':30,'sources':{}}]
        with patch.object(paused,'from_acquisition',side_effect=ValueError('Stopped retirement')):
            with self.assertRaisesRegex(ValueError,'Stopped retirement'):paused.verify_pause(r,{})
    def test_foreign_case_uses_prior_chain(self):
        with patch.object(secondary,'prior_current',return_value=42)as f:
            self.assertEqual(matrix.prior(None,None,{'case':{'id':'other'}},{}),42);f.assert_called_once()
    def test_selector_foreign_case_refused(self):
        with self.assertRaises(ValueError):matrix.coordinator_record(None,{'case':{'id':'other'},'family':'interaction','role':'world'})
    def test_selector_foreign_role_refused(self):
        with self.assertRaises(ValueError):matrix.coordinator_record(None,{'case':{'id':matrix.RECOVERED},'family':'interaction','role':'menu'})
    def test_stale_controls_refused(self):
        r=Mock();r.json.return_value={'success':True,'tests':20,'sources':{'x':'old'}};r.digest.return_value='changed'
        with patch.object(paused,'original',side_effect=ValueError(MESSAGE)),patch.object(paused,'from_acquisition')as f:
            with self.assertRaises(ValueError):paused.verify_pause(r,{})
            f.assert_not_called()
    def test_acquire_forbidden(self):
        path=Path(__file__).resolve().parents[1]/'tools/run-broader-terminal-source-case.py'
        sp=importlib.util.spec_from_file_location('terminal_admission_guard_control',path);m=importlib.util.module_from_spec(sp);sp.loader.exec_module(m)
        with patch.object(sys,'argv',[str(path),'acquire']):
            with self.assertRaises(ValueError):m.main()
if __name__=='__main__':unittest.main()
