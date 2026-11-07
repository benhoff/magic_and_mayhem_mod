#!/usr/bin/env python3
"""Admit only the two four-orientation rows from a complete current live matrix."""
import argparse
import json
from pathlib import Path
from coverage_claims import behavior_contract, fingerprint, required_sources, scenario_contract
from coverage_evidence import Reader, write_report
from rendering_acceptance import requirements, validate
from rendering_pilot import require

ROOT = Path(__file__).resolve().parents[1]
PREFIX = 'research/runtime/rendering-interaction-results-20261107/'
LEAF = 'journey.camera-motion.four-orientations'


def close_rows(plan, parent, matrix, binding):
    validate(plan)
    definitions = requirements(plan)
    expected = {(r['configuration'], r['id']) for r in definitions}
    require(parent['dossier_identity'] == fingerprint(definitions)
            and len(parent['requirements']) == len(expected) == 156
            and {(r['configuration'], r['id']) for r in parent['requirements']} == expected,
            'Frozen acceptance inventory differs')
    passed = sum(r['state'] == 'passed' for r in parent['requirements'])
    require(passed == parent['passed_requirements'] and matrix['success']
            and matrix['sources_stable'] and matrix['four_view_required_routes_proved']
            and matrix['mismatching_pixels'] == 0, 'Incomplete parent/matrix')
    cases = matrix['cases']
    require(len(cases) == 4 and {(p['domain'][0], tuple(p['domain'][1])) for p in cases} ==
            {(software, size) for software in (False, True) for size in ((800, 600), (640, 480))},
            'Missing/duplicate four-view domain')
    for case in cases:
        require(case['verified'] and case['cleanup']['verified'] and case['cleanup']['normal_quit'],
                'Four-view case did not complete normally')
        views = case['views'];routes = {r['generation']: r['camera'] for r in case['four_view_source_routes']}
        require(len(views) == 8 and {(v['mode'], v['view']) for v in views} ==
                {(mode, view) for mode in ('sharp', 'integer') for view in range(4)}, 'Incomplete four-view/scaling cycle')
        for view in views:
            require(len(set(view['frames'])) >= 3, 'Insufficient actual view generations')
            for generation in view['frames']:
                source = routes[generation]
                require(source['view'] == view['view'] and source['creature_markers'] > 0
                        and source['terrain_sequence'] > 0 and source['camera_sequence'] > 0
                        and source['cell_sequences'] and source['creature_sequence'] > 0
                        and {r['backend'] for r in source['raster']} >= {1, 2, 9, 10},
                        'Missing required creature/effect/terrain/minimap route')
    rows = []
    for old in parent['requirements']:
        row = dict(old)
        if row['id'] == LEAF:
            require(row['state'] == 'pending', 'Four-view row already accepted')
            software = row['configuration'] == 'player-software'
            row.update(state='passed', admitted_execution_evidence=[dict(
                matrix=binding, actual_world_extents=[p['domain'][1]], scaling=['sharp', 'integer'],
                source_generations=p['source_generations'], world_presentations=p['world_presentations'],
                reason='Current native four-view output: exact owned source orientation/body/order, nonempty creature markers, indexed scene/shadow/displacement routes, full independent viewport pixels and normal Quit; fixed Xvfb/Mesa provider.')
                for p in cases if p['domain'][0] is software])
        rows.append(row)
    require(sum(r['state'] == 'passed' for r in rows) == passed + 2, 'Closeout exceeded two rows')
    return rows


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--parent', required=True)
    parser.add_argument('--matrix', required=True)
    parser.add_argument('--report', required=True)
    args = parser.parse_args()
    require(args.report.startswith(PREFIX) and not (ROOT / args.report).exists(), 'Fresh owned closeout required')
    with Reader(ROOT) as reader:
        parent, matrix = reader.json(args.parent), reader.json(args.matrix)
        require(all(reader.digest(n) == h for n, h in matrix['sources'].items()), 'Matrix execution/analyzer source changed')
        matrix_binding = dict(record=args.matrix, sha256=reader.digest(args.matrix))
        plan = reader.json('docs/native-rendering-closure.json')
        rows = close_rows(plan, parent, matrix, matrix_binding)
        register = reader.json('research/runtime/coverage/register.json')
        behavior = next(b for b in register['behaviors'] if b['id'] == 'NP.four-view-acceptance-closeout')
        scenario = next(s for s in register['scenarios'] if s['id'] == 'native.four-view-acceptance-closeout')
        builds = {b['id']: b for b in register['builds']}
        sources = {n: reader.digest(n) for n in required_sources(behavior)}
        result = dict(schema=1, kind='current_four_view_acceptance_closeout', success=True,
            sources=parent['sources'], analysis_sources=sources, sources_stable=True,
            claims=[dict(behavior=behavior['id'], contract_sha256=behavior_contract(behavior, builds),
                         scenarios={scenario['id']:scenario_contract(scenario)})],
            parent=dict(record=args.parent, sha256=reader.digest(args.parent)), matrix=matrix_binding,
            requirements=rows, dossier_identity=parent['dossier_identity'], decisions=parent['decisions'],
            closed_leaf_ids=sorted(set(parent['closed_leaf_ids']) | {LEAF}),
            passed_requirements=parent['passed_requirements'] + 2,
            pending_requirements=156 - parent['passed_requirements'] - 2, newly_passed_requirements=2,
            original_game_runs=0, new_pixel_comparisons=0, native_source_waivers=False,
            current_native_scope='Only the named four-view matrix; inherited rows retain their exact recorded source phases.',
            inherited_current_source_drift={n:dict(recorded=h,current=reader.digest(n)) for n,h in parent['sources'].items()
                if (ROOT/n).is_file() and reader.digest(n)!=h},
            response_deadlines_proved=False, pan_hud_requirement_proved=False,
            whole_picking_selection_hotspot_proved=False, viewport_boundaries_proved=False,
            historical_failures_retired=False)
        print(write_report(ROOT, args.report, (json.dumps(result, indent=2) + '\n').encode()))


if __name__ == '__main__':
    main()
