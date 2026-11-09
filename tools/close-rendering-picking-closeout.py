#!/usr/bin/env python3
"""Admit only both finite picking rows after full raw-output and lifetime proof."""
import argparse
import json
from pathlib import Path
from coverage_claims import behavior_contract, fingerprint, required_sources, scenario_contract
from coverage_evidence import Reader, write_report
from rendering_acceptance import requirements, validate
from rendering_pilot import require
from picking_closeout import PREFIX, BID, SID, LEAF, complete_cases

ROOT = Path(__file__).resolve().parents[1]


def close_rows(plan, parent, cases):
    validate(plan)
    definitions = requirements(plan)
    if parent['dossier_identity'] != fingerprint(definitions):
        review = parent['definition_review']
        require(review['preserved_dossier_identity'] == parent['dossier_identity']
                and review['current_definition_sha256'] == fingerprint(definitions)
                and review['all_acceptance_conditions_and_owners_unchanged'] is True,
                'Missing exact inherited annotation review')
        restored = json.loads(json.dumps(definitions))
        by_key = {(d['configuration'],d['id']):d for d in restored}
        changes = review['only_added_candidate_annotations']
        require(len(changes)==2 and {(c['configuration'],c['id']) for c in changes} ==
                {(cfg,'operation.copy-alias-overlap') for cfg in ('player-default','player-software')},
                'Unreviewed annotation domain')
        for change in changes:
            row = by_key[(change['configuration'],change['id'])]
            require(change['after']==change['before']+['ALIAS08.caller-leads01']
                    and row['candidate_evidence']==change['after'], 'Changed candidate annotation')
            row['candidate_evidence'] = change['before']
        require(fingerprint(restored)==parent['dossier_identity'], 'Acceptance conditions changed')
    require(len(parent['requirements']) == 156
            and {(r['configuration'],r['id']) for r in parent['requirements']} ==
                {(r['configuration'],r['id']) for r in definitions},
            'Frozen acceptance inventory differs')
    complete_cases(cases)
    passed = sum(r['state'] == 'passed' for r in parent['requirements'])
    require(passed == parent['passed_requirements'], 'Parent totals differ')
    rows = []
    for old in parent['requirements']:
        row = dict(old)
        if row['id'] == LEAF:
            require(row['state'] == 'pending', 'Picking row already accepted')
            case = next(c for c in cases if c['software'] is
                        (row['configuration'] == 'player-software'))
            row.update(state='passed', admitted_execution_evidence=[dict(
                acquisition=case['acquisition'], comparison=case['binding'],
                actual_world_extents=[[800,600],[640,480],[800,600]],
                same_original_process=True, same_world=False,
                scaling=['sharp','integer'],
                reason='48 source-derived object/cell action windows, four views and pan, original return/input identities, both actual display recreations, complete independent viewport history and normal retirement.')])
        rows.append(row)
    require(sum(r['state']=='passed' for r in rows) == passed+2,
            'Admission exceeded both picking rows')
    return rows


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--parent', required=True)
    parser.add_argument('--comparison', action='append', required=True)
    parser.add_argument('--report', required=True)
    args = parser.parse_args()
    require(args.report.startswith(PREFIX) and not (ROOT/args.report).exists(),
            'Fresh owned admission required')
    with Reader(ROOT) as r:
        parent = r.json(args.parent)
        register = r.json('research/runtime/coverage/register.json')
        behavior = next(b for b in register['behaviors'] if b['id']==BID)
        scenario = next(s for s in register['scenarios'] if s['id']==SID)
        claim = dict(behavior=BID, contract_sha256=behavior_contract(
            behavior,{b['id']:b for b in register['builds']}),
            scenarios={SID:scenario_contract(scenario)})
        cases, sources = [], {}
        for name in args.comparison:
            c = r.json(name)
            a = r.json(c['acquisition']['record'])
            require(r.digest(c['acquisition']['record']) == c['acquisition']['sha256']
                    and c['claims'] == [claim] and a['success']
                    and a['sources_stable'] and a['prefix_pool']['returned']
                    and not a.get('forced_private_prefix_stop'), 'Unbound acquisition or claim')
            for path, digest in c['sources'].items():
                require(r.digest(path)==digest and sources.get(path,digest)==digest,
                        'Mixed or changed execution sources')
                sources[path] = digest
            cases.append(dict(c, software=a['configuration']['software_flag'],
                binding=dict(record=name,sha256=r.digest(name))))
        require(required_sources(behavior) <= sources.keys(), 'Incomplete source closure')
        rows = close_rows(r.json('docs/native-rendering-closure.json'),parent,cases)
        result = dict(schema=1,kind='finite_picking_closeout',success=True,
            claims=[claim], analysis_sources=sources, analysis_sources_stable=True,
            parent=dict(record=args.parent,sha256=r.digest(args.parent)),
            cases=cases, requirements=rows, dossier_identity=parent['dossier_identity'],
            definition_review=parent.get('definition_review'),
            decisions=parent['decisions'], passed_requirements=parent['passed_requirements']+2,
            pending_requirements=parent['pending_requirements']-2, newly_passed_requirements=2,
            closed_leaf_ids=sorted(set(parent['closed_leaf_ids'])|{LEAF}),
            original_game_runs=0, new_pixel_comparisons=0,
            historical_failures_retired=False, current_whole_player_proved=False,
            scope=behavior['scope'])
        print(write_report(ROOT,args.report,(json.dumps(result,indent=2)+'\n').encode()))


if __name__ == '__main__': main()
