"""Strict current-context fence journal; never infers all-context completion."""
import struct
from pathlib import Path
from drawing_api_session import sha


def decode(data, complete=True):
    if len(data)<192 or (len(data)-64)%128 or len(data)>64+131072*128:
        raise ValueError('GPU completion envelope')
    h=struct.unpack_from('<16I',data)
    if data[:8]!=b'MNMWGC01' or h[2:7]!=(1,128,0x40209ca7,131072,1000000000) or any(h[7:]):
        raise ValueError('GPU completion header')
    rows=list(struct.iter_unpack('<32I',data[64:])); result=[]; seen=set(); terminal=None
    for ordinal,r in enumerate(rows,1):
        if r[0]!=ordinal or any(r[18:]):raise ValueError('GPU completion ordinal/reserve')
        if r[1]==4:
            if ordinal!=len(rows) or r[2] not in (1,2) or r[3]!=len(result) or any(r[4:8]) or any(r[11:18]) or r[8]!=sum(c['status']>2 for c in result) or r[9]!=sum(c['status']==1 for c in result) or r[10]!=sum(c['status']==2 for c in result):
                raise ValueError('GPU completion terminal')
            terminal=r[2];continue
        if r[1]!=1 or not r[2] or r[2] in seen or not r[5] or bool(r[3])!=bool(r[4]) or not 1<=r[8]<=8 or r[11]!=(r[7]-r[6])%2**32 or r[12]&~7 or r[13] not in (0,1) or r[14]!=r[13] or r[15] not in (0,1) or r[16:18]!=(1000000000,0):
            raise ValueError('GPU completion identity/status')
        seen.add(r[2]);status=r[8]
        if status==1:
            if any(r[9:11]) or any(r[12:16]):raise ValueError('GPU no-context fields')
        else:
            if not r[9]:raise ValueError('GPU context identity')
            if status!=7 and not r[15]:raise ValueError('GPU context changed')
            if status==7 and r[15]:raise ValueError('GPU context-change status')
            if r[12]!=7 and (r[13] or r[10]) or not r[13] and r[10]:raise ValueError('GPU fence/procedure fields')
            expected=3 if r[12]!=7 else 4 if not r[13] else {0x911a:2,0x911c:2,0x911b:5,0x911d:6}.get(r[10],8)
            if status!=7 and status!=expected:raise ValueError('GPU wait result/status')
        result.append(dict(worker=r[2],command=r[3],submission=r[4],thread=r[5],status=status,context=r[9],wait_result=r[10],duration_ms=r[11],procedures=r[12],fence_created=bool(r[13]),fence_deleted=bool(r[14]),context_unchanged=bool(r[15]),timeout_ns=r[16],same_stream_completion_witnessed=status==2))
    if complete and (terminal is None or any(c['status']>2 for c in result)):
        raise ValueError('GPU incomplete or refused cohort')
    return dict(schema=1,success=terminal is not None and all(c['status']<=2 for c in result),closed=terminal is not None,journal_sha256=sha(data),calls=result,signaled=sum(c['status']==2 for c in result),unwitnessed=sum(c['status']==1 for c in result),all_contexts_completed=False,source_backing_ownership_proved=False)


def analyze(directory):
    from drawing_wine_blit_packet import decode as packets
    directory=Path(directory);gpu=decode((directory/'wine-gpu-completion.bin').read_bytes())
    binding=packets((directory/'wine-blit-packet.bin').read_bytes())
    known={c['worker']:c for c in binding['bindings']};unknown={c['worker'] for c in binding['unknown_worker_events']}
    if binding['pending_commands'] or set(known)&unknown or {c['worker'] for c in gpu['calls']}!=set(known)|unknown:
        raise ValueError('GPU worker cohort incomplete')
    for c in gpu['calls']:
        b=known.get(c['worker'])
        if b and (c['command'],c['submission'],c['thread'])!=(b['command'],b['submission'],b['worker_thread']) or not b and (c['command'] or c['submission']):
            raise ValueError('GPU packet/worker binding')
    gpu.update(bound_commands=len(known),unknown_workers=len(unknown),packet_journal_sha256=binding['journal_sha256'])
    return gpu
