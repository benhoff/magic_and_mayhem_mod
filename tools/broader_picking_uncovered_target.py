"""Choose a player body pixel outside every other captured creature body mask."""
from functools import lru_cache
import hashlib,struct
from broader_picking_pose_target import targets as pose_targets,PosedRows
from rendering_interaction import signed
from rendering_interaction_live import body_record,SpriteNotReady
from rendering_pilot import require
from scene_actor_observation import crc32c
POLICY='source-body-disjoint-opacity-v1'

@lru_cache(maxsize=64)
def opaque_rows(blob):
    require(len(blob)>=40 and struct.unpack_from('<I',blob)[0]==len(blob),'Unclosed source body')
    w,h,ox,oy=struct.unpack_from('<4I',blob,4);table=40+h*8
    require(0<w<=256 and 0<h<=256 and table<=len(blob),'Source body geometry')
    require(-32768<=signed(ox)<=32767 and -32768<=signed(oy)<=32767,'Source body origin outside bounded no-wrap domain')
    end_table=struct.unpack_from('<I',blob,44)[0];require(table<=end_table<=len(blob),'Source body table')
    rows=[]
    for y in range(h):
        begin=struct.unpack_from('<I',blob,40+y*8)[0]
        end=struct.unpack_from('<I',blob,48+y*8)[0]if y+1<h else end_table
        require(table<=begin<=end<=end_table,'Source body row bounds')
        x=0;opaque=False;runs=[]
        for length in blob[begin:end]:
            require(x+length<=w,'Source body row overflow')
            if opaque and length:runs.append((x,x+length))
            x+=length;opaque=not opaque
        require(x==w,'Incomplete source body row');rows.append(tuple(runs))
    return w,h,signed(ox),signed(oy),tuple(rows)

def contains(shape,anchor,point):
    w,h,ox,oy,rows=shape;x=point[0]-anchor[0]+ox;y=point[1]-anchor[1]+oy
    return 0<=x<w and 0<=y<h and any(a<=x<b for a,b in rows[y])

def choose(shape,anchor,others,extent):
    w,h,ox,oy,rows=shape;candidates=[]
    for y,runs in enumerate(rows):
        for begin,end in runs:
            for x in range(begin,end):
                point=[anchor[0]-ox+x,anchor[1]-oy+y]
                if all(0<=v<n for v,n in zip(point,extent))and not any(contains(s,a,point)for s,a in others):
                    candidates.append(((2*x-w+1)**2+(2*y-h+1)**2,y,x))
    if not candidates:raise SpriteNotReady('No source-only player body pixel outside other creature masks')
    _,y,x=min(candidates);return [anchor[0]-ox+x,anchor[1]-oy+y],[x,y]

def targets(records,history,position,frame):
    body,cell=pose_targets(records,history,position,frame)
    matched=[];others=[];source_masks=set()
    for f,raw in records:
        if not body_record(f):continue
        blob=raw[:f[19]];anchor=[signed(f[8]),signed(f[9])];digest=hashlib.sha256(blob).hexdigest()
        require(all(-32768<=v<=32767 for v in anchor),'Source body anchor outside bounded no-wrap domain')
        shape=opaque_rows(blob)
        if anchor==history[:2]and digest==body['frame_sha256']:matched.append(blob);continue
        key=(digest,*anchor)
        if key not in source_masks:others.append((shape,anchor));source_masks.add(key)
    require(matched and len(set(matched))==1,'Missing unique tracked body source')
    blob=matched[0];checksum=crc32c(blob)
    require(not any(d[0]!=0 and d[1]==0 and [signed(d[5]),signed(d[6])]==history[:2]
        and d[11:13]==(len(blob),checksum)for d in frame['draws']),'Another actor shares tracked body identity')
    point,pixel=choose(opaque_rows(blob),history[:2],others,frame['extent'])
    body=dict(body,point=point,opaque_pixel=pixel,target_policy=POLICY,
              excluded_other_source_body_masks=len(others),source_actor_generation=frame['generation'],
              hit_feedback_used=False,original_hit_order_assumed=False)
    return body,cell

def bound_targets(records,history,position):
    require(isinstance(records,PosedRows),'Missing same-generation actor binding')
    return targets(records,history,position,records.actor_frame)
