"""Incremental strict v2 passive actor-sidecar reader; no engine calls or writes."""
import hashlib
from pathlib import Path
import struct
from scene_actor_observation import BUILD,LIMIT,crc32c,require


def frame_record(raw,ordinal,owner,nonce,last_generation,last_journal):
    require(len(raw)>=128,'Truncated actor frame header')
    f=struct.unpack_from('<32I',raw)
    require(f[0]==len(raw)and f[1]==ordinal and ordinal<=8192
            and 128<=len(raw)<=128+256*800+4096*80,'Actor frame envelope/order')
    generation=f[2]+(f[3]<<32);identity=f[4]+(f[5]<<32)
    require(generation>last_generation and identity and f[6]and f[9]<=3 and 0<f[10]<=12320
            and f[13]<=f[12]<=256 and f[14]<=4096 and f[16]>0 and f[17]>0
            and list(f[18:20])in([640,480],[800,600])and f[22]<=3 and f[23]&~3==0
            and not any(f[25:])and len(raw)==128+f[13]*800+f[14]*80,'Actor source/frame domain')
    require((owner is None or owner==f[6])and(nonce is None or nonce==identity)
            and bool(f[23]&1)==bool(f[11])==bool(f[12]),'Actor owner/session/pool identity')
    require((f[23]&2 and last_journal<=f[15]<65536)or(not f[23]&2 and f[15]==0),'Actor journal identity/order')
    payload=raw[128:];require(crc32c(payload)==f[24],'Actor frame payload CRC32C')
    actors=[];by_slot={};tokens=set()
    for off in range(0,f[13]*800,800):
        a=struct.unpack_from('<200I',payload,off)
        require(a[0]<f[12]and a[0]not in by_slot and a[3]and a[12]&~4095==0
                and not any(a[196:]),'Actor slot/validity')
        require(all(bool(a[16+r])==bool(a[12]&(1<<r))for r in range(12))
                and max(a[16:28])<=3072,'Actor frame-local tokens')
        require(all(a[160+r]<=8192 and a[172+r]<=a[160+r]for r in range(12)if a[12]&(1<<r)),
                'Actor SPR inclusive bound')
        by_slot[a[0]]=a;tokens.update(v for v in a[16:28]if v);actors.append(a)
    draws=[];seen=set()
    for off in range(f[13]*800,len(payload),80):
        d=struct.unpack_from('<20I',payload,off)
        require(d[0]in by_slot and d[1]<12 and d[2]<f[10]and d[3]in tokens
                and 40<=d[11]<=1024**2 and d[18]<=1 and d[19]==0,'Actor matched draw envelope')
        a=by_slot[d[0]];key=d[:3]
        require(key not in seen and d[3]==a[16+d[1]]and d[17]==a[172+d[1]],'Actor role/draw identity')
        seen.add(key);draws.append(d)
    return dict(version=2,ordinal=f[1],generation=generation,nonce=identity,thread=f[6],tick=f[7],
        world_counter=f[8],view=f[9],queue_count=f[10],pool_epoch=f[11],pool_capacity=f[12],
        gameplay_sequence=f[15],source_entry_sequence=f[16],canvas=f[17],extent=list(f[18:20]),
        queue_auxiliary=list(f[20:22]),raw_view=f[22],validity=f[23],actors=actors,draws=draws,
        raw_record_sha256=hashlib.sha256(raw).hexdigest())


class LiveActorFrames:
    def __init__(self,path,*,keep_all=False):
        self.path=Path(path);self.keep_all=keep_all;self.at=0;self.ordinal=0;self.owner=None;self.nonce=None
        self.generation=0;self.journal=0;self.identity=None;self.frames={};self.complete=False

    def poll(self,through=None):
        require(not self.path.is_symlink(),'Actor observation must not be redirected')
        with self.path.open('rb')as source:
            import os
            stat=os.fstat(source.fileno());identity=(stat.st_dev,stat.st_ino)
            require(self.identity is None or identity==self.identity,'Actor source identity changed')
            self.identity=identity;size=stat.st_size
            require(64<=size<=LIMIT and size>=self.at,'Actor extent/shrink')
            if not self.at:
                header=source.read(64);h=struct.unpack('<16I',header)
                require(header[:8]==b'MNMSAO02'and h[2:9]==(2,64,BUILD,8192,800,80,LIMIT)
                        and not any(h[9:]),'Actor observation v2 header')
                self.at=64
            if self.complete:
                require(size==self.at,'Actor data after terminal closure');return self.frames
            while self.at<size and (through is None or self.generation<through):
                source.seek(self.at);head=source.read(min(128,size-self.at))
                if head[:8]==b'MNMSAOE2':
                    require(size-self.at==64 and len(head)==64,'Actor terminal extent')
                    terminal=struct.unpack('<16I',head)
                    require(terminal[2:7]==(2,64,self.ordinal,self.at,0)and terminal[7]==self.owner
                            and not any(terminal[8:]),'Actor loss/terminal identity')
                    self.complete=True;self.at+=64;break
                if len(head)<128:break
                length=struct.unpack_from('<I',head)[0]
                require(128<=length<=128+256*800+4096*80,'Actor declared frame extent')
                if length>size-self.at:break
                raw=head+source.read(length-128)
                require(len(raw)==length,'Actor frame changed while reading')
                frame=frame_record(raw,self.ordinal+1,self.owner,self.nonce,self.generation,self.journal)
                self.ordinal=frame['ordinal'];self.owner=frame['thread'];self.nonce=frame['nonce']
                self.generation=frame['generation']
                if frame['validity']&2:self.journal=frame['gameplay_sequence']
                self.frames[self.generation]=frame;self.at+=length
                if not self.keep_all:
                    for old in list(self.frames):
                        if old+64<self.generation:del self.frames[old]
            return self.frames
