"""Conservative same-clock physical-read/EGL interval accounting; no causality."""
import re
def uint(value,bits=64,positive=False):
 if type(value) is not int or not (1 if positive else 0)<=value<1<<bits:raise ValueError('Invalid timing identity or clock')
 return value
def nanoseconds(value):
 if not isinstance(value,str) or not re.fullmatch(r'0|[1-9][0-9]*',value) or int(value)>=1<<64:raise ValueError('Invalid exact nanosecond string')
 return int(value)
def capture_span(value):
 if not isinstance(value,dict) or set(value)!= {'begin_ns','end_ns'}:raise ValueError('Invalid physical capture span')
 begin,end=nanoseconds(value['begin_ns']),nanoseconds(value['end_ns'])
 if begin>end:raise ValueError('Physical read time reversed')
 return begin,end
def analyze_timing(peer,delivery):
 if peer.get('success') is not True or not peer.get('frames') or delivery.get('success') is not True or delivery.get('pending'):raise ValueError('Successful complete retained observations required')
 swaps=[call for call in delivery['calls'] if call['method']==11]
 if not swaps:raise ValueError('No reached EGL swaps')
 seen=set()
 for call in swaps:
  identity=(uint(call['pid'],32,True),uint(call['entry'],32,True))
  if identity in seen or uint(call['enter_ns'])>uint(call['return_ns']):raise ValueError('Duplicate or reversed EGL interval')
  seen.add(identity)
 swaps=sorted(swaps,key=lambda call:call['enter_ns'])
 first_swap=swaps[0];frames=[];previous=0;entries=set()
 for ordinal,f in enumerate(peer['frames'],1):
  if type(f['ordinal']) is not int or f['ordinal']!=ordinal or f.get('clock_domain')!='linux_CLOCK_MONOTONIC':raise ValueError('Physical identity/clock changed')
  entry=uint(f['entry'],32,True)
  if entry in entries:raise ValueError('Duplicate physical entry')
  entries.add(entry)
  if not 0<nanoseconds(f['x11_target'])<2**32 or any(type(f[k]) is not int or not -(2**31)<=f[k]<2**31 for k in ('root_x','root_y')):raise ValueError('Physical target/root coordinates changed')
  a,b=capture_span(f['first_capture']),capture_span(f['second_capture'])
  if a[0]<previous or a[1]>b[0]:raise ValueError('Physical sample pair/order impossible')
  samples=f.get('settling_samples',[]);prior=previous
  for i,s in enumerate(samples,1):
   if type(s['ordinal']) is not int or type(s['sample']) is not int or s['ordinal']!=ordinal or s['sample']!=i:raise ValueError('Settling sample identity changed')
   span=capture_span(s['capture'])
   if span[0]<prior or span[1]>a[0]:raise ValueError('Settling time/order impossible')
   prior=span[1]
  previous=b[1]
  window=[c for c in swaps if c['enter_ns']<=b[1] and c['return_ns']>=a[0]]
  before=[c for c in swaps if c['return_ns']<a[0]]
  frames.append(dict(ordinal=ordinal,entry=f['entry'],x11_target=f['x11_target'],root_x=f['root_x'],root_y=f['root_y'],first_capture=dict(f['first_capture']),second_capture=dict(f['second_capture']),settling_samples=len(samples),egl_returns_before_first_read=len(before),egl_intervals_overlapping_pair=len(window),before_first_egl_entry=b[1]<first_swap['enter_ns'],after_first_egl_return=a[0]>first_swap['return_ns'],last_completed_egl_before_read=dict(pid=max(before,key=lambda c:c['return_ns'])['pid'],entry=max(before,key=lambda c:c['return_ns'])['entry']) if before else None,coherent_presented_frame=False))
 return dict(schema=1,success=True,clock_domain='linux_CLOCK_MONOTONIC',physical_frames=len(frames),egl_swaps=len(swaps),frames=frames,first_egl=dict(pid=first_swap['pid'],entry=first_swap['entry'],enter_ns=first_swap['enter_ns'],return_ns=first_swap['return_ns']),physical_pairs_before_first_egl=sum(f['before_first_egl_entry'] for f in frames),pairs_overlapping_egl=sum(bool(f['egl_intervals_overlapping_pair']) for f in frames),source_submission_identity_bound=False,scanout_established=False,coherent_presented_frames=False,complete_journey_coverage=False)
