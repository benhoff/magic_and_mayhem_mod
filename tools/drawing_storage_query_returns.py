"""Associate numerical query inputs with unique same-thread wrapper Lock returns."""


def bind(observation):
    if not observation['closed']:
        raise ValueError('Completed storage transition trace required')
    returns = {e['call']: e for e in observation['events'] if e['phase'] == 2}
    ordered = [(e['ordinal'], 'event', e) for e in observation['events']]
    ordered += [(q['ordinal'], 'query', q) for q in observation['queries']]
    active, bindings, unbound = {}, [], []
    for ordinal, kind, row in sorted(ordered):
        if kind == 'event':
            if row['phase'] == 1:
                active[row['call']] = row
            else:
                active.pop(row['call'], None)
            continue
        metadata = row['metadata']
        input_address = row.get('input_address')
        locks = [e for e in active.values() if e['tag'] == 2 and e['thread'] == metadata[14]]
        reason = None
        if input_address is None:
            reason = 'legacy-input-identity-absent'
        elif not input_address:
            reason = 'null-input-address'
        elif row['route'] != 1:
            reason = 'non-map-query'
        elif metadata[2] & 7 != 3 or metadata[5]:
            reason = 'not-live-or-failed'
        elif metadata[2] not in (3, 19, 131):
            reason = 'unsupported-point-state'
        elif len(locks) != 1:
            reason = 'no-same-thread-lock' if not locks else 'ambiguous-same-thread-lock'
        else:
            before = locks[0]
            returned = returns[before['call']]
            if not before['ordinal'] < ordinal < returned['ordinal']:
                raise ValueError('Query outside its observed Lock interval')
            if not before['cache_known'] or not before['canvas'] or not returned['cache_known'] or returned['canvas'] != before['canvas']:
                reason = 'unknown-or-changed-canvas'
            elif returned['returned_address'] != input_address:
                reason = 'different-returned-address'
            elif returned['cached_address'] != input_address:
                raise ValueError('Returned address differs from recorded cache')
            elif metadata[6] and metadata[6] != returned['canvas']:
                reason = 'query-canvas-differs'
            else:
                bindings.append(dict(query=row['query'], input_address=input_address,
                                     call=before['call'], thread=metadata[14],
                                     entry_ordinal=before['ordinal'], query_ordinal=ordinal,
                                     return_ordinal=returned['ordinal'], canvas=returned['canvas'],
                                     lock_return_epoch=returned['lock_return_epoch'],
                                     query_producer_prefix=metadata[3],
                                     return_producer_prefix=returned['producer_prefix'],
                                     point_flags=metadata[2], allocation_ownership_proved=False))
        if reason:
            unbound.append(dict(query=row['query'], input_address=input_address, reason=reason))
    return dict(schema=1, success=True, bindings=bindings, unbound=unbound,
                queries_accounted=len(bindings) + len(unbound),
                pointer_equality_only=True, original_game_reexecuted=False,
                source_storage_owned=False, allocation_ownership_proved=False,
                source_upload_identity_proved=False, complete_presented_journey=False)
