"""Actual accepted-update windows; counts alone do not establish source ownership."""
from collections import Counter
from engine_cadence import decode_bytes,EVENTS
from drawing_observed_inputs import regular_bytes
from drawing_api_session import sha


def analyze(path):
    raw=regular_bytes(path,64+262144*96)
    frequency,rows=decode_bytes(raw);stack=[];windows=[];current=None;queue_ordinal=0
    for r in rows:
        event=r[1]
        if event in (1,5,6,7):stack.append(r)
        elif event in (11,15,16,17):
            before=stack.pop() if stack else None
            if before is None or before[1]!=event-10 or event!=11 and before[13]!=r[13]:raise ValueError('Update trace unpaired original call')
        if event==2:
            if stack:raise ValueError('Accepted update inside selected call')
            if current:
                if not 0<(r[6]-current['sequence'])&0xffffffff<2**31:raise ValueError('Accepted update sequence duplicate/reversal')
                windows.append(current)
            current=dict(sequence=r[6],counter_before=r[5],mode=r[10],begin_record=r[0],qpc=r[3]|r[4]<<32,thread=r[2],events=Counter(),ticks=[],queues=[])
        if event==8:queue_ordinal+=1
        if current:
            current['events'][EVENTS[event]]+=1
            if event==3:current['ticks'].append(r[5])
            if event==8:
                if r[13] not in (0x4fd4b0,0x4fd559):raise ValueError('Unknown selected World queue caller')
                current['queues'].append(dict(ordinal=queue_ordinal,record=r[0],counter=r[5],caller=r[13],qpc=r[3]|r[4]<<32))
    if stack:raise ValueError('Update trace incomplete original call')
    run=longest=0;previous=None
    for w in windows:
        w['advancing']=len(w['ticks'])==1 and (w['ticks'][0]-w['counter_before'])&0xffffffff==1
        if len(w['ticks'])>1 or len(w['ticks'])==1 and (w['ticks'][0]-w['counter_before'])&0xffffffff!=1:raise ValueError('Update world counter inconsistent')
        if any(q['counter']!=(w['ticks'][0] if w['ticks'] else w['counter_before']) for q in w['queues']):raise ValueError('Queue counter not in observed update')
        contiguous=previous is not None and previous['advancing'] and (w['sequence']-previous['sequence'])&0xffffffff==1 and w['counter_before']==previous['ticks'][0]
        run=run+1 if w['advancing'] and contiguous else 1 if w['advancing'] else 0;longest=max(longest,run);previous=w
        w['events']=dict(w['events'])
    complete_queue_records={q['record'] for w in windows for q in w['queues']}
    excluded_queue_records=[r[0] for r in rows if r[1]==8 and r[0] not in complete_queue_records]
    return dict(schema=1,success=True,journal_sha256=sha(raw),frequency=frequency,owner_thread=rows[0][2],windows=windows,complete_update_windows=len(windows),advancing_update_windows=sum(w['advancing'] for w in windows),non_advancing_update_windows=sum(not w['advancing'] for w in windows),longest_contiguous_advancing_run=longest,excluded_final_update=True,excluded_final_sequence=current['sequence'] if current else None,queue_records_outside_complete_windows=excluded_queue_records,producer_update_identity_bound=False,all_presentations_in_update_interval_retained=False,complete_presented_journey=False)
