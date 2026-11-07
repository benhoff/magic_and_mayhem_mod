import copy
import importlib.util
import json
from pathlib import Path
import sys
import unittest
from unittest.mock import patch

ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'tools'))
from coverage_evidence import Reader
from coverage_rendering_artifacts import rendering_artifacts_view
import coverage_rendering_artifacts as progress_reader
s=importlib.util.spec_from_file_location('join_operations',ROOT/'tools/join-rendering-full-fill-acceptance.py')
m=importlib.util.module_from_spec(s);s.loader.exec_module(m)

class Checks(unittest.TestCase):
 @classmethod
 def setUpClass(cls):
  with Reader(ROOT) as r:
   cls.a=r.json(m.m.PREFIX+'closeout-analysis-02.json')
   cls.parent=r.json('research/runtime/rendering-first-world-results-20261107/merged-acceptance-03.json')
   cls.plan=r.json('docs/native-rendering-closure.json')
 def result(self,a=None,parent=None):
  with Reader(ROOT) as r:return m.join(r,a or self.a,parent or self.parent,self.plan)
 def test_preserves_every_parent_row(self):
  q=self.result();self.assertEqual(q['passed_requirements'],self.parent['passed_requirements']+2)
  self.assertEqual([v for v in q['requirements'] if v['id']!=m.m.LEAF],[v for v in self.parent['requirements'] if v['id']!=m.m.LEAF])
  self.assertFalse(q['case_execution_repeated'])
 def test_actual_configured_progress_available(self):
  definition=json.loads((ROOT/'apps/coverage-ui/tracks.json').read_text())
  joined=self.result()
  with patch.object(progress_reader,'read_json',return_value=joined):
   view=rendering_artifacts_view(ROOT,definition)['progress']
   self.assertIn(view['state'],('matching_sources','historical'))
   self.assertEqual(view['total'],156)
   self.assertEqual(sum(view['counts'].values()),156)
  broken=dict(joined);broken.pop('scope')
  with patch.object(progress_reader,'read_json',return_value=broken):
   self.assertEqual(rendering_artifacts_view(ROOT,definition)['progress']['state'],'unavailable')
  self.assertIn(rendering_artifacts_view(ROOT,definition)['progress']['state'],('matching_sources','historical'))
 def test_failed_component_refuses(self):
  a=copy.deepcopy(self.a);a['success']=False
  with self.assertRaises(ValueError):self.result(a)
 def test_missing_full_fill_witnesses_refuses(self):
  a=copy.deepcopy(self.a);a['full_fill_witnesses']-=1
  with self.assertRaises(ValueError):self.result(a)
 def test_changed_component_source_refuses(self):
  a=copy.deepcopy(self.a);a['analysis_sources'][next(iter(a['analysis_sources']))]='0'*64
  with self.assertRaises(ValueError):self.result(a)
 def test_changed_input_refuses(self):
  a=copy.deepcopy(self.a);a['cases'][0]['pair']['acquisition']['sha256']='0'*64
  with self.assertRaises(ValueError):self.result(a)
 def test_unadmitted_case_refuses(self):
  p=copy.deepcopy(self.parent)
  for row in p['requirements']:row['admitted_execution_evidence']=[]
  with self.assertRaises(ValueError):self.result(parent=p)

if __name__=='__main__':unittest.main()
