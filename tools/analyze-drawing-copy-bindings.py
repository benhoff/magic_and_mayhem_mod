#!/usr/bin/env python3
"""Retain exact bounded copy request/API/worker associations, without replay changes."""
import argparse
import json
from pathlib import Path
from coverage_evidence import read_bytes, write_report
import struct
from drawing_api_session import sha
from drawing_copy_order import decode
from drawing_copy_bindings import associate
from drawing_producer_points import inspect_points
from drawing_wine_blit_packet import decode as decode_packets

ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--reference', type=Path, required=True)
    parser.add_argument('--report', type=Path, required=True)
    args = parser.parse_args()
    if args.report.exists():
        parser.error('Refusing evidence overwrite')
    raw = read_bytes(args.reference, root=ROOT)
    reference = json.loads(raw)
    if not reference['success'] or not reference['normal_original_quit_verified']:
        raise ValueError('Completed original capture required')
    capture = Path(reference['experiment']) / 'capture'
    request = decode((capture / 'copy-order-0000.bin').read_bytes())
    if request != reference['copy_order_observation'] or inspect_points(capture) != reference['producer_observation']:
        raise ValueError('Recorded request/source points changed')
    calls = {r[29]: r for r in struct.iter_unpack('<32I', (capture / 'session.bin').read_bytes()[64:-128]) if r[1] == 1}
    points = {}
    for row in struct.iter_unpack('<48I', (capture / 'api-producer-points.bin').read_bytes()[64:-192]):
        phases = points.setdefault(row[2], {})
        if row[1] in phases:
            raise ValueError('Duplicate API point phase')
        phases[row[1]] = row
    # Lifetime-bound submission and worker decoding remains in the original
    # capture report. Pin the raw journals before checking packet cross-links.
    for name, key in [('wine-submission.bin', 'wine_submission_observation'),
                      ('wine-blit-worker.bin', 'wine_blit_worker_observation')]:
        if sha((capture / name).read_bytes()) != reference[key]['journal_sha256']:
            raise ValueError('Recorded command journal changed')
    submissions = reference['wine_submission_observation']['calls']
    workers = reference['wine_blit_worker_observation']['calls']
    packets = decode_packets((capture / 'wine-blit-packet.bin').read_bytes(),
                             {s['call']: s for s in submissions}, {w['call']: w for w in workers})
    if packets != reference['wine_blit_packet_observation'] or not packets['complete_packet_binding']:
        raise ValueError('Incomplete or changed packet bindings')
    result = associate(request, calls, points, submissions, packets['bindings'])
    result.update(schema=1, success=True, review_only=True,
                  reference=dict(path=str(args.reference), sha256=sha(raw)),
                  sources={p: sha((ROOT / p).read_bytes()) for p in
                           ['tools/analyze-drawing-copy-bindings.py', 'tools/drawing_copy_bindings.py',
                            'tools/drawing_copy_order.py', 'tools/drawing_producer_points.py',
                            'tools/drawing_api_session.py', 'tools/drawing_wine_blit_packet.py']},
                  original_game_reexecuted=False, native_replay_changed=False,
                  scope='Static same-owner/geometry/key/logical-source-and-destination association within explicit request entry/return producer intervals at all three API phases; exact submission/packet/worker IDs. Unbound calls remain outside the admitted source scope. No general replay-order, view/alias/lifetime, native parity or fresh original execution claim.')
    write_report(ROOT,args.report,(json.dumps(result,indent=2)+'\n').encode())
    print(json.dumps(dict(success=True, bound=len(result['bindings']),
                          unbound_requests=len(result['unbound_requests']),
                          excluded_api_entries=len(result['excluded_api_entries']),
                          reversed_returns=len(result['adjacent_return_reversals']))))


if __name__ == '__main__':
    main()
