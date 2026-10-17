#!/usr/bin/env python3
"""Hash-bound negative original source refusal and same-run cursor mutex calls."""
import argparse
import json
from pathlib import Path
import struct
from drawing_api_session import sha
from drawing_cursor_mutex import decode as decode_mutex
from drawing_source_pending_refusal import decode as decode_pending

ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--negative-reference', type=Path, required=True)
    parser.add_argument('--report', type=Path, required=True)
    args = parser.parse_args()
    if args.report.exists():
        parser.error('Refusing evidence overwrite')
    reference_raw = args.negative_reference.read_bytes()
    reference = json.loads(reference_raw)
    if reference.get('success') is not False or reference.get('admitted') is not False:
        raise ValueError('Explicit negative reference required')
    capture = Path(reference['experiment']) / 'capture'
    refused = reference.get('source_pending_frame_refusals', [])
    if len(refused) != 1:
        raise ValueError('Expected one retained bounded refusal')
    pending_raw = (capture / refused[0]['path']).read_bytes()
    pending = decode_pending(pending_raw)
    if pending['journal_sha256'] != refused[0]['journal_sha256']:
        raise ValueError('Immutable pending refusal differs')
    mutex_raw = (capture / 'cursor-mutex.bin').read_bytes()
    mutex = decode_mutex(mutex_raw)
    api_raw = (capture / 'session.bin').read_bytes()
    if len(api_raw) < 64 or (len(api_raw) - 64) % 128:
        raise ValueError('Malformed partial API journal')
    rows = list(struct.iter_unpack('<32I', api_raw[64:]))
    if (capture / 'session.done').exists():
        raise ValueError('Negative missing-completion-marker boundary differs')
    terminals = [r for r in rows if r[1] == 4]
    if len(terminals) > 1 or terminals and rows[-1] != terminals[0]:
        raise ValueError('Malformed negative API terminal placement')
    foreign = [f for f in pending['frames'] if f['owner'] != pending['thread'] and f['tag'] == 32 and f['original_caller_rva'] + 0x400000 == 0x4a4b01]
    if len(foreign) != 1:
        raise ValueError('Exact foreign cursor caller missing')
    frame = foreign[0]
    waits = [c for c in mutex['calls'] if c['entry']['thread'] == frame['owner'] and c['entry']['operation'] == 2 and c['entry']['caller_rva'] + 0x400000 == 0x4a4a8f and c['returned']['raw_result'] == 0 and (pending['tick'] - c['returned']['tick']) % 2**32 < 2**31]
    if not waits:
        raise ValueError('Same-run successful timer inner wait missing')
    wait = max(waits, key=lambda c: c['returned']['ordinal'])
    subsequent_release = [c for c in mutex['calls'] if c['entry']['thread'] == frame['owner'] and c['entry']['operation'] == 3 and c['entry']['mutex_id'] == wait['entry']['mutex_id'] and c['entry']['ordinal'] > wait['returned']['ordinal']]
    point = wait['returned']['point']
    if point is None or point['record_prefix'] > len(rows) or point['entry_prefix'] < max((r[29] for r in rows[:point['record_prefix']]), default=0):
        raise ValueError('Timer wait point exceeds partial API prefix')
    cursor = next((r for r in rows if r[0] == point['cursor_record']), None)
    if cursor is None or cursor[1:3] != (2, 3) or cursor[6] != frame['owner'] or (cursor[29], cursor[11], list(cursor[27:29])) != (point['cursor_entry'], point['cursor_tick'], point['cursor']):
        raise ValueError('Exact timer cursor partial-record identity differs')
    names = ('tools/analyze-drawing-cursor-refusal.py', 'tools/drawing_cursor_mutex.py', 'tools/drawing_input_point.py', 'tools/drawing_source_pending_refusal.py')
    sources = {n: sha((ROOT / n).read_bytes()) for n in names}
    if args.negative_reference.read_bytes() != reference_raw or (capture / refused[0]['path']).read_bytes() != pending_raw or (capture / 'cursor-mutex.bin').read_bytes() != mutex_raw or (capture / 'session.bin').read_bytes() != api_raw:
        raise ValueError('Negative reference changed')
    report = dict(schema=1, success=True, sources=sources, negative_reference=dict(path=str(args.negative_reference), sha256=sha(reference_raw)), journal_sha256=dict(pending=sha(pending_raw), mutex=sha(mutex_raw), partial_api=sha(api_raw)), pending=pending, foreign_frame=frame, latest_timer_inner_wait=wait, wait_to_refusal_ms=(pending['tick'] - wait['returned']['tick']) % 2**32, subsequent_matching_release_count=len(subsequent_release), mutex_call_pairs=len(mutex['calls']), remaining_live_mutex_ids=mutex['remaining_live_ids'], partial_api_records=len(rows), api_terminal_row_present=bool(terminals), api_completion_marker_present=False, original_game_executed=False, normal_original_quit_verified=False, complete_source_capture_admitted=False, source_overlap_proved=False, race_repaired=False, safe_source_synchronization_proved=False, scope='Immutable negative queue/lease/source/foreign-cursor identity and same-run latest zero-result inner Wait with exact partial cursor/API point. Flushed mutex calls are paired, but live IDs remain; final API terminal row is retained separately from its missing completion marker after diagnostic termination. No physical overlap, original mutex ownership, repaired race, added source locking or complete comparison/native parity claim.')
    with args.report.open('x') as out:
        json.dump(report, out, indent=2)
        out.write('\n')
    print(json.dumps(dict(success=True, queue=pending['queue'], lease=pending['lease'], mutex_pairs=len(mutex['calls']), wait_to_refusal_ms=report['wait_to_refusal_ms'])))


if __name__ == '__main__':
    main()
