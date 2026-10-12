"""Strict observed wrapper transitions; numerical identities never prove ownership."""
import struct
from drawing_api_session import sha


def decode(data):
    if len(data)<160 or (len(data)-64)%96 or len(data)>64+131072*96:
        raise ValueError('Storage transition envelope')
    h=struct.unpack_from('<16I',data)
    if data[:8]!=b'MNMCST02' or h[2:7]!=(1,96,0x40209ca7,131072,512) or any(h[7:]):
        raise ValueError('Storage transition header')
    rows=list(struct.iter_unpack('<24I',data[64:]));pending={};calls=returns=epochs=0
    events=[];last_prefix=0;addresses=set()
    for ordinal,r in enumerate(rows[:-1],1):
        if r[0]!=ordinal or r[1] not in (1,2) or r[3]>2 or not r[4] or r[5]>=0x1c0000 or r[6]<last_prefix or r[14]&~3 or any(r[22:]):
            raise ValueError('Storage transition identity/order')
        last_prefix=r[6]
        if not r[14]&2 and any(r[9:14]) or not r[14]&1 and any(r[15:20]):
            raise ValueError('Absent cache/wrapper contains fields')
        for identity in (r[10],r[15],r[16],r[21] if r[3]==2 else 0):
            if identity>512:raise ValueError('Address identity budget')
            if identity:addresses.add(identity)
        if r[1]==1:
            calls+=1
            if r[2]!=calls or r[2] in pending or any(r[20:22]):
                raise ValueError('Storage transition duplicate/malformed entry')
            pending[r[2]]=r
        else:
            before=pending.pop(r[2],None)
            if before is None or r[3:6]!=before[3:6]:
                raise ValueError('Storage transition unmatched return')
            returns+=1
            if r[3]==2:
                epochs+=1
                if r[20]!=epochs:raise ValueError('Missing observed Lock return epoch')
            elif any(r[20:22]):raise ValueError('Non-Lock result/epoch')
        events.append(dict(ordinal=r[0],phase=r[1],call=r[2],tag=r[3],thread=r[4],caller_rva=r[5],
                           producer_prefix=r[6],queue=r[7],failures=r[8],canvas=r[9],
                           cached_address=r[10],extent=list(r[11:14]),wrapper_readable=bool(r[14]&1),
                           cache_known=bool(r[14]&2),interface_addresses=list(r[15:17]),
                           wrapper_extent=list(r[17:20]),lock_return_epoch=r[20],returned_address=r[21]))
    t=rows[-1]
    if t[0]!=len(rows) or t[1]!=4 or t[2:5]!=(calls,returns,len(pending)) or t[5]<last_prefix or t[8]!=len(addresses) or t[9]!=epochs or any(t[10:]):
        raise ValueError('Storage transition terminal')
    if addresses!=set(range(1,t[8]+1)):raise ValueError('Address identity sequence gap')
    return dict(schema=1,success=True,journal_sha256=sha(data),calls=calls,returns=returns,
                open_calls=sorted(pending),closed=not pending,observed_lock_return_epochs=epochs,
                address_identities=len(addresses),events=events,allocation_ownership_proved=False,
                all_lock_calls_observed=False,complete_presented_journey=False)


def bind_producer(observation,records):
    """Bind cache identities to their exact closed producer prefixes."""
    active={};seen=set();cursor=0;known=unknown=lock_checkpoints=0;address_canvases={}
    for event in observation['events']:
        prefix=event['producer_prefix']
        if prefix>len(records):raise ValueError('Transition prefix outside producer journal')
        while cursor<prefix:
            r,_=records[cursor];cursor+=1
            if r[1]!=cursor:raise ValueError('Producer sequence gap')
            if r[2]==1:
                if not r[3] or r[3] in seen:raise ValueError('Reused logical canvas generation')
                active[r[3]]=tuple(r[5:7]);seen.add(r[3])
            elif r[2]==2:
                if r[3] not in active:raise ValueError('Release of absent canvas generation')
                del active[r[3]]
            elif r[2]==10 and r[15]==3:lock_checkpoints+=1
        if event['cache_known']:
            if active.get(event['canvas'])!=tuple(event['extent'][:2]):
                raise ValueError('Transition cache generation/extent absent at prefix')
            known+=1
            if event['cached_address']:address_canvases.setdefault(event['cached_address'],set()).add(event['canvas'])
            if event['phase']==2 and event['tag']==2 and event['cached_address']!=event['returned_address']:
                raise ValueError('Returned Lock address differs from recorder cache')
        else:unknown+=1
    return dict(success=True,known_cache_points=known,unknown_cache_points=unknown,
                source_lock_checkpoints_through_last_transition=lock_checkpoints,
                observed_lock_returns=observation['observed_lock_return_epochs'],
                addresses_seen_on_multiple_canvas_generations={str(k):sorted(v) for k,v in address_canvases.items() if len(v)>1},
                allocation_ownership_proved=False,complete_storage_lifetime_proved=False)
