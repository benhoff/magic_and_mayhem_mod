#!/usr/bin/env python3
"""Reinspect immutable worker/physical/source-domain joins and explicit interval end."""
import argparse
import json
from pathlib import Path
from drawing_api_session import BUILD, sha
from drawing_egl_source_domains import account

ROOT = Path(__file__).resolve().parents[1]


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--reference', type=Path, required=True)
    p.add_argument('--report', type=Path, required=True)
    a = p.parse_args()
    if a.report.exists():
        p.error('Refusing evidence overwrite')
    raw = a.reference.read_bytes()
    r = json.loads(raw)
    if not r['success'] or r['source_sha256'] != BUILD or not r['normal_original_quit_verified']:
        raise ValueError('Successful pinned immutable original reference required')
    marker = (Path(r['experiment']) / 'capture/egl-physical.done').read_bytes()
    result = account(r, marker)
    refused = {l['api_entry'] for l in r['api_source_lease_observation']['calls']
               if not l['lease'] and l['begin_result'] == 3 and l['before'][2] == 19
               and l['before'][7:10] == [800, 600, 800] and l['before'][12]}
    joined = {d['api_entry'] for d in result['domains']}
    result['live_full_source_pending_refusals'] = len(refused)
    result['live_full_source_pending_refusals_with_egl_context'] = len(refused & joined)
    names = ['tools/drawing_egl_source_domains.py', 'tools/analyze-drawing-egl-source-domains.py',
             'tools/drawing_api_session.py', 'tests/test-drawing-egl-source-domains.py']
    report = dict(schema=1, success=True, review_only=True,
                  sources={n: sha((ROOT / n).read_bytes()) for n in names},
                  reference=dict(path=str(a.reference), sha256=sha(raw)), close_marker_sha256=sha(marker),
                  analysis=result, original_game_reexecuted=False,
                  scope='Static immutable original worker/command/API/source-lease and physical interval accounting. Preserve explicit after-close contexts separately and every in-interval source refusal/exclusion; no byte ownership, hardware completion, all-route or native parity promotion.')
    with a.report.open('x') as out:
        json.dump(report, out, indent=2)
        out.write('\n')
    print(json.dumps(dict(success=True, closed=result['closed_physical_calls'],
                          outside=len(result['outside_closed_interval']), domains=result['domain_counts'],
                          pending_refusals_with_egl=result['live_full_source_pending_refusals_with_egl_context'])))


if __name__ == '__main__':
    main()
