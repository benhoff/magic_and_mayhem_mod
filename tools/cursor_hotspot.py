"""Finite original cursor descriptors, hotspot geometry and actual-copy proof inputs."""
from configparser import ConfigParser
from pathlib import Path
import hashlib
import io
import struct
import numpy as np
from rendering_pilot import require

ROOT=Path(__file__).resolve().parents[1]
BID='NP.cursor-hotspot-closeout'
PREFIX='research/runtime/cursor-hotspot-results-20261108/'
PLAN=PREFIX+'input-plan-01.json'
PARENT='research/runtime/rendering-selection-results-20261108/analysis-03.json'
LEAF='journey.cursor-interaction.hotspot'
REQUIRED_CLASSES={6,7,9}
MAX_REQUESTS=9  # Explicit user approval20261108; six original starts unchanged.
MAX_GAMES=6


def descriptors(raw):
    config=ConfigParser(interpolation=None);config.read_string(raw.decode('ascii'))
    def integer(section,key):return int(config[section][key].split(';',1)[0].strip())
    require(integer('HEADER','BitmapWidth')==400 and integer('HEADER','BitmapHeight')==280,
            'Unexpected original cursor atlas dimensions')
    result={}
    for number in range(22):
        section='CURSOR_'+str(number)
        x,y=integer(section,'Hotspotx'),integer(section,'Hotspoty')
        frames=[int(v.strip()) for v in config[section]['Frames'].split(';',1)[0].split(',')]
        delay=integer(section,'Delay')
        require(0<=x<40 and 0<=y<40 and 1<=len(frames)<=12 and all(0<=v<70 for v in frames)
                and 0<=delay<=100,'Unsupported original cursor descriptor')
        result[number]=dict(hotspot=[x,y],frames=frames,delay=delay)
    require({n:(result[n]['hotspot'],result[n]['frames']) for n in REQUIRED_CLASSES}==
            {6:([19,0],[3]),7:([39,19],[5]),9:([0,19],[4])},'Required nonzero-hotspot descriptors changed')
    return result


def points(size):
    w,h=size;require(size in ([640,480],[800,600]),'Unsupported actual cursor frame')
    result=[('center',[(w-200)//2,(h-50)//2])]
    for label,width,height in (('playfield',w-200,h-50),('frame',w,h)):
        for name,xy in (('top',[width//2,0]),('right',[width-1,height//2]),
                        ('bottom',[width//2,height-1]),('left',[0,height//2]),
                        ('top-left',[0,0]),('top-right',[width-1,0]),
                        ('bottom-left',[0,height-1]),('bottom-right',[width-1,height-1])):
            result.append((label+'-'+name,xy))
    return result


def acceptance_points(size):
    """Smallest fixed schedule for actual classes6/7/9; preserve the old scout."""
    return [(name,xy) for name,xy in points(size)
            if name in ('center','frame-top','frame-right','frame-left')]


def geometry(point,descriptor,size,frame=None):
    require(len(point)==2 and all(type(v)is int for v in point) and
            all(0<=point[i]<size[i] for i in (0,1)),'Pointer is outside actual frame')
    require(frame in descriptor['frames'] if frame is not None else len(descriptor['frames'])==1,
            'Animated cursor needs its actual frame, not a guessed image')
    frame=descriptor['frames'][0] if frame is None else frame
    x,y=[point[i]-descriptor['hotspot'][i] for i in (0,1)]
    l,t,r,b=max(0,x),max(0,y),min(size[0],x+40),min(size[1],y+40)
    require(l<r and t<b,'Empty hotspot footprint')
    sx,sy=(frame%10)*40,(frame//10)*40
    return dict(point=list(point),origin=[x,y],saved=[l,t,r,b],destination=[l,t,r,b],
                source=[sx+l-x,sy+t-y,sx+r-x,sy+b-y],frame=frame)


def original_geometry(row,descs,size,frame=None):
    require(row['kind']==1 and row['frame'] and row['cursor'] and row['saved_cursor']
            and row['saved_cursor'][0]==1 and row['cursor_type'] in descs
            and row['high_resolution']==int(size==[800,600]),'Unadmitted original cursor state')
    desc=descs[row['cursor_type']]
    result=geometry(row['cursor']['point'],desc,size,frame)
    require(row['saved_cursor'][1:]==result['origin'],'Original saved origin differs from configured hotspot')
    return dict(result,cursor_class=row['cursor_type'],generation=row['frame']['generation'],
                nonce=row['frame']['nonce'],owner=row['frame']['owner'],ordinal=row['ordinal'],
                poll=row['cursor']['poll'],descriptor=desc)


def bitmap_words(raw):
    """Independent BI_RGB byte decoding; pixels from a native surface are not inputs."""
    require(len(raw)>=54 and raw[:2]==b'BM' and struct.unpack_from('<I',raw,2)[0]==len(raw),
            'Original cursor BMP envelope')
    offset=struct.unpack_from('<I',raw,10)[0]
    dib,w,h,planes,bits,compression=struct.unpack_from('<IiiHHI',raw,14)
    require(dib==40 and (w,abs(h),planes,compression)==(400,280,1,0) and bits in (8,24),
            'Unsupported original cursor BMP')
    stride=((w*bits+31)//32)*4
    end=offset+stride*abs(h)
    require(offset>=54 and end<=len(raw) and len(raw)-end<=3 and not any(raw[end:]),'Cursor BMP pixel extent/padding')
    rows=np.frombuffer(raw[offset:end],dtype='u1').reshape(abs(h),stride)
    if h>0:rows=rows[::-1]
    if bits==8:
        used=struct.unpack_from('<I',raw,46)[0] or 256
        require(used==256 and offset>=54+used*4,'Missing cursor BMP palette')
        palette=np.frombuffer(raw[54:54+1024],dtype='u1').reshape(256,4)
        colors=palette[rows[:,:w],:3].astype('uint16')
    else:colors=rows[:,:w*3].reshape(abs(h),w,3).astype('uint16')
    return ((colors[:,:,2]>>3)<<11)|((colors[:,:,1]>>2)<<5)|(colors[:,:,0]>>3)
