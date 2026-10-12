"""Validate observer call ordering and expose competing calls at physical fences.

No interval proves producer ownership, driver ordering or scanout completeness.
COM identities are local generations; distinct aliases may share backing storage.
"""
import bisect
import struct
from pathlib import Path
from drawing_api_session import decode, sha

# Conservative calls capable of changing storage, lifetime or presentation.
# Lock/GetDC open a CPU/DC lease until a successful Unlock/ReleaseDC.
MUTATORS = {2, 3, 4, 5, 6, 7, 9, 10, 11, 17, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35}


def decode_intervals(data, journal):
    observed = decode(journal)
    if not observed['physical_fence_enabled']:
        raise ValueError('Call intervals require the physical API mode')
    if len(data) < 128 or len(data) > 64 + 786433*64 or len(data) % 64:
        raise ValueError('Incomplete call interval envelope')
    header = struct.unpack_from('<16I', data)
    if data[:8] != b'MNMDINT1' or header[2:6] != (1, 64, 0x40209ca7, 786433) or any(header[6:]):
        raise ValueError('Unsupported call interval header')
    calls = {r[29]: r for r in struct.iter_unpack('<32I', journal[64:-128])}
    rows = list(struct.iter_unpack('<16I', data[64:]))
    if rows[-1][1] != 4 or any(r[1] == 4 for r in rows[:-1]):
        raise ValueError('Missing/early call interval terminal')
    active, completed, entered = {}, {}, {}
    leases, lease_gaps = {}, []
    lease_events = []
    max_pending = 0
    for ordinal, r in enumerate(rows[:-1], 1):
        if r[0] != ordinal or r[1] not in (1, 2, 3) or r[14] or r[15] or r[2] not in calls:
            raise ValueError('Invalid call interval identity')
        call = calls[r[2]]
        if r[3:6] != call[1:4] or r[7:9] != (call[6], call[5]):
            raise ValueError('Call interval differs from API identity')
        if r[1] == 1:
            if r[2] in entered or any(r[10:13]):
                raise ValueError('Duplicate/invalid call entry')
            if call[1] == 1 and call[2] in (5, 7, 11) and r[6] != call[4]:
                raise ValueError('Call source generation changed')
            active[r[2]] = dict(entry=r[2], enter=ordinal, identity=r[2:9])
            entered[r[2]] = active[r[2]]
        else:
            entry = active.get(r[2])
            if entry is None or entry['identity'] != r[2:9] or r[10] != call[8]:
                raise ValueError('Unpaired or changed call return/completion')
            if r[1] == 2:
                if 'returned' in entry or r[11] or r[12]:
                    raise ValueError('Duplicate/invalid call return')
                entry['returned'] = ordinal
                if call[1] == 1 and not call[8] & 0x80000000:
                    kind = {17: 'dc', 25: 'cpu', 26: 'dc', 32: 'cpu'}.get(call[2])
                    if kind:
                        key = (call[3], kind)
                        if call[2] in (17, 25):
                            if key in leases: lease_gaps.append(dict(event=ordinal, reason='duplicate-acquire', object=key[0], kind=kind))
                            leases[key] = r[2]
                        elif key in leases:
                            del leases[key]
                        else:
                            lease_gaps.append(dict(event=ordinal, reason='unobserved-acquire', object=key[0], kind=kind))
                        lease_events.append(dict(event=ordinal, object=key[0], kind=kind, open=call[2] in (17, 25), entry=r[2]))
            else:
                if 'returned' not in entry or r[11:13] != (call[12], call[0]):
                    raise ValueError('Completion lacks return or exact journal/frame binding')
                entry['completed'] = ordinal
                completed[r[2]] = entry
        if r[13] != len(active):
            raise ValueError('Call interval pending ownership changed')
        max_pending = max(max_pending, len(active))
        if r[1] == 3: del active[r[2]]
    end = rows[-1]
    terminal = struct.unpack_from('<32I', journal, len(journal)-128)
    if (end[0] != len(rows) or end[1] != 4 or end[2] != terminal[2] or any(end[3:7]) or not end[7] or end[8] or
            end[10:16] != (terminal[0], terminal[22], len(calls), 0, 0, 0) or active or set(completed) != set(calls)):
        raise ValueError('Incomplete call interval terminal accounting')
    mutations = sorted((e for key, e in completed.items() if calls[key][1] == 1 and calls[key][2] in MUTATORS), key=lambda e: e['enter'])
    starts = [e['enter'] for e in mutations]
    frames = []
    for frame in observed['frames']:
        call = next(r for r in calls.values() if r[0] == frame['record'])
        entry = completed[call[29]]
        competitors = [e for e in mutations[:bisect.bisect_left(starts, entry['completed'])]
                       if e['entry'] != entry['entry'] and e['returned'] > entry['enter']]
        # Retain a bounded diagnostic listing with an explicit full count. No
        # truncation is admitted as complete source/frame proof.
        frames.append(dict(ordinal=frame['ordinal'], entry=entry['entry'], thread=call[6],
                           enter_event=entry['enter'], return_event=entry['returned'], ack_event=entry['completed'],
                           competing_mutation_count=len(competitors),
                           competing_entries=[e['entry'] for e in competitors[:256]],
                           competitor_listing_complete=len(competitors) <= 256,
                           other_thread_mutation_count=sum(calls[e['entry']][6] != call[6] for e in competitors)))
    return dict(success=True, schema=1, boundary='observer_call_intervals', journal_sha256=sha(journal),
                interval_sha256=sha(data), events=len(rows), calls=len(calls), max_pending=max_pending, frames=frames,
                frames_with_competing_mutations=sum(f['competing_mutation_count'] > 0 for f in frames),
                frames_with_other_thread_mutations=sum(f['other_thread_mutation_count'] > 0 for f in frames),
                lease_events=lease_events, lease_gaps=lease_gaps,
                remaining_leases=[dict(object=k[0], kind=k[1], entry=v) for k, v in sorted(leases.items())],
                producer_inputs_owned=False, aliases_resolved=False, all_global_presentation_routes_observed=False,
                complete_presented_journey=False)


def inspect_intervals(directory):
    directory = Path(directory)
    return decode_intervals((directory/'api-call-intervals.bin').read_bytes(), (directory/'session.bin').read_bytes())
