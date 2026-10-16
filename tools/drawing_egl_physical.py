"""Bind post-EGL root witnesses to exact returned calls without frame ownership."""
import json
from pathlib import Path
import struct
from drawing_api_session import sha
from drawing_x11_delivery import decode_delivery
from drawing_delivery_timing import capture_span
def decode_request(raw):
 if len(raw)!=128 or raw[:8] not in (b'MNMEGL01',b'MNMEGL02'):raise ValueError('Invalid EGL physical request')
 r=struct.unpack('<32I',raw)
 if r[2] not in (1,2) or r[3]!=128 or raw[:8]!=(b'MNMEGL01' if r[2]==1 else b'MNMEGL02') or not 1<=r[4]<=1024 or not r[5] or not r[6] or not r[7] or r[8]!=11 or not 1<=r[9]<=256 or not 1<=r[10]<=256 or not 1<=r[11]<=16 or r[12]!=1 or not 1<=r[14]<=2048 or not 1<=r[15]<=2048 or (r[2]==1 and r[14:16]!=(800,600)) or any(r[18:]):raise ValueError('Invalid EGL physical identity/extent/reserved fields')
 return r
def analyze_egl_physical(directory,peer_path):
 root=Path(directory);peer=json.loads(Path(peer_path).read_text());delivery=decode_delivery((root/'x11-delivery.bin').read_bytes(),False)
 if peer.get('success') is not True or peer.get('original_egl_caller_fenced') is not True or peer.get('native_rendering') is not False or peer.get('boundary')!='observed_post_egl_return_physical' or peer.get('all_global_presentation_routes') is not False or peer.get('source_inputs_bound') is not False or peer.get('all_physical_changes_captured') is not False or peer.get('scanout_established',False) is not False:raise ValueError('Invalid EGL physical peer scope')
 calls={(c['pid'],c['entry']):c for c in delivery['calls'] if c['method']==11 and c['result']==1};frames=peer['frames']
 if not frames or len(frames)>1024:raise ValueError('Missing/over-budget EGL physical frames')
 if any(len(list(root.glob(pattern)))!=len(frames) for pattern in ('egl-physical-request-*.bin','egl-physical-ack-*.bin','egl-physical-oracle-*.rgba')):raise ValueError('Missing/extra EGL physical artifacts')
 marker=(root/'egl-physical.done').read_bytes()
 if len(marker)!=32 or marker[:8]!=b'MNMEGD01':raise ValueError('Missing EGL physical close marker')
 closed=struct.unpack('<8I',marker)
 if closed[2]!=1 or closed[4]!=len(frames) or closed[6:]!=(1,0):raise ValueError('Invalid EGL physical close accounting')
 calls={key:c for key,c in calls.items() if key[0]==closed[3] and key[1]<=closed[5]}
 seen=set();result=[];pids=set();threads=set();versions=set();previous_end=None;transient_names=set();transient_bytes=0
 for ordinal,f in enumerate(frames,1):
  raw=(root/f'egl-physical-request-{ordinal:06d}.bin').read_bytes();r=decode_request(raw);key=(r[5],r[7]);c=calls.get(key)
  identity=dict(ordinal=ordinal,pid=r[5],thread=r[6],entry=r[7],method=11,display=r[9],surface=r[10],alias=r[11],width=r[14],height=r[15])
  if r[4]!=ordinal or c is None or key in seen or c['thread']!=r[6] or c['arguments'][:3]!=list(r[9:12]) or c['return_errno']!=r[13] or any(f.get(k)!=v for k,v in identity.items()):raise ValueError('EGL physical call/peer identity mismatch')
  versions.add(r[2]);seen.add(key);pids.add(r[5]);threads.add((r[5],r[6]));clock=r[16]|r[17]<<32
  if r[2]==2:
   expected=[f['root_x'],f['root_y'],r[14],r[15]]
   for name in ('first_geometry','second_geometry'):
    g=f.get(name,{})
    if g.get('stable') is not True or g.get('before')!=expected or g.get('after')!=expected:raise ValueError('Unstable EGL witness window geometry')
  first,second=capture_span(f['first_capture']),capture_span(f['second_capture'])
  if previous_end is not None and (clock<previous_end or c['enter_ns']<previous_end):raise ValueError('Overlapping EGL physical handoffs')
  previous_end=second[1]
  if f.get('clock_domain')!='linux_CLOCK_MONOTONIC' or first[0]<clock or first[1]>second[0] or clock<c['return_ns']:raise ValueError('EGL physical clock/order mismatch')
  ack=(root/f'egl-physical-ack-{ordinal:06d}.bin').read_bytes()
  if ack!=b'MNMEACK1'+struct.pack('<2I',ordinal,r[7]):raise ValueError('EGL physical ACK changed')
  name=f'egl-physical-oracle-{ordinal:06d}.rgba';plane=(root/name).read_bytes()
  if len(plane)!=r[14]*r[15]*4 or f.get('path')!=name or f.get('sha256')!=sha(plane) or f.get('request_sha256')!=sha(raw) or f.get('stable_double_capture') is not True:raise ValueError('EGL physical plane/hash mismatch')
  previous=clock
  for index,sample in enumerate(f.get('settling_samples',[]),1):
   path=f'egl-physical-transient-{ordinal:06d}-{index:02d}.rgba';span=capture_span(sample['capture']);data=(root/path).read_bytes()
   if sample['ordinal']!=ordinal or sample['sample']!=index or sample['path']!=path or sha(data)!=sample['sha256'] or len(data)!=len(plane) or span[0]<previous or span[1]>first[0]:raise ValueError('EGL settling sample changed')
   previous=span[1];transient_names.add(path);transient_bytes+=len(data)
  result.append(dict(**identity,path=name,sha256=sha(plane),first_capture=f['first_capture'],second_capture=f['second_capture'],source_inputs_bound=False,coherent_presented_frame=False))
 if seen!=set(calls) or len(pids)!=1 or len(versions)!=1 or (versions=={1} and len(threads)!=1):raise ValueError('Incomplete/competing successful EGL swap witnesses')
 if len(list(root.glob('egl-physical-transient-*.rgba')))!=len(transient_names) or peer.get('transient_bytes')!=transient_bytes or transient_bytes>peer['transient_byte_budget']:raise ValueError('EGL transient accounting mismatch')
 return dict(schema=1,success=True,boundary='observed_post_egl_return_physical',frames=result,pixels=sum(f["width"]*f["height"] for f in result),delivery_sha256=delivery['sha256'],transient_bytes=transient_bytes,original_pixels_used_as_native_inputs=False,all_physical_changes_captured=False,all_global_presentation_routes_observed=False,source_inputs_bound=False,scanout_established=False,complete_presented_journey=False)
