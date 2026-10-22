#!/usr/bin/env python3
"""Conservative source-call correlation inside observed cursor mutex prefixes."""
import argparse
from collections import Counter
import json
from pathlib import Path
from coverage_evidence import read_bytes, write_report
from drawing_api_session import sha
from drawing_cursor_mutex import decode
from drawing_api_session import decode as decode_api

ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--original-reference', type=Path, required=True)
    parser.add_argument('--report', type=Path, required=True)
    args = parser.parse_args()
    if args.report.exists():
        parser.error('Refusing evidence overwrite')
    reference_raw = read_bytes(args.original_reference, root=ROOT)
    reference = json.loads(reference_raw)
    if reference.get('success') is not True or not reference.get('normal_original_quit_verified'):
        raise ValueError('Reference lacks admitted normal Quit')
    capture = Path(reference['experiment']) / 'capture'
    mutex_raw = (capture / 'cursor-mutex.bin').read_bytes()
    api_raw = (capture / 'session.bin').read_bytes()
    if sha(mutex_raw) != reference['cursor_mutex_sha256'] or sha(api_raw) != reference['observation']['journal_sha256']:
        raise ValueError('Immutable journal identity differs')
    mutex = decode(mutex_raw)
    api = decode_api(api_raw)
    release_calls = [c for c in mutex['calls'] if c['entry']['operation'] == 3 and c['entry']['caller_rva'] + 0x400000 == 0x4a4c50]
    intervals = []
    excluded = []
    caller_counts = Counter()
    for call in mutex['calls']:
        begin, returned = call['entry'], call['returned']
        if begin['operation'] != 2 or begin['caller_rva'] + 0x400000 != 0x4a4a8f:
            continue
        candidates = [r for r in release_calls if r['entry']['thread'] == begin['thread'] and r['entry']['ordinal'] > returned['ordinal']]
        if not candidates:
            excluded.append(dict(call=call['call'], reason='no later same-thread inner-release entry'))
            continue
        release = min(candidates, key=lambda c: c['entry']['ordinal'])
        lower, upper = returned['point'], release['entry']['point']
        if lower is None or upper is None or not lower['flags'] & 1 or not upper['flags'] & 1 or returned['raw_result'] != 0 or release['entry']['mutex_id'] != begin['mutex_id']:
            excluded.append(dict(call=call['call'], reason='point/result/mutex identity unadmitted'))
            continue
        if upper['entry_prefix'] < lower['entry_prefix']:
            raise ValueError('Inner cursor interval prefix regressed')
        selected = [c for c in api['calls'] if c['thread'] == begin['thread'] and lower['entry_prefix'] < c['entry'] <= upper['entry_prefix']]
        caller_counts.update(c['caller'] for c in selected)
        intervals.append(dict(mutex_call=call['call'], release_call=release['call'], thread=begin['thread'], mutex_id=begin['mutex_id'], lower_entry_prefix=lower['entry_prefix'], upper_entry_prefix=upper['entry_prefix'], source_calls=[dict(entry=c['entry'], record=c['ordinal'], caller=c['caller'], method=c['method'], object=c['object'], related=c['related']) for c in selected], atomic_original_mutex_region_proved=False))
    names = ('tools/analyze-drawing-cursor-source-intervals.py', 'tools/drawing_cursor_mutex.py', 'tools/drawing_input_point.py', 'tools/drawing_api_session.py')
    sources = {name: sha((ROOT / name).read_bytes()) for name in names}
    if read_bytes(args.original_reference, root=ROOT) != reference_raw or (capture / 'cursor-mutex.bin').read_bytes() != mutex_raw or (capture / 'session.bin').read_bytes() != api_raw:
        raise ValueError('Reference changed during analysis')
    report = dict(schema=1, success=True, sources=sources, reference=dict(path=str(args.original_reference), sha256=sha(reference_raw)), mutex_sha256=sha(mutex_raw), api_sha256=sha(api_raw), intervals=intervals, exclusions=excluded, source_call_count=sum(len(x['source_calls']) for x in intervals), caller_counts=[dict(caller=k, count=v) for k, v in sorted(caller_counts.items())], original_game_executed=False, source_overlap_proved=False, race_repaired=False, safe_source_synchronization_proved=False, scope='Immutable conservative correlation of same-thread API entries between sampled points after inner Wait return and before Release entry. Points precede row publication and do not prove an atomic critical region; unknown aliases/source ownership and intermittent refusal remain open. No new engine execution or synchronization/native parity change.')
    write_report(ROOT,args.report,(json.dumps(report,indent=2)+'\n').encode())
    print(json.dumps(dict(success=True, intervals=len(intervals), source_calls=report['source_call_count'], exclusions=len(excluded))))


if __name__ == '__main__':
    main()
