"""External source planes copied while a diagnostic worker lease holds protection."""
import struct
from drawing_api_session import sha
from drawing_source_lease import decode as decode_leases
from drawing_source_worker_lease import decode as decode_workers


def decode(data):
    if len(data)<256 or (len(data)-64)%192 or len(data)>64+2049*192:raise ValueError('Held snapshot envelope')
    h=struct.unpack_from('<16I',data)
    if data[:8]!=b'MNMSHB01' or h[2] not in (1,2) or h[3:8]!=(192,0x40209ca7,2048,1024,1073741824) or any(h[8:]):raise ValueError('Held snapshot header')
    rows=list(struct.iter_unpack('<48I',data[64:]));pending={};frames=[];planes=set();leases=set()
    for ordinal,r in enumerate(rows[:-1],1):
        if r[0]!=ordinal or r[1] not in (1,2) or not r[2] or not r[3] or r[7] or not r[10] or any(r[14:16]) or h[2]==1 and r[13]:raise ValueError('Held snapshot identity/status')
        before,after=r[16:32],r[32:48]
        if before!=after or before[:3]!=(64,1,19) or before[5] or not before[6] or before[7:10]!=(800,600,800) or h[2]==1 and before[12]>1 or h[2]==2 and bool(before[12]>1)!=bool(r[13]) or before[13] or before[15]:raise ValueError('Held snapshot source metadata')
        if r[8]!=len(planes)+1 or r[9]!=960000:raise ValueError('Held snapshot plane extent/ordinal')
        planes.add(r[8])
        if r[1]==1:
            if pending or r[10] in leases or r[11] or r[12]:raise ValueError('Held snapshot acquisition')
            leases.add(r[10]);pending[r[2]]=r
        else:
            entry=pending.pop(r[2],None)
            if not entry or r[3]!=entry[3] or r[5:7]!=entry[5:7] or r[10]!=entry[10] or before[6]!=entry[22] or not r[11] or r[11]!=r[12]:raise ValueError('Held snapshot release/worker completion')
        frames.append(dict(api_entry=r[2],phase=r[1],thread=r[3],status=0,oracle=r[8],bytes=r[9],lease=r[10],commands=r[11],completed=r[12],before=list(before),after=list(after),source_storage_owned=False,pending_admission=r[13]))
    t=rows[-1]
    if pending or t[0]!=len(rows) or t[1]!=4 or t[2] not in (1,2) or any(t[3:8]) or t[8]!=len(planes) or t[9]!=960000*len(planes) or any(t[10:]):raise ValueError('Held snapshot closure')
    return dict(schema=1,success=True,wire_version=h[2],journal_sha256=sha(data),frames=frames,admitted_planes=len(planes),stable_snapshot_pairs=len(leases),original_pixels_used_as_native_inputs=False,source_storage_owned=False,hardware_completion_covered=False,complete_presented_journey=False)


def analyze(directory):
    result=decode((directory/'source-held-snapshot.bin').read_bytes())
    leases={c['api_entry']:c for c in decode_leases((directory/'api-source-lease.bin').read_bytes())['calls']}
    workers={c['api_entry']:c for c in decode_workers((directory/'api-source-worker-lease.bin').read_bytes())['calls']}
    pairs={};admissions=set()
    for f in result['frames']:
        a,w=leases.get(f['api_entry']),workers.get(f['api_entry'])
        if not a or not w or a['lease']!=f['lease'] or w['lease']!=f['lease'] or a['thread']!=f['thread'] or w['owner']!=f['thread']:raise ValueError('Held snapshot lease/API identity')
        if f['phase']==2 and f['commands']!=len(w['commands']):raise ValueError('Held snapshot command identity')
        if f['pending_admission']:
            from drawing_source_pending_admission import decode as admission
            number=f['pending_admission'];name=f"source-pending-admission-{number:04d}.bin"
            record=admission((directory/name).read_bytes());m=f['before']
            if number in admissions or (record['lease'],record['source_canvas'],record['thread'],record['sequence'],record['queue'],record['pending_frames'])!=(f['lease'],m[6],f['thread'],m[3],m[4],m[12]):raise ValueError('Held snapshot pending admission identity')
            admissions.add(number);f['pending_admission_sha256']=record['journal_sha256']
        p=directory/f"source-held-oracle-{f['oracle']:06d}.565";raw=p.read_bytes()
        if len(raw)!=960000:raise ValueError('Held snapshot plane extent')
        f.update(path=p.name,sha256=sha(raw))
        if f['phase']==1:pairs[f['lease']]=f['sha256']
        elif pairs[f['lease']]!=f['sha256']:raise ValueError('Protected source bytes changed')
    if result['wire_version']==2 and len(list(directory.glob('source-pending-admission-*.bin')))!=len(admissions):raise ValueError('Unbound pending source admissions')
    return result
