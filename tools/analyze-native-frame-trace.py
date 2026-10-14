#!/usr/bin/env python3
"""Analyze bounded public host frame traces without assigning GPU/scanout CPU costs."""
import argparse,hashlib,json,statistics
from pathlib import Path

def stats(values):
    if not values:return {'samples':0}
    a=sorted(values)
    return {'samples':len(a),'mean':statistics.fmean(a),'median':statistics.median(a),'p95':a[min(len(a)-1,int(.95*(len(a)-1)))],'p99':a[min(len(a)-1,int(.99*(len(a)-1)))],'max':a[-1]}

def analyze(report):
    t=report['frame_trace'];assert t['enabled'] and t['schema']==1
    assert all(len(r)==len(t['columns']) for r in t['rows'])
    rows=[dict(zip(t['columns'],r)) for r in t['rows']]
    gui=[dict(zip(t['gui_columns'],r)) for r in t['gui_rows']]
    bounds=['begin_us','world_end_us','commands_end_us','extra_end_us','export_end_us','status_end_us','wait_end_us']
    for r in rows:
        assert len(r)==len(t['columns']) and all(r[a]<=r[b] for a,b in zip(bounds,bounds[1:]))
        assert int(r['frames_after'])-int(r['frames_before']) in (0,1)
    assert all(int(a['turn'])<int(b['turn']) for a,b in zip(rows,rows[1:]))
    warm=lambda r:r['draw_allowed'] and not r['paused'] and int(r['source_after'])>=64
    active=[r for r in rows if warm(r)];sources=[r for r in active if r['source_after']!=r['source_before']]
    intervals=[];kinds={'source':[],'extra':[]};previous=None;previous_count=None
    for r in rows:
        if not warm(r):previous=previous_count=None;continue
        if r['frames_after']==r['frames_before']:continue
        kind='source' if r['source_before']!=r['source_after'] else 'extra'
        end=r['world_end_us'] if kind=='source' else r['extra_end_us']
        if previous is not None and int(r['frames_before'])==previous_count:
            ms=(end-previous)/1000.;assert ms>=0;intervals.append(ms);kinds[kind].append(ms)
        previous=end;previous_count=int(r['frames_after'])
    exports={int(r['export_serial']):r for r in rows if int(r['export_serial'])}
    pickups={};swaps={};swap_events=0;paint_begin={};paint_end={};paint_spans=[];paint_to_swap=[]
    for g in gui:
        serial=int(g['serial'])
        if not serial or not g['draw_allowed'] or g['paused'] or int(g['generation'])<64:continue
        if g['kind']==0:pickups.setdefault(serial,g['time_us'])
        elif g['kind']==1:
            swaps.setdefault(serial,g['time_us']);swap_events+=1
            if serial in paint_end:
                delay=(g['time_us']-paint_end[serial])/1000.;assert delay>=0;paint_to_swap.append(delay)
        elif g['kind']==2:paint_begin[serial]=g['time_us']
        elif g['kind']==3:
            paint_end[serial]=g['time_us']
            if serial in paint_begin:
                delay=(g['time_us']-paint_begin.pop(serial))/1000.;assert delay>=0;paint_spans.append(delay)
        else:raise ValueError('Unknown GUI event')
    pickup_delays=[];swap_delays=[]
    for serial,when in pickups.items():
        if serial in exports and warm(exports[serial]):
            delay=(when-exports[serial]['published_us'])/1000.;assert delay>=0;pickup_delays.append(delay)
    for serial,when in swaps.items():
        if serial in pickups:
            delay=(when-pickups[serial])/1000.;assert delay>=0;swap_delays.append(delay)
    def span(a,b):return stats([(r[b]-r[a])/1000. for r in active])
    out={'success':True,'scope':__doc__,'tail_rows':len(rows),'tail_active_rows':len(active),'tail_sources':len(sources),'overwritten_turns':t['overwritten_turns'],'overwritten_gui_events':t['overwritten_gui_events'],'visible_native_world_fps':t['visible_native_world_fps'],'visible_owner_seconds':t['visible_owner_seconds'],'all_visible_phase_costs':t['visible_timings'],'native_frame_intervals_ms':stats(intervals),'native_frame_intervals_by_kind_ms':{k:stats(v) for k,v in kinds.items()},'intervals_over_16_667_ms':sum(x>16.667 for x in intervals),'intervals_over_20_ms':sum(x>20 for x in intervals),'tail_phase_ms':{name:span(a,b) for name,a,b in [('world','begin_us','world_end_us'),('commands','world_end_us','commands_end_us'),('extra','commands_end_us','extra_end_us'),('export','extra_end_us','export_end_us'),('status','export_end_us','status_end_us'),('wait','status_end_us','wait_end_us')]},'source_ms':{k:stats([r[k] for r in sources]) for k in ['source_queue_ms','source_ready_ms','source_decode_ms','source_handoff_ms','native_producer_ms','native_prepare_ms','native_submit_ms','original_capture_ms','original_ack_wait_ms','original_return_ms']},'export_to_gui_pickup_ms':stats(pickup_delays),'gui_pickup_to_qt_swap_ms':stats(swap_delays),'gpu_widget_paint_cpu_ms':stats(paint_spans),'completed_paint_to_qt_swap_ms':stats(paint_to_swap),'tail_distinct_swapped_exports':len(swaps),'tail_qt_swap_events':swap_events,'whole_report_native_fps':report.get('native_render_fps'),'original_engine_fps':report.get('engine_fps'),'presentation_exports_skipped':report.get('presentation_exports_skipped'),'ordinary_readbacks':{k:report.get(k) for k in ['world_readbacks','native_readbacks','rgba_readbacks','world_pixel_reply_bytes']},'limitations':['Tail interval and GUI samples exclude overwritten/unknown exports and reset across visibility/paused boundaries. Aggregate visible timings cover all traced owner turns, not physical scanout.','Source-ready latency includes native capture/decode/ACK/verified-return wall time. Original tick durations use a different coarse clock; native producer means reconstructed drawing CPU, not original simulation CPU.','CPU wall timings include driver waits. Tracing overhead and distinct live gameplay workloads require separate controls. No full functional/60FPS/original equivalence claim from this analysis.']}
    return out

def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('report',type=Path);p.add_argument('--output',type=Path,required=True);a=p.parse_args();assert not a.output.exists()
    data=a.report.read_bytes();r=analyze(json.loads(data));r.update(input_report=str(a.report.resolve()),input_sha256=hashlib.sha256(data).hexdigest());a.output.write_text(json.dumps(r,indent=2)+'\n');print(a.output)
if __name__=='__main__':main()
