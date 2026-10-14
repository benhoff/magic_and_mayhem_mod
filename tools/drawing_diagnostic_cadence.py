"""Bounded update timing measurements; counter observation is still diagnostic."""
from statistics import median

DISABLED = ('session_producers','producer_samples','source_byte_witness','source_readonly_lease','source_worker_lease',
            'wine_gpu_completion','source_write_trace','wine_source_version','wine_source_map_bounded','wine_source_map','wine_source_descriptor','source_pending_copy','source_held_snapshot','cursor_mutex','actor_latches','region_input','hud_state','hud_input_point','campaign_guidance','semantic_state','campaign_pause','canvas_storage_transitions','whole_updates','producer_update_binding','wine_submission',
            'wine_texture_entry','wine_blit_worker','wine_blit_packet','wine_worker_context',
            'wine_resource_copy','wine_resource_map','wine_map_storage_point','wine_client_memory','wine_retained_map_source',
            'egl_physical','x11_delivery_library','physical_peer')


def validate_control(args):
    if not args.cadence_control:
        return
    if not args.control or not args.engine_cadence or args.observer_mode!='metadata':
        raise ValueError('Cadence control requires explicit control/counter-only metadata mode')
    if any(getattr(args,name) for name in DISABLED):
        raise ValueError('Cadence control cannot enable drawing/source/Wine/display capture')


def measurements(observation,limit=120):
    if not 1<=limit<=120 or observation.get('success') is not True or observation.get('frequency',0)<=0:
        raise ValueError('Cadence observation bounds')
    windows=observation['windows'];samples=[]
    for i,w in enumerate(windows[:-1]):
        following=windows[i+1]
        if not w['queues']:
            continue
        if not w['advancing'] or (following['sequence']-w['sequence'])&0xffffffff!=1 or following['counter_before']!=w['ticks'][0] or following['qpc']<=w['qpc']:
            raise ValueError('Queued cadence window incomplete/inconsistent')
        samples.append(dict(sequence=w['sequence'],counter=w['counter_before'],queues=len(w['queues']),
                            milliseconds=1000*(following['qpc']-w['qpc'])/observation['frequency']))
        if len(samples)==limit:
            break
    if not samples:
        raise ValueError('No bounded queued update timing samples')
    values=sorted(s['milliseconds'] for s in samples)
    p95=values[min(len(values)-1,(95*len(values)+99)//100-1)]
    return dict(samples=samples,count=len(samples),median_ms=median(values),p95_ms=p95,
                total_ms=sum(values),min_ms=values[0],max_ms=values[-1],
                identical_gameplay_work_proved=False,unobtrusive_capture_proved=False)


def compare(control,diagnostic,limit=120):
    a=measurements(control,limit);b=measurements(diagnostic,limit)
    common=min(a['count'],b['count']);a=measurements(control,common);b=measurements(diagnostic,common)
    return dict(schema=1,success=True,common_queued_updates=common,control=a,diagnostic=b,
                median_ratio=b['median_ms']/a['median_ms'],p95_ratio=b['p95_ms']/a['p95_ms'],
                source_ownership_proved=False,identical_gameplay_work_proved=False,
                unobtrusive_capture_proved=False,complete_presented_journey=False)


def results_action(duration):
    """Declared diagnostic key policy; actual menu state must still be observed."""
    if type(duration) is not int or duration not in (60,600):
        raise ValueError('Results key hold must be60or600ms')
    return dict(kind='tap',key='Escape',after_ms=600) if duration==60 else dict(kind='hold',key='Escape',duration_ms=duration,after_ms=600)
