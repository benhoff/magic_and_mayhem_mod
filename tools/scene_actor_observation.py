"""Strict reader for the pointer-free queue-entry actor observation sidecar."""
import struct

MAGIC={1:b'MNMSAO01',2:b'MNMSAO02'}
END={1:b'MNMSAOE1',2:b'MNMSAOE2'}
BUILD=0x40209ca7
LIMIT=256*1024**2

def require(value,message):
    if not value:raise ValueError(message)

def _crc_table():
    values=[]
    for byte in range(256):
        for _ in range(8):byte=(byte>>1)^(0x82f63b78 if byte&1 else 0)
        values.append(byte)
    return tuple(values)

CRC_TABLE=_crc_table()

def crc32c(raw):
    h=0xffffffff
    for byte in raw:h=(h>>8)^CRC_TABLE[(h^byte)&255]
    return h^0xffffffff

def decode(raw,complete=True):
    require(64<=len(raw)<=LIMIT,'Actor observation extent')
    h=struct.unpack_from('<16I',raw)
    version=h[2]
    require(version in MAGIC,'Actor observation version')
    actor_bytes,roles,ani,bounds,indices,reserved=(256,3,20,54,57,60) if version==1 else (800,12,28,160,172,196)
    require(raw[:8]==MAGIC[version] and h[2:9]==(version,64,BUILD,8192,actor_bytes,80,LIMIT) and not any(h[9:]),'Actor observation header/version')
    at=64;frames=[];owner=None;identity=None;last_generation=0;last_journal=0;terminal=None
    while at<len(raw):
        if raw[at:at+8]==END[version]:
            require(len(raw)-at==64,'Actor terminal extent')
            terminal=struct.unpack_from('<16I',raw,at)
            require(terminal[2:7]==(version,64,len(frames),at,0) and terminal[7]==owner and not any(terminal[8:]),'Actor observation loss/terminal identity')
            at+=64;break
        if len(raw)-at<128:
            require(not complete,'Truncated actor frame header');break
        f=struct.unpack_from('<32I',raw,at)
        require(128<=f[0]<=128+256*actor_bytes+4096*80 and f[1]==len(frames)+1 and f[1]<=8192,'Actor frame envelope/order')
        if f[0]>len(raw)-at:
            require(not complete,'Truncated actor frame');break
        g=f[2]+(f[3]<<32);nonce=f[4]+(f[5]<<32)
        require(g>last_generation and nonce and f[6] and f[9]<=3 and 0<f[10]<=12320
                and f[13]<=f[12]<=256 and f[14]<=4096 and f[16]>0 and f[17]>0
                and [f[18],f[19]] in ([640,480],[800,600]) and f[22]<=3 and f[23]&~3==0
                and not any(f[25:]) and f[0]==128+f[13]*actor_bytes+f[14]*80,'Actor source/frame domain')
        owner=owner or f[6];identity=identity or nonce
        require(owner==f[6] and identity==nonce and (bool(f[23]&1)==bool(f[11])==bool(f[12])),'Actor owner/session/pool identity')
        require((f[23]&2 and last_journal<=f[15]<65536) or (not f[23]&2 and f[15]==0),'Actor gameplay journal identity/order')
        payload=raw[at+128:at+f[0]]
        require(crc32c(payload)==f[24],'Actor frame payload CRC32C')
        actors=[];by_slot={};tokens=set()
        for off in range(0,f[13]*actor_bytes,actor_bytes):
            a=struct.unpack_from('<'+str(actor_bytes//4)+'I',payload,off)
            require(a[0]<f[12] and a[0] not in by_slot and a[3] and a[12]&~((1<<roles)-1)==0 and not any(a[reserved:]),'Actor slot/validity')
            require(all(bool(a[16+r])==bool(a[12]&(1<<r)) for r in range(roles)) and max(a[16:16+roles])<=256*roles,'Actor frame-local tokens')
            require(all(a[bounds+r]<=8192 and a[indices+r]<=a[bounds+r] for r in range(roles) if a[12]&(1<<r)),'Actor SPR inclusive bound')
            by_slot[a[0]]=a;tokens.update(t for t in a[16:16+roles] if t);actors.append(a)
        draws=[];seen=set()
        for off in range(f[13]*actor_bytes,len(payload),80):
            d=struct.unpack_from('<20I',payload,off)
            require(d[0] in by_slot and d[1]<roles and d[2]<f[10] and d[3] in tokens
                    and 40<=d[11]<=1024**2 and d[18]<=1 and d[19]==0,'Actor matched draw envelope')
            actor=by_slot[d[0]];key=d[:3]
            require(key not in seen and d[3]==actor[16+d[1]] and d[17]==actor[indices+d[1]],'Actor role/draw identity')
            seen.add(key);draws.append(d)
        frames.append(dict(version=version,ordinal=f[1],generation=g,nonce=nonce,thread=f[6],tick=f[7],world_counter=f[8],view=f[9],
            queue_count=f[10],pool_epoch=f[11],pool_capacity=f[12],gameplay_sequence=f[15],source_entry_sequence=f[16],
            canvas=f[17],extent=list(f[18:20]),queue_auxiliary=list(f[20:22]),raw_view=f[22],validity=f[23],actors=actors,draws=draws))
        last_generation=g
        if f[23]&2:last_journal=f[15]
        at+=f[0]
    require(not complete or terminal is not None,'Missing actor terminal closure')
    return dict(schema=1,version=version,role_layout=dict(count=roles,ani=ani,bounds=bounds,indices=indices),success=True,frames=frames,owner=owner,nonce=identity,complete=terminal is not None,
                actor_allocation_generations_proved=False,attachment_placement_proved=False,combat_output_proved=False)
