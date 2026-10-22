"""Exact source producer queue points; presentation/action ownership is separate."""
import importlib.util,struct
from pathlib import Path
from drawing_api_session import sha
from engine_cadence import decode as decode_cadence
from drawing_engine_windows import analyze as analyze_windows
ROOT=Path(__file__).resolve().parents[1]


def decode(data):
    if len(data)<160 or (len(data)-64)%96 or len(data)>64+257*96:raise ValueError('Producer update envelope')
    h=struct.unpack_from('<16I',data)
    if data[:8]!=b'MNMDUP01' or h[2:6]!=(1,96,0x40209ca7,256) or not 1<=h[6]<=120 or any(h[7:]):raise ValueError('Producer update header')
    rows=list(struct.iter_unpack('<24I',data[64:]));pending=None;queues=[];events=set();owner=None;frequency=None
    for ordinal,r in enumerate(rows[:-1],1):
        p=r[8:24]
        if r[0]!=ordinal or r[1] not in (1,2) or not r[3] or r[6] not in (1,2) or r[7]!=1 or p[:2]!=(64,1) or not p[2] or p[5]>255 or p[6]!=r[3] or p[7]!=r[6] or not(p[10]|p[11]) or p[12]!=8 or p[13]>1 or any(p[14:]):raise ValueError('Producer update point identity')
        if owner is None:owner=r[3];frequency=p[10:12]
        if r[3]!=owner or p[10:12]!=frequency:raise ValueError('Producer update owner/frequency changed')
        if r[1]==1:
            if queues and r[4]<queues[-1]['producer_after']:raise ValueError('Producer update source prefix regressed')
            if pending or r[2]!=len(queues)+1 or p[2] in events:raise ValueError('Producer update duplicate/nested queue')
            pending=r;events.add(p[2])
        else:
            if not pending or r[2]!=pending[2] or r[3]!=pending[3] or p!=pending[8:24] or r[4]<=pending[4]:raise ValueError('Producer update changed/missing return')
            queues.append(dict(queue=r[2],thread=r[3],producer_before=pending[4],producer_after=r[4],canvas_before=pending[5],canvas_after=r[5],cadence_record=p[2],world_counter=p[3],accepted_sequence=p[4],mode=p[5],caller_id=p[7],qpc=p[8]|p[9]<<32,frequency=p[10]|p[11]<<32,active_world=bool(p[13])));pending=None
    t=rows[-1]
    if pending or t[0]!=len(rows) or t[1:3]!=(4,1) or t[3] or t[4]!=len(queues) or len(queues)!=h[6] or t[5]<(queues[-1]['producer_after'] if queues else 0) or any(t[6:]):raise ValueError('Producer update incomplete closure')
    return dict(schema=1,success=True,journal_sha256=sha(data),queues=queues,queue_count=len(queues),cadence_trace_crosschecked=False,producer_trace_crosschecked=False,all_presentations_in_update_interval_retained=False,complete_presented_journey=False)


def analyze(directory):
    result=decode((directory/'producer-update-points-0000.bin').read_bytes());frequency,rows=decode_cadence(directory/'engine-cadence.bin');cadence={r[0]:r for r in rows};windows=analyze_windows(directory/'engine-cadence.bin');window_by_sequence={w['sequence']:w for w in windows['windows']}
    spec=importlib.util.spec_from_file_location('producer_updates_corpus',ROOT/'tools/inspect-canvas-producers.py');m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);producer_header,records=m.decode((directory/'canvas-producers.bin').read_bytes());entries={r[14]:r for r,_ in records if r[2]==11};returns={r[14]:r for r,_ in records if r[2]==12}
    for q in result['queues']:
        r=cadence.get(q['cadence_record']);w=window_by_sequence.get(q['accepted_sequence']);entry,returned=entries.get(q['queue']),returns.get(q['queue'])
        if not r or r[1]!=8 or r[2]!=q['thread'] or r[5]!=q['world_counter'] or r[6]!=q['accepted_sequence'] or r[10]!=q['mode'] or r[13]!=(0x4fd4b0 if q['caller_id']==1 else 0x4fd559) or r[3]|r[4]<<32!=q['qpc'] or frequency!=q['frequency']:raise ValueError('Producer/cadence exact queue identity')
        if not entry or not returned or not q['producer_before']<entry[1]<returned[1]<=q['producer_after'] or entry[3]!=q['canvas_before'] or returned[3]!=q['canvas_after']:raise ValueError('Producer record boundary changed')
        q['complete_update_window']=w is not None;q['advancing_update_window']=bool(w and w['advancing']);q['update_queue_record_matches']=bool(w and any(x['record']==q['cadence_record'] for x in w['queues']))
        if w and not q['update_queue_record_matches']:raise ValueError('Producer queue absent from complete update')
    if producer_header[2]==4:
        end=records[-1][0];boundary=cadence.get(end[19]);last=result['queues'][-1]
        if not boundary or boundary[1]!=2 or boundary[2]!=end[21] or boundary[5]!=end[16] or boundary[6]!=end[17] or boundary[10]!=end[20] or end[15]!=last['accepted_sequence'] or end[16]!=last['world_counter'] or end[19]<=last['cadence_record']:raise ValueError('Whole update boundary cadence identity')
        result['whole_update_source_boundary']=dict(last_sequence=end[15],next_sequence=end[17],cadence_record=end[19],counter_before=end[16],verified=True)
    result.update(cadence_trace_crosschecked=True,producer_trace_crosschecked=True,bound_advancing_queues=sum(q['advancing_update_window'] for q in result['queues']))
    return result
