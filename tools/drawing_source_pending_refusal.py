"""Pending producer frame identities at a protected-copy refusal; no admission."""
import struct
from drawing_api_session import sha


def decode(data):
    if len(data)<64 or (len(data)-64)%64 or len(data)>64+128*64:
        raise ValueError('Pending refusal envelope')
    h=struct.unpack_from('<16I',data)
    if data[:8]!=b'MNMSPF01' or h[2:6]!=(1,64,0x40209ca7,128) or h[6]!=(len(data)-64)//64 or h[13]!=h[6] or not all(h[7:10]) or h[14] not in (1,2) or h[15]:
        raise ValueError('Pending refusal header')
    frames=[];slots=set()
    for ordinal,r in enumerate(struct.iter_unpack('<16I',data[64:]),1):
        if r[0]!=ordinal or not 1<=r[1]<=128 or r[1] in slots or r[2] not in {*range(56),100} or any(r[i]>1 for i in (4,10,11,12,13,14)) or r[15] or r[5]>=0x1c0000:
            raise ValueError('Pending frame identity/reserve')
        if not r[13] and any(r[7:10]):raise ValueError('Unknown owned record has fields')
        if r[10]!=bool(r[6] and r[6]==h[8]) or r[11]!=bool(r[8] and r[8]==h[8]) or r[12]!=bool(r[9] and r[9]==h[8]):
            raise ValueError('Pending frame numeric source identity')
        slots.add(r[1]);frames.append(dict(slot=r[1],tag=r[2],owner=r[3],primitive=bool(r[4]),original_caller_rva=r[5],object_canvas=r[6],record_kind=r[7],record_canvas_word3=r[8],record_word4=r[9],object_is_source=bool(r[10]),record_word3_equals_source=bool(r[11]),record_word4_equals_source=bool(r[12]),record_available=bool(r[13]),reserved_frame=bool(r[14]),source_overlap_proved=False))
    return dict(schema=1,success=True,journal_sha256=sha(data),lease=h[7],source_canvas=h[8],thread=h[9],tick=h[10],sequence=h[11],queue=h[12],reason=h[14],frames=frames,pending_frames=len(frames),source_copy_admitted=False,source_overlap_proved=False,source_ownership_proved=False)
