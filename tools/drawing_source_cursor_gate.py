"""Prospective strict MNMDGA01API/token/source binding, diagnostic only."""
import struct

def decode(data):
 if len(data)<64 or data[:8]!=b'MNMDGA01':raise ValueError('Gate binding header')
 h=struct.unpack('<16I',data[:64])
 if h[2:6]!=(1,192,0x40209ca7,8192) or any(h[6:]) or (len(data)-64)%192:raise ValueError('Gate binding header/extent')
 count=(len(data)-64)//192
 if not 1<=count<=8192:raise ValueError('Gate binding capacity')
 pending={};completed=[];refused=[];terminal=None
 for i in range(count):
  r=struct.unpack('<48I',data[64+i*192:64+(i+1)*192])
  if r[0]!=i+1 or any(r[7:16]):raise ValueError('Gate binding ordinal/reserved')
  phase,api,owner,token,lease,reason=r[1:7];point=r[16:]
  if phase==4:
   if i!=count-1 or pending or any(r[3:]):raise ValueError('Gate binding terminal/pending')
   terminal=api;continue
  if phase not in (1,2,3) or not api or not owner:raise ValueError('Gate binding phase/identity')
  if point[:3]!=(1,128,0x40209ca7) or any(point[16:]) or point[6]!=owner or point[5]!=token:raise ValueError('Gate scalar point mismatch')
  if phase==1:
   if api in pending:raise ValueError('Duplicate API gate entry')
   pending[api]=(owner,token,reason)
  else:
   if pending.pop(api,None)!=(owner,token,reason):raise ValueError('Gate entry/return mismatch')
   row=dict(api_entry=api,owner=owner,token=token,source_lease=lease,reason=reason,phase=phase,point=list(point))
   if phase==3:raise ValueError('Gate release fault retained')
   if token:
    if reason or point[13]:raise ValueError('Gate return owns level/failure')
    completed.append(row)
   else:refused.append(row)
 if terminal is None:raise ValueError('Missing gate binding terminal')
 return dict(completed=completed,refused=refused,terminal_reason=terminal,complete_reference=False,source_alias_proved=False)
