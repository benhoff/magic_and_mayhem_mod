import copy
import importlib.util
from pathlib import Path
import sys
import unittest

ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'tools'))
from coverage_evidence import Reader
s=importlib.util.spec_from_file_location('join_operations',ROOT/'tools/join-rendering-operations-acceptance.py')
m=importlib.util.module_from_spec(s);s.loader.exec_module(m)

class Checks(unittest.TestCase):
 @classmethod
 def setUpClass(cls):
  with Reader(ROOT) as r:
   cls.a=r.json(m.m.PREFIX+'closeout-analysis-final-02.json')
   cls.parent=r.json('research/runtime/rendering-picking-resize-results-20261107/pan-closeout-02.json')
   cls.plan=r.json('docs/native-rendering-closure.json')
 def result(self,a=None,parent=None):
  with Reader(ROOT) as r:return m.join(r,a or self.a,parent or self.parent,self.plan)
 def test_preserves_every_parent_row(self):
  q=self.result();self.assertEqual(q['passed_requirements'],self.parent['passed_requirements']+2)
  self.assertEqual([v for v in q['requirements'] if v['id']!=m.m.LEAF],[v for v in self.parent['requirements'] if v['id']!=m.m.LEAF])
  self.assertFalse(q['case_execution_repeated'])
 def test_failed_component_refuses(self):
  a=copy.deepcopy(self.a);a['success']=False
  with self.assertRaises(ValueError):self.result(a)
 def test_missing_replies_refuses(self):
  a=copy.deepcopy(self.a);a['actual_cpu_reads']-=1
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
