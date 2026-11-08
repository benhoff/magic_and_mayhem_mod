#!/usr/bin/env python3
"""Admit exactly four scene rows from current actual eight-case scene proofs."""
import argparse
from copy import deepcopy
from hashlib import sha256
import json
from pathlib import Path
import struct
import subprocess
import sys

from coverage_claims import behavior_contract,required_sources,scenario_contract
from coverage_evidence import Reader,write_report
from rendering_closeout import inventory
from rendering_menu_acceptance import artifact
from rendering_pilot import require
from scene_actor_observation import decode
from scene_actor_output import ROOT,PREFIX,PARENT,scoped_artifacts,combat_pose_change

BID='NP.scene-attachment-combat-admission'
SID='native.scene-attachment-combat-admission'
LEAVES={'journey.creatures-effects.attachment':'attachment','journey.creatures-effects.combat':'combat'}
LIVE={'NP.scene-actor-generation-observation':('native.scene-combat-first-contact','native.scene-attachment-public-selection'),
      'NP.scene-combat-physical-observation':('native.scene-combat-first-contact',),
      'NP.scene-attachment-physical-observation':('native.scene-attachment-public-selection',)}
COLLECTOR='tools/run-scene-attachment.py'
DRIVER='tools/scene_attachment_journey.py'
BUDGET_EDITS=((b"plan['maximum_actual_games'] == 5",b"plan['maximum_actual_games'] in (5,6)"),
    (b"require(len(previous)<5,'Five-game attachment scenario budget exhausted')",
     b"require(len(previous)<plan['maximum_actual_games'],'Attachment scenario budget exhausted')"))
READY_BEFORE=b'''    def select_spell(self):
        initial = spell_inventory(self.read()[1]); self.proof['inventory'] = initial
'''
READY_AFTER=b'''    def ready_spellbox_inventory(self):
        require(self.last and self.last['screen'] == 7 and self.last['ready'],
                'A saved ready Spellbox visit is required')
        captured = menus.state(menus.ROOT/self.last['wire_path'])
        require(captured and all(captured[0][key] == self.last[key] for key in
                ('sequence', 'generation', 'screen', 'ready', 'ack', 'status', 'thread', 'handoff')),
                'Saved Spellbox wire differs from the ready visit')
        return spell_inventory(captured[1])

    def select_spell(self):
        initial = self.ready_spellbox_inventory(); self.proof['inventory'] = initial
'''


def budget_only_change(before,after,maximum=6):
    """Recognize exactly the two host charging checks, never a capture edit."""
    require(maximum in(6,8,9,10),'Unreviewed attachment start bound')
    edits=list(BUDGET_EDITS)
    if maximum==8:edits[0]=(edits[0][0],b"plan['maximum_actual_games'] in (5,6,8)")
    if maximum==9:edits[0]=(edits[0][0],b"plan['maximum_actual_games'] in (5,6,8,9)")
    if maximum==10:edits[0]=(edits[0][0],b"plan['maximum_actual_games'] in (5,6,8,9,10)")
    for old,new in edits:
        require(before.count(old)==1,'Unknown five-start collector preimage')
        before=before.replace(old,new)
    require(before==after,'Collector changed outside its two prospective budget checks')


def collector_phase(reader,name,*,require_current=True,seen=()):
    if name is None:return None
    require(name not in seen,'Cyclic collector budget review')
    q=reader.json(name)
    require(q['success']is True and q['path']==COLLECTOR and q['maximum_before']==5 and q['maximum_after']in(6,8,9,10)
            and q['user_approved']is True,'Unapproved or different collector budget review')
    for key in('before','after','authority'):
        require(reader.digest(q[key]['record'])==q[key]['sha256'],'Changed collector source preimage')
    authority=reader.json(q['authority']['record'])
    require(authority['success']is True and authority['maximum_actual_games']==q['maximum_after'],
            'Collector bound differs from the explicit user authority')
    budget_only_change(reader.read_bytes(q['before']['record']),reader.read_bytes(q['after']['record']),q['maximum_after'])
    accepted={q['before']['sha256']}
    prior=q.get('prior_phases',[])
    require(len(prior)==(1 if q['maximum_after']>=8 else 0),'Missing or unexpected intermediate collector phase')
    for ref in prior:
        require(reader.digest(ref['record'])==ref['sha256'],'Changed intermediate collector review')
        previous=collector_phase(reader,ref['record'],require_current=False,seen=(*seen,name))
        require(previous['maximum_after']=={8:6,9:8,10:9}[q['maximum_after']] and previous['before']==q['before'],
                'Unrelated intermediate collector source phase')
        accepted.update(previous['accepted_before_sha256s'])
        accepted.add(previous['after']['sha256'])
    if require_current:
        require(reader.digest(COLLECTOR)==q['after']['sha256'],'Current collector differs from the reviewed budget-only version')
    return dict(record=name,sha256=reader.digest(name),**q,accepted_before_sha256s=sorted(accepted))


def ready_only_change(before,after):
    require(before.count(READY_BEFORE)==1 and READY_AFTER not in before,
            'Unknown optional-live-inventory driver preimage')
    require(before.replace(READY_BEFORE,READY_AFTER)==after,
            'Driver changed outside the exact saved-ready-inventory correction')


POSTPROOF_BEFORE=b'''            self.game_key('attachment-view-'+str(ordinal+1), ord('.'))
            time.sleep(1)
'''
POSTPROOF_AFTER=b'''            if ordinal < 3:
                self.game_key('attachment-view-'+str(ordinal+1), ord('.'))
                time.sleep(1)
'''


def postproof_only_change(before,after):
    require(before.count(POSTPROOF_BEFORE)==1 and POSTPROOF_AFTER not in before,
            'Unknown four-rotation source preimage')
    require(before.replace(POSTPROOF_BEFORE,POSTPROOF_AFTER)==after,
            'Input changed outside the redundant post-proof rotation')


def postproof_phase(reader,name,budget):
    if name is None:return None
    q=reader.json(name)
    require(q['success']is True and q['path']==DRIVER and q['original_game_runs']==0 and
            q['new_route_live_validated']is False and len(q['retained_inputs'])==3,
            'Unaccepted post-proof control review')
    for key in('before','after','inventory_preimage','controls'):
        require(reader.digest(q[key]['record'])==q[key]['sha256'],'Changed post-proof source review')
    ready_only_change(reader.read_bytes(q['inventory_preimage']['record']),reader.read_bytes(q['before']['record']))
    postproof_only_change(reader.read_bytes(q['before']['record']),reader.read_bytes(q['after']['record']))
    require(reader.digest(DRIVER)==q['after']['sha256'],'Current driver differs from reviewed post-proof source')
    controls=reader.json(q['controls']['record'])
    require(controls['success']is True and controls['passed']>=46 and
            {DRIVER,'tools/close-scene-attachment-combat.py','tests/test-scene-attachment-admission.py'}<=set(controls['sources'])and
            all(reader.digest(n)==h for n,h in controls['sources'].items()),'Post-proof controls missing or changed')
    domains=set()
    for item in q['retained_inputs']:
        for ref in item.values():require(reader.digest(ref['record'])==ref['sha256'],'Changed retained post-proof input')
        acquired=reader.json(item['acquisition']['record']);compared=reader.json(item['comparison']['record'])
        output=reader.json(item['analysis']['record'])
        require(acquired['success']is compared['success']is output['success']is True and
                acquired['retirement']['state']=='completed'and compared['acquisition']==item['acquisition']and
                len(output['cases'])==1 and output['cases'][0]['pair']=={k:item[k]for k in('acquisition','comparison')},
                'Unaccepted original post-proof input')
        case=output['cases'][0];proof=case['attachment']
        require(case['purpose']=='attachment'and proof['role']==1 and
                {s['view']for s in proof['states']}==set(range(4))and
                all(v['actual_rgba']==v['original_rgba']for v in proof['native'])and
                {s['view']for s in acquired['attachment_route']['views']}==set(range(4))and
                len([a for a in acquired['actions']if a.get('action','').startswith('attachment-view-')])==4,
                'Old positive input lacks its original four-view proof')
        for sources in(acquired['sources'],compared['sources'],output['analysis_sources']):
            checked_sources(reader,sources,budget,dict(before=q['inventory_preimage']),q)
        domains.add((case['software'],tuple(case['actual_extent'])))
    require(domains=={(False,(800,600)),(True,(800,600)),(False,(640,480))},
            'Post-proof review requires all three accepted attachment domains')
    return dict(record=name,sha256=reader.digest(name),**q)


def ready_input_phase(reader,name,postproof=None):
    if name is None:return None
    q=reader.json(name)
    require(q['success']is True and q['path']==DRIVER and q['original_game_runs']==0,
            'Unaccepted saved-ready input review')
    for key in('before','after','diagnosis','controls'):
        require(reader.digest(q[key]['record'])==q[key]['sha256'],'Changed ready-input review artifact')
    ready_only_change(reader.read_bytes(q['before']['record']),reader.read_bytes(q['after']['record']))
    require((postproof is not None and q['before']==postproof['inventory_preimage']and q['after']==postproof['before'])or
            reader.digest(DRIVER)==q['after']['sha256'],'Current driver differs from the ready-input correction')
    diagnosis=reader.json(q['diagnosis']['record']);controls=reader.json(q['controls']['record'])
    require(diagnosis['kind']=='attachment_live_inventory_snapshot_race'and diagnosis['driver_preimage']==q['before'],
            'Unrelated failed-input diagnosis')
    require(controls['success']is True and controls['passed']>=39 and
            {DRIVER,'tools/close-scene-attachment-combat.py','tests/test-scene-attachment-admission.py',
             'tools/run-rendering-menu-acceptance.py'}<=set(controls['sources'])and
            all(reader.digest(n)==h for n,h in controls['sources'].items()),'Ready-input controls are missing or changed')
    from scene_attachment_journey import spell_inventory
    fields=('owner','offered','counts','assignments','recipes','ingredients','spells')
    domains=set()
    for item in q['retained_inputs']:
        ref=item['acquisition'];require(reader.digest(ref['record'])==ref['sha256'],'Changed retained input acquisition')
        acquired=reader.json(ref['record']);require(acquired['success']is True and acquired['scenario']=='normal-attachment'
            and acquired['sources'][DRIVER]==q['before']['sha256'],'Unaccepted original ready-inventory source phase')
        domain=(acquired['configuration']['software_flag'],acquired['configuration']['requested_size']);domains.add(domain)
        state=acquired['states'][5];require(state['screen']==7 and state['ready'],'Retained visit is not a ready Spellbox')
        saved=next(a for a in acquired['artifacts']if a['path']==state['wire_path'])
        require(item['ready_wire']==dict(record=saved['record'],sha256=saved['sha256'])and
                reader.digest(saved['record'])==saved['sha256'],'Ready snapshot is not the original retained wire')
        raw=reader.read_bytes(saved['record']);words=struct.unpack_from('<8I',raw,128)
        require(words==tuple(state[k]for k in('sequence','generation','screen','ready','ack','status','thread','handoff')),
                'Retained ready visit identity differs')
        observed=spell_inventory(raw);old=acquired['attachment_route']['inventory']
        facts={k:observed[k]for k in fields}
        require(facts=={k:old[k]for k in fields}and
                sha256(json.dumps(facts,sort_keys=True,separators=(',',':')).encode()).hexdigest()==item['inventory_semantics_sha256'],
                'Saved ready inventory changes an originally used selection input')
    require(len(q['retained_inputs'])==2 and domains=={(False,'800x600'),(True,'800x600')},
            'Ready-input applicability requires both exact accepted800 profiles')
    return dict(record=name,sha256=reader.digest(name),**q)


def checked_sources(reader,sources,phase,ready=None,postproof=None):
    for n,h in sources.items():
        if reader.digest(n)==h:continue
        if phase is not None and n==COLLECTOR and h in phase.get('accepted_before_sha256s',[phase['before']['sha256']]):continue
        if ready is not None and n==DRIVER and h==ready['before']['sha256']:continue
        if postproof is not None and n==DRIVER and h==postproof['before']['sha256']:continue
        require(False,'Changed scene source outside the exact host control reviews: '+n)


def retained_cases(reader,names,phase,partial=False,ready=None,postproof=None):
    """Reuse unchanged pixel analysis; review the charging policy separately."""
    register=reader.json('research/runtime/coverage/register.json');builds={b['id']:b for b in register['builds']}
    behaviors=[b for b in register['behaviors']if b['id']in('NP.scene-attachment-output','NP.scene-combat-output')]
    scenario=next(s for s in register['scenarios']if s['id']=='native.scene-attachment-combat-output')
    contracts={b['id']:behavior_contract(b,builds)for b in behaviors};scenario_hash={scenario['id']:scenario_contract(scenario)}
    required=set().union(*(required_sources(b)for b in behaviors))
    cases=[];refs=[]
    for name in names:
        q=reader.json(name)
        require(q['success']is True and q['analysis_sources_stable']is True and q['cases']
                and q['analysis_contracts']==contracts and q['analysis_scenario']==scenario_hash,
                'Unaccepted or differently scoped retained scene analysis')
        require(required<=set(q['analysis_sources']),'Incomplete retained pixel analyzer source closure')
        checked_sources(reader,q['analysis_sources'],phase,ready,postproof)
        cases.extend(deepcopy(q['cases']));refs.append(dict(record=name,sha256=reader.digest(name)))
    expected={(p,s,(w,h))for p in('attachment','combat')for s in(False,True)for w,h in((640,480),(800,600))}
    domains={(c['purpose'],c['software'],tuple(c['actual_extent']))for c in cases}
    require(len(cases)==len(domains)and domains<=expected and(partial or domains==expected)
            and all(c['success']is True and c[c['purpose']]for c in cases),'Incomplete/duplicate scene domain proof')
    complete={p:{d for d in domains if d[0]==p}=={d for d in expected if d[0]==p}for p in('attachment','combat')}
    claims=[]
    for b in behaviors:
        purpose='attachment'if b['id']=='NP.scene-attachment-output'else'combat'
        if not complete[purpose]:continue
        selected=[scenario]if all(complete.values())else[]
        if purpose=='combat':selected.append(next(s for s in register['scenarios']if s['id']=='native.scene-combat-output'))
        claims.append(dict(behavior=b['id'],contract_sha256=contracts[b['id']],scenarios={s['id']:scenario_contract(s)for s in selected}))
    return dict(success=True,analysis_sources_stable=True,task_complete=all(complete.values()),attachment_complete=complete['attachment'],combat_complete=complete['combat'],
        cases=cases,analysis_sources={n:reader.digest(n)for n in required},
        claims=claims),refs


def close_rows(parent,cases,partial=False):
    expected={(p,s,(w,h))for p in('attachment','combat')for s in(False,True)for w,h in((640,480),(800,600))}
    domains={(c['purpose'],c['software'],tuple(c['actual_extent']))for c in cases}
    require(len(cases)==len(domains)and domains<=expected and(partial or domains==expected)
            and len({c['identity']for c in cases})==len(cases),'Missing/duplicate actual scene domain or session')
    require(all(c['success']is True and c[c['purpose']]for c in cases),'Unproved actual scene case')
    rows=deepcopy(parent['requirements']);changed=[]
    for row in rows:
        purpose=LEAVES.get(row['id'])
        if purpose is None:continue
        if partial and row['state']=='passed':continue
        require(row['state']=='pending','Scene requirement was not pending')
        require(row['configuration']in('player-default','player-software'),'Unknown player configuration')
        selected=[c for c in cases if c['purpose']==purpose and c['software']is(row['configuration']=='player-software')]
        if partial and len(selected)!=2:continue
        require(len(selected)==2,'Incomplete scene profile size matrix')
        row.update(state='passed',admitted_execution_evidence=[dict(execution=c['pair'],actual_world_extents=[c['actual_extent']],
            provider=c['provider'],reason=('Actual mode1/Excalibur parent-child ANI/SPR placement in all four stored views, with eight visible source/original/native points per sprite in one current-import paint and complete independent viewport comparison.'if purpose=='attachment'else
            'Actual original wizard/Redcap melee event, exact source-generation health and engagement bracket, changed rendered body frame and eight visible source/original/native points per participant in one current-import paint; complete independent viewport comparison.'))for c in selected])
        changed.append((row['configuration'],row['id']))
    require(1<=len(changed)<=4 and(partial or len(changed)==4),'Scene closeout changed another row count')
    for old,new in zip(parent['requirements'],rows):
        if old['id']not in LEAVES:require(old==new,'Inherited requirement or source phase changed')
    require(sum(r['state']=='passed'for r in rows)==parent['passed_requirements']+len(changed),'Unexpected scene pass count')
    return rows


def attachment_transition(reader,acquired,compared):
    acquired,excluded=scoped_artifacts(acquired)
    poses=decode(artifact(reader,acquired,'scene-actors-0001.bin'));frames=poses['frames']
    route=acquired['attachment_route'];event=compared['scene_attachment_route']['original_cast']
    require(event[4:8]==[83,0,0,0]and event[10]==0 and event[8]>event[9]and event[15]==1,
            'Original attachment cast does not name the living player self target and mana debit')
    raw=artifact(reader,acquired,'spell-events.bin')
    require(len(raw)>=16 and(len(raw)-16)%64==0 and tuple(event)in struct.iter_unpack('<16I',raw[16:]),
            'Named original cast is absent from its exact private session')
    start=route['target']['source_generation'];end=route['first_attachment_generation']
    require(1<=start<end<=len(frames),'Invalid cast/active attachment source interval')
    interval=frames[start-1:end];first,last=interval[0],interval[-1]
    require(event[2]==poses['owner']==first['thread'] and
            ((event[3]-first['tick'])&0xffffffff)<=((last['tick']-first['tick'])&0xffffffff)<30000,
            'Original attachment cast is outside its source-bound observation interval')
    actors=[]
    for frame in interval:
        require(all(frame[k]==first[k]for k in('nonce','thread','pool_epoch','canvas','extent','view')),
                'Attachment target owner/pool/camera changed before active output')
        actor=next((a for a in frame['actors']if a[0]==0),None)
        require(actor and actor[1:3]==(0,0)and actor[3]and 0<actor[4]<2**31,
                'Attachment player tuple/liveness changed before active output')
        actors.append(actor)
    require(actors[0][14]==0 and actors[-1][14]==1 and actors[-1][12]&2 and
            any(d[0:2]==(0,1)for d in last['draws']),'No source-bound unarmed-to-active child transition')
    changes=[dict(generation=frame['generation'],mode=a[14],valid_roles=a[12])
             for i,(frame,a)in enumerate(zip(interval,actors))if i==0 or a[14]!=actors[i-1][14]or i==len(interval)-1]
    first_mode=next(frame['generation']for frame,a in zip(interval,actors)if a[14]==1)
    first_draw=next(frame['generation']for frame,a in zip(interval,actors)
                    if a[14]==1 and a[12]&2 and any(d[0:2]==(0,1)for d in frame['draws']))
    return dict(target_generation=start,driver_first_active_generation=end,
                first_mode1_source_generation=first_mode,first_child_draw_source_generation=first_draw,original_cast=event,
                source_mode_changes=changes,actor_allocation_identity_proved=False,
                scope='Finite exact original self-cast/observed mode transition and active child.30seconds is a driver observation bound, not recovered spell timing, admission/refund formula or complete lifecycle equivalence.')


def join(reader,analysis_name,parent_name,case_names=None,phase_name=None,partial=False,ready_name=None,postproof_name=None):
    phase=collector_phase(reader,phase_name)
    postproof=postproof_phase(reader,postproof_name,phase)
    ready=ready_input_phase(reader,ready_name,postproof)
    if case_names:
        analysis,refs=retained_cases(reader,case_names,phase,partial,ready,postproof);analysis_ref=dict(retained_case_analyses=refs)
    else:
        analysis=reader.json(analysis_name);analysis_ref=dict(record=analysis_name,sha256=reader.digest(analysis_name))
    parent=reader.json(parent_name);plan=reader.json('docs/native-rendering-closure.json')
    require(analysis['success']is True and analysis['analysis_sources_stable']is True and
            (partial or analysis['task_complete']is True and analysis['attachment_complete']is analysis['combat_complete']is True),
            'Incomplete actual eight-case scene analysis')
    require(all(reader.digest(n)==h for n,h in analysis['analysis_sources'].items()),'Changed scene analysis source')
    inventory(plan,parent)
    require(parent['decisions']=={'supported-player':'pending','original-recovery':'pending'},'Separate final decisions changed')
    cases=deepcopy(analysis['cases']);live_sources={};transitions=[]
    for case in cases:
        pair=case['pair'];require(all(reader.digest(v['record'])==v['sha256']for v in pair.values()),'Changed accepted scene pair')
        acquired,compared=[reader.json(pair[k]['record'])for k in('acquisition','comparison')]
        require(compared['acquisition']==pair['acquisition']and acquired['session_identity']==case['identity']
                and acquired['success']is compared['success']is True and acquired['sources_stable']is compared['sources_stable']is True
                and acquired['retirement']['state']=='completed'and acquired['private_wine_stopped']is True,
                'Scene source, actual identity or normal retirement is unaccepted')
        require(acquired['configuration']['software_flag']is case['software']and
                ('--software-rendering'in acquired['command'])is case['software']and
                compared['world_domain']['extents']==[case['actual_extent']], 'Scene execution profile or size relabelled')
        if ready and acquired['sources'].get(DRIVER)==ready['before']['sha256']:
            require(pair['acquisition']in[item['acquisition']for item in ready['retained_inputs']],
                    'Unreviewed original acquisition under the old optional inventory reader')
        for source_phase in(acquired['sources'],compared['sources']):
            checked_sources(reader,source_phase,phase,ready,postproof)
            for n,h in source_phase.items():
                current=reader.digest(n)
                require(n not in live_sources or live_sources[n]==current,'Scene execution source phase changed')
                live_sources[n]=current
        case['provider']=acquired['resident']['renderer']
        if case['software']:require(case['provider'].lower().startswith(('llvmpipe','softpipe')),'Software provider unobserved')
        proof=case[case['purpose']]
        require(proof and all(v['actual_rgba']==v['original_rgba']for v in proof['native']), 'Scene native sample mismatch')
        if case['purpose']=='combat':
            require(compared['scene_combat_route']['verified']is True and combat_pose_change(proof,proof['states']),
                    'No actual engaged melee/changed body proof')
        else:
            require(compared['scene_attachment_route']['verified']is True and proof['role']==1 and proof['branch']=='first-point'
                    and {s['view']for s in proof['states']}==set(range(4)), 'No complete selected original attachment role/views')
            transitions.append(dict(identity=case['identity'],**attachment_transition(reader,acquired,compared)))
    rows=close_rows(parent,cases,partial);register=reader.json('research/runtime/coverage/register.json')
    builds={b['id']:b for b in register['builds']};behaviors={b['id']:b for b in register['behaviors']}
    scenarios={s['id']:s for s in register['scenarios']}
    names=set(required_sources(behaviors[BID]))|{'tools/close-scene-attachment-combat.py'}
    for bid in LIVE:
        names.update(required_sources(behaviors[bid]))
        for sid in LIVE[bid]:names.update(scenarios[sid]['tests'])
    for n in names:live_sources[n]=reader.digest(n)
    result=dict(schema=1,kind='scene_attachment_combat_admission',success=True,
        parent=dict(record=parent_name,sha256=reader.digest(parent_name)),
        analysis=analysis_ref,cases=cases,collector_budget_phase=phase,ready_inventory_input_phase=ready,post_four_view_control_phase=postproof,
        requirements=rows,dossier_identity=parent['dossier_identity'],decisions=deepcopy(parent['decisions']),
        sources=deepcopy(parent['sources']),analysis_sources=dict(analysis['analysis_sources']),
        live_observation_sources=live_sources,closed_leaf_ids=sorted(set(parent['closed_leaf_ids'])|{leaf for leaf in LEAVES if all(r['state']=='passed'for r in rows if r['id']==leaf)}),
        passed_requirements=sum(r['state']=='passed'for r in rows),pending_requirements=sum(r['state']=='pending'for r in rows),
        newly_passed_requirements=sum(r['state']=='passed'for r in rows)-parent['passed_requirements'],task_rows=[r for r in rows if r['id']in LEAVES],
        remaining_task_rows=sum(r['state']!='passed'for r in rows if r['id']in LEAVES),task_complete=analysis['task_complete'],
        original_game_runs=0,current_native_engine_execution=False,current_scene_captures_referenced=True,
        historical_failures_retired=False,attachment_cast_transitions=transitions,
        source_phase_review=dict(parent_sources_preserved=True,new_scene_sources_current=True,
            reason='Inherited accepted rows and their original source/native/compiler/oracle phases remain exact. Actual scene cases independently bind the unchanged passive observer, physical route, original source/history comparison, live import/paint ownership and complete native output. Exact host budget checks and the saved-ready inventory correction may receive separate narrow reviews. The latter verifies identical used inputs from both accepted800 ready wires and current controls; it grants no original execution of the new None-read domain. A separate exact post-four-view control review preserves all three accepted attachment inputs and their historical four camera turns, while current controls verify four observations with three turns. That new route has no live validation from this review. These reviews waive no native/runtime, pixel-analyzer, oracle or protocol hash, unrelated physical input change, or failed start. This retained-data join launches no game and refreshes no historical source hashes.'),
        scope='Admit only scene profile rows with both actual640/800cases. Explicit partial reports preserve every incomplete row pending; default final admission requires all eight domains. Excalibur mode1 parent/child placement spans four stored views and eight visible points per sprite per paint. Actual wizard/Redcap melee binds original event/health/engagement, changed body and eight visible points per participant. Complete comparisons and normal native-only Quit/resource checks remain required. Other status/attachment/caller/map/provider/input/timing/lifetime domains, native gameplay reconstruction and both final decisions remain pending.')
    result['analysis_sources'].update({n:reader.digest(n)for n in required_sources(behaviors[BID])})
    result['claims']=deepcopy(analysis['claims'])+[dict(behavior=BID,contract_sha256=behavior_contract(behaviors[BID],builds),scenarios={SID:scenario_contract(scenarios[SID])})]
    result['live_claims']=[]
    for bid,ids in LIVE.items():
        selected=[sid for sid in ids if (analysis['combat_complete']if sid=='native.scene-combat-first-contact'else analysis['attachment_complete'])]
        if selected:result['live_claims'].append(dict(behavior=bid,contract_sha256=behavior_contract(behaviors[bid],builds),scenarios={sid:scenario_contract(scenarios[sid])for sid in selected}))
    inventory(plan,result)
    return result


def main():
    p=argparse.ArgumentParser(description=__doc__);inputs=p.add_mutually_exclusive_group(required=True)
    inputs.add_argument('--analysis');inputs.add_argument('--case-analysis',action='append',default=[])
    p.add_argument('--collector-phase');p.add_argument('--parent',default=PARENT)
    p.add_argument('--ready-input-phase')
    p.add_argument('--postproof-phase')
    p.add_argument('--partial',action='store_true',help='Preserve incomplete profile rows pending; grants no incomplete size-domain credit')
    p.add_argument('--report',required=True);a=p.parse_args()
    require(a.report.startswith(PREFIX)and '..'not in Path(a.report).parts and not(ROOT/a.report).exists(),'Fresh owned scene admission required')
    controls=[];q={}
    with Reader(ROOT)as reader:
        try:
            q=join(reader,a.analysis,a.parent,a.case_analysis,a.collector_phase,a.partial,a.ready_input_phase,a.postproof_phase)
            for name in('tests/test-scene-attachment-admission.py','tests/test-coverage-ui.py','tests/test-coverage-ui-client.js'):
                command=['node',name]if name.endswith('.js')else[sys.executable,'-B',name]
                r=subprocess.run(command,cwd=ROOT,capture_output=True,text=True,timeout=120)
                controls.append(dict(command=command,exit_code=r.returncode,stdout=r.stdout,stderr=r.stderr))
                require(r.returncode==0,'Scene admission/UI control failed: '+r.stderr)
            q['analysis_sources_stable']=all(reader.digest(n)==h for n,h in q['analysis_sources'].items())
            q['live_observation_sources_stable']=all(reader.digest(n)==h for n,h in q['live_observation_sources'].items())
            require(q['analysis_sources_stable']and q['live_observation_sources_stable'],'Scene admission source changed')
            q['controls']=controls
        except Exception as e:q.update(schema=1,kind='scene_attachment_combat_admission',success=False,error=str(e),controls=controls,claims=[],live_claims=[],original_game_runs=0,newly_passed_requirements=0)
    h=write_report(ROOT,a.report,(json.dumps(q,indent=2)+'\n').encode())
    print(json.dumps(dict(record=a.report,sha256=h,**{k:q.get(k)for k in('success','error','passed_requirements','pending_requirements','remaining_task_rows')})),flush=True)
    return int(not q['success'])


if __name__=='__main__':raise SystemExit(main())
