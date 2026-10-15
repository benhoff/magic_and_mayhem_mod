"""Admit selected opcode42worker metadata without assigning source or EGLframes."""
import struct
from drawing_api_session import sha


def decode(data,lifetime):
    from drawing_wine_lifetime import decode as decode_lifetime
    decode_lifetime(lifetime);states={0:{}};active={}
    for row in struct.iter_unpack('<24I',lifetime[64:]):
        if row[1]==2:
            kind=1 if row[3]<=2 else 3
            if row[3] in (1,3) and not row[8]&0x80000000 and not row[12]:active[kind,row[10]]=row[11]
            elif row[3] in (2,4) and row[12]==2:active.pop((kind,row[10]))
        states[row[0]]=dict(active)
    if len(data)<256 or len(data)>64+131072*192 or (len(data)-64)%192:raise ValueError('Wine worker envelope')
    h=struct.unpack_from('<16I',data)
    if data[:8]!=b'MNMWBK01' or h[2:7]!=(1,192,0x40209ca7,131072,42) or any(h[7:]):raise ValueError('Wine worker header')
    rows=list(struct.iter_unpack('<48I',data[64:]));pending={};calls=[];entered=0
    if rows[-1][1]!=4 or any(r[1]==4 for r in rows[:-1]):raise ValueError('Wine worker terminal')
    for ordinal,r in enumerate(rows[:-1],1):
        if r[0]!=ordinal or r[1] not in (1,2) or r[3]!=42 or not r[4] or any(r[34:]):raise ValueError('Wine worker identity/reserve')
        state=states.get(r[7])
        if state is None or (r[9],r[11])!=(state.get((1,r[8]),0),state.get((1,r[10]),0)):raise ValueError('Wine worker lifetime generation')
        if r[1]==1:
            entered+=1
            if r[2]!=entered or r[6]:raise ValueError('Wine worker entry')
            pending[r[2]]=r
        else:
            entry=pending.pop(r[2],None)
            if entry is None or entry[3:5]!=r[3:5] or entry[7:]!=r[7:]:raise ValueError('Wine worker changed/unpairedreturn')
            calls.append(dict(call=r[2],thread=r[4],lifetime_prefix=r[7],destination=r[8],destination_generation=r[9],source=r[10],source_generation=r[11],destination_subresource=r[12],source_subresource=r[13],destination_box=list(r[14:20]),source_box=list(r[20:26]),flags=r[26],effects=list(r[27:33]),filter=r[33],last_error=r[6],source_submission_identity_bound=False))
    t=rows[-1]
    if pending or t[0]!=len(rows) or t[2] not in (1,2) or t[3]!=entered or any(t[6:]):raise ValueError('Wine worker incomplete/count')
    return dict(schema=1,success=True,journal_sha256=sha(data),calls=calls,threads=len({c['thread'] for c in calls}),
                unknown_destination_generations=sum(not c['destination_generation'] for c in calls),
                unknown_source_generations=sum(c['source'] and not c['source_generation'] for c in calls),
                source_storage_owned=False,source_submission_identity_bound=False,
                egl_temporal_assignment_performed=False,complete_presented_journey=False)
