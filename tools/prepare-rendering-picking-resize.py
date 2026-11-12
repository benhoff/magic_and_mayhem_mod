#!/usr/bin/env python3
"""Freeze unchanged native inputs and the bounded live picking/rebuild scenario."""
import argparse
import hashlib
import json
from pathlib import Path
from coverage_evidence import Reader, write_report
from rendering_pilot import require
ROOT=Path(__file__).resolve().parents[1]
PREFIX='research/runtime/rendering-picking-resize-results-20261107/'


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--report',default=PREFIX+'input-plan-01.json')
    p.add_argument('--include-preparation-refusals',action='store_true')
    args=p.parse_args()
    require(args.report.startswith(PREFIX) and not (ROOT/args.report).exists(),'Fresh owned plan required')
    with Reader(ROOT) as r:
        parent='research/runtime/rendering-interaction-results-20261107/input-plan-08.json'
        old=r.json(parent)
        for section in ('runtime_sources','compiler_sources'):
            require(all(r.digest(n)==h for n,h in old[section].items()),'Changed accepted native/runtime input')
        require(all(r.digest(n)==h for n,h in old['oracle']['native_sources'].items()),'Changed original reference source')
        for b in [old['player_binary'],*old['oracle']['binaries']]:
            with (ROOT/b['path']).open('rb') as source:
                current=hashlib.file_digest(source,'sha256').hexdigest()
            require(r.digest(b['record'])==b['sha256']==current,'Changed accepted executable')
        controls=['tools/run-rendering-picking-resize.py','tools/prepare-rendering-picking-resize.py',
                  'tools/check-rendering-picking-resize.py','tools/rendering_picking_resize.py',
                  'tools/rendering_picking_resize_assessment.py',
                  'tests/test-rendering-picking-resize.py']
        plan=dict(schema=1,success=True,kind='finite_picking_live_rebuild_plan',
                  parent=dict(record=parent,sha256=r.digest(parent)),
                  accepted_matrix=dict(record='research/runtime/rendering-interaction-results-20261107/matrix-01.json',
                      sha256=r.digest('research/runtime/rendering-interaction-results-20261107/matrix-01.json')),
                  compiler_sources=old['compiler_sources'],runtime_sources=old['runtime_sources'],
                  player_binary=old['player_binary'],oracle=old['oracle'],
                  sources=old['sources']|{n:r.digest(n) for n in controls},
                  bounds=dict(maximum_requests=6 if args.include_preparation_refusals else 4,maximum_original_games=3 if args.include_preparation_refusals else 4,primary_games=2,repairs=1 if args.include_preparation_refusals else 2,
                              stop_on_first_failure=True,successful_pilot_comparison_before_expansion=True),
                  domains=[dict(software=False,actual_sizes=[[800,600],[640,480],[800,600]],scaling=['sharp','integer'])],
                  map='Celtic Forest One (small)',actor=dict(slot=0,type=0,owner=0),
                  input_policy=dict(source_only_targets=True,minimum_displayed_source_generations=3,
                      readiness_seconds=8,minimum_pointer_seconds=1.6,
                      foot_hypothesis='Last opaque body row nearest horizontal centre projects to the observed wizard XY cell; actual original XYZ must remain the same across each no-order World phase. Recreated Worlds may spawn the wizard at a different cell. Never calibrate by hit-coordinate search.',
                      rebuild='Quick Results -> Main Preferences 800->640->800 -> same-map recreated Quick World, within one original process; actual original acknowledgements, frames and resource epochs required. No same-World continuity.'),
                  scope='Only default journey.cursor-interaction.picking. Real original object/cell picks, pan/rotation and actual frame-size changes with complete independent viewport output and normal Quit. Software, same-World Mini rebuilds, selection/highlights, whole cursor/hotspot, general ray recovery, allocation lifetimes, other maps/providers and response deadlines remain separate.')
        if args.include_preparation_refusals:
            plan['preparation_refusals']=[dict(record=PREFIX+n+'-acquisition.json',sha256=r.digest(PREFIX+n+'-acquisition.json')) for n in ('default-01','default-02','default-03')]
            require(all(r.json(x['record'])['original_game_runs']==0 for x in plan['preparation_refusals']), 'Preparation-only bounds require three retained zero-game refusals')
            plan['request_budget_reason']='Count all3zero-game preparation refusals. The hard6request cap is unchanged; before the first original launch, tighten the actual-game cap to3:2primary profiles and1possible repaired actual game. No count is reset or later expansion authorized.'
            plan['remaining_request_scope']='Exactly one repaired default request remains after default-05. Software is deferred, not passed. Preserve all five failed requests and their original prospective plans.'
        print(write_report(ROOT,args.report,(json.dumps(plan,indent=2)+'\n').encode()))


if __name__=='__main__':main()
