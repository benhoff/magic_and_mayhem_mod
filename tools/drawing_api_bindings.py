"""Strict per-run API allocation mapping; never nearest-function attribution."""
import hashlib
import struct
from drawing_census import BUILD_SHA256
from drawing_api_journal import trampoline


def decode(path, manifest):
    before=path.stat();raw=path.read_bytes()
    hooks=manifest['hooks']
    if manifest['original_sha256']!=BUILD_SHA256 or not 0<len(hooks)<=512 or any(type(r['tag']) is not int for r in hooks) or [r['tag'] for r in hooks]!=list(range(len(hooks))):raise ValueError('Invalid binding manifest')
    if len(raw)!=192+48*len(hooks):raise ValueError('API binding extent mismatch')
    h=struct.unpack_from('<32I',raw)
    if raw[:8]!=b'MNMDAB01' or h[2:7]!=(1,128,48,len(hooks),0x400000) or raw[64:96].hex()!=BUILD_SHA256 or any(h[14:16]+h[24:]):raise ValueError('API binding header/build/reserved mismatch')
    modules={}
    for name,base,size in [('api',h[7],h[8]),('core',h[9],h[10]),('menu',h[12],h[13])]:
        if bool(base)!=bool(size) or base and base+size>0x100000000 or name=='api' and not base:raise ValueError('Invalid binding module bounds')
        modules[name]={'base':base,'size':size}
    if h[11] and not h[9]<=h[11]<h[9]+h[10]:raise ValueError('Exact core leave outside core module')
    if not h[9] and h[11]:raise ValueError('Absent core cannot have leave address')
    ranges=[]
    for name,module in modules.items():
        if module['base']:ranges.append((module['base'],module['base']+module['size'],name))
    if any(a[0]<b[1] and b[0]<a[1] for index,a in enumerate(ranges) for b in ranges[index+1:]):raise ValueError('Binding module ranges overlap')
    rows=[];allocations=[];stubs=set()
    for tag,item in enumerate(hooks):
        if any(type(item[k]) is not int for k in ('site','length','call_length')) or not 0<item['site']<=0xffffffff or not (2 if item.get('patch_mode')=='breakpoint' else 5)<=item['length']<=20 or not 2<=item['call_length']<=item['length'] or len(bytes.fromhex(item['signature']))!=item['length'] or len(bytes.fromhex(item['call']['bytes']))!=item['call_length'] or ''.join(r['bytes'] for r in [item['call']]+item['remainder'])!=item['signature']:
            raise ValueError('Binding manifest exact instruction extent mismatch')
        if item.get('patch_mode')=='breakpoint' and (item.get('patch_bytes')!=1 or item['length']!=item['call_length'] or item['remainder']):raise ValueError('Invalid breakpoint signature/continuation')
        cursor=item['site']
        for instruction in [item['call']]+item['remainder']:
            if instruction['address']!=cursor:raise ValueError('Binding manifest copied instruction address mismatch')
            cursor+=len(bytes.fromhex(instruction['bytes']))
        if cursor!=item['site']+item['length'] or cursor>0x100000000:raise ValueError('Binding manifest original address overflow')
        row=struct.unpack_from('<12I',raw,128+48*tag)
        t,site,length,call_length,base,size,rest,entry,after,pc,original,reserved=row
        expected,relative_rest=trampoline(item,0,0)
        if (t,site,length,call_length)!=(tag,item['site'],item['length'],item['call_length']) or reserved:raise ValueError('API binding exact site/tag/splice mismatch')
        if not base or base+size>0x100000000 or size!=len(expected) or rest!=base+relative_rest or pc!=base+call_length or original!=site+call_length:raise ValueError('API binding trampoline/return PC mismatch')
        api=modules['api']
        if any(not api['base']<=stub<api['base']+api['size'] or stub in stubs for stub in (entry,after)) or entry==after:raise ValueError('API binding stub identity/bounds mismatch')
        stubs.update((entry,after))
        if any(base<end and start<base+size for start,end,_ in ranges+allocations):raise ValueError('API binding allocation overlaps recorded address range')
        allocations.append((base,base+size,tag));rows.append({'tag':tag,'site':site,'length':length,'call_length':call_length,'trampoline':base,'trampoline_size':size,'remainder':rest,'before_stub':entry,'after_stub':after,'physical_call_return':pc,'original_call_return':original})
    end=struct.unpack_from('<16I',raw,128+48*len(hooks))
    if raw[-64:-56]!=b'MNMDABEN' or end[2:4]!=(1,len(hooks)) or any(end[4:]):raise ValueError('API binding terminal mismatch')
    final=path.stat()
    if (before.st_ino,before.st_size,before.st_mtime_ns)!=(final.st_ino,final.st_size,final.st_mtime_ns):raise ValueError('API binding map changed during review')
    return {'schema':1,'scope':'Prepared exact per-run allocations/module/physical CALL identities only; no installed-hook success or inferred original/logical/API/COM ownership.','integrity':True,'sha256':hashlib.sha256(raw).hexdigest(),'original_sha256':BUILD_SHA256,'modules':modules,'core_leave':h[11],'hooks':rows}


def physical(address, bindings):
    """Exact recorded return PCs first; module membership is diagnostic only."""
    if type(address) is not int or not 0<=address<=0xffffffff:raise ValueError('Invalid physical address')
    for item in bindings['hooks']:
        if address==item['physical_call_return']:return {'kind':'exact_api_trampoline_call_return','tag':item['tag'],'original_call_site':item['site'],'original_call_return':item['original_call_return']}
    if bindings['core_leave'] and address==bindings['core_leave']:return {'kind':'exact_core_leave','logical_parent':'pending_stack_thread_and_tail_flow_review'}
    for name,module in bindings['modules'].items():
        if module['base']<=address<module['base']+module['size']:return {'kind':'instrumentation_module_address','module':name,'rva':address-module['base'],'logical_owner':'unassigned'}
    for item in bindings['hooks']:
        if item['trampoline']<=address<item['trampoline']+item['trampoline_size']:return {'kind':'api_trampoline_address','tag':item['tag'],'logical_owner':'unassigned'}
    return {'kind':'unassigned_address'}
