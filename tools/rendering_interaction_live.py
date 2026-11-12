"""Bounded source-only sprite targeting; never search hit feedback or pixels."""
import hashlib,importlib.util,struct,time
from pathlib import Path
from resident_capture import PrefixDecoder
from rendering_interaction import signed
from rendering_pilot import require

class SpriteNotReady(ValueError):
    """No admitted visible creature body in the captured source frame yet."""

BODY_CALLERS={1:{0x500512,0x500a4e},4:{0x5005a2,0x500aca},
              5:{0x5005c0,0x500ae9},6:{0x500584,0x500aab}}

def body_record(f):return f[2]==9 and f[22] in BODY_CALLERS.get(f[15],())


def sprite_target(rows,history):
    matches={}
    for f,body in rows:
        if body_record(f) and [signed(f[8]),signed(f[9])]==history[:2]:
            frame=body[:f[19]];key=hashlib.sha256(frame).hexdigest()
            if key not in matches:matches[key]=dict(frame=frame,sequences=[],anchor=history[:2])
            matches[key]['sequences'].append(f[1])
    if not matches:raise SpriteNotReady('Tracked visible creature body is not ready')
    require(len(matches)==1,'Tracked draw history has ambiguous captured creature sprites')
    digest,item=next(iter(matches.items()));frame=item['frame']
    require(len(frame)>=40 and struct.unpack_from('<I',frame)[0]==len(frame),'Unclosed tracked sprite')
    width,height,ox,oy=struct.unpack_from('<4I',frame,4)
    require(0<width<=256 and 0<height<=256 and 40+height*8<=len(frame),'Tracked sprite geometry bounds')
    table_end=40+height*8;first=struct.unpack_from('<I',frame,44)[0]
    require(table_end<=first<=len(frame),'Tracked sprite data bounds')
    candidates=[]
    for y in range(height):
        begin=struct.unpack_from('<I',frame,40+y*8)[0]
        end=struct.unpack_from('<I',frame,48+y*8)[0] if y+1<height else first
        require(table_end<=begin<=end<=first,'Tracked sprite row bounds')
        x=0;opaque=False
        for run in frame[begin:end]:
            require(x+run<=width,'Tracked sprite row overflow')
            if opaque:
                for px in range(x,x+run):candidates.append(((2*px-width+1)**2+(2*y-height+1)**2,y,px))
            x+=run;opaque=not opaque
        require(x==width,'Tracked sprite row incomplete')
    require(candidates,'Tracked sprite is entirely transparent')
    _,y,x=min(candidates)
    return dict(point=[history[0]-signed(ox)+x,history[1]-signed(oy)+y],anchor=history[:2],
                frame_sha256=digest,raster_sequences=item['sequences'],size=[width,height],
                origin=[signed(ox),signed(oy)],opaque_pixel=[x,y])


class LiveSources:
    def __init__(self,path):
        self.path=path;self.at=8;self.decoder=PrefixDecoder();self.frames={};self.worlds=0;self.chunks=0
        spec=importlib.util.spec_from_file_location('live_producers',Path(__file__).with_name('inspect-canvas-producers.py'))
        self.module=importlib.util.module_from_spec(spec);spec.loader.exec_module(self.module)

    def close(self):self.decoder.close()

    def poll(self,through):
        # A producer may keep appending while we decode. Read only through the
        # requested identity; chasing EOF can evict that very frame.
        end=self.path.stat().st_size
        with self.path.open('rb') as f:
            require(f.read(8)==b'MNMPROF4','Live source archive version')
            while self.worlds<through and end>=self.at+80:
                f.seek(self.at);h=f.read(80);g,c,s,q,n,_=struct.unpack('<QQIIII',h[:32])
                require(g==self.worlds+1 and g<=8192 and 16<=s<=16777216 and (q==0 if c else 64<=q<=197184),'Live source identity/bounds')
                require(4<n<=s+q+(s+q)//128+1024,'Live source compressed extent')
                if end<self.at+80+n:break
                body=self.decoder.decode(h,f.read(n),s+q);self.at+=80+n
                if c:
                    require(c==self.chunks+1,'Live source chunk sequence');self.chunks+=1;continue
                self.worlds=g
                _,rows=self.module.decode(body[:s],complete=False)
                self.frames[g]=[(a,b) for a,b in rows if body_record(a)]
                for old in list(self.frames):
                    if old+64<g:del self.frames[old]

    def target(self,generation,history):
        deadline=time.monotonic()+10
        while time.monotonic()<deadline:
            self.poll(generation)
            if generation in self.frames:return dict(sprite_target(self.frames[generation],history),source_generation=generation)
            require(generation+64>=self.worlds,'Tracked source history fell outside bounded live cache')
            time.sleep(.02)
        raise ValueError('Tracked source capture deadline')
