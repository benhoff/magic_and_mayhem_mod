"""Reference-only completion and optional source-GPU oracles; never native inputs."""
from pathlib import Path
import struct,hashlib

def decode(raw):
 if len(raw)!=128 or raw[:8]!=b'MNMESGC1':raise ValueError('EGL completion extent/magic')
 r=struct.unpack('<32I',raw)
 if r[2:4]!=(1,128) or not all(r[4:9]) or max(r[7:9])>2048 or r[9] not in (0x911a,0x911c) or r[10:14]!=(1,1,1,0) or r[31] or r[26]>1 or (r[16]|r[17]<<32)<(r[14]|r[15]<<32):raise ValueError('EGL completion identity/result')
 return dict(call=r[4],pid=r[5],thread=r[6],width=r[7],height=r[8],wait_result=r[9],begin_ns=r[14]|r[15]<<32,end_ns=r[16]|r[17]<<32,viewport=list(r[18:22]),scissor=list(r[22:26]),scissor_enabled=r[26],draw_fbo=r[27],read_fbo=r[28],draw_buffer=r[29],read_buffer=r[30],context_unchanged=True,fence_created_deleted=True)

def decode_blit(raw):
 if len(raw)!=96 or raw[:8]!=b'MNMEGBT1':raise ValueError('EGL blit extent/magic')
 r=struct.unpack('<24I',raw)
 if r[2:4]!=(1,96) or not all(r[4:7]) or r[7]>16384 or not r[7] and any(r[8:]):raise ValueError('EGL blit identity/count')
 return dict(call=r[4],pid=r[5],thread=r[6],count=r[7],source_box=list(struct.unpack_from('<4i',raw,32)),destination_box=list(struct.unpack_from('<4i',raw,48)),mask=r[16],filter=r[17],read_fbo=r[18],draw_fbo=r[19],read_texture=r[20],read_width=r[21],read_height=r[22],read_buffer=r[23])

def decode_surface(raw):
 if len(raw)!=64 or raw[:8]!=b'MNMESBF1':raise ValueError('EGL pre-swap extent/magic')
 r=struct.unpack('<16I',raw)
 if r[2:4]!=(1,64) or not all(r[4:9]) or max(r[7:9])>2048 or any(r[9:]):raise ValueError('EGL pre-swap identity')
 return dict(call=r[4],pid=r[5],thread=r[6],width=r[7],height=r[8])

def decode_window(raw):
 if len(raw)!=96 or raw[:8]!=b'MNMEW001':raise ValueError('EGL window extent/magic')
 r=struct.unpack('<24I',raw)
 if r[2:4]!=(1,96) or not all(r[4:7]) or r[7] not in (1,2) or r[8]>1 or any(r[21:]) or r[15]>2 or (r[8]==0 and any(r[9:])) or (r[8]==1 and (not all(r[9:11]) or not all(r[13:15]))):raise ValueError('EGL window identity')
 x,y=struct.unpack_from('<2i',raw,44)
 return dict(call=r[4],pid=r[5],thread=r[6],phase=r[7],known=bool(r[8]),window=r[9],generation=r[10],root_x=x,root_y=y,width=r[13],height=r[14],map_state=r[15])

def decode_viewport_policy(raw):
 if len(raw)!=64 or raw[:8]!=b'MNMEVP01':raise ValueError('EGL viewport policy extent')
 r=struct.unpack('<16I',raw)
 if r[2:4]!=(1,64) or not all(r[4:8]) or r[7]>16384 or not all(r[12:14]) or max(r[12:14])>2048 or r[14:]!=(1,0):raise ValueError('EGL viewport policy identity')
 return dict(call=r[4],pid=r[5],thread=r[6],count=r[7],previous=list(r[8:12]),temporary=[0,0,r[12],r[13]],restored=True)

def analyze(directory,physical):
 result=[];observed={}
 for path in Path(directory).glob('egl-swap-completion-*.bin'):
  raw=path.read_bytes();row=decode(raw);key=(row['pid'],row['call'])
  if key in observed or path.name!=f"egl-swap-completion-{row['call']:06d}.bin":raise ValueError('Duplicate/misnamed EGL completion')
  row.update(path=path.name,sha256=hashlib.sha256(raw).hexdigest());observed[key]=row
 for frame in physical['frames']:
  row=observed[frame['pid'],frame['entry']]
  if any(row[k]!=frame[k] for k in ('thread','width','height')) or row['end_ns']>int(frame['first_capture']['begin_ns']):raise ValueError('EGL completion frame mismatch')
  policy=Path(directory)/f"egl-viewport-refresh-{row['call']:06d}.bin"
  if policy.exists():
   raw=policy.read_bytes();v=decode_viewport_policy(raw)
   if any(v[k]!=row[k] for k in ('call','pid','thread')) or v['previous']!=row['viewport']:raise ValueError('Viewport policy restoration differs')
   row['viewport_policy']=dict(**v,sha256=hashlib.sha256(raw).hexdigest(),path=policy.name)
  for phase,name in ((1,'before'),(2,'after')):
   window=Path(directory)/f"egl-window-{name}-{row['call']:06d}.bin"
   if window.exists():
    raw=window.read_bytes();w=decode_window(raw)
    if w['phase']!=phase or any(w[k]!=row[k] for k in ('call','pid','thread')):raise ValueError('EGL native window call differs')
    row['window_'+name]=dict(**w,sha256=hashlib.sha256(raw).hexdigest(),path=window.name)
  before=Path(directory)/f"egl-surface-before-swap-{row['call']:06d}.bin"
  if before.exists():
   raw=before.read_bytes();b=decode_surface(raw)
   if any(b[k]!=row[k] for k in ('call','pid','thread')):raise ValueError('EGL pre-swap call differs')
   row['pre_swap']=dict(**b,sha256=hashlib.sha256(raw).hexdigest(),path=before.name)
  blit=Path(directory)/f"egl-blit-before-swap-{row['call']:06d}.bin"
  if blit.exists():
   raw=blit.read_bytes();b=decode_blit(raw)
   if any(b[k]!=row[k] for k in ('call','pid','thread')):raise ValueError('EGL blit call differs')
   row['last_blit']=dict(**b,sha256=hashlib.sha256(raw).hexdigest(),path=blit.name)
   source=Path(directory)/f"egl-gl-source-{row['call']:06d}.565"
   if source.exists():
    pixels=source.read_bytes()
    if len(pixels)!=b['read_width']*b['read_height']*2:raise ValueError('EGL GPU source extent differs')
    row['gpu_source']=dict(path=source.name,sha256=hashlib.sha256(pixels).hexdigest(),bytes=len(pixels),original_oracle_only=True)
  result.append(dict(ordinal=frame['ordinal'],**row))
 return dict(schema=1,success=True,frames=result,other_returned_calls=len(observed)-len(result),original_pixels_used_as_native_inputs=False,hardware_scanout_established=False,scope='Post-return same-context GL fence completion plus viewport/scissor/buffer metadata; explicit diagnostic perturbation. Optional source-GPU pixels remain separately labelled comparator-only oracles. No source ownership or query/error-state neutrality assertion.')
