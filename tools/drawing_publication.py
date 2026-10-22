"""Admit guarded opaque-copy COM results and independent primary readbacks.

This observes two original sites. It does not establish completeness of all
DirectDraw routes, applied input actions, full journeys or live replacement.
"""
from collections import Counter
import hashlib
import struct
from pathlib import Path
import importlib.util


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def analyze(directory):
    directory=Path(directory)
    spec=importlib.util.spec_from_file_location('publication_producers',Path(__file__).with_name('inspect-canvas-producers.py'))
    tool=importlib.util.module_from_spec(spec);spec.loader.exec_module(tool)
    source=directory/'canvas-producers.bin';raw=source.read_bytes();header,records=tool.decode(raw,complete=False)
    mark=(directory/'canvas-producers.done').read_bytes()
    failures=[{'sequence':r[1],'code':r[14],'detail':hex(r[15])} for r,_ in records if r[2]==13]
    checkpoints=[]
    for r,_ in records:
        if r[2]!=10:continue
        p=directory/f'producer-oracle-{r[14]:04}.565'
        if r[14]!=len(checkpoints)+1 or not r[3] or not 1<=r[5]<=2048 or not 1<=r[6]<=2048 or p.stat().st_size!=r[5]*r[6]*2:
            raise ValueError('Missing or malformed original producer checkpoint')
        checkpoints.append({'sequence':r[1],'oracle':r[14],'canvas':r[3],'width':r[5],'height':r[6],'reason':r[15],'queue':r[16],'path':p.name,'sha256':sha(p)})
    returns=[r[14] for r,_ in records if r[2]==12];entries=[r[14] for r,_ in records if r[2]==11]
    if (len(mark)!=32 or mark[:8]!=b'MNMPDONE' or struct.unpack_from('<6I',mark,8)!=(1,header[6],len(records),len(failures),len(checkpoints),len(raw)) or
        entries!=list(range(1,header[6]+1)) or returns!=entries):
        raise ValueError('Producer publication prefix is incomplete')
    trace=directory/'drawing-publication-0000.bin';data=trace.read_bytes()
    if len(data)<128 or len(data)>32+65536*96 or (len(data)-32)%96 or data[:8] not in (b'MNMDP001',b'MNMDP002') or struct.unpack_from('<6I',data,8)!=((1 if data[:8]==b'MNMDP001' else 2),96,0x40209ca7,65536,1024,0):
        raise ValueError('Invalid original publication trace envelope')
    version=1 if data[:8]==b'MNMDP001' else 2
    calls=[];frames=[];drops=0;last_sequence=0
    for i,r in enumerate(struct.iter_unpack('<24I',data[32:])):
        status=struct.unpack('<i',struct.pack('<I',r[2]))[0]
        if r[0]!=i+1 or r[1] not in (1,2) or r[8] not in (0,1) or r[9]!=(0x1000000 if r[1]==1 else 0x10) or (any(r[22:]) if version==1 else not r[22] or r[23]>8192) or not last_sequence<=r[4]<len(records) or r[5]>header[6]:
            raise ValueError('Invalid call identity, ordering or guarded flags')
        last_sequence=r[4]
        if r[19]:
            if (not r[8] or status<0 or r[10] or r[19]!=len(frames)+1 or r[14:18]!=(16,0xf800,0x7e0,31) or
                not 1<=r[12]<=2048 or not 1<=r[13]<=2048 or not r[12]*2<=r[18]<=8192):
                raise ValueError('Malformed primary readback identity/format')
            p=directory/f'publication-oracle-{r[19]:04}.565'
            if p.stat().st_size!=r[12]*r[13]*2:raise ValueError('Missing full primary oracle')
            frames.append({'ordinal':r[19],'attempt':r[0],'source_sequence':r[4],'queue':r[5],'canvas':r[6],
                           'destination':r[7],'width':r[12],'height':r[13],'path':p.name,'sha256':sha(p)})
        if r[8] and status>=0 and (not r[19] or r[11]&0x80000000):drops+=1
        if r[20]!=drops or r[21]!=len(frames):raise ValueError('Primary readback loss accounting changed')
        calls.append({'attempt':r[0],'route':'Blt' if r[1]==1 else 'BltFast','hresult':status,'last_error':r[3],
                      'source_sequence':r[4],'queue':r[5],'source_canvas':r[6],'destination_canvas':r[7],
                      'primary':bool(r[8]),'flags':r[9],'observer_lock_hresult':r[10],'observer_unlock_hresult':r[11],
                      'readback_ordinal':r[19],'thread':r[22],'action_record':r[23]})
    last_world_return=max((r[1] for r,_ in records if r[2]==12),default=0)
    if (not calls[-1]['primary'] or calls[-1]['hresult']<0 or calls[-1]['queue']!=header[6] or
            calls[-1]['source_sequence']<last_world_return):
        raise ValueError('Missing final successful publication after requested World return')
    producers={'path':source.name,'sha256':sha(source),'records':len(records),'queues':header[6],
               'operations':dict(Counter(tool.NAMES[r[2]] for r,_ in records)),'failures':failures,'checkpoints':checkpoints,
               'closed_prefix':True,'all_observed_inputs_admitted':not failures,'original_pixels_used_as_native_inputs':False}
    camera=[];current_queue=0
    for r,payload in records:
        if r[2]==11:current_queue=r[14]
        elif r[2]==24:
            state=struct.unpack_from('<37I',payload)
            if state[0]==2 and r[17]==0:camera.append({'source_sequence':r[1],'queue':current_queue,'view':state[9],'center':list(state[3:5]),'next_frame':next((f['ordinal'] for f in frames if f['source_sequence']>=r[1]),None)})
    world=[]
    for path in sorted(directory.glob('scene-*.bin')):
        snapshot=path.read_bytes()
        if len(snapshot)<64 or snapshot[:8]!=b'MNMSCNE1' or struct.unpack_from('<I',snapshot,16)[0]!=len(snapshot) or struct.unpack_from('<I',snapshot,40)[0]!=0x40209ca7:
            raise ValueError('Invalid independently observed World entry')
        queue,view=struct.unpack_from('<I',snapshot,20)[0],struct.unpack_from('<I',snapshot,32)[0]
        if queue!=len(world)+1 or view>3:raise ValueError('World orientation sequence is discontinuous')
        world.append({'queue':queue,'view':view,'path':path.name,'sha256':sha(path)})
    if world and len(world)!=header[6]:raise ValueError('Missing original World orientation source')
    camera_scope={'world_entries':world,'world_orientations':sorted({s['view'] for s in world}),'states':camera,'orientations':sorted({s['view'] for s in camera}),'centers':sorted({tuple(s['center']) for s in camera}),'scope':'Observed effective original minimap camera centers/orientations from owned producer inputs; XTest attempts alone do not supply these results.'}
    # Keep report JSON roundtrips type-identical.
    camera_scope['centers']=[list(c) for c in camera_scope['centers']]
    actions=applied_actions(directory,calls,frames,len(records)) if version==2 else None
    return {'schema':1,'wire_version':version,'applied_actions':actions,'camera_state':camera_scope,'boundary':'observed_primary_blt','trace':trace.name,'trace_sha256':sha(trace),'calls':calls,'frames':frames,
            'readback_drops':drops,'sampling_interval':1,'all_primary_routes_observed':False,'actions_bound':False,
            'producers':producers,'original_pixels_used_as_native_inputs':False,
            'scope':'Actual guarded opaque wrapper COM returns and successful primary readbacks; alternate primary/Flip routes and applied action identities remain unadmitted.'}


def applied_actions(directory,calls,frames,producer_records):
    path=directory/'drawing-actions-0000.bin';data=path.read_bytes()
    if (len(data)<32 or len(data)>32+8192*64 or (len(data)-32)%64 or data[:8]!=b'MNMDAC01' or
        struct.unpack_from('<6I',data,8)!=(1,64,0x40209ca7,8192,0,0)):
        raise ValueError('Invalid original applied-action envelope')
    rows=list(struct.iter_unpack('<16I',data[32:]));pending={};actions=[];thread=None
    for i,r in enumerate(rows):
        if (r[0]!=i+1 or r[1] not in (1,2) or not r[2] or not r[3] or not r[4] or not r[5] or
            r[8]>producer_records or r[9]>len(calls) or r[10]!=1+sum(c['readback_ordinal']!=0 for c in calls[:r[9]]) or
            not r[11] or any(r[13:])):raise ValueError('Invalid original action identity/boundary')
        if thread is None:thread=r[11]
        if r[11]!=thread:raise ValueError('Original action thread changed')
        if r[1]==1:
            if r[2] in pending or any(a['request']==r[2] for a in actions) or r[7]!=0xffffffff:
                raise ValueError('Duplicate or malformed original action entry')
            pending[r[2]]=r
        else:
            first=pending.pop(r[2],None)
            if r[7] not in range(1,6) or not first or r[2:7]!=first[2:7] or r[8]<first[8] or r[9]<first[9] or r[12]<first[12]:raise ValueError('Unmatched original action return')
            actions.append({'request':r[2],'action':r[3],'screen':r[4],'generation':r[5],'argument':r[6],
                            'status':r[7],'entry_source_sequence':first[8],'return_source_sequence':r[8],
                            'entry_next_frame':first[10],'return_next_frame':r[10],'thread':r[11],
                            'entry_record':first[0],'return_record':r[0]})
    for c in calls:
        count=c['action_record']
        if count>len(rows) or c['thread']!=thread or (count and rows[count-1][9]>=c['attempt']) or (count<len(rows) and rows[count][9]<c['attempt']):
            raise ValueError('Publication/action thread or phase boundary mismatch')
    return {'path':path.name,'sha256':sha(path),'records':len(rows),'requests':actions,
            'pending_requests':sorted(pending),'successful_requests':sum(a['status']==1 for a in actions),
            'scope':'Observed original menu adapter dispatch entry/return and result, bound to COM attempt and next readback ordinal; physical camera input outcomes require independent original source-state transitions.'}
