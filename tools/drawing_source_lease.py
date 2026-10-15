"""BoundedAPIreturnread-onlylease;notasyncEGLsourceownership."""
import struct
from drawing_api_session import sha


def decode(data):
    if len(data)<192 or len(data)>64+4097*128 or (len(data)-64)%128:raise ValueError('Sourcelease envelope')
    h=struct.unpack_from('<16I',data)
    if data[:8]!=b'MNMSLS01' or h[2:6]!=(1,128,0x40209ca7,4096) or any(h[6:]):raise ValueError('Sourcelease header')
    rows=list(struct.iter_unpack('<32I',data[64:]));pending={};leases=set();calls=[]
    if rows[-1][1]!=4 or any(r[1]==4 for r in rows[:-1]):raise ValueError('Sourcelease terminal')
    for ordinal,r in enumerate(rows[:-1],1):
        if r[0]!=ordinal or r[1] not in (1,2) or not r[2] or not r[3] or any(r[8:16]):raise ValueError('Sourcelease identity/reserve')
        if r[1]==1:
            if r[2] in pending:raise ValueError('Duplicateleaseentry')
            accepted=bool(r[6]&0x80000000)
            if accepted:
                if r[7]!=(r[6]&0x7fffffff) or not r[7] or r[7] in leases or r[16:18]!=(64,1) or r[18]!=19 or r[21] or r[29]:raise ValueError('Leaseacceptedmetadata')
                leases.add(r[7])
            elif r[6] not in (1,2,3,4) or r[7]:raise ValueError('Leaserefusalmetadata')
            pending[r[2]]=r
        else:
            entry=pending.pop(r[2],None)
            if entry is None or r[3]!=entry[3] or r[5]!=entry[5] or r[7]!=entry[7]:raise ValueError('Leasechanged/unpairedreturn')
            if r[7] and r[6] or not r[7] and r[6]!=2:raise ValueError('Leasereleasefailed')
            calls.append(dict(api_entry=r[2],thread=r[3],lease=r[7],begin_result=entry[6],release_result=r[6],before=list(entry[16:32]),after=list(r[16:32]),api_interval_readonly=bool(r[7]),async_egl_completion_covered=False))
    t=rows[-1]
    if pending or t[0]!=len(rows) or t[2] not in (1,2) or any(t[3:]):raise ValueError('Sourceleaseincompleteclosure')
    return dict(schema=1,success=True,journal_sha256=sha(data),calls=calls,accepted_leases=len(leases),refused_leases=sum(not c['lease'] for c in calls),async_egl_completion_covered=False,complete_presented_journey=False)
