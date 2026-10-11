"""Strict V2 formal-layout/bounded-buffer diagnostics, independent from V1."""
from collections import Counter
import hashlib
import struct
from drawing_census import BUILD_SHA256
from drawing_api_contracts import contract,validate,UNKNOWN_WORDS
from drawing_api_pairs import ApiPairs

RECORD_BYTES=800
MAX_RECORDS=1048576


class Details:
    def __init__(self,manifest):
        if type(manifest.get('synthetic_fixture',False)) is not bool:raise ValueError('V2 synthetic namespace flag must be boolean')
        self.hooks={row['tag']:row for row in manifest['hooks']};self.frames={}
        self.layouts=Counter();self.buffers=Counter();self.receiver_calls=0
        for hook in self.hooks.values():
            validate(hook['details'])
            if not manifest.get('synthetic_fixture',False) and hook['details']!=contract(hook):
                raise ValueError('V2 metadata differs from reviewed exact CALL contract')

    def accept(self,row):
        if len(row)!=200:raise ValueError('V2 record extent mismatch')
        cid,tag,event,phase,site=row[2],row[3],row[1],row[4],row[6]
        if tag not in self.hooks or event not in (1,2):raise ValueError('V2 detail identity mismatch')
        spec=self.hooks[tag]['details'];words=spec['formal_words'];count=16 if words==UNKNOWN_WORDS else words;plans=spec['snapshots']
        if tuple(row[32:34])!=(words,count) or row[35]!=len(plans) or row[55] or row[54] not in (0,1):
            raise ValueError('V2 formal layout/snapshot/candidate fields mismatch')
        args=tuple(row[36:52]);candidate=tuple(row[52:55])
        if not row[54] and any(row[52:54]) or row[54] and not all(row[52:54]):raise ValueError('V2 receiver candidate state invalid')
        if event==1:
            if cid in self.frames or row[34] not in (0,1) or not row[34] and any(args) or any(args[count:]):
                raise ValueError('V2 BEFORE argument availability/unused fields invalid')
            if row[34] and row[17] and tuple(row[18:18+min(count,8)])!=args[:min(count,8)]:
                raise ValueError('V2 formal/diagnostic stack disagreement')
            if row[54]:
                mode=self.hooks[tag]['target_mode']
                if not row[34] or not count or row[52]!=args[0] or mode['kind']!='register_memory' or (row[53]+mode['displacement'])&0xffffffff!=row[15]:
                    raise ValueError('V2 receiver candidate lacks exact BEFORE slot equality')
            saved=(tag,phase,site,row[34],args,candidate,row[27]);self.frames[cid]=saved
            self.layouts[(phase,site,words,row[34])]+=1;self.receiver_calls+=row[54]
        else:
            saved=self.frames.pop(cid,None)
            if not saved or saved[:3]!=(tag,phase,site) or row[34] or any(args) or candidate!=saved[5]:
                raise ValueError('V2 AFTER saved argument/candidate identity mismatch')
            if words!=UNKNOWN_WORDS and (row[7]-saved[6])&0xffffffff!=words*4:
                raise ValueError('V2 actual cleanup disagrees with reviewed PE32 stdcall layout')
        for index in range(4):
            slot=row[56+index*36:92+index*36]
            if index>=len(plans) or not plans[index]['events']&event:
                if any(slot):raise ValueError('V2 unused/non-event snapshot must be zero')
                continue
            plan=plans[index];address,requested,state,captured=slot[:4];payload=struct.pack('<32I',*slot[4:])
            available=saved[3];original_args=saved[4]
            if requested!=plan['bytes'] or state not in (1,2,3,4):raise ValueError('V2 snapshot plan/state mismatch')
            if not available:
                if (address,state,captured)!=(0,4,0):raise ValueError('V2 unavailable arguments cannot supply snapshot pointer')
            else:
                expected=original_args[plan['argument']]
                if address!=expected or state==4 or bool(address)!=(state!=2):raise ValueError('V2 snapshot lacks exact saved pointer/null identity')
                if state==1 and (address+requested>=0x100000000 or captured!=requested):raise ValueError('V2 readable snapshot extent invalid')
                if state!=1 and captured:raise ValueError('V2 failed snapshot has captured bytes')
            if any(payload[captured:]):raise ValueError('V2 snapshot unused payload must be zero')
            self.buffers[(phase,site,event,index,address,requested,state,payload[:captured].hex())]+=1

    def result(self):
        if self.frames:raise ValueError('V2 detail frames remain open')
        return {'receiver_candidate_calls':self.receiver_calls,
                'argument_layouts':[{'phase':p,'site':s,'formal_words':w,'arguments_read':bool(r),'calls':n} for (p,s,w,r),n in sorted(self.layouts.items())],
                'buffer_snapshots':[{'phase':p,'site':s,'event':e,'slot':i,'address':a,'requested_bytes':b,'state':state,'data':data,'count':n} for (p,s,e,i,a,b,state,data),n in sorted(self.buffers.items())]}


def trace(path,manifest):
    pairs=ApiPairs(manifest);details=Details(manifest)
    if not 0<len(pairs.hooks)<=512 or manifest.get('journal_version')!=2:raise ValueError('V2 hook/version declaration invalid')
    before=path.stat();size=before.st_size
    if size<128 or size>128+MAX_RECORDS*RECORD_BYTES or (size-128)%RECORD_BYTES:raise ValueError('V2 journal extent invalid')
    with path.open('rb') as source:
        header=source.read(64);h=struct.unpack('<16I',header)
        if header[:8]!=b'MNMDAPI2' or h[2:8]!=(2,64,RECORD_BYTES,0x400000,len(pairs.hooks),MAX_RECORDS) or header[32:].hex()!=BUILD_SHA256:
            raise ValueError('V2 header/layout/build/budget mismatch')
        while source.tell()<size-64:
            row=struct.unpack('<200I',source.read(RECORD_BYTES));pairs.accept(row[:32]);details.accept(row)
        terminal=source.read(64);end=struct.unpack('<16I',terminal)
        if terminal[:8]!=b'MNMDAEND':raise ValueError('V2 terminal magic mismatch')
        pairs.finish(end,2);extra=details.result()
    with path.open('rb') as source:digest=hashlib.file_digest(source,'sha256').hexdigest()
    after=path.stat()
    if (before.st_ino,before.st_size,before.st_mtime_ns)!=(after.st_ino,after.st_size,after.st_mtime_ns):raise ValueError('V2 journal changed during review')
    return {'schema':2,'scope':'Closed V2 API diagnostic pairs with reviewed exact layouts and bounded raw buffers. Receiver candidates are byte equality only; failed outputs, external object lifetime, full semantics/graphics/API/writer/scenario closure are not inferred.','journal_integrity':True,'original_sha256':BUILD_SHA256,'journal_sha256':digest,**pairs.result(),'details':extra}
