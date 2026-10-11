"""Exact per-run core splice bytes/return identities; no guessed ownership."""
import hashlib
import struct
from drawing_census import BUILD_SHA256


def relocated(item, base):
    """Reconstruct the reviewed prefix and its exact continuation jump."""
    keys = ('tag', 'entry', 'length', 'pop')
    if any(type(item[k]) is not int for k in keys) or not 0 <= item['tag'] < 256 or not 0 < item['entry'] <= 0xffffffff or not 5 <= item['length'] <= 16 or not 0 <= item['pop'] <= 0xffff or item['pop'] % 4 or type(item['site']) is not bool or item['site'] and item['pop']:
        raise ValueError('Invalid core binding manifest identity/RET/SITE')
    signature = bytes.fromhex(item['signature'])
    if len(signature) != item['length'] or item['entry'] + len(signature) + 5 > 0x100000000:
        raise ValueError('Invalid core prefix extent')
    offsets = item['relocations']
    if type(offsets) is not list or len(offsets) > 3 or any(type(at) is not int or not 0 <= at <= len(signature)-5 for at in offsets) or offsets != sorted(set(offsets)) or any(b-a < 5 for a,b in zip(offsets,offsets[1:])):
        raise ValueError('Invalid core relative instruction offsets')
    code = bytearray(signature)
    for at in offsets:
        if signature[at] not in (0xe8, 0xe9): raise ValueError('Core relocation is not a reviewed E8/E9')
        target = (item['entry'] + at + 5 + struct.unpack_from('<i', signature, at+1)[0]) & 0xffffffff
        struct.pack_into('<I', code, at+1, (target-base-at-5) & 0xffffffff)
    code += b'\xe9' + struct.pack('<I', (item['entry']+len(signature)-base-len(signature)-5) & 0xffffffff)
    return bytes(code)


def decode(path, manifest):
    before = path.stat(); raw = path.read_bytes(); hooks = manifest['hooks']
    if manifest.get('schema') != 1 or manifest['original_sha256'] != BUILD_SHA256 or not 0 < len(hooks) <= 256 or [r['tag'] for r in hooks] != list(range(len(hooks))): raise ValueError('Invalid core binding manifest')
    if len(raw) != 192 + 96*len(hooks): raise ValueError('Core binding extent mismatch')
    h = struct.unpack_from('<32I', raw)
    if raw[:8] != b'MNMDCB01' or h[2:7] != (1,128,96,len(hooks),0x400000) or raw[64:96].hex() != BUILD_SHA256 or any(h[10:16]+h[24:]): raise ValueError('Core binding layout/build/reserved mismatch')
    base,size,leave = h[7:10]
    if not base or not size or base+size > 0x100000000 or not base <= leave < base+size: raise ValueError('Core module/leave bounds invalid')
    ranges = [(base,base+size)]; stubs = {leave}; entries = set(); rows = []
    for tag,item in enumerate(hooks):
        row = struct.unpack_from('<24I',raw,128+96*tag)
        t,entry,length,pop,site,allocation,extent,stub,nrel = row[:9]
        if (t,entry,length,pop,site,nrel) != (tag,item['entry'],item['length'],item['pop'],int(item['site']),len(item['relocations'])) or any(row[22:]): raise ValueError('Core binding exact hook identity/reserved mismatch')
        expected = relocated(item,allocation)
        if row[9:12] != tuple(item['relocations']+[0]*(3-nrel)): raise ValueError('Core relocation offsets/padding mismatch')
        if raw[128+96*tag+48:128+96*tag+64] != bytes.fromhex(item['signature']).ljust(16,b'\0') or raw[128+96*tag+64:128+96*tag+88] != expected.ljust(24,b'\0'): raise ValueError('Core relocated bytes/continuation/padding mismatch')
        if entry in entries or not allocation or extent != length+5 or allocation+extent > 0x100000000 or any(allocation < end and start < allocation+extent for start,end in ranges): raise ValueError('Core allocation/entry bounds overlap or overflow')
        if not base <= stub < base+size or stub in stubs: raise ValueError('Core entry stub bounds/identity mismatch')
        entries.add(entry);stubs.add(stub);ranges.append((allocation,allocation+extent))
        calls=[]
        original=bytes.fromhex(item['signature'])
        for at in item['relocations']:
            if original[at] == 0xe8:
                calls.append({'offset':at,'original_call_site':entry+at,'original_call_return':entry+at+5,'physical_call_return':allocation+at+5,'target':(entry+at+5+struct.unpack_from('<i',original,at+1)[0])&0xffffffff})
        rows.append({'tag':tag,'entry':entry,'length':length,'pop':pop,'site':bool(site),'trampoline':allocation,'trampoline_size':extent,'entry_stub':stub,'relocated_calls':calls})
    end=struct.unpack_from('<16I',raw,128+96*len(hooks))
    if raw[-64:-56] != b'MNMDCBEN' or end[2:4] != (1,len(hooks)) or any(end[4:]): raise ValueError('Core binding terminal mismatch')
    after=path.stat()
    if (before.st_ino,before.st_size,before.st_mtime_ns)!=(after.st_ino,after.st_size,after.st_mtime_ns): raise ValueError('Core bindings changed during review')
    return {'schema':1,'integrity':True,'sha256':hashlib.sha256(raw).hexdigest(),'original_sha256':BUILD_SHA256,'core_module':{'base':base,'size':size},'core_leave':leave,'hooks':rows,'scope':'Prepared exact per-run core allocations, copied bytes, named leave and relocated E8 identities only. No installed-hook, inferred logical tail/original owner, side-effect or scenario closure assertion.'}


def physical(address, bindings):
    if type(address) is not int or not 0 <= address <= 0xffffffff: raise ValueError('Invalid physical address')
    for item in bindings['hooks']:
        for call in item['relocated_calls']:
            if address == call['physical_call_return']: return {'kind':'exact_core_trampoline_call_return','tag':item['tag'],**call}
    if address == bindings['core_leave']: return {'kind':'exact_core_leave','logical_parent':'pending_stack_thread_and_tail_flow_review'}
    module=bindings['core_module']
    if module['base'] <= address < module['base']+module['size']:return {'kind':'instrumentation_module_address','module':'core','rva':address-module['base'],'logical_owner':'unassigned'}
    for item in bindings['hooks']:
        if item['trampoline'] <= address < item['trampoline']+item['trampoline_size']:return {'kind':'core_trampoline_address','tag':item['tag'],'logical_owner':'unassigned'}
    return {'kind':'unassigned_address'}
