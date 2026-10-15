"""Public resource-copy call intervals, never asynchronous upload completion."""
import struct
from drawing_api_session import sha


def decode(data,resources):
    if len(data)<256 or (len(data)-64)%192 or len(data)>64+131072*192:
        raise ValueError('Resource copy envelope')
    h=struct.unpack_from('<16I',data)
    if data[:8]!=b'MNMWCP01' or h[2:6]!=(1,192,0x40209ca7,131072) or any(h[6:]):
        raise ValueError('Resource copy header')
    pending={};calls=[];rows=list(struct.iter_unpack('<48I',data[64:]))
    known_resources={c['returned_resource'] if c['method']==1 else c['object'] for c in resources['calls']}
    known_resources.discard(0)
    for ordinal,r in enumerate(rows[:-1],1):
        if r[0]!=ordinal or r[1] not in (1,2) or not r[2] or not r[3] or r[9] or r[22] not in (0,1,2) or any(r[33:]):
            raise ValueError('Resource copy identity/reserved')
        if r[22]!=1 and any(r[23:29]):raise ValueError('Unknown copy box has coordinates')
        if r[1]==1:
            if r[2] in pending or r[2]!=len(calls)+len(pending)+1 or any(r[7:9]) or any(r[31:33]):
                raise ValueError('Resource copy entry identity')
            pending[r[2]]=r
        else:
            e=pending.pop(r[2],None)
            if e is None or r[3:6]!=e[3:6] or r[10:31]!=e[10:31] or r[31]<r[29] or r[32]<r[30]:
                raise ValueError('Resource copy return identity/prefix')
            calls.append(dict(call=r[2],thread=r[3],api_entry=r[4],api_depth=r[5],begin_tick=e[6],end_tick=r[6],result=r[7],last_error=r[8],
                              destination_resource=r[10],destination_texture=r[11],destination_generation=r[12],source_resource=r[13],source_texture=r[14],source_generation=r[15],
                              destination_subresource=r[16],source_subresource=r[17],destination_origin=list(r[18:21]),flags=r[21],box_status=r[22],source_box=list(r[23:29]) if r[22]==1 else None,
                              resource_prefix_before=r[29],lifetime_prefix_before=r[30],resource_prefix_after=r[31],lifetime_prefix_after=r[32],async_completion_proved=False))
    t=rows[-1]
    if pending or t[0]!=len(rows) or t[1]!=4 or t[2] not in (1,2) or t[3]!=len(calls) or not t[4] or any(t[i] for i in range(48) if i not in (0,1,2,3,4,6)):
        raise ValueError('Resource copy closure')
    # Copy-only resources can be absent from map calls; retain them as unlinked.
    unlinked=sorted({c[k] for c in calls for k in ('source_resource','destination_resource') if c[k] and c[k] not in known_resources})
    return dict(schema=1,success=True,journal_sha256=sha(data),calls=calls,unlinked_map_resources=unlinked,
                source_transfer_identity_bound=False,async_completion_proved=False,source_storage_owned=False,complete_presented_journey=False)
