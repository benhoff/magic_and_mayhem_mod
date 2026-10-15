"""Temporal QueryInterface aliases and CPU/DC leases in admitted call intervals.

This is read-only diagnostic analysis. It never treats an attached surface as an
alias, infers CPU writes from pixels, or proves all backing-storage aliases.
"""
import struct
from drawing_call_intervals import decode_intervals


def analyze_lifetimes(intervals, journal):
    ordering = decode_intervals(intervals, journal)
    calls = {r[29]: r for r in struct.iter_unpack('<32I', journal[64:-128])}
    events = list(struct.iter_unpack('<16I', intervals[64:-64]))
    parent, active, leases = {}, {}, []
    query_links, gaps, retirements, windows = [], [], [], {}
    open_windows, relevant = {}, {}
    frame_entries = {f['entry']: f['ordinal'] for f in ordering['frames']}

    def root(identity):
        parent.setdefault(identity, identity)
        owner, path = identity, []
        while parent[owner] != owner:
            path.append(owner); owner = parent[owner]
        for item in path: parent[item] = owner
        return owner

    def snapshot():
        return sorted(lease['entry'] for lease in active.values())

    def relevant_snapshot(call):
        owners = {root(identity) for identity in call[3:5] if identity}
        return {lease['entry'] for (identity, _), lease in active.items() if root(identity) in owners}

    def merge(a, b, event, entry):
        left, right = root(a), root(b)
        query_links.append(dict(event=event, entry=entry, source=a, alias=b))
        if left == right: return
        keep, drop = min(left, right), max(left, right)
        parent[drop] = keep
        for key in list(active):
            if key[0] != drop: continue
            lease = active.pop(key)
            new_key = (keep, key[1])
            if new_key in active:
                # Both observed acquisitions remain visible; ambiguity is a
                # gap, never resolved by discarding an earlier lease.
                gaps.append(dict(event=event, reason='alias-merge-with-two-leases', entries=[active[new_key]['entry'], lease['entry']]))
                lease['ambiguous_end'] = event
            else: active[new_key] = lease

    for event in events:
        ordinal, phase, entry = event[:3]
        call = calls[entry]
        if entry in frame_entries:
            if phase == 1:
                windows[entry] = dict(ordinal=frame_entries[entry], entry=entry, enter_event=ordinal,
                                      open_lease_entries_at_entry=snapshot())
                open_windows[entry] = call
                relevant[entry] = relevant_snapshot(call)
            elif phase == 3:
                windows[entry]['ack_event'] = ordinal
                windows[entry]['open_lease_entries_at_ack'] = snapshot()
                relevant[entry].update(relevant_snapshot(call))
                del open_windows[entry]
        for owner, target in open_windows.items(): relevant[owner].update(relevant_snapshot(target))
        if phase != 2: continue
        successful = not call[8] & 0x80000000
        if call[1] in (1, 5) and call[2] == 0 and successful and call[3] and call[4]:
            merge(call[3], call[4], ordinal, entry)
        if call[1] != 1: continue
        if call[2] == 2 and call[8] == 0:
            owner = root(call[3])
            owned = [lease['entry'] for (identity, _), lease in active.items() if identity == owner]
            retirements.append(dict(event=ordinal, entry=entry, interface=call[3], open_lease_entries=owned))
            if owned: gaps.append(dict(event=ordinal, reason='zero-reference-release-with-open-lease', entries=owned))
        kind = {17: 'dc', 25: 'cpu', 26: 'dc', 32: 'cpu'}.get(call[2])
        if not successful or not kind: continue
        key = (root(call[3]), kind)
        if call[2] in (17, 25):
            if key in active:
                gaps.append(dict(event=ordinal, reason='duplicate-acquire', entries=[active[key]['entry'], entry]))
                active[key]['ambiguous_end'] = ordinal
            lease = dict(entry=entry, interface=call[3], kind=kind, acquire_event=ordinal, release_event=None)
            leases.append(lease); active[key] = lease
        elif key not in active:
            gaps.append(dict(event=ordinal, reason='unobserved-acquire', entry=entry, interface=call[3], kind=kind))
        else:
            lease = active.pop(key)
            lease.update(release_event=ordinal, release_entry=entry, release_interface=call[3])
        for owner, target in open_windows.items(): relevant[owner].update(relevant_snapshot(target))
    frames = sorted(windows.values(), key=lambda frame: frame['ordinal'])
    for frame in frames:
        overlapping = [lease for lease in leases if lease['acquire_event'] < frame['ack_event'] and
                       (lease['release_event'] is None or lease['release_event'] > frame['enter_event'])]
        frame['overlapping_lease_count'] = len(overlapping)
        frame['overlapping_lease_entries'] = [lease['entry'] for lease in overlapping[:256]]
        frame['lease_listing_complete'] = len(overlapping) <= 256
        frame['relevant_lease_count'] = len(relevant[frame['entry']])
        frame['relevant_lease_entries'] = sorted(relevant[frame['entry']])[:256]
        frame['relevant_lease_listing_complete'] = len(relevant[frame['entry']]) <= 256
    groups = {}
    for identity in sorted(parent): groups.setdefault(root(identity), []).append(identity)
    return dict(schema=1, success=True, boundary='query_bound_observer_lifetimes',
                query_links=query_links, alias_groups=list(groups.values()), leases=leases, lease_gaps=gaps,
                remaining_leases=[lease for lease in leases if lease['release_event'] is None],
                retirements=retirements, frames=frames,
                frames_with_open_leases=sum(f['overlapping_lease_count'] > 0 for f in frames),
                frames_with_source_or_destination_leases=sum(f['relevant_lease_count'] > 0 for f in frames),
                competing_mutation_frames=ordering['frames_with_competing_mutations'],
                all_backing_aliases_resolved=False, cpu_writes_observed=False,
                producer_inputs_owned=False, complete_presented_journey=False)
