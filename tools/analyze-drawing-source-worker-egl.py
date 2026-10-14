#!/usr/bin/env python3
"""Revalidate raw source/command/worker/EGL records and compare physical regions."""
import argparse
import json
import struct
from pathlib import Path
from drawing_api_session import BUILD, sha, inspect
from drawing_source_worker_egl import bind, delivery_boundary
from drawing_source_frame_binding import evaluate
from drawing_api_physical import analyze as analyze_api_physical
from drawing_wine_source_map import analyze as maps
from drawing_wine_submission import decode as submissions
from drawing_wine_blit_worker import decode as workers
from drawing_wine_blit_packet import decode as packets
from drawing_worker_context import decode as contexts
from drawing_egl_physical import analyze_egl_physical

ROOT = Path(__file__).resolve().parents[1]


def main():
    p = argparse.ArgumentParser(description=__doc__)
    for name in ('original', 'claims', 'report'):
        p.add_argument('--'+name, type=Path, required=True)
    a = p.parse_args()
    if a.report.exists():
        p.error('Refusing evidence overwrite')
    claim = json.loads(a.claims.read_text())
    sources = claim['sources']
    def verify_sources():
        if any(sha((ROOT/n).read_bytes()) != h for n, h in sources.items()):
            raise ValueError('Prospective analysis sources changed')
    verify_sources()
    raw = a.original.read_bytes()
    original = json.loads(raw)
    if original.get('source_sha256') != BUILD or not all(original.get(k) is True for k in ('success', 'sources_stable', 'original_drawing_active', 'original_manifest_verified_before_after', 'normal_original_quit_verified', 'original_sections_unchanged')):
        raise ValueError('Completed original run with unchanged originals required')
    root = Path(original['experiment'])
    capture = root/'capture'
    api_observation = inspect(capture)
    if api_observation != original['observation']:
        raise ValueError('Raw API caller/results observation changed')
    life = (capture/'wine-lifetime.bin').read_bytes()
    api_rows = list(struct.iter_unpack('<32I', (capture/'session.bin').read_bytes()[64:-128]))
    api = {r[29]: r for r in api_rows}
    if len(api) != len(api_rows):
        raise ValueError('Ambiguous API entry')
    sub_raw = (capture/'wine-submission.bin').read_bytes()
    sub = submissions(sub_raw, api, life)
    worker = workers((capture/'wine-blit-worker.bin').read_bytes(), life)
    packet = packets((capture/'wine-blit-packet.bin').read_bytes(), {c['call']: c for c in sub['calls']}, {c['call']: c for c in worker['calls']})
    source = maps(capture)
    context = contexts((capture/'egl-worker-context.bin').read_bytes())
    peer = root/'egl-physical-report.json'
    if sha(peer.read_bytes()) != original['egl_physical_peer_report_sha256']:
        raise ValueError('EGL physical peer report changed')
    physical = analyze_egl_physical(capture, peer)
    for name, value in [('wine_submission_observation', sub), ('wine_blit_worker_observation', worker), ('wine_blit_packet_observation', packet), ('wine_source_map_observation', source), ('egl_worker_context_observation', context), ('egl_physical_observation', physical)]:
        if value != original[name]:
            raise ValueError('Raw observation changed: '+name)
    if not packet['complete_packet_binding'] or packet['unbound_submissions']:
        raise ValueError('Closed selected packet bindings required')
    # The portable submission decoder intentionally omits these scalar fields;
    # recover them from the already validated original entry rows for this join.
    extras = {r[2]: r for r in struct.iter_unpack('<48I', sub_raw[64:]) if r[1] == 1}
    calls = [dict(c, destination_subresource=extras[c['call']][12], source_subresource=extras[c['call']][13]) for c in sub['calls']]
    result = bind(calls, source['samples'], source['payloads'], packet['bindings'], worker['calls'], context['calls'], physical['frames'], capture, api_observation['calls'])
    api_peer = root/'physical-report.json'
    if sha(api_peer.read_bytes()) != original['physical_peer_report_sha256']:
        raise ValueError('API physical peer report changed')
    api_physical = analyze_api_physical(capture, api_peer)
    if api_physical != original['physical_api_observation']:
        raise ValueError('Raw API physical observation changed')
    api_frames = {f['entry']: f for f in api_physical['frames']}
    api_timings = {f['entry']: f for f in json.loads(api_peer.read_bytes())['frames']}
    call_by = {c['call']: c for c in calls}
    sample_by = {s['submission']: s for s in source['samples']}
    context_by = {(c['pid'], c['call']): c for c in context['calls']}
    boundaries, boundary_refusals = [], []
    for comparison in result['comparisons']:
        frame = api_frames.get(comparison['frame_entry'])
        if frame is None:
            boundary_refusals.append(dict(egl_call=comparison['egl_call'], reason='no_api_physical_witness'))
            continue
        sid = comparison['submission']
        api_result = evaluate([call_by[sid]], [sample_by[sid]], source['payloads'], [frame], capture)
        if api_result['refusals']:
            boundary_refusals.append(dict(egl_call=comparison['egl_call'], reason=api_result['refusals'][0]['reason']))
            continue
        boundaries.append(delivery_boundary(api_result['comparisons'][0], comparison, api_timings[frame['entry']], context_by[comparison['egl_pid'], comparison['egl_call']]))
    result.update(delivery_boundary_observations=boundaries, delivery_boundary_refusals=boundary_refusals,
                  api_mismatches_before_matching_egl=sum(b['api_mismatch_before_matching_egl'] for b in boundaries))
    verify_sources()
    result.update(sources=sources, sources_stable=True, claims=claim['claims'], original_report=str(a.original), original_report_sha256=sha(raw), original_game_executed=False,
                  scope='Reference-only exact observed command/source/worker/context association and post-EGL physical copy-region agreement. Every refusal and mismatch retained; no original-source atomicity, all alias writers/routes, GPU completion, scanout or native parity.')
    a.report.write_text(json.dumps(result, indent=2)+'\n')
    print(json.dumps({k: result[k] for k in ('source_samples', 'all_observed_sources_worker_bound', 'physical_witnesses', 'matched_regions', 'mismatched_regions', 'all_physical_copy_regions_bound')}))


if __name__ == '__main__':
    main()
