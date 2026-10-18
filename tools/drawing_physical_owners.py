"""Strict private-allocation ownership journal; global driver ownership is separate."""
import struct
from drawing_api_session import sha


def decode(data):
    if len(data)<192 or (len(data)-64)%128 or len(data)>64+262144*128:raise ValueError('Physical owner envelope')
    h=struct.unpack_from('<16I',data)
    if data[:8]!=b'MNMPOW02' or h[2:8]!=(2,128,0x40209ca7,262144,2048,51) or any(h[8:]):raise ValueError('Physical owner header')
    owners={};seen=set();leases={};bindings=[];stacks={};active=0;calls=0
    rows=list(struct.iter_unpack('<32I',data[64:]))
    for ordinal,r in enumerate(rows,1):
        _,phase,thread,owner,submission,kind,value,reason=r[:8]
        if r[0]!=ordinal or not thread or reason or any(r[8:]):raise ValueError('Physical owner identity/refusal/reserved')
        if phase==1:
            if not owner or owner in seen or submission or kind not in (1,2) or not 1<=value<=0x40000000:raise ValueError('Physical owner generation')
            owners[owner]=dict(size=value,refs=0,kind=kind);seen.add(owner)
        elif phase==2:
            if owner not in owners or not submission or submission in leases or kind or value!=owners[owner]['refs']+1 or active or (owners[owner]['kind']==2 and 'heap_generation' not in owners[owner]):raise ValueError('Physical owner acquisition')
            owners[owner]['refs']=value;leases[submission]=dict(owner=owner,acquire_thread=thread,acquire_record=ordinal)
        elif phase==3:
            if leases.get(submission,{}).get('owner')!=owner or owner not in owners or kind or value!=owners[owner]['refs']-1:raise ValueError('Physical owner retirement')
            owners[owner]['refs']=value;bindings.append(dict(submission=submission,owner=owner,bytes=owners[owner]['size'],kind=owners[owner]['kind'],acquire_thread=leases[submission]['acquire_thread'],release_thread=thread,acquire_record=leases[submission]['acquire_record'],release_record=ordinal,heap_generation=owners[owner].get('heap_generation'),heap_offset=owners[owner].get('heap_offset')));del leases[submission]
        elif phase==9:
            if owner not in owners or owners[owner]['kind']!=2 or 'heap_generation' in owners[owner] or owners[owner]['refs'] or submission or not kind:raise ValueError('Physical client heap generation')
            owners[owner]['heap_generation']=kind;owners[owner]['heap_offset']=value
        elif phase==4:
            if owner not in owners or owners[owner]['refs'] or submission or kind not in (0,owners[owner]['kind']) or value!=owners[owner]['size']:raise ValueError('Physical owner free/reuse')
            del owners[owner]
        elif phase in (7,8):
            if owner or submission or kind>=51:raise ValueError('Physical owner worker identity')
            stack=stacks.setdefault(thread,[])
            if phase==7:
                if leases and kind not in {0,5,*range(6,35),36,37,38,39,40,42,43,44,45}:raise ValueError('Unreviewed worker admitted while leased')
                stack.append(kind);active+=1;calls+=1
            else:
                if not stack or stack.pop()!=kind:raise ValueError('Physical owner worker order')
                active-=1
            if value!=active:raise ValueError('Physical owner active worker count')
        elif phase==6:
            if ordinal!=len(rows) or owner or submission or kind or value not in (1,2) or leases or active or any(x['refs'] for x in owners.values()):raise ValueError('Physical owner terminal')
            break
        else:raise ValueError('Physical owner refused/unknown transition')
    else:raise ValueError('Physical owner missing terminal')
    return dict(schema=1,success=True,journal_sha256=sha(data),allocations=len(seen),reservations=len(bindings),worker_calls=calls,bindings=bindings,owned_private_sysmem=True,borrowed_private_client_pages=True,unknown_backing_admitted=False,global_host_driver_alias_ownership=False,native_replacement=False)


def join_reservation(binding,reservation):
    # These ordinals belong to independent journals. Preserve both domains.
    if set(binding)&set(reservation)!={'submission','release_record'} or binding['submission']!=reservation['submission']:
        raise ValueError('Physical/reservation binding fields changed')
    result={k:v for k,v in reservation.items() if k not in ('acquisition_record','release_record')}
    result.update(reservation_acquisition_record=reservation['acquisition_record'],reservation_release_record=reservation['release_record'])
    result.update(binding)
    return result


def analyze(directory,report):
    from drawing_source_reservations import decode as decode_reservations
    from drawing_gpu_completion import analyze as analyze_gpu
    result=decode((directory/'wine-physical-owners.bin').read_bytes())
    fatal=directory/'drawing-fatal.bin'
    if fatal.exists() and fatal.read_bytes():raise ValueError('Observer fatal breadcrumb retained')
    fault=directory/'wine-physical-owner-fault.bin'
    if fault.exists() and fault.read_bytes():raise ValueError('Physical backing fault retained')
    reservations=decode_reservations((directory/'wine-source-reservations.bin').read_bytes())
    reported=report['wine_source_reservation_observation']
    if any(reported.get(k)!=v for k,v in reservations.items()):raise ValueError('Physical owner reservation report changed')
    gpu=analyze_gpu(directory)
    expected={x['submission']:x for x in reservations['bindings']};actual={x['submission']:x for x in result['bindings']}
    fences={x['submission']:x for x in gpu['calls']}
    if len(actual)!=len(result['bindings']) or set(actual)!=set(expected) or len(fences)!=len(gpu['calls']) or set(fences)!=set(actual):raise ValueError('Physical owner/reservation/fence denominator')
    for submission,link in actual.items():
        original=expected[submission];fence=fences[submission]
        if fence['status']!=2 or (fence['command'],fence['worker'],fence['thread'])!=(original['command'],original['worker'],link['release_thread']):raise ValueError('Physical owner exact worker/fence retirement')
    result['bindings']=[join_reservation(x,expected[x['submission']]) for x in result['bindings']]
    result.update(reservation_journal_sha256=reservations['journal_sha256'],gpu_journal_sha256=gpu['journal_sha256'],packet_journal_sha256=gpu['packet_journal_sha256'],owned_copy_reservations=sum(x['kind']==1 for x in result['bindings']),client_copy_reservations=sum(x['kind']==2 for x in result['bindings']))
    return result


def pin_prefix_ntdll(prefix):
    """Select PE32 in a 32-bit or WOW64 prefix; system32 can be PE64."""
    candidates=[]
    for name in ('system32/ntdll.dll','syswow64/ntdll.dll'):
        path=prefix/'drive_c/windows'/name
        if not path.exists():continue
        data=path.read_bytes()
        if len(data)<64 or data[:2]!=b'MZ':raise ValueError('Invalid prefix NT image')
        pe=struct.unpack_from('<I',data,60)[0]
        if pe>len(data)-26 or data[pe:pe+4]!=b'PE\0\0':raise ValueError('Invalid prefix NT PE header')
        if struct.unpack_from('<H',data,pe+4)[0]==0x14c:candidates.append((name,sha(data)))
    if not candidates or any(digest!='6e09e71463d50fcc92259e1add310818e21628bc0a9a706f4e0c0f92540071d0' for _,digest in candidates):raise ValueError('Physical owner prefix PE32 NT module changed')
    return dict(schema=1,pe32_images=[dict(path=name,sha256=digest) for name,digest in candidates])
