"""Exact public client binding and submitted cache points; never byte ownership."""
import struct
from drawing_api_session import sha


def point(m, thread, pitch):
    if not any(m):
        return dict(known=False, metadata=list(m), canvas=0)
    flags=m[2]
    if m[:2]!=(64,1) or flags & ~255 or not flags & 1 or m[14]!=thread or m[15]:
        raise ValueError('Client point header/thread')
    if sum(bool(flags & x) for x in (8,16,32,128))>1:
        raise ValueError('Client point conflicting association')
    if flags & 16:
        if not flags & 64 or not flags & 2 or flags & 4 or m[5] or not m[6] or not 0<m[7]<=2048 or not 0<m[8]<=2048 or not m[7]<=m[9]<=4096 or pitch!=m[9]*2:
            raise ValueError('Client point pitch/live/cache mismatch')
    elif any(m[6:12]) or flags & 64:
        raise ValueError('Unknown client point has canvas/extent')
    return dict(known=bool(flags & 16), metadata=list(m), canvas=m[6])


def decode(data, submission_observation, submission_data):
    if len(data)<256 or (len(data)-64)%192 or len(data)>64+131072*192:
        raise ValueError('Client memory envelope')
    h=struct.unpack_from('<16I',data)
    if data[:8]!=b'MNMWCM01' or h[2:7]!=(1,192,0x40209ca7,131072,2048) or any(h[7:]):
        raise ValueError('Client memory header')
    if len(submission_data)<256 or (len(submission_data)-64)%192 or sha(submission_data)!=submission_observation['journal_sha256']:
        raise ValueError('Client source submission journal changed')
    source_subs={r[2]:r[13] for r in struct.iter_unpack('<48I',submission_data[64:-192]) if r[1]==1 and r[3]==1}
    submissions={c['call']:c for c in submission_observation['calls'] if c['method']==1}
    rows=list(struct.iter_unpack('<48I',data[64:]));pending={};setters=[];queries=[];seen=set();bindings={};versions=0;ids=set()
    for ordinal,r in enumerate(rows[:-1],1):
        if r[0]!=ordinal or not r[2] or not r[4] or r[3] not in (1,2):
            raise ValueError('Client memory identity')
        if r[14]:ids.add(r[14])
        before=point(r[16:32],r[4],r[13]);after=point(r[32:48],r[4],r[13])
        if r[3]==1:
            if r[1]==1:
                if r[2]!=len(setters)+len(pending)+1 or r[2] in pending or any(r[8:10]) or r[15] or any(r[32:]):
                    raise ValueError('Client setter entry')
                pending[r[2]]=r
            elif r[1]==2:
                e=pending.pop(r[2],None)
                if e is None or r[3:7]!=e[3:7] or r[10:15]!=e[10:15] or r[16:32]!=e[16:32]:
                    raise ValueError('Client setter return identity')
                success=r[8]<0x80000000
                if success:
                    if not r[10]:raise ValueError('Successful setter has no texture')
                    versions+=1
                    if r[15]!=versions:
                        raise ValueError('Client setter version')
                    bindings[(r[10],r[12])]=(r[11],r[13],r[14],r[15])
                elif r[15]:
                    raise ValueError('Failed setter changed binding')
                setters.append(dict(call=r[2],thread=r[4],api_entry=r[5],texture=r[10],generation=r[11],subresource=r[12],pitch=r[13],data=r[14],binding=r[15],result=r[8],last_error=r[9],before=before,after=after))
            else:
                raise ValueError('Client setter phase')
        else:
            c=submissions.get(r[2])
            if r[1]!=3 or r[2] in seen or c is None or any(r[8:10]) or any(r[32:]):
                raise ValueError('Client source query identity')
            if r[12]!=source_subs.get(r[2]):raise ValueError('Client source subresource mismatch')
            if (r[4],r[5],r[6],r[10],r[11])!=(c['thread'],c['api_entry'],c['api_depth'],c['second_object'],c['second_generation']):
                raise ValueError('Client query submission mismatch')
            seen.add(r[2]);binding=bindings.get((r[10],r[12]))
            if binding and r[11] and binding[0]==r[11]:
                if (r[13],r[14],r[15])!=binding[1:]:
                    raise ValueError('Client query stale or changed binding')
                if not r[14] and any(r[16:32]):
                    raise ValueError('Cleared binding has a point')
            elif any(r[13:]):
                raise ValueError('Unknown generation has a binding')
            queries.append(dict(submission=r[2],thread=r[4],api_entry=r[5],texture=r[10],generation=r[11],subresource=r[12],pitch=r[13],data=r[14],binding=r[15],point=before,source_ownership_proved=False))
    t=rows[-1]
    allowed={0,1,2,3,4,5,7,10,14}
    if pending or seen!=set(submissions) or t[0]!=len(rows) or t[1]!=4 or t[2] not in (1,2) or t[3]!=len(setters) or t[5]!=len(queries) or not t[4] or t[10]!=versions or t[14]>2048 or max(ids,default=0)>t[14] or any(t[i] for i in range(48) if i not in allowed):
        raise ValueError('Client memory closure/coverage')
    return dict(schema=1,success=True,journal_sha256=sha(data),setters=setters,queries=queries,
                known_source_points=sum(q['point']['known'] for q in queries),
                unknown_source_points=sum(not q['point']['known'] for q in queries),
                source_ownership_proved=False,source_upload_identity_proved=False,
                private_layout_read=False,complete_presented_journey=False)
