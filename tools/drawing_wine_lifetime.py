"""Admit selected imported Wine lifetimes; retain unknown/internal-route gaps."""
import struct
from drawing_api_session import sha


def decode(data):
    if len(data)<160 or len(data)>64+131072*96 or (len(data)-64)%96:
        raise ValueError('Wine lifetime envelope')
    h=struct.unpack_from('<16I',data)
    if data[:8]!=b'MNMWLF01' or h[2:7]!=(1,96,0x40209ca7,131072,2048) or any(h[7:]):
        raise ValueError('Wine lifetime header')
    rows=list(struct.iter_unpack('<24I',data[64:]))
    if rows[-1][1]!=4 or any(r[1]==4 for r in rows[:-1]):raise ValueError('Wine lifetime terminal')
    pending={};active={};generations=set();events=[];entered=0;unknown=0
    for ordinal,r in enumerate(rows[:-1],1):
        if r[0]!=ordinal or r[1] not in (1,2) or r[3] not in (1,2,3,4) or not r[4] or r[6]>32 or bool(r[5])!=bool(r[6]) or any(r[13:]):
            raise ValueError('Wine lifetime identity/reserve')
        kind=1 if r[3]<=2 else 3
        if r[1]==1:
            entered+=1
            if r[2]!=entered or any(r[8:10]) or r[12] or (r[3] in (1,3) and any(r[10:12])):
                raise ValueError('Wine lifetime entry')
            if r[3] in (2,4):
                if not r[10] or active.get((kind,r[10]),0)!=r[11]:raise ValueError('Wine lifetime release generation differs')
                unknown+=not r[11]
            pending[r[2]]=r
        else:
            entry=pending.pop(r[2],None)
            if entry is None or entry[3:7]!=r[3:7]:raise ValueError('Wine lifetime unpaired return')
            if r[3] in (1,3):
                if r[8]&0x80000000:
                    if any(r[10:13]):raise ValueError('Failed create supplied generation')
                elif r[12]==1:
                    if any(r[10:12]):raise ValueError('Invalid create output generation')
                else:
                    if r[12] or not r[10] or r[11]!=len(generations)+1 or (kind,r[10]) in active:raise ValueError('Wine lifetime duplicate/reused creation')
                    generations.add(r[11]);active[kind,r[10]]=r[11]
            else:
                if r[10:12]!=entry[10:12] or r[12]!=(2 if r[8]==0 and r[11] else 0):raise ValueError('Wine lifetime release result')
                if r[12]==2:active.pop((kind,r[10]))
            events.append(dict(call=r[2],method=r[3],thread=r[4],api_entry=r[5],object=r[10],generation=r[11],result=r[8],last_error=r[9],flags=r[12]))
    terminal=rows[-1]
    if pending or terminal[0]!=len(rows) or terminal[2] not in (1,2) or terminal[3]!=entered or terminal[11]!=len(generations) or any(terminal[5:7]) or any(terminal[8:11]) or any(terminal[12:]):raise ValueError('Wine lifetime incomplete/count')
    return dict(schema=1,success=True,journal_sha256=sha(data),events=events,observed_generations=len(generations),
                remaining_observed_objects=len(active),unknown_release_calls=unknown,
                all_internal_lifetimes_observed=False,source_storage_owned=False,
                egl_temporal_assignment_performed=False,complete_presented_journey=False)
