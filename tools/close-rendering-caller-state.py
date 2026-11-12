#!/usr/bin/env python3
"""Admit the two caller-visible state rows from original contracts and real publications."""
import argparse,importlib.util,io,json,struct,tarfile,tempfile
from pathlib import Path
from coverage_evidence import Reader,write_report
from coverage_claims import behavior_contract,scenario_contract,required_sources,fingerprint
from rendering_acceptance import requirements,validate
from rendering_pilot import require
from rendering_menu_acceptance import artifact
from drawing_caller_state import decode,decode_bindings,decode_tail
from resident_capture import records
from caller_state_source_phase import review as source_phase_review
from rendering_caller_state_join import profile_operations,join_native,retained_census,join_retained_sources,validate_model_results,caller_inventory
ROOT=Path(__file__).resolve().parents[1]
PREFIX='research/runtime/caller-state-installed-results-20261108/'
COMPONENT='research/runtime/caller-state-results-20261108/'
BID='NP.rendering-caller-state-closeout';SID=BID+'.conjunction'
LEAF='operation.flags-and-return-values'
NAMES=['default640-05','software640-01','default800-02','software800-01']
COMPONENTS=['word-policy-02','world-word-05','generic-state-06','add-state-05','effect-preparation-04','glyph-full-state-01','glyph-save-clip-01','image-abi-09','kernel-matrix-02','surface-provider-03','producer-controls-13','observer-codec-09','world-requirements-09','publication-controls-04']
ANALYSIS=['tools/close-rendering-caller-state.py','tests/test-caller-state-admission.py','tools/rendering_caller_state_join.py','tools/drawing_caller_state.py','tools/inspect-canvas-producers.py','tools/resident_capture.py','tools/rendering_acceptance.py','tools/rendering_pilot.py','tools/rendering_menu_acceptance.py','tools/rendering_capture.py','tools/coverage_claims.py','tools/coverage_evidence.py','docs/native-rendering-closure.json','tools/test-rendering-glyph-caller-state.py','tools/caller_state_source_phase.py','tests/test-caller-state-source-phase.py']

def validate_cases(cases):
 require(len(cases)==4 and len({c['identity']for c in cases})==4,'Four distinct actual sessions required')
 require({(c['software'],tuple(c['size']))for c in cases}=={(s,d)for s in(False,True)for d in((640,480),(800,600))},'Missing profile/resolution domain')
 require(len({c['native_phase_sha256']for c in cases})==1,'Mixed native/player/oracle source phases')
 for c in cases:
  require(c['verified']and c['state_cases']>0 and c['pixels']>0 and c['mismatches']==0 and c['normal_retirement']and c['source_accounted']and c['independent_expected_bytes']and not c['supplied_contexts'],'Incomplete native caller-state and pixel conjunction')

def close_rows(plan,parent,cases):
 validate(plan);validate_cases(cases);expected=requirements(plan);rows=parent['requirements']
 require(parent['success']and parent['dossier_identity']==fingerprint(expected)and len(rows)==len(expected)==156 and {(r['configuration'],r['id'])for r in rows}=={(r['configuration'],r['id'])for r in expected}and sum(r['state']=='passed'for r in rows)==parent['passed_requirements'],'Parent identity/count differs')
 output=[];changed=0
 for old in rows:
  row=dict(old)
  if row['id']==LEAF:
   require(row['state']=='pending','Caller-state row already admitted')
   row.update(state='passed',admitted_execution_evidence=[dict(acquisition=c['acquisition'],comparison=c['comparison'],actual_world_extents=[c['size']],provider=c['provider'],reason='Actual V5 native entry/return state, exact source identity and independently replayed full viewport pixels. Full original-compared models for Word/generic/Add/glyph, actual producer/private kernel and provider proofs, closed consumed-state obligations at every inventoried caller, retained original composition and normal zero-owner retirement are required together. Native supported-profile policy; original driver, alias and whole-engine decisions remain separate.')for c in cases if c['software']==(row['configuration']=='player-software')]);changed+=1
  output.append(row)
 require(changed==2 and all(n==o for n,o in zip(output,rows)if o['id']!=LEAF),'Unrelated acceptance row changed')
 return output

def component(reader,name):
 record=COMPONENT+name+'.json';d=reader.json(record)
 require(d['success']and d['sources_stable']and all(reader.digest(n)==h for n,h in d['sources'].items()),'Changed/incomplete original or control component: '+name)
 if 'original_sha256'in d:require(d['original_sha256']=='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168','Wrong original component build')
 return dict(record=record,sha256=reader.digest(record),kind=d.get('kind','original_native_component'))

def read_case(reader,name):
 record=PREFIX+name+'-comparison-01.json';c=reader.json(record)
 require(c['kind']=='public_native_caller_state_comparison'and c['success']and c['sources_stable']and c['comparison_processes_stopped']and not c.get('supplied_diagnostic_contexts')and c['original_manifest_verified_before_after']and not c['original_pixels_used_as_native_inputs'],'Diagnostic/incomplete comparison')
 phase=source_phase_review(reader,record,c)
 ref=c['acquisition'];require(reader.digest(ref['record'])==ref['sha256'],'Changed acquisition');a=reader.json(ref['record'])
 require(a['success']and a['sources_stable']and a['private_wine_stopped']and a['exit_code']==0 and a['original_game_runs']==1 and a['prefix_pool']['returned']and a['original_manifest_verified_before_after']and a['prospective_plan']==c['prospective_plan'],'Unretired actual session')
 graphics=a['retirement']['graphics_retirement'];resident=json.loads(artifact(reader,a,'resident-world-report.json'))
 require(resident['normal_quit_retired']and not resident['pending']and not resident['error']and resident['stage_profile']['capture_complete']and graphics['resources_released']and not graphics['surviving_owners']and all(v==0 for v in graphics['counts'].values())and a['retirement']['forwarded']==0,'Incomplete prefix/native lifetime')
 size=a['configuration']['actual_size'];require(c['world_domain']['verified']and c['world_domain']['format']=='RGB565'and c['world_domain']['extents']==[size],'Unproved actual World domain')
 if a['configuration']['software_flag']:require(resident['renderer'].lower().startswith(('llvmpipe','softpipe')),'Software provider unproved')
 pixels=c['comparison'];require(pixels['success']and pixels['mismatching_pixels']==0 and pixels['pixels_compared']==sum(p['pixels']for p in pixels['comparisons'])and all(p['mismatching_pixels']==0 and p['actual_sha256']==p['expected_sha256']for p in pixels['comparisons']),'Incomplete independent pixel comparison')
 reqref=c['requirements'];require(reader.digest(reqref['record'])==reqref['sha256'],'Changed caller requirements');req=reader.json(reqref['record']);inventory=caller_inventory(req);require(inventory==c['caller_inventory']and inventory['sites']==63,'Incomplete original caller inventory')
 spec=importlib.util.spec_from_file_location('caller_closeout_cps',ROOT/'tools/inspect-canvas-producers.py');cps=importlib.util.module_from_spec(spec);spec.loader.exec_module(cps)
 state=decode(artifact(reader,a,'caller-state-0000.bin'));require(state['version']==5,'Actual V5 required');bindings=decode_bindings(artifact(reader,a,'caller-bindings-0000.bin'),state);tail=decode_tail(artifact(reader,a,'caller-source-tail-0000.bin'),cps.decode)
 with tempfile.TemporaryDirectory(prefix='caller-admission-')as temp:
  path=Path(temp)/'source.bin';path.write_bytes(artifact(reader,a,'worlds.bin'));blocks=list(records(path))
 require(tail['generation']==sum(not ch for g,ch,s,q in blocks)+1 and tail['source']==sum(bool(ch)for g,ch,s,q in blocks)+1,'Incomplete final source tail')
 if tail['records']:blocks.append((tail['generation'],tail['source'],tail['stream'],b''))
 operations=profile_operations(blocks,cps.decode);joined=join_native(state['rows'],operations,req,{r['id']:r for r in bindings['rows']},tail['nonce']);census=retained_census(state['rows']);retained=join_retained_sources(state['rows'],operations,joined['used'],census,tail['nonce'])
 require(fingerprint(census)==fingerprint(c['retained_census'])and fingerprint(retained)==fingerprint(c['retained_source_join']),'Raw original/native source accounting changed')
 archive=next(o for o in c['artifacts']if o['path']=='outputs.tar.gz');require(reader.digest(archive['record'])==archive['sha256'],'Changed model output archive')
 with tarfile.open(fileobj=io.BytesIO(reader.read_bytes(archive['record'])),mode='r:gz')as tar:
  member=tar.getmember('state-history/model-results.bin');descriptor=next(m for m in archive['members']if m['path']==member.name);raw=tar.extractfile(member).read()
  import hashlib
  require(len(raw)==descriptor['bytes']and hashlib.sha256(raw).hexdigest()==descriptor['sha256'],'Changed retained expected bytes')
 fresh=validate_model_results(raw,joined['joined']);require(all(c['caller_state'][k]==v for k,v in fresh.items())and c['caller_state']['normal_retirement']and c['caller_state']['history']['model_mismatches']==0,'Expected-byte publication comparison changed')
 return dict(identity=a['session_identity'],software=a['configuration']['software_flag'],size=size,verified=True,state_cases=fresh['cases'],models=fresh['models'],pixels=pixels['pixels_compared'],mismatches=0,normal_retirement=True,source_accounted=True,primitive_sources=retained['primitive_sources'],retained_roots=len(retained['roots']),independent_expected_bytes=True,supplied_contexts=False,plan_sha256=c['prospective_plan']['sha256'],native_phase_sha256=phase['native_phase_sha256'],source_phase_review=phase,acquisition=ref,comparison=dict(record=record,sha256=reader.digest(record)),provider=resident['renderer'],raw_reanalysis=True,caller_sites=inventory['sites'])

def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--parent',default='research/runtime/palette-master-merge-results-20261108/acceptance-01.json');p.add_argument('--report',required=True);a=p.parse_args();require(a.report.startswith(PREFIX)and not(ROOT/a.report).exists(),'Fresh owned report required')
 with Reader(ROOT)as r:
  sources={n:r.digest(n)for n in ANALYSIS};capacity_controls=r.json(PREFIX+'capacity-repair-controls-01.json');require(capacity_controls['success']and capacity_controls['controls']==4 and capacity_controls['actual_retained_budget']['requests_before']==7 and capacity_controls['actual_retained_budget']['original_games_before']==5,'Missing cumulative repair controls');components=[component(r,n)for n in COMPONENTS];cases=[read_case(r,n)for n in NAMES];parent=r.json(a.parent);rows=close_rows(r.json('docs/native-rendering-closure.json'),parent,cases)
  reg=r.json('research/runtime/coverage/register.json');b=next(b for b in reg['behaviors']if b['id']==BID);s=next(s for s in reg['scenarios']if s['id']==SID);require(required_sources(b)==set(ANALYSIS),'Closeout source closure differs');require(sources=={n:r.digest(n)for n in ANALYSIS},'Closeout sources changed')
  result=dict(schema=1,kind='caller_visible_state_operation_admission',success=True,sources_stable=True,analysis_sources=sources,sources=parent['sources']|{n:h for n,h in sources.items()if n not in parent['sources']},parent_source_phases=parent['sources'],components=components,capacity_repair_controls=dict(record=PREFIX+'capacity-repair-controls-01.json',sha256=r.digest(PREFIX+'capacity-repair-controls-01.json')),cases=cases,requirements=rows,verified_domains=4,required_domains=4,newly_passed_requirements=2,passed_requirements=parent['passed_requirements']+2,pending_requirements=parent['pending_requirements']-2,closed_leaf_ids=sorted(set(parent['closed_leaf_ids'])|{LEAF}),dossier_identity=parent['dossier_identity'],decisions=parent['decisions'],parent=dict(record=a.parent,sha256=r.digest(a.parent)),claims=[dict(behavior=BID,contract_sha256=behavior_contract(b,{}),scenarios={SID:scenario_contract(s)})],original_game_runs=0,verified_positive_original_starts=4,checked_pixels=sum(c['pixels']for c in cases),checked_native_states=sum(c['state_cases']for c in cases),mismatching_pixels=0,caller_state_mismatches=0,current_whole_native_execution=False,original_driver_comparison=False,historical_failures_retired=False,alias_rows_closed=False,scope='Supported default/software caller-visible state obligations alongside complete original-history viewport pixels. Conjunction of bounded original/private proofs and actual public publications. No claim of arbitrary hidden caller/FP/driver/alias/gameplay equivalence; original fallback remains accounted original work.')
 digest=write_report(ROOT,a.report,(json.dumps(result,indent=2)+'\n').encode());print(json.dumps(dict(success=True,record=a.report,sha256=digest,passed=result['passed_requirements'],pending=result['pending_requirements'],states=result['checked_native_states'],pixels=result['checked_pixels'])),flush=True)
if __name__=='__main__':main()
