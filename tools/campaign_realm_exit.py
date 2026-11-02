"""Read-only original mode4 Mini/parent observations for bounded physical input."""
import configparser
import hashlib
from pathlib import Path
import struct
from campaign_route import frame_size

HEADER=b'MNMMENU1'+struct.pack('<II',1,64)


def read_events(path,thread,complete=False):
    data=Path(path).read_bytes()
    if data[:16]!=HEADER or complete and (len(data)-16)%64:
        raise ValueError('Invalid original Realm exit trace')
    end=16+(len(data)-16)//64*64
    rows=list(struct.iter_unpack('<16I',data[16:end]))
    if not thread or len(rows)>=256 or any(r[0]!=i+1 or r[2]!=thread for i,r in enumerate(rows)):
        raise ValueError('Exhausted,foreign-thread or unordered Realm exit trace')
    return rows


def mode4_state(rows,after=0,modal=False):
    for i in range(len(rows)-3,-1,-1):
        a,b,c=rows[i:i+3]
        if (a[0]<=after or a[1] not in (13,16) or b[1]!=a[1]+1 or c[1]!=a[1]+2
                or b[0]!=a[0]+1 or c[0]!=a[0]+2):continue
        if (a[3:7]!=(17,0x6a5088,0x5c6644,1) or a[7] or a[8]
                or a[9:13]!=(4,5,0x6a5088,4) or b[9:11]!=(0,5)
                or bool(c[9])!=modal):continue
        if any(r[2:9]!=a[2:9] or r[11:13]!=a[11:13] for r in (b,c)):continue
        return dict(sequence=c[0],thread=a[2],mode=4,context=5,owner=a[11],depth=4,modal_pointer=c[9])
    return None


def fresh_main(rows,after=0):
    for r in reversed(rows):
        if (r[0]>after and r[1] in (1,4) and r[3:7]==(3,0x657ce0,0x5c63e4,1)
                and not r[7] and not r[8] and r[11:13]==(0x657ce0,2)):
            return dict(sequence=r[0],thread=r[2],owner=r[11],depth=2)
    return None


def options_point(size):
    width,height=frame_size(size)
    # Original54fd70: four48x48 controls52pixels apart at top-right.
    return width-184,28


def quit_point(game,size):
    width,height=frame_size(size)
    path=Path(game)/'Interface/MiniMenu/screen (Mini Menu).cfg';raw=path.read_bytes()
    cfg=configparser.ConfigParser(interpolation=None,inline_comment_prefixes=(';',))
    cfg.read_string(raw.decode('cp1252'));section=cfg['TEXTBUTTON_4']
    rect=tuple(map(int,section['Rect2' if width==800 else 'Rect1'].split(',')))
    expected=(250,330,550,370) if width==800 else (200,264,440,296)
    if rect!=expected or section['Text'].split(';',1)[0].strip()!='50':
        raise ValueError('Unreviewed original Realm Mini Quit rectangle/text')
    return ((rect[0]+rect[2])//2,(rect[1]+rect[3])//2),dict(path=str(path),sha256=hashlib.sha256(raw).hexdigest(),rectangle=list(rect))
