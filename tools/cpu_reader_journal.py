"""Strict opt-in CPU-reader/DC sideband; observed addresses stay process-local."""
from collections import Counter
import struct
from rendering_pilot import require


def wire_reads(raw, session, count_bytes, terminal_reply):
    """Join closed CPU requests and full captured replies; no pixel oracle."""
    import hashlib
    from rendering_capture import capture
    from rendering_pilot import commands
    from storage_cpu_read_binding import counters, reply_header
    rows=capture(raw)
    require(rows[0][0]['role']=='commands' and rows[0][0]['session']==session
            and rows[-2][0]['kind']=='ended'
            and all(r['kind'] in ('fragment','cpu-read','ended') for r,_ in rows[1:-1]),
            'Reader command capture session/closure')
    replies={}
    for row,body in rows:
        if row['kind']!='cpu-read':continue
        require(row['sequence'] not in replies,'Duplicate reader reply')
        replies[row['sequence']]=(row,body)
    stream=b''.join(body for row,body in rows if row['kind']=='fragment')
    live={};generation=0;nonce=None;last_created=0;requests={};used=set();last_reply=None
    for op,seq,payload in commands(stream):
        if op==1:
            require(len(payload)>=28,'Reader CREATE extent')
            w=struct.unpack_from('<7I',payload)
            require(w[0]>last_created and 1<=w[1]<=2048 and 1<=w[2]<=2048 and w[3] in (8,16,24,32)
                    and len(payload)==28+w[1]*w[2]*(w[3]//8),'Reader CREATE identity')
            live[w[0]]=w;last_created=w[0]
        elif op==7:
            require(len(payload)==4 and struct.unpack('<I',payload)[0] in live,'Reader DELETE identity')
            del live[struct.unpack('<I',payload)[0]]
        elif op==19:
            require(len(payload)==20,'Reader WORLD extent');w=struct.unpack('<5I',payload)
            g=w[1]|w[2]<<32;n=w[3]|w[4]<<32
            require(w[0] in live and g==generation+1 and n and (nonce is None or nonce==n),'Reader WORLD identity')
            generation,nonce=g,n
        elif op==21:
            require(len(payload)==40,'Reader CPU request extent');w=list(struct.unpack('<10I',payload))
            expected=reply_header(w)
            require(w[1]==len(requests)+1 and w[0] in live and seq in replies,'Reader request/reply gap')
            s=live[w[0]]
            require(s[1:3]==tuple(w[4:6]) and s[3:]==(16,0xf800,0x7e0,31)
                    and 0<(w[6]|w[7]<<32)<=generation and (w[8]|w[9]<<32)==nonce,'Reader request source history/format')
            row,body=replies[seq]
            require(len(body)==64+w[4]*w[5]*2 and body[:64]==expected,'Reader reply identity/extent')
            requests[w[1]]=dict(words=w,sequence=seq,reply_sha256=hashlib.sha256(body).hexdigest(),payload_bytes=len(body)-64,captured_row=row['index'])
            used.add(seq);last_reply=body
        elif op==8:
            require(not live and seq==rows[-2][0]['commands'],'Reader command surface retirement')
    counts=counters(count_bytes)
    require(requests and len(requests)==len(replies)==counts['completed'] and used==set(replies),'Reader completion denominator')
    require(last_reply==terminal_reply,'Reader terminal reply differs')
    return requests,counts,hashlib.sha256(stream).hexdigest()

CAPACITY = 65536
SIZE = 128+CAPACITY*128
REASONS = {1:'GetDC',2:'Preferences preflight',3:'Save preflight',4:'Load preflight',
           5:'explicit reader',6:'ReleaseDC',7:'other materialization'}


def decode(raw, *, closed=True):
    require(len(raw)==SIZE, 'Reader journal extent')
    h=struct.unpack_from('<32I',raw)
    require(raw[:8]==b'MNMCRJ01' and h[2:6]==(1,128,128,CAPACITY), 'Reader journal version/layout')
    require(h[6]==h[7] and h[6]<=CAPACITY and not h[8] and not h[9], 'Reader journal lost, contended or uncommitted rows')
    require(h[11] and h[12] and h[13] and h[14] and h[15] and h[25]==7 and not any(h[26:]), 'Reader journal identity/gate')
    if closed:
        require(h[10]==h[18]==1 and h[19]==h[22] and not h[23] and not h[24], 'Reader journal incomplete ownership/closure')
        elapsed=(h[17]-h[16])&0xffffffff;require(elapsed<0x80000000, 'Reader journal tick ambiguity')
    rows=[];last_tick=0
    for i in range(h[6]):
        r=struct.unpack_from('<32I',raw,128+i*128)
        require(r[0]==i+1 and 1<=r[1]<=7 and r[2] and r[4]==h[11]
                and 0<r[5]<=h[22] and r[6] in REASONS, 'Reader journal row/session identity')
        if r[8]:
            require(r[7]>=r[8] and r[7]-r[8]==r[9]<r[10] and 0<r[13]<=4096,
                    'Reader journal caller module/RVA')
        else:require(r[9]==0xffffffff and not any(r[10:14]), 'Reader journal unknown module fields')
        if closed:
            tick=(r[3]-h[16])&0xffffffff
            require(last_tick<=tick<=elapsed, 'Reader journal tick ordering');last_tick=tick
        rows.append(r)
    require(not any(raw[128+h[6]*128:]), 'Reader journal unused tail changed')
    return h,rows


def chains(h, rows, wire_requests):
    """Check ticket/physical metadata and DC borrow chains; never infer callers."""
    contexts={};active={};requests={};completions={};borrows={};dc_returns={};missing=[];counts=Counter()
    for r in rows:
        event,thread,ctx=r[1],r[2],r[5];counts[event]+=1
        if event==1:
            require(ctx not in contexts and r[31]==active.get(thread,0), 'Reader context parent/duplicate')
            contexts[ctx]=dict(begin=r,events=[],end=None);active[thread]=ctx;continue
        require(ctx in contexts and active.get(thread)==ctx, 'Reader context ownership/order')
        c=contexts[ctx];b=c['begin']
        require(r[6:14]==b[6:14] and thread==b[2], 'Reader caller context changed')
        c['events'].append(r)
        if event==6:
            require(c['end'] is None,'Duplicate reader context end');c['end']=r
            if b[31]:active[thread]=b[31]
            else:del active[thread]
        elif event in (2,3):
            ticket=r[18];target=requests if event==2 else completions
            require(ticket and ticket not in target and r[15] and r[14] and r[16]
                    and r[28] and r[29]>=r[19]*2 and r[30] and not r[27]&~(3|256), 'Reader physical identity/flags')
            fields=[r[15],ticket,0,r[4],r[19],r[20],*r[21:25]]
            require(ticket in wire_requests and fields==wire_requests[ticket]['words'], 'Reader wire request differs')
            if event==2:
                require(ticket==len(requests)+1 and not r[27]&256, 'Reader request order/validation')
                if r[27]&3 in (0,3):require(r[16]==r[30], 'Reader expected source generation differs')
            else:
                require(ticket in requests and r[27]&256, 'Reader completion precedes request or lacks validation')
                q=requests[ticket]
                require(q[5]==ctx and q[2]==thread and q[14:16]==r[14:16] and q[17]==r[17]
                        and q[19:25]==r[19:25] and q[28:31]==r[28:31]
                        and (q[27]&3)==(r[27]&3), 'Reader physical lease/generation changed')
            target[ticket]=r
        elif event==7:
            require(r[31]==ctx and r[25] and r[14] and r[27]==1024 and ctx not in borrows,
                    'DC acquisition borrow identity')
            borrows[ctx]=dict(acquire=r,releases=[],retired=False)
        elif event==4:
            require(ctx not in dc_returns,'Duplicate GetDC return');dc_returns[ctx]=r
        elif event==5:
            if not r[31]:
                missing.append(dict(kind='release_without_tracked_acquisition',context=ctx));continue
            require(r[31] in borrows,'Unknown DC borrow')
            borrow=borrows[r[31]];a=borrow['acquire']
            require(not borrow['retired'] and r[25]==a[25] and r[14]==a[14] and r[27]&1024,
                    'DC release borrow/handle mismatch')
            borrow['releases'].append(r)
            if not r[26]&0x80000000:
                borrow['retired']=True
                if r[2]!=a[2] or r[17]!=a[17] or r[30]!=a[16] or not r[27]&512:
                    missing.append(dict(kind='release_checkpoint_or_owner_unproved',borrow=r[31],context=ctx))
    require(not active and len(contexts)==h[22]==h[19] and all(c['end'] is not None for c in contexts.values()),
            'Unclosed reader contexts')
    require(set(requests)==set(completions)==set(wire_requests) and len(requests)==h[20]==h[21], 'Reader request/completion coverage')
    require(all(b['retired'] for b in borrows.values()) and len(borrows)==counts[7], 'Unretired DC borrows')
    for ctx,c in contexts.items():
        b=c['begin']
        if not b[8]:missing.append(dict(kind='caller_module_unidentified',context=ctx,caller=hex(b[7])))
        if b[6]==7:missing.append(dict(kind='materialization_reason_unassigned',context=ctx))
        if b[6]==1:
            if ctx not in dc_returns:missing.append(dict(kind='GetDC_return_unobserved',context=ctx));continue
            returned=dc_returns[ctx]
            if not returned[26]&0x80000000 and ctx not in borrows:
                missing.append(dict(kind='successful_GetDC_not_tracked',context=ctx))
            if ctx in borrows:
                require(not returned[26]&0x80000000 and returned[25]==borrows[ctx]['acquire'][25], 'GetDC return/borrow disagreement')
    request_rows=[]
    for ticket,r in requests.items():
        c=contexts[r[5]];reason=REASONS[r[6]]
        request_rows.append(dict(ticket=ticket,sequence=wire_requests[ticket]['sequence'],context=r[5],reason=reason,
            caller_pc=hex(r[7]),module_base=hex(r[8]),caller_rva=None if r[9]==0xffffffff else hex(r[9]),
            module_extent=r[10],module_timestamp=r[11],module_header_fnv=r[12],module_header_bytes=r[13],
            surface=r[15],source_generation=r[16],epoch=r[17],width=r[19],height=r[20],
            source_world_generation=str(r[21]|r[22]<<32),history_nonce=str(r[23]|r[24]<<32),
            backing_token=hex(r[28]),pitch=r[29],expected_physical_generation=r[30],validated_generation=completions[ticket][30],reader_kind=r[27]&3,
            completion_sequence=completions[ticket][0],physical_validation_observed=True,
            dc_borrow=r[5] if r[6]==1 and r[5] in borrows else None,
            original_upstream_caller_proved=False))
    return dict(context_count=len(contexts),request_count=len(requests),completion_count=len(completions),
        reasons=dict(Counter(r['reason'] for r in request_rows)),requests=request_rows,
        dc_borrows=len(borrows),dc_release_attempts=counts[5],missing_links=missing,
        complete_observed_chain=not missing,original_upstream_callers_recovered=0,
        context_callers=[dict(context=ctx,reason=REASONS[c['begin'][6]],pc=hex(c['begin'][7]),
            base=hex(c['begin'][8]),rva=None if c['begin'][9]==0xffffffff else hex(c['begin'][9]),
            extent=c['begin'][10],timestamp=c['begin'][11],header_fnv=c['begin'][12],header_bytes=c['begin'][13])
            for ctx,c in contexts.items()])


def pe_identity(raw):
    require(len(raw)>=64 and raw[:2]==b'MZ','Module DOS header')
    pe=struct.unpack_from('<I',raw,60)[0]
    require(pe<=4096 and pe+88<=len(raw) and raw[pe:pe+4]==b'PE\0\0','Module PE header')
    extent,headers=struct.unpack_from('<II',raw,pe+80)
    require(0<headers<=4096 and headers<=len(raw) and extent,'Module header extent')
    value=2166136261
    for b in raw[:headers]:value=((value^b)*16777619)&0xffffffff
    return (extent,struct.unpack_from('<I',raw,pe+8)[0],value,headers)


def resolve_callers(callers, modules):
    """Unique pinned binary identity from mapped PE-header metadata, not code."""
    catalog={}
    for module in modules:
        catalog.setdefault(tuple(module['identity']),{})[module['sha256']]=module
    rows=[]
    for c in callers:
        matches=catalog.get((c['extent'],c['timestamp'],c['header_fnv'],c['header_bytes']),{}) if c['rva'] is not None else {}
        item=dict(c,identity_method='mapped PE-header metadata plus unique pinned file candidate',mapped_code_hash_verified=False)
        if len(matches)==1:
            m=next(iter(matches.values()));item.update(module=m['name'],module_sha256=m['sha256'],module_resolved=True)
        else:item.update(module_resolved=False,candidate_binary_count=len(matches))
        rows.append(item)
    return rows
