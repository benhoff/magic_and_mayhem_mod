"""Read a bounded recorder refusal snapshot; never grant source admission."""
import hashlib
import struct


def decode_refusal(data):
    if len(data) < 64 or len(data) > 64+128*32 or (len(data)-64) % 32:
        raise ValueError('Session closure refusal extent')
    h = struct.unpack_from('<16I', data)
    if (data[:8] != b'MNMPSC01' or h[2:6] != (1,64,0x40209ca7,128)
            or h[6] != (len(data)-64)//32 or not h[7]
            or h[13] not in (0,1) or h[14] != 120 or h[15] not in (0,1)):
        raise ValueError('Session closure refusal header')
    frames = []
    previous = 0
    for row in struct.iter_unpack('<8I', data[64:]):
        slot,tag,owner,primitive,caller,stack,kind,obj = row
        if (not previous < slot <= 128 or tag not in {*range(61),100}
                or not owner or primitive not in (0,1) or not stack
                or caller != 0xffffffff and caller >= 0x1c0000):
            raise ValueError('Session closure refusal frame')
        previous = slot
        frames.append(dict(slot=slot,tag=tag,owner=owner,primitive=bool(primitive),
                           caller_rva=None if caller==0xffffffff else caller,
                           stack=stack,reserved=stack==0xffffffff,
                           record_kind=kind,object=obj))
    return dict(schema=1,sha256=hashlib.sha256(data).hexdigest(),
                thread=h[7],tick=h[8],sequence=h[9],queues=h[10],bytes=h[11],
                failures=h[12],world_open=bool(h[13]),queue_limit=h[14],
                journal_failed=bool(h[15]),frames=frames,
                source_closure_accepted=False,
                scope='One requested observer-state snapshot; local pointers and thread IDs only. '
                      'No frame retirement, alias/ownership proof or source/session acceptance.')
