#!/usr/bin/env python3
"""Setup supplement refusal controls; fixtures are not Native execution evidence."""
import copy,sys,tempfile,unittest
from pathlib import Path
from unittest.mock import patch
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
import broader_setup_final_gate as gate
from broader_player_copy_matrix import P
class Final(unittest.TestCase):
 def setUp(self):
  self.tmp=tempfile.TemporaryDirectory();self.addCleanup(self.tmp.cleanup);self.root=Path(self.tmp.name)
  self.closed=dict(case_id='closed',report='absent-failed-capture.json',reserved_games=1,state='failed')
  self.rows={'old-'+str(i):dict(reserved_games=2 if i<32 else 1)for i in range(71)};self.rows['closed']=copy.deepcopy(self.closed)
  self.values=[]
  for i in range(90):
   family=['attachment','font','palette','picking'][i//4]if i<16 else'other';cid='positive-'+str(i)
   if i==88:cid='bp27-world-hotspot-default640-01'
   if i==89:cid='bp27-world-hotspot-default800-01'
   phase='phase-'+str(i);games=2 if 16<=i<24 else 1
   g=dict(phase=phase,changed_guards=1,all_other_AST_nodes_unchanged=True,actions_deadlines_budgets_Native_code_unchanged=True,original_source=dict(record='original',sha256='hash:original'),source_sha256='hash:original',adapted_source=dict(record='adapted',sha256='hash:adapted'),adapted_source_sha256='hash:adapted')
   self.values.append(dict(spec=dict(family=family,plan=phase,case=dict(id=cid,reserved_original_games=games)),acquired=dict(provider_adaptation=dict(canonical_plan_guard_adaptation=g)),validation=dict(record='CONTROL-validation'),acquisition=dict(record='CONTROL-capture'),analysis_revalidation=dict(record='CONTROL-reanalysis')if i>=88 else None))
   self.rows[cid]=dict(reserved_games=games,state='admitted')
  self.plan=dict(setup_continuation_review=dict(record='review'))
  self.review=dict(closed_setup_requests=[copy.deepcopy(self.closed)],current_controls=dict(record='controls',sha256='hash:controls'),closed_setup_certificate=dict(record='failure',sha256='hash:failure'))
  self.controls=dict(success=True,tests=33,files=3,original_game_runs=0,new_native_executions=0,sources_stable=True,Native_sources_unchanged=True,affected_families=['attachment','font','palette','picking'],sources={'source':'hash:source'})
  hotspot=dict(success=True,tests=93,files=2,original_game_runs=0,new_native_executions=0,sources_stable=True,sources={'source':'hash:source'})
  negative=P.replace('broader-player-acceptance-results-20261110/','cursor-hotspot-results-20261108/bp27/world-hotspot/')+'bp27-world-hotspot-default640-01-comparison-01.json'
  self.data={'controls':self.controls,'failure':dict(success=True),P+'B06-hotspot-analysis-controls-01.json':hotspot,negative:dict(success=False,comparison=dict(success=True,mismatching_pixels=0),error='Original cursor draw did not precede the actual readiness paint')};outer=self
  class Reader:
   def digest(self,n):return'hash:'+n
   def json(self,n):return outer.data[n]
  self.reader=Reader();mock=patch.object(gate,'review',return_value=self.review);mock.start();self.addCleanup(mock.stop)
 def check(self):return gate.supplement(self.reader,self.root,self.rows,self.values,self.plan)
 def test_complete_synthetic_shape_only_passes_supplement(self):self.assertTrue(self.check()['verified'])
 def test_closed_row_rewrite_refuses(self):
  self.rows['closed']['state']='admitted'
  with self.assertRaises(ValueError):self.check()
 def test_missing_positive_case_refuses(self):
  self.values.pop()
  with self.assertRaises(ValueError):self.check()
 def test_invented_failed_acquisition_refuses(self):
  (self.root/self.closed['report']).touch()
  with self.assertRaises(ValueError):self.check()
 def test_missing_guarded_context_refuses(self):
  self.values[0]['spec']['family']='other'
  with self.assertRaises(ValueError):self.check()
 def test_unbound_controller_phase_refuses(self):
  self.values[0]['acquired']['provider_adaptation']['canonical_plan_guard_adaptation']['phase']='other'
  with self.assertRaises(ValueError):self.check()
 def test_changed_other_controller_code_refuses(self):
  self.values[0]['acquired']['provider_adaptation']['canonical_plan_guard_adaptation']['all_other_AST_nodes_unchanged']=False
  with self.assertRaises(ValueError):self.check()
 def test_source_bytes_hash_mismatch_refuses(self):
  self.values[0]['acquired']['provider_adaptation']['canonical_plan_guard_adaptation']['adapted_source_sha256']='other'
  with self.assertRaises(ValueError):self.check()
 def test_stale_controls_refuse(self):
  self.controls['sources']['source']='old'
  with self.assertRaises(ValueError):self.check()
 def test_lost_retained_hotspot_reanalysis_refuses(self):
  self.values[-1]['analysis_revalidation']=None
  with self.assertRaises(ValueError):self.check()
if __name__=='__main__':unittest.main()
