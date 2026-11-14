#!/usr/bin/env python3
"""Dashboard admission rejects mixed proofs and preserves independent recovery."""
import copy,hashlib,importlib.util,json,sys,unittest
from pathlib import Path
from unittest.mock import patch
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools'))
from broader_player_dashboard import KIND,PROJECTION_KIND,READER,validate
from broader_player_verdicts import issue
from coverage_rendering_artifacts import rendering_artifacts_view
spec=importlib.util.spec_from_file_location('verdict_fixture',ROOT/'tests/test-broader-player-verdicts.py');module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)

def fixture():
 current,parent,prior=module.fixture();definition=json.loads((ROOT/'apps/coverage-ui/tracks.json').read_text());closure=json.loads((ROOT/definition['rendering_artifacts']['closure']).read_text());gaps=closure['required_gap_register'];rows=[]
 for cfg in closure['completion_contract']['configurations']:
  for journey in gaps['journeys']:
   for leaf in journey['subrequirements']:rows.append(dict(id='journey.'+journey['id']+'.'+leaf['id'],configuration=cfg['id'],state='passed',owner_task='EP04'))
  for prefix,key in [('operation.','operations'),('gate.','gates')]:
   for row in gaps[key]:rows.append(dict(id=prefix+row['id'],configuration=cfg['id'],state='passed',owner_task='EP06'))
 ref=lambda n:dict(record='research/runtime/fixture-'+n+'.json',sha256='f'*64)
 sources={n:hashlib.sha256((ROOT/n).read_bytes()).hexdigest()for n in [READER,'tools/broader_player_dashboard.py','tools/broader_player_verdicts.py']}
 current['sources']=sources.copy();current['required_boundary_impact']=ref('boundary');parent.update(kind=KIND,requirements=rows,proof=ref('scope'),decision_record=ref('prior'));boundary=dict(retained_finite_master=ref('parent'));verdict=dict(kind='broader_current_independent_rendering_verdicts',success=True,sources_stable=True,current_acceptance=ref('current'),finite_requirement_parent=ref('parent'),finite_requirement_rows=copy.deepcopy(rows),all156parent_rows_preserved_exactly=True,sources=sources.copy(),**issue(current,parent,prior))
 projection=dict(kind=KIND,projection_kind=PROJECTION_KIND,success=True,current_acceptance=ref('current'),independent_decisions=ref('verdict'),finite_requirement_parent=ref('parent'),requirements=copy.deepcopy(rows),proof=parent['proof'],scope=current['scope'],decisions=copy.deepcopy(verdict['decisions']),sources=sources.copy())
 scope=dict(success=True,gate_ready=True,supported_player_accepted=False,original_recovery_complete=False,decisions=[dict(v,player_scope_state='resolved',original_recovery_state='unresolved')for v in closure['completion_contract']['scope_decisions']])
 payloads={ref(n)['record']:v for n,v in [('current',current),('verdict',verdict),('boundary',boundary),('parent',parent),('prior',prior),('scope',scope)]}
 return projection,payloads,definition

class Projection(unittest.TestCase):
 def setUp(self):self.q,self.payloads,self.definition=fixture()
 def check(self):return validate(self.q,lambda ref:self.payloads[ref['record']])
 def test_complete_projection_preserves_rows_and_original_decision(self):
  old=copy.deepcopy((self.q,self.payloads));parent=self.check();self.assertEqual(parent['requirements'],self.q['requirements']);self.assertEqual((self.q,self.payloads),old);self.assertEqual(self.q['decisions'],{'supported-player':'accepted','original-recovery':'pending'})
 def test_unchanged_ui_reader_displays_complete_decision_and_eight_boundaries(self):
  self.check();path=self.definition['rendering_artifacts']['progress_record']
  def read(p,**kwargs):
   n=Path(p).relative_to(ROOT).as_posix();return self.q if n==path else self.payloads[n]
  with patch('coverage_rendering_artifacts.read_json',side_effect=read),patch('coverage_rendering_artifacts.evidence_digest',return_value='f'*64):
   view=rendering_artifacts_view(ROOT,self.definition)
  self.assertEqual(view['progress']['total'],156);self.assertEqual(view['progress']['counts'],{'passed':156});self.assertEqual(view['progress']['decisions'],self.q['decisions']);self.assertEqual(view['progress']['boundary_dispositions']['resolved_player_scopes'],8);self.assertTrue(all(v['state']=='resolved'for v in view['contract']['scope_decisions']))
 def test_partial_cohort_cannot_be_projected(self):
  self.payloads[self.q['current_acceptance']['record']]['current_cases']=89
  with self.assertRaises(ValueError):self.check()
 def test_original_verdict_cannot_be_rewritten(self):
  self.payloads[self.q['independent_decisions']['record']]['verdicts'][1]['contract_groups'].pop()
  with self.assertRaises(ValueError):self.check()
 def test_projection_cannot_rewrite_rows_scope_decisions_or_boundary(self):
  for change in [lambda q:q['requirements'][0].update(state='pending'),lambda q:q.update(scope='widened'),lambda q:q['decisions'].update(**{'original-recovery':'accepted'}),lambda q:q.update(proof=dict(record='wrong',sha256='f'*64))]:
   with self.subTest(change=change):
    self.q,self.payloads,_=fixture();change(self.q)
    with self.assertRaises(ValueError):self.check()
 def test_projection_cannot_drop_or_rewrite_source_bindings(self):
  for mode in ['missing','changed']:
   self.q,self.payloads,_=fixture();n=READER
   if mode=='missing':self.q['sources'].pop(n)
   else:self.q['sources'][n]='0'*64
   with self.assertRaises(ValueError):self.check()
 def test_mixed_current_decision_or_parent_identity_refused(self):
  for key in ['current_acceptance','finite_requirement_parent']:
   self.q,self.payloads,_=fixture();self.payloads[self.q['independent_decisions']['record']][key]=dict(record='foreign',sha256='f'*64)
   with self.assertRaises(ValueError):self.check()
 def test_compatibility_kind_does_not_replace_projection_discriminator(self):
  self.q.pop('projection_kind')
  with self.assertRaises((KeyError,ValueError)):self.check()
if __name__=='__main__':unittest.main()
