"""Read the optional bounded producer state stream; observations are not proof."""
import struct

HEADER=64
ROW=768
MAX_ROWS=262144
ARGUMENTS={10:4,11:3,12:3,13:5,14:5,15:5,16:5,17:5,18:3,19:3,20:3,
           21:2,22:3,23:3,24:3,25:1,26:2,34:5,36:3,52:2,57:3,58:3,59:5,60:3}
POPS={10:16,11:12,18:12,19:12,20:12,21:8,25:4,26:8,52:8,57:12,58:12,59:20,60:12}


def fx_state(raw):
    if len(raw)!=224 or any(raw[5:8])or any(raw[220:]):
        raise ValueError('Malformed semantic floating state')
    control,status=struct.unpack_from('<HH',raw)
    tag=raw[4];top=(status>>11)&7
    registers=tuple(raw[12+i*10:22+i*10]for i in range(8))
    if any(any(value)for i,value in enumerate(registers)if not tag&(1<<((top+i)&7))):
        raise ValueError('Empty floating register has a noncanonical payload')
    return dict(control=control,status=status,tag=tag,mxcsr=struct.unpack_from('<I',raw,8)[0],
                registers=registers,xmm=raw[92:220])


def records(raw):
    versions={b'MNMCSA01':1,b'MNMCSA02':2,b'MNMCSA03':3}
    magic=bytes(raw[:8])
    if magic not in versions:raise ValueError('Unknown caller-state magic')
    version=versions[magic]
    width=832 if version==3 else ROW
    if len(raw)<HEADER+width or (len(raw)-HEADER)%width:
        raise ValueError('Truncated caller-state stream')
    h=struct.unpack_from('<14I',raw,8)
    if h[:5]!=(version,HEADER,width,MAX_ROWS,0x40209ca7)or any(h[5:]):
        raise ValueError('Unknown caller-state header contract')
    total=(len(raw)-HEADER)//width
    if total>MAX_ROWS:raise ValueError('Caller-state capacity exceeded')
    footer=struct.unpack_from('<'+str(width//4)+'I',raw,len(raw)-width)
    if footer[:6]!=(total,3,total-1,total-1,0,0)or any(footer[6:]):
        raise ValueError('Missing or incomplete normal caller-state retirement')
    seen=set()
    for at in range(HEADER,len(raw)-width,width):
        row=struct.unpack_from('<192I',raw,at);meta=row[:16]
        ident,kind,tag=meta[:3]
        if not 1<=ident<total or ident in seen or kind not in(1,2)or tag not in ARGUMENTS:
            raise ValueError('Unknown or duplicate caller-state completion')
        seen.add(ident)
        if not meta[3]or not meta[4]or not meta[5]or meta[11]!=ARGUMENTS[tag]or meta[14]!=(0 if POPS.get(tag,0)else 1):
            raise ValueError('Invalid caller identity or formal argument contract')
        if meta[6]!=meta[5]+4+POPS.get(tag,0)or row[19]!=meta[5]or row[28]!=meta[6]:
            raise ValueError('Inconsistent saved-entry/return stack contract')
        epoch=None
        if version==3:
            e=struct.unpack_from('<16I',raw,at+768)
            if any(e[13:])or any(v not in(0,1)for v in e[10:13]):
                raise ValueError('Unknown caller-state epoch contract')
            if not e[10]:
                if any(e):raise ValueError('Inactive continuous channel has epoch data')
            else:
                pairs=[e[i]|e[i+1]<<32 for i in range(0,10,2)]
                if not all(pairs)or pairs[2]<pairs[0]or pairs[3]<pairs[1]:
                    raise ValueError('Missing or backwards caller-state epoch')
                if (e[11]or e[12])and(pairs[0]!=pairs[2]or pairs[1]!=pairs[3]or e[11]!=e[12]):
                    raise ValueError('World identity changed within primitive')
                epoch=dict(generation_before=pairs[0],source_before=pairs[1],generation_after=pairs[2],source_after=pairs[3],nonce=pairs[4],world_before=bool(e[11]),world_after=bool(e[12]))
        reset=epoch is not None and (epoch['generation_before'],epoch['source_before'])!=(epoch['generation_after'],epoch['source_after'])
        if meta[7]!=meta[9]or(meta[10]<meta[8]and not reset)or not row[24]&2 or not row[33]&2:
            raise ValueError('Invalid queue/sequence or processor flags')
        count=meta[11]
        if any(row[34+count:40])or any(row[40+(count if meta[14]&1 else 0):46]):
            raise ValueError('Noncanonical unused arguments or reserved bytes')
        context_kind,parent=row[190:]
        if (version==1 and(context_kind or parent))or context_kind>4 or bool(context_kind)!=bool(parent):
            raise ValueError('Unknown or incomplete enclosing caller context')
        before=fx_state(raw[at+312:at+536]);after=fx_state(raw[at+536:at+760])
        yield dict(id=ident,kind=kind,tag=tag,thread=meta[3],caller=meta[4],entry_sp=meta[5],return_sp=meta[6],
            queue=meta[7],sequence_before=meta[8],sequence_after=meta[10],last_error_before=meta[12],last_error_after=meta[13],canvas=meta[15],
            before_registers=row[16:25],after_registers=row[25:34],before_arguments=row[34:34+count],
            after_arguments=row[40:40+count]if meta[14]&1 else None,
            before_workspace=row[46:62],after_workspace=row[62:78],before_fx=before,after_fx=after,
            context_kind=context_kind,enclosing_caller=parent,epoch=epoch)
    if len(seen)!=total-1:raise ValueError('Missing caller-state completion IDs')


def decode(raw):
    rows=list(records(raw))
    return dict(version=struct.unpack_from('<I',raw,8)[0],rows=rows,completed=len(rows),normal_retirement=True,
                scope='Saved native output before shared POP/RET, or retained normal return; no state equivalence asserted.')


def decode_bindings(raw,state):
    if len(raw)<96 or raw[:8]!=b'MNMCSB01'or (len(raw)-64)%32:
        raise ValueError('Truncated caller-binding companion')
    header=struct.unpack_from('<14I',raw,8)
    if header[:5]!=(1,64,32,MAX_ROWS,0x40209ca7)or any(header[5:]):
        raise ValueError('Unknown caller-binding header')
    total=(len(raw)-64)//32
    if total>MAX_ROWS:raise ValueError('Caller-binding capacity exceeded')
    footer=struct.unpack_from('<8I',raw,len(raw)-32)
    if footer!=(0,total-1,state['completed'],0,0,0,0,0):
        raise ValueError('Incomplete caller-binding retirement')
    expected={r['id']:r for r in state['rows']if r['context_kind']==2}
    result=[];seen=set()
    for fields in struct.iter_unpack('<8I',raw[64:-32]):
        ident=fields[0]
        if ident not in expected or ident in seen:raise ValueError('Unpaired or duplicate caller binding')
        row=expected[ident];seen.add(ident)
        if fields[:6]!=(ident,row['thread'],row['caller'],row['tag'],row['queue'],row['sequence_before']):
            raise ValueError('Caller binding identity differs from entry observation')
        result.append(dict(id=ident,word_target=fields[6],generic_target=fields[7]))
    if seen!=set(expected):raise ValueError('Missing opaque caller binding')
    return dict(version=1,rows=result,normal_retirement=True,
                scope='Raw selected targets paired with state entries; unknown targets are retained, not admitted.')
