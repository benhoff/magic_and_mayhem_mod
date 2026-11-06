#!/usr/bin/env python3
"""Bind retained camera producer arguments to exact, compared World generations."""
import argparse
import hashlib
import importlib.util
import json
import struct
import tempfile
from collections import Counter
from pathlib import Path

from coverage_evidence import Reader, write_report
from rendering_gameplay_acceptance import windows, bind_markers
from rendering_menu_acceptance import artifact
from rendering_pilot import acquisition, require
from resident_capture import records
from campaign_input_observation import decode as admission_decode

ROOT = Path(__file__).resolve().parents[1]
PREFIX = 'research/runtime/rendering-source-camera-results-20261106/'
ACQUISITION = 'research/runtime/rendering-normal-results-20261031/gameplay-default-800-acquisition-03.json'
COMPARISON = 'research/runtime/rendering-normal-results-20261031/gameplay-default-800-comparison-03.json'
ACTIONS = ('center-second', 'rotate-1', 'rotate-2', 'rotate-3', 'rotate-4', 'pan-up')
VIEWS = (0, 3, 2, 1, 0, 0)


def module(name):
    spec = importlib.util.spec_from_file_location('camera_' + name.replace('-', '_'), ROOT / 'tools' / (name + '.py'))
    value = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(value)
    return value


def context(rows):
    """These are owned arguments in a source packet, not periodic samples."""
    overlays = [(f, struct.unpack_from('<37I', body)) for f, body in rows if f[2] == 24]
    terrain = [f for f, body in rows if f[2] == 22]
    if not overlays:
        require(not terrain, 'Terrain without owned camera context')
        return None  # Explicit startup prefix; never a camera witness.
    require([m[0] for f, m in overlays] == [2, 0, 1] and len(terrain) == 1,
            'Missing/reordered camera, cell, creature or terrain producer')
    f, camera = overlays[0]
    view, center = camera[9], list(camera[3:5])
    require(view in range(4) and all(m[9] == view and list(m[3:5]) == center for _, m in overlays),
            'Camera/marker orientations or centers disagree')
    t = terrain[0]
    require(t[16] == view and list(t[10:12]) == center and t[3] == f[3]
            and all(row[3] == f[3] for row, _ in overlays), 'Terrain/overlay source identity differs')
    require(f[17] == (0, 2, 0, 1)[view] and f[1] < overlays[1][0][1] < overlays[2][0][1],
            'Wrong original four-case camera body/order')
    raster = Counter((row[14], row[15], row[17], row[22]) for row, _ in rows if row[2] == 9)
    require(raster, 'Missing scene raster producers')
    return dict(view=view, center=center, surface=f[3], camera_sequence=f[1],
                camera_body=f[17], terrain_sequence=t[1], cell_sequence=overlays[1][0][1],
                creature_sequence=overlays[2][0][1], creature_markers=overlays[2][1][17],
                raster=[dict(operation=k[0], backend=k[1], indexed=k[2], caller=k[3], count=n)
                        for k, n in sorted(raster.items())])


def source_contexts(reader, acquired):
    decoder = module('inspect-canvas-producers')
    contexts, chunks = {}, 0
    # Existing reader consumes a path. This bounded interchange file is removed
    # on exit; the acquisition's immutable SQLite object remains authoritative.
    with tempfile.TemporaryDirectory(prefix='source-camera-', dir=ROOT / 'working') as directory:
        path = Path(directory) / 'worlds.bin'
        path.write_bytes(artifact(reader, acquired, 'worlds.bin'))
        for generation, chunk, stream, request in records(path):
            _, rows = decoder.decode(stream, complete=False)
            if chunk:
                chunks += 1
                require(context(rows) is None, 'Camera context crosses a source chunk boundary')
                continue
            require(generation == len(contexts) + 1, 'Source generation missing/reordered')
            contexts[generation] = dict(generation=generation, source_sha256=hashlib.sha256(stream).hexdigest(),
                                        camera=context(rows))
    require(contexts and chunks > 0, 'Missing complete owned source history')
    return contexts, chunks


def output_bindings(acquired, compared, observed, contexts, lifetimes):
    require(acquired['configuration']['gameplay_witness'] is True
            and acquired['configuration']['software_flag'] is False
            and acquired['configuration']['skip_intros'] is False
            and acquired['success'] is compared['success'] is True
            and acquired['private_wine_stopped'] is True, 'Wrong/incomplete gameplay execution')
    require(compared['acquisition']['sha256'] and compared['original_reference']['continuous_from_owned_initial_history'] is True
            and compared['original_reference']['native_player_pixels_read'] is False
            and compared['original_reference']['original_destination_capture_read'] is False
            and compared['comparison']['success'] is True and compared['comparison']['mismatching_pixels'] == 0
            and compared['comparison']['remaining_surfaces'] == 0, 'Incomplete/dependent original history comparison')
    require(hashlib.sha256(observed['commands']).hexdigest() == compared['captured_command_stream_sha256'],
            'Different compared commands')
    world_rows = compared['world_domain']['worlds']
    require(compared['world_domain']['verified'] is True
            and len(world_rows) == len(contexts) == len(lifetimes['imports']) == compared['original_reference']['worlds']
            and all(row == dict(generation=i, width=800, height=600) for i, row in enumerate(world_rows, 1)),
            'Source generation/domain accounting differs')
    require(all(int(acquired['resident'][k]) == len(contexts) for k in ('imports', 'completed', 'verified_returns'))
            and acquired['resident']['pending'] is False and acquired['resident']['normal_quit_retired'] is True,
            'Missing returned source or normal retirement')
    shown = [observed['paints'][p] for p in observed['swaps'] if int(observed['paints'][p][0]['world']) > 0]
    expected = [v for v in compared['comparison']['comparisons'] if v['world'] > 0]
    require(shown and len(shown) == len(expected), 'Missing actual swap occurrence')
    result = []
    hashes = {}
    for (paint, body), match in zip(shown, expected):
        generation, ordinal = int(paint['world']), int(paint['paint'])
        require(generation in contexts and match['world'] == generation and match['paint'] == ordinal
                and match['lease'] == int(paint['lease']) > 0, 'Paint/lease/source comparison identity differs')
        imported = lifetimes['imports'][generation - 1]
        require(imported['import_sequence'] <= paint['commands'] < imported['deleted_sequence']
                and paint['presentation_owner'] == 'gui' and paint['native_gpu_frame'] is True
                and paint['gpu_read_complete'] is True and paint['pixels_captured'] is True,
                'Paint lacks a live owned source import or completed GPU output')
        if ordinal not in hashes:
            hashes[ordinal] = hashlib.sha256(body() if callable(body) else body).hexdigest()
        require(match['mismatching_pixels'] == 0 and match['actual_sha256'] == match['expected_sha256'] == hashes[ordinal]
                and match['pixels'] == paint['width'] * paint['height'] > 0, 'Changed/incomplete compared paint')
        result.append(dict(paint=ordinal, world=generation, lease=int(paint['lease']),
                           host_us=int(paint['host_us']), pixels=match['pixels'], rgba_sha256=hashes[ordinal],
                           import_sequence=imported['import_sequence'], command_sequence=paint['commands'],
                           camera=contexts[generation]['camera']))
    return result


def action_bindings(markers, paints):
    schedule = windows(markers)
    result = []
    for action, view in zip(ACTIONS, VIEWS):
        start, end = (int(schedule[action][p]['host_us']) for p in ('released', 'settled'))
        interval = [p for p in paints if start < p['host_us'] <= end]
        generations = list(dict.fromkeys(p['world'] for p in interval))
        require(len(generations) >= 3 and all(p['camera'] is not None for p in interval),
                'No source-bound camera progress inside action')
        tail = generations[-3:]
        selected = [p for p in interval if p['world'] in tail]
        require(all(p['camera']['view'] == view for p in selected), 'Settled source orientation differs')
        for p in selected:
            c = p['camera']
            require(c['creature_markers'] > 0
                    and {r['backend'] for r in c['raster']} >= {0, 1, 2, 3, 9, 10},
                    'Missing captured scene/marker/shadow/displacement routes')
        result.append(dict(action=action, expected_view=view, paints=len(interval), generations=generations,
                           settled_paints=selected, last_camera=selected[-1]['camera']))
    require([r['last_camera']['view'] for r in result] == list(VIEWS), 'Four-view source cycle incomplete')
    require(result[-2]['last_camera']['center'] != result[-1]['last_camera']['center'],
            'Pan did not change the source map center')
    return result


def analyze(reader):
    previous, impact = module('close-rendering-world-exit').parent_plan(
        ROOT, reader, 'MR06.natural-movie-return-closed01', 'NP.normal-movie-return-closeout')
    accepted, world_impact = module('close-rendering-world-exit').parent_plan(
        ROOT, reader, 'WX06.normal-world-exit-closed01', 'NP.normal-world-exit-closeout')
    old = reader.json(accepted['inputs']['progress']['record'])
    retained = next(v for v in old['executions'] if v['scenario'] == 'normal-quick' and v['domain'] == ['player-default', [800, 600]])
    pair = retained['pair']
    require(pair['acquisition']['record'] == ACQUISITION and pair['comparison']['record'] == COMPARISON,
            'Gameplay input is not the accepted normal execution')
    a, c = (reader.json(pair[k]['record']) for k in ('acquisition', 'comparison'))
    require(all(reader.digest(pair[k]['record']) == pair[k]['sha256'] for k in pair)
            and c['acquisition'] == pair['acquisition'], 'Mixed/changed accepted acquisition')
    sources = dict(previous['sources'])
    # The accepted parent checks native/runtime and oracle sources. The old
    # gameplay report keeps its historical shared-test hash, without refresh.
    require(all(reader.digest(n) == h for n, h in sources.items()), 'Accepted native source phase changed')
    streams = [artifact(reader, a, Path(v['path']).name) for v in a['artifacts']
               if '/capture/' in v['path'] and v['path'].endswith('.bin')]
    observed = acquisition(streams, a['session_identity'])
    contexts, chunks = source_contexts(reader, a)
    require(chunks == c['original_reference']['source_chunks'], 'Source chunks differ from independent replay')
    lifetimes = module('close-rendering-world-exit').imported_lifetimes(observed['commands'])
    paints = output_bindings(a, c, observed, contexts, lifetimes)
    admission = admission_decode(artifact(reader, a, 'input-admission.bin'))
    markers = bind_markers(a['flow']['gameplay_witness'], admission)
    actions = action_bindings(a['flow']['gameplay_witness'], paints)
    for name in ('tools/close-rendering-source-camera.py', 'tests/test-rendering-source-camera-closeout.py',
                 'tools/inspect-canvas-producers.py', 'tools/resident_capture.py',
                 'tools/rendering_gameplay_acceptance.py', 'tools/campaign_input_observation.py',
                 'reconstruction/rendering/minimap_camera_dispatch.hpp'):
        sources[name] = reader.digest(name)
    progress = reader.json(previous['inputs']['progress']['record'])
    return dict(schema=1, kind='native_rendering_source_camera_output_closeout', success=True, sources=sources,
        accepted_parent=dict(evidence='MR06.natural-movie-return-closed01', binding=impact['retained_input'], applicability=impact),
        world_parent=dict(evidence='WX06.normal-world-exit-closed01', binding=world_impact['retained_input'], applicability=world_impact),
        pair=pair, identity=a['session_identity'], domain=retained['domain'], provider=retained['provider'],
        source_contexts=list(contexts.values()), source_chunks=chunks, original_marker_binding=markers,
        compared_paints=paints, actions=actions, source_generations=len(contexts),
        world_presentations=len(paints), retained_pixels=sum(p['pixels'] for p in paints),
        source_orientation_to_presented_output_proved=True, source_map_center_pan_proved=True,
        exact_periodic_scalar_to_source_frame_binding_proved=False,
        full_camera_picking_hud_requirement_proved=False, original_game_runs=0, new_pixel_comparisons=0,
        new_native_code=False, new_game_fixture=False, newly_passed_requirements=0,
        requirements=progress['requirements'], dossier_identity=progress['dossier_identity'],
        closed_leaf_ids=progress['closed_leaf_ids'], passed_requirements=38, pending_requirements=118,
        decisions=progress['decisions'], historical_failures_retired=False,
        scope='Close the finite default800 Quick source-camera/output binding gap: exact owned minimap/terrain arguments, four-case body/order, nonempty creature markers and captured raster/shadow/displacement routes join to source generations, live import incarnations and every independently compared native swap. Six physical action windows show0,3,2,1,0 and changed source map center. This is source-argument identity, not periodic HUD/semantic scalar identity, picking alignment, actor animation/combat, whole minimap or pause semantics. Software/640 gameplay and whole camera leaves remain open; preserve all38accepted rows and both pending decisions. No new game, pixel comparison, native code/build or fixture.')


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--report', required=True)
    args = p.parse_args()
    require(args.report.startswith(PREFIX) and '..' not in Path(args.report).parts and not (ROOT / args.report).exists(),
            'Fresh owned immutable report required')
    result = dict(schema=1, kind='native_rendering_source_camera_output_closeout', success=False, original_game_runs=0)
    try:
        with Reader(ROOT) as reader:
            result = analyze(reader)
    except Exception as error:
        result['error'] = str(error)
    digest = write_report(ROOT, args.report, (json.dumps(result, indent=2) + '\n').encode())
    print(json.dumps(dict(success=result['success'], error=result.get('error'), record=args.report, sha256=digest,
                          source_generations=result.get('source_generations'), actions=len(result.get('actions', [])))))
    return 0 if result['success'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
