#!/usr/bin/env python3
"""Refuse incomplete/current-source/provider claims and preserve independent recovery."""
import copy,sys,unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from broader_player_verdicts import issue,ROLES,PROFILES

def fixture():
 cases=[]
 for role,count in ROLES.items():
  for i in range(count):
   profile=PROFILES[i%2]
   cases.append(dict(id=role+str(i),role=role,profile=profile,actual_drivers=['NVIDIA isolated-test-fixture'if profile==PROFILES[0]else'llvmpipe isolated-test-fixture'],exact_current_native_source_candidate=True))
 current=dict(kind='broader_current_public_player_acceptance',success=True,sources_stable=True,current_whole_player_proved=True,supported_player_accepted=True,original_recovery_complete=False,current_native_sources_verified_without_waiver=True,current_cases=90,current_source_original_game_starts=98,journey_cells=94,operation_cells=42,ignored_pixels=0,all_required_output_pixels_compared=True,original_padding_equivalence=False,arbitrary_maps_saves_durations_supported=False,historical_failures_retired=False,physical_driver_reset_proved=False,natural_player_overlap_domains=0,actual_cases=cases,scope_amendments=[dict(id='retained-amendment')],supported_input_domain=dict(record='test-domain'),required_boundary_impact=dict(record='test-boundary'),historical_failure_relevance=dict(record='test-relevance'),scope='ISOLATED SYNTHETIC MODEL CONTROL ONLY')
 parent=dict(success=True,passed_requirements=156,pending_requirements=0,requirements=[dict(id=str(i),state='passed')for i in range(156)],scope_amendments=copy.deepcopy(current['scope_amendments']))
 original=dict(id='original-recovery',issued=True,state='pending',accepted=False,contract_groups=[dict(id=i,unresolved=True)for i in range(15)],historical_scope_review=[dict(id=i,unresolved=True)for i in range(8)],profiles=[dict(configuration=p,state='pending',accepted=False)for p in PROFILES],reason='Retain exact original findings',exit_conditions=['Unchanged independent exit'])
 prior=dict(success=True,original_recovery_complete=False,approved_scope_amendments=copy.deepcopy(current['scope_amendments']),verdicts=[original])
 return current,parent,prior

class Verdicts(unittest.TestCase):
 def test_separate_verdicts_preserve_exact_original_object(self):
  a,b,c=fixture();old=copy.deepcopy(c);q=issue(a,b,c);self.assertEqual(q['decisions'],{'supported-player':'accepted','original-recovery':'pending'});self.assertEqual(q['verdicts'][1],c['verdicts'][0]);self.assertEqual(c,old);q['verdicts'][1]['contract_groups'][0]['unresolved']=False;self.assertTrue(c['verdicts'][0]['contract_groups'][0]['unresolved'])
 def test_partial_or_stale_current_execution_refused(self):
  for mutate in [lambda a:a.update(success=False),lambda a:a.update(current_cases=88),lambda a:a['actual_cases'].pop(),lambda a:a['actual_cases'][0].update(exact_current_native_source_candidate=False)]:
   a,b,c=fixture();mutate(a)
   with self.assertRaises(ValueError):issue(a,b,c)
 def test_duplicate_or_missing_selected_plane_fault_or_stress_case_refused(self):
  for role in ['component','fault','strict','boundary']:
   a,b,c=fixture();v=next(x for x in a['actual_cases']if x['role']==role);v['role']='world'
   with self.assertRaises(ValueError):issue(a,b,c)
  a,b,c=fixture();a['actual_cases'][0]['id']=a['actual_cases'][1]['id']
  with self.assertRaises(ValueError):issue(a,b,c)
 def test_provider_fallback_or_empty_observation_refused(self):
  for driver in ['llvmpipe NVIDIA fallback','',None]:
   a,b,c=fixture();a['actual_cases'][0]['actual_drivers']=[driver]if driver is not None else[]
   with self.assertRaises(ValueError):issue(a,b,c)
 def test_required_boundary_or_amendment_change_refused(self):
  a,b,c=fixture();b['requirements'][0]['state']='pending'
  with self.assertRaises(ValueError):issue(a,b,c)
  a,b,c=fixture();c['approved_scope_amendments']=[]
  with self.assertRaises(ValueError):issue(a,b,c)
 def test_original_recovery_promotion_or_missing_contract_refused(self):
  for mutate in [lambda c:c.update(original_recovery_complete=True),lambda c:c['verdicts'][0].update(accepted=True),lambda c:c['verdicts'][0]['contract_groups'].pop(),lambda c:c['verdicts'][0]['historical_scope_review'].pop()]:
   a,b,c=fixture();mutate(c)
   with self.assertRaises(ValueError):issue(a,b,c)
 def test_pixel_historical_and_physical_waivers_refused(self):
  for k,v in [('ignored_pixels',1),('original_padding_equivalence',True),('historical_failures_retired',True),('physical_driver_reset_proved',True),('arbitrary_maps_saves_durations_supported',True),('natural_player_overlap_domains',1)]:
   a,b,c=fixture();a[k]=v
   with self.assertRaises(ValueError):issue(a,b,c)
if __name__=='__main__':unittest.main()
