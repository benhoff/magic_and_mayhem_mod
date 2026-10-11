"""Bounded client lease accounting from an independently closed V2 journal."""
from collections import Counter
import hashlib
import struct
from drawing_api_details import trace as detail_trace


class Resources:
    def __init__(self):
        self.leases={};self.intervals=[];self.outputs=[];self.releases=[];self.gaps=[];self.calls=Counter()

    def gap(self,call,reason):
        self.gaps.append({'call_id':call['call_id'],'site':call['site'],'reason':reason})

    def observe(self,call):
        """Completed, already wire-validated call; output snapshots remain raw."""
        method=call['method'];self.calls[method]+=1
        if not method.startswith(('IDirectDrawSurface2.','IUnknown.','IDirectDraw2.','IAMMultiMediaStream.','IMultiMediaStream.','IDirectDrawMediaStream.','IDirectDrawStreamSample.')):return
        args=call['args'];receiver=call['receiver'];result=call['eax']
        if not call['arguments_read'] or not args or not receiver or receiver!=args[0]:
            self.gap(call,'Reviewed call has no readable exact BEFORE receiver candidate; no semantic interval assigned')
            return
        for slot in call['snapshots']:
            if slot['event']==2 and 'pointer output' in slot['name']:
                self.outputs.append({'call_id':call['call_id'],'site':call['site'],'receiver':receiver,'method':method,'hresult':result,'read_state':slot['state'],'raw_bytes':slot['data'],'validity':'successful nonnull output' if result==0 and slot['state']==1 and len(slot['data'])==8 and int.from_bytes(bytes.fromhex(slot['data']),'little') else 'raw, validity unassigned'})
        if method=='IUnknown.Release':
            self.releases.append({'call_id':call['call_id'],'site':call['site'],'receiver':receiver,'eax':result,'scope':'Client Release observed; return does not prove destruction, unique ownership or absence of aliases'})
            if any(k[0]==receiver for k in self.leases):self.gap(call,'Release observed with a client borrow still open; no external lifetime inferred')
            return
        if method not in ('IDirectDrawSurface2.GetDC','IDirectDrawSurface2.ReleaseDC','IDirectDrawSurface2.Lock','IDirectDrawSurface2.Unlock'):return
        kind='DC' if method.endswith(('GetDC','ReleaseDC')) else 'CPU'
        key=(receiver,kind)
        if result!=0:return
        acquiring=method.endswith(('GetDC','Lock'))
        if acquiring:
            slots=[s for s in call['snapshots'] if s['event']==2 and s['state']==1]
            if kind=='DC':
                slots=[s for s in slots if s['name']=='HDC output' and len(s['data'])==8]
                pointer=int.from_bytes(bytes.fromhex(slots[0]['data']),'little') if len(slots)==1 else 0;descriptor=None
            else:
                slots=[s for s in slots if s['name']=='DDSURFACEDESC input/output' and len(s['data'])==216]
                descriptor=struct.unpack('<27I',bytes.fromhex(slots[0]['data'])) if len(slots)==1 else None
                # Lock's successful output pointer is returned with0x100f in the
                # original Wine cohort; DDSD_LPSURFACE is not an output prerequisite.
                pointer=descriptor[9] if descriptor and descriptor[0]==108 else 0
            if not pointer:
                self.gap(call,'Successful acquire lacks a captured valid nonnull output/descriptor pointer; borrow origin pending');return
            if key in self.leases:
                self.gap(call,'Successful acquire overlaps an existing observed borrow; previous interval retained as unresolved')
                previous=self.leases.pop(key);previous['state']='overlapped, close unassigned';self.intervals.append(previous)
            self.leases[key]={'receiver':receiver,'kind':kind,'pointer':pointer,'thread':call['thread'],'begin_call':call['call_id'],'begin_site':call['site'],'begin_phase':call['phase'],'state':'open','descriptor':list(descriptor) if descriptor else None}
        else:
            lease=self.leases.get(key)
            if lease is None:self.gap(call,'Successful release/unlock has no selected observed acquire; origin outside capture or alias unresolved');return
            if method.endswith('ReleaseDC') and (len(args)<2 or args[1]!=lease['pointer']):
                self.gap(call,'Successful ReleaseDC handle differs from observed GetDC output; interval remains open');return
            lease=self.leases.pop(key);lease.update(state='closed client interval',end_call=call['call_id'],end_site=call['site'],end_phase=call['phase'],end_thread=call['thread'])
            self.intervals.append(lease)

    def result(self):
        return {'scope':'Selected successful original client API borrow intervals only. Failure outputs remain raw; open/overlapping/unobserved origins, external aliases/reference ownership, concurrent use, memory writers and actual destruction remain unresolved. No pixel/caller/scenario/DRAW-01 closure.',
                'calls_by_reviewed_method':dict(sorted(self.calls.items())), 'closed_or_unresolved_intervals':self.intervals,'open_intervals':list(self.leases.values()),'interface_output_observations':self.outputs,'release_observations':self.releases,'gaps':self.gaps}


def trace(path,manifest):
    verified=detail_trace(path,manifest);hooks={h['tag']:h for h in manifest['hooks']};frames={};resources=Resources()
    before=path.stat()
    with path.open('rb') as source:
        source.seek(64)
        for unused in range(verified['events']):
            row=struct.unpack('<200I',source.read(800));spec=hooks[row[3]]['details']
            if row[1]==1:frames[row[2]]=row
            else:
                pre=frames.pop(row[2]);slots=[]
                for original,event in ((pre,1),(row,2)):
                    for index,plan in enumerate(spec['snapshots']):
                        if not plan['events']&event:continue
                        at=56+index*36;payload=struct.pack('<32I',*original[at+4:at+36])
                        slots.append({'name':plan['name'],'event':event,'state':original[at+2],'data':payload[:original[at+3]].hex()})
                count=pre[33]
                resources.observe({'call_id':pre[2],'site':pre[6],'phase':pre[4],'thread':pre[5],'method':spec['name'],'args':list(pre[36:36+count]),'arguments_read':bool(pre[34]),'receiver':pre[52] if pre[54] else 0,'eax':row[10],'snapshots':slots})
    with path.open('rb') as source:digest=hashlib.file_digest(source,'sha256').hexdigest()
    after=path.stat()
    if frames or digest!=verified['journal_sha256'] or (before.st_ino,before.st_size,before.st_mtime_ns)!=(after.st_ino,after.st_size,after.st_mtime_ns):raise ValueError('Resource journal changed during independent interval review')
    return {'schema':1,'journal_integrity':True,'journal_sha256':digest,'events':verified['events'],'paired_calls':verified['paired_calls'],**resources.result()}
