#!/usr/bin/env python3
"""Join verified minimap facts onto the latest reviewed frozen inventory."""
import argparse
import importlib.util
import json
from pathlib import Path
from coverage_claims import behavior_contract,required_sources,scenario_contract
from coverage_evidence import Reader,write_report
from rendering_pilot import require
ROOT=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('minimap_facts',ROOT/'tools/close-rendering-markers.py')
facts=importlib.util.module_from_spec(spec);spec.loader.exec_module(facts)


def join(plan,parent,analysis):
    require(analysis['success'] and analysis['sources_stable'] and analysis['marker_intervals']==80
            and analysis['mismatching_pixels']==0 and analysis['original_game_runs']==0,
            'Unaccepted minimap retained-data analysis')
    rows=facts.close_rows(plan,parent,analysis['cases'])
    require(all(a==b for a,b in zip(parent['requirements'],rows) if a['id']!=facts.LEAF),
            'Minimap admission altered an inherited row')
    return rows


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for name in ('parent','analysis','report'):p.add_argument('--'+name,required=True)
    args=p.parse_args()
    require(args.report.startswith(facts.PREFIX) and not (ROOT/args.report).exists(),'Fresh owned minimap join required')
    with Reader(ROOT) as r:
        reg=r.json('research/runtime/coverage/register.json');b=next(v for v in reg['behaviors'] if v['id']==facts.BID);s=next(v for v in reg['scenarios'] if v['id']==facts.SID)
        sources={n:r.digest(n) for n in required_sources(b)};analysis=r.json(args.analysis)
        require(all(r.digest(n)==h for n,h in analysis['analysis_sources'].items()),'Minimap analysis source changed')
        parent=r.json(args.parent);rows=join(r.json('docs/native-rendering-closure.json'),parent,analysis)
        result=dict(analysis,kind='presented_marker_inventory_join',analysis=dict(record=args.analysis,sha256=r.digest(args.analysis)),
            parent=dict(record=args.parent,sha256=r.digest(args.parent)),sources=parent['sources'],analysis_sources=sources,
            claims=[dict(behavior=facts.BID,contract_sha256=behavior_contract(b,{v['id']:v for v in reg['builds']}),scenarios={facts.SID:scenario_contract(s)})],
            requirements=rows,decisions=parent['decisions'],closed_leaf_ids=sorted(set(parent['closed_leaf_ids'])|{facts.LEAF}),
            passed_requirements=parent['passed_requirements']+2,pending_requirements=154-parent['passed_requirements'])
        require(all(r.digest(n)==h for n,h in sources.items()),'Minimap join source changed')
        print(write_report(ROOT,args.report,(json.dumps(result,indent=2)+'\n').encode()))


if __name__=='__main__':main()
