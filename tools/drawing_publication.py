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
    if len(data)<128 or len(data)>32+65536*96 or (len(data)-32)%96 or data[:8]!=b'MNMDP001' or struct.unpack_from('<6I',data,8)!=(1,96,0x40209ca7,65536,1024,0):
        raise ValueError('Invalid original publication trace envelope')
    calls=[];frames=[];drops=0;last_sequence=0
    for i,r in enumerate(struct.iter_unpack('<24I',data[32:])):
        status=struct.unpack('<i',struct.pack('<I',r[2]))[0]
        if r[0]!=i+1 or r[1] not in (1,2) or r[8] not in (0,1) or r[9]!=(0x1000000 if r[1]==1 else 0x10) or any(r[22:]) or not last_sequence<=r[4]<len(records) or r[5]>header[6]:
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
                      'readback_ordinal':r[19]})
    if not calls[-1]['primary'] or calls[-1]['hresult']<0 or calls[-1]['queue']!=header[6]:
        raise ValueError('Missing final successful publication after requested World return')
    producers={'path':source.name,'sha256':sha(source),'records':len(records),'queues':header[6],
               'operations':dict(Counter(tool.NAMES[r[2]] for r,_ in records)),'failures':failures,'checkpoints':checkpoints,
               'closed_prefix':True,'all_observed_inputs_admitted':not failures,'original_pixels_used_as_native_inputs':False}
    return {'schema':1,'boundary':'observed_primary_blt','trace':trace.name,'trace_sha256':sha(trace),'calls':calls,'frames':frames,
            'readback_drops':drops,'sampling_interval':1,'all_primary_routes_observed':False,'actions_bound':False,
            'producers':producers,'original_pixels_used_as_native_inputs':False,
            'scope':'Actual guarded opaque wrapper COM returns and successful primary readbacks; alternate primary/Flip routes and applied action identities remain unadmitted.'}
