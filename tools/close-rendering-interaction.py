#!/usr/bin/env python3
"""Join four independently compared finite camera/cursor/picking domains."""
import argparse
import hashlib
import json
import importlib.util
from pathlib import Path
from coverage_evidence import Reader, write_report
from coverage_claims import behavior_contract, required_sources, scenario_contract
from rendering_pilot import require
from resident_capture import records

ROOT = Path(__file__).resolve().parents[1]
PREFIX = 'research/runtime/rendering-interaction-results-20261107/'

def view_routes(reader, acquired, proof):
    """Verify the existing four-view leaf's creature/effect/terrain/minimap routes."""
    spec=importlib.util.spec_from_file_location('interaction_camera_routes',ROOT/'tools/close-rendering-camera-matrix.py')
    camera=importlib.util.module_from_spec(spec);spec.loader.exec_module(camera)
    decoder=camera.module('inspect-canvas-producers')
    wanted={g for view in proof['views'] for g in view['frames']};found={}
    source=Path(acquired['temporary_root'])/'worlds.bin'
    from rendering_menu_acceptance import artifact
    require(hashlib.sha256(source.read_bytes()).hexdigest()==hashlib.sha256(artifact(reader,acquired,'worlds.bin')).hexdigest(),'Retained route source differs')
    for generation,chunk,stream,request in records(source):
        if chunk or generation not in wanted:continue
        _,rows=decoder.decode(stream,complete=False);context=camera.context_many_cells(rows)
        require(context and context['creature_markers']>0
                and {r['backend'] for r in context['raster']}>={1,2,9,10},
                'Four-view source lacks required creature/shadow/displacement routes')
        found[generation]=dict(generation=generation,source_sha256=hashlib.sha256(stream).hexdigest(),camera=context)
        if len(found)==len(wanted):break
    require(set(found)==wanted,'Four-view source route frame missing')
    for view in proof['views']:
        require(len(set(view['frames']))>=3 and all(found[g]['camera']['view']==view['view'] for g in view['frames']),
                'Four-view source/settled view differs')
    return [found[g] for g in sorted(found)]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--comparison', action='append', required=True)
    parser.add_argument('--report', required=True)
    args = parser.parse_args()
    require(len(args.comparison) == 4 and len(set(args.comparison)) == 4, 'Exactly four unique comparisons required')
    require(args.report.startswith(PREFIX) and not (ROOT / args.report).exists(), 'Fresh owned report required')
    register = json.loads((ROOT / 'research/runtime/coverage/register.json').read_text())
    behaviors = [b for b in register['behaviors'] if b['id'] in
                 {'NP.camera-cursor-picking-frame-witness', 'HOST.input-framebuffer-pixel-centres'}]
    builds = {b['id']: b for b in register['builds']}
    scenario = next(s for s in register['scenarios'] if s['id'] == 'native.camera-cursor-picking-four-domains')
    with Reader(ROOT) as reader:
        bindings, cases, sources = [], [], {}
        for name in args.comparison:
            c = reader.json(name)
            require(c['success'] and c['sources_stable'] and c['comparison_processes_stopped']
                    and c['comparison']['mismatching_pixels'] == 0 and c['proof']['verified'], 'Incomplete comparison')
            require(all(reader.digest(n) == h for n, h in c['sources'].items()), 'Comparison source changed')
            sources.update(c['sources'])
            bindings.append(dict(record=name, sha256=reader.digest(name)))
            p = c['proof']
            require(len(p['views']) == 8 and len(p['pans']) == 2 and len(p['selection_pairs']) == 10
                    and len(p['source_targets']) == 12 and len(p['cursor_bindings']) == 40,
                    'Incomplete four-view/two-scale interaction schedule')
            acquired=reader.json(c['acquisition']['record'])
            cases.append(dict(p,four_view_source_routes=view_routes(reader,acquired,p)))
        require({(p['domain'][0], tuple(p['domain'][1])) for p in cases} ==
                {(software, size) for software in (False, True) for size in ((800, 600), (640, 480))}, 'Missing/repeated domain')
        controls = []
        for name in ('input-after-02.json', 'xcb-input-01.json', 'controls-03.json', 'retained-controls-04.json', 'current-guards-11.json'):
            path = PREFIX + name
            c = reader.json(path)
            require(c['success'], 'Failed control')
            # Keep historical control source phases; only unchanged complete
            # binding sources below support the current contract claim.
            controls.append(dict(record=path, sha256=reader.digest(path)))
            if name == 'current-guards-11.json':
                require(c['sources_stable'] and c['python_tests'] == 30 and c['skips'] == 0
                        and all(reader.digest(n) == h for n,h in c['sources'].items()), 'Current guard sources changed')
        for name in set().union(*(required_sources(b) for b in behaviors)) | {'tools/close-rendering-interaction.py'}:
            sources[name] = reader.digest(name)
        result = dict(schema=1, success=True, kind='finite_camera_cursor_picking_matrix', sources=sources,
                      sources_stable=True, comparisons=bindings, controls=controls, cases=cases,
                      claims=[dict(behavior=b['id'], contract_sha256=behavior_contract(b, builds),
                                   scenarios={scenario['id']: scenario_contract(scenario)}) for b in behaviors],
                      original_source_sha256=builds['nocd-40209ca7']['source_sha256'],
                      primary_original_games=4, actual_domains=4, scaling_policies=['sharp', 'integer'],
                      view_windows=sum(len(p['views']) for p in cases), pan_windows=sum(len(p['pans']) for p in cases),
                      selection_toggles=sum(2 * len(p['selection_pairs']) for p in cases),
                      source_generations=sum(p['source_generations'] for p in cases),
                      world_presentations=sum(p['world_presentations'] for p in cases),
                      retained_pixels=sum(p['retained_pixels'] for p in cases), mismatching_pixels=0,
                      four_view_required_routes_proved=True,
                      four_view_source_route_frames=sum(len(p['four_view_source_routes']) for p in cases),
                      timing_qualification=dict(record=PREFIX+'software-first-toggle-gap-01.json',
                                                sha256=reader.digest(PREFIX+'software-first-toggle-gap-01.json')),
                      response_deadlines_proved=False, atomic_hud_snapshot_proved=False,
                      allocation_generations_proved=False, whole_game_picking_proved=False,
                      physical_hardware_provider_proved=False, full_rendering_completion_proved=False,
                      newly_passed_requirements=0, historical_failures_retired=False)
        print(json.dumps(dict(record=args.report, sha256=write_report(ROOT, args.report,
                         (json.dumps(result, indent=2) + '\n').encode()), **{k: result[k] for k in
                         ('success', 'source_generations', 'world_presentations', 'retained_pixels', 'selection_toggles')})))


if __name__ == '__main__':
    main()
