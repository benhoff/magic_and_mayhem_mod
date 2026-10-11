"""Admit complete bounded original-active API journals; keep global scene gates separate."""
import hashlib,json,struct
from pathlib import Path
BUILD='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
PRIMARY_METHODS={5,6,7,11,26,27,31,32}
NAMES={5:'Blt',6:'BltBatch',7:'BltFast',11:'Flip',17:'GetDC',25:'Lock',26:'ReleaseDC',27:'Restore',31:'SetPalette',32:'Unlock'}
def sha(data):return hashlib.sha256(data).hexdigest()
def decode(data,oracles=None):
    if len(data)<192 or len(data)>64+262145*128 or (len(data)-64)%128:raise ValueError('Incomplete API session envelope')
    header=struct.unpack_from('<14I',data,8)
    if data[:8]!=b'MNMDAPI1' or header[:6]!=(1,128,0x40209ca7,262144,1024,1073741824) or header[6] not in (0,1,2) or any(header[7:]):raise ValueError('Unsupported API session header')
    mode=header[6]
    rows=list(struct.iter_unpack('<32I',data[64:]));frames=[];calls=[];inputs=[];actions=[];entry_ids=set();last_frames=0
    if rows[-1][1]!=4 or any(r[1]==4 for r in rows[:-1]):raise ValueError('Missing/early terminal API session record')
    for i,r in enumerate(rows,1):
        if r[0]!=i or r[1] not in (1,2,3,4,5,6) or not r[6]:raise ValueError('Invalid API session identity')
        if r[1]!=4:
            if not r[29] or r[29] in entry_ids:raise ValueError('Duplicate/missing API entry identity')
            entry_ids.add(r[29])
        if mode==2 and r[1] in (1,5):raise ValueError('Surface observation in input-only session')
        if r[1]==1:
            if r[2]>35 or not r[3] or r[13]>2:raise ValueError('Invalid surface method/generation')
            calls.append({'ordinal':i,'entry':r[29],'method':r[2],'route':NAMES.get(r[2],f'method-{r[2]}'),'object':r[3],'related':r[4],'caller':r[5],'thread':r[6],'entry_error':r[7],'hresult':r[8] if r[8]<2**31 else r[8]-2**32,'last_error':r[9],'flags':r[21],'primary':r[13]})
        elif r[1]==2:
            if r[2] not in (1,2,3,4) or r[3] or r[12]:raise ValueError('Invalid original input observation')
            if r[2] in (1,2,4) and r[21]>255:raise ValueError('Invalid consumed key identity')
            inputs.append({'ordinal':i,'method':r[2],'code':r[21],'result':r[8],'last_error':r[9],'thread':r[6],'after_frame':r[30],'cursor':list(r[27:29]) if r[2]==3 and r[31] else None})
        elif r[1]==3:
            if r[2] not in (1,2) or r[26]!=r[2] or not r[21] or r[12]:raise ValueError('Invalid applied menu phase')
            actions.append({'ordinal':i,'request':r[21],'action':r[22],'screen':r[23],'generation':r[24],'argument':r[25],'phase':r[26],'status':r[27],'thread':r[6],'after_frame':r[30]})
        if mode==0 and r[1]==1 and r[2] in PRIMARY_METHODS and not r[8]&0x80000000 and r[13]==2:raise ValueError('Unknown primary mutation classification')
        if r[12]:
            if mode:raise ValueError('Readback in metadata/input-only session')
            if r[1]!=1 or r[2] not in PRIMARY_METHODS or r[13]!=1 or r[8]&0x80000000 or r[12]!=len(frames)+1 or r[18]&0x80000000 or r[19]&0x80000000 or r[16]!=16 or not 1<=r[14]<=2048 or not 1<=r[15]<=2048:raise ValueError('Invalid primary mutation oracle')
            f={'ordinal':r[12],'record':i,'method':r[2],'width':r[14],'height':r[15],'object':r[3]}
            if oracles is not None:
                pixel=oracles(r[12]);
                if len(pixel)!=r[14]*r[15]*2:raise ValueError('Incomplete full primary plane')
                f['sha256']=sha(pixel)
            frames.append(f)
        if r[1]!=4 and (r[30]<last_frames or r[30]!=len(frames)):raise ValueError('Broken input/frame sequence')
        last_frames=len(frames)
    end=rows[-1]
    if end[2] not in (1,2) or end[21]!=len(rows)-1 or end[22]!=len(frames) or end[23] or end[24] or end[25]!=sum(f['width']*f['height']*2 for f in frames):raise ValueError('API session loss/gap/budget or terminal accounting failure')
    grouped={}
    for action in actions:grouped.setdefault(action['request'],[]).append(action)
    for pair in grouped.values():
        if len(pair)!=2 or [a['phase'] for a in pair]!=[1,2] or any(pair[0][k]!=pair[1][k] for k in ('request','action','screen','generation','argument','thread')):raise ValueError('Unpaired applied original menu action')
    return {'schema':1,'success':True,'original_sha256':BUILD,'boundary':['observed_primary_api_mutations','api_results_without_driver_readbacks','input_calls_only'][mode],'driver_readbacks_enabled':mode==0,'records':len(rows),'calls':calls,'inputs':inputs,'actions':actions,'frames':frames,'pixels_observed':sum(f['width']*f['height'] for f in frames),'drops':end[23],'unclassified_interfaces_or_routes':end[24],'all_global_presentation_routes_observed':False,'semantic_input_outcomes_complete':False,'new_native_pixel_parity':False,'original_work_bypassed':False}
def inspect(directory):
    directory=Path(directory);raw=(directory/'session.bin').read_bytes();done=(directory/'session.done').read_bytes()
    if len(done)!=128 or done!=raw[-128:]:raise ValueError('Terminal marker identity mismatch')
    result=decode(raw,lambda n:(directory/f'api-oracle-{n:06d}.565').read_bytes());result['journal_sha256']=sha(raw);return result
