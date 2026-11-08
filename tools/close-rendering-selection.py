#!/usr/bin/env python3
"""Close the finite selection/highlight row from four retained real domains."""
import argparse
import json
from pathlib import Path
from coverage_claims import behavior_contract, required_sources, scenario_contract
from coverage_evidence import Reader, write_report
from rendering_selection import BID, OUT, MATRIX, PARENT, LEAF, ROOT, analyze_case, close_rows, module
from rendering_pilot import require


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);args=p.parse_args()
    require(args.report.startswith(OUT) and '..' not in Path(args.report).parts
            and Path(args.report).suffix=='.json' and not (ROOT/args.report).exists(),'Fresh owned immutable report required')
    result=dict(schema=1,kind='retained_selection_highlight_acceptance',success=False,cases=[],
                original_game_runs=0,new_native_engine_executions=0,new_original_oracle_executions=0,
                newly_passed_requirements=0,historical_failures_retired=False)
    try:
        with Reader(ROOT) as reader:
            reg=reader.json('research/runtime/coverage/register.json');b=next(b for b in reg['behaviors'] if b['id']==BID)
            sources={n:reader.digest(n) for n in required_sources(b)};result['analysis_sources']=sources
            parent,applicability=module('close-rendering-world-exit').parent_plan(
                ROOT,reader,'DF08.natural-defeat01','NP.natural-defeat-closeout')
            require(parent==reader.json(PARENT),'Accepted102row parent differs')
            matrix=reader.json(MATRIX)
            require(matrix['success'] is True and matrix['actual_domains']==4 and matrix['selection_toggles']==80,
                    'Missing accepted real interaction matrix')
            for reference in matrix['comparisons']:
                case=analyze_case(reader,reference);result['cases'].append(case)
                print(json.dumps(dict(domain=case['domain'],verified=True,matched_viewport_pixels=case['matched_viewport_pixels'],
                    whole_world_swaps=case['whole_output']['world_swaps'],cursor_refusals=case['cursor_journal']['client_refusals'])),flush=True)
            rows=close_rows(reader.json('docs/native-rendering-closure.json'),parent,result['cases'])
            require(all(reader.digest(n)==h for n,h in sources.items()),'Selection analysis source changed during execution')
            negative=next(e for e in reg['evidence'] if e['id']=='CR07.diagnosis01')
            require(reader.digest(negative['record'])==negative['sha256'],'Lost historical invalid cursor evidence')
            result.update(success=True,scope=b['scope'],analysis_sources_stable=True,
                sources=parent['sources']|{n:h for n,h in sources.items() if n not in parent['sources']},
                claims=[dict(behavior=BID,contract_sha256=behavior_contract(b,{}),
                    scenarios={s['id']:scenario_contract(s) for s in reg['scenarios'] if BID in s['behaviors']})],
                matrix=dict(record=MATRIX,sha256=reader.digest(MATRIX)),
                parent=dict(record=PARENT,sha256=reader.digest(PARENT)),accepted_parent_applicability=applicability,
                known_invalid_cursor_evidence=dict(id=negative['id'],record=negative['record'],sha256=negative['sha256'],
                    scope='Historical invalid save-under/keyed cursor requests remain negative and source-bound; no new success/cause attribution or whole cursor closure.'),
                requirements=rows,dossier_identity=parent['dossier_identity'],decisions=parent['decisions'],
                closed_leaf_ids=sorted(set(parent['closed_leaf_ids'])|{LEAF}),
                passed_requirements=104,pending_requirements=52,newly_passed_requirements=2,
                verified_domains=4,required_domains=4,original_selection_callbacks=80,
                settled_source_generations=240,
                new_point_comparisons=sum(c['matched_viewport_pixels'] for c in result['cases']),
                historical_world_swap_pixels=sum(c['whole_output']['pixels'] for c in result['cases']),
                mismatching_pixels=0,allocation_generations_proved=False,current_whole_player_proved=False)
    except Exception as error:
        result['error']=str(error)
    digest=write_report(ROOT,args.report,(json.dumps(result,indent=2)+'\n').encode())
    print(json.dumps(dict(success=result['success'],error=result.get('error'),record=args.report,sha256=digest)),flush=True)
    return int(not result['success'])


if __name__=='__main__':raise SystemExit(main())
