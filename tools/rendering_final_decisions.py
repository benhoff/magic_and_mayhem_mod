"""Issue independent, evidence-bound pending verdicts for the retained rendering dossier.

This adjudicator has no positive-acceptance operation. Accepted finite gates keep
all their original source/execution limits; the decision-recording gate records
an issued verdict, not acceptance of either engine or product claim.
"""
import ast
import copy
import importlib.util
import json
from pathlib import Path
from coverage_claims import behavior_contract, fingerprint
from rendering_acceptance import requirements, validate
from rendering_pilot import require

ROOT=Path(__file__).resolve().parents[1]
PREFIX='research/runtime/rendering-final-decisions-results-20261110/'
BID='NP.final-rendering-decisions'
SID='native.final-rendering-decisions'
GATE='gate.separate-decisions'
BOUNDARY='gate.required-boundaries'
PROFILES=('player-default','player-software')
DECISIONS=('supported-player','original-recovery')
SCOPE=('Issue separate supported-player and original-recovery verdicts from the exact current EP10 matrix, '
       'accepted finite gate evidence, original contract scopes and retained failures. Both verdicts remain '
       'pending and accepted=false: finite historical player proofs and current host joins do not supply '
       'a final current whole-player acceptance receipt or complete original recovery. Record precise '
       'profile/blocker/exit evidence; preserve all154other rows, original behavior milestones, historical '
       'native/compiler/oracle hashes, failures and approved support amendments. Close only the two '
       'decision-recording gates. No new game, native execution, pixel comparison, positive acceptance, '
       'original-recovery completion, required-boundary closure or gameplay change.')


def local_python_sources():
    pending=[Path(__file__),ROOT/'tools/close-rendering-alias-bounded.py'];seen=set()
    while pending:
        p=pending.pop().resolve();name=p.relative_to(ROOT).as_posix()
        if name in seen:continue
        seen.add(name)
        for node in ast.walk(ast.parse(p.read_bytes())):
            names=[v.name for v in node.names]if isinstance(node,ast.Import)else([node.module]if isinstance(node,ast.ImportFrom)and node.module else[])
            for value in names:
                for base in (ROOT/'tools',ROOT/'protocols/python'):
                    target=base/Path(*value.split('.')).with_suffix('.py')
                    if target.is_file():pending.append(target)
    return seen


def module(name):
    spec=importlib.util.spec_from_file_location('final_decisions_'+name.replace('-','_'),ROOT/'tools'/(name+'.py'))
    m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);return m


def basis(plan,parent,bindings):
    validate(plan);definitions={(r['configuration'],r['id']):r for r in requirements(plan)}
    reviewed=module('close-rendering-alias-bounded').definition_review(plan,parent)
    annotations={(r['configuration'],r['id']):r for r in(reviewed or{}).get('only_added_candidate_annotations',[])}
    rows=parent['requirements'];keys=[(v['configuration'],v['id'])for v in rows]
    require(parent['success']is True and len(rows)==len(set(keys))==len(definitions)==156 and set(keys)==set(definitions),'Changed requirement inventory')
    require(parent['passed_requirements']==sum(v['state']=='passed'for v in rows)and parent['pending_requirements']==sum(v['state']=='pending'for v in rows)
            and parent['passed_requirements']+parent['pending_requirements']==156,'Changed requirement totals')
    require(parent['decisions']=={n:'pending'for n in DECISIONS},'Parent has an unreviewed final verdict')
    require(parent['scope_amendments']==bindings['scope_amendments']==plan['acceptance_scope_amendments'],'Approved scope amendment changed')
    require(parent['current_whole_player_proved']is False and parent['current_combined_player_proved']is False
            and parent['historical_failures_retired']is False,'Changed whole-player/failure evidence needs a new adjudicator')
    frozen={(v['configuration'],v['id']):v for v in bindings['rows']if v['id']not in (GATE,BOUNDARY)}
    require(len(frozen)==152,'Incomplete frozen positive requirement domain')
    for row in rows:
        key=row['configuration'],row['id'];expected=copy.deepcopy(definitions[key])
        if key in annotations:expected['candidate_evidence']=annotations[key]['before']
        require(all(row[k]==v for k,v in expected.items()),'Changed requirement owner or condition')
        if row['id']==GATE:require(row['state']=='pending'and row['admitted_execution_evidence']==[],'Decision gate already changed')
        elif row['id']==BOUNDARY:require(row['state']in('pending','passed')and bool(row['admitted_execution_evidence'])is(row['state']=='passed'),'Invalid required-boundary disposition')
        else:require(row==frozen[key]and row['state']=='passed'and finite_evidence(row),'Changed accepted finite requirement')
    return rows


def finite_evidence(row):
    if row['admitted_execution_evidence']:return True
    return (row['id']=='operation.copy-alias-overlap'and row.get('acceptance_scope')=='bounded_controlled_opaque_overlap'
            and row.get('tested_surface_extent')==[8,6]and row.get('natural_player_reachability')=='pending'
            and row.get('actual_player_overlap_domains')==0 and row.get('admitted_mechanism_evidence')
            and row.get('retained_obligation',{}).get('state')=='pending'
            and row['retained_obligation'].get('gate')==BOUNDARY)


class Inputs:
    def __init__(self,reader):self.reader=reader;self.records={};self.phases={}
    def checked(self,ref):
        require(isinstance(ref,dict)and {'record','sha256'}<=ref.keys()and ref['record'].startswith('research/runtime/')and '..'not in Path(ref['record']).parts
                and self.reader.digest(ref['record'])==ref['sha256'],'Retained report identity changed')
        q=self.reader.json(ref['record']);require(q['success']is True,'Failed retained analysis')
        self.records[ref['record']]=ref;sources=q.get('sources',q.get('analysis_sources',{}))
        self.phases[ref['record']]=dict(recorded_sources=sources,current_drift={n:dict(recorded=h,current=self.reader.digest(n)if(ROOT/n).is_file()else None)
            for n,h in sources.items()if not(ROOT/n).is_file()or self.reader.digest(n)!=h},current_native_execution_claimed=False)
        return q


def references(v):
    if isinstance(v,dict):
        if {'record','sha256'}<=v.keys():yield v
        else:
            for x in v.values():yield from references(x)
    elif isinstance(v,list):
        for x in v:yield from references(x)


def historical_failures(inputs,bindings):
    progress=inputs.checked(bindings['supports'][bindings['historical_negative_progress']]);rows=progress['negative_executions']
    require(rows and len({r['reference']['record']for r in rows})==len(rows),'Historical normal failure omitted or duplicated')
    result=[]
    for row in rows:
        ref=row['reference'];require(inputs.reader.digest(ref['record'])==ref['sha256'],'Historical failure bytes changed')
        a=inputs.reader.json(ref['record']);require(a['success']is False and a['original_manifest_verified_before_after']is True,'Historical negative is not a retained execution failure')
        require(row['resolution']=='unresolved'and 'supported-player'in row['blocks_final_decisions'],'Historical failure disposition was silently retired')
        result.append(dict(reference=ref,recorded_error=row['error'],recorded_scope=row['scope'],recorded_resolution=row['resolution'],
            current_relevance='not_established_by_this_adjudication',current_failure_asserted=False,
            exit='Bind a scoped source/caller/configuration relevance review and any affected repair/rerun; retain the original failure and source hashes.'))
    return result


def recovery_review(reader,plan,bindings):
    reg=reader.json('research/runtime/coverage/register.json');behaviors={b['id']:b for b in reg['behaviors']};builds={b['id']:b for b in reg['builds']}
    require(len(plan['contracts'])==15 and len({v['id']for v in plan['contracts']})==15,'Original contract groups changed')
    frozen={v['definition']['id']:v for v in bindings['recovery_groups']};groups=[]
    for definition in plan['contracts']:
        require(definition==frozen[definition['id']]['definition'],'Original recovery scope changed')
        current=[dict(id=n,scope=behaviors[n]['scope'],kind=behaviors[n]['kind'],status=behaviors[n]['status'],contract_sha256=behavior_contract(behaviors[n],builds),original=behaviors[n].get('original',[]))for n in definition['behaviors']]
        require(current==frozen[definition['id']]['registered_behaviors'],'Registered recovery milestones changed; review the new input scope')
        groups.append(dict(id=definition['id'],title=definition['title'],baseline_established_scope=definition['established'],baseline_remaining_scope=definition['remaining'],
            recovery_exit=definition['recovery_exit'],registered_behaviors=current,whole_group_completion_established=False,
            assessment='Registered milestones retain their exact scopes/statuses. Neither a function link nor permitted original supplier establishes this complete original contract.'))
    return groups


def verdicts(parent,groups,failures,scope_review,gate_evidence):
    require({v['id']for v in scope_review['decisions']}=={'SC.alternate-ownership','SC.padded-fullscreen','SC.natural-callers','SC.prior-history','SC.map-save-domain','SC.screen-provider-lifetime','SC.rgb555-live','SC.physical-compatibility'},'Scope review domain changed')
    require(len(groups)==15 and failures and scope_review['original_recovery_complete']is False,'Missing recovery/failure limits')
    boundaries=[v for v in parent['requirements']if v['id']==BOUNDARY]
    require(len(boundaries)==2 and {v['configuration']for v in boundaries}==set(PROFILES),'Required-boundary profile omitted')
    profile_rows=[]
    for profile in PROFILES:
        boundary=next(v for v in boundaries if v['configuration']==profile)
        blockers=[dict(id='final-current-player-receipt',state='pending',owner_task='EP11',reason='No final current whole-player execution acceptance receipt is supplied by this retained finite matrix. Response deadlines, forward progress, resource bounds and exact supported input/provider/source domain remain required; FPS alone is diagnostic.')]
        if boundary['state']=='pending':blockers.insert(0,dict(id=BOUNDARY,state='pending',owner_task=boundary['owner_task'],reason=boundary['pending_reason']))
        profile_rows.append(dict(configuration=profile,state='pending',accepted=False,boundary_requirement=boundary,
            finite_gates=[v for v in gate_evidence if v['configuration']==profile],blockers=blockers,
            response_progress_resources=dict(state='finite_evidence_preserved_whole_claim_pending',response_evidence=['gate.input-media','gate.journeys','gate.transitions'],
                progress_evidence=['gate.pixels-history','gate.journeys','gate.transitions'],resource_evidence=['gate.normal-lifetime','gate.fault-lifetime'],
                fixed_fps_floor_required=False,whole_claim_response_progress_resource_validation_established=False)))
    player=dict(id='supported-player',issued=True,state='pending',accepted=False,profiles=profile_rows,
        historical_failures=failures,reason='Accepted finite execution/source phases remain useful within their stated bounds. Required-boundary dispositions and the absent final current whole-player acceptance receipt preclude a positive product verdict.',
        exit_conditions=['Resolve each applicable required-boundary row with evidence and explicit supported map/save/provider/duration limits.',
            'Supply adequate current complete public-player execution acceptance for both configurations, including exact build/compiler/input/policy identities, independent required output/state and response/progress/resource bounds.',
            'Record applicability and any required repair/rerun of retained negative observations; preserve every historical failure and source hash.'])
    original=dict(id='original-recovery',issued=True,state='pending',accepted=False,profiles=[dict(configuration=p,state='pending',accepted=False)for p in PROFILES],
        contract_groups=groups,historical_scope_review=scope_review['decisions'],
        reason='Complete original recovery is not established for the fifteen original contract groups. Native policies, permitted original suppliers and finite player proofs retain separate claims; existing recovered behavior milestones remain unchanged.',
        exit_conditions=['Supply scope-complete original behavior/branch/dispatch/lifetime/failure evidence for the claimed original contract groups.',
            'Resolve applicable original-recovery scope conditions separately from any supported-player boundary or native policy.',
            'Retain independent original comparison, implementation, integration and replacement milestones; a positive player decision alone supplies no original-recovery completion.'])
    return [player,original]


def validate_verdicts(q):
    require(q['success']is True and q['decisions_issued']==2 and len(q['verdicts'])==2
            and {v['id']for v in q['verdicts']}==set(DECISIONS),'Missing/duplicate independent verdict')
    for v in q['verdicts']:
        require(v['issued']is True and v['state']=='pending'and v['accepted']is False and v['reason'].strip()and v['exit_conditions'],'Unreviewed positive or unexplained verdict')
        require(len(v['profiles'])==2 and {p['configuration']for p in v['profiles']}==set(PROFILES)
                and all(p['state']=='pending'and p['accepted']is False for p in v['profiles']),'Missing/positive profile verdict')
    player=next(v for v in q['verdicts']if v['id']=='supported-player');recovery=next(v for v in q['verdicts']if v['id']=='original-recovery')
    require(all(any(b['id']=='final-current-player-receipt'and b['state']=='pending'for b in p['blockers'])for p in player['profiles']), 'Whole-player evidence blocker omitted')
    for p in player['profiles']:
        budget=p['response_progress_resources']
        require(budget['fixed_fps_floor_required']is False and budget['whole_claim_response_progress_resource_validation_established']is False
                and all(budget[k]for k in ('response_evidence','progress_evidence','resource_evidence')),'Response/progress/resource evidence replaced by FPS')
        require(p['boundary_requirement']['state']!='pending'or any(b['id']==BOUNDARY for b in p['blockers']),'Pending required boundary omitted')
    require(len(recovery['contract_groups'])==15 and len({g['id']for g in recovery['contract_groups']})==15
            and all(g['whole_group_completion_established']is False and g['registered_behaviors']for g in recovery['contract_groups']),'Original contract limits omitted or promoted')
    require(player['historical_failures']and all(v['current_failure_asserted']is False for v in player['historical_failures']),'Historical observation silently removed or made a current regression')
    require(q['supported_player_accepted']is q['original_recovery_complete']is q['current_whole_player_proved']is False,'Unsupported engine/product acceptance')
    return True


def analyze(reader,parent,bindings=None):
    bindings=bindings or reader.json(PREFIX+'bindings-01.json');plan=reader.json('docs/native-rendering-closure.json');rows=basis(plan,parent,bindings);inputs=Inputs(reader)
    support={n:inputs.checked(ref)for n,ref in bindings['supports'].items()}
    gate_evidence=[]
    for row in rows:
        if row['kind']!='gate'or row['id']in(GATE,BOUNDARY):continue
        refs=list({v['record']:v for v in references(row['admitted_execution_evidence'])}.values())
        require(refs and all(reader.digest(v['record'])==v['sha256']for v in refs),'Accepted gate attachment identity changed')
        gate_evidence.append(dict(configuration=row['configuration'],id=row['id'],state=row['state'],attachments=refs,scope='Only the unchanged accepted finite gate and its original execution/source limits; no final whole-player or original-recovery promotion.'))
    require(len(gate_evidence)==16,'Missing finite gate/profile context')
    groups=recovery_review(reader,plan,bindings);failures=historical_failures(inputs,bindings)
    q=dict(schema=1,kind='native_rendering_final_decision_record',success=True,artifact=5,decisions_issued=2,verdicts=verdicts(parent,groups,failures,support[bindings['scope_review']],gate_evidence),
        decisions={n:'pending'for n in DECISIONS},requirement_rows=156,decision_gate_meaning='Both separate verdicts have been issued; a passed decision-recording gate is not a positive verdict.',
        accepted_finite_gates=gate_evidence,approved_scope_amendments=parent['scope_amendments'],retained_records=list(inputs.records.values()),retained_execution_source_phases=inputs.phases,
        supported_player_accepted=False,original_recovery_complete=False,current_whole_player_proved=False,
        original_game_runs=0,new_native_executions=0,new_pixel_comparisons=0,historical_failures_retired=False,scope=SCOPE)
    validate_verdicts(q);return q


def close_rows(parent,ref,proof):
    validate_verdicts(proof);rows=copy.deepcopy(parent['requirements'])
    for row in rows:
        if row['id']==GATE:
            require(row['state']=='pending'and not row['admitted_execution_evidence'],'Decision gate already changed')
            row.update(state='passed',admitted_execution_evidence=[dict(decision_record=ref,decision_ids=list(DECISIONS),issued_verdicts={n:'pending'for n in DECISIONS},accepted_claims=[],reason=proof['decision_gate_meaning'])])
    require(sum(v['state']=='passed'for v in rows)==parent['passed_requirements']+2,'Exceeded exactly two decision gates')
    return rows
