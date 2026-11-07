#!/usr/bin/env python3
"""Admit one finite default picking row and preserve every inherited decision."""
import argparse
import importlib.util
import json
from pathlib import Path
from coverage_claims import behavior_contract, fingerprint, required_sources, scenario_contract
from coverage_evidence import Reader, write_report
from rendering_acceptance import requirements, validate
from rendering_pilot import require

ROOT=Path(__file__).resolve().parents[1]
PREFIX='research/runtime/rendering-picking-resize-results-20261107/'
BID='NP.picking-live-frame-rebuild'
SID='native.picking-live-frame-rebuild'
LEAF='journey.cursor-interaction.picking'


def close_rows(plan,parent,compared,binding):
    validate(plan)
    definitions=requirements(plan)
    expected={(r['configuration'],r['id']) for r in definitions}
    require(len(parent['requirements'])==len(expected)==156
            and {(r['configuration'],r['id']) for r in parent['requirements']}==expected
            and parent['dossier_identity']==fingerprint(definitions),'Frozen acceptance inventory differs')
    passed=sum(r['state']=='passed' for r in parent['requirements'])
    p=compared['proof']
    require(parent['passed_requirements']==passed and compared['success'] and compared['sources_stable']
            and compared['comparison_processes_stopped'] and compared['comparison']['success']
            and compared['comparison']['mismatching_pixels']==0 and p['verified']
            and p['windows']==48 and p['object_windows']==p['cell_windows']==24
            and p['displayed_source_witnesses']==144 and p['live_display_rebuilds']==2
            and p['same_world_continuity'] is False,'Incomplete live picking comparison')
    rows=[]
    for old in parent['requirements']:
        row=dict(old)
        if (row['configuration'],row['id'])==('player-default',LEAF):
            require(row['state']=='pending','Default picking already accepted')
            row.update(state='passed',admitted_execution_evidence=[dict(
                execution=dict(acquisition=compared['acquisition'],comparison=binding),
                actual_world_extents=[[800,600],[640,480],[800,600]],scaling=['sharp','integer'],
                reason='48 source-derived original object/cell windows after pan/rotation and real Main Preferences frame-size changes in one original process; exact displayed input/World-return identities, complete independent viewport pixels and normal retirement. Worlds are recreated; software and same-World Mini continuity remain separate.')])
        rows.append(row)
    require(sum(r['state']=='passed' for r in rows)==passed+1,'Closeout exceeded one row')
    return rows


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for name in ('parent','comparison','report'):p.add_argument('--'+name,required=True)
    args=p.parse_args()
    require(args.report.startswith(PREFIX) and not (ROOT/args.report).exists(),'Fresh owned report required')
    with Reader(ROOT) as r:
        c=r.json(args.comparison);a=r.json(c['acquisition']['record'])
        require(r.digest(c['acquisition']['record'])==c['acquisition']['sha256']
                and all(r.digest(n)==h for n,h in c['sources'].items()),'Comparison source/input phase changed')
        require(a['configuration']['software_flag'] is False,'Default picking requires the actual default profile')
        parent=r.json(args.parent);binding=dict(record=args.comparison,sha256=r.digest(args.comparison))
        rows=close_rows(r.json('docs/native-rendering-closure.json'),parent,c,binding)
        # Apply the existing raw normal-lifetime admission to the new primary
        # execution. Its command/source retirement must be present as well.
        spec=importlib.util.spec_from_file_location('picking_retirement',ROOT/'tools/close-rendering-retained-rows.py')
        retirement=importlib.util.module_from_spec(spec);spec.loader.exec_module(retirement)
        retired=retirement.retire_case(r,dict(reference=c['acquisition'],profile='player-default',
            rows=[LEAF],comparisons=[binding]))
        require(retired['proof']['command_lifetimes'] is not None,'New picking case omitted full command retirement')
        reg=r.json('research/runtime/coverage/register.json')
        b=next(b for b in reg['behaviors'] if b['id']==BID);s=next(s for s in reg['scenarios'] if s['id']==SID)
        builds={b['id']:b for b in reg['builds']}
        analysis_sources={n:r.digest(n) for n in required_sources(b)}
        result=dict(schema=1,kind='finite_picking_frame_change_closeout',success=True,
            scope=b['scope'],sources=parent['sources'],analysis_sources=analysis_sources,sources_stable=True,
            claims=[dict(behavior=BID,contract_sha256=behavior_contract(b,builds),scenarios={SID:scenario_contract(s)})],
            parent=dict(record=args.parent,sha256=r.digest(args.parent)),comparison=binding,
            picking_proof=c['proof'],normal_lifetime=retired,
            requirements=rows,dossier_identity=parent['dossier_identity'],decisions=parent['decisions'],
            closed_leaf_ids=sorted(set(parent['closed_leaf_ids'])|{LEAF}),
            passed_requirements=parent['passed_requirements']+1,pending_requirements=155-parent['passed_requirements'],
            newly_passed_requirements=1,original_game_runs=0,new_pixel_comparisons=0,
            inherited_current_source_drift={n:dict(recorded=h,current=r.digest(n)) for n,h in parent['sources'].items()
                if (ROOT/n).is_file() and r.digest(n)!=h},
            historical_failures_retired=False,software_picking_proved=False,
            same_world_continuity_proved=False,selection_hotspot_or_response_deadlines_proved=False,
            whole_rendering_complete=False)
        print(write_report(ROOT,args.report,(json.dumps(result,indent=2)+'\n').encode()))


if __name__=='__main__':main()
