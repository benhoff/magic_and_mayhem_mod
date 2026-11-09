#!/usr/bin/env python3
"""Admit only both surface-loss rows from four current raw provider-loss proofs."""
import argparse,copy,json,sqlite3
from pathlib import Path
from coverage_evidence import Reader,write_report
from coverage_claims import behavior_contract,scenario_contract,required_sources,fingerprint
from rendering_acceptance import requirements,validate
from rendering_pilot import require
from rendering_transition_assessment import observed_epochs,assess
from rendering_surface_loss import source_phase_review
ROOT=Path(__file__).resolve().parents[1]
PREFIX='research/runtime/surface-loss-restore-results-20261109/'
BID='NP.surface-loss-restore-acceptance'
LEAF='operation.surface-loss-restore'

def definition(plan,parent):
    validate(plan);rows=requirements(plan);current=fingerprint(rows);frozen=parent['dossier_identity']
    if current==frozen:return dict(preserved_dossier_identity=frozen,current_definition_sha256=current,all_acceptance_conditions_and_owners_unchanged=True)
    review=parent.get('definition_review',{})
    require(review.get('preserved_dossier_identity')==frozen and review.get('current_definition_sha256')==current and
            review.get('all_acceptance_conditions_and_owners_unchanged') is True,'Unreviewed acceptance definition drift')
    changes=review.get('only_added_candidate_annotations',[])
    if isinstance(changes,dict):changes=changes.get('parent',[])
    require(len(changes)==2 and {(c['configuration'],c['id']) for c in changes}==
        {(p,'operation.copy-alias-overlap') for p in ('player-default','player-software')},'Unexpected annotation domain')
    restored=copy.deepcopy(rows);by={(r['configuration'],r['id']):r for r in restored}
    for c in changes:
        row=by[(c['configuration'],c['id'])]
        require(c['after']==c['before']+['ALIAS08.caller-leads01'] and row['candidate_evidence']==c['after'],'Changed acceptance condition/owner')
        row['candidate_evidence']=c['before']
    require(fingerprint(restored)==frozen,'Acceptance contract changed')
    return dict(preserved_dossier_identity=frozen,current_definition_sha256=current,
        all_acceptance_conditions_and_owners_unchanged=True,only_added_candidate_annotations=changes,source_parent_review=review)

def validate_cases(cases):
    require(len(cases)==4 and len({c['identity'] for c in cases})==4,'Four independent native loss sessions required')
    require({(c['domain']['software'],tuple(c['domain']['size'])) for c in cases}==
        {(p,size) for p in (False,True) for size in ((640,480),(800,600))},'Incomplete provider/profile/extent domain')
    require(len({c['player_sha256'] for c in cases})==1 and all(c['compiler_sources']==cases[0]['compiler_sources'] for c in cases),
            'Mixed native player source phases')
    for c in cases:
        p=c['proof'];loss=p.get('provider_loss',{})
        require(p['verified'] and p['cleanup']['verified'] and p['physical_public_restart'] and p['fresh_positive_input'] and
                p['old_rendering_accepted'] is False and p['failed_epoch_normal_completion_claimed'] is False,'Incomplete failure/restart lifetime')
        require(loss.get('provider_owner')=='gui-resident' and loss.get('actual_platform_target_destroyed') is True and loss.get('restored_for_deletion_only') is True and
                loss.get('old_generation_continuation') is False and loss.get('physical_hardware_reset') is False,'Wrong provider-loss scope')
        require(p['terminal_samples']>=8 and p['terminal_stability_ns']>=600_000_000 and len(set(p['distinct_world_nonces']))==2 and
                len({(v['pid'],v['start_ticks']) for v in p['distinct_runner_identities']})==2 and c['pixels']>0,'Missing bounded terminal/new-owner proof')

def close_rows(plan,parent,cases):
    review=definition(plan,parent);validate_cases(cases)
    expected={(r['configuration'],r['id']) for r in requirements(plan)}
    require(parent['success'] and len(parent['requirements'])==156 and {(r['configuration'],r['id']) for r in parent['requirements']}==expected and
        parent['passed_requirements']==sum(r['state']=='passed' for r in parent['requirements']),'Parent identity/count mismatch')
    rows=[];changed=0
    for old in parent['requirements']:
        row=copy.deepcopy(old)
        if row['id']==LEAF:
            require(row['state']=='pending','Surface-loss row already changed');changed+=1
            row.update(state='passed',admitted_execution_evidence=[dict(acquisition=c['acquisition'],comparison=c['comparison'],
                actual_world_extents=[c['domain']['size']],provider=c['provider'],
                reason='Actual ordinary native Qt offscreen target destroyed at a parked render turn. Typed context-acquisition failure, target restoration for deletion only, zero retained command/storage/frame owners, stable terminal input/output and physical public fresh-generation restart with distinct original runner/channel/World identities. Complete presented failed prefix and fresh normal Quit output independently compared. Native provider compatibility boundary excludes physical reset, arbitrary driver/races, transparent old-generation continuation and isolated-mode recovery.')
                for c in cases if c['domain']['software']==(row['configuration']=='player-software')])
        rows.append(row)
    require(changed==2 and all(a==b for a,b in zip(rows,parent['requirements']) if b['id']!=LEAF),'Admission changed unrelated rows')
    return rows,review

def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--parent',default='research/runtime/caller-state-master-results-20261108/admission-01.json')
    p.add_argument('--comparison',action='append',required=True);p.add_argument('--phase-plan');p.add_argument('--report',required=True);a=p.parse_args()
    require(a.report.startswith(PREFIX) and not (ROOT/a.report).exists(),'Fresh owned admission required')
    result=dict(schema=1,kind='native_surface_loss_operation_admission',success=False,newly_passed_requirements=0,original_game_runs=0,
        physical_hardware_reset=False,current_whole_native_execution=False,historical_failures_retired=False)
    try:
        with Reader(ROOT) as r:
            phase=r.json(a.phase_plan) if a.phase_plan else None
            phase_ref=dict(record=a.phase_plan,sha256=r.digest(a.phase_plan)) if phase else None
            reg=r.json('research/runtime/coverage/register.json');b=next(v for v in reg['behaviors'] if v['id']==BID)
            sources={n:r.digest(n) for n in sorted(required_sources(b))};cases=[];plans=set()
            for name in a.comparison:
                c=r.json(name);require(c['success'] and c['sources_stable'] and c['comparison_processes_stopped'] and c['original_manifest_verified_before_after'],
                    'Incomplete surface-loss comparison')
                ref=c['acquisition'];require(r.digest(ref['record'])==ref['sha256'],'Acquisition identity drift');ac=r.json(ref['record'])
                require(ac['kind']=='public_native_surface_loss_acquisition' and ac['success'] and ac['sources_stable'] and ac['original_game_runs']==2 and
                        ac['private_wine_stopped'] and not ac['original_pixels_used_as_native_inputs'] and not c['original_pixels_used_as_native_inputs'],
                        'Wrong/dependent loss execution')
                source_review=source_phase_review(r,name,c,phase)
                _,epochs,partition=observed_epochs(r,ac,provider_loss=True);proof=assess(r,ac,c,epochs,provider_loss=True)
                require(proof==c['proof'] and partition==c['partition'],'Raw native loss assessment changed')
                require(c['comparison']['success'] and c['comparison']['pixels_compared']>0 and c['comparison']['mismatching_pixels']==0,'Uncompared native output')
                cases.append(dict(identity=ac['session_identity'],domain=proof['domain'],proof=proof,comparison=dict(record=name,sha256=r.digest(name)),
                    acquisition=ref,pixels=c['comparison']['pixels_compared'],provider=ac['resident']['renderer'],
                    player_sha256=ac['player_binary']['sha256'],compiler_sources=ac['compiler_sources'],execution_sources=c['sources'],
                    prospective_plan=ac['prospective_plan'],source_scope_review=source_review))
                plans.add((ac['prospective_plan']['record'],ac['prospective_plan']['sha256']))
            if phase:
                require(phase['success'] and r.digest(phase['parent_plan']['record'])==phase['parent_plan']['sha256'] and
                    plans=={(phase_ref['record'],phase_ref['sha256']),(phase['parent_plan']['record'],phase['parent_plan']['sha256'])},
                    'Different/unreviewed frozen native loss phases')
                low={c['comparison']['record'] for c in cases if c['domain']['size']==[640,480]}
                require(low=={v['record'] for v in phase['controller_continuation']['retained_low_comparisons']} and
                    all(c['prospective_plan']==(phase['parent_plan'] if c['domain']['size']==[640,480] else phase_ref) for c in cases),
                    'Extent/controller source phase substituted')
            else:require(len(plans)==1,'Different frozen native loss acquisition phases')
            parent=r.json(a.parent);rows,review=close_rows(r.json('docs/native-rendering-closure.json'),parent,cases)
            plan_name,plan_sha=(phase_ref['record'],phase_ref['sha256']) if phase else next(iter(plans))
            plan=r.json(plan_name);require(r.digest(plan_name)==plan_sha,'Plan changed')
            budget=ROOT/plan['execution_paths']['budget']
            with sqlite3.connect(budget.as_uri()+'?mode=ro',uri=True) as db:attempts=list(db.execute('SELECT report,repair_of,charged_games FROM attempts'))
            require(len(attempts)<=plan['bounds']['maximum_requests'] and sum(v[2] for v in attempts)<=plan['bounds']['maximum_original_games'],'Acquisition budget exceeded')
            requests=[]
            for name,repair,charge in attempts:
                ac=r.json(name);require(ac['charged_original_starts']==charge and ac['original_game_runs']<=charge,'Lost original-start charge')
                requests.append(dict(record=name,sha256=r.digest(name),repair_of=repair,charged_games=charge,observed_game_starts=ac['original_game_runs'],actual_start_count_complete=ac['actual_start_count_complete'],success=ac['success']))
            execution_sources={}
            for c in cases:
                for n,h in c['execution_sources'].items():
                    current=r.digest(n)
                    require(h==current or n in c['source_scope_review'].get('preserved_recorded_sources',{}),'Unreviewed recorded execution source')
                    require(execution_sources.setdefault(n,current)==current,'Mixed current analysis dependency')
            result.update(success=True,sources=execution_sources|sources,analysis_sources=sources,cases=cases,verified_domains=4,requirements=rows,
                dossier_identity=parent['dossier_identity'],definition_review=review,decisions=parent['decisions'],
                passed_requirements=parent['passed_requirements']+2,pending_requirements=parent['pending_requirements']-2,newly_passed_requirements=2,
                closed_leaf_ids=sorted(set(parent['closed_leaf_ids'])|{LEAF}),parent=dict(record=a.parent,sha256=r.digest(a.parent)),
                parent_source_phases=parent.get('sources',{}),parent_source_drift={n:dict(recorded=h,current=r.digest(n)) for n,h in parent.get('sources',{}).items() if r.digest(n)!=h},
                checked_pixels=sum(c['pixels'] for c in cases),mismatching_pixels=0,requests=requests,
                original_game_runs=sum(v['observed_game_starts'] for v in requests),charged_original_starts=sum(v['charged_games'] for v in requests),
                actual_start_count_complete=all(v['actual_start_count_complete'] for v in requests),
                claims=[dict(behavior=BID,contract_sha256=behavior_contract(b,{}),scenarios={s['id']:scenario_contract(s) for s in reg['scenarios'] if BID in s['behaviors']})],
                controller_extent_continuation=phase_ref,
                retained_execution_source_phases=[dict(comparison=c['comparison'],prospective_plan=c['prospective_plan'],
                    sources=c['execution_sources'],source_scope_review=c['source_scope_review']) for c in cases],
                compatibility_boundary=dict(provider='ordinary native Qt offscreen GL target',profiles=['player-default','player-software'],
                    recovery='terminal stop, target recreation for deletion, physical public fresh-generation restart',
                    physical_hardware_reset=False,transparent_continuation=False,legacy_directdraw_driver_equivalence=False,isolated_service_continuation=False),
                scope='Only the two finite ordinary native provider-loss cells. Accepted parent source phases/rows remain exact; no new whole-player or original-recovery decision.')
            result['sources_stable']=all(r.digest(n)==h for n,h in result['sources'].items());result['success']&=result['sources_stable']
    except Exception as e:result.update(success=False,error=str(e),newly_passed_requirements=0)
    sha=write_report(ROOT,a.report,(json.dumps(result,indent=2)+'\n').encode())
    print(json.dumps(dict(success=result['success'],error=result.get('error'),sha256=sha,passed=result.get('passed_requirements'),pending=result.get('pending_requirements'))),flush=True)
    return int(not result['success'])
if __name__=='__main__':raise SystemExit(main())
