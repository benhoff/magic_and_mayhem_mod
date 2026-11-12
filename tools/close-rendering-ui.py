#!/usr/bin/env python3
"""Admit exactly six HUD/tooltip/pause rows from four independently proved cases."""
import argparse
from copy import deepcopy
import importlib.util
import json
from pathlib import Path
from coverage_evidence import Reader,write_report
from coverage_claims import behavior_contract,scenario_contract,required_sources
from rendering_closeout import inventory
from rendering_ui_acceptance import PREFIX,LEAVES
from rendering_pilot import require
ROOT=Path(__file__).resolve().parents[1]
BID='NP.hud-tooltip-pause-acceptance'
SID='UI07.public-ui'
PARENT='research/runtime/rendering-enabled-intros-results-20261107/integrated-acceptance-03.json'


def contracts(reader):
    register=reader.json('research/runtime/coverage/register.json')
    b=next(v for v in register['behaviors'] if v['id']==BID)
    s=next(v for v in register['scenarios'] if v['id']==SID)
    sources={n:reader.digest(n) for n in sorted(required_sources(b))}
    return sources,[dict(behavior=BID,contract_sha256=behavior_contract(b,{v['id']:v for v in register['builds']}),scenarios={SID:scenario_contract(s)})]


def close_rows(plan,parent,cases):
    inventory(plan,parent)
    require(parent['passed_requirements']==88 and len(parent['requirements'])==156,'Changed 88-row parent')
    require(len(cases)==4 and len({v['identity'] for v in cases})==4
        and {(v['software'],tuple(v['size'])) for v in cases}=={(s,z) for s in (False,True) for z in ((800,600),(640,480))}
        and all(v['verified'] is True for v in cases),'Missing/duplicate/incomplete UI domains')
    rows=deepcopy(parent['requirements']);changed=set()
    for row in rows:
        if row['id'] not in LEAVES:continue
        require(row['state']=='pending','UI target was already accepted or failed')
        row.update(state='passed',admitted_execution_evidence=[dict(execution=v['execution'],reason='Original state/text/key/Mini callbacks and inactive owners joined to independently matched complete native output and normal original Quit.') for v in cases if v['software'] is (row['configuration']=='player-software')])
        changed.add((row['configuration'],row['id']))
    require(changed=={(p,n) for p in ('player-default','player-software') for n in LEAVES}
            and sum(v['state']=='passed' for v in rows)==94,'UI closeout exceeded exactly six rows')
    return rows


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--comparison',action='append',required=True);p.add_argument('--report',required=True);args=p.parse_args()
    require(args.report.startswith(PREFIX) and '..' not in Path(args.report).parts and not (ROOT/args.report).exists(),'Fresh immutable UI join required')
    result=dict(schema=1,success=False)
    try:
        with Reader(ROOT) as r:
            sources,claims=contracts(r);cases=[];phases=[];validation_sources=dict(sources)
            for name in args.comparison:
                q=r.json(name)
                require(q['kind']=='public_native_ui_comparison' and q['success'] and q['sources_stable'] and q['comparison_processes_stopped']
                    and q['comparison']['success'] and q['comparison']['mismatching_pixels']==0 and q['ui_proof']['verified']
                    and q['claims']==claims and all(r.digest(n)==h for n,h in q['sources'].items()),'Incomplete/changed UI proof')
                a=r.json(q['acquisition']['record']);require(r.digest(q['acquisition']['record'])==q['acquisition']['sha256'] and a['success'],'Changed primary execution')
                for file,fn,field in [('close-rendering-retained-rows','check_retirement','normal_lifetime'),('close-rendering-identity-gates','check_case','identity')]:
                    spec=importlib.util.spec_from_file_location('ui_join_'+file,ROOT/'tools'/(file+'.py'));m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);getattr(m,fn)(q[field])
                cases.append(dict(identity=a['session'],software=a['configuration']['software_flag'],size=a['configuration']['actual_size'],verified=True,
                    execution=dict(acquisition=q['acquisition'],comparison=dict(record=name,sha256=r.digest(name)))))
                phases.append(dict(record=name,sha256=r.digest(name),sources=q['sources']))
                require(all(n not in validation_sources or validation_sources[n]==h for n,h in q['sources'].items()),'Mixed current UI validation source phases')
                validation_sources.update(q['sources'])
            parent=r.json(PARENT);rows=close_rows(r.json('docs/native-rendering-closure.json'),parent,cases)
            requests=[]
            for path in sorted((ROOT/PREFIX).glob('*-acquisition.json')):
                name=path.relative_to(ROOT).as_posix();a=r.json(name)
                require(type(a['original_game_runs']) is int and a['original_game_runs'] in (0,1),'Unknown charged UI request')
                requests.append(dict(record=name,sha256=r.digest(name),original_game_runs=a['original_game_runs'],success=a['success'],error=a.get('error'),plan=a['prospective_plan']))
            starts=sum(v['original_game_runs'] for v in requests)
            require(len(requests)<=13 and starts<=11,'Cumulative revised UI budget exceeded')
            result=dict(parent,schema=1,kind='recorded_native_rendering_acceptance_join',success=True,requirements=rows,
                parent=dict(record=PARENT,sha256=r.digest(PARENT)),cases=cases,analysis_sources=sources,validation_sources=validation_sources,claims=claims,
                source_phases=parent.get('source_phases',[])+phases,
                sources=parent['sources']|{n:h for n,h in sources.items() if n not in parent['sources']},
                passed_requirements=94,pending_requirements=62,newly_passed_requirements=6,
                closed_leaf_ids=sorted(set(parent['closed_leaf_ids'])|LEAVES),
                acquisition_requests=requests,new_original_game_runs=starts,new_per_pixel_comparisons=4,accepted_original_game_runs=4,
                scope='Exactly six HUD-update, portrait tooltip open/close and pause rows across default/software High/Low. Original state, SFT glyphs, full viewport, Space and campaign Mini ownership/Cancel plus normal retirement. No complete font/DC/actor lifetime, all-simulation pause or final-player/recovery decision.')
            inventory(r.json('docs/native-rendering-closure.json'),result)
            require(contracts(r)==(sources,claims),'UI join source/contract drift')
    except Exception as e:result.update(success=False,error=str(e))
    h=write_report(ROOT,args.report,(json.dumps(result,indent=2)+'\n').encode());print(json.dumps(dict(success=result['success'],error=result.get('error'),sha256=h)))
    return int(not result['success'])


if __name__=='__main__':raise SystemExit(main())
