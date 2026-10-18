"""Validate source snapshot, private binding, exact fenced consumption and restore."""
import struct
import zlib
from drawing_api_session import sha


def decode(data):
    if len(data)<320 or (len(data)-64)%256 or len(data)>64+131072*256:raise ValueError('Source handoff envelope')
    h=struct.unpack_from('<16I',data)
    if data[:8]!=b'MNMSHF01' or h[2:7]!=(1,256,0x40209ca7,131072,2048) or any(h[7:]):raise ValueError('Source handoff header')
    rows=list(struct.iter_unpack('<64I',data[64:]));pending={};seen=set();owners=set();bindings=[];prefix=0;count=0
    for ordinal,r in enumerate(rows,1):
        if r[0]!=ordinal or not r[2] or any(r[21:]) or r[15]<prefix:raise ValueError('Source handoff identity/reserved/prefix')
        prefix=r[15];phase=r[1]
        if phase==8:
            if ordinal!=len(rows) or pending or r[16] not in (1,2) or r[14]!=count or any(r[3:14]) or any(r[17:21]):raise ValueError('Source handoff terminal')
            break
        if phase not in range(1,8) or r[16] or r[20]!=phase or not r[3] or not r[4] or not r[5] or not r[6] or r[7] not in (1,2) or not 1<=r[8]<=800 or not 1<=r[9]<=600 or not r[8]*2<=r[10]<=32768 or r[11]!=r[8]*r[9]*2 or not r[13] or not r[14]:raise ValueError('Source handoff shape/refusal')
        if (r[7]==1 and (not r[19] or r[19]==r[13] or r[17] or r[18])) or (r[7]==2 and (not r[17] or r[19])):raise ValueError('Source handoff allocation provenance')
        identity=(r[2],r[3:12],r[13:15],r[17:20])
        if phase==1:
            count+=1
            if r[3] in seen or r[13] in owners or r[14]!=count or r[12]:raise ValueError('Source handoff duplicate/epoch')
            seen.add(r[3]);owners.add(r[13]);pending[r[3]]=dict(identity=identity,state=1,prefixes={1:r[15]},kind=r[7],owner=r[13],submission=r[3],texture=r[4],generation=r[5],resource=r[6],width=r[8],height=r[9],pitch=r[10],bytes=r[11],heap_generation=r[17],heap_offset=r[18],original_owner=r[19])
        else:
            s=pending.get(r[3])
            if not s or s['identity']!=identity or s['state']!=phase-1:raise ValueError('Source handoff transition changed')
            if phase==2 and r[12]:raise ValueError('Source handoff premature CRC')
            if phase==3:
                if r[15]!=s['prefixes'][2]:raise ValueError('Original callback ran inside snapshot')
                s['crc32']=r[12]
            elif phase>3 and r[12]!=s['crc32']:raise ValueError('Source handoff value changed')
            s['prefixes'][phase]=r[15];s['state']=phase
            if phase==7:
                bindings.append({k:v for k,v in s.items() if k not in ('identity','state')});del pending[r[3]]
    else:raise ValueError('Source handoff missing terminal')
    return dict(schema=1,success=True,journal_sha256=sha(data),handoffs=count,bindings=bindings,original_aliases_retained_as_consumer_inputs=False,source_binding_restored_before_api_return=True,original_pixels_used_as_native_inputs=False,global_host_driver_alias_ownership=False,native_replacement=False)


def analyze(directory,report):
    from drawing_physical_owners import analyze as physical
    from drawing_wine_source_map import decode as source_map
    result=decode((directory/'wine-source-handoff.bin').read_bytes());owned=physical(directory,report)
    snapshots=source_map((directory/'wine-source-map.bin').read_bytes(),directory)
    by_submission={x['submission']:x for x in owned['bindings']};samples={x['submission']:x for x in snapshots['samples']};payloads={x['ordinal']:x for x in snapshots['payloads']}
    if set(by_submission)!={x['submission'] for x in result['bindings']} or set(samples)!=set(by_submission):raise ValueError('Source handoff consumer/snapshot denominator')
    physical_rows=list(struct.iter_unpack('<32I',(directory/'wine-physical-owners.bin').read_bytes()[64:]));free={r[3]:r[0] for r in physical_rows if r[1]==4}
    for h in result['bindings']:
        link=by_submission[h['submission']];sample=samples[h['submission']]
        if link['owner']!=h['owner'] or link['kind']!=1 or link['source_texture']!=h['texture'] or link['source_generation']!=h['generation'] or link['resource']!=h['resource']:raise ValueError('Source handoff private consumer identity')
        p=h['prefixes']
        if not p[3]<link['acquire_record']<=p[4]<=link['release_record']<=p[5]<=p[6]<free.get(h['owner'],0)<=p[7]:raise ValueError('Source handoff transfer/fence/restore/free order')
        payload=payloads[sample['payload_ordinal']];raw=(directory/payload['path']).read_bytes()
        if zlib.crc32(raw)!=h['crc32'] or (payload['width'],payload['height'],len(raw))!=(h['width'],h['height'],h['bytes']):raise ValueError('Source handoff reference bytes changed')
        h.update(command=link['command'],worker=link['worker'],release_thread=link['release_thread'],payload_ordinal=sample['payload_ordinal'],payload_sha256=payload['sha256'])
    result.update(physical_journal_sha256=owned['journal_sha256'],source_map_journal_sha256=snapshots['journal_sha256'],gpu_journal_sha256=owned['gpu_journal_sha256'],private_copy_value_ownership_closed=True)
    return result
