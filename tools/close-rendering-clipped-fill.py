#!/usr/bin/env python3
"""Join bounded clipped-fill inputs/current native replay to accepted rows."""
import argparse
import importlib.util
import json
from pathlib import Path
from coverage_evidence import Reader,write_report
from coverage_claims import fingerprint
from rendering_acceptance import requirements,validate
from rendering_pilot import require

ROOT=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('clipped_fill',ROOT/'tools/check-rendering-clipped-fill.py')
m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)
SCOPE=('Bounded clipped constant-fill acceptance in four admitted default/software640/800 World contexts, all with actual800x600 primary storage. '
       'Original owner caller/constant argument and immutable RGNDATA bind unique actual UPDATEs and live primary storage. '
       'Current exact-input native GPU replay checks every destination/exterior WORD; two Low historical complete presentations additionally match. '
       'High intermediate paints remain coalesced and are not recovered. Native prior reply/recorded uploads are transform inputs, not new original prior-history proof. '
       'No physical store trace, partial-prefix failure, arbitrary format/provider, original driver/whole ABI or current whole-player claim. Preserve every other row, phase, failure and both decisions.')

def join(reader,analysis,parent,plan):
    validate(plan)
    require(parent['success'] and parent['dossier_identity']==fingerprint(requirements(plan))
        and len(parent['requirements'])==156,'Changed accepted requirement inventory')
    require(analysis['success'] and analysis['sources_stable'] and len(analysis['cases'])==4
        and analysis['current_native_replay_cases']==4 and analysis['historical_positive_presentations']==2
        and not analysis['current_native_engine_execution'] and analysis['original_game_runs']==0,
        'Clipped-fill positive native replay/admission incomplete')
    require(all(reader.digest(n)==h for n,h in analysis['sources'].items()),'Clipped-fill analysis source changed')
    cases=analysis['cases'];domains={(c['domain'][0],tuple(c['domain'][1])) for c in cases}
    require(domains=={(p,s) for p in ('player-default','player-software') for s in ((640,480),(800,600))}
        and len({c['identity'] for c in cases})==4,'Missing/repeated actual clipped-fill domain')
    for c in cases:
        m.check_case(c)
        pair={k:c[k] for k in ('acquisition','comparison')}
        require(all(reader.digest(v['record'])==v['sha256'] for v in pair.values()),'Changed accepted case input')
        require(any(r['state']=='passed' and r['configuration']==c['domain'][0]
            and any(v.get('execution')==pair for v in r.get('admitted_execution_evidence',[]))
            for r in parent['requirements']),'Clipped-fill input missing from admitted parent cohort')
    rows=[]
    for old in parent['requirements']:
        row=dict(old)
        if row['id']==m.LEAF:
            require(row['state']=='pending','Clipped-fill row already passed')
            row.update(state='passed',admitted_execution_evidence=[dict(execution={k:c[k] for k in ('acquisition','comparison')},
                actual_world_extents=[c['domain'][1]],reason=SCOPE,native_replay=c['witness']['native_replay']['artifacts'])
                for c in cases if c['domain'][0]==row['configuration']])
        rows.append(row)
    require(sum(r['state']=='passed' for r in rows)==parent['passed_requirements']+2,'Promotion exceeds two clipped-fill rows')
    return dict(analysis,kind='retained_clipped_fill_acceptance_join',requirements=rows,scope=SCOPE,
        decisions=parent['decisions'],dossier_identity=parent['dossier_identity'],
        parent_source_phases=parent['sources'],sources=parent['sources']|{n:h for n,h in analysis['sources'].items() if n not in parent['sources']},
        closed_leaf_ids=sorted(set(parent['closed_leaf_ids'])|{m.LEAF}),passed_requirements=parent['passed_requirements']+2,
        pending_requirements=154-parent['passed_requirements'],newly_passed_requirements=2)

def main():
    p=argparse.ArgumentParser(description=__doc__)
    for name in ('analysis','parent','report'):p.add_argument('--'+name,required=True)
    a=p.parse_args();require(a.report.startswith(m.OUT) and not (ROOT/a.report).exists(),'Fresh clipped-fill join required')
    result=dict(schema=1,success=False)
    try:
        with Reader(ROOT) as r:
            result=join(r,r.json(a.analysis),r.json(a.parent),r.json('docs/native-rendering-closure.json'))
            result.update(parent=dict(record=a.parent,sha256=r.digest(a.parent)),component_analysis=dict(record=a.analysis,sha256=r.digest(a.analysis)))
    except Exception as e:result['success']=False;result['error']=str(e)
    sha=write_report(ROOT,a.report,(json.dumps(result,indent=2)+'\n').encode());print(json.dumps(dict(success=result['success'],error=result.get('error'),sha256=sha)))
    return int(not result['success'])

if __name__=='__main__':raise SystemExit(main())
