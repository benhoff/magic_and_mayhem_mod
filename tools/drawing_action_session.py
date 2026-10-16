"""Join reference-only original action epochs to exact fenced EGL calls.

An epoch is a coherent original callback observation, not proof of semantic
input consumption or source causality. Entry/return brackets retain ambiguity.
"""
import hashlib
import struct
from pathlib import Path
from drawing_api_session import decode


def action_epochs(journal):
    decode(journal)
    states={0:(0,)*12};events=[];completed_request=completed_epoch=0;pending=None
    for row in struct.iter_unpack('<32I',journal[64:]):
        if row[1]!=3:continue
        request,action,screen,generation,argument,phase,status=row[21:28]
        identity=(request,action,screen,generation,argument)
        if phase==1:
            if pending is not None:raise ValueError('Overlapping original menu callbacks')
            pending=identity
        elif phase==2:
            if pending!=identity:raise ValueError('Original callback identity changed')
            pending=None;completed_request=request;completed_epoch=len(events)+1
        else:raise ValueError('Invalid original action phase')
        epoch=len(events)+1
        state=(epoch,*identity,phase,status,row[29],row[30],completed_request,completed_epoch)
        states[epoch]=state;events.append(dict(epoch=epoch,request=request,action=action,screen=screen,generation=generation,argument=argument,phase=phase,status=status,entry=row[29],after_api_frame=row[30]))
    if pending is not None:raise ValueError('Unclosed original callback')
    return states,events


def frame_epoch(raw,ordinal,entry,states):
    if len(raw)!=128 or raw[:8]!=b'MNMDAF01':raise ValueError('Invalid action/frame envelope')
    r=struct.unpack('<32I',raw)
    if r[2:6]!=(1,128,ordinal,entry) or any(r[30:]):raise ValueError('Action/frame identity mismatch')
    before,after=r[6:18],r[18:30]
    if states.get(before[0])!=before or states.get(after[0])!=after or before[0]>after[0]:raise ValueError('Unbound or reversed original action epoch')
    return dict(before_epoch=before[0],after_epoch=after[0],before_completed_request=before[10],after_completed_request=after[10],callback_in_flight=before[6]==1 or after[6]==1,action_changed_during_swap=before!=after,stable_completed_epoch=before==after and before[6]!=1)


def worker_handoff(events,completed,before_epoch,used):
    candidates=[e for e in events if e['request']==completed and e['phase']==2 and e['action']==19 and e['status']==1 and e['epoch']<=before_epoch]
    if len(candidates)!=1 or completed in used:raise ValueError('EGL worker change lacks a fresh completed display action')
    return candidates[0]['epoch']


def bind(directory,observation):
    root=Path(directory);journal=(root/'session.bin').read_bytes();states,events=action_epochs(journal);frames=observation['frames']
    paths=list(root.glob('egl-physical-actions-*.bin'))
    if len(paths)!=len(frames):raise ValueError('Missing or extra action/frame witness')
    rows=[];previous=0;previous_thread=None;handoffs=[];used_handoffs=set()
    for ordinal,frame in enumerate(frames,1):
        if frame['ordinal']!=ordinal:raise ValueError('Reordered action-bound frame')
        p=root/f'egl-physical-actions-{ordinal:06d}.bin';raw=p.read_bytes();row=frame_epoch(raw,ordinal,frame['entry'],states)
        if row['before_epoch']<previous:raise ValueError('Action epochs regressed across swaps')
        if previous_thread is not None and frame['thread']!=previous_thread:
            completed=row['before_completed_request'];epoch=worker_handoff(events,completed,row['before_epoch'],used_handoffs)
            used_handoffs.add(completed);handoffs.append(dict(ordinal=ordinal,previous_thread=previous_thread,thread=frame['thread'],completed_display_request=completed,epoch=epoch))
        previous_thread=frame['thread'];previous=row['after_epoch'];rows.append(dict(ordinal=ordinal,entry=frame['entry'],path=p.name,sha256=hashlib.sha256(raw).hexdigest(),**row))
    return dict(schema=1,success=True,journal_sha256=hashlib.sha256(journal).hexdigest(),actions=events,worker_handoffs=handoffs,frames=rows,frames_bound=len(rows),stable_completed_frames=sum(r['stable_completed_epoch'] for r in rows),in_flight_frames=sum(r['callback_in_flight'] or r['action_changed_during_swap'] for r in rows),all_global_presentation_routes_observed=False,semantic_outcomes_assumed=False,scope='Atomic original menu callback epoch snapshots immediately before/after each observed EGL swap. In-flight/changed epochs are retained, not silently assigned a completed action; exact API journal identities bind the snapshots. No source causality, global-route completeness or generic presented_frame claim.')
