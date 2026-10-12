#!/usr/bin/env python3
"""Revalidate bounded original cursor mutex caller order; no ownership inference."""
import argparse
from collections import Counter, defaultdict
import json
from pathlib import Path
from drawing_api_session import sha
from drawing_cursor_mutex import decode

ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--original-reference', type=Path, required=True)
    parser.add_argument('--report', type=Path, required=True)
    args = parser.parse_args()
    if args.report.exists():
        parser.error('Refusing evidence overwrite')
    reference_raw = args.original_reference.read_bytes()
    reference = json.loads(reference_raw)
    raw_path = Path(reference['experiment']) / 'capture/cursor-mutex.bin'
    raw = raw_path.read_bytes()
    if not reference.get('success') or sha(raw) != reference['cursor_mutex_sha256']:
        raise ValueError('Immutable admitted mutex reference differs')
    observation = decode(raw)
    grouped = defaultdict(list)
    constructors = []
    closes = []
    waits = Counter()
    for call in observation['calls']:
        begin, returned = call['entry'], call['returned']
        grouped[begin['thread']].extend((begin, returned))
        if begin['operation'] == 1:
            constructors.append(dict(call=call['call'], caller=begin['caller_rva'] + 0x400000, id=returned['mutex_id'], initial_owner=begin['parameter'], security_nonnull=begin['security_nonnull'], name_nonnull=begin['name_nonnull'], nonnull_result=returned['raw_result']))
        if begin['operation'] == 4 and begin['mutex_id']:
            closes.append(dict(call=call['call'], caller=begin['caller_rva'] + 0x400000, id=begin['mutex_id'], raw_result=returned['raw_result']))
        if begin['operation'] == 2:
            waits[(begin['caller_rva'] + 0x400000, begin['mutex_id'], begin['parameter'], returned['raw_result'])] += 1
    chains = []
    gaps = []
    for thread, events in grouped.items():
        events.sort(key=lambda row: row['ordinal'])
        outer = inner = released = None
        for row in events:
            caller = row['caller_rva'] + 0x400000
            if row['phase'] != 2:
                continue
            if row['operation'] == 2 and caller == 0x4a494e and row['raw_result'] == 0:
                if outer:
                    gaps.append(dict(thread=thread, reason='new outer wait before prior outer release'))
                outer, inner, released = row, None, None
            elif row['operation'] == 2 and caller == 0x4a4a8f:
                if not outer or inner:
                    gaps.append(dict(thread=thread, reason='unpaired timer inner wait'))
                inner = row
            elif row['operation'] == 3 and caller == 0x4a4c50:
                if not inner or not outer or row['mutex_id'] != inner['mutex_id']:
                    gaps.append(dict(thread=thread, reason='unpaired timer inner release'))
                released = row
            elif row['operation'] == 3 and caller == 0x4a4dda:
                if not outer or row['mutex_id'] != outer['mutex_id']:
                    gaps.append(dict(thread=thread, reason='unpaired timer outer release'))
                if inner:
                    if not released:
                        gaps.append(dict(thread=thread, reason='inner wait lacks inner release before outer release'))
                    else:
                        chains.append(dict(thread=thread, outer_wait=outer['ordinal'], inner_wait=inner['ordinal'], inner_release=released['ordinal'], outer_release=row['ordinal'], outer_mutex=outer['mutex_id'], inner_mutex=inner['mutex_id'], inner_timeout=inner['parameter'], inner_wait_result=inner['raw_result'], inner_release_result=released['raw_result'], outer_release_result=row['raw_result']))
                outer = inner = released = None
        if outer or inner or released:
            gaps.append(dict(thread=thread, reason='unfinished timer caller chain'))
    sources = {name: sha((ROOT / name).read_bytes()) for name in ('tools/analyze-drawing-cursor-mutex-order.py', 'tools/drawing_cursor_mutex.py', 'tools/drawing_input_point.py')}
    if args.original_reference.read_bytes() != reference_raw or raw_path.read_bytes() != raw:
        raise ValueError('Reference changed during analysis')
    report = dict(schema=1, success=not gaps, sources=sources, reference=dict(path=str(args.original_reference), sha256=sha(reference_raw)), raw_sha256=sha(raw), constructors=constructors, tracked_closes=closes, waits=[dict(caller=k[0], mutex_id=k[1], timeout=k[2], raw_result=k[3], count=v) for k, v in sorted(waits.items())], nested_timer_chains=chains, gaps=gaps, original_game_executed=False, real_mutex_ownership_proved=False, safe_source_synchronization_proved=False, scope='Immutable caller/order analysis of one original run. Exact tracked constructor/wait/release/close rows, including both returns in timer nested chains. No full aliases/lock order/ownership/abandoned semantics, source protection or new game/native equivalence claim.')
    with args.report.open('x') as out:
        json.dump(report, out, indent=2)
        out.write('\n')
    print(json.dumps(dict(success=not gaps, nested_chains=len(chains), gaps=len(gaps))))


if __name__ == '__main__':
    main()
