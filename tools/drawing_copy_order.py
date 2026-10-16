"""Normalize observed driver COPY order without importing original pixels.

Only COPY runs separated by recorder-only checkpoints are eligible. API entry,
return and next-call producer bounds must identify each member uniquely. Other
producer families are barriers. Source snapshots in a run bind to the exact driver command that reads them,
rather than to the wrapper return count.
"""
from collections import defaultdict
from hashlib import sha256
import struct


def normalize(raw, records, point_raw, original, snapshots):
    points=defaultdict(dict)
    for row in struct.iter_unpack('<48I',point_raw[64:-192]):
        points[row[2]][row[1]]=row
    by_thread=defaultdict(list)
    for call in original['observation']['calls']:
        by_thread[call['thread']].append(call)
    copies=defaultdict(list)
    for record,_ in records:
        if record[2]==6:copies[record[3],record[4]].append(record[1])
    workers=defaultdict(list)
    samples={s['submission']:s for s in original['wine_source_map_observation']['samples']}
    packets={p['worker']:p for p in original['wine_blit_packet_observation']['bindings']}
    for worker in original['wine_blit_worker_observation']['calls']:
        packet=packets[worker['call']];sample=samples[packet['submission']]
        workers[sample['api_entry']].append(worker)
    bound={}
    for thread_calls in by_thread.values():
        thread_calls.sort(key=lambda c:c['entry'])
        for index,call in enumerate(thread_calls):
            if call['method'] not in (5,7) or not call['related'] or call['hresult']:continue
            phases=points[call['entry']];entry,returned=phases[1],phases[3]
            end=points[thread_calls[index+1]['entry']][1][19] if index+1<len(thread_calls) else len(records)
            choices=[s for s in copies[entry[22],entry[38]] if returned[19]<s<=end]
            if len(choices)!=1:raise ValueError('COPY API return/next-call bounds are ambiguous')
            sequence=choices[0]
            if sequence in bound:raise ValueError('COPY has multiple API bindings')
            observed=workers.get(call['entry'],[])
            if not observed:continue  # CPU-only/unsupported driver routes stay barriers.
            worker=min(observed,key=lambda w:w['call']);record=records[sequence-1][0]
            sx,sy,right,bottom=record[10:14];x,y=record[8:10]
            flags=0x41000000 if record[14]==0xffffffff else 0x41008000
            if (worker['source_box']!=[sx,sy,right,bottom,0,1] or
                worker['destination_box']!=[x,y,x+right-sx,y+bottom-sy,0,1] or
                worker['flags']!=flags or worker['effects']!=[0]*6 or worker['filter']!=1 or
                worker['source_subresource'] or worker['destination_subresource']):continue
            bound[sequence]=dict(api_entry=call['entry'],worker=worker['call'])
    offsets=[];cursor=64
    for _ in records:
        offsets.append(cursor);cursor+=struct.unpack_from('<I',raw,cursor)[0]
    if cursor!=len(raw):raise ValueError('COPY wire extent changed')
    output=bytearray(raw);changes=[];reads=[]
    queries=[(snapshot,snapshot['sequence']) for snapshot in snapshots]
    def reorder(run,end):
        if not run:return
        ordered=sorted(run,key=lambda s:bound[s]['worker'])
        for snapshot,prefix in queries:
            if not run[0]-1<=prefix<=end:continue
            candidates=workers.get(snapshot.get('api_entry'),[])
            read_worker=min(w['call'] for w in candidates) if candidates else snapshot.get('worker')
            if type(read_worker) is not int:raise ValueError('COPY snapshot lacks an exact driver read')
            completed=[slot for slot,source in zip(run,ordered) if bound[source]['worker']<read_worker]
            native_prefix=max(completed,default=run[0]-1)
            if native_prefix!=prefix:
                snapshot['original_sequence']=prefix;snapshot['sequence']=native_prefix
                reads.append(dict(original_sequence=prefix,native_sequence=native_prefix,canvas=snapshot['canvas'],read_worker=read_worker))
        for target,source in zip(run,ordered):
            if target==source:continue
            record,payload=records[source-1]
            if payload or record[0]!=96:raise ValueError('COPY carries unexpected payload')
            new=list(record);new[1]=target;struct.pack_into('<24I',output,offsets[target-1],*new)
            changes.append(dict(native_sequence=target,original_sequence=source,**bound[source]))
    run=[]
    for record,_ in records:
        sequence,operation=record[1:3]
        if operation==6 and sequence in bound:run.append(sequence)
        elif operation!=10:reorder(run,sequence-1);run=[]
    reorder(run,len(records))
    return bytes(output),dict(schema=1,input_sha256=sha256(raw).hexdigest(),normalized_sha256=sha256(output).hexdigest(),bound_driver_copies=len(bound),moved_copies=changes,rebound_reads=reads,
        original_pixels_used_as_native_inputs=False,scope='Exact driver-ordered COPY runs and command-bound source reads only; CPU/other producers remain barriers. No global ordering or source ownership claim.')
