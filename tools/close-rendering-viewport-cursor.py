#!/usr/bin/env python3
"""Admit only the four requested viewport-boundary/cursor-motion profile rows."""
import argparse
import json
from pathlib import Path
from coverage_claims import behavior_contract, fingerprint, required_sources, scenario_contract
from coverage_evidence import Reader, write_report
from rendering_acceptance import requirements, validate
from rendering_pilot import require
from viewport_cursor import PREFIX, LEAVES, complete_domains

ROOT=Path(__file__).resolve().parents[1]


def close_rows(plan,parent,cases):
    validate(plan)
    definitions=requirements(plan)
    require(parent['dossier_identity']==fingerprint(definitions)
            and len(parent['requirements'])==156
            and {(v['configuration'],v['id']) for v in parent['requirements']}==
                {(v['configuration'],v['id']) for v in definitions},'Frozen acceptance inventory differs')
    complete_domains([v['proof'] for v in cases])
    passed=sum(v['state']=='passed' for v in parent['requirements'])
    require(passed==parent['passed_requirements'],'Parent totals differ')
    rows=[]
    for old in parent['requirements']:
        row=dict(old)
        if row['id'] in LEAVES:
            require(row['state']=='pending','Requested row is already accepted; preserve its original attachment')
            software=row['configuration']=='player-software'
            case=next(v for v in cases if v['proof']['software'] is software)
            row.update(state='passed',admitted_execution_evidence=[dict(
                acquisition=case['acquisition'],comparison=case['comparison'],
                actual_world_extents=[[800,600],[640,480]],same_original_process=True,
                same_world=False,scaling=['sharp','integer'],
                reason='Finite physical viewport edges/corners and actual original cursor polls, complete independently checked viewport history, Main Preferences800/640/800 recreation and normal native owner retirement.')])
        rows.append(row)
    require(sum(r['state']=='passed' for r in rows)==passed+4,'Admission exceeded the four requested rows')
    return rows


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--parent',required=True)
    p.add_argument('--comparison',action='append',required=True)
    p.add_argument('--report',required=True)
    args=p.parse_args()
    require(args.report.startswith(PREFIX) and not(ROOT/args.report).exists(),'Fresh owned admission required')
    with Reader(ROOT) as reader:
        parent=reader.json(args.parent);cases=[];sources={}
        for name in args.comparison:
            compared=reader.json(name)
            require(compared['success'] and compared['sources_stable']
                    and compared['original_manifest_verified_before_after']
                    and compared['comparison_processes_stopped']
                    and compared['comparison']['success'] and compared['comparison']['mismatching_pixels']==0,
                    'Incomplete exact original/native comparison')
            acquired=reader.json(compared['acquisition']['record'])
            require(reader.digest(compared['acquisition']['record'])==compared['acquisition']['sha256'],
                    'Acquisition identity changed')
            require(acquired['success'] and acquired['original_manifest_verified_before_after']
                    and acquired['configuration']['software_flag'] is compared['proof']['software'],
                    'Acquisition/profile differs')
            for n,h in compared['sources'].items():
                require(n not in sources or sources[n]==h,'Mixed source phases need explicit scoped review')
                require(reader.digest(n)==h,'Current source differs from named comparison')
                sources[n]=h
            cases.append(dict(proof=compared['proof'],claims=compared['claims'],acquisition=compared['acquisition'],
                              comparison=dict(record=name,sha256=reader.digest(name))))
        rows=close_rows(reader.json('docs/native-rendering-closure.json'),parent,cases)
        register=reader.json('research/runtime/coverage/register.json')
        behavior=next(v for v in register['behaviors'] if v['id']=='NP.viewport-cursor-continuity')
        scenario=next(v for v in register['scenarios'] if v['id']=='native.viewport-cursor-continuity')
        builds={v['id']:v for v in register['builds']}
        claim=dict(behavior=behavior['id'],contract_sha256=behavior_contract(behavior,builds),
                   scenarios={scenario['id']:scenario_contract(scenario)})
        require(all(v['claims']==[claim] for v in cases),'Executed contract differs from admission')
        for name in required_sources(behavior):
            require(name in sources and sources[name]==reader.digest(name),
                    'Required source was not pinned by execution: '+name)
        result=dict(schema=1,kind='viewport_cursor_acceptance',success=True,
            parent=dict(record=args.parent,sha256=reader.digest(args.parent)),cases=cases,
            analysis_sources=sources,analysis_sources_stable=True,
            claims=[claim],
            requirements=rows,dossier_identity=parent['dossier_identity'],decisions=parent['decisions'],
            passed_requirements=parent['passed_requirements']+4,pending_requirements=parent['pending_requirements']-4,
            newly_passed_requirements=4,closed_leaf_ids=sorted(set(parent['closed_leaf_ids'])|LEAVES),
            mismatching_pixels=0,verified_profiles=2,historical_failures_retired=False,
            same_world_continuity_proved=False,current_whole_player_proved=False,
            scope='Only the user-requested four finite viewport-boundary/cursor-motion profile rows. Preserve every accepted parent row, exact prior source phases, failures, exhausted budgets and both pending final decisions. No whole picking, arbitrary clamp saturation, driver/platform or gameplay reconstruction claim.')
        print(write_report(ROOT,args.report,(json.dumps(result,indent=2)+'\n').encode()))


if __name__=='__main__':main()
