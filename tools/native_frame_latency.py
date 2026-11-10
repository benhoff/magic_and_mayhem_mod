"""Bounded native host delivery distributions; Qt swap is not physical scanout."""
import math
import re
from profile_native_resident_helpers import distribution


def identity(value):
    """Read Qt decimal uint64 identifiers without truncating floats or bools."""
    if isinstance(value, str) and re.fullmatch(r'0|[1-9][0-9]{0,19}', value):
        value = int(value)
    if type(value) is not int or not 0 <= value < 2**64:
        raise ValueError('Invalid native trace identity')
    return value


def rows(trace, key, columns):
    names=trace[columns]
    if len(set(names))!=len(names):raise ValueError('Duplicate trace columns')
    result=[]
    for values in trace[key]:
        if len(values)!=len(names):raise ValueError('Trace row extent')
        result.append(dict(zip(names,values)))
    return result


def elapsed(begin,end):
    if any(type(v) not in (int,float) or not math.isfinite(v) for v in (begin,end)) or end<begin:
        raise ValueError('Invalid native host timestamp interval')
    return (end-begin)/1000.


def original_cadence(turns):
    """Disjoint original-clock spans. Pacer/screen timings precede this entry."""
    samples={k:[] for k in ('entry_interval_ms','world_body_ms','between_world_ms',
        'preceding_pacer_ms','preceding_screen_ms','unclassified_between_world_ms')}
    previous=None;unobserved=0;gaps=0
    def word(value):
        if type(value) not in (int,float) or not math.isfinite(value) or int(value)!=value or not 0<=value<2**32:
            raise ValueError('Invalid original uint32 timing')
        return int(value)
    for r in turns:
        if not r['draw_allowed'] or r['paused'] or identity(r['source_after'])<64:
            previous=None;continue
        if identity(r['source_before'])==identity(r['source_after']):continue
        if 'original_tick' not in r:
            previous=None;continue
        tick=word(r['original_tick'])
        body=sum(word(r[k]) for k in ('original_capture_ms','original_ack_wait_ms','original_return_ms'))
        if previous is not None:
            a,old_tick,old_body=previous
            if identity(r['source_after'])!=identity(a['source_after'])+1:gaps+=1
            else:
                interval=(tick-old_tick)%2**32
                if interval>=2**31 or old_body>interval:raise ValueError('Invalid original cadence ordering')
                between=interval-old_body
                samples['entry_interval_ms'].append(interval);samples['world_body_ms'].append(old_body)
                samples['between_world_ms'].append(between)
                if r.get('original_pacer_mask')==3:
                    pacer=(word(r['original_pacer_end'])-word(r['original_pacer_start']))%2**32
                    if pacer>=2**31:raise ValueError('Invalid original pacer ordering')
                    screen=word(r['original_screen_ms'])
                    samples['preceding_pacer_ms'].append(pacer);samples['preceding_screen_ms'].append(screen)
                    samples['unclassified_between_world_ms'].append(between-pacer-screen)
                else:unobserved+=1
        previous=(r,tick,body)
    return dict(distributions={k:distribution(v) for k,v in samples.items()},
        unobserved_pacer_pairs=unobserved,nonadjacent_pairs=gaps,
        negative_residuals=sum(v<0 for v in samples['unclassified_between_world_ms']),
        scope='Adjacent warm visible source generations only; hidden/pause/cold rows reset pairs. uint32 original clock wrap is preserved. Body and between-World spans partition entry intervals. Pacer and screen belong to the following entry. Residual is elapsed unclassified work, including simulation/UI/bridge blocking, not CPU execution; absent pacer mask is unobserved. Distributions have separate medians and must not be added.')


def summarize(report):
    trace=report['frame_trace']
    if trace.get('enabled') is not True or type(trace.get('schema')) is not int or trace['schema'] != 4:
        raise ValueError('Unsupported or disabled native frame trace')
    turns=rows(trace,'rows','columns');gui=rows(trace,'gui_rows','gui_columns')
    warm=lambda r:r['draw_allowed'] and not r['paused']
    active=[r for r in turns if warm(r) and identity(r['source_after'])>=64]
    sources=[r for r in active if identity(r['source_before'])!=identity(r['source_after'])]
    exports={identity(r['export_serial']):r for r in active if identity(r['export_serial'])}
    if len(exports)!=sum(bool(identity(r['export_serial'])) for r in active):raise ValueError('Duplicate native export')
    phases={name:[] for name in ('world','commands','extra_world_ui','export','status','wait')}
    bounds=('begin_us','world_end_us','commands_end_us','extra_end_us','export_end_us','status_end_us','wait_end_us')
    for r in active:
        for name,a,b in zip(phases,bounds,bounds[1:]):phases[name].append(elapsed(r[a],r[b]))
    pickup={};paints={};first_swaps={};pickup_ms=[];swap_ms=[];total_ms=[];paint_ms=[];intervals=[]
    last_swap=None;last_time=None
    for g in gui:
        when=g['time_us']
        elapsed(when,when)
        if last_time is not None:elapsed(last_time,when)
        last_time=when
        if not warm(g) or identity(g['generation'])<64:
            last_swap=None;continue
        serial=identity(g['serial']);kind=g['kind']
        if type(kind) is not int or kind not in range(10):raise ValueError('Unknown native GUI event')
        if not serial:continue
        if kind==0:
            pickup.setdefault(serial,(when,identity(g['generation'])))
        elif kind==2:paints[serial]=when
        elif kind==3 and serial in paints:paint_ms.append(elapsed(paints.pop(serial),when))
        elif kind==1 and serial not in first_swaps:
            first_swaps[serial]=when
            if last_swap is not None:intervals.append(elapsed(last_swap,when))
            last_swap=when
            if serial in pickup:
                picked,generation=pickup[serial]
                if generation!=identity(g['generation']):raise ValueError('GUI export generation changed')
                swap_ms.append(elapsed(picked,when))
                if serial in exports:
                    if generation!=identity(exports[serial]['source_after']):
                        raise ValueError('Owner and GUI export generations differ')
                    published=exports[serial]['published_us']
                    pickup_ms.append(elapsed(published,picked))
                    total_ms.append(elapsed(published,when))
    if not sources or len(total_ms)<5:raise ValueError('Insufficient warm native source/delivery samples')
    source_fields=('source_queue_ms','source_ready_ms','source_decode_ms','source_handoff_ms',
                   'native_producer_ms','native_prepare_ms','native_submit_ms',
                   'original_capture_ms','original_ack_wait_ms','original_return_ms')
    return dict(schema=2,success=True,original_cadence=original_cadence(turns),warm_sources=len(sources),matched_exports=len(total_ms),
        engine_fps=report['engine_fps'],native_render_fps=report['native_render_fps'],
        visible_native_world_fps=trace['visible_native_world_fps'],
        export_to_gui_pickup_ms=distribution(pickup_ms),gui_pickup_to_qt_swap_ms=distribution(swap_ms),
        export_to_qt_swap_ms=distribution(total_ms),widget_paint_cpu_ms=distribution(paint_ms),
        distinct_export_swap_interval_ms=distribution(intervals),
        owner_turn_phases_ms={k:distribution(v) for k,v in phases.items()},
        source_ms={k:distribution([r[k] for r in sources]) for k in source_fields},
        overwritten_turns=trace['overwritten_turns'],overwritten_gui_events=trace['overwritten_gui_events'],
        limitations=['Warm source generations64+; delivery distributions cover only matched retained tail events. Different sample sets and nested stages must not be added.',
          'Host steady-clock delivery begins after native output publication. Original millisecond durations use a separate coarse clock.',
          'Qt swap on a private hardware display is not monitor scanout, input-to-photon latency, GPU busy time or a sustained performance pass.',
          'Tracing and automation can perturb timings. A separate trace-disabled run is required; unlike workloads do not establish a causal speedup.'])
