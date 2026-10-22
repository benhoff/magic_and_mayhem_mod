"""Portable worker channel/EGLsnapshots; interval metadata only."""
import struct
from drawing_api_session import sha


def channel(words):
    if len(words)!=16 or struct.pack('<2I',*words[:2])!=b'MNMWCX01' or words[2:4]!=(1,64) or words[4]&1 or words[5]>1 or words[11]>1 or any(words[12:]):raise ValueError('Worker context wire')
    if not words[5] and any(words[6:12]) or words[5] and (not words[6] or not words[9] or bool(words[11])!=bool(not words[7]) or not words[7] and words[8]):raise ValueError('Worker context active fields')
    return dict(sequence=words[4],active=bool(words[5]),worker=words[6],command=words[7],submission=words[8],win_thread=words[9],lifetime_prefix=words[10],flags=words[11])


def decode(data):
    if len(data)<64 or len(data)>64+131072*128 or (len(data)-64)%128:raise ValueError('EGL worker context envelope')
    h=struct.unpack_from('<16I',data)
    if data[:8]!=b'MNMEWC01' or h[2:5]!=(1,128,131072) or any(h[5:]):raise ValueError('EGL worker context header')
    calls={}
    for ordinal,r in enumerate(struct.iter_unpack('<32I',data[64:]),1):
        if r[0]!=ordinal or r[1] not in (1,2) or not all(r[2:5]) or r[7] not in (0,1,2) or any(r[24:]):raise ValueError('EGL worker context identity/reserve')
        context=channel(r[8:24]) if r[7]==0 else None
        if r[7] and any(r[8:24]):raise ValueError('Invalid unavailable context payload')
        key=(r[3],r[2]);entry=calls.setdefault(key,[])
        if len(entry)!=r[1]-1:raise ValueError('EGL worker context missing/duplicate phase')
        entry.append(dict(phase=r[1],pid=r[3],unix_thread=r[4],time_ns=r[5]|r[6]<<32,status=r[7],context=context))
    result=[]
    for (pid,call),points in calls.items():
        if len(points)!=2 or points[0]['unix_thread']!=points[1]['unix_thread'] or points[0]['time_ns']>points[1]['time_ns']:raise ValueError('EGL worker context incomplete/changedthread')
        stable=points[0]['context']==points[1]['context'] and points[0]['context'] is not None
        result.append(dict(pid=pid,call=call,points=points,stable_active_context=stable and points[0]['context']['active'],source_storage_owned=False,egl_frame_assignment_proved=False))
    return dict(schema=1,success=True,journal_sha256=sha(data),calls=result,stable_active_contexts=sum(c['stable_active_context'] for c in result),missing_or_unstable_contexts=sum(any(p['status'] for p in c['points']) for c in result),win_unix_thread_relation_validated=False,source_storage_owned=False,complete_presented_journey=False)
