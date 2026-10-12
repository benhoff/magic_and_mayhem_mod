"""Prospective strict MNMDGT01diagnostic journal, no successful-prefix admission."""
import struct

def decode(data):
    if len(data)<64 or data[:8]!=b'MNMDGT01':raise ValueError('Gate journal magic/header')
    header=struct.unpack('<16I',data[:64])
    if header[2:6]!=(1,192,0x40209ca7,8192) or any(header[12:]):raise ValueError('Gate journal header fields')
    count,drops,reason,active,fault,orphans=header[6:12]
    if count>8192 or len(data)!=64+count*192:raise ValueError('Gate journal extent')
    rows=[]
    for ordinal in range(count):
        row=struct.unpack('<48I',data[64+ordinal*192:64+(ordinal+1)*192])
        if row[0]!=ordinal+1 or row[1] not in range(1,10) or not row[2] or any(row[32:]):raise ValueError('Gate journal row fields')
        point=row[16:32]
        if any(point):
            if point[0:3]!=(1,128,0x40209ca7) or point[5]!=row[5] or point[7]!=row[6] or point[8]!=row[7]:raise ValueError('Gate point binding')
        rows.append(dict(ordinal=row[0],phase=row[1],owner=row[2],tick=row[3],reason=row[4],token=row[5],outer_identity=row[6],inner_identity=row[7],original_result=row[8],diagnostic_result=row[9],last_error=row[10],role_mask=row[11],call_identity=row[12],mutex_identity=row[13],pending_wrappers=row[14],flags=row[15],point=list(point)))
    return dict(rows=rows,dropped=drops,terminal_reason=reason,active_leases=active,fault=fault,orphans=orphans,complete=False,clean_header=(not drops and not active and not fault and not orphans and reason==1),original_behavior_equivalence=False)
