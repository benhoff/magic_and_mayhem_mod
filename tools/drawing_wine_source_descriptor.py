"""Public diagnostic descriptor queries; no source bytes or ownership proof."""
import struct
from drawing_api_session import sha


def decode(data):
    if len(data)<384 or (len(data)-64)%160 or len(data)>64+16384*160:
        raise ValueError('Source descriptor envelope')
    h=struct.unpack_from('<16I',data)
    if data[:8]!=b'MNMWSD01' or h[2:8]!=(1,160,0x40209ca7,16384,2048,2) or any(h[8:]):
        raise ValueError('Source descriptor header')
    rows=list(struct.iter_unpack('<40I',data[64:]));samples=[];seen=set()
    for ordinal,r in enumerate(rows[:-1],1):
        if r[0]!=ordinal or r[1]!=1 or not r[2] or r[2] in seen or not r[4] or r[10]>4 or r[26]>2 or any(r[27:]):
            raise ValueError('Source descriptor sample identity/status')
        seen.add(r[2])
        if r[14]<r[13] or (r[10]==0 and (not r[6] or not r[7] or not r[9] or r[7]!=r[15] or r[11]&0x80000000 or r[26]!=2)):
            raise ValueError('Source descriptor generation/query')
        if r[10]==1 and (r[9] or r[11] or r[12] or r[26] or any(r[16:26])):
            raise ValueError('Source descriptor untrusted query')
        if r[10]==2 and (r[9] or r[26]!=1 or any(r[16:26])):
            raise ValueError('Source descriptor null resource')
        if r[10]==3 and (r[26]!=2 or not r[11]&0x80000000 or any(r[16:26])):
            raise ValueError('Source descriptor failed query')
        if r[10]==4 and (not r[7] or r[7]==r[15]):
            raise ValueError('Source descriptor changed generation')
        samples.append(dict(submission=r[2],api_entry=r[3],thread=r[4],tick=r[5],source_texture=r[6],source_generation=r[7],subresource=r[8],resource=r[9],status=r[10],query_result=r[11],query_error=r[12],lifetime_before=r[13],lifetime_after=r[14],source_generation_after=r[15],descriptor=list(r[16:26]),extra_public_queries=r[26]))
    end=rows[-1]
    if end[0]!=len(rows) or end[1]!=4 or end[2] not in (1,2) or end[3]!=len(samples) or any(end[4:]):
        raise ValueError('Source descriptor closure')
    return dict(schema=1,success=True,journal_sha256=sha(data),samples=samples,known_descriptors=sum(s['status']==0 for s in samples),extra_public_queries=sum(s['extra_public_queries'] for s in samples),private_layout_read=False,source_pixels_read=False,source_ownership_proved=False,complete_presented_journey=False)


def analyze(directory):
    from drawing_wine_submission import decode as decode_submissions
    result=decode((directory/'wine-source-descriptor.bin').read_bytes())
    raw=(directory/'wine-submission.bin').read_bytes();lifetime=(directory/'wine-lifetime.bin').read_bytes()
    calls={c['call']:c for c in decode_submissions(raw,lifetime=lifetime)['calls'] if c['method']==1}
    entries={r[2]:r for r in struct.iter_unpack('<48I',raw[64:-192]) if r[1]==1 and r[3]==1}
    from drawing_lifetime_index import LifetimeIndex
    lifetimes=LifetimeIndex(lifetime)
    if {s['submission'] for s in result['samples']}!=set(calls):
        raise ValueError('Source descriptor missing/extra submission samples')
    for s in result['samples']:
        c=calls[s['submission']]
        if (s['api_entry'],s['thread'],s['source_texture'],s['source_generation'])!=(c['api_entry'],c['thread'],c['second_object'],c['second_generation']):
            raise ValueError('Source descriptor submission/source identity differs')
        if s['lifetime_before']<c['lifetime_prefix']:
            raise ValueError('Source descriptor lifetime prefix precedes submission')
        if s['subresource']!=entries[s['submission']][13]:
            raise ValueError('Source descriptor subresource differs')
        if not lifetimes.valid_prefix(s['lifetime_before']) or not lifetimes.valid_prefix(s['lifetime_after']) or lifetimes.generation(s['lifetime_after'],1,s['source_texture'])!=s['source_generation_after']:
            raise ValueError('Source descriptor exact lifetime state differs')
        if s['status']!=1 and lifetimes.generation(s['lifetime_before'],1,s['source_texture'])!=s['source_generation']:
            raise ValueError('Source descriptor query lacks its live generation')
    result.update(submission_journal_sha256=sha(raw),lifetime_journal_sha256=sha(lifetime),all_sample_submission_identities_bound=True)
    return result
