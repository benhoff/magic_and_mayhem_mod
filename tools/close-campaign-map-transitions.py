#!/usr/bin/env python3
"""Admit only map exit/loading from four complete, independently checked domains."""
import argparse
import hashlib
import importlib.util
import json
from pathlib import Path
import struct

from campaign_map_resources import assess
from campaign_transition_review import semantic_handoff,native_handoff,referenced_output,LEAVES
from coverage_claims import behavior_contract,scenario_contract,required_sources,fingerprint
from coverage_evidence import Reader,write_report
from rendering_acceptance import requirements,validate
from rendering_capture import capture
from rendering_menu_acceptance import artifact
from rendering_pilot import require,sha

ROOT=Path(__file__).resolve().parents[1]
BID='NP.campaign-map-transition-closeout'
SID='native.campaign-map-transition-closeout'
RESOURCE_BID='NP.campaign-map-resource-boundaries'
RESOURCE_SID='native.campaign-map-resource-boundaries'
OUT='research/runtime/campaign-progression-closeout-results-20261108/'
PARENT='research/runtime/rendering-copy-natural-results-20261108/master-admission-01.json'


def module(name):
    spec=importlib.util.spec_from_file_location(name.replace('-','_'),ROOT/'tools'/(name+'.py'))
    m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);return m


def bound(reader,ref):
    raw=reader.read_bytes(ref['record']);require(sha(raw)==ref['sha256'],'Changed report reference')
    return json.loads(raw)


def command_history(reader,a):
    refs=[r for r in a['artifacts'] if '/capture/commands-' in r['path'] and r['path'].endswith('.bin')]
    require(len(refs)==1 and 0<refs[0]['bytes']<=1024**3,'Missing/oversized complete command history')
    raw=artifact(reader,a,refs[0]['path']);at=8;decoded=0
    require(raw[:8]==b'MNMACE01','Wrong command capture envelope')
    while at<len(raw):
        require(len(raw)-at>=8,'Truncated command envelope')
        m,n=struct.unpack_from('<II',raw,at);at+=8
        require(0<m<=65536 and n<=33*1024**2 and m+n<=len(raw)-at,'Command envelope extent')
        at+=m;require(not n or n>=4,'Compressed command header')
        decoded+=struct.unpack_from('>I',raw,at)[0] if n else 0
        require(decoded<=2*1024**3,'Decoded command RAM budget exceeded');at+=n
    rows=capture(raw)
    require(rows[0][0]['session']==a['session_identity'] and rows[0][0]['role']=='commands'
            and rows[-2][0]['kind']=='ended','Mixed/incomplete command capture')
    return b''.join(body for h,body in rows if h['kind']=='fragment')


def domain(a,c,ref):
    require(a['kind']=='public_native_progression_acquisition'
            and c['kind']=='public_native_progression_comparison' and c['acquisition']==ref,
            'Diagnostic, reload or mixed comparison cannot supply nominal map progression')
    for r in (a,c):
        require(r['success'] is r['sources_stable'] is r['original_manifest_verified_before_after']
                is r['artifact_retention_complete'] is True
                and r['original_pixels_used_as_native_inputs'] is False,'Incomplete independent nominal phase')
    if a.get('entry_checkpoint'):
        entry=a['entry_checkpoint'];loaded=a.get('entry_loaded_snapshot',{})
        require(a['configuration'].get('entry_mode')=='original-checkpoint' and loaded.get('verified') is True
            and loaded.get('sha256')==entry['sha256'] and a['flow'].get('entry_load_verified') is True
            and a['flow'].get('entry_checkpoint_sha256')==entry['sha256'],'Initial original checkpoint input/Load proof incomplete')
    else:require(a['configuration'].get('entry_mode','new-game')=='new-game','Unbound initial Load cannot supply progression')
    cfg=a['configuration'];size=list(map(int,cfg['requested_size'].split('x')))
    require(size in ([640,480],[800,600]) and cfg['id'] in ('player-default','player-software')
            and cfg['software_flag'] is (cfg['id']=='player-software') and cfg['skip_intros'] is False
            and cfg['diagnostic_only'] is False and cfg['evidence_mode']=='full'
            and cfg['capture_policy']['pixel_comparison_eligible'] is True
            and cfg['terrain_lighting']=='native' and cfg['native_lighting_capture'] is False
            and cfg['map_resource_observation']=='bounded-world-resource-boundaries-v1',
            'Nominal map/resource configuration differs')
    require(a['exit_code']==0 and a['private_wine_stopped'] is a['compiler_closure_stable']
            is a['player_binary_stable'] is c['comparison_processes_stopped'] is True
            and a['retirement']['state']=='completed' and a['retirement']['forwarded']==0,
            'Incomplete normal private process/render retirement')
    require(c['player_build_binding']['verified'] is True
            and c['player_build_binding']['build']==a['player_build'],'Unbound tested native player')
    require(c['world_domain']['verified'] is True and c['world_domain']['format']=='RGB565'
            and c['world_domain']['extents']==[size],'Original World extent/format differs')
    if cfg['software_flag']:
        require(a['resident']['renderer'].lower().startswith(('llvmpipe','softpipe')),'Actual software provider unproved')
    cap=a['resident']['stage_profile']['capture']
    n=int(a['resident']['imports'])
    require(n==int(a['resident']['verified_returns'])==int(a['resident']['completed'])
            ==c['original_reference']['worlds']==cap['accepted_worlds']==cap['written_worlds']>0
            and cap['complete'] is cap['finished'] is True and cap['truncated'] is False
            and not cap['error'] and cap['queue_bytes']==0 and not a['resident']['pending'],
            'Incomplete source/import/return history')
    return cfg['id'],size,n


CONTROLLER_SOURCES=frozenset(('tools/run-native-game.py','tools/run-rendering-progression.py',
    'tools/prepare-rendering-progression.py','tools/campaign-outcome-input.py',
    'tools/campaign_wizard_cast.py','tools/campaign-portal-outcome-input.py','tools/campaign_retained_policies.py'))


def controller_source_review(sources,current,review,acquisition_ref,comparison_ref,controls):
    drift={p for p,h in sources.items() if current.get(p)!=h}
    require(drift and drift<=CONTROLLER_SOURCES,'Runtime/native/oracle/capture drift cannot use a controller-only review')
    require(review.get('kind')=='campaign_controller_source_applicability' and review.get('success') is True
        and review.get('original_game_runs')==0 and review.get('historical_hashes_rewritten') is False
        and review.get('current_policy_execution_claimed') is False,'Unscoped controller source review')
    require(review.get('acquisition')==acquisition_ref and review.get('comparison')==comparison_ref,
        'Controller review belongs to another execution/comparison')
    require(review.get('execution_sources')==sources and review.get('current_sources')==current
        and set(review.get('changed_sources',{}))==drift,'Controller review source identities differ')
    require(controls.get('success') is controls.get('sources_stable') is True
        and controls.get('original_game_executed') is False,'Missing current controller controls')
    for path in drift:
        row=review['changed_sources'][path]
        require(row.get('before')==sources[path] and row.get('after')==current[path]
            and isinstance(row.get('reason'),str) and row['reason'].strip()
            and controls.get('sources',{}).get(path)==current[path],'Incomplete controller impact or current control binding')
    return dict(verified=True,changed_sources=sorted(drift),native_runtime_oracle_capture_hashes_unchanged=True,
        current_input_policy_execution_claimed=False,scope='Preserve the exact independently compared native map execution. Controller changes have current scoped controls; its old actual input policy remains an explicit historical phase.')


def analyze(reader,path,controller_reviews=()):
    a=reader.json(path);ref=dict(record=path,sha256=reader.digest(path))
    comparison=path.removesuffix('.json')+'-comparison.json'
    c=reader.json(comparison);profile,size,n=domain(a,c,ref)
    current={p:reader.digest(p) if (reader.root/p).is_file() else None for p in c['sources']}
    drift={p for p,h in c['sources'].items() if current[p]!=h};source_review=None
    if drift:
        comparison_ref=dict(record=comparison,sha256=reader.digest(comparison))
        matches=[(reference,review) for reference,review in controller_reviews
            if review.get('acquisition')==ref and review.get('comparison')==comparison_ref]
        require(len(matches)==1,'Current execution/oracle source drift requires one exact controller-only review: '+', '.join(sorted(drift)))
        reference,review=matches[0];controls=bound(reader,review['controls'])
        require(all(reader.digest(p)==h for p,h in controls['sources'].items()),'Controller control source drift')
        source_review=controller_source_review(c['sources'],current,review,ref,comparison_ref,controls)|dict(report=reference)

    events=artifact(reader,a,'events.bin');original=semantic_handoff(a['flow'],events)
    resources=assess(artifact(reader,a,'map-resource-boundaries.bin'),a['flow'])
    first,later=c['original_menu_actions']['original_world_epochs']
    require(resources['initial_resources']['menu_event']+1==first['first_sequence']
            and resources['later_resources']['menu_event']+1==later['first_sequence'],
            'Resource epochs do not bind the original World tick observations')
    retired=resources['retirements'][0]['after'];pop=original['original_world_pop']
    require(retired['menu_event']+2==pop[0] and retired['thread']==pop[2]
            and [retired['stack_owner'],retired['depth']]==list(pop[11:13]),
            'Original resource retirement does not bind the observed World pop')
    raw=command_history(reader,a);native=native_handoff(raw,a['flow']['worlds'][1]['world'],size,prior_generation=a['flow']['worlds'][1]['after_world'])
    output=referenced_output(c,a['flow'],sha(raw));require(native['imports']==n,'Native World import count differs')
    normal=module('join-rendering-normal-acceptance');exit_module=module('close-rendering-world-exit')
    cleanup=normal.cleanup_proof(reader,a);imports=exit_module.imported_lifetimes(raw)
    terminal=exit_module.terminal_source(artifact(reader,a,'world-continuous.bin'),
        artifact(reader,a,'world-terminal-0001.bin'),imports,original['later_entry']['thread'])
    require(cleanup['verified'] is cleanup['normal_quit'] is True
            and cleanup['pending_jobs']==cleanup['graphics_owners']==0
            and terminal['native_owned_slots']==0,'Incomplete final ownership retirement')
    return dict(identity=a['session_identity'],configuration=profile,extent=size,
                acquisition=ref,comparison=dict(record=comparison,sha256=reader.digest(comparison)),
                controller_source_applicability=source_review,
                original=original,resources=resources,native=native,cleanup=cleanup,terminal_source=terminal,
                output=output,provider=a['resident']['renderer'],verified=True)


def close_rows(plan,parent,cases):
    validate(plan)
    require(parent['success'] and parent['dossier_identity']==fingerprint(requirements(plan))
            and len(parent['requirements'])==156 and parent['passed_requirements']==116
            and parent['pending_requirements']==40,'Acceptance parent identity/count differs')
    expected={(p,s) for p in ('player-default','player-software') for s in ((640,480),(800,600))}
    require(len(cases)==4 and {(c['configuration'],tuple(c['extent'])) for c in cases}==expected
            and len({c['identity'] for c in cases})==4 and all(c['verified'] for c in cases),
            'Four distinct complete actual domain proofs required')
    rows=[]
    for old in parent['requirements']:
        row=dict(old)
        if row['id'] in LEAVES:
            require(row['state']=='pending','Requested row already accepted')
            row.update(state='passed',admitted_execution_evidence=[dict(acquisition=c['acquisition'],
                comparison=c['comparison'],actual_world_extents=[c['extent']],provider=c['provider'],
                reason='Fresh natural region1 exit/Realm/travel/region2 loading. Paired original populated bitmap/queue owner retirement before fresh resource acquisition, explicit persistent display identities, native prior World surface retirement before later creation, complete independent continuous original-history viewport output and normal terminal ownership drain.')
                for c in cases if c['configuration']==row['configuration']])
        rows.append(row)
    require(sum(r['state']=='passed' for r in rows)==120
            and all(n==o for n,o in zip(rows,parent['requirements']) if o['id'] not in LEAVES),
            'Map closeout exceeds the requested four configuration rows')
    return rows


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--acquisition',action='append',required=True)
    p.add_argument('--report',required=True);p.add_argument('--controller-review',action='append',default=[]);args=p.parse_args()
    require(args.report.startswith(OUT) and '..' not in Path(args.report).parts and not (ROOT/args.report).exists(),
            'Fresh owned immutable report required')
    q=dict(schema=1,kind='four_domain_map_exit_loading_acceptance',success=False,newly_passed_requirements=0)
    try:
        with Reader(ROOT) as r:
            reg=r.json('research/runtime/coverage/register.json');b=next(b for b in reg['behaviors'] if b['id']==BID)
            s=next(s for s in reg['scenarios'] if s['id']==SID)
            resource=next(v for v in reg['behaviors'] if v['id']==RESOURCE_BID);resource_scenario=next(v for v in reg['scenarios'] if v['id']==RESOURCE_SID)
            sources={p:r.digest(p) for p in sorted(required_sources(b)|required_sources(resource))}
            reviews=[(dict(record=name,sha256=r.digest(name)),r.json(name)) for name in args.controller_review]
            cases=[];q['cases']=cases
            for path in args.acquisition:
                c=analyze(r,path,reviews);cases.append(c);print(json.dumps(dict(domain=[c['configuration'],c['extent']],verified=True)),flush=True)
            parent=r.json(PARENT);rows=close_rows(r.json('docs/native-rendering-closure.json'),parent,cases)
            require(all(r.digest(p)==h for p,h in sources.items()),'Mixed analysis source phase')
            q.update(success=True,sources_stable=True,analysis_sources=sources,
                sources=parent['sources']|{p:h for p,h in sources.items() if p not in parent['sources']},
                parent=dict(record=PARENT,sha256=r.digest(PARENT)),parent_source_phases=parent['sources'],
                dossier_identity=parent['dossier_identity'],requirements=rows,decisions=parent['decisions'],
                closed_leaf_ids=sorted(set(parent['closed_leaf_ids'])|set(LEAVES)),
                passed_requirements=120,pending_requirements=36,newly_passed_requirements=4,
                verified_domains=4,required_domains=4,
                claims=[dict(behavior=BID,contract_sha256=behavior_contract(b,{v['id']:v for v in reg['builds']}),
                             scenarios={SID:scenario_contract(s)}),
                        dict(behavior=RESOURCE_BID,contract_sha256=behavior_contract(resource,{v['id']:v for v in reg['builds']}),
                             scenarios={RESOURCE_SID:scenario_contract(resource_scenario)})],
                scope='Only two map transition leaves in default/software actual640/800. Simulation and external '
                      'allocator internals remain admitted original suppliers; persistent display providers span maps. '
                      'No victory/next-map leaf, original full-recovery or final supported-player decision promotion.')
    except Exception as e:q['error']=str(e)
    data=(json.dumps(q,indent=2)+'\n').encode();h=write_report(ROOT,args.report,data)
    print(json.dumps(dict(success=q['success'],error=q.get('error'),sha256=h,report=args.report)),flush=True)
    return int(not q['success'])


if __name__=='__main__':raise SystemExit(main())
