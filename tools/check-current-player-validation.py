#!/usr/bin/env python3
"""Admit one current public-player pair; preserve finite scope and launch budget."""
import argparse
import hashlib
import importlib.util
import json
import math
import sqlite3
from pathlib import Path
from coverage_evidence import Reader, write_report
from rendering_build_binding import native_runtime_source_paths, require_player_source_closure
from rendering_pilot import acquisition, require
from rendering_gameplay_acceptance import assess, bind_markers, minimap, windows
from drawing_hud_state import decode as hud_decode
from drawing_semantic_state import decode as semantic_decode
from drawing_actor_latches import decode as latch_decode, bind_callbacks
from drawing_region_input import decode as region_decode
from campaign_input_observation import decode as admission_decode

ROOT = Path(__file__).resolve().parents[1]
PREFIX = 'research/runtime/whole-player-validation-results-20261110/'


def joined():
    spec = importlib.util.spec_from_file_location('current_player_normal_join', ROOT/'tools/join-rendering-normal-acceptance.py')
    value = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(value)
    return value


def response_progress(a, observed, comparison):
    """Keep source/command/driver deadlines distinct from diagnostic FPS."""
    resident = a['resident']
    profile = resident['stage_profile']
    samples = profile['samples']
    require(profile['samples_dropped'] == 0 and len(samples) == int(resident['completed'])
            and [int(s['generation']) for s in samples] == list(range(1, len(samples)+1)),
            'Incomplete source timing/progress sample history')
    require(all(k in sample and math.isfinite(float(sample[k])) and float(sample[k]) >= 0
                for sample in samples for k in ('producer_wait_ack_ms', 'native_consume_ms')),
            'Missing or invalid source deadline measurement')
    maximum = {k:max(float(s.get(k, 0)) for s in samples) for k in
               ('producer_wait_ack_ms', 'producer_world_ms', 'native_consume_ms', 'engine_between_world_ms')}
    # These limits are existing wire/driver contracts, not an FPS-derived gate.
    require(maximum['producer_wait_ack_ms'] < 120000 and maximum['native_consume_ms'] < 120000,
            'Original World acknowledgement exceeded its existing 120s producer deadline')
    require(int(resident['source_chunk_max_records']) <= 16384
            and int(resident['source_chunk_max_interval_records']) <= 16384,
            'Source chunk work bound exceeded')
    capture = profile['capture']
    require(capture['peak_queue_bytes'] <= capture['queue_limit_bytes'] == 256*1024**2
            and capture['queue_bytes'] == 0 and capture['file_bytes'] <= 2*1024**3,
            'Source retention queue/file bound exceeded')
    paint_rows = [observed['paints'][i][0] for i in observed['swaps']]
    shown = {int(p['world']) for p in paint_rows if int(p['world']) > 0}
    require(len(shown) >= 3 and max(shown) > 5 and len(samples) > 5
            and comparison['comparison']['world_presentations'] > 0,
            'Repaint repetition cannot establish advancing source work')
    result = dict(verified=True,source_generations=len(samples),presented_generations=len(shown),
        maximum_observed_ms=maximum,source_ack_deadline_ms=120000,
        source_queue_peak_bytes=capture['peak_queue_bytes'],source_queue_limit_bytes=capture['queue_limit_bytes'],
        source_file_bytes=capture['file_bytes'],source_file_limit_bytes=2*1024**3,
        command_pressure_deadline_ms=5000,menu_action_deadline_ms=10000,
        command_and_menu_deadlines_enforced_by_current_runtime=True,
        fps_diagnostic=resident['engine_fps'],fps_acceptance_threshold=None,
        scope='Finite timed session: complete original source returns, advancing independently compared presentations, current command/menu watchdogs and bounded lossless capture. Menu/movie intervals are not active-World frame deadlines; no physical VRAM, unbounded soak or scheduling guarantee.')
    if a['scenario'] == 'normal-quick':
        flow = a['flow']
        require(flow['original_input_wait'] is True and flow['original_input_wait_limit_ms'] == 3000
                and comparison['input_acknowledgements']['verified'] is True,
                'Original input effect did not satisfy the existing 3s deadline')
        result['input_ack_deadline_ms'] = 3000
        if a['configuration'].get('gameplay_witness') is True:
            intervals = windows(flow['gameplay_witness'])
            result['gameplay_action_windows_ms'] = {name:(int(v['settled']['host_us'])-int(v['begin']['host_us']))/1000
                                                    for name,v in intervals.items()}
    else:
        require(a['configuration']['deadline_ms'] == 530000 and a['flow']['success'] is True,
                'Enabled-intro storage route did not meet its existing total deadline')
        result['storage_route_deadline_ms'] = 530000
    return result


def assess_pair(reader, acquisition_name, comparison_name):
    ref = lambda n:dict(record=n,sha256=reader.digest(n))
    pair = dict(acquisition=ref(acquisition_name),comparison=ref(comparison_name))
    a,c = (reader.json(n) for n in (acquisition_name,comparison_name))
    profile = a['current_player_profile']
    require(a['runtime_sources_stable'] is True and a['compiler_closure_stable'] is True
            and a['player_binary_stable'] is True and c['comparison_processes_stopped'] is True,
            'Current runtime/compiler/binary or comparison process ownership is unproved')
    driver={'quick-low':'tools/run-current-player-low-validation.py','quick-low-fixture':'tools/run-current-player-low-fixture.py'}.get(a['current_player_scenario'],'tools/run-current-player-validation.py')
    require_player_source_closure(a['sources'],native_runtime_source_paths(ROOT)|{driver})
    normal = joined()
    admitted = normal.sustained_admit(reader,pair,profile)
    streams = [reader.read_bytes(v['record']) for v in a['artifacts'] if '/capture/' in v['path'] and v['path'].endswith('.bin')]
    observed = acquisition(streams,a['session_identity'])
    require(hashlib.sha256(observed['commands']).hexdigest() == c['captured_command_stream_sha256'],
            'Resource/action analysis differs from independently compared commands')
    resources = normal.resource_history(observed['commands'])
    require(len(resources['worlds']) == int(a['resident']['completed']) == c['original_reference']['worlds'],
            'Source/resource/oracle generation accounting differs')
    normal.presented_world_interval(observed,c['comparison'])
    result = dict(verified=True,pair=pair,identity=a['session_identity'],profile=profile,scenario=a['scenario'],
                  provider=admitted['provider'],world_extents=admitted['world_extents'],
                  source_generations=len(resources['worlds']),world_presentations=admitted['world_presentations'],
                  pixels_compared=admitted['pixels'],resources=resources,cleanup=normal.cleanup_proof(reader,a),
                  response_progress=response_progress(a,observed,c),sources=admitted['sources'],
                  current_player_build=a['current_player_build'],current_player_plan=a['current_player_plan'])
    if a['scenario'] == 'normal-quick' and a['configuration'].get('gameplay_witness') is True:
        raw = lambda n:normal.retained_artifact(reader,a,n)
        hud,semantic,latches,region = (f(raw(n)) for f,n in (
            (hud_decode,'hud-state.bin'),(semantic_decode,'semantic-state.bin'),
            (latch_decode,'actor-latches.bin'),(region_decode,'input-admission-region.bin')))
        admission = admission_decode(raw('input-admission.bin'))
        require(admission['wire_version'] == 3 and admission['owner_thread'] == hud['owner_thread'],
                'Original gameplay admission owner/version differs')
        result['callback_binding'] = bind_callbacks(latches,region)
        result['marker_binding'] = bind_markers(a['flow']['gameplay_witness'],admission)
        paints = [observed['paints'][i][0] for i in observed['swaps']]
        result['gameplay'] = assess(a['flow']['gameplay_witness'],hud,semantic,latches,region,paints)
        result['minimap_routes'] = minimap(raw('native-pixel-producers.bin'))
        require(result['gameplay']['success'] is True,'Current original-state gameplay witness failed')
    elif a['scenario'] == 'normal-storage':
        delivery=c['cpu_delivery']
        require(delivery['aggregate_completion_reconciles'] is True and delivery['terminal_reply_matches'] is True
                and delivery['request_count'] == delivery['complete_reply_matches'] > 0,
                'Original save/load CPU delivery is unproved')
        result.update(storage_actions=c['original_menu_actions'],preferences=c['preferences_boundary'],cpu_delivery=c['cpu_delivery'])
    require(all(reader.digest(n)==h for n,h in result['sources'].items()),'Current pair source drift')
    return result


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for name in ('acquisition','comparison','report'):p.add_argument('--'+name,required=True)
    args=p.parse_args()
    require(args.report.startswith(PREFIX) and '..' not in Path(args.report).parts and not (ROOT/args.report).exists(),
            'Fresh owned immutable result required')
    result=dict(schema=1,kind='current_player_finite_execution_validation',success=False,
                whole_player_accepted=False,required_boundaries_closed=False,original_recovery_accepted=False)
    with Reader(ROOT) as reader:
        local_names=('tools/check-current-player-validation.py','tools/join-rendering-normal-acceptance.py',
                   'tools/rendering_gameplay_acceptance.py','tools/drawing_hud_state.py','tools/drawing_semantic_state.py',
                   'tools/drawing_actor_latches.py','tools/drawing_region_input.py','tools/campaign_input_observation.py',
                   'tools/rendering_build_binding.py','tools/rendering_capture.py','tools/native_cursor_journal.py',
                   'tests/test-current-player-validation.py')
        local_sources={n:reader.digest(n) for n in local_names}
        result['sources']=dict(local_sources)
        try:
            result['execution']=assess_pair(reader,args.acquisition,args.comparison)
            execution_sources=result['execution'].pop('sources')
            require(all(n not in execution_sources or execution_sources[n]==h for n,h in local_sources.items()),
                    'Admission analyzer source changed during execution')
            result['sources'].update(execution_sources)
            require(all(reader.digest(n)==h for n,h in result['sources'].items()),'Admission source changed during analysis')
            result['success']=True
        except (ValueError,KeyError,OSError) as error:result['error']=str(error)
        result['scope']='Current complete application in this exact finite executed route/profile, including independently compared native output, original input/callback effects, response/progress/resource conservation and normal retirement. Other journeys, fault combinations, natural caller/provider domains and historical root causes are not inferred.'
        result['inputs']={k:dict(record=n,sha256=reader.digest(n)) for k,n in (('acquisition',args.acquisition),('comparison',args.comparison))}
    sha=write_report(ROOT,args.report,(json.dumps(result,indent=2)+'\n').encode())
    db=sqlite3.connect(ROOT/'working/whole-player/requests.sqlite3')
    try:
        require(db.execute('SELECT state FROM requests WHERE report=?',(args.acquisition,)).fetchone()==('acquired',),
                'Launch budget predecessor is missing or already reviewed')
        db.execute('UPDATE requests SET state=?,comparison=? WHERE report=?',
                   ('compared' if result['success'] else 'failed',args.report,args.acquisition));db.commit()
    finally:db.close()
    print(json.dumps(dict(success=result['success'],error=result.get('error'),record=args.report,sha256=sha)),flush=True)
    return 0 if result['success'] else 1

if __name__=='__main__':raise SystemExit(main())
