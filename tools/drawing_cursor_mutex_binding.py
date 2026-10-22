"""Bind sampled mutex prefixes to exact immutable API/cursor journal records."""
import struct
from bisect import bisect_right
from drawing_api_session import decode as decode_api
from drawing_cursor_mutex import decode as decode_mutex


def analyze(mutex_raw, api_raw):
    observation = decode_mutex(mutex_raw)
    api = decode_api(api_raw)
    rows = list(struct.iter_unpack('<32I', api_raw[64:]))
    cursors = {}
    prefixes = [(0, 0)]
    completed_entry = 0
    maximum_entry = max((r[29] for r in rows[:-1]), default=0)
    for row in rows:
        completed_entry = max(completed_entry, row[29])
        if row[1] == 2 and row[2] == 3:
            cursors.setdefault(row[6], []).append((row[0], dict(record=row[0], entry=row[29], tick=row[11], valid=bool(row[31]), xy=list(row[27:29]) if row[31] else None)))
        prefixes.append((completed_entry, row[30] if row[1] != 4 else row[22]))
    cursor_records = {owner: [r[0] for r in records] for owner, records in cursors.items()}
    linked = []
    inactive = refused = 0
    for call in observation['calls']:
        result = dict(call=call['call'], points=[])
        for sample in (call['entry'], call['returned']):
            point = sample['point']
            if point is None:
                refused += 1
                result['points'].append(dict(phase=sample['phase'], state='query_refused'))
                continue
            index = point['record_prefix']
            if index >= len(prefixes):raise ValueError('Mutex point prefix outside retained journal')
            prefix = prefixes[index]
            if point['entry_prefix'] < prefix[0] or point['after_frame'] != prefix[1] or point['drops'] or point['gaps']:
                raise ValueError('Mutex point prefix/loss identity differs')
            # Entry count can lead completed records while another call is
            # pending; never require completion order to equal entry order.
            if point['entry_prefix'] > maximum_entry:
                raise ValueError('Mutex point entry beyond retained journal')
            owner_records = cursors.get(sample['thread'], [])
            at = bisect_right(cursor_records.get(sample['thread'], []), index) - 1
            cursor = owner_records[at][1] if at >= 0 else None
            active = bool(point['flags'] & 1)
            if active == (index == len(rows)) or not active and (point['pending'] or point['entry_prefix'] != maximum_entry):
                raise ValueError('Mutex point active/terminal identity differs')
            if cursor is None:
                if point['cursor_record'] or point['cursor_entry'] or point['cursor_tick'] or point['cursor'] is not None:
                    raise ValueError('Unobserved mutex cursor identity')
            elif (point['cursor_record'], point['cursor_entry'], point['cursor_tick'], point['cursor']) != (cursor['record'], cursor['entry'], cursor['tick'], cursor['xy']):
                raise ValueError('Mutex cursor record/entry/tick/value differs')
            active = bool(point['flags'] & 1)
            if not active:
                inactive += 1
            result['points'].append(dict(phase=sample['phase'], state='active' if active else 'inactive', record_prefix=point['record_prefix'], entry_prefix=point['entry_prefix'], cursor_record=point['cursor_record'], pending=point['pending']))
        linked.append(result)
    return dict(schema=1, success=True, calls=len(linked), sampled_points=len(linked) * 2, query_refusals=refused, inactive_points=inactive, links=linked, api_records=api['records'], real_mutex_ownership_proved=False, safe_source_synchronization_proved=False, scope='Exact sampled API/cursor prefix identities, including pending entries and inactive/query-refused points. Query sampling precedes observer event publication; no atomic call-entry/return or ownership/source-lock claim.')
