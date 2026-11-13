#!/usr/bin/env python3
"""Reject altered history and incorrect recovery report/configuration bindings."""
import copy,sys,unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from broader_collection_budget import validate_rows
from broader_collection_paths import rebind

def rows():
 return [dict(case_id='old'+str(i),report='old'+str(i),reserved_games=2 if i<41 else 1,state='failed'if i==0 else 'admitted',acquisition_sha256=None if i==0 else 'h')for i in range(128)]
def new():return {'new'+str(i):2 if i<8 else 1 for i in range(67)}
def phase():
 cases=[]
 for software in [False,True]:
  for size in [[640,480],[800,600]]:
   n='report'+str(len(cases));cases.append(dict(report=n,profile='player-software'if software else 'player-default',size=size,reserved_original_games=2,command=['python3','tools/collector.py','--report',n]+(['--software']if software else [])+(['--low']if size==[640,480]else [])))
 return dict(family='loss',broader_cases=cases,execution_paths=dict(reports={'stale':{}}),counter_bindings={})
class Ledger(unittest.TestCase):
 def test_original_snapshot(self):self.assertEqual(validate_rows(rows(),rows(),new()),169)
 def test_complete(self):
  old=rows();v=old+[dict(case_id=k,report=k,reserved_games=n,state='admitted',acquisition_sha256='h')for k,n in new().items()];self.assertEqual(validate_rows(v,old,new()),244)
 def test_no_refund(self):
  v=rows();v[0]['reserved_games']=1
  with self.assertRaises(ValueError):validate_rows(v,rows(),new())
 def test_no_history_promotion(self):
  v=rows();v[0]['state']='admitted'
  with self.assertRaises(ValueError):validate_rows(v,rows(),new())
 def test_no_omission(self):
  with self.assertRaises(ValueError):validate_rows(rows()[1:],rows(),new())
 def test_no_duplicate(self):
  with self.assertRaises(ValueError):validate_rows(rows()+[rows()[0]],rows(),new())
 def test_stop_after_new_failure(self):
  with self.assertRaises(ValueError):validate_rows(rows()+[dict(case_id='new0',report='new0',reserved_games=2,state='failed',acquisition_sha256=None)],rows(),new())
 def test_no_unknown_case(self):
  with self.assertRaises(ValueError):validate_rows(rows()+[dict(case_id='extra',report='extra',reserved_games=2,state='admitted',acquisition_sha256='h')],rows(),new())
class Maps(unittest.TestCase):
 def test_rebind_all_keys_and_no_mutation(self):
  p=phase();before=copy.deepcopy(p);q=rebind(p);self.assertEqual(p,before);self.assertEqual(set(q['execution_paths']['reports']),{c['report']for c in p['broader_cases']});self.assertEqual(q['counter_bindings']['recovery_collection_budget'],q['execution_paths']['budget'])
 def test_other_family_unchanged(self):self.assertEqual(rebind({'family':'menu'}),{'family':'menu'})
 def test_wrong_software(self):
  p=phase();p['broader_cases'][0]['command'].append('--software')
  with self.assertRaises(ValueError):rebind(p)
 def test_wrong_extent(self):
  p=phase();p['broader_cases'][0]['command'].remove('--low')
  with self.assertRaises(ValueError):rebind(p)
 def test_duplicate(self):
  p=phase();p['broader_cases'][1]=p['broader_cases'][0]
  with self.assertRaises(ValueError):rebind(p)
 def test_unbound_report(self):
  p=phase();p['broader_cases'][0]['report']='other'
  with self.assertRaises(ValueError):rebind(p)
 def test_wrong_charge(self):
  p=phase();p['broader_cases'][0]['reserved_original_games']=1
  with self.assertRaises(ValueError):rebind(p)
 def test_unsafe_report(self):
  p=phase();c=p['broader_cases'][0];c['report']='../bad';c['command'][3]='../bad'
  with self.assertRaises(ValueError):rebind(p)
if __name__=='__main__':unittest.main()
