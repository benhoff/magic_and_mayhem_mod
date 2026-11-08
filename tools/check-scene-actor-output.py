#!/usr/bin/env python3
"""Retain actual scene/source/health/native joins without automatic row promotion."""
import argparse
import json
from pathlib import Path

from coverage_claims import behavior_contract,required_sources,scenario_contract
from coverage_evidence import Reader,write_report
from rendering_pilot import require
from scene_actor_output import ROOT,PREFIX,analyze_case


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--case',nargs=2,action='append',default=[],metavar=('ACQUISITION','COMPARISON'))
    p.add_argument('--case-analysis',action='append',default=[],help='Reuse an unchanged, complete current per-case analysis; verify its exact contracts, sources and execution pairs')
    p.add_argument('--report',required=True);a=p.parse_args();path=Path(a.report)
    require(a.report.startswith(PREFIX) and '..' not in path.parts and not(ROOT/path).exists(),'New owned immutable report required')
    require(a.case or a.case_analysis,'Actual execution pair or current retained case analysis required')
    q=dict(schema=1,kind='actual_scene_actor_output_analysis',success=False,cases=[],newly_passed_requirements=0,
           original_game_runs=0,task_complete=False,attachment_complete=False,combat_complete=False)
    with Reader(ROOT)as reader:
        register=reader.json('research/runtime/coverage/register.json')
        bids={'NP.scene-attachment-output','NP.scene-combat-output'}
        behaviors=[b for b in register['behaviors'] if b['id'] in bids]
        names={'tools/check-scene-actor-output.py','tools/scene_actor_output.py'}
        for b in behaviors:names.update(required_sources(b))
        sources={n:reader.digest(n) for n in sorted(names)}
        q['analysis_sources']=sources
        builds={v['id']:v for v in register['builds']}
        scenario=next(s for s in register['scenarios']if s['id']=='native.scene-attachment-combat-output')
        q['analysis_contracts']={b['id']:behavior_contract(b,builds)for b in behaviors}
        q['analysis_scenario']={scenario['id']:scenario_contract(scenario)}
        q['retained_case_analyses']=[]
        try:
            for acq,cmp in a.case:
                acquired,compared=reader.json(acq),reader.json(cmp)
                pair=dict(acquisition=dict(record=acq,sha256=reader.digest(acq)),comparison=dict(record=cmp,sha256=reader.digest(cmp)))
                require(compared['acquisition']==pair['acquisition'],'Comparison is bound to another execution')
                case=analyze_case(reader,acquired,compared)
                q['cases'].append(dict(pair=pair,identity=acquired['session_identity'],**case))
            for name in a.case_analysis:
                saved=reader.json(name)
                require(saved['success']is True and saved['analysis_sources_stable']is True and saved['cases']
                        and saved.get('analysis_contracts')==q['analysis_contracts']and saved.get('analysis_scenario')==q['analysis_scenario'],
                        'Retained case analysis has another contract or incomplete execution')
                require(set(sources)<=set(saved['analysis_sources'])and
                        all(reader.digest(n)==h for n,h in saved['analysis_sources'].items()),'Retained case analyzer source changed')
                for case in saved['cases']:
                    pair=case['pair']
                    require(all(reader.digest(v['record'])==v['sha256']for v in pair.values()),'Retained execution pair changed')
                    acquired,compared=[reader.json(pair[k]['record'])for k in('acquisition','comparison')]
                    require(compared['acquisition']==pair['acquisition']and acquired['session_identity']==case['identity']
                            and acquired['success']is compared['success']is True,'Retained case execution binding changed')
                    require(all(reader.digest(n)==h for phase in(acquired['sources'],compared['sources'])for n,h in phase.items()),
                            'Retained case execution/comparison source changed')
                    q['cases'].append(case)
                q['retained_case_analyses'].append(dict(record=name,sha256=reader.digest(name)))
            expected={(s,(w,h)) for s in(False,True) for w,h in ((640,480),(800,600))}
            domains={(c['purpose'],c['software'],tuple(c['actual_extent'])) for c in q['cases']}
            require(len(domains)==len(q['cases']) and len({c['identity']for c in q['cases']})==len(q['cases']),'Duplicate scene execution/domain')
            combat=[c for c in q['cases']if c['purpose']=='combat']
            attachment=[c for c in q['cases']if c['purpose']=='attachment']
            q['combat_complete']={(c['software'],tuple(c['actual_extent']))for c in combat}==expected and all(c['combat']for c in combat)
            q['attachment_complete']={(c['software'],tuple(c['actual_extent']))for c in attachment}==expected and all(c['attachment']for c in attachment)
            q['task_complete']=q['combat_complete'] and q['attachment_complete']
            q['success']=True
            if q['task_complete']:
                q['claims']=[dict(behavior=b['id'],contract_sha256=behavior_contract(b,builds),
                     scenarios={scenario['id']:scenario_contract(scenario)})for b in behaviors]
            else:q['claims']=[]
        except Exception as error:q['error']=str(error);q['claims']=[]
        q['analysis_sources_stable']=all(reader.digest(n)==h for n,h in sources.items())
        q['success'] &= q['analysis_sources_stable']
    h=write_report(ROOT,a.report,(json.dumps(q,indent=2)+'\n').encode())
    print(json.dumps(dict(record=a.report,sha256=h,**{k:q.get(k)for k in('success','error','combat_complete','attachment_complete','task_complete')})))
    return int(not q['success'])


if __name__=='__main__':raise SystemExit(main())
