"""Exact two-blit protected source chains; never infer physical allocation ownership."""
from drawing_observed_inputs import unique

FULL_RECT=[0,0,800,600]
FULL_BOX=[0,0,800,600,0,1]


def bind(reference,prediction):
    if reference.get('success') is not True or prediction.get('success') is not True:
        raise ValueError('Successful independent reference/prediction required')
    submissions=unique(reference['wine_submission_observation']['calls'],'call')
    points=unique(reference['wine_retained_map_source_observation']['points'],'submission')
    queries=unique(reference['canvas_storage_transition_observation']['queries'],'query')
    unique(reference['wine_blit_packet_observation']['bindings'],'command')
    unique(reference['wine_blit_packet_observation']['bindings'],'worker')
    packets={}
    for p in reference['wine_blit_packet_observation']['bindings']:packets.setdefault(p['submission'],[]).append(p)
    workers=unique(reference['wine_blit_worker_observation']['calls'],'call')
    leases=unique(reference['api_source_worker_lease_observation']['calls'],'api_entry')
    held={}
    for h in reference['source_held_snapshot_observation']['frames']:
        pair=held.setdefault(h['api_entry'],{})
        if h['phase'] not in (1,2) or h['phase'] in pair:raise ValueError('Duplicate/invalid held source phase')
        pair[h['phase']]=h
    accepted=[];excluded=[];seen=set()
    for frame in prediction['comparisons']:
        key=(frame['entry'],frame['submission'],frame['egl_call'])
        if key in seen:raise ValueError('Duplicate prediction frame')
        seen.add(key)
        try:
            final=submissions[frame['submission']];pair=held[frame['entry']];lease=leases[frame['entry']]
            if frame['different_pixels'] or frame['source_different_pixels'] or set(pair)!={1,2}:
                raise ValueError('Unmatched independent pixels/held pair')
            initial=[s for s in submissions.values() if s['api_entry']==frame['entry'] and s['call']<final['call'] and (s['first_object'],s['first_generation'])==(final['second_object'],final['second_generation'])]
            if len(initial)!=1:raise ValueError('Missing/ambiguous initial texture link')
            first=initial[0];point=points[first['call']];m=point['metadata'];query=queries.get(m[15])
            source=pair[1]['before']
            if not frame['lease'] or any(h['thread']!=lease['owner'] or h['before'][6:12]!=source[6:12] for h in pair.values()):
                raise ValueError('Changed held source owner/state')
            if not query or not query['epoch_known'] or query['metadata']!=m or point['status']!=1 or not point['matched'] or m[1]!=2 or m[2]!=19:
                raise ValueError('Unknown initial mapped-source epoch')
            if m[3]!=source[3] or m[6:12]!=source[6:12] or m[14]!=pair[1]['thread'] or source[6]!=frame['boundary']['canvas'] or source[3]!=frame['boundary']['sequence']:
                raise ValueError('Initial mapped source differs from held/native prefix')
            if pair[1]['sha256']!=pair[2]['sha256'] or pair[1]['sha256']!=frame['source_sha256'] or pair[1]['lease']!=frame['lease'] or pair[2]['lease']!=frame['lease'] or lease['lease']!=frame['lease']:
                raise ValueError('Protected source version/lease differs')
            for s in (first,final):
                if s['method']!=1 or s['hresult'] or s['source_rect']!=FULL_RECT or s['destination_rect']!=FULL_RECT or s['api_entry']!=frame['entry'] or s['thread']!=lease['owner'] or not s['first_generation'] or not s['second_generation']:
                    raise ValueError('Unsupported two-stage submission state')
                ps=packets.get(s['call'],[])
                if len(ps)!=1:raise ValueError('Missing/ambiguous stage command')
                packet=ps[0];worker=workers.get(packet['worker']);returned=lease['commands'].get(str(packet['command']))
                if not packet['worker_entry_record']<packet['worker_return_record']:
                    raise ValueError('Incomplete stage worker interval')
                if not worker or not returned or (returned['worker'],returned['thread'])!=(packet['worker'],packet['worker_thread']) or worker['thread']!=packet['worker_thread']:
                    raise ValueError('Stage command/worker/lease mismatch')
                if (worker['destination'],worker['destination_generation'],worker['source'],worker['source_generation'])!=(s['first_object'],s['first_generation'],s['second_object'],s['second_generation']):
                    raise ValueError('Stage worker texture generation mismatch')
                if worker['source_box']!=FULL_BOX or worker['destination_box']!=FULL_BOX or worker['source_subresource'] or worker['destination_subresource'] or worker['filter']!=1 or any(worker['effects']):
                    raise ValueError('Unsupported stage worker geometry/effects')
            a,b=packets[first['call']][0],packets[final['call']][0]
            if a['worker_return_record']>=b['worker_entry_record'] or b['command']!=frame['command'] or b['worker']!=frame['worker']:
                raise ValueError('Stage worker ordering/final witness mismatch')
            accepted.append(dict(api_entry=frame['entry'],initial_submission=first['call'],final_submission=final['call'],query=query['query'],observed_epoch=query['epoch'],source_canvas=source[6],producer_prefix=source[3],intermediate_texture=first['first_object'],intermediate_generation=first['first_generation'],initial_command=a['command'],final_command=b['command'],initial_worker=a['worker'],final_worker=b['worker'],lease=frame['lease'],egl_call=frame['egl_call'],physical_ordinal=frame['ordinal'],different_pixels=0,source_different_pixels=0,allocation_ownership_proved=False))
        except (KeyError,ValueError) as error:
            excluded.append(dict(api_entry=frame['entry'],submission=frame['submission'],egl_call=frame['egl_call'],reason=str(error)))
    return dict(schema=1,success=True,accepted=accepted,excluded=excluded,accepted_frames=len(accepted),excluded_frames=len(excluded),original_game_executed=False,allocation_ownership_proved=False,unobserved_uploads_proved=False,scanout_proved=False,complete_presented_journey=False)
