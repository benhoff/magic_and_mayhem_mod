"""Strict bounded Unix delivery call metadata; no source/presentation ownership."""
import struct
from collections import Counter
from drawing_api_session import sha
METHODS={1:'XPutImage',2:'XShmPutImage',3:'XCopyArea',4:'XCopyPlane',5:'XFillRectangle',6:'XFillRectangles',7:'XClearArea',8:'XFlush',9:'XSync'}
def decode_delivery(raw,complete=True):
 if len(raw)<64 or (len(raw)-64)%96 or len(raw)>64+65536*96:raise ValueError('Incomplete X11 envelope')
 h=struct.unpack_from('<16I',raw)
 if raw[:8]!=b'MNMX1101' or h[2:5]!=(1,96,65536) or any(h[5:]):raise ValueError('Unsupported X11 header')
 rows=list(struct.iter_unpack('<24I',raw[64:]));calls={};entries=Counter();results=[]
 for r in rows:
  phase,identity,method,pid,tid=r[:5]
  if phase not in (1,2) or not identity or method not in METHODS or not pid or not tid or any(r[17:]):raise ValueError('Invalid X11 record identity/reserved fields')
  key=(pid,identity);time=r[5]|r[6]<<32
  if phase==1:
   if key in calls or r[7]:raise ValueError('Duplicate X11 entry or premature result')
   calls[key]=r;entries[pid]+=1
  else:
   before=calls.get(key)
   if before is None or len(before)!=24 or before[2:5]!=r[2:5] or before[9:17]!=r[9:17] or time<(before[5]|before[6]<<32):raise ValueError('Unknown/duplicate/reordered X11 return')
   results.append(dict(pid=pid,thread=tid,entry=identity,method=method,name=METHODS[method],enter_ns=before[5]|before[6]<<32,return_ns=time,arguments=list(r[9:17]),result=r[7],entry_errno=before[8],return_errno=r[8]));calls[key]=None
 pending=[dict(pid=pid,entry=identity) for (pid,identity),r in calls.items() if r is not None]
 for pid,count in entries.items():
  if {identity for p,identity in calls if p==pid}!=set(range(1,count+1)):raise ValueError('X11 entry identity gap')
 if not results or complete and pending:raise ValueError('Incomplete X11 calls')
 return dict(schema=1,success=True,sha256=sha(raw),records=len(rows),calls=results,pending=pending,methods=dict(Counter(r['name'] for r in results)),processes=len(entries),observed_selected_calls_complete=not pending,all_display_routes_observed=False,producer_identity_bound=False,coherent_presented_frames=False,original_pixels_used_as_native_inputs=False)
