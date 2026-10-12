"""Checked command completion for diagnostic source leases; hardware completion stays open."""
import struct
from drawing_api_session import sha


def decode(data):
    if len(data)<128 or (len(data)-64)%64 or len(data)>64+16385*64:
        raise ValueError('Worker lease envelope')
    h=struct.unpack_from('<16I',data)
    if data[:8]!=b'MNMSWL01' or h[2:8]!=(1,64,0x40209ca7,16384,2048,5000) or any(h[8:]):
        raise ValueError('Worker lease header')
    rows=list(struct.iter_unpack('<16I',data[64:]));active=None;calls=[];leases=set();commands=set()
    for ordinal,r in enumerate(rows,1):
        if r[0]!=ordinal or any(r[11:]):raise ValueError('Worker lease ordinal/reserve')
        phase,api,lease,thread,tick,command,worker,count,completed,unknown=r[1:11]
        if phase==5:
            if ordinal!=len(rows) or active or api not in (1,2) or any(r[3:]):raise ValueError('Worker lease terminal')
            continue
        if ordinal==len(rows) or unknown or not api or not lease or not thread:raise ValueError('Worker lease identity/closure')
        if phase==1:
            if active or lease in leases or command or worker or count or completed:raise ValueError('Worker lease begin')
            leases.add(lease);active=dict(api_entry=api,lease=lease,owner=thread,commands={},begin_tick=tick)
        else:
            if not active or active['api_entry']!=api or active['lease']!=lease:raise ValueError('Worker lease API changed')
            owned=active['commands']
            if phase==2:
                if thread!=active['owner'] or not command or command in commands or worker:raise ValueError('Worker lease emission')
                commands.add(command);owned[command]=None
            elif phase==3:
                if command not in owned or owned[command] is not None or not worker:raise ValueError('Worker lease completion')
                owned[command]=dict(worker=worker,thread=thread,tick=tick)
            elif phase==4:
                if thread!=active['owner'] or command or worker or not owned or any(v is None for v in owned.values()):raise ValueError('Worker lease premature release')
            else:raise ValueError('Worker lease phase')
            if count!=len(owned) or completed!=sum(v is not None for v in owned.values()):raise ValueError('Worker lease counters')
            if phase==4:
                active['release_ready_tick']=tick;calls.append(active);active=None
    if rows[-1][1]!=5:raise ValueError('Missing worker lease terminal')
    return dict(schema=1,success=True,journal_sha256=sha(data),calls=calls,accepted_leases=len(leases),completed_commands=len(commands),hardware_completion_covered=False,complete_presented_journey=False)
