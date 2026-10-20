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
    output=bytearray(raw);changes=[];reads=[];snapshot_updates=[]
    queries=[(snapshot,snapshot['sequence']) for snapshot in snapshots]
    def reorder(run,end):
        if not run:return
        ordered=sorted(run,key=lambda s:bound[s]['worker'])
        for snapshot,prefix in queries:
            if not run[0]-1<=prefix<=end:continue
            candidates=workers.get(snapshot.get('api_entry'),[])
            explicit=snapshot.get('worker')
            if candidates:
                if explicit is not None and (type(explicit) is not int or sum(w['call']==explicit for w in candidates)!=1):
                    raise ValueError('COPY snapshot worker disagrees with its API')
                read_worker=min(w['call'] for w in candidates) if explicit is None else explicit
            else:
                read_worker=explicit
            if type(read_worker) is not int or not any(w['call']==read_worker for values in workers.values() for w in values):
                raise ValueError('COPY snapshot lacks an exact driver read')
            completed=[slot for slot,source in zip(run,ordered) if bound[source]['worker']<read_worker]
            native_prefix=max(completed,default=run[0]-1)
            if native_prefix!=prefix:
                snapshot_updates.append((snapshot,prefix,native_prefix))
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
    # Publish source-prefix changes only after every run/read has validated.
    for snapshot,prefix,native_prefix in snapshot_updates:
        snapshot['original_sequence']=prefix;snapshot['sequence']=native_prefix
    return bytes(output),dict(schema=1,input_sha256=sha256(raw).hexdigest(),normalized_sha256=sha256(output).hexdigest(),bound_driver_copies=len(bound),moved_copies=changes,rebound_reads=reads,
        original_pixels_used_as_native_inputs=False,scope='Exact driver-ordered COPY runs and command-bound source reads only; CPU/other producers remain barriers. No global ordering or source ownership claim.')
"""Strict bounded copy request entry/return journal, independent of driver order."""
import struct
from drawing_api_session import sha

TAGS = {7, 8, 32, 33, 37, 38, 39, 41, 42, 43}


def decode(data):
    if len(data) < 192 or (len(data) - 64) % 128 or len(data) > 64 + 65536 * 128:
        raise ValueError('Copy order envelope')
    h = struct.unpack_from('<16I', data)
    if data[:8] != b'MNMDCQ01' or h[2:6] != (1, 128, 0x40209ca7, 65536) or any(h[6:]):
        raise ValueError('Copy order header')
    rows = list(struct.iter_unpack('<32I', data[64:]))
    pending, entries, returns, requests, previous = {}, [], [], 0, 0
    for ordinal, r in enumerate(rows, 1):
        if r[0] != ordinal or r[1] not in (1, 2, 3) or r[6] < previous or any(r[22:24]) or any(r[25:]):
            raise ValueError('Copy order identity/reserved/prefix')
        previous = r[6]
        if r[1] == 3:
            if ordinal != len(rows) or r[2] != requests or pending or r[24] or any(r[3:6]) or any(r[8:22]):
                raise ValueError('Copy order incomplete terminal')
            continue
        if not r[2] or not r[3] or r[4] not in TAGS or not all(r[8:12]) or r[10] > 2048 or r[11] > 2048 or r[12] > 4096 or r[12] and r[12] < r[10] or r[20] >= 0x1c0000:
            raise ValueError('Copy request descriptor/owner')
        record = dict(ordinal=ordinal, request=r[2], owner=r[3], tag=r[4], tick=r[5],
                      producer_prefix=r[6], queue=r[7], destination=r[8], source=r[9],
                      destination_extent_stride=list(r[10:13]), destination_position=list(r[13:15]),
                      source_rectangle=list(r[15:19]), raw_key=r[19], caller_rva=r[20],
                      producer_return_sequence=r[21], active_requests=r[24])
        if r[1] == 1:
            requests += 1
            if r[2] != requests or r[2] in pending or r[21]:
                raise ValueError('Copy request duplicate/out-of-order entry')
            pending[r[2]] = r
            entries.append(record)
        else:
            entry = pending.pop(r[2], None)
            if entry is None or r[3:5] != entry[3:5] or r[8:21] != entry[8:21] or not entry[6] < r[21] <= r[6]:
                raise ValueError('Copy request unmatched/changed return')
            returns.append(record)
        if r[24] != len(pending):
            raise ValueError('Copy request active count')
    if rows[-1][1] != 3:
        raise ValueError('Copy order missing terminal')
    return dict(schema=1, success=True, journal_sha256=sha(data), requests=requests,
                entries=entries, returns=returns, terminal_producer_prefix=rows[-1][6],
                terminal_queue=rows[-1][7], driver_order_proved=False,
                api_request_bindings_proved=False, source_or_physical_ownership_proved=False)
