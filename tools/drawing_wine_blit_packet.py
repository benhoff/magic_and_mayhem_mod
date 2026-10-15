"""Exact selected emitted packet to worker metadata; no source/EGLownership."""
import struct
from drawing_api_session import sha


def decode(data,submissions=None,workers=None):
    if len(data)<128 or len(data)>64+131072*64 or len(data)%64:raise ValueError('Wine packet envelope')
    h=struct.unpack_from('<16I',data)
    if data[:8]!=b'MNMWPK01' or h[2:7]!=(1,64,0x40209ca7,131072,2048) or any(h[7:]):raise ValueError('Wine packet header')
    rows=list(struct.iter_unpack('<16I',data[64:]));active={};complete=[];unknown=[];entered=0;worker_ids=set()
    if rows[-1][1]!=4 or any(r[1]==4 for r in rows[:-1]):raise ValueError('Wine packet terminal')
    for ordinal,r in enumerate(rows[:-1],1):
        if r[0]!=ordinal or r[1] not in (1,2,3) or not r[4] or any(r[9:]):raise ValueError('Wine packet identity/reserve')
        if r[1]==1:
            entered+=1
            if r[2]!=entered or any(r[7:9]):raise ValueError('Wine packet emission sequence')
            if submissions is not None and r[3] and r[3] not in submissions:raise ValueError('Wine packet unknown submission')
            active[r[2]]=dict(command=r[2],submission=r[3],emit_record=r[0],emit_thread=r[4],lifetime_prefix=r[6])
        elif r[8]==1:
            if r[2] or r[3] or r[6] or not r[7]:raise ValueError('Wine packet unknown fields')
            unknown.append(dict(phase=r[1],worker=r[7],record=r[0]))
        else:
            c=active.get(r[2])
            if r[8] or r[6] or c is None or c['submission']!=r[3] or not r[7]:raise ValueError('Wine packet changed/unpaired binding')
            if workers is not None and r[7] not in workers:raise ValueError('Wine packet unknown worker')
            if r[1]==2:
                if 'worker' in c or r[7] in worker_ids:raise ValueError('Wine packet duplicate worker')
                worker_ids.add(r[7]);c.update(worker=r[7],worker_thread=r[4],worker_entry_record=r[0])
            else:
                if c.get('worker')!=r[7] or c.get('worker_thread')!=r[4]:raise ValueError('Wine packet worker return identity')
                c['worker_return_record']=r[0];complete.append(c);active.pop(r[2])
    t=rows[-1]
    if t[0]!=len(rows) or t[2] not in (1,2) or t[3]!=entered or t[8]!=len(active) or t[6] or t[7] or any(t[9:]):raise ValueError('Wine packet terminal counts')
    return dict(schema=1,success=True,journal_sha256=sha(data),bindings=complete,pending_commands=list(active.values()),unknown_worker_events=unknown,
                complete_packet_binding=not active and not unknown,unbound_submissions=sum(not c['submission'] for c in complete),
                source_storage_owned=False,egl_temporal_assignment_performed=False,complete_presented_journey=False)
