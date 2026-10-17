"""Explain selected World queue absence using original comparison flags, not counts."""
import struct
from engine_cadence import decode
from drawing_engine_windows import analyze as windows


def analyze(path):
    if struct.unpack_from('<I',path.read_bytes(),8)[0]!=2:
        raise ValueError('Original route comparisons require cadence V2')
    frequency,rows=decode(path);observed=windows(path)
    boundaries=[r for r in rows if r[1]==2]
    results=[]
    for window,next_boundary in zip(observed['windows'],boundaries[1:]):
        group=[r for r in rows if window['begin_record']<=r[0]<next_boundary[0]]
        routes={kind:[r for r in group if r[1]==kind] for kind in (9,10,18,20,21)}
        for kind,items in routes.items():
            if len(items)>1:raise ValueError('Repeated selected route within accepted update')
            for r in items:
                site={9:0x46bb9d,10:0x46bba5,18:0x46bbd8,20:0x46bc19,21:0x4fd536}[kind]
                if r[13]!=site or any(r[20:]) or r[16]>255 or r[17]>255 or r[19]>1:
                    raise ValueError('Route comparison site/scalar identity')
                if kind==21 and bool(r[15]&64)==bool(r[19]):raise ValueError('Target comparison flags/result differ')
        gate=routes[9];transition=routes[10];scene=routes[18];target=routes[21]
        if transition and (not gate or not gate[0][15]&64 or gate[0][0]>=transition[0][0]):
            raise ValueError('Transition gate without passed suppression gate')
        if scene and (not transition or not transition[0][15]&64 or transition[0][0]>=scene[0][0]):
            raise ValueError('Scene without passed original gates')
        if target and (not scene or target[0][0]<=scene[0][0]):raise ValueError('Target gate without selected scene')
        queues=window['queues']
        if queues and (not scene or any(q['record']<=scene[0][0] for q in queues)):
            raise ValueError('Queue without selected World scene')
        if queues and target and (not target[0][19] or any(q['caller']!=0x4fd559 or q['record']<=target[0][0] for q in queues)):
            raise ValueError('Default target/queue branch mismatch')
        if queues:reason='selected_world_queue'
        elif gate and not gate[0][15]&64:reason='original_suppression_branch'
        elif transition and not transition[0][15]&64:reason='original_transition_branch'
        elif target and not target[0][19]:reason='original_default_target_unavailable'
        else:reason='unresolved_before_or_inside_selected_scene'
        results.append(dict(sequence=window['sequence'],advancing=window['advancing'],counter_before=window['counter_before'],
                            begin_qpc=window['qpc'],end_qpc=next_boundary[3]|next_boundary[4]<<32,
                            begin_record=window['begin_record'],end_record=next_boundary[0],thread=window['thread'],
                            queue_count=len(queues),queue_absence_or_route=reason,
                            branch_accounted=reason!='unresolved_before_or_inside_selected_scene',
                            route_points=[dict(record=r[0],event=r[1],site=r[13],qpc=r[3]|r[4]<<32,
                                               comparison_zero=bool(r[15]&64),transition_byte=r[16],results_pending_byte=r[17],
                                               mode_word=r[18],target_available=bool(r[19])) for r in group if r[1] in routes],
                            results_push_observed=bool(routes[20])))
    return dict(schema=1,success=True,frequency=frequency,journal_sha256=observed['journal_sha256'],windows=results,
                all_selected_branches_accounted=all(w['branch_accounted'] for w in results),
                all_presentation_routes_observed=False,no_rendering_in_queue_free_windows_proved=False,
                complete_presented_journey=False)
