"""Strict passive font lifecycle journal; pixel/DC acceptance is independent."""
import struct

LIMIT=16*1024**2
ROWS=65536

def require(value,message):
    if not value:raise ValueError(message)

def decode(raw):
    require(192<=len(raw)<=LIMIT+128,'Font journal extent')
    header=struct.unpack_from('<16I',raw)
    version=header[2]
    require(version in (1,2) and raw[:8]==(b'MNMFLT01' if version==1 else b'MNMFLT02') and header[2:7]==(version,64,0x40209ca7,LIMIT,ROWS)
            and not any(header[7:]),'Font journal header')
    at=64;rows=[];pending={};stack=[];owner=None;tokens={};current={};loads=[];uses=[];retired=[];terminal=None;frames={}
    while at<len(raw):
        require(len(raw)-at>=128,'Truncated font record')
        r=struct.unpack_from('<32I',raw,at)
        require(128<=r[0]<=128+1048576 and r[0]<=len(raw)-at and r[1]==len(rows)+1
                and r[2] in (1,2,3,4),'Font record order/extent')
        if r[2]==4:
            require(r[0]==128 and at+128==len(raw) and r[3]==owner and r[5]==0
                    and r[6]==len(rows) and r[7]==at and r[8]==len(tokens)
                    and not r[9] and not pending and not stack
                    and r[10]==len(current) and not any(r[11:]),'Font terminal loss/lifetime accounting')
            terminal=r;at+=128;break
        require(r[3] and r[5]<=10 and r[6]<=64 and r[8]<=4 and not any(r[25:] if version==2 else r[24:])
                and (r[2]==3 or not r[24]),'Font record domain')
        owner=owner or r[3];require(owner==r[3],'Font owner changed')
        if r[2]==1:
            require(r[0]==(384 if r[5]==8 else 128) and r[7]==current.get(r[6],0),'Font entry resource generation')
            body=raw[at+128:at+r[0]]
            if body:require(b'\0' in body,'Unterminated font path')
            pending[r[1]]=r;stack.append(r[1])
        elif r[2]==2:
            require(r[0]==128 and stack and stack[-1]==r[10] and r[10] in pending,'Font return is not nested with its entry')
            entry=pending.pop(r[10]);stack.pop()
            require(r[5:10]==entry[5:10],'Font return identity')
            if r[5]==8:
                token=r[13];prior=r[7]
                if token and token!=prior:
                    require(token==len(tokens)+1 and token not in tokens,'Reused font generation')
                    tokens[token]=dict(font=r[6],role=r[8],entry=r[10],success=bool(r[15]))
                require(bool(r[16])==bool(prior and prior!=token),'Font replacement flag')
                if token:current[r[6]]=token
                else:current.pop(r[6],None)
                loads.append(dict(ordinal=r[1],font=r[6],role=r[8],token=token,prior=prior,
                                  success=bool(r[15]),replacement_preserves_old_allocation_question=bool(r[16])))
            elif r[5]<8:
                if r[5]&1:retired.append(dict(ordinal=r[1],font=r[6],token=r[7],role=r[8]))
                current.pop(r[6],None)
        elif r[2]==3:
            require(r[5]==9 and r[0]==128+(r[15] if version==2 and r[24] else 0)
                    and (not r[24] or version==2 and r[24]==1),'Font glyph extent/tag')
            if not r[23] or version==2 and r[23]==2:
                parent=(r[10] in pending and pending[r[10]][5]==9 and r[6]==pending[r[10]][6]) if not r[23] else r[10]==r[13]==0
                require(parent and r[7] in tokens and current.get(r[6])==r[7]
                        and tokens[r[7]]['font']==r[6] and tokens[r[7]]['role']==r[8]
                        and 40<=r[15]<=1048576,'Font glyph lacks current live byte caller')
                key=(r[7],r[14])
                if version==2:
                    if r[24]:
                        body=raw[at+128:at+r[0]]
                        require(key not in frames and len(body)==r[15] and struct.unpack_from('<I',body)[0]==r[15]
                                and not any(body[28:32]),'Glyph snapshot identity')
                        frames[key]=body
                    require(key in frames and len(frames[key])==r[15],'Glyph has no earlier complete source snapshot')
                uses.append(dict(ordinal=r[1],font=r[6],role=r[8],token=r[7],byte=r[13],
                                 offset=r[14],bytes=r[15],normalized_frame_fnv=r[16],
                                 x=r[19],y=r[20],queue=r[21],sequence_before=r[22],caller=r[9],direct_range_binding=r[23]==2))
            else:require(r[23]==1 and not any(r[6:9]) and not any(r[10:19]),'Malformed unparented glyph')
        rows.append(r);at+=r[0]
    require(terminal is not None,'Missing font terminal')
    return dict(success=True,version=version,rows=rows,loads=loads,uses=uses,retired=retired,frame_snapshots=frames,
                tokens=tokens,live_at_detach=current,owner=owner,complete=True,
                normalized_hash_is_identity_proof=False,pixel_output_verified=False,
                dc_image_lifetime_verified=False,acceptance_ready=False)
