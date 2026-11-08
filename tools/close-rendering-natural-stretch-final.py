#!/usr/bin/env python3
"""Join the three accepted domains with the final queued software800 domain."""
import argparse,importlib.util,json
from pathlib import Path
from coverage_evidence import Reader,write_report
from coverage_claims import behavior_contract,scenario_contract,required_sources
from rendering_pilot import require

ROOT=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('stretch_final_admission',ROOT/'tools/close-rendering-natural-stretch.py')
m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)
NAMES=['default640-10','default800-02','software640-01','software800-02']
ANALYSIS=m.ANALYSIS+['tools/close-rendering-natural-stretch-final.py']


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--parent',default='research/runtime/rendering-recovery-four-results-20261108/master-admission-01.json')
    p.add_argument('--report',required=True);a=p.parse_args()
    require(a.report.startswith(m.PREFIX) and not (ROOT/a.report).exists(),'Fresh owned final admission required')
    with Reader(ROOT) as reader:
        sources={n:reader.digest(n) for n in ANALYSIS};cases=[m.read_case(reader,n) for n in NAMES]
        parent=reader.json(a.parent);rows=m.close_rows(reader.json('docs/native-rendering-closure.json'),parent,cases)
        reg=reader.json('research/runtime/coverage/register.json')
        behavior=next(b for b in reg['behaviors'] if b['id']==m.BID);scenario=next(s for s in reg['scenarios'] if s['id']==m.SID)
        require(required_sources(behavior)==set(ANALYSIS),'Final admission source closure differs')
        require(sources=={n:reader.digest(n) for n in ANALYSIS},'Final admission sources changed')
        result=dict(schema=1,kind='natural_stretch_operation_admission',success=True,sources_stable=True,
            analysis_sources=sources,sources=parent['sources']|{n:h for n,h in sources.items() if n not in parent['sources']},
            parent_source_phases=parent['sources'],cases=cases,requirements=rows,verified_domains=4,required_domains=4,
            newly_passed_requirements=2,passed_requirements=parent['passed_requirements']+2,pending_requirements=parent['pending_requirements']-2,
            closed_leaf_ids=sorted(set(parent['closed_leaf_ids'])|{m.LEAF}),dossier_identity=parent['dossier_identity'],
            decisions=parent['decisions'],parent=dict(record=a.parent,sha256=reader.digest(a.parent)),
            claims=[dict(behavior=m.BID,contract_sha256=behavior_contract(behavior,{}),scenarios={m.SID:scenario_contract(scenario)})],
            original_game_runs=0,verified_positive_original_starts=4,checked_pixels=sum(c['pixels'] for c in cases),
            mismatching_pixels=0,current_whole_native_execution=False,original_driver_comparison=False,
            historical_failures_retired=False,alias_rows_closed=False)
    digest=write_report(ROOT,a.report,(json.dumps(result,indent=2)+'\n').encode())
    print(json.dumps(dict(success=True,record=a.report,sha256=digest,newly_passed=2,
                         passed=result['passed_requirements'],pending=result['pending_requirements'])))


if __name__=='__main__':main()
