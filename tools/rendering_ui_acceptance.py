"""Strict original UI callback/World observations; no interpolated frame identity."""
import struct
from rendering_pilot import require

PREFIX='research/runtime/rendering-ui-results-20261107/'
LEAVES={'journey.hud-minimap-tooltips.hud-update',
        'journey.hud-minimap-tooltips.tooltip-open-close',
        'journey.pause-preferences.pause'}


def decode(raw,allow_pending=False):
    require(raw[:8]==b'MNMUIA01' and len(raw)>=16 and struct.unpack_from('<2I',raw,8) in ((1,512),(2,512),(3,512))
            and (len(raw)-16)%512==0 and len(raw)<16+8192*512,'UI witness envelope/loss')
    version=struct.unpack_from('<I',raw,8)[0]
    rows=[];owner=None;pairs={};previous_tick=None;last_world=0
    for ordinal,w in enumerate(struct.iter_unpack('<128I',raw[16:]),1):
        require(w[0]==ordinal and w[1] in range(1,7) and w[4]>0 and w[50]&~(63 if version==3 else 31)==0,'UI identity/kind/validity')
        require(owner is None or owner==w[4],'UI original owner changed');owner=w[4]
        require(previous_tick is None or (w[3]-previous_tick)%2**32<2**31,'UI clock reordered');previous_tick=w[3]
        require(not any(w[8:10]) and (version==3 or not w[7]) and not w[19] and (version>=2 or not (w[6] or w[18])) and not w[36] and not w[45] and not w[46], 'UI reserved data')
        if version==3:
            require((w[50]&32 and w[10] in (0,1)) or (not w[50]&32 and not (w[10] or w[7])),'UI World pause byte/validity')
        frame=None
        if w[50]&4:
            require(w[56:59]==(1,32,0x40209ca7) and w[63]==owner,'UI foreign frame identity')
            g=w[59]+(w[60]<<32);nonce=w[61]+(w[62]<<32)
            require(g>0 and nonce>0 and g>=last_world,'UI frame generation regressed');last_world=g
            frame=dict(generation=g,nonce=str(nonce),owner=owner)
        else: require(not any(w[56:64]),'UI invalid frame populated')
        if w[1]==1:
            require(frame and (w[5]==frame['generation'] or w[16]!=2 and w[5]==frame['generation']+1),
                    'UI World return has neither its sealed generation nor a terminal menu transfer')
        text=raw[16+(ordinal-1)*512+256:16+ordinal*512]
        require(w[50]&8 and w[47]<256 and text[w[47]]==0 and not any(text[w[47]:]),'UI unterminated/truncated text')
        require(w[20] in (0,1) and w[21] in (0,1),'UI tooltip state')
        if w[1] in (2,3,5,6):
            require(w[2]>0,'UI callback identity absent')
            if w[1] in (2,5):require(w[2] not in pairs,'UI duplicate callback entry');pairs[w[2]]=[w[1]]
            else:
                require(w[2] in pairs and pairs[w[2]]==[w[1]-1],'UI missing/duplicate callback return');pairs[w[2]].append(w[1])
        rows.append(dict(ordinal=ordinal,kind=w[1],call=w[2],tick=w[3],thread=w[4],world_returns=w[5],
            frame=frame,mode=w[10] if version==3 and w[50]&32 else None,input_mode=w[18] if version>=2 else w[10],accepted_sequence=w[7] if version==3 else w[10] if version==2 else None,counter=w[11],menu=dict(screen=w[12],generation=w[13],request=w[14],status=w[15],top=w[16],depth=w[17]),
            tooltip=dict(visible=w[20],pending=w[21],font=w[22],font_raw=w[23],palette=w[24],surface=w[25],
                width=w[26],height=w[27],source=list(w[28:32]),destination=list(w[32:36]),text=text[:w[47]].decode('cp1252')),
            wizard=dict(address=w[37],slot=w[38],type=w[39],owner=w[40],active=w[41],health=w[42],mana=w[43] if version>=2 else None,maximum_mana=w[6] if version>=2 else w[43],selected=w[44]) if w[50]&2 else None,
            widget=dict(address=w[51],cached_health=w[52],maximum=w[53],actor=w[54]) if w[50]&16 else None,
            argument=w[55],result=w[48],last_error=w[49],validity=w[50]))
    require(allow_pending or all(len(p)==2 for p in pairs.values()),'UI pending callback')
    return dict(wire_version=version,rows=rows,owner=owner,calls=len(pairs))


def decode_keys(raw):
    require(len(raw)>=64 and raw[:8]==b'MNMUIK01','UI original key header')
    h=struct.unpack_from('<16I',raw)
    require(h[2:4]==(1,48) and 0<h[4]<h[5]==64 and not any(h[6:])
            and len(raw)==64+h[4]*48,'UI original key extent/loss')
    result=[];down={};owner=None;previous=None
    for ordinal,w in enumerate(struct.iter_unpack('<12I',raw[64:]),1):
        require(w[0]==ordinal and w[2]>0 and w[3] in (27,32) and w[4]<=65535 and w[5] in (0,1)
                and (w[6]>0 and not w[6]&1 if w[5] else w[6]==0)
                and 0x400000<=w[7]<0x5d0000 and not any(w[8:]),'UI original key caller/result/ownership')
        require(owner is None or owner==w[2],'UI key original owner changed');owner=w[2]
        require(previous is None or (w[1]-previous)%2**32<2**31,'UI key clock regressed');previous=w[1]
        pressed=bool(w[4]&0x8000);require(pressed!=down.get(w[3],False),'UI duplicate/missing key transition');down[w[3]]=pressed
        result.append(dict(ordinal=ordinal,tick=w[1],thread=w[2],code=w[3],value=w[4],owned=bool(w[5]),input_sequence=w[6],caller=w[7],down=pressed))
    require(not down.get(32,False),'UI terminal Space release was not polled')
    return dict(rows=result,owner=owner,escape_release_polled=not down.get(27,False))
