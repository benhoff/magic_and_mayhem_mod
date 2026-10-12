"""Selected buffer getter origins; returned objects are not invented lifetimes."""
import struct
from drawing_api_session import sha


def decode(data,lifetime):
    from drawing_wine_lifetime import decode as decode_lifetime
    decode_lifetime(lifetime)
    states={0:{}};active={}
    for r in struct.iter_unpack('<24I',lifetime[64:]):
        if r[1]==2:
            kind=1 if r[3]<=2 else 3
            if r[3] in (1,3) and not r[8]&0x80000000 and not r[12]:active[kind,r[10]]=r[11]
            elif r[3] in (2,4) and r[12]==2:active.pop((kind,r[10]))
        states[r[0]]=dict(active)
    if len(data)<160 or len(data)>64+131072*96 or (len(data)-64)%96:raise ValueError('Wine buffer envelope')
    h=struct.unpack_from('<16I',data)
    if data[:8]!=b'MNMWBF01' or h[2:6]!=(1,96,0x40209ca7,131072) or any(h[6:]):raise ValueError('Wine buffer header')
    rows=list(struct.iter_unpack('<24I',data[64:]));pending={};calls=[];entered=0
    if rows[-1][1]!=4 or any(r[1]==4 for r in rows[:-1]):raise ValueError('Wine buffer terminal')
    for ordinal,r in enumerate(rows[:-1],1):
        if r[0]!=ordinal or r[1] not in (1,2) or r[3] not in (1,2) or not r[4] or r[6]>32 or bool(r[5])!=bool(r[6]) or not r[10] or any(r[17:]) or r[3]==1 and r[12]:raise ValueError('Wine buffer identity/arguments')
        before=states.get(r[15])
        if before is None or before.get((3,r[10]),0)!=r[11]:raise ValueError('Wine buffer swapchain generation')
        if r[1]==1:
            entered+=1
            if r[2]!=entered or any(r[8:10]) or any(r[13:15]) or r[16]:raise ValueError('Wine buffer entry')
            pending[r[2]]=r
        else:
            entry=pending.pop(r[2],None);after=states.get(r[16])
            if entry is None or r[3:7]!=entry[3:7] or r[10:13]!=entry[10:13] or r[15]!=entry[15] or r[8] not in (0,1) or bool(r[13])!=bool(r[8]) or after is None or r[16]<r[15] or after.get((1,r[13]),0)!=r[14]:raise ValueError('Wine buffer paired return/generation')
            calls.append(dict(call=r[2],method=r[3],thread=r[4],api_entry=r[5],swapchain=r[10],swapchain_generation=r[11],index=r[12],texture=r[13],texture_generation=r[14],entry_lifetime_prefix=r[15],return_lifetime_prefix=r[16],nonnull=bool(r[8]),last_error=r[9]))
    t=rows[-1]
    if pending or t[0]!=len(rows) or t[2] not in (1,2) or t[3]!=entered or any(t[5:7]) or any(t[8:]):raise ValueError('Wine buffer incomplete/count')
    return dict(schema=1,success=True,journal_sha256=sha(data),calls=calls,unknown_texture_returns=sum(c['nonnull'] and not c['texture_generation'] for c in calls),all_internal_lifetimes_observed=False,source_storage_owned=False,egl_temporal_assignment_performed=False,complete_presented_journey=False)
