"""Per-update accounting of the observed routes; global ownership stays independent."""
from collections import Counter
from drawing_observed_inputs import unique as observed_unique


def unique(items,key):
    return observed_unique(items,key,allow_zero=key=='sequence')


def overlaps(call,begin,end):
    a,b=call['enter_qpc'],call['return_qpc']
    return begin<=a<end if a==b else a<end and b>begin


def build(routes,timing,sources,queues,producer_records,physical_frames,egl,source_frames,count=120):
    if count!=120:raise ValueError('The accepted interval denominator is120')
    if type(count) is not int:raise ValueError('Integer interval denominator required')
    if type(routes['frequency']) is not int or routes['frequency']<=0:raise ValueError('Positive integer QPC frequency required')
    if routes['frequency']!=timing['frequency']:raise ValueError('Windows QPC frequency differs')
    points=unique(sources['points'],'sequence');calls=unique(timing['calls'],'entry');frames=unique(physical_frames,'entry')
    source_points=unique(source_frames,'entry');unique(physical_frames,'ordinal')
    if any(entry not in calls for entry in frames):raise ValueError('Primary frame lacks its admitted interval')
    eligible=[w for w in routes['windows'] if w['sequence'] in points]
    if len(eligible)<count:raise ValueError('Fewer than120 source-bound accepted windows')
    selected=eligible[:count];seen=[];previous=None;counter=None
    for w in selected:
        if not w['advancing'] or previous is not None and ((w['sequence']-previous)&0xffffffff!=1 or w['counter_before']!=counter):raise ValueError('120consecutive advancing updates required')
        previous=w['sequence'];counter=(w['counter_before']+1)&0xffffffff
        following=points.get((w['sequence']+1)&0xffffffff)
        if not following or points[w['sequence']]['cadence_record']!=w['begin_record'] or following['cadence_record']!=w['end_record']:
            raise ValueError('Missing exact source callback boundary')
    comparisons=egl['comparisons'];comparison_ids=[(c['egl_pid'],c['egl_call'],c['physical_ordinal']) for c in comparisons]
    if len(set(comparison_ids))!=len(comparison_ids):raise ValueError('Duplicate physical EGL comparison')
    by_entry={}
    for c in comparisons:
        if c['frame_entry'] not in calls:raise ValueError('EGL witness lacks admitted API interval')
        by_entry.setdefault(c['frame_entry'],[]).append(c)
    queues_by={}
    unique(queues['queues'],'queue')
    for q in queues['queues']:queues_by.setdefault(q['accepted_sequence'],[]).append(q)
    missing_primary=[];pending_source=[];straddlers=set();source_point_gaps=[];pixel_gaps=[]
    for w in selected:
        begin,end=w['begin_qpc'],w['end_qpc']
        if not 0<begin<end<2**64:raise ValueError('Accepted window time bounds')
        a=points[w['sequence']];b=points[(w['sequence']+1)&0xffffffff]
        bound_queues=queues_by.get(w['sequence'],[])
        if len(bound_queues)!=w['queue_count'] or b['queue_prefix']-a['queue_prefix']!=w['queue_count']:
            raise ValueError('Source/route queue denominator differs')
        for q in bound_queues:
            if not q['complete_update_window'] or not q['advancing_update_window'] or not q['update_queue_record_matches'] or not a['producer_prefix']<=q['producer_before']<q['producer_after']<=b['producer_prefix']:
                raise ValueError('Queue does not lie inside its source callback prefixes')
        if any(type(p['producer_prefix']) is not int or not 0<=p['producer_prefix']<=len(producer_records) for p in (a,b)) or a['producer_prefix']>b['producer_prefix']:raise ValueError('Source callback prefix outside producer denominator')
        if a['pending_frames'] or b['pending_frames']:pending_source.append(w['sequence'])
        admitted=[c for c in calls.values() if overlaps(c,begin,end)]
        crossing=[c['entry'] for c in admitted if c['enter_qpc']<begin or c['return_qpc']>end];straddlers.update(crossing)
        physical=[f for entry,f in frames.items() if entry in calls and begin<=calls[entry]['complete_qpc']<end]
        source_links=[];egl_links=[]
        for frame in physical:
            entry=frame['entry'];p=source_points.get(entry);linked=by_entry.get(entry,[])
            if not linked:missing_primary.append(dict(sequence=w['sequence'],entry=entry,ordinal=frame['ordinal']))
            if p is None or not p['both_generations_known'] or not p['producer_live_at_all_points']:
                source_point_gaps.append(dict(sequence=w['sequence'],entry=entry,reason='missing_unknown_or_stopped_producer_points'))
            elif any(not a['producer_prefix']<=v[3]<=b['producer_prefix'] for point in p['points'] for v in (point['source'],point['destination'])):
                source_point_gaps.append(dict(sequence=w['sequence'],entry=entry,reason='producer_points_cross_callback_prefixes'))
            if p is not None:
                source_links.append(dict(entry=entry,ordinal=frame['ordinal'],points=p['points'],both_generations_known=p['both_generations_known'],producer_live_at_all_points=p['producer_live_at_all_points']))
            for c in linked:
                if not c['pixels_match'] or not c['whole_source_image_agreement'].get('pixels_match'):pixel_gaps.append(dict(sequence=w['sequence'],entry=entry,egl_call=c['egl_call']))
                egl_links.append(dict(entry=entry,api_ordinal=frame['ordinal'],egl_pid=c['egl_pid'],egl_call=c['egl_call'],egl_ordinal=c['physical_ordinal'],
                                      submission=c['submission'],worker=c['worker'],command=c['command'],source_sha256=c['source_sha256'],physical_sha256=c['physical_sha256']))
        segment=producer_records[a['producer_prefix']:b['producer_prefix']]
        seen.append(dict(sequence=w['sequence'],counter_before=w['counter_before'],begin_qpc=begin,end_qpc=end,
                         route=w['queue_absence_or_route'],route_points=w['route_points'],results_push=w['results_push_observed'],
                         source_begin=a,source_end=b,producer_records=len(segment),producer_opcode_counts=dict(Counter(str(r[0][2]) for r in segment)),
                         source_queue_ids=[q['queue'] for q in bound_queues],original_call_entries=[c['entry'] for c in admitted],
                         input_calls=[c for c in admitted if c['type']==2],menu_actions=[c for c in admitted if c['type']==3],
                         calls_crossing_boundary=crossing,physical_api_entries=[f['entry'] for f in physical],physical_api_frames=physical,source_point_links=source_links,egl_witnesses=egl_links,
                         every_selected_branch_accounted=w['branch_accounted']))
    first,last=selected[0]['begin_qpc'],selected[-1]['end_qpc']
    cohort=[c for c in calls.values() if overlaps(c,first,last)]
    accounted={entry for w in seen for entry in w['original_call_entries']}
    if accounted!={c['entry'] for c in cohort}:raise ValueError('An admitted interval disappeared from the denominator')
    unknown=[w['sequence'] for w in selected if not w['branch_accounted']]
    gates=dict(consecutive_advancing_windows=True,exact_source_callback_prefixes=True,all_admitted_call_intervals_accounted=True,
               selected_world_branches_accounted=not unknown,source_callback_boundaries_quiescent=not pending_source,
               every_primary_witness_has_live_source_points=not source_point_gaps,every_primary_witness_source_worker_egl_bound=not missing_primary,
               all_linked_reference_regions_and_whole_images_match=not pixel_gaps,
               global_presentation_routes_closed=False,all_original_backing_alias_writers_closed=False)
    scoped=all(v for k,v in gates.items() if k not in ('global_presentation_routes_closed','all_original_backing_alias_writers_closed'))
    return dict(schema=1,success=True,requested_windows=count,first_sequence=selected[0]['sequence'],last_sequence=selected[-1]['sequence'],
                frequency=routes['frequency'],clock_domain='Windows_QueryPerformanceCounter',windows=seen,gates=gates,
                source_queues=sum(len(w['source_queue_ids']) for w in seen),queue_free_updates=[w['sequence'] for w in selected if not w['queue_count']],
                input_calls=sum(c['type']==2 for c in cohort),original_calls=len(cohort),original_call_intervals=cohort,calls_crossing_boundaries=sorted(straddlers),
                physical_primary_witnesses=sum(len(w['physical_api_entries']) for w in seen),egl_witnesses=sum(len(w['egl_witnesses']) for w in seen),
                unresolved_selected_branches=unknown,pending_source_boundary_updates=pending_source,missing_primary_egl_links=missing_primary,
                source_point_gaps=source_point_gaps,reference_pixel_gaps=pixel_gaps,
                complete_observed_route_120_accounting=scoped,complete_presented_interval=all(gates.values()),
                semantic_input_outcomes_proved=False,native_parity_performed=False,scanout_proved=False,complete_presented_journey=False)
