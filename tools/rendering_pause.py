"""Read-only, bounded public Mini/World/input/media ownership witnesses."""
import sqlite3
import struct
import time
from pathlib import Path
from rendering_pilot import require


def stable(path, size, sequences):
    for attempt in range(8):
        with Path(path).open('rb') as f:
            raw=f.read(size+1);f.seek(0);again=f.read(size+1)
        if len(raw)==size and raw==again and all(struct.unpack_from('<I',raw,offset)[0]%2==0 for offset in sequences):return raw
        time.sleep(.001)
    raise ValueError('Unstable/incomplete pause channel after bounded reads')


def input_state(raw):
    require(len(raw)==1088 and raw[:8]==b'MNMINK01','Wrong input wire')
    w=struct.unpack('<272I',raw)
    require(w[2:4]==(1,1088) and w[4]>0 and not w[4]&1 and w[5] in (0,1)
            and not any(w[10:16]),'Invalid published input state')
    return dict(sequence=w[4],active=w[5],position=list(w[6:8]),size=list(w[8:10]),
                keys=list(w[16:]),held=[i for i,v in enumerate(w[16:]) if v&0x80000000])


def media_state(raw):
    require(len(raw)==2048 and raw[:8]==b'MNMMED01','Wrong media wire')
    w=struct.unpack('<512I',raw)
    require(w[2:4]==(1,2048) and not w[4]&1 and not w[132]&1,'Torn media state')
    return dict(sequence=w[4],request=w[5],operation=w[6],flags=w[7],response=w[128],
                status=w[129],heartbeat=w[130],accepted=w[131],response_sequence=w[132])


def music_state(raw):
    require(len(raw)==1024 and raw[:8]==b'MNMMUS01','Wrong music wire')
    w=struct.unpack('<256I',raw)
    require(w[2:4]==(1,1024) and not w[4]&1 and not w[16]&1,'Torn music state')
    return dict(sequence=w[4],request=w[5],operation=w[6],track=w[7],response=w[17],
                status=w[18],heartbeat=w[20],ready=w[21],phase=w[22],current_track=w[23],
                position=w[24],failed=w[29],recoveries=w[30],losses=w[31])


def world_state(raw):
    require(len(raw)==384 and raw[:8]==b'MNMCON06','Wrong continuous World wire')
    h=struct.unpack_from('<32I',raw)
    require(h[2:6]==(6,128,35869312,3) and h[8:12]==(1,1,0,0),'Unowned/terminal World channel')
    slots=[]
    for at in (128,256):
        w=struct.unpack_from('<32I',raw,at)
        require(w[0]<=8 and w[1]==0,'Invalid World slot')
        slots.append(dict(state=w[0],generation=w[2]|w[3]<<32,request_bytes=w[5]))
    return dict(nonce=str(h[6]|h[7]<<32),slots=slots,
                pending_world_jobs=sum(s['state'] in (2,3,4) and s['request_bytes']>0 for s in slots))


class Samples:
    """SQLite is the live diagnostic sink; no synchronous GUI-path writes."""
    def __init__(self,path):
        self.db=sqlite3.connect(path)
        self.db.execute('CREATE TABLE samples(ordinal INTEGER PRIMARY KEY,phase TEXT NOT NULL,host_ns INTEGER NOT NULL,menu BLOB NOT NULL,input BLOB NOT NULL,media BLOB NOT NULL,music BLOB NOT NULL,world BLOB NOT NULL,frame BLOB NOT NULL,world_channel_bytes INTEGER NOT NULL)')
        self.count=0
    def append(self,phase,session,menu):
        session=Path(session)
        values=[stable(session/'render-input.bin',1088,(16,)),
                stable(session/'native-media.bin',2048,(16,528)),
                stable(session/'native-music.bin',1024,(16,64))]
        with (session/'world-continuous.bin').open('rb') as f:
            header=f.read(128);f.seek(128);a=f.read(128);f.seek(128+17934592);b=f.read(128)
        world=header+a+b;world_state(world)
        path=session/'frame-input.bin';size=path.stat().st_size
        require(size>=400 and (size-16)//384>0,'No original World return witness')
        with path.open('rb') as f:
            begin=max(16,16+((size-16)//384-32)*384);f.seek(begin);tail=f.read(size-begin)
        rows=[tail[i:i+384] for i in range(0,len(tail)-383,384)]
        frame=next((v for v in reversed(rows) if struct.unpack_from('<I',v,8)[0]==1),None)
        require(frame is not None,'No bounded original World return in tail')
        words=struct.unpack('<96I',frame)
        require(words[7]&256 and str(words[61]|words[62]<<32)==world_state(world)['nonce'],
                'Original return belongs to another World nonce')
        self.count+=1;now=time.monotonic_ns()
        self.db.execute('INSERT INTO samples VALUES(?,?,?,?,?,?,?,?,?,?)',
            (self.count,phase,now,menu,*values,world,frame,(session/'world-continuous.bin').stat().st_size))
        if self.count%10==0:self.db.commit()
        return dict(sample=self.count,phase=phase,host_ns=now,input=input_state(values[0]),
                    media=media_state(values[1]),music=music_state(values[2]),world=world_state(world),
                    original_world_return=words[3],original_generation=words[59]|words[60]<<32)
    def close(self):
        self.db.commit();self.db.close()


def validate_dwell(rows):
    require(len(rows)>=30 and rows[-1]['host_ns']-rows[0]['host_ns']>=2_000_000_000,
            'Insufficient actual Mini dwell')
    first=rows[0]
    require(all(b['host_ns']>a['host_ns'] for a,b in zip(rows,rows[1:])),
            'Reordered or repeated pause sample clock')
    require(len({v['original_world_return'] for v in rows})==1
            and all(v['world']['nonce']==first['world']['nonce'] and v['world']['pending_world_jobs']==0 for v in rows),
            'World work progressed or retained native-owned jobs while Mini owned input')
    require(all(not v['input']['active'] and not v['input']['held']
                and v['input']['keys']==first['input']['keys'] for v in rows)
            and rows[-1]['input']['sequence']>first['input']['sequence'],
            'Gameplay input leaked, held state survived, or host heartbeat stalled')
    require(all(not (v['media']['operation']==1 and v['media']['status'] in (1,2))
                and v['media']['request']==v['media']['response'] for v in rows),
            'An unacknowledged/blocking movie owns the paused menu')
    require(all(v['music']['ready']==1 and v['music']['failed']==first['music']['failed']
                and v['music']['losses']==first['music']['losses'] for v in rows)
            and rows[-1]['media']['heartbeat']!=first['media']['heartbeat']
            and rows[-1]['music']['heartbeat']!=first['music']['heartbeat'],
            'Media/music host ownership failed or stalled during Mini')
    return dict(verified=True,samples=len(rows),duration_ns=rows[-1]['host_ns']-first['host_ns'],
                original_world_return=first['original_world_return'],world_nonce=first['world']['nonce'],
                input_released=True,media_live=True,music_live=True,
                music_playback_frozen_claimed=False,all_simulation_animation_pause_claimed=False)
