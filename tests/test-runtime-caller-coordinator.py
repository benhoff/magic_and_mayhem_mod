#!/usr/bin/env python3
"""Only the reviewed caller coordinator path may change; Native guards stay exact."""
import ast,copy,inspect,sys,unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from broader_player_runtime_caller_matrix import P,FIRST,REVIEW,coordinator_record,patched_source,original_current

def fixture():
 phase=dict(record=P+'phase.json',sha256='a'*64);old=dict(record=P+FIRST+'-coordinator-01.json',sha256='b'*64);acq=dict(record='acq',sha256='c'*64);execution=dict(record='execution',sha256='d'*64)
 review=dict(kind='reviewed_runtime29_caller_root_retained_analysis_plan',success=True,case=FIRST,original_game_runs=0,tests=11,resolved_source_count=1376,evidence_payload_controls=9,total_controls=20,resolved_evidence_payloads={str(i):{}for i in range(7)},Native_source_waivers=False,source_lookup_waivers=False,sources={'code':'e'*64},phase=phase,failed_coordinator=old,acquisition=acq)
 successor=dict(kind='broader_domain_case_coordinator',success=True,case=FIRST,plan=phase,acquisition=acq,previous_failed_coordinator=old,retained_analysis_plan=REVIEW,original_game_runs_added=0,new_acquisition_performed=False,retained_analysis_execution=execution)
 proof=dict(success=True,plan=REVIEW,original_game_runs=0,new_public_player_executions=0,new_request_reservations=0,request_rows_unchanged=True)
 values={REVIEW['record']:review,old['record']:dict(success=False,case=FIRST,plan=phase),P+FIRST+'-coordinator-02.json':successor,'execution':proof};hashes={REVIEW['record']:REVIEW['sha256'],old['record']:old['sha256'],phase['record']:phase['sha256'],'code':'e'*64,'execution':'d'*64,P+FIRST+'-coordinator-02.json':'f'*64}
 class Reader:
  def json(self,name):return values[name]
  def digest(self,name):return hashes[name]
 return Reader(),values,dict(case=dict(id=FIRST),family='caller',role='world',plan=phase['record'])

class Coordinator(unittest.TestCase):
 def test_one_reference_changes_and_every_other_predicate_is_preserved(self):
  source=inspect.getsource(original_current);result=patched_source(source);self.assertEqual(result.count('_caller_coordinator_record(r,spec)'),1);self.assertEqual(result.replace('co_ref=ref(_caller_coordinator_record(r,spec))',"co_ref=ref(P+case['id']+'-coordinator-01.json')"),source)
 def test_missing_or_ambiguous_reference_refused(self):
  for source in ['def current():return None',inspect.getsource(original_current)*2]:
   with self.assertRaises(ValueError):patched_source(source)
 def test_actual_successor_path_is_returned_without_aliasing_old_report(self):
  r,v,s=fixture();before=copy.deepcopy(v);self.assertEqual(coordinator_record(r,s),P+FIRST+'-coordinator-02.json');self.assertEqual(v,before);self.assertFalse(v[P+FIRST+'-coordinator-01.json']['success'])
 def test_other_context_keeps_its_original_reference(self):
  r,v,s=fixture();s['case']['id']='bp29-world-combat-default640-01';self.assertEqual(coordinator_record(r,s),P+s['case']['id']+'-coordinator-01.json')
 def test_role_family_or_phase_relabel_refused(self):
  for key,value in [('family','attachment'),('role','fault'),('plan','wrong')]:
   r,v,s=fixture();s[key]=value
   with self.assertRaises((ValueError,KeyError)):coordinator_record(r,s)
 def test_old_refusal_must_remain_false_and_bound_to_exact_case(self):
  for key,value in [('success',True),('case','foreign')]:
   r,v,s=fixture();v[P+FIRST+'-coordinator-01.json'][key]=value
   with self.assertRaises(ValueError):coordinator_record(r,s)
 def test_new_game_reservation_or_changed_acquisition_refused(self):
  for key,value in [('original_game_runs_added',1),('new_acquisition_performed',True),('acquisition',{})]:
   r,v,s=fixture();v[P+FIRST+'-coordinator-02.json'][key]=value
   with self.assertRaises(ValueError):coordinator_record(r,s)
 def test_unreviewed_execution_or_row_rewrite_refused(self):
  for key,value in [('new_public_player_executions',1),('new_request_reservations',1),('request_rows_unchanged',False),('plan',{})]:
   r,v,s=fixture();v['execution'][key]=value
   with self.assertRaises(ValueError):coordinator_record(r,s)
 def test_source_waiver_or_incomplete_root_refused(self):
  for key,value in [('Native_source_waivers',True),('source_lookup_waivers',True),('resolved_source_count',1375)]:
   r,v,s=fixture();v[REVIEW['record']][key]=value
   with self.assertRaises(ValueError):coordinator_record(r,s)
if __name__=='__main__':unittest.main()
