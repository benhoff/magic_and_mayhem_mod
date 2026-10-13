"""Prospective strict MNMDGA01API/token/source binding, diagnostic only."""
import struct

def decode(data):
 if len(data)<64 or data[:8]!=b'MNMDGA01':raise ValueError('Gate binding header')
 h=struct.unpack('<16I',data[:64])
 if h[2:6]!=(1,192,0x40209ca7,8192) or any(h[6:]) or (len(data)-64)%192:raise ValueError('Gate binding header/extent')
 count=(len(data)-64)//192
 if not 1<=count<=8192:raise ValueError('Gate binding capacity')
 pending={};seen=set();completed=[];refused=[];terminal=None
 for i in range(count):
  r=struct.unpack('<48I',data[64+i*192:64+(i+1)*192])
  if r[0]!=i+1 or any(r[7:16]):raise ValueError('Gate binding ordinal/reserved')
  phase,api,owner,token,lease,reason=r[1:7];point=r[16:]
  if phase==4:
   if i!=count-1 or pending or any(r[3:]):raise ValueError('Gate binding terminal/pending')
   terminal=api;continue
  if phase not in (1,2,3) or not api or not owner:raise ValueError('Gate binding phase/identity')
  if point[:3]!=(1,128,0x40209ca7) or any(point[16:]) or point[6]!=owner or (token and point[5]!=token):raise ValueError('Gate scalar point mismatch')
  if phase!=3 and (point[3]!=(1 if phase==1 or not token else 2) or point[4]!=reason):raise ValueError('Gate scalar phase/reason mismatch')
  identity=(owner,token,reason,point[5],point[7],point[8])
  if phase==1:
   if api in seen:raise ValueError('Duplicate API gate entry')
   seen.add(api)
   if token and (reason or point[13]!=3 or not point[7] or not point[8]):raise ValueError('Gate entry lacks acquired levels/identities')
   if not token and (not reason or point[13]):raise ValueError('Gate refusal retains acquired levels/no reason')
   pending[api]=identity
  else:
   if pending.pop(api,None)!=identity:raise ValueError('Gate entry/return mismatch')
   row=dict(api_entry=api,owner=owner,token=token,attempted_token=point[5],source_lease=lease,reason=reason,phase=phase,point=list(point))
   if phase==3:raise ValueError('Gate release fault retained')
   if token:
    if reason or point[13]:raise ValueError('Gate return owns level/failure')
    completed.append(row)
   else:refused.append(row)
 if terminal is None:raise ValueError('Missing gate binding terminal')
 return dict(completed=completed,refused=refused,terminal_reason=terminal,complete_reference=False,source_alias_proved=False)
