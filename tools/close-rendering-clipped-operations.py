#!/usr/bin/env python3
"""Close only clipped-copy rows from four exact owner-thread operation cases."""
import argparse
import json
from pathlib import Path
from coverage_evidence import Reader,write_report
from coverage_claims import fingerprint
from rendering_acceptance import requirements,validate
from rendering_pilot import require

ROOT=Path(__file__).resolve().parents[1]
PREFIX='research/runtime/rendering-clipped-operations-results-20261107/'
BID='NP.clipped-operation-closeout'
LEAF='operation.copy-clipped'
SCOPE=('Clipped CPU/GPU COPY on the original World owner thread in four native-only '
       'default/software actual640/800 domains. Exact provider RGNDATA inputs, original '
       'caller/result/clipper, independent ordered plans and complete selected raw actual '
       'source/prior/destination planes including every exterior WORD value. Other '
       'threads, global truncated C4, whole World/viewport, fullscreen/physical driver '
       'and whole ABI are excluded. Every parent attachment and both decisions remain.')


def sources(reader):
    b=next(b for b in reader.json('research/runtime/coverage/register.json')['behaviors'] if b['id']==BID)
    return {n:reader.digest(n) for role in ('implementation','tests','validation_dependencies') for n in b[role]}


def join(reader,parent,comparisons,plan):
    validate(plan)
    require(parent['success'] and parent['dossier_identity']==fingerprint(requirements(plan))
            and len(parent['requirements'])==156,'Changed accepted requirement inventory')
    require(len(comparisons)==4 and len(set(comparisons))==4,'Four distinct clipping cases required')
    cases=[]
    for name in comparisons:
        q=reader.json(name)
        require(q['kind']=='public_native_clipping_comparison' and q['success'] and q['analysis_completed']
                and q['analysis_sources_stable'] and q['complete_output_verified']
                and q['native_clip_regions_verified'] and q['positive_clipped_calls']>0
                and all(reader.digest(n)==h for n,h in q['analysis_sources'].items()),'Incomplete/changed clipping case')
        c=q['case'];a=reader.json(c['acquisition']['record'])
        require(c['acquisition']==q['acquisition'] and reader.digest(c['acquisition']['record'])==c['acquisition']['sha256']
                and a['success'] and a['sources_stable'] and c['source_domain']['verified']
                and c['proof']['primary_identity_verified'] and c['proof']['normal_lifetime_verified']
                and c['proof']['actual_clip_region_verified'] and c['proof']['caller_clip_plan_verified']
                and c['region_scope']['owner_records_complete'] and c['region_bound_callers']>0,
                'Missing clipping case identity/lifetime/input admission')
        witnesses=c['raw_pixel_witnesses']
        require(len(witnesses)>=2 and {w['caller'][15] for w in witnesses}=={1,2}
                and all(w['actual_clip_region_snapshot_retained'] and w['clip_plan']
                        and w['changed_words']>0 and w['preserved_exterior_words']>0
                        and w['full_input_and_actual_output_verified'] for w in witnesses),
                'Missing meaningful complete CPU/GPU clipping output')
        cases.append(dict(domain=[c['profile'],c['actual_size']],identity=a['session_identity'],
            execution=dict(acquisition=c['acquisition'],comparison=dict(record=name,sha256=reader.digest(name))),
            scope=SCOPE,clip_callers=c['region_bound_callers'],pixel_witnesses=len(witnesses),
            primary_identity=c['identity'],normal_lifetime=c['terminal'],
            source_phase=a['sources'],analysis_source_phase=q['analysis_sources']))
    require({(c['domain'][0],tuple(c['domain'][1])) for c in cases}==
            {(p,s) for p in ('player-default','player-software') for s in ((640,480),(800,600))}
            and len({c['identity'] for c in cases})==4,'Missing/repeated clipping profile/actual-size domain')
    rows=[]
    for old in parent['requirements']:
        row=dict(old)
        additions=[dict(execution=c['execution'],actual_world_extents=[c['domain'][1]],reason=SCOPE)
                   for c in cases if c['domain'][0]==row['configuration']]
        if row['id']==LEAF:
            require(row['state']=='pending','Clipped COPY already accepted')
            row.update(state='passed',admitted_execution_evidence=additions)
        elif row['id'] in ('gate.identity','gate.normal-lifetime'):
            require(row['state']=='passed','Primary admission gate was not already accepted')
            row['admitted_execution_evidence']=old['admitted_execution_evidence']+additions
        rows.append(row)
    require(sum(r['state']=='passed' for r in rows)==parent['passed_requirements']+2,'Promotion exceeds two operation rows')
    return dict(schema=1,kind='public_native_clipped_copy_closeout',success=True,scope=SCOPE,
        dossier_identity=parent['dossier_identity'],requirements=rows,decisions=parent['decisions'],cases=cases,
        closed_leaf_ids=sorted(set(parent['closed_leaf_ids'])|{LEAF}),passed_requirements=parent['passed_requirements']+2,
        pending_requirements=154-parent['passed_requirements'],newly_passed_requirements=2,
        actual_cases=4,complete_pixel_witnesses=sum(c['pixel_witnesses'] for c in cases),
        current_native_engine_execution=True,new_fixture=False,whole_copy_diagnostic_accepted=False,
        sources=dict(parent['sources']))


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--comparison',action='append',required=True)
    p.add_argument('--parent',required=True);p.add_argument('--report',required=True);args=p.parse_args()
    require(args.report.startswith(PREFIX) and not (ROOT/args.report).exists(),'Fresh owned clipping join required')
    result=dict(schema=1,success=False)
    try:
        with Reader(ROOT) as r:
            before=sources(r)
            result=join(r,r.json(args.parent),args.comparison,r.json('docs/native-rendering-closure.json'))
            require(before==sources(r),'Clipping join sources changed')
            result.update(analysis_sources=before,analysis_sources_stable=True,
                parent=dict(record=args.parent,sha256=r.digest(args.parent)))
            result['sources'].update({n:h for n,h in before.items() if n not in result['sources']})
    except Exception as error:result=dict(schema=1,success=False,error=str(error))
    h=write_report(ROOT,args.report,(json.dumps(result,indent=2)+'\n').encode())
    print(json.dumps(dict(success=result['success'],error=result.get('error'),sha256=h)))
    return int(not result['success'])


if __name__=='__main__':raise SystemExit(main())
