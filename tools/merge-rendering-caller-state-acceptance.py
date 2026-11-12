#!/usr/bin/env python3
"""Preserve actual master rows and add only the two verified caller-state cells."""
import argparse,json,subprocess
from pathlib import Path
from coverage_evidence import Reader,write_report
from coverage_claims import behavior_contract,scenario_contract,required_sources,fingerprint
from rendering_acceptance import requirements,validate
from rendering_pilot import require
ROOT=Path(__file__).resolve().parents[1]
BID='NP.rendering-caller-state-master-union';SID=BID+'.checked-parents'
LEAF='operation.flags-and-return-values'
ANALYSIS=['tools/merge-rendering-caller-state-acceptance.py','tests/test-caller-state-parent-union.py','tools/rendering_acceptance.py','tools/rendering_pilot.py','tools/coverage_claims.py','tools/coverage_evidence.py','docs/native-rendering-closure.json']

def definition_identity(plan,base,parent):
 validate(plan);current=requirements(plan);current_hash=fingerprint(current);frozen=base['dossier_identity']
 if current_hash==frozen:return frozen,dict(preserved_dossier_identity=frozen,current_definition_sha256=current_hash,all_acceptance_conditions_and_owners_unchanged=True,only_added_candidate_annotations=[])
 review=parent.get('definition_review',{})
 require(review.get('preserved_dossier_identity')==frozen and review.get('current_definition_sha256')==current_hash and review.get('all_acceptance_conditions_and_owners_unchanged')is True,'Missing exact candidate-annotation review')
 changes=review.get('only_added_candidate_annotations',{}).get('parent',[])
 require(len(changes)==2 and {(c['configuration'],c['id'])for c in changes}=={(cfg,'operation.copy-alias-overlap')for cfg in('player-default','player-software')},'Unreviewed definition annotation domain')
 restored=json.loads(json.dumps(current));by={(r['configuration'],r['id']):r for r in restored}
 for c in changes:
  require(c['after']==c['before']+['ALIAS08.caller-leads01'],'Annotation is not the exact additive candidate lead')
  row=by[(c['configuration'],c['id'])];require(row.get('candidate_evidence')==c['after'],'Current candidate annotation differs');row['candidate_evidence']=c['before']
 require(fingerprint(restored)==frozen,'Acceptance conditions or owners changed with annotations')
 return frozen,dict(preserved_dossier_identity=frozen,current_definition_sha256=current_hash,all_acceptance_conditions_and_owners_unchanged=True,only_added_candidate_annotations=changes,source_parent_review=review)

def union(plan,base,owned,parent):
 identity,_=definition_identity(plan,base,parent);expected=requirements(plan);keys={(r['configuration'],r['id'])for r in expected}
 for p in(base,owned,parent):
  require(p['success']and p['dossier_identity']==identity and len(p['requirements'])==len(keys)==156 and {(r['configuration'],r['id'])for r in p['requirements']}==keys,'Parent inventory identity differs')
  require(p['passed_requirements']==sum(r['state']=='passed'for r in p['requirements'])and p['pending_requirements']==sum(r['state']=='pending'for r in p['requirements']),'Parent totals differ')
  require(p['decisions']==base['decisions'],'Final decisions differ')
 maps=[{(r['configuration'],r['id']):r for r in p['requirements']}for p in(base,owned,parent)];b,o,t=maps
 changed=[k for k in keys if o[k]!=b[k]]
 require(len(changed)==2 and all(k[1]==LEAF and b[k]['state']=='pending'and o[k]['state']=='passed'for k in changed),'Owned admission changed unrelated rows')
 require(all(t[k]==v for k,v in b.items()if v['state']=='passed'),'Accepted common-parent row changed')
 rows=[]
 for r in parent['requirements']:
  key=(r['configuration'],r['id'])
  if r['id']==LEAF:require(r['state']=='pending','Actual master caller row already changed');rows.append(o[key])
  else:rows.append(r)
 require(sum(r['state']=='passed'for r in rows)==parent['passed_requirements']+2 and all(n==r for n,r in zip(rows,parent['requirements'])if r['id']!=LEAF),'Unrelated actual-master row changed')
 return rows

def main():
 p=argparse.ArgumentParser(description=__doc__)
 for n in('parent','owned','common','actual-master','owned-commit','report'):p.add_argument('--'+n,required=True)
 a=p.parse_args();require(not(ROOT/a.report).exists(),'Fresh report required')
 git=lambda *args:subprocess.check_output(['git','-C',ROOT,*args],text=True).strip()
 require(git('rev-parse','HEAD')==a.actual_master and git('rev-parse','MERGE_HEAD')==a.owned_commit,'Actual pending merge parents differ')
 with Reader(ROOT)as r:
  sources={n:r.digest(n)for n in ANALYSIS};base=r.json(a.common);owned=r.json(a.owned);parent=r.json(a.parent)
  require(owned['kind']=='caller_visible_state_operation_admission'and owned['verified_domains']==4 and owned['newly_passed_requirements']==2 and owned['caller_state_mismatches']==0 and owned['mismatching_pixels']==0 and owned['checked_native_states']>0,'Owned full-row proof missing')
  plan=r.json('docs/native-rendering-closure.json');rows=union(plan,base,owned,parent);_,definition_review=definition_identity(plan,base,parent)
  reg=r.json('research/runtime/coverage/register.json');b=next(b for b in reg['behaviors']if b['id']==BID);s=next(s for s in reg['scenarios']if s['id']==SID);require(required_sources(b)==set(ANALYSIS),'Union source closure differs')
  phases={label:{n:dict(recorded=h,current=r.digest(n))for n,h in p.get('sources',{}).items()if r.digest(n)!=h}for label,p in [('actual_master',parent),('owned',owned)]}
  result=dict(schema=1,kind='caller_state_actual_master_row_union',success=True,analysis_sources=sources,sources=parent.get('sources',{}),parent_source_phases=parent.get('sources',{}),owned_source_phases=owned['sources'],raw_source_drift=phases,raw_source_drift_preserved=True,parent=dict(record=a.parent,sha256=r.digest(a.parent)),owned_admission=dict(record=a.owned,sha256=r.digest(a.owned)),common_parent=dict(record=a.common,sha256=r.digest(a.common)),actual_master_commit=a.actual_master,owned_commit=a.owned_commit,dossier_identity=owned['dossier_identity'],definition_review=definition_review,requirements=rows,decisions=parent['decisions'],passed_requirements=parent['passed_requirements']+2,pending_requirements=parent['pending_requirements']-2,newly_passed_requirements=2,preserved_parent_passed_requirements=parent['passed_requirements'],preserved_unrelated_parent_rows=154,closed_leaf_ids=sorted(set(parent['closed_leaf_ids'])|{LEAF}),claims=[dict(behavior=BID,contract_sha256=behavior_contract(b,{}),scenarios={SID:scenario_contract(s)})],original_game_runs=0,new_original_reference_execution=False,current_whole_native_execution=False,historical_failures_retired=False,scope='Exact actual-master row union only: preserve every incoming accepted cell/source phase and add only both caller-visible-state cells from the verified owned conjunction. Reviewed incoming changes add an exact invisible High-to-Low Spellbox native policy and host/accounting analysis tools; native Word/generic/Add/glyph state implementations and private row proofs remain their frozen phases. No new combined-player, driver, alias, exceptional or final-decision equivalence is asserted.')
  require(sources=={n:r.digest(n)for n in ANALYSIS},'Union sources changed')
 digest=write_report(ROOT,a.report,(json.dumps(result,indent=2)+'\n').encode());print(json.dumps(dict(success=True,sha256=digest,passed=result['passed_requirements'],pending=result['pending_requirements'])),flush=True)
if __name__=='__main__':main()
