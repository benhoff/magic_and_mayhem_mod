"""Bounded actual-swap metadata history; keep the original exact ready predicate."""
from collections import deque
import json,struct,time
from pathlib import Path
from rendering_picking_resize import ready
from rendering_pilot import require

class PresentedViewport:
    def __init__(self,directory):
        self.directory=Path(directory);self.path=None;self.at=8;self.index=0;self.paint=None;self.last=None
        self.presented=deque();self.active=None;self.candidates={};self.seen=set();self.trace=[];self.completed=[]

    def _candidate(self,paint):
        if self.active is None:return
        host=int(paint['host_us'])
        if not self.active['released_us']<host<=self.active['deadline_us']:return
        if not all(paint.get(k)is True for k in ['native_gpu_frame','gpu_read_complete','pixels_captured']):return
        key=(paint['world'],paint.get('scaling'),paint.get('frame_width'),paint.get('frame_height'),
             paint.get('left'),paint.get('top'),paint.get('image_width'),paint.get('image_height'))
        self.candidates.setdefault(key,paint)

    def poll(self):
        paths=list(self.directory.glob('viewport-*.bin'));require(len(paths)<=1,'Ambiguous live viewport collector')
        if not paths:return self.last
        path=paths[0];require(self.path is None or self.path==path,'Viewport collector changed');self.path=path
        with path.open('rb')as stream:
            size=path.stat().st_size
            if size<8:return self.last
            require(stream.read(8)==b'MNMACE01','Wrong live collector')
            while size>=self.at+8:
                stream.seek(self.at);m,n=struct.unpack('<II',stream.read(8))
                require(0<m<=65536 and n<=33*1024**2,'Live capture record budget')
                if size<self.at+8+m+n:break
                row=json.loads(stream.read(m));require(row['index']==self.index,'Live capture sequence gap')
                self.index+=1;self.at+=8+m+n
                if row['kind']=='paint':self.paint=row
                elif row['kind']=='swap'and int(row['paint']):
                    require(self.paint and row['paint']==self.paint['paint'],'Live swap/paint identity')
                    if int(self.paint['world'])>0:
                        self.last=self.paint;self.presented.append(self.paint);self._candidate(self.paint)
                        if len(self.presented)>4096:
                            removed=self.presented.popleft()
                            require(self.active is None or not self.active['released_us']<int(removed['host_us'])<=self.active['deadline_us'],
                                    'Active picking display-history budget exhausted')
                elif row['kind']=='close':require(row.get('complete')is True,'Live capture refused: '+str(row.get('error')))
        return self.last

    def arm(self,name,released_us):
        require(self.active is None,'Picking display windows overlap')
        self.active=dict(name=name,released_us=int(released_us),deadline_us=int(released_us)+8_000_000)
        self.candidates={};self.seen=set()
        for paint in self.presented:self._candidate(paint)

    def finish(self,name):
        require(self.active is not None and self.active['name']==name,'Picking display window identity changed')
        self.completed.append(dict(self.active,witness_generations=sorted(self.seen)))
        self.active=None;self.candidates={};self.seen=set()

    def match(self,rows,admissions,latest,target,baseline,kind):
        require(self.active is not None,'Missing released picking display window')
        fact=None
        for paint in self.candidates.values():
            if int(paint['world'])in self.seen:continue
            candidate=ready(rows,admissions,paint,target,baseline,kind)
            if candidate is not None:fact=candidate;self.seen.add(candidate['generation']);break
        require(len(self.trace)<16384,'Picking witness diagnostic budget exhausted')
        self.trace.append(dict(window=self.active['name'],host_us=time.monotonic_ns()//1000,
            latest_world=int(latest['world'])if latest else None,input_rows=len(rows),admission_rows=len(admissions),
            candidate_geometries=len(self.candidates),returned_generation=fact['generation']if fact else None))
        return fact
