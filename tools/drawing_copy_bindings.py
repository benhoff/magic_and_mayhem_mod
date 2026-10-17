"""Conservative request/API/command associations within recorded producer intervals."""


def associate(observation, calls, points, submissions, packets):
    entries = {r['request']: r for r in observation['entries']}
    returns = {r['request']: r for r in observation['returns']}
    if set(entries) != set(returns):
        raise ValueError('Incomplete request identity set')
    matched, excluded = {}, []
    for api, call in calls.items():
        if call[2] != 7:
            continue
        phases = points.get(api, {})
        if set(phases) != {1, 2, 3}:
            raise ValueError('Missing API point phases')
        candidates = []
        for request, entry in entries.items():
            returned = returns[request]
            if call[8] or call[6] != entry['owner']:
                continue
            expected_flags = 16 if entry['raw_key'] == 0xffffffff else 17
            if call[21] != expected_flags or list(call[22:24]) != entry['destination_position'] or list(call[24:28]) != entry['source_rectangle']:
                continue
            valid = True
            for point in phases.values():
                if (point[7] != entry['owner'] or point[30] != entry['owner'] or point[46] != entry['owner'] or
                        point[18] != 19 or point[34] != 19 or point[21] or
                        point[19] != point[35] or
                        not entry['producer_prefix'] <= point[19] < returned['producer_return_sequence'] or
                        point[22] != entry['destination'] or point[38] != entry['source'] or
                        list(point[23:25]) != entry['destination_extent_stride'][:2] or
                        entry['destination_extent_stride'][2] and point[25] != entry['destination_extent_stride'][2]):
                    valid = False
                    break
            if valid:
                candidates.append(request)
        if len(candidates) > 1:
            raise ValueError('Ambiguous request/API association')
        if not candidates:
            excluded.append(api)
            continue
        request = candidates[0]
        if request in matched:
            raise ValueError('Multiple API calls match one request')
        selected = [s for s in submissions if s['api_entry'] == api]
        if not selected or any(s['thread'] != call[6] or s['hresult'] for s in selected):
            raise ValueError('Matched API lacks successful same-thread submission')
        commands = [p for p in packets if p['submission'] in {s['call'] for s in selected}]
        if not commands or any(not any(p['submission'] == s['call'] for p in commands) for s in selected):
            raise ValueError('Matched request lacks completed worker commands')
        if any(p['emit_thread'] != call[6] or p['worker_entry_record'] >= p['worker_return_record'] for p in commands):
            raise ValueError('Request command owner/order changed')
        matched[request] = dict(request=request, api_entry=api, owner=call[6],
                                producer_return_sequence=returns[request]['producer_return_sequence'],
                                producer_entry_prefix=entries[request]['producer_prefix'],
                                producer_point_prefixes=[phases[i][19] for i in (1, 2, 3)],
                                submissions=[s['call'] for s in selected], commands=commands)
    worker_ids = [p['worker'] for m in matched.values() for p in m['commands']]
    command_ids = [p['command'] for m in matched.values() for p in m['commands']]
    if len(set(worker_ids)) != len(worker_ids) or len(set(command_ids)) != len(command_ids):
        raise ValueError('Shared worker/command identity between requests')
    positions = {r['request']: i for i, r in enumerate(observation['returns'])}
    reversals = []
    for request in sorted(matched):
        following = request + 1
        if following not in matched:
            continue
        first, second = matched[request]['commands'], matched[following]['commands']
        if max(p['worker_return_record'] for p in first) < min(p['worker_entry_record'] for p in second):
            order = [request, following]
        elif max(p['worker_return_record'] for p in second) < min(p['worker_entry_record'] for p in first):
            order = [following, request]
        else:
            order = None
        if positions[request] > positions[following]:
            reversals.append(dict(entry_order=[request, following],
                                  return_order=[following, request], worker_order=order))
    return dict(bindings=list(matched.values()), unbound_requests=sorted(set(entries) - set(matched)),
                excluded_api_entries=excluded, adjacent_return_reversals=reversals,
                all_requests_bound=len(matched) == len(entries),
                general_replay_order_proved=False, source_or_physical_ownership_proved=False)
