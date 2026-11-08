#!/usr/bin/env python3
"""Admit only both stretch rows from four complete actual native domains."""
import argparse,json,tempfile
from pathlib import Path
from coverage_evidence import Reader,write_report
from coverage_claims import behavior_contract,scenario_contract,required_sources,fingerprint
from rendering_acceptance import requirements,validate
from rendering_pilot import require
from natural_stretch_copy import analyze
from natural_copy_applicability import execution_review

ROOT=Path(__file__).resolve().parents[1]
PREFIX='research/runtime/rendering-copy-natural-results-20261108/'
BID='NP.natural-stretch-operation-closeout';SID='native.natural-stretch-operation-closeout'
NAMES=['default640-10','default800-02','software640-01','software800-01']
LEAF='operation.copy-stretched'
ANALYSIS=['tools/close-rendering-natural-stretch.py','tools/natural_stretch_copy.py',
 'tools/analyze-rendering-natural-stretch.py','tests/test-natural-stretch-proof.py','tests/test-natural-stretch-admission.py',
 'tools/rendering_acceptance.py','docs/native-rendering-closure.json','tools/rendering_pilot.py',
 'tools/rendering_capture.py','tools/rendering_menu_acceptance.py','tools/rendering_copy_readiness.py',
 'tools/native_copy_routes.py','tools/native_gpu_copy_diagnostics.py','tools/coverage_evidence.py','tools/coverage_claims.py',
 'tools/natural_copy_applicability.py','tests/test-natural-copy-applicability.py']


def validate_cases(cases):
    require(len(cases)==4 and len({c['identity'] for c in cases})==4,'Four distinct natural sessions required')
    require({(c['software'],tuple(c['size'])) for c in cases}==
            {(s,d) for s in (False,True) for d in ((640,480),(800,600))}, 'Missing profile/resolution domain')
    for c in cases:
        require(c['verified'] is True and c['calls']>0 and c['pixels']>0 and c['mismatches']==0
                and c['normal_retirement'] is True and c['actual_gpu_planes'] is True
                and c['native_policy'] is True and c['original_driver_comparison'] is False,
                'Incomplete independent native stretch proof')


def close_rows(plan,parent,cases):
    validate(plan);validate_cases(cases)
    expected=requirements(plan);rows=parent['requirements']
    require(parent['success'] and parent['dossier_identity']==fingerprint(expected)
            and len(rows)==len(expected)==156
            and {(r['configuration'],r['id']) for r in rows}=={(r['configuration'],r['id']) for r in expected}
            and sum(r['state']=='passed' for r in rows)==parent['passed_requirements'],'Parent acceptance identity/count differs')
    output=[];changed=0
    for old in rows:
        row=dict(old)
        if row['id']==LEAF:
            require(row['state']=='pending','Stretch row already admitted')
            row.update(state='passed',admitted_execution_evidence=[dict(acquisition=c['acquisition'],comparison=c['comparison'],
                actual_world_extents=[c['size']],provider=c['provider'],
                reason='Actual original58cbc0 keyed RGB565 stretch arguments during public Grimoire navigation; complete independent CPU destination and immediately following whole GPU source/destination-before readback. Exact native floor sampling policy, preserved key holes/exterior, ordinary original Main Quit and zero native owners. Original driver sampling and natural alias/overlap remain separate.')
                for c in cases if c['software']==(row['configuration']=='player-software')])
            changed+=1
        output.append(row)
    require(changed==2 and all(n==o for n,o in zip(output,rows) if o['id']!=LEAF),'Unrelated acceptance row changed')
    return output


def read_case(reader,name):
    record=PREFIX+name+('-branch-review-02.json' if name=='default640-10' else '-branch-review.json');proof=reader.json(record)
    require(proof['success'] and proof['kind']=='natural_stretch_complete_output_proof'
            and proof['sources_stable'] and all(reader.digest(n)==h for n,h in proof['analysis_sources'].items()),'Changed/incomplete stretch analysis')
    ref=proof['acquisition'];require(reader.digest(ref['record'])==ref['sha256'],'Acquisition changed')
    acquired=reader.json(ref['record'])
    require(execution_review(reader,acquired)==proof['execution_source_review'],'Native execution source review changed')
    with tempfile.TemporaryDirectory(prefix='stretch-admission-') as scratch:
        fresh=json.loads(json.dumps(analyze(reader,acquired,scratch)))
    require(all(proof[k]==v for k,v in fresh.items()),'Retained raw output proof differs')
    require([p['target_size'] for p in proof['proofs']]==[proof['configuration']['actual_size']]*proof['natural_stretch_calls'],
            'Actual destination size differs')
    from rendering_menu_acceptance import artifact
    resident=json.loads(artifact(reader,acquired,'resident-world-report.json'))
    graphics=acquired['retirement']['graphics_retirement']
    require(resident['normal_quit_retired'] and not resident['pending'] and not resident['error']
            and resident['stage_profile']['capture_complete'] and int(resident['source_chunks'])>0
            and graphics['resources_released'] and not graphics['surviving_owners']
            and all(v==0 for v in graphics['counts'].values()) and acquired['retirement']['forwarded']==0,
            'Native source-prefix/graphics lifetime incomplete')
    if proof['configuration']['software']:
        require(resident['renderer'].lower().startswith(('llvmpipe','softpipe')),'Software provider unproved')
    return dict(identity=acquired['session_identity'],software=proof['configuration']['software'],
        size=proof['configuration']['actual_size'],verified=True,calls=proof['natural_stretch_calls'],
        pixels=proof['compared_pixels'],mismatches=proof['mismatching_pixels'],normal_retirement=True,
        actual_gpu_planes=all(p['gpu_checkpoint_verified'] for p in proof['proofs']),
        native_policy=proof['native_policy'],original_driver_comparison=proof['original_driver_comparison'],
        acquisition=ref,comparison=dict(record=record,sha256=reader.digest(record)),provider=resident['renderer'],
        tracked_alias_calls=proof['tracked_alias_calls'])


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--parent',default='research/runtime/rendering-recovery-four-results-20261108/master-admission-01.json')
    p.add_argument('--report',required=True);a=p.parse_args()
    require(a.report.startswith(PREFIX) and not (ROOT/a.report).exists(),'Fresh owned admission required')
    with Reader(ROOT) as reader:
        sources={n:reader.digest(n) for n in ANALYSIS};cases=[read_case(reader,n) for n in NAMES]
        parent=reader.json(a.parent);rows=close_rows(reader.json('docs/native-rendering-closure.json'),parent,cases)
        reg=reader.json('research/runtime/coverage/register.json')
        behavior=next(b for b in reg['behaviors'] if b['id']==BID);scenario=next(s for s in reg['scenarios'] if s['id']==SID)
        require(required_sources(behavior)==set(ANALYSIS),'Admission source closure differs')
        require(sources=={n:reader.digest(n) for n in ANALYSIS},'Admission sources changed')
        result=dict(schema=1,kind='natural_stretch_operation_admission',success=True,sources_stable=True,
            analysis_sources=sources,sources=parent['sources']|{n:h for n,h in sources.items() if n not in parent['sources']},parent_source_phases=parent['sources'],
            cases=cases,requirements=rows,verified_domains=4,required_domains=4,newly_passed_requirements=2,
            passed_requirements=parent['passed_requirements']+2,pending_requirements=parent['pending_requirements']-2,
            closed_leaf_ids=sorted(set(parent['closed_leaf_ids'])|{LEAF}),dossier_identity=parent['dossier_identity'],
            decisions=parent['decisions'],parent=dict(record=a.parent,sha256=reader.digest(a.parent)),
            claims=[dict(behavior=BID,contract_sha256=behavior_contract(behavior,{}),scenarios={SID:scenario_contract(scenario)})],
            original_game_runs=0,verified_positive_original_starts=4,checked_pixels=sum(c['pixels'] for c in cases),
            mismatching_pixels=0,current_whole_native_execution=False,original_driver_comparison=False,
            historical_failures_retired=False,alias_rows_closed=False)
    digest=write_report(ROOT,a.report,(json.dumps(result,indent=2)+'\n').encode())
    print(json.dumps(dict(success=True,record=a.report,sha256=digest,newly_passed=2,
                         passed=result['passed_requirements'],pending=result['pending_requirements'])))


if __name__=='__main__':main()
