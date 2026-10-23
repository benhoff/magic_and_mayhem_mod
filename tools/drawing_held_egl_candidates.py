"""A new exact held-source/command/EGL cohort; legacy API exclusions stay intact."""
from drawing_comparison import index
FULL_RECT=[0,0,800,600]
FULL_BOX=[0,0,800,600,0,1]


def candidates(reference,records):
    snapshots={}
    for f in reference['source_held_snapshot_observation']['frames']:
        pair=snapshots.setdefault(f['api_entry'],{})
        if type(f['phase']) is not int or f['phase'] not in (1,2) or f['phase'] in pair:
            raise ValueError('Invalid or duplicate held snapshot phase')
        pair[f['phase']]=f
    leases=index(reference['api_source_worker_lease_observation']['calls'],'api_entry','leases')
    packets=index(reference['wine_blit_packet_observation']['bindings'],'command','packets')
    workers=index(reference['wine_blit_worker_observation']['calls'],'call','workers')
    submissions=index(reference['wine_submission_observation']['calls'],'call','submissions')
    physical=index(reference['egl_physical_observation']['frames'],'entry','physical')
    state={0:(False,0)};active=False;completed=0
    for r,_ in records:
        if r[2]==11:active=True
        elif r[2]==12:active=False;completed=r[14]
        state[r[1]]=(active,completed)
    selected=[];excluded=[];prior=0
    for call in reference['egl_worker_context_observation']['calls']:
        reason=None;point=call['points'][0]['context'];command=point['command'];packet=packets.get(command);worker=workers.get(point['worker']);submission=submissions.get(point['submission']);frame=physical.get(call['call'])
        if not call['stable_active_context']:reason='inactive-unstable-context'
        elif not packet or not worker or not submission or not frame:reason='missing-exact-command-or-physical-witness'
        elif (packet['worker'],packet['submission'],packet['worker_thread'])!=(point['worker'],point['submission'],point['win_thread']) or worker['thread']!=point['win_thread']:reason='changed-command-worker-identity'
        else:
            api=submission['api_entry'];lease=leases.get(api);pair=snapshots.get(api,{})
            if not lease or set(pair)!={1,2} or str(command) not in lease['commands']:reason='command-without-accepted-source-lease'
            elif pair[1]['lease']!=lease['lease'] or pair[2]['lease']!=lease['lease'] or pair[1]['thread']!=lease['owner'] or pair[2]['thread']!=lease['owner'] or pair[1]['sha256']!=pair[2]['sha256']:reason='changed-held-source-version'
            elif lease['commands'][str(command)]['worker']!=point['worker'] or lease['commands'][str(command)]['thread']!=point['win_thread']:reason='lease-worker-return-mismatch'
            elif submission['hresult'] or submission['method']!=1 or submission['source_rect']!=FULL_RECT or submission['destination_rect']!=FULL_RECT:reason='unsupported-submission-result-or-geometry'
            elif worker['source_box']!=FULL_BOX or worker['destination_box']!=FULL_BOX or worker['source_subresource'] or worker['destination_subresource'] or worker['filter']!=1 or any(worker['effects']):reason='partial-or-unsupported-worker-command'
            elif not worker['source_generation'] or not worker['destination_generation']:reason='unknown-worker-generation'
            elif (worker['destination'],worker['source'],worker['destination_generation'],worker['source_generation'])!=(submission['first_object'],submission['second_object'],submission['first_generation'],submission['second_generation']):reason='worker-submission-generation-mismatch'
            else:
                source=pair[1]['before'];sequence=source[3]
                if source[2]!=19 or source[5] or source[7:10]!=[800,600,800] or source[13] or sequence not in state:reason='unsupported-held-source-point'
                elif state[sequence][0]:reason='world-queue-active-at-source-copy'
                elif sequence<=prior:reason='repeated-or-reordered-source-prefix'
                elif frame['width']!=800 or frame['height']!=600:reason='physical-extent-changed'
                else:
                    prior=sequence;selected.append(dict(ordinal=frame['ordinal'],egl_call=call['call'],entry=api,command=command,worker=point['worker'],submission=point['submission'],lease=lease['lease'],source_oracle=pair[1]['oracle'],source_path=pair[1]['path'],source_sha256=pair[1]['sha256'],boundary=dict(sequence=sequence,queues=state[sequence][1],canvas=source[6],width=800,height=600)))
        if reason:excluded.append(dict(egl_call=call['call'],command=command,reason=reason))
    return selected,excluded
