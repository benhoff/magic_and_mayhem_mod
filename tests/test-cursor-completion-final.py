#!/usr/bin/env python3
"""Final reader selection preserves parsing, complete-cohort guards and cleanup."""
import importlib.util,sys,unittest
from pathlib import Path
from unittest.mock import patch
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
p=Path(__file__).resolve().parents[1]/'tools/check-broader-cursor-completion-final.py';s=importlib.util.spec_from_file_location('cursor_final_controls',p);m=importlib.util.module_from_spec(s);s.loader.exec_module(m)
class Tests(unittest.TestCase):
 def test_kind_required(self):
  with patch.object(sys,'argv',[str(p)]):
   with self.assertRaises(ValueError):m.main()
 def test_unknown_kind_refused(self):
  with patch.object(sys,'argv',[str(p),'--kind','foreign']):
   with self.assertRaises(ValueError):m.main()
 def test_selected_matrix_and_restoration(self):
  prior=m.frozen.current;writer=m.coverage_evidence.write_report;argv=[str(p),'--kind','failure','--report','unused']
  def run(kind,caller):
   self.assertEqual(kind,'failure');self.assertIs(m.frozen.current,m.selected.current);self.assertEqual(sys.argv,[str(p),'--report','unused']);return 7
  with patch.object(sys,'argv',argv),patch.object(m,'run',side_effect=run):self.assertEqual(m.main(),7);self.assertIs(sys.argv,argv)
  self.assertIs(m.frozen.current,prior);self.assertIs(m.coverage_evidence.write_report,writer)
 def test_exception_restores_bindings(self):
  prior=m.frozen.current;writer=m.coverage_evidence.write_report
  with patch.object(sys,'argv',[str(p),'--kind','acceptance','--report','unused']),patch.object(m,'run',side_effect=ValueError('full cohort required')):
   with self.assertRaises(ValueError):m.main()
  self.assertIs(m.frozen.current,prior);self.assertIs(m.coverage_evidence.write_report,writer)
if __name__=='__main__':unittest.main()
