"""Strict bounded engine trace decoder; counters and nesting, not frame parity."""
from collections import Counter
import math
import statistics
import struct

EVENTS={1:'pacer-enter',11:'pacer-return',2:'accepted-update',3:'world-tick',
        5:'animation-enter',15:'animation-return',6:'scheduler-enter',16:'scheduler-return',
        7:'maintenance-enter',17:'maintenance-return',8:'world-queue-enter'}

def distribution(values):
    if not values:return None
    values=sorted(values)
    return dict(samples=len(values),mean=statistics.mean(values),median=statistics.median(values),
                p95=values[math.ceil(len(values)*.95)-1],maximum=values[-1])

def decode(path):
    if path.stat().st_size>64+262144*96:raise ValueError('Engine trace size limit')
    raw=path.read_bytes()
    if len(raw)<64 or raw[:8]!=b'MNMCAD01':raise ValueError('Engine trace header')
    h=struct.unpack_from('<16I',raw)
    if h[2:4]!=(1,96) or h[10]!=262144 or any(h[11:]):raise ValueError('Engine trace version/bounds')
    frequency=h[4]|h[5]<<32
    if not frequency or not 1<=h[6]<=h[10] or h[7] or h[8] or not h[9]:raise ValueError('Engine trace incomplete/foreign-thread')
    if len(raw)!=64+h[6]*96:raise ValueError('Engine trace length')
    rows=list(struct.iter_unpack('<24I',raw[64:]));previous=0
    for index,r in enumerate(rows):
        qpc=r[3]|r[4]<<32
        if r[0]!=index+1 or r[1] not in EVENTS or r[2]!=h[9] or qpc<previous:raise ValueError('Engine trace identity/time')
        previous=qpc
    return frequency,rows

def summarize(path):
    frequency,rows=decode(path);counts=Counter(EVENTS[r[1]] for r in rows)
    stack=[];durations={e:[] for e in (1,5,6,7)};animation_changes=0;animation_active=0;frames=[];current=None
    for r in rows:
        event=r[1]
        if event in durations:stack.append(r)
        elif event in (11,15,16,17):
            if not stack or stack[-1][1]!=event-10:raise ValueError('Engine trace unmatched return')
            before=stack.pop()
            if event!=11 and before[13]!=r[13]:raise ValueError('Engine trace return caller')
            durations[event-10].append(((r[3]|r[4]<<32)-(before[3]|before[4]<<32))*1000/frequency)
            if event==15:
                animation_changes+=before[15:24]!=r[15:24]
                animation_active+=before[17]!=0
        if event==2:
            if current:frames.append(current)
            current=dict(sequence=r[6],counter_before=r[5],pacing_ms=r[7],companion=r[8],mode=r[10],
                         qpc=r[3]|r[4]<<32,events=Counter(),queue_counters=[],tick_counters=[],suppress_values=[])
        if current:
            current['events'][EVENTS[event]]+=1
            if event==3:current['tick_counters'].append(r[5])
            if event==8:current['queue_counters'].append(r[5]);current['suppress_values'].append(r[9])
    if stack:raise ValueError('Engine trace incomplete call')
    # Last accepted update has no following boundary and is deliberately excluded.
    active=[f for f in frames if f['events']['world-tick']==1 and f['mode']==0]
    if len(active)<100:raise ValueError('Insufficient counter-advancing updates')
    deltas=Counter((f['tick_counters'][0]-f['counter_before'])&0xffffffff for f in active)
    queue_counts=Counter(f['events']['world-queue-enter'] for f in active)
    animation_counts=Counter(f['events']['animation-enter'] for f in active)
    scheduler_counts=Counter(f['events']['scheduler-enter'] for f in active)
    queue_counter_matches=sum(all(q==f['tick_counters'][0] for q in f['queue_counters']) for f in active if f['queue_counters'])
    consecutive=[]
    for a,b in zip(frames,frames[1:]):
        if a['mode']==b['mode']==0 and (b['sequence']-a['sequence'])&0xffffffff==1:
            consecutive.append((b['qpc']-a['qpc'])*1000/frequency)
    return dict(events=dict(counts),owner_thread=rows[0][2],frequency=frequency,records=len(rows),
        accepted_update_windows=len(frames),advancing_update_windows=len(active),counter_delta_counts=dict(deltas),
        queues_per_advancing_update=dict(queue_counts),animation_calls_per_advancing_update=dict(animation_counts),
        scheduler_calls_per_advancing_update=dict(scheduler_counts),queue_windows_matching_tick=queue_counter_matches,
        pacing_target_counts=dict(Counter(f['pacing_ms'] for f in active)),
        companion_counts=dict(Counter(f['companion'] for f in active)),
        mode_counts=dict(Counter(f['mode'] for f in frames)),
        suppressed_queue_values=dict(Counter(v for f in active for v in f['suppress_values'])),
        active_animation_calls=animation_active,animation_state_changes=animation_changes,
        accepted_update_interval_ms=distribution(consecutive),
        call_wall_ms={EVENTS[e]:distribution(v) for e,v in durations.items()},
        excluded_final_update=True,
        scope='Observed pinned call sites only, unchanged pacing and simulation. Eight global ANI controllers, not all creature/effect animations. No render-only invocation, independent simulation clock, pixel equivalence or uninstrumented speed guarantee.')
