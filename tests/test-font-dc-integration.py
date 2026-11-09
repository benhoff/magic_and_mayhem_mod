#!/usr/bin/env python3
import copy,importlib.util,sys,unittest
from pathlib import Path
from unittest.mock import patch
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools'))
from coverage_evidence import Reader
spec=importlib.util.spec_from_file_location('font_dc_review',ROOT/'tools/check-font-dc-integration.py');review=importlib.util.module_from_spec(spec);spec.loader.exec_module(review)
class Proxy:
 def __init__(self,reader,needle,mutate):self.reader=reader;self.needle=needle;self.mutate=mutate
 def json(self,name):
  q=self.reader.json(name)
  if name.endswith(self.needle):q=copy.deepcopy(q);self.mutate(q)
  return q
 def digest(self,name):return self.reader.digest(name)
class Controls(unittest.TestCase):
 @classmethod
 def setUpClass(cls):cls.reader=Reader(ROOT)
 @classmethod
 def tearDownClass(cls):cls.reader.close()
 def test_positive_current_controls_and_preserved_phases(self):
  q=review.analyze(self.reader);self.assertEqual(q['passed_requirements'],126);self.assertEqual(len(q['execution_source_phases']),4);self.assertFalse(q['current_combined_player_proved']);self.assertEqual(q['fresh_native_engine_executions'],0)
 def refused(self,needle,mutation):
  with self.assertRaises(ValueError):review.analyze(Proxy(self.reader,needle,mutation))
 def test_reject_changed_parent_count(self):self.refused('acceptance-01.json',lambda q:q.update(passed_requirements=125))
 def test_reject_missing_actual_case(self):self.refused('acceptance-01.json',lambda q:q.update(cases=q['cases'][:3]))
 def test_reject_failed_current_control(self):self.refused('merged-observer-controls-02.json',lambda q:q.update(success=False))
 def test_reject_missing_legacy_conflict_guard(self):self.refused('font-dc-merged-controls-02.json',lambda q:q.update(conflicting_activation_refused=False))
 def test_reject_changed_native_catalog(self):self.refused('merged-native-font-controls-01.json',lambda q:q['known_catalogs'][0].update(key_sha256='0'*64))
 def test_reject_changed_font_algorithm(self):
  old=review.original
  with patch.object(review,'original',lambda n:old(n)+(b'changed'if n=='apps/qt-shell/menu_fonts.cpp'else b'')),self.assertRaises(ValueError):review.analyze(self.reader)
 def test_reject_changed_frontend_body(self):
  old=review.original
  with patch.object(review,'original',lambda n:old(n)+(b'changed'if n=='tools/run-font-lifetime.py'else b'')),self.assertRaises(ValueError):review.analyze(self.reader)
if __name__=='__main__':unittest.main()
