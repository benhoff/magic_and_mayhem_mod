"""Strict finite frame/input witnesses; pool slots are not allocation generations."""
import struct
from rendering_pilot import require


def signed(n):
    return n if n < 2**31 else n-2**32


def decode(raw,allow_pending=False):
    require(len(raw)>=16 and raw[:8]==b'MNMIFR01' and struct.unpack_from('<II',raw,8)==(1,384)
            and (len(raw)-16)%384==0 and len(raw)<16+8192*384,'Incomplete/overflow frame-input stream')
    rows=[];owner=None;nonce=None;pairs={};previous_generation=0
    for ordinal,r in enumerate(struct.iter_unpack('<96I',raw[16:]),1):
        require(r[0]==ordinal and r[2] in (1,2,3) and r[5]>0 and not r[7]&~511 and not r[95], 'Frame/input identity/flags/reserved fields')
        require(owner is None or owner==r[5], 'Frame/input owner changed')
        owner=r[5]
        if rows:require((r[4]-rows[-1]['tick'])%2**32<2**31,'Frame/input clock moved backwards')
        frame=None
        if r[7]&256:
            require(r[56:59]==(1,32,0x40209ca7) and (r[59] or r[60]) and (r[61] or r[62]) and r[63]==owner,'Invalid source frame identity')
            generation=r[59]+(r[60]<<32);session=r[61]+(r[62]<<32)
            require(generation>=previous_generation and (nonce is None or nonce==session),'Source generation/session changed')
            nonce=session;previous_generation=generation
            frame=dict(generation=generation,nonce=str(session),owner=owner)
        else:require(not any(r[56:64]),'Unadmitted source frame has data')
        cursor=None
        require(r[64] in (0,1),'Cursor observation result')
        if r[64]:
            p=r[65:81]
            require(p[:5]==(1,64,0x40209ca7,1,owner) and p[5]>0 and p[6]>0 and not p[6]&1
                    and p[10] in (0,1) and p[11]==1 and p[12]>0 and not any(p[13:]),'Invalid completed native cursor observation')
            cursor=dict(poll=p[5],input_sequence=p[6],tick=p[7],point=[signed(p[8]),signed(p[9])],desktop=bool(p[10]),caller=p[12])
        else:require(not any(r[65:81]),'Unadmitted cursor has data')
        require(r[24] in (0,1,2) and r[45]&~7==0 and r[45]&4 and r[55]==0,'Invalid hit/tracked request')
        tracked=None;hit=None
        if r[45]&2:
            require(r[45]&1 and r[46]>0 and 1<=r[48]<=r[47]<=256 and r[48]==1,'Invalid tracked pool/slot')
            tracked=dict(pool=r[46],capacity=r[47],slot=r[48]-1,type=r[49],owner=r[50],active=r[51],health=r[52],flag68=r[53],flag70=r[54])
        if r[24]==1:
            require(r[25]>0 and 1<=r[27]<=r[26]<=256,'Invalid hit pool/slot')
            hit=dict(pool=r[25],capacity=r[26],slot=r[27]-1,type=r[28],owner=r[29],active=r[30],health=r[31],flag68=r[42],flag70=r[43])
            if tracked:
                require((hit['pool'],hit['capacity'])==(tracked['pool'],tracked['capacity']),'Hit/tracked pool mismatch')
                if hit['slot']==tracked['slot']:require(hit==tracked,'Same actor fields disagree')
        if r[2]==1:require(r[1]==0 and r[6]==0 and not any(r[8:18]),'World observation contains callback arguments')
        else:
            require(r[1]>0,'Callback call identity absent')
            if r[2]==2:
                require(r[1] not in pairs,'Duplicate callback entry');pairs[r[1]]=[r]
            else:
                require(r[1] in pairs and len(pairs[r[1]])==1,'Missing/duplicate callback return')
                before=pairs[r[1]][0]
                require(before[6]==r[6] and before[8:18]==r[8:18] and before[7]&1==r[7]&1,'Callback argument identity changed')
                pairs[r[1]].append(r)
        require(all(r[i] in (0,1) for i in (81,85,89,93)), 'Scalar validity differs')
        row=dict(ordinal=ordinal,call=r[1],kind=r[2],world_return=r[3],tick=r[4],thread=r[5],mask=r[6],flags=r[7],mouse=list(r[8:18]),
                 frame=frame,cursor=cursor,hit=hit,hit_class=r[22],draw_index=r[23],tracked=tracked,
                 camera=list(r[32:39]),mode=r[39],cursor_type=r[40],ring=r[41],
                 saved_cursor=[r[82],signed(r[83]),signed(r[84])] if r[81] else None,
                 position=list(r[86:89]) if r[85] else None,history=[signed(r[90]),signed(r[91]),r[92]] if r[89] else None,
                 high_resolution=r[94] if r[93] else None)
        rows.append(row)
    require(allow_pending or all(len(v)==2 for v in pairs.values()),'Pending original callback')
    return dict(rows=rows,owner=owner,nonce=str(nonce) if nonce else None,callbacks=len(pairs),allocation_generations_proved=False)
