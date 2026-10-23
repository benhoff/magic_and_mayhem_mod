"""Bind current source-only checkpoints to a closed first-World storage prefix."""
import hashlib


def unique(rows, key):
    result = {}
    for row in rows:
        value = row[key]
        if type(value) is not int or not 0 < value <= 0xffffffff or value in result:
            raise ValueError('Duplicate or invalid checkpoint ' + key)
        result[value] = row
    return result


def analyze(records, storage, replay, original, predicted):
    from drawing_storage_transitions import bind_producer
    if not replay['success'] or not replay['sources_stable'] or replay['native_exit']:
        raise ValueError('Successful current replay required')
    native = replay['native']
    if (not native['success'] or native['remaining_surfaces'] or replay['replay_order_changed'] or
        any(native[k] for k in ('original_oracles_read', 'original_pixels_used_as_native_inputs',
                               'original_raster_results_used_as_native_inputs', 'live_replacement')) or
        replay['original_pixels_used_as_native_inputs']):
        raise ValueError('Source-only unchanged-order native replay required')
    expected = [r for r, _ in records if r[2] == 10]
    comparisons = unique(replay['comparisons'], 'ordinal')
    checkpoints = unique(native['checkpoints'], 'ordinal')
    unique(replay['comparisons'], 'sequence')
    unique(native['checkpoints'], 'sequence')
    if list(comparisons) != [r[14] for r in expected] or list(checkpoints) != list(comparisons):
        raise ValueError('Missing or reordered complete checkpoint cohort')
    for r in expected:
        c, n = comparisons[r[14]], checkpoints[r[14]]
        identity = (r[1], r[3], r[5], r[6])
        if (tuple(c[k] for k in ('sequence', 'canvas', 'width', 'height')) != identity or
            tuple(n[k] for k in ('sequence', 'canvas', 'width', 'height')) != identity or
            type(c['different_pixels']) is not int or c['different_pixels'] != 0 or
            c['original_sha256'] != c['native_sha256'] or c['native_sha256'] != n['sha256']):
            raise ValueError('Checkpoint identity or full native comparison differs')
    first = next(r for r, _ in records if r[2] == 11)
    before = next(r for r, _ in reversed(records[:first[1]-1]) if r[2] == 10 and r[3] == first[3])
    if before[5:8] != first[5:8]:
        raise ValueError('Initial checkpoint/World extent changed')
    intervening = [r for r, _ in records if before[1] < r[1] < first[1]]
    if any(r[3] == first[3] and r[2] not in (3, 10) for r in intervening):
        raise ValueError('Writer/lifecycle transition after compared checkpoint')
    generations = [r for r, _ in records if r[1] < first[1] and r[3] == first[3] and r[2] in (1, 2)]
    if [r[2] for r in generations] != [1] or not storage['closed'] or storage['calls'] != storage['returns']:
        raise ValueError('Unclosed logical canvas/storage generation')
    locks = [e for e in storage['events'] if e['canvas'] == first[3] and
             e['producer_prefix'] < first[1] and e['phase'] == 2 and e['tag'] == 2 and
             e['cache_known'] and e['lock_return_epoch'] and e['returned_address']]
    if not locks:
        raise ValueError('Missing same-run observed Lock return for initial canvas')
    if len(original) != first[5]*first[6]*2 or original != predicted:
        raise ValueError('Full initial native canvas differs')
    digest = hashlib.sha256(original).hexdigest()
    if comparisons[before[14]]['original_sha256'] != digest:
        raise ValueError('Initial full plane is not the compared checkpoint')
    binding = bind_producer(storage, records)
    return dict(success=True, first_world_queue=first[14], first_world_sequence=first[1],
                logical_canvas=first[3], initial_checkpoint_sequence=before[1],
                initial_oracle=before[14], initial_pixels_compared=first[5]*first[6],
                initial_original_sha256=digest, initial_native_sha256=digest,
                native_initial_canvas_fully_defined=True, all_replay_checkpoints=len(expected),
                first_canvas_creation_sequence=generations[0][1],
                intervening_records=[dict(sequence=r[1], op=r[2], destination=r[3], source=r[4]) for r in intervening],
                preworld_storage_events=[e for e in storage['events'] if e['canvas'] == first[3] and e['producer_prefix'] < first[1]],
                storage_summary={k: storage[k] for k in ('calls', 'returns', 'closed', 'observed_lock_return_epochs', 'address_identities')},
                storage_producer_binding=binding, original_pixels_used_as_native_inputs=False,
                last_observed_lock_return_epoch=locks[-1]['lock_return_epoch'],
                physical_allocation_ownership_proved=False, all_writers_observed=False, live_replacement=False)
