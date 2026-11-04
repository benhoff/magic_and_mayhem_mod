"""Decode bounded passive Region command-buffer submission, never delivery."""
import struct


def decode(data):
    if len(data)<16 or data[:8]!=b'MNMCOA01':
        raise ValueError('Invalid order observation prefix')
    version,row_bytes=struct.unpack_from('<II',data,8)
    if ((version,row_bytes) not in ((1,256),(2,256),(3,272))
            or (len(data)-16)%row_bytes or len(data)>16+8192*row_bytes):
        raise ValueError('Invalid order observation prefix')
    rows=[];owner=None;pending={};calls=0
    for index,r in enumerate(struct.iter_unpack('<68I' if version==3 else '<64I',data[16:]),1):
        if r[0]!=index or r[3] not in (1,2,3) or r[4]&~(39 if version==1 else 103) or version==1 and r[63] or not r[2]:
            raise ValueError('Order observation loss/overflow/foreign owner/reserved fields')
        if owner is not None and r[2]!=owner or rows and (r[1]-rows[-1]['tick'])%2**32>=2**31:
            raise ValueError('Order observation owner/clock changed')
        owner=r[2];flags=r[4]
        primary=[s for s in range(256) if r[12+s//32]&(1<<(s%32))]
        secondary=[s for s in range(256) if r[20+s//32]&(1<<(s%32))]
        if (not flags&1 and any(r[8:28]) or flags&1 and
                (not 1<=r[8]<=256 or r[9]>r[8] or len(primary)!=r[10] or len(secondary)!=r[11]
                 or any(s>=r[8] for s in primary+secondary) or max(r[10],r[11])>r[9])):
            raise ValueError('Invalid whole-pool selection sample')
        if not flags&2 and any(r[28:34]) or flags&2 and not (20<=r[33]<=1048576 and r[30]<=r[33] and 20<=r[32]<=r[33]):
            raise ValueError('Invalid command manager sample')
        if version==3 and any(r[65:]):raise ValueError('Reserved order World-return fields')
        if version==3 and rows and (r[64]-rows[-1]['world_return_ordinal'])%2**32>=2**31:
            raise ValueError('Order World-return ordinal moved backwards')
        if not flags&32 and any(r[60:63]):raise ValueError('Invalid scalar sample')
        if version>=2 and (flags&64 and (not flags&1 or r[63]>r[8]) or not flags&64 and r[63]):
            raise ValueError('Invalid original target-candidate sample')
        if r[3]==1 and (r[5] or r[6]) or r[3]!=1 and not r[5]:raise ValueError('Invalid callback identity')
        if r[3]==2:
            calls+=1
            if r[5]!=calls:raise ValueError('Callback entry lost')
            pending[r[5]]=r
        elif r[3]==3:
            before=pending.pop(r[5],None)
            if before is None or before[6]!=r[6]:raise ValueError('Unpaired callback return')
        records=[];raw=struct.pack('<24I',*r[36:60])
        if flags&4:
            if r[3]!=3 or not flags&2 or not before[4]&2 or r[34]>96 or r[35]>8:
                raise ValueError('Invalid append witness')
            if r[30]-before[30]!=r[34] or r[32]-before[32]!=r[34] or r[31]-before[31]!=r[35]:
                raise ValueError('Append counters disagree')
            offset=0
            while offset<r[34]:
                if r[34]-offset<12:raise ValueError('Truncated command header')
                opcode,length=struct.unpack_from('<II',raw,offset+4)
                if length<12 or length>r[34]-offset:raise ValueError('Invalid command length')
                record=dict(opcode=opcode,length=length,raw_hex=raw[offset:offset+length].hex())
                if opcode in (5,6,7,11,12,13) and length>=13:record['actor_slot']=raw[offset+12]
                if opcode in (7,11,13) and length>=14:record['target_slot']=raw[offset+13]
                if opcode in (5,12) and length>=21:
                    record.update(destination=list(struct.unpack_from('<3h',raw,offset+13)),mode=raw[offset+19],extra=raw[offset+20])
                records.append(record);offset+=length
            if len(records)!=r[35] or any(raw[r[34]:]):raise ValueError('Append length/count/padding disagrees')
        elif any(r[34:60]):raise ValueError('Unverified or over-budget append')
        rows.append(dict(sequence=r[0],tick=r[1],thread=r[2],kind=r[3],flags=flags,call=r[5],mask=r[6],
                         world_counter=r[7],capacity=r[8],active_count=r[9],primary=primary,secondary=secondary,
                         used=r[30],command_count=r[31],records=records,selected_count=r[60],mode=r[61],cursor=r[62],
                         candidate_valid=bool(flags&64),candidate_slot_plus_one=r[63] if version>=2 else 0))
        if version==3:rows[-1]['world_return_ordinal']=r[64]
    return dict(wire_version=version,rows=rows,pending_callbacks=sorted(pending),prefix_only=True,command_delivery_proved=False)
