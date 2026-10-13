"""Bounded diagnostic mapped RGB565 snapshots, with exact metadata binding."""
import struct
import zlib
from drawing_api_session import sha


def decode(data, directory):
    if len(data)<576 or (len(data)-64)%256 or len(data)>64+16384*256:
        raise ValueError('Source map envelope/closure')
    h=struct.unpack_from('<16I',data)
    if data[:8]!=b'MNMWMS01' or h[2:]!=(1,256,0x40209ca7,16384,16,2,4,0x80000000,116,3635200,40,40,400,280):
        raise ValueError('Source map header')
    rows=list(struct.iter_unpack('<64I',data[64:]));samples=[];seen=set();attempts=[0,0];payloads=[]
    for ordinal,r in enumerate(rows[:-1],1):
        if r[0]!=ordinal or r[1]!=1 or not r[2] or r[2] in seen or not r[4] or r[10]>7 or r[34]>4 or r[35]>2 or any(r[39:]):
            raise ValueError('Source map sample identity/status/reserved')
        seen.add(r[2])
        if r[17]<r[16] or (r[10]==7 and (not r[7] or r[7]==r[18])):
            raise ValueError('Source map lifecycle')
        if r[37]:
            if r[37]!=0x80000000 or not r[35] or r[31]!=attempts[r[35]-1]+1 or r[31]>16:
                raise ValueError('Source map attempt/read flags')
            attempts[r[35]-1]+=1
            if r[34]!=(3 if r[12]&0x80000000 else 4) or (not r[12]&0x80000000 and r[14]&0x80000000):
                raise ValueError('Source map release/query count')
            if r[12]&0x80000000 and (r[10] not in (5,7) or any(r[14:16]) or r[29] or r[30]):
                raise ValueError('Source map failed lease')
        elif r[31]!=(16 if r[10]==3 else 0) or r[34]>2 or any(r[12:16]) or any(r[29:31]):
            raise ValueError('Source map unattempted lease')
        if r[10] in (0,3,4,5,6) and (not r[35] or not r[6] or not r[7] or not r[9] or r[11]&0x80000000):
            raise ValueError('Source map trusted identity')
        if r[35]:
            width,height=(40,40) if r[35]==1 else (400,280)
            if r[19]!=116 or r[20] or r[21] or r[24]&5!=5 or (r[25],r[26],r[27])!=(width,height,1):
                raise ValueError('Source map eligibility')
        if r[10] in (0,5,6) and not r[37]:
            raise ValueError('Source map missing attempt')
        if r[10]==0:
            if r[7]!=r[18] or r[12]&0x80000000 or r[14]&0x80000000 or not r[35] or not r[25]*2<=r[29]<=32768 or r[32]!=r[25]*r[26]*2 or r[33]!=len(payloads)+1:
                raise ValueError('Source map owned payload shape')
            name=f'wine-source-map-{r[33]:06d}.565';raw=(directory/name).read_bytes()
            if len(raw)!=r[32] or zlib.crc32(raw)!=r[36]:
                raise ValueError('Source map payload size/CRC')
            payloads.append(dict(ordinal=r[33],path=name,size=len(raw),sha256=sha(raw),width=r[25],height=r[26]))
        elif r[32] or r[33] or r[36]:
            raise ValueError('Source map refused payload')
        samples.append(dict(submission=r[2],api_entry=r[3],thread=r[4],source_texture=r[6],source_generation=r[7],subresource=r[8],resource=r[9],status=r[10],descriptor_result=r[11],map_result=r[12],unmap_result=r[14],lifetime_before=r[16],lifetime_after=r[17],source_generation_after=r[18],descriptor=list(r[19:29]),row_pitch=r[29],slice_pitch=r[30],shape_attempt=r[31],payload_ordinal=r[33],extra_public_calls=r[34],shape=r[35],query_refusal=r[38]))
    end=rows[-1]
    if end[:7]!=(len(rows),4,end[2],len(samples),len(payloads),*attempts) or end[2] not in (1,2) or any(end[7:]):
        raise ValueError('Source map closure/counts')
    expected={p['path'] for p in payloads}
    if {p.name for p in directory.glob('wine-source-map-*.565')}!=expected:
        raise ValueError('Source map unexpected/missing payload')
    return dict(schema=1,success=True,journal_sha256=sha(data),samples=samples,payloads=payloads,map_attempts=attempts,retained_payloads=len(payloads),extra_public_calls=sum(s['extra_public_calls'] for s in samples),mapped_pointers_exported=False,all_source_storage_owned=False,every_submission_pixels_retained=False,complete_presented_journey=False)


def analyze(directory):
    from drawing_wine_source_descriptor import analyze as descriptors
    result=decode((directory/'wine-source-map.bin').read_bytes(),directory)
    reference=descriptors(directory);sources={s['submission']:s for s in reference['samples']}
    if {s['submission'] for s in result['samples']}!=set(sources):
        raise ValueError('Source map missing/extra submission')
    raw=(directory/'wine-lifetime.bin').read_bytes();states={0:{}};active={}
    for r in struct.iter_unpack('<24I',raw[64:]):
        if r[1]==2:
            kind=1 if r[3]<=2 else 3
            if r[3] in (1,3) and not r[8]&0x80000000 and not r[12]:active[kind,r[10]]=r[11]
            elif r[3] in (2,4) and r[12]==2:active.pop((kind,r[10]),None)
        states[r[0]]=dict(active)
    for s in result['samples']:
        d=sources[s['submission']]
        keys=('api_entry','thread','source_texture','source_generation','subresource')
        if any(s[k]!=d[k] for k in keys) or s['lifetime_before']<d['lifetime_after']:
            raise ValueError('Source map exact descriptor/submission binding')
        if s['status']!=1 and (d['status']!=0 or s['resource']!=d['resource'] or s['descriptor']!=d['descriptor']):
            raise ValueError('Source map resource/descriptor changed')
        before,after=states.get(s['lifetime_before']),states.get(s['lifetime_after'])
        if before is None or after is None or after.get((1,s['source_texture']),0)!=s['source_generation_after']:
            raise ValueError('Source map exact lifecycle state')
        if s['status']!=1 and before.get((1,s['source_texture']),0)!=s['source_generation']:
            raise ValueError('Source map lacks its live generation')
    result.update(descriptor_journal_sha256=reference['journal_sha256'],lifetime_journal_sha256=sha(raw),all_sample_submission_identities_bound=True)
    return result
