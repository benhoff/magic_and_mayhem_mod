"""Temporal QueryInterface aliases and CPU/DC leases in admitted call intervals.

This is read-only diagnostic analysis. It never treats an attached surface as an
alias, infers CPU writes from pixels, or proves all backing-storage aliases.
"""
import struct
from drawing_call_intervals import MUTATORS, decode_intervals
from drawing_lease_intervals import LeaseIntervals


def analyze_lifetimes(intervals, journal):
    ordering = decode_intervals(intervals, journal)
    calls = {r[29]: r for r in struct.iter_unpack('<32I', journal[64:-128])}
    events = list(struct.iter_unpack('<16I', intervals[64:-64]))
    parent, active, leases = {}, {}, []
    query_links, gaps, retirements, windows = [], [], [], {}
    open_windows, relevant = {}, {}
    frame_entries = {f['entry']: f['ordinal'] for f in ordering['frames']}
    pending_mutators = set()
    exposure = {f['entry']: dict(listed=set(f['competing_entries']), shared={}, frame=f)
                for f in ordering['frames']}

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

    def surface_generations(call):
        # Only these methods carry a source/target surface generation. Other
        # related fields must not become aliases or shared-surface evidence.
        return {identity for identity in ([call[3], call[4]] if call[2] in (5, 7, 11)
                                         else [call[3]]) if identity}

    def observe_mutator_relations(event):
        for owner, target in open_windows.items():
            state = exposure[owner]
            target_ids = surface_generations(target)
            target_roots = {root(identity) for identity in target_ids}
            for other in pending_mutators & state['listed']:
                if other in state['shared']:
                    continue
                other_ids = surface_generations(calls[other])
                common = target_roots & {root(identity) for identity in other_ids}
                if common:
                    state['shared'][other] = dict(entry=other, observed_event=event,
                        frame_generations=sorted(target_ids),
                        competing_generations=sorted(other_ids),
                        shared_owner_groups=sorted(common))

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
        if phase == 1 and call[1] == 1 and call[2] in MUTATORS:
            pending_mutators.add(entry)
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
        observe_mutator_relations(ordinal)
        if phase != 2: continue
        successful = not call[8] & 0x80000000
        if call[1] in (1, 5) and call[2] == 0 and successful and call[3] and call[4]:
            merge(call[3], call[4], ordinal, entry)
            # A query returned during the overlap can establish a relationship;
            # a future query cannot repair an already returned competitor.
            observe_mutator_relations(ordinal)
        pending_mutators.discard(entry)
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
    lease_intervals = LeaseIntervals(leases)
    for frame in frames:
        count, entries = lease_intervals.overlaps(frame['enter_event'], frame['ack_event'])
        frame['overlapping_lease_count'] = count
        frame['overlapping_lease_entries'] = entries
        frame['lease_listing_complete'] = count <= 256
        frame['relevant_lease_count'] = len(relevant[frame['entry']])
        frame['relevant_lease_entries'] = sorted(relevant[frame['entry']])[:256]
        frame['relevant_lease_listing_complete'] = len(relevant[frame['entry']]) <= 256
    groups = {}
    for identity in sorted(parent): groups.setdefault(root(identity), []).append(identity)
    relation_frames = []
    for state in sorted(exposure.values(), key=lambda state: state['frame']['ordinal']):
        frame = state['frame']; listed = state['listed']; shared = state['shared']
        relation_frames.append(dict(ordinal=frame['ordinal'], entry=frame['entry'],
            competing_mutation_count=frame['competing_mutation_count'],
            listed_competing_mutation_count=len(listed),
            known_shared_competing_mutation_count=len(shared),
            unresolved_listed_competing_entries=sorted(listed - shared.keys()),
            unlisted_competing_mutation_count=frame['competing_mutation_count'] - len(listed),
            competitor_listing_complete=frame['competitor_listing_complete'],
            known_shared_relations=[shared[entry] for entry in sorted(shared)]))
    mutator_alias_exposure = dict(schema=1, frames=relation_frames,
        frames_with_known_shared_competitors=sum(bool(f['known_shared_relations']) for f in relation_frames),
        frames_with_unresolved_listed_competitors=sum(bool(f['unresolved_listed_competing_entries']) for f in relation_frames),
        frames_with_unlisted_competitors=sum(bool(f['unlisted_competing_mutation_count']) for f in relation_frames),
        disjoint_backing_storage_proved=False, actual_writes_proved=False,
        exclusive_frame_ownership_proved=False, all_backing_aliases_resolved=False)
    return dict(schema=1, success=True, boundary='query_bound_observer_lifetimes',
                query_links=query_links, alias_groups=list(groups.values()), leases=leases, lease_gaps=gaps,
                remaining_leases=[lease for lease in leases if lease['release_event'] is None],
                retirements=retirements, frames=frames,
                frames_with_open_leases=sum(f['overlapping_lease_count'] > 0 for f in frames),
                frames_with_source_or_destination_leases=sum(f['relevant_lease_count'] > 0 for f in frames),
                competing_mutation_frames=ordering['frames_with_competing_mutations'],
                mutator_alias_exposure=mutator_alias_exposure,
                all_backing_aliases_resolved=False, cpu_writes_observed=False,
                producer_inputs_owned=False, complete_presented_journey=False)
