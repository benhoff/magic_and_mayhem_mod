"""Strict same-Windows-QPC call boundaries; temporal association is not ownership."""
import struct
from drawing_api_session import decode as decode_session, sha


def decode(raw, journal):
    decode_session(journal)
    if len(raw)<128 or (len(raw)-64)%64 or len(raw)>64+786433*64:
        raise ValueError('Update timing envelope')
    h=struct.unpack_from('<16I',raw)
    if raw[:8]!=b'MNMUTIM1' or h[2:6]!=(1,64,0x40209ca7,786433) or not (h[6]|h[7]) or any(h[8:]):
        raise ValueError('Update timing header')
    rows=list(struct.iter_unpack('<16I',raw[64:]));main=list(struct.iter_unpack('<32I',journal[64:]));terminal=main[-1]
    originals={r[29]:r for r in main[:-1]};grouped={}
    if len(originals)!=len(main)-1:raise ValueError('Ambiguous original entry')
    for index,r in enumerate(rows[:-1],1):
        original=originals.get(r[2])
        if r[0]!=index or r[1] not in (1,2,3) or original is None or r[3:6]!=(original[6],original[1],original[2]) or any(r[13:]):
            raise ValueError('Update timing identity')
        if r[1]==1 and any(r[8:13]) or r[1]>=2 and r[8:11]!=(original[8],original[9],original[21]):
            raise ValueError('Update timing original result/input')
        if r[1]==2 and any(r[11:13]) or r[1]==3 and r[11:13]!=(original[0],original[12]):
            raise ValueError('Update timing completed record/frame')
        grouped.setdefault(r[2],[]).append(r)
    calls=[]
    for entry,original in originals.items():
        points=grouped.get(entry,[])
        if [p[1] for p in points]!=[1,2,3]:raise ValueError('Update timing missing/reordered phases')
        times=[p[6]|p[7]<<32 for p in points]
        if times!=sorted(times):raise ValueError('Update timing reversed interval')
        calls.append(dict(entry=entry,type=original[1],method=original[2],thread=original[6],
                          enter_qpc=times[0],return_qpc=times[1],complete_qpc=times[2],
                          record=original[0],frame=original[12],result=original[8],last_error=original[9],
                          arguments=list(original[21:29])))
    end=rows[-1];end_time=end[6]|end[7]<<32
    if len(rows)!=3*len(originals)+1 or end[0]!=len(rows) or end[1]!=4 or end[2:6]!=(terminal[2],terminal[0],0,terminal[22]) or end[8:10]!=(0,0) or any(end[10:]) or any(c['complete_qpc']>end_time for c in calls):
        raise ValueError('Update timing terminal')
    return dict(schema=1,success=True,journal_sha256=sha(raw),session_sha256=sha(journal),
                frequency=h[6]|h[7]<<32,clock_domain='Windows_QueryPerformanceCounter',calls=calls,
                all_admitted_calls_timestamped=True,source_ownership_proved=False,complete_presented_journey=False)
