"""OriginalCPUsourceplanes are externaloracles,neverownednativeinput."""
import struct
from drawing_api_session import sha
from drawing_observed_inputs import regular_bytes


def decode(data):
    if len(data)<256 or len(data)>64+4097*192 or (len(data)-64)%192:raise ValueError('Sourcebytewitness envelope')
    h=struct.unpack_from('<16I',data)
    if data[:8]!=b'MNMSBY01' or h[2:8]!=(1,192,0x40209ca7,4096,1024,1073741824) or any(h[8:]):raise ValueError('Sourcebytewitness header')
    rows=list(struct.iter_unpack('<48I',data[64:]));frames=[];entries={};owners={};planes=set();total=0
    if rows[-1][1]!=4 or any(r[1]==4 for r in rows[:-1]):raise ValueError('Sourcebytewitness terminal')
    for ordinal,r in enumerate(rows[:-1],1):
        if r[0]!=ordinal or r[1] not in (1,2) or not r[2] or not r[3] or r[7]>5 or any(r[10:16]):raise ValueError('Sourcebytewitness identity/status')
        identity=(r[3],r[5],r[6])
        if r[1]==1:owners[r[2]]=identity
        elif owners.get(r[2])!=identity:raise ValueError('Sourcebytewitness changed API owner/generations')
        phases=entries.setdefault(r[2],[])
        if len(phases)!=r[1]-1:raise ValueError('Sourcebytewitness duplicate/missing phase')
        phases.append(r[1])
        if not r[7]:
            before,after=r[16:32],r[32:48]
            if before!=after or before[:2]!=(64,1) or before[2]!=19 or before[5] or before[7:9]!=(800,600) or (before[12]>1 or before[13]) or before[15] or not before[6] or before[9]<800 or not 1<=r[8]<=1024 or r[8] in planes or r[9]!=960000:raise ValueError('Sourcebytewitness admitted source/plane')
            planes.add(r[8]);total+=r[9]
        elif r[8] or r[9]:raise ValueError('Refusedsourcewitness hasplane')
        frames.append(dict(api_entry=r[2],phase=r[1],thread=r[3],status=r[7],oracle=r[8],bytes=r[9],before=list(r[16:32]),after=list(r[32:48]),source_storage_owned=False))
    t=rows[-1]
    if planes!=set(range(1,len(planes)+1)) or any(v!=[1,2] for v in entries.values()) or t[0]!=len(rows) or t[2] not in (1,2) or t[8]!=len(planes) or t[9]!=total or any(t[3:8]) or any(t[10:]) or total>1073741824:raise ValueError('Sourcebytewitness closure/count')
    return dict(schema=1,success=True,journal_sha256=sha(data),frames=frames,admitted_planes=len(planes),refused_points=sum(bool(f['status']) for f in frames),original_pixels_used_as_native_inputs=False,source_storage_owned=False,complete_presented_journey=False)


def analyze(directory):
    result=decode(regular_bytes(directory/'api-source-bytes.bin',64+4097*192))
    for frame in result['frames']:
        if not frame['oracle']:continue
        path=directory/f"api-source-oracle-{frame['oracle']:06d}.565";data=regular_bytes(path,960000)
        if len(data)!=frame['bytes']:raise ValueError('Sourcebytewitness plane extent')
        frame.update(path=path.name,sha256=sha(data))
    expected={f['path'] for f in result['frames'] if f['oracle']}
    if {p.name for p in directory.glob('api-source-oracle-*')}!=expected:raise ValueError('Unbound sourcebytewitness oracle artifacts')
    return result
