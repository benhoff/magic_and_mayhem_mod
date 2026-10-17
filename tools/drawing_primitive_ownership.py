"""Classify observed cross-boundary calls without replacing ownership gates."""


def overlaps(begin,end,lower,upper):
    return lower <= begin < upper if begin == end else begin < upper and end > lower


def join(accounting,lifetimes):
    if not accounting.get('success') or not lifetimes.get('success') or accounting['frequency']!=lifetimes['frequency']:
        raise ValueError('Successful same-QPC accounting and lifetimes required')
    if not lifetimes.get('producer_packets_crosschecked') or not lifetimes.get('source_callbacks_crosschecked'):
        raise ValueError('Independent producer/callback binding required')
    if accounting['requested_windows']!=120 or len(accounting['windows'])!=120:
        raise ValueError('The first120update denominator is required')
    boundaries={b['sequence']:b for b in lifetimes['boundaries']}
    calls={c['id']:c for c in lifetimes['pending_boundary_calls']}
    if len(boundaries)!=len(lifetimes['boundaries']) or len(calls)!=len(lifetimes['pending_boundary_calls']):
        raise ValueError('Duplicate primitive identity/boundary')
    pending=[];affected=set();crossings=[]
    api={c['entry']:c for c in accounting['original_call_intervals']}
    source_gaps={(g['sequence'],g['entry']) for g in accounting['source_point_gaps']}
    for w in accounting['windows']:
        for side in ('begin','end'):
            point=w['source_'+side];b=boundaries.get(point['sequence'])
            if b is None or (b['cadence_record'],b['producer_prefix'],b['queue_prefix'],b['pending'])!=(point['cadence_record'],point['producer_prefix'],point['queue_prefix'],point['pending_frames']) or len(b['calls'])!=b['pending']:
                raise ValueError('Primitive/source callback denominator differs')
            if len(set(b['calls']))!=len(b['calls']) or any(i not in calls for i in b['calls']):raise ValueError('Missing or duplicate pending primitive identity')
            if b['calls']:
                pending.append(dict(update=w['sequence'],side=side,callback_sequence=b['sequence'],calls=b['calls']))
                affected.update(b['calls'])
        for link in w['source_point_links']:
            if (w['sequence'],link['entry']) not in source_gaps:continue
            c=api.get(link['entry'])
            if c is None or c['type']!=1:raise ValueError('Source gap lacks original surface call')
            begin,end=w['begin_qpc'],w['end_qpc']
            execution_overlaps=overlaps(c['enter_qpc'],c['return_qpc'],begin,end)
            acknowledgement_crosses_start=c['return_qpc']<=begin<c['complete_qpc']
            points=[]
            for point in link['points']:
                for role in ('source','destination'):
                    value=point[role]
                    if not w['source_begin']['producer_prefix']<=value[3]<=w['source_end']['producer_prefix']:
                        points.append(dict(phase=point['phase'],role=role,producer_prefix=value[3],canvas_generation=value[6]))
            primitive_candidates=[]
            for p in calls.values():
                lo,hi=p['original_execution_envelope_qpc']
                if overlaps(lo,hi,c['enter_qpc'],c['return_qpc']):
                    primitive_candidates.append(p['id'])
            crossings.append(dict(update=w['sequence'],entry=c['entry'],thread=c['thread'],api_original_interval_qpc=[c['enter_qpc'],c['return_qpc']],api_acknowledgement_qpc=c['complete_qpc'],original_execution_overlaps_update=execution_overlaps,acknowledgement_crosses_update_start=acknowledgement_crosses_start,outside_prefix_points=points,temporally_overlapping_pending_primitives=primitive_candidates,exclusive_source_ownership_proved=False))
    return dict(schema=1,success=True,requested_windows=120,pending_boundary_associations=pending,unique_pending_calls=[calls[i] for i in sorted(affected)],
                unclassified_source_gaps=[g for g in accounting['source_point_gaps'] if (g['sequence'],g['entry']) not in {(c['update'],c['entry']) for c in crossings}],
                pending_tag_counts={str(tag):sum(calls[i]['tag']==tag for i in affected) for tag in sorted({calls[i]['tag'] for i in affected})},source_point_crossings=crossings,
                crossings_after_original_return=sum(c['acknowledgement_crosses_update_start'] for c in crossings),
                original_admission_gates=dict(accounting['gates']),complete_presented_interval=False,source_bytes_immutable=False,native_parity_performed=False,
                scope='Exact active call identities at observed source callbacks; source acquisition/execution envelopes and temporal API association only. ACK completion is separate from original return. Every original ownership gate and queue-free update remains unchanged.')
