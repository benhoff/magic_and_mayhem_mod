#!/usr/bin/env python3
"""Analyze retained camera facets and negative pixels; incomplete matrices stay open."""
import argparse
import hashlib
import importlib.util
import json
import struct
import tempfile
from pathlib import Path

from coverage_evidence import Reader, write_report
from rendering_pilot import acquisition, require
from rendering_menu_acceptance import artifact
from campaign_input_observation import decode as admission_decode
from resident_capture import records

ROOT = Path(__file__).resolve().parents[1]
PREFIX = 'research/runtime/rendering-camera-matrix-results-20261106/'
LEAF = 'journey.camera-motion.four-orientations'
ACTIONS = ('center-first', 'center-second', 'rotate-1', 'rotate-2', 'rotate-3', 'rotate-4', 'pan-up', 'open-results')
SELECTED = ('center-second', 'rotate-1', 'rotate-2', 'rotate-3', 'rotate-4', 'pan-up')
VIEWS = (0, 3, 2, 1, 0, 0)


def module(name):
    s = importlib.util.spec_from_file_location('matrix_' + name.replace('-', '_'), ROOT / 'tools' / (name + '.py'))
    m = importlib.util.module_from_spec(s); s.loader.exec_module(m)
    return m


def context_many_cells(rows):
    """Validate every actual Cell call with the unchanged single-cell contract."""
    old = module('close-rendering-source-camera')
    cells = {i for i, (f, b) in enumerate(rows) if f[2] == 24 and struct.unpack_from('<I', b)[0] == 0}
    if not cells:
        return old.context(rows)  # Startup is explicit; missing required calls refuse.
    checked = [old.context([(f, b) for j, (f, b) in enumerate(rows) if j not in cells or j == i]) for i in sorted(cells)]
    result = dict(checked[0], cell_sequences=[v['cell_sequence'] for v in checked])
    require(all(v['camera_sequence'] == result['camera_sequence'] and v['creature_sequence'] == result['creature_sequence']
                for v in checked), 'Cell calls do not share the same camera/creature context')
    return result


def source_contexts(reader, acquired):
    decoder = module('inspect-canvas-producers'); contexts, chunks = {}, 0
    with tempfile.TemporaryDirectory(prefix='camera-matrix-source-', dir=ROOT / 'working') as directory:
        path = Path(directory) / 'worlds.bin'; path.write_bytes(artifact(reader, acquired, 'worlds.bin'))
        for generation, chunk, stream, request in records(path):
            _, rows = decoder.decode(stream, complete=False)
            context = context_many_cells(rows)
            if chunk:
                chunks += 1; require(context is None, 'Camera packet crosses a source chunk'); continue
            require(generation == len(contexts) + 1, 'Source generation missing/reordered')
            contexts[generation] = dict(generation=generation, source_sha256=hashlib.sha256(stream).hexdigest(), camera=context)
    require(contexts and chunks > 0, 'Missing owned source prefix/history')
    return contexts, chunks


def camera_windows(rows):
    require(len(rows) == len(ACTIONS) * 3, 'Missing bounded camera schedule')
    result, previous = {}, 0
    for i, action in enumerate(ACTIONS):
        group = rows[3*i:3*i+3]
        require([r['phase'] for r in group] == ['begin', 'released', 'settled']
                and all(r['action'] == action and r['accepted'] is True and r['host_focus']['host_focused'] is True for r in group),
                'Wrong/refused physical camera action')
        stamps = [int(r['host_us']) for r in group]
        require(previous <= stamps[0] <= stamps[1] < stamps[2], 'Reordered camera action clocks')
        previous = stamps[-1]
        result[action] = dict(zip(('begin', 'released', 'settled'), group))
    return result


def action_sources(rows, paints):
    windows = camera_windows(rows); result = []
    for action, view in zip(SELECTED, VIEWS):
        start, end = (int(windows[action][p]['host_us']) for p in ('released', 'settled'))
        interval = [p for p in paints if start < p['host_us'] <= end]
        generations = list(dict.fromkeys(p['world'] for p in interval))
        require(len(generations) >= 3 and all(p['camera'] is not None for p in interval), 'No source-bound camera interval')
        settled = [p for p in interval if p['world'] in generations[-3:]]
        for paint in settled:
            c = paint['camera']
            require(c['view'] == view and c['creature_markers'] > 0
                    and {r['backend'] for r in c['raster']} >= {1, 2, 9, 10},
                    'Wrong settled view or missing indexed scene/shadow/displacement/marker routes')
        result.append(dict(action=action, expected_view=view, paints=len(interval), generations=generations,
                           settled_paints=settled, last_camera=settled[-1]['camera']))
    require(result[-2]['last_camera']['center'] != result[-1]['last_camera']['center'], 'Source map center did not pan')
    return result


def assess(reader, acquired, compared, observed):
    """Strict prospective positive gate; no successful camera case has passed it yet."""
    config = acquired['configuration']; size = config['actual_size']
    require(size == ([800, 600] if config['high_resolution'] else [640, 480])
            and type(config['software_flag']) is bool and config['skip_intros'] is False
            and ('--software-rendering' in acquired['command']) is config['software_flag']
            and config.get('viewport_scaling') == 'integer'
            and config['compiled_test_driver_invoked'] is False and config['terrain_lighting'] == 'native', 'Wrong camera domain/policy')
    require(acquired['success'] is True and acquired['original_manifest_verified_before_after'] is True
            and acquired['original_game_runs'] == 1
            and acquired['private_wine_stopped'] is True and acquired['prefix_pool']['returned'] is True
            and not acquired.get('forced_private_prefix_stop', False), 'Incomplete nominal camera acquisition')
    require(compared['comparison']['success'] is True and compared['comparison']['mismatching_pixels'] == 0
            and compared['original_reference']['continuous_from_owned_initial_history'] is True
            and compared['original_reference']['native_player_pixels_read'] is False
            and compared['original_reference']['original_destination_capture_read'] is False
            and hashlib.sha256(observed['commands']).hexdigest() == compared['captured_command_stream_sha256'],
            'Uncompared/dependent camera history')
    contexts, chunks = source_contexts(reader, acquired)
    life = module('close-rendering-world-exit').imported_lifetimes(observed['commands'])
    worlds = compared['world_domain']['worlds']
    require(len(worlds) == len(contexts) == len(life['imports']) == compared['original_reference']['worlds']
            and chunks == compared['original_reference']['source_chunks']
            and all(r == dict(generation=i, width=size[0], height=size[1]) for i, r in enumerate(worlds, 1)), 'Mixed camera source generation/domain')
    expected = [r for r in compared['comparison']['comparisons'] if r['world'] > 0]
    shown = [observed['paints'][p] for p in observed['swaps'] if int(observed['paints'][p][0]['world']) > 0]
    require(shown and len(shown) == len(expected), 'Camera output omits/duplicates actual swaps')
    paints, hashes = [], {}
    for (p, body), match in zip(shown, expected):
        g, paint = int(p['world']), int(p['paint'])
        require(g in contexts and match['world'] == g and match['paint'] == paint
                and match['lease'] == int(p['lease']) > 0, 'Camera source/paint/lease mismatch')
        imported = life['imports'][g-1]
        require(imported['import_sequence'] <= p['commands'] < imported['deleted_sequence']
                and p['native_gpu_frame'] is p['gpu_read_complete'] is p['pixels_captured'] is True
                and p['presentation_owner'] == 'gui' and [p['frame_width'], p['frame_height']] == size,
                'Camera output lacks a live native GPU source')
        require(p['scaling'] == 2, 'Camera matrix must use the accepted Integer display policy')
        if paint not in hashes:
            hashes[paint] = hashlib.sha256(body() if callable(body) else body).hexdigest()
        require(match['actual_sha256'] == match['expected_sha256'] == hashes[paint]
                and match['pixels'] == p['width'] * p['height'] and match['mismatching_pixels'] == 0, 'Camera viewport pixels changed')
        paints.append(dict(world=g, paint=paint, lease=int(p['lease']), host_us=int(p['host_us']),
                           pixels=match['pixels'], rgba_sha256=hashes[paint], camera=contexts[g]['camera']))
    actions = action_sources(acquired['camera_actions'], paints)
    admission = admission_decode(artifact(reader, acquired, 'input-admission.bin'))
    marker_binding = module('rendering_gameplay_acceptance').bind_markers(acquired['camera_actions'], admission)
    require(all(row['thread'] == admission['owner_thread'] for row in acquired['states']), 'Camera/menu original owners differ')
    for marker in acquired['camera_actions']:
        original = admission['rows'][marker['original_row']-1]
        require(marker['original_thread'] == admission['owner_thread']
                and marker['world_return_ordinal'] == original['world_return_ordinal'], 'Camera original return token differs')
    events = module('test-menu-observer').decode(artifact(reader, acquired, 'events.bin'))
    callbacks = [[r['menu_id'], r['argument']] for r in events if r['event'] == 3]
    require(callbacks[:8] == [[3, 3], [10, 1], [3, 2], [22, 2], [14, 2], [25, 0], [14, 1], [7, 12]]
            and [26, 2] in callbacks and callbacks[-3:] == [[26, 2], [22, 3], [3, 4]], 'Missing original camera entry/Results/Quit callbacks')
    require(acquired['proof']['map']['map'] == 1 and acquired['proof']['map']['map_name'] == 'Celtic Forest One (small)'
            and acquired['proof']['preferences'][2] == int(not config['high_resolution']), 'Wrong map/original resolution selector')
    cleanup = module('join-rendering-normal-acceptance').cleanup_proof(reader, acquired)
    require(cleanup['verified'] and cleanup['normal_quit'] and cleanup['source_imports'] == len(contexts), 'Camera source work did not retire')
    provider = acquired['resident']['renderer']
    require(isinstance(provider, str) and provider, 'Actual OpenGL provider unrecorded')
    return dict(verified=True, domain=['player-software' if config['software_flag'] else 'player-default', size],
        identity=acquired['session_identity'], provider=provider, actions=actions, source_generations=len(contexts),
        source_chunks=chunks, world_presentations=len(paints), retained_pixels=sum(v['pixels'] for v in paints),
        source_contexts=list(contexts.values()), compared_paints=paints, marker_binding=marker_binding,
        original_callbacks=callbacks, cleanup=cleanup,
        scope='Actual external physical Qt four-view camera cycle on Celtic Forest One with native lighting and the existing Integer display policy, exact owned terrain/minimap source arguments including every ordered Cell call, nonempty creature markers and indexed scene/shadow/displacement routes, complete independent original viewport output and normal original Quit. Pan source-center changes are retained but full pan/picking/HUD alignment and noninteger Sharp sampling remain separate.')



def source_facets(reader, acquired):
    """Source/action evidence remains separate from independent pixel acceptance."""
    contexts, chunks = source_contexts(reader, acquired)
    streams = [artifact(reader, acquired, Path(v['path']).name) for v in acquired['artifacts']
               if '/capture/' in v['path'] and v['path'].endswith('.bin')]
    observed = acquisition(streams, acquired['session_identity'])
    paints = []
    for index in observed['swaps']:
        row, _ = observed['paints'][index]
        generation = int(row['world'])
        if generation:
            require(generation in contexts, 'Unowned camera source generation')
            paints.append(dict(world=generation, paint=int(row['paint']), host_us=int(row['host_us']),
                               camera=contexts[generation]['camera']))
    actions = action_sources(acquired['camera_actions'], paints)
    counts = {}
    for value in contexts.values():
        camera = value['camera']
        count = len(camera['cell_sequences']) if camera else 0
        counts[str(count)] = counts.get(str(count), 0) + 1
    return dict(verified=True, source_generations=len(contexts), source_chunks=chunks,
                cell_call_counts=counts, source_contexts=list(contexts.values()),
                source_bound_paints=paints, actions=actions, whole_viewport_accepted=False)


def diagnose_sharp(r):
    import numpy as np
    from rendering_pilot import History, commands, viewport, inflate, counter
    cn=PREFIX+'software800-comparison-01.json';c=r.json(cn);a=r.json(c['acquisition']['record']);require(not c['success'] and a['success'],'Expected retained negative comparison')
    streams=[artifact(r,a,Path(v['path']).name) for v in a['artifacts'] if '/capture/' in v['path'] and v['path'].endswith('.bin')];observed=acquisition(streams,a['session_identity'])
    references={v['path']:v for v in c['artifacts']}
    def retained(name):
     x=references[name];raw=r.read_bytes(x['record']);require(hashlib.sha256(raw).hexdigest()==x['sha256'],'Retained expected data changed');return raw
    index=json.loads(retained('reference/worlds.json'));worlds={int(v['generation']):np.frombuffer(inflate(retained('reference/'+v['path'])),dtype='<u2').astype('uint32').reshape(v['height'],v['width']) for v in index['worlds']}
    history=History(worlds);exports=iter(observed['exports'].values());pending=next(exports,None);by_lease={}
    for i in observed['swaps']:
     p,body=observed['paints'][i];lease=p.get('reference_binding')
     if lease:by_lease.setdefault(lease,[]).append((i,p,body))
    rows=[];expected_rows=iter(c['comparison']['comparisons'])
    for op,seq,payload in commands(observed['commands']):
     history.apply(op,seq,payload)
     while pending is not None and pending['commands']==seq:
      lease=counter(pending['lease'])
      if lease in by_lease:
       frame=history.render(pending)
       for ordinal,p,body in by_lease[lease]:
        actual=np.frombuffer(body() if callable(body) else body,dtype='u1').reshape(p['height'],p['width'],4)
        expected=np.frombuffer(viewport(frame,p),dtype='u1').reshape(actual.shape);different=np.any(actual!=expected,axis=2)
        recorded=next(expected_rows);count=int(different.sum())
        require(recorded['paint']==ordinal and recorded['world']==int(p['world'])
                and recorded['reference_binding']==lease and recorded['lease']==int(p['lease'])
                and recorded['mismatching_pixels']==count
                and recorded['actual_sha256']==hashlib.sha256(actual.tobytes()).hexdigest()
                and recorded['expected_sha256']==hashlib.sha256(expected.tobytes()).hexdigest(),
                'Negative reproduction identity/digests differ')
        left,top,w,h=(p[k] for k in ('left','top','image_width','image_height'));fw,fh=frame.shape[1],frame.shape[0]
        xnum=(2*np.arange(w)+1)*fw;ynum=(2*np.arange(h)+1)*fh
        tx=xnum%(2*w)==0;ty=ynum%(2*h)==0
        tied=np.zeros(different.shape,dtype=bool);tied[top:top+h,left:left+w]=tx[None,:]|ty[:,None]
        outside=int(np.count_nonzero(different&~tied));neighbor=np.zeros(different.shape,dtype=bool)
        xs=np.minimum(xnum//(2*w),fw-1);ys=np.minimum(ynum//(2*h),fh-1)
        for dx,dy in ((1,0),(0,1),(1,1)):
         allowed=(tx[None,:] if dx else np.ones((1,w),dtype=bool))&(ty[:,None] if dy else np.ones((h,1),dtype=bool))
         previous=frame[np.maximum(ys-dy,0)[:,None],np.maximum(xs-dx,0)[None,:]]
         neighbor[top:top+h,left:left+w]|=allowed&np.all(actual[top:top+h,left:left+w]==previous,axis=2)
        unexplained=int(np.count_nonzero(different&~neighbor))
        rows.append(dict(paint=ordinal,world=int(p['world']),pixels=p['width']*p['height'],mismatching=count,non_tie_mismatches=outside,not_adjacent_expected_texel=unexplained,source=[fw,fh],viewport=[p['width'],p['height']],image=[left,top,w,h],scaling=p['scaling']))
      pending=next(exports,None)
    require(pending is None and len(rows)==len(c['comparison']['comparisons']),'Incomplete retained negative reproduction')
    return dict(verified=True, comparison=dict(record=cn, sha256=r.digest(cn)), rows=rows,
                total_mismatches=sum(v['mismatching'] for v in rows),
                non_tie_mismatches=sum(v['non_tie_mismatches'] for v in rows),
                not_adjacent_expected_texel=sum(v['not_adjacent_expected_texel'] for v in rows),
                whole_viewport_accepted=False)


def analyze(reader):
    stopped = reader.json(PREFIX + 'batch-stop-01.json')
    require(stopped['stop_further_original_runs'] and stopped['original_game_runs'] == 3,
            'Missing exact stopped-batch accounting')
    name = PREFIX + 'software800-acquisition-02.json'
    acquired = reader.json(name)
    facets = source_facets(reader, acquired)
    negative = diagnose_sharp(reader)
    require(facets['source_generations'] == 215 and facets['cell_call_counts'] == {'0': 1, '1': 120, '2': 94}
            and negative['total_mismatches'] == 3819024 and negative['non_tie_mismatches'] == 0
            and negative['not_adjacent_expected_texel'] == 0, 'Retained diagnostic result changed')
    misses = []
    for filename, screen, sequence in (('software800-integer-acquisition-02.json', 25, 5),
                                       ('software800-integer-acquisition-03.json', 3, 2)):
        report = PREFIX + filename; failed = reader.json(report)
        relative = str(Path(failed['session']).relative_to(ROOT) / 'channel.bin')
        entry = next(v for v in failed['artifacts'] if v['path'] == relative)
        raw = reader.read_bytes(entry['record'])
        require(hashlib.sha256(raw).hexdigest() == entry['sha256'], 'Changed failed-input wire')
        fields = struct.unpack_from('<40I', raw)
        callbacks = [[v['menu_id'], v['argument']] for v in module('test-menu-observer').decode(artifact(reader, failed, 'events.bin')) if v['event'] == 3]
        require(not failed['success'] and failed['original_game_runs'] == 1
                and fields[7] == fields[36] == sequence and fields[34:38] == (screen, 1, sequence, 1),
                'Failure is no longer the recorded missing host request')
        require(([25, 0] if screen == 25 else [3, 2]) not in callbacks, 'Expected callback was delivered')
        misses.append(dict(record=report, sha256=reader.digest(report), wire=entry, screen=screen,
                           request=sequence, acknowledged=sequence, ready=True, status=1,
                           callbacks=callbacks, precise_mouse_delivery_cause_isolated=False,
                           normal_quit=False))
    return dict(schema=1, kind='retained_camera_source_and_negative_sampling_analysis', success=True,
                acquisition=dict(record=name, sha256=reader.digest(name)), source_facets=facets,
                sharp_sampling=negative, input_misses=misses, batch_stop=dict(record=PREFIX+'batch-stop-01.json',
                sha256=reader.digest(PREFIX+'batch-stop-01.json')), analysis_original_game_runs=0,
                batch_original_game_runs=3, zero_game_preparations=2, full_orientation_matrix_passed=False,
                newly_passed_requirements=0, passed_requirements=38, pending_requirements=118,
                historical_failures_retired=False)


def main():
    p = argparse.ArgumentParser(description=__doc__); p.add_argument('--report', required=True); a = p.parse_args()
    require(a.report.startswith(PREFIX) and '..' not in Path(a.report).parts and not (ROOT / a.report).exists(), 'Fresh owned result required')
    result = dict(schema=1, kind='retained_camera_source_and_negative_sampling_analysis', success=False)
    try:
        with Reader(ROOT) as reader:
            result = analyze(reader)
            reg = json.loads((ROOT / 'research/runtime/coverage/register.json').read_text())
            behavior = next(v for v in reg['behaviors'] if v['id'] == 'NP.camera-capture-diagnostics')
            names = set(behavior['implementation'] + behavior['tests'] + behavior['validation_dependencies'])
            plan = reader.json(PREFIX + 'input-plan-01.json')
            result['sources'] = plan['compiler_sources'] | plan['oracle']['native_sources'] | {n: reader.digest(n) for n in sorted(names)}
            require(all(reader.digest(n) == h for n, h in result['sources'].items()), 'Native/source phase changed')
    except Exception as error: result['error'] = str(error)
    h = write_report(ROOT, a.report, (json.dumps(result, indent=2) + '\n').encode())
    print(json.dumps(dict(success=result['success'], error=result.get('error'), record=a.report, sha256=h)), flush=True)
    return 0 if result['success'] else 1


if __name__ == '__main__': raise SystemExit(main())
