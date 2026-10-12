"""Strict public map metadata; open/unknown intervals do not establish ownership."""
import struct
from drawing_api_session import sha


def decode(data):
    if len(data)<192 or (len(data)-64)%128 or len(data)>64+131073*128:raise ValueError('Resource map envelope')
    h=struct.unpack_from('<16I',data)
    if data[:8]!=b'MNMWRM01' or h[2:7]!=(1,128,0x40209ca7,131072,2048) or any(h[7:]):raise ValueError('Resource map header')
    rows=list(struct.iter_unpack('<32I',data[64:]));pending={};calls=[];intervals={};unknown=[];seen_calls=set()
    for ordinal,r in enumerate(rows[:-1],1):
        if r[0]!=ordinal or r[1] not in (1,2) or r[3] not in (1,2,3) or not r[2] or not r[4] or r[31] or r[30]>1 or r[17]>2:raise ValueError('Resource map identity/reserve')
        if r[1]==1:
            if r[2] in pending or r[2] in seen_calls:raise ValueError('Resource map duplicatecall')
            pending[r[2]]=r;seen_calls.add(r[2])
        else:
            entry=pending.pop(r[2],None)
            if entry is None or r[3:7]!=entry[3:7] or r[10:14]!=entry[10:14] or r[16:24]!=entry[16:24]:raise ValueError('Resource map unmatched/changedreturn')
            success=r[8]<0x80000000;method=r[3]
            if method==1:
                if r[8] not in (0,1) or bool(r[14])!=bool(r[8]) or any(r[16:]):raise ValueError('Resource getter result')
            elif method==2:
                if success:
                    if not r[27] or r[27] in intervals:raise ValueError('Resource map interval identity')
                    intervals[r[27]]=dict(resource=r[10],subresource=r[12],data=r[26],row_pitch=r[24],slice_pitch=r[25],map_call=r[2],closed=False)
                elif any(r[24:28]):raise ValueError('Failedmapdescriptor')
            else:
                if r[24:28]!=entry[24:28] or r[30]!=entry[30]:raise ValueError('Resource unmap interval changed')
                if r[30]:unknown.append(r[2])
                elif not r[27] or r[27] not in intervals or intervals[r[27]]['closed'] or (intervals[r[27]]['resource'],intervals[r[27]]['subresource'])!=(r[10],r[12]):raise ValueError('Resource unmap interval mismatch')
                elif success:intervals[r[27]]['closed']=True
            calls.append(dict(call=r[2],method=method,thread=r[4],api_entry=r[5],depth=r[6],result=r[8],last_error=r[9],object=r[10],generation=r[11],subresource=r[12],lifetime_before=r[13],lifetime_after=r[15],returned_resource=r[14],flags=r[16],box_present=bool(r[17]),box_status=r[17],box=list(r[18:24]),row_pitch=r[24],slice_pitch=r[25],data=r[26],map_interval=r[27],texture=r[28],texture_generation=r[29],unknown_unmap=bool(r[30])))
    t=rows[-1];open_count=sum(not x['closed'] for x in intervals.values())
    if pending or t[0]!=len(rows) or t[1]!=4 or t[2] not in (1,2) or t[3]!=len(calls) or t[10]>2048 or t[27]!=len(intervals) or t[30]!=open_count or any(t[i] for i in list(range(5,7))+list(range(8,10))+list(range(11,27))+[28,29,31]):raise ValueError('Resource map incompleteclosure/count')
    return dict(schema=1,success=True,journal_sha256=sha(data),calls=calls,map_intervals=list(intervals.values()),open_map_intervals=open_count,unknown_unmaps=unknown,source_upload_identity_proved=False,private_layout_read=False,complete_presented_journey=False)
