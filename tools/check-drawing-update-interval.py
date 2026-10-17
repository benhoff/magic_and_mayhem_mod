#!/usr/bin/env python3
"""Admit the full120-update presented interval only when every ownership gate closes."""
import argparse
import json
import importlib.util
from pathlib import Path
from drawing_api_session import sha,inspect
from drawing_api_physical import analyze as physical
from drawing_update_routes import analyze as routes
from drawing_update_timing import decode as timing
from drawing_source_callback_windows import analyze as source_windows
from drawing_producer_updates import analyze as queues
from drawing_producer_points import inspect_points
from drawing_update_accounting import build


def validate_accounting(value):
    if not all(value.get(k) is True for k in ('success','analysis_completed','sources_stable','raw_observations_match_original_report','acquisition_observers_match_captured_sources')):
        raise ValueError('Successful current raw accounting required')
    if value.get('requested_windows')!=120 or len(value.get('windows',[]))!=120:
        raise ValueError('The120accepted-update denominator cannot change')
    gates=value.get('gates')
    names={'consecutive_advancing_windows','exact_source_callback_prefixes','all_admitted_call_intervals_accounted',
           'selected_world_branches_accounted','source_callback_boundaries_quiescent','every_primary_witness_has_live_source_points',
           'every_primary_witness_source_worker_egl_bound','all_linked_reference_regions_and_whole_images_match',
           'global_presentation_routes_closed','all_original_backing_alias_writers_closed'}
    if not isinstance(gates,dict) or set(gates)!=names or any(type(v) is not bool for v in gates.values()):raise ValueError('Exact boolean admission gates required')
    if value.get('complete_presented_interval') is not all(gates.values()):raise ValueError('Presented interval claim contradicts its gates')
    previous=None;end=None;counter=None
    for w in value['windows']:
        if previous is not None and ((w['sequence']-previous)&0xffffffff!=1 or w['begin_qpc']!=end or w['counter_before']!=counter):raise ValueError('Accepted sequence/counter/QPC continuity')
        if not 0<w['begin_qpc']<w['end_qpc']<2**64:raise ValueError('Accepted interval time bounds')
        a,b=w['source_begin'],w['source_end']
        if a['sequence']!=w['sequence'] or (b['sequence']-a['sequence'])&0xffffffff!=1 or b['producer_prefix']-a['producer_prefix']!=w['producer_records'] or b['queue_prefix']-a['queue_prefix']!=len(w['source_queue_ids']):raise ValueError('Source-prefix/queue boundaries')
        previous=w['sequence'];end=w['end_qpc'];counter=(w['counter_before']+1)&0xffffffff
    if value['first_sequence']!=value['windows'][0]['sequence'] or value['last_sequence']!=previous:raise ValueError('Declared interval differs from windows')
    if value['source_queues']!=sum(len(w['source_queue_ids']) for w in value['windows']):raise ValueError('Queue-free updates removed from denominator')
    pending=any(w['source_begin']['pending_frames'] or w['source_end']['pending_frames'] for w in value['windows'])
    if gates['source_callback_boundaries_quiescent'] is pending:raise ValueError('Pending source frames contradict quiescence claim')
    if gates['selected_world_branches_accounted'] is not all(w['every_selected_branch_accounted'] for w in value['windows']):raise ValueError('Unresolved branch contradicts admission claim')
    if gates['every_primary_witness_has_live_source_points'] is bool(value['source_point_gaps']):raise ValueError('Source-point gaps contradict admission claim')
    if gates['every_primary_witness_source_worker_egl_bound'] is bool(value['missing_primary_egl_links']):raise ValueError('Missing physical links contradict admission claim')
    if gates['all_linked_reference_regions_and_whole_images_match'] is bool(value['reference_pixel_gaps']):raise ValueError('Pixel mismatch contradicts admission claim')
    # The current acquisition declares these proof boundaries open. No option
    # or self-reported boolean can manufacture the missing original proof.
    if gates['global_presentation_routes_closed'] or gates['all_original_backing_alias_writers_closed']:raise ValueError('Unsupported global route/backing ownership promotion')
    entries={c['entry'] for c in value['original_call_intervals']}
    if len(entries)!=value['original_calls'] or entries!={e for w in value['windows'] for e in w['original_call_entries']}:raise ValueError('Original admitted calls disappeared')
    if value['physical_primary_witnesses']!=sum(len(w['physical_api_frames']) for w in value['windows']) or value['egl_witnesses']!=sum(len(w['egl_witnesses']) for w in value['windows']):raise ValueError('Physical witness denominator changed')
    failed=[name for name in sorted(gates) if not gates[name]]
    return dict(schema=1,success=True,analysis_completed=True,admitted=not failed,requested_windows=120,failed_gates=failed,
                first_sequence=value['first_sequence'],last_sequence=value['last_sequence'],source_queues=value['source_queues'],
                queue_free_updates=value['queue_free_updates'],pending_source_boundary_updates=value['pending_source_boundary_updates'],
                source_point_gaps=value['source_point_gaps'],missing_primary_egl_links=value['missing_primary_egl_links'],reference_pixel_gaps=value['reference_pixel_gaps'],
                complete_presented_interval=not failed,complete_presented_journey=False,native_parity_performed=False)


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for name in ('analysis','claims','report'):p.add_argument('--'+name,type=Path,required=True)
    a=p.parse_args()
    if a.report.exists():p.error('Refusing evidence overwrite')
    root=Path(__file__).resolve().parents[1];declaration=json.loads(a.claims.read_bytes());sources=declaration['sources']
    stable=lambda:all(sha((root/name).read_bytes())==digest for name,digest in sources.items())
    if not stable():raise ValueError('Prospective gate sources changed')
    raw=a.analysis.read_bytes();value=json.loads(raw)
    if any(sha((root/name).read_bytes())!=digest for name,digest in value['sources'].items()):raise ValueError('Accounting source fingerprint changed')
    for key in ('original_report','egl_report'):
        if sha(Path(value[key]).read_bytes())!=value[key+'_sha256']:raise ValueError('Accounting input report changed')
    original=json.loads(Path(value['original_report']).read_bytes());egl=json.loads(Path(value['egl_report']).read_bytes())
    capture=Path(original['experiment'])/'capture';peer=capture.parent/'physical-report.json'
    if sha(peer.read_bytes())!=original['physical_peer_report_sha256']:raise ValueError('Physical peer changed')
    observed={'observation':inspect(capture),'physical_api_observation':physical(capture,peer),
              'update_route_observation':routes(capture/'engine-cadence.bin'),
              'update_call_timing_observation':timing((capture/'update-call-timing.bin').read_bytes(),(capture/'session.bin').read_bytes()),
              'source_update_window_observation':source_windows(capture),'producer_update_observation':queues(capture),
              'producer_observation':inspect_points(capture)}
    for key,data in observed.items():
        if data!=original[key]:raise ValueError('Raw interval observation changed: '+key)
    spec=importlib.util.spec_from_file_location('interval_producers',root/'tools/inspect-canvas-producers.py');module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
    _,records=module.decode((capture/'canvas-producers.bin').read_bytes())
    expected=build(observed['update_route_observation'],observed['update_call_timing_observation'],observed['source_update_window_observation'],
                   observed['producer_update_observation'],records,observed['physical_api_observation']['frames'],egl,observed['producer_observation']['frames'])
    if any(value.get(key)!=item for key,item in expected.items()):raise ValueError('Accounting differs from independently rejoined raw inputs')
    result=validate_accounting(value)
    result.update(analysis=str(a.analysis),analysis_sha256=sha(raw),sources=sources,sources_stable=stable(),claims=declaration['claims'],original_game_executed=False,
                  scope='Static admission of the retained120-update accounting result. Every source/frame/route ownership gate is required; counter or queue counts cannot override pending boundaries/straddlers/global routes/backing writers. A valid rejected result is preserved, never promoted to complete reference/native parity.')
    if not result['sources_stable']:raise ValueError('Gate sources changed during execution')
    with a.report.open('x') as out:json.dump(result,out,indent=2);out.write('\n')
    print(json.dumps(dict(admitted=result['admitted'],failed_gates=result['failed_gates'])))
    return 0 if result['admitted'] else 2


if __name__=='__main__':raise SystemExit(main())
