#!/usr/bin/env python3
import copy,sys,unittest
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools'))
from broader_player_join import join_rows,exact_rows
class JoinControls(unittest.TestCase):
 def setUp(self):
  self.old=dict(record='old',sha256='1'*64);self.row=dict(configuration='player-default',requirement='journey.active-world.animation',executions=[dict(primary=self.old,output=dict(independent=True))]);self.current=dict(historical_primary=self.old,profile='player-default',success=True,sources_stable=True,current_native_phase=dict(current_native_source_candidate=True),accepted=True,role='world',validation=dict(record='v',sha256='2'*64),acquisition=dict(record='a',sha256='3'*64),semantic_binding=dict(unchanged_route=True));self.mapping={('old','1'*64):self.current};self.expected={('player-default','journey.active-world.animation')}
 def run_join(self):return join_rows([self.row],self.expected,self.mapping,dict(record='parent',sha256='4'*64),journey=True)
 def test_preserves_each_attachment_and_parent(self):
  before=copy.deepcopy(self.row);self.row['executions'].append(copy.deepcopy(self.row['executions'][0]));q=self.run_join();self.assertEqual(len(q[0]['attachments']),2);self.assertEqual(self.row['executions'][0],before['executions'][0]);self.assertEqual([a['historical_execution_index']for a in q[0]['attachments']],[0,1]);self.assertTrue(q[0]['parent_row_preserved_exactly'])
 def test_missing_and_duplicate_rows_refused(self):
  for rows in([], [self.row,self.row]):
   with self.assertRaises(ValueError):exact_rows(rows,self.expected)
 def test_foreign_requirement_refused(self):
  self.row['requirement']='journey.active-world.missing'
  with self.assertRaises(ValueError):self.run_join()
 def test_no_historical_hash_refresh(self):
  self.row['executions'][0]['primary']=dict(record='old',sha256='9'*64)
  with self.assertRaises(ValueError):self.run_join()
 def test_no_missing_primary(self):
  self.mapping.clear()
  with self.assertRaises(ValueError):self.run_join()
 def test_failed_stale_unadmitted_historical_and_wrong_profile_refused(self):
  for field,value in [('success',False),('sources_stable',False),('accepted',False),('profile','player-software'),('current_native_phase',dict(current_native_source_candidate=False))]:
   original=self.current[field];self.current[field]=value
   with self.assertRaises(ValueError):self.run_join()
   self.current[field]=original
 def test_failed_epochs_and_selected_planes_do_not_credit_journey(self):
  for role in('fault','component'):
   self.current['role']=role
   with self.assertRaises(ValueError):self.run_join()
 def test_empty_attachment_list_refused(self):
  self.row['executions']=[]
  with self.assertRaises(ValueError):self.run_join()
 def test_controlled_overlap_keeps_pending_natural_scope(self):
  row=dict(configuration='player-default',requirement='operation.copy-alias-overlap',natural_player_reachability='pending',controlled_mechanism=dict(natural_player_call_proved=False,actual_player_domains=0));control=dict(verified=True,natural_player_call_proved=False,approved_amendment='ALIAS09.controlled-opaque-overlap-v1');expected={('player-default',row['requirement'])}
  q=join_rows([row],expected,{},dict(record='p'),journey=False,controlled_overlap=control);self.assertFalse(q[0]['attachments'][0]['natural_player_call_proved'])
  row['natural_player_reachability']='passed'
  with self.assertRaises(ValueError):join_rows([row],expected,{},dict(record='p'),journey=False,controlled_overlap=control)
if __name__=='__main__':unittest.main()
