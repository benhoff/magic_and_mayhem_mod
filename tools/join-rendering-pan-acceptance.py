#!/usr/bin/env python3
"""Join unchanged verified pan facts into the latest frozen acceptance inventory."""
import argparse
import importlib.util
import json
from pathlib import Path
from coverage_claims import behavior_contract, required_sources, scenario_contract
from coverage_evidence import Reader,write_report
from rendering_pilot import require
ROOT=Path(__file__).resolve().parents[1]
PREFIX='research/runtime/rendering-picking-resize-results-20261107/'
BID='NP.pan-picking-hud-acceptance';SID='native.pan-picking-hud-acceptance'
spec=importlib.util.spec_from_file_location('pan_facts',ROOT/'tools/close-rendering-pan-acceptance.py')
facts=importlib.util.module_from_spec(spec);spec.loader.exec_module(facts)


def join(plan,parent,analysis):
    require(analysis['success'] and analysis['sources_stable'] and analysis['pan_windows']==8
            and analysis['mismatching_pixels']==0 and analysis['original_game_runs']==0,
            'Unaccepted retained pan analysis')
    rows=facts.close_rows(plan,parent,analysis['cases'])
    require(all(a==b for a,b in zip(parent['requirements'],rows) if a['id']!=facts.LEAF),
            'Pan admission altered an inherited row')
    return rows


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for name in ('parent','analysis','report'):p.add_argument('--'+name,required=True)
    args=p.parse_args()
    require(args.report.startswith(PREFIX) and not (ROOT/args.report).exists(),'Fresh owned join required')
    with Reader(ROOT) as r:
        reg=r.json('research/runtime/coverage/register.json');b=next(v for v in reg['behaviors'] if v['id']==BID);s=next(v for v in reg['scenarios'] if v['id']==SID);builds={v['id']:v for v in reg['builds']}
        source_start={n:r.digest(n) for n in required_sources(b)}
        analysis=r.json(args.analysis)
        require(all(r.digest(n)==h for n,h in analysis['analysis_sources'].items()),'Retained pan analyzer/input source changed')
        parent=r.json(args.parent);rows=join(r.json('docs/native-rendering-closure.json'),parent,analysis)
        require(all(r.digest(n)==h for n,h in source_start.items()),'Pan join sources changed during execution')
        result=dict(schema=1,kind='pan_acceptance_inventory_join',success=True,scope=b['scope'],sources=parent['sources'],analysis_sources=source_start,sources_stable=True,
            claims=[dict(behavior=BID,contract_sha256=behavior_contract(b,builds),scenarios={SID:scenario_contract(s)})],
            parent=dict(record=args.parent,sha256=r.digest(args.parent)),analysis=dict(record=args.analysis,sha256=r.digest(args.analysis)),
            matrix=analysis['matrix'],cases=analysis['cases'],requirements=rows,dossier_identity=parent['dossier_identity'],decisions=parent['decisions'],
            closed_leaf_ids=sorted(set(parent['closed_leaf_ids'])|{facts.LEAF}),
            passed_requirements=parent['passed_requirements']+2,pending_requirements=154-parent['passed_requirements'],newly_passed_requirements=2,
            pan_windows=8,compared_pixels=analysis['compared_pixels'],mismatching_pixels=0,
            original_game_runs=0,new_native_executions=0,new_original_oracle_runs=0,new_pixel_comparisons=0,
            retained_pixel_verification=dict(record=args.analysis,sha256=r.digest(args.analysis)),
            historical_failures_retired=False,picking_resize_or_hotspot_closed=False,atomic_hud_snapshot_proved=False,
            inherited_current_source_drift={n:dict(recorded=h,current=r.digest(n)) for n,h in parent['sources'].items() if (ROOT/n).is_file() and r.digest(n)!=h})
        print(write_report(ROOT,args.report,(json.dumps(result,indent=2)+'\n').encode()))


if __name__=='__main__':main()
