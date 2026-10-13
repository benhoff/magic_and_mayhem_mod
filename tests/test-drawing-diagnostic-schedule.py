import importlib.util,json
from pathlib import Path
import unittest
ROOT=Path(__file__).resolve().parents[1]
import sys
sys.path.insert(0,str(ROOT/'tools'))
spec=importlib.util.spec_from_file_location('cadence_compare',ROOT/'tools/compare-drawing-diagnostic-cadence.py');module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
class DeclaredSchedule(unittest.TestCase):
 def test_actions_bind_the_selected_source(self):
  name='tests/fixtures/drawing-journeys/interactive-schedule.json';raw=(ROOT/name).read_bytes();actions=json.loads(raw)['actions']
  report={'sources':{name:module.sha(raw)},'input_schedule':{'actions':[dict(x,start_ms=i,end_ms=i+1) for i,x in enumerate(actions)]}}
  self.assertEqual(module.executed_schedule(report,raw,name),module.sha(raw))
  report['input_schedule']['actions'][-1]['after_ms']=800
  with self.assertRaisesRegex(ValueError,'Executed input'):module.executed_schedule(report,raw,name)
 def test_source_presence_alone_cannot_prove_execution(self):
  raw=json.dumps({'actions':[{'kind':'wait','after_ms':8000}]}).encode();name='schedule.json'
  for actions in [[],[{'kind':'wait','after_ms':800}]]:
   with self.assertRaises(ValueError):module.executed_schedule({'sources':{name:module.sha(raw)},'input_schedule':{'actions':actions}},raw,name)
  with self.assertRaises(ValueError):module.executed_schedule({'sources':{},'input_schedule':{'actions':[]}},raw,name)
if __name__=='__main__':unittest.main()
