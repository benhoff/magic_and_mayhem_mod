"""Last-observed map addresses at source submission; closed is never ownership."""
import struct
from drawing_storage_point import point_header
from drawing_api_session import sha
from drawing_wine_resource_mapping import decode as resource_decode
from drawing_wine_lifetime import decode as lifetime_decode
from drawing_wine_submission import decode as submission_decode


def decode(data, resource_data, submission_data, lifetime_data):
    if len(data)<256 or (len(data)-64)%192 or len(data)>64+131072*192:
        raise ValueError('Retained map source envelope')
    h=struct.unpack_from('<16I',data)
    if data[:8]!=b'MNMWQS01' or h[2:7]!=(1,192,0x40209ca7,131072,2048) or any(h[7:]):
        raise ValueError('Retained map source header')
    resource=resource_decode(resource_data);lifetime=lifetime_decode(lifetime_data)
    submission=submission_decode(submission_data,lifetime=lifetime_data)
    sources={r[2]:r for r in struct.iter_unpack('<48I',submission_data[64:-192]) if r[1]==1 and r[3]==1}
    rows=list(struct.iter_unpack('<48I',data[64:]))
    from drawing_resource_index import ResourceIndex
    from drawing_lifetime_index import LifetimeIndex
    rs=ResourceIndex(resource_data);ls=LifetimeIndex(lifetime_data)
    seen=set();points=[]
    for ordinal,r in enumerate(rows[:-1],1):
        s=sources.get(r[2]);m=r[24:40]
        if r[0]!=ordinal or r[1]!=3 or s is None or r[2] in seen or r[14]>1 or r[15]>4 or r[23] or any(r[40:]):
            raise ValueError('Retained map source identity/reserve')
        if (r[3],r[4],r[5],r[6],r[7])!=(s[4],s[5],s[11],s[28],s[13]):
            raise ValueError('Retained source submission mismatch')
        seen.add(r[2])
        if not rs.valid_prefix(r[16]) or not rs.valid_prefix(r[17]) or not ls.valid_prefix(r[18]) or not ls.valid_prefix(r[19]) or r[17]<r[16] or r[19]<r[18]:
            raise ValueError('Retained source journal prefix')
        before=rs.candidates(r[16],r[5],r[6],r[7])
        after=rs.candidates(r[17],r[5],r[6],r[7])
        if len(before)!=r[22]:raise ValueError('Retained map candidate count')
        stable_life=ls.generation(r[18],1,r[5])==r[6] and ls.generation(r[19],1,r[5])==r[6]
        if len(before)==1:
            b=before[0]
            if tuple(r[8:15])!=(b['resource'],b['call'],b['interval'],b['data'],b['row'],b['slice'],b['open']):
                raise ValueError('Retained source last map mismatch')
            stable_map=len(after)==1 and after[0]==b
            if r[15] in (0,1) and (not stable_map or not stable_life):raise ValueError('Changed map/lifetime admitted')
            if r[15]==2 and stable_map:raise ValueError('Map-change refusal without changed state')
        elif any(r[8:15]) or r[15] not in (0,3,4):raise ValueError('Unbound map has descriptor')
        if r[15]==3 and len(before)<2:raise ValueError('Ambiguous source without candidates')
        if r[15]==4 and stable_life:raise ValueError('Lifetime refusal without changed state')
        if r[15]!=1:
            if any(m):raise ValueError('Unadmitted source has cache point')
        else:
            flags=m[2]
            point_header(m,r[3])
            if flags & 16:
                if flags & 4 or not flags & 2 or m[5] or not m[6] or not 0<m[7]<=2048 or not 0<m[8]<=2048 or not m[7]<=m[9]<=4096 or r[12]!=m[9]*2 or r[13]<r[12]*m[8]:
                    raise ValueError('Retained source cache extent/live mismatch')
            elif any(m[6:12]):raise ValueError('Unknown retained source has canvas')
        points.append(dict(submission=r[2],thread=r[3],api_entry=r[4],texture=r[5],generation=r[6],subresource=r[7],resource=r[8],map_call=r[9],map_interval=r[10],data=r[11],row_pitch=r[12],slice_pitch=r[13],map_open=bool(r[14]),status=r[15],resource_prefix_before=r[16],resource_prefix_after=r[17],lifetime_prefix_before=r[18],lifetime_prefix_after=r[19],metadata=list(m),canvas=m[6],matched=bool(r[15]==1 and m[2]&16),post_unmap_allocation_owned=False,source_ownership_proved=False))
    t=rows[-1]
    if t[0]!=len(rows) or t[1]!=4 or t[2] not in (1,2) or not t[3] or t[4]!=len(points) or any(t[5:]) or seen!=set(sources):
        raise ValueError('Retained source incomplete closure/coverage')
    return dict(schema=1,success=True,journal_sha256=sha(data),resource_journal_sha256=resource['journal_sha256'],submission_journal_sha256=submission['journal_sha256'],lifetime_journal_sha256=lifetime['journal_sha256'],points=points,matched=sum(p['matched'] for p in points),unknown=sum(not p['matched'] for p in points),matched_closed_maps=sum(p['matched'] and not p['map_open'] for p in points),post_unmap_allocation_owned=False,source_ownership_proved=False,source_upload_identity_proved=False,complete_presented_journey=False)
