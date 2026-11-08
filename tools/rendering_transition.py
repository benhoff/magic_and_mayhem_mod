"""Explicit terminal-prefix and fresh-epoch admission, without synthetic ENDs."""
from collections import defaultdict
import hashlib,struct
from pathlib import Path
import numpy as np
from rendering_pilot import require,number,counter,sha,capture,History,viewport,commands,compare

TERMINAL_KEYS={'terminal','owners_retired','timers_stopped','frame_cleared','input_detached','input_suspended','restart_available','retry_disabled','fallback_disabled'}

def capture_path(channel,directory):
 require(Path(channel).is_absolute() and Path(directory).is_absolute(),'Absolute capture namespace required')
 return str(Path(directory)/('worlds-'+hashlib.sha256(channel.encode('utf-8')).hexdigest()+'.bin'))

def fault_receipt(value,expected):
 require(value.get('success') is True and type(value.get('schema')) is int and value.get('schema')==1 and type(value.get('producer')) is int and value['producer']==expected
  and value.get('reason')==5 and value.get('writer_claimed') is False and value.get('cursor_or_payload_written') is False,'Invalid declared terminal injection')
 before=bytes.fromhex(value['before_header_hex']);after=bytes.fromhex(value['after_header_hex']);require(len(before)==len(after)==64,'Fault header extent')
 a,b=struct.unpack('<16I',before),struct.unpack('<16I',after)
 require(before[:8]==after[:8]==b'MNMRDC03' and a[3]==b[3]==1048640 and not any(a[10:16]) and a[2]==b[2]==3 and a[4]==b[4]==expected and a[6]==1 and not a[7] and not a[8] and a[5]==a[9]
  and b[6]==3 and b[7]==5 and all(b[i]==a[i] for i in (0,1,2,3,4,10,11,12,13,14,15)),'Wrong producer failure boundary')
 # A peer may cancel or advance its own cursor concurrently. The native helper
 # itself only calls mnm_ring_fail; its real controls verify payload/cursor safety.
 require(counter(value['begin_host_ns'])<=counter(value['end_host_ns']),'Fault clock reversed')
 return dict(producer=expected,before=list(a),after=list(b),begin_host_ns=value['begin_host_ns'],end_host_ns=value['end_host_ns'])

def viewport_rows(raw,session):
 rows=capture(raw);require(rows[0][0]['role']=='viewport' and rows[0][0]['session']==session,'Wrong viewport identity')
 paints={};swaps=[];latest=0
 for row,body in rows[1:-1]:
  if row['kind']=='swap':
   index=counter(row['paint']);require(index==latest,'Unbound actual swap')
   if index:swaps.append(index)
   continue
  require(row['kind']=='paint','Unknown viewport row');index=counter(row['paint']);require(index==latest+1,'Viewport paint gap');latest=index
  width,height=number(row['width'],1,4096),number(row['height'],1,4096)
  require((row['pixels_captured'] is True and len(body)==width*height*4) or(row['native_gpu_frame'] is False and row['pixels_captured'] is False and not body),'Incomplete actual viewport plane')
  if counter(row['lease']):
   require(row['native_gpu_frame'] is True and row['presentation_owner']=='gui' and row['immutable_lease'] is False and row['gpu_read_complete'] is True and row['transport']=='opengl','Uncovered epoch presentation owner')
   number(row['commands'],1,2**32-1);number(row['mode'],0,1);counter(row['sealed_world']);number(row['scaling'],0,2)
  else:require(counter(row['world'])==0 and row['native_gpu_frame'] is False,'Unbound World frame')
  paints[index]=(row,body)
 require(swaps,'No actual swaps');return dict(rows=rows,paints=paints,swaps=swaps)

def epoch_admission(command_raw,binding,global_view,session,root,identity,*,failed,capture_file):
 rows=capture(command_raw);opening,closing=rows[0][0],rows[-1][0]
 require(opening['role']=='commands' and opening['session']==session and opening['version']==4
  and opening['policy']=='command-history-lossless-v1','Wrong command epoch policy')
 require(type(binding['schema']) is int and binding['schema']==1 and binding['kind']=='command-producer-binding' and type(binding['producer']) is int
  and binding['producer']==identity and binding['transport_version']==3
  and binding['channel']==str(Path(root)/'render-commands.bin') and binding['capture']==capture_file and Path(binding['capture']).name.startswith('commands-'),'Wrong actual producer association')
 low,high=counter(opening['host_us']),counter(closing['host_us']);require(low<=counter(binding['host_us'])<=high,'Binding outside actual epoch')
 command_rows=rows[1:-1];require(command_rows and all(m['kind'] in ('fragment','cpu-read','ended') for m,_ in command_rows),'Unknown command epoch record')
 ends=[m for m,_ in command_rows if m['kind']=='ended']
 require((not ends) if failed else(len(ends)==1 and command_rows[-1][0]['kind']=='ended'),'Terminal/normal stream class mismatch')
 paints={};swaps=[];exports={};last_commands=0
 for index,(row,body) in global_view['paints'].items():
  at=counter(row['host_us'])
  if not low<=at<=high or not counter(row['lease']):continue
  require(row['commands']>=last_commands,'Epoch command order reversed');last_commands=row['commands'];row=dict(row,reference_binding=index)
  paints[index]=(row,body);exports[index]=dict(row,lease=str(index),source_lease=row['lease'],width=row['frame_width'],height=row['frame_height'])
 for index in global_view['swaps']:
  if index in paints:swaps.append(index)
 require(swaps and any(counter(paints[i][0]['world']) for i in swaps),'No actual epoch World swap')
 return dict(exports=exports,paints=paints,swaps=swaps,commands=b''.join(b for m,b in command_rows if m['kind']=='fragment'),command_rows=rows,
  opening=opening,closing=closing,binding=binding,failed=failed,observations=dict(paints=len(paints),swaps=len(swaps),world_swaps=sum(bool(counter(paints[i][0]['world'])) for i in swaps)))

def terminal_commands(raw,limit):
 """Only the exact prefix needed by every actual visible swap gets credit."""
 require(len(raw)>=16 and raw[:8]==b'MNMCMD05' and struct.unpack_from('<II',raw,8)==(5,16),'Wrong terminal command stream')
 number(limit,1,2**32-1);at=16;sequence=0
 while sequence<limit:
  require(len(raw)-at>=12,'Truncated required command header');op,index,size=struct.unpack_from('<III',raw,at);at+=12
  require(index==sequence+1 and size<=32*1024**2 and size<=len(raw)-at and op!=8,'Invalid required command prefix')
  body=raw[at:at+size];at+=size;sequence=index;yield op,index,body
 return at

def terminal_domain(observed):
 surfaces={};worlds=[]
 for op,seq,raw in terminal_commands(observed['commands'],max(row['commands'] for row,_ in observed['paints'].values())):
  if op==1:
   sid,w,h,bits,r,g,b=struct.unpack_from('<7I',raw);surfaces[sid]=(w,h,bits,r,g,b)
  elif op==7:surfaces.pop(struct.unpack_from('<I',raw)[0])
  elif op==19:
   sid,lo,hi=struct.unpack_from('<3I',raw);w,h,bits,r,g,b=surfaces[sid]
   require((bits,r,g,b)==(16,0xf800,0x7e0,31) and (w,h) in ((640,480),(800,600)),'Terminal World source domain differs')
   worlds.append(dict(generation=lo+(hi<<32),width=w,height=h))
 require(worlds,'No terminal-prefix World domain')
 return dict(verified=True,format='RGB565',extents=[list(v) for v in sorted({(v['width'],v['height']) for v in worlds})],worlds=worlds)

def terminal_compare(observed,worlds,terminal):
 require(type(terminal['schema']) is int and terminal['schema']==1 and terminal['error'] and set(terminal['state'])==TERMINAL_KEYS and all(v is True for v in terminal['state'].values()),'Terminal owners/input/frame/timers not retired')
 require(observed['failed'],'Terminal prefix cannot replace normal END admission')
 limit=max(row['commands'] for row,_ in observed['paints'].values());history=History(worlds)
 exports=iter(observed['exports'].values());pending=next(exports,None);shown=defaultdict(list)
 for index in observed['swaps']:
  row,body=observed['paints'][index];shown[row['reference_binding']].append((index,row,body))
 cpu={}
 for m,body in observed['command_rows']:
  if m['kind']=='cpu-read':
   sequence=number(m['sequence'],1,2**32-1);require(sequence not in cpu,'Duplicate actual CPU reply');cpu[sequence]=body
 results=[];reads=[];iterator=terminal_commands(observed['commands'],limit);at=None
 while True:
  try:op,seq,body=next(iterator)
  except StopIteration as e:at=e.value;break
  history.apply(op,seq,body)
  if op==21:
   require(seq in cpu,'Missing actual pre-fault CPU reply');w=history.words(body,10)
   expected=b'MNMCREAD'+struct.pack('<14I',1,64,w[3],w[1],w[2],w[0],w[4],w[5],w[4]*w[5]*2,*w[6:10],1)+history.reads.pop(seq)
   require(cpu.pop(seq)==expected,'Pre-fault CPU reply mismatch');reads.append(seq)
  while pending is not None and pending['commands']==seq:
   lease=counter(pending['lease'])
   if lease in shown:
    frame=history.render(pending)
    for index,row,actual in shown[lease]:
     actual=actual() if callable(actual) else actual;expected=viewport(frame,row);a=np.frombuffer(actual,dtype='u1').reshape(row['height'],row['width'],4);b=np.frombuffer(expected,dtype='u1').reshape(a.shape);difference=int(np.count_nonzero(np.any(a!=b,axis=2)))
     results.append(dict(paint=index,world=counter(row['world']),pixels=row['width']*row['height'],mismatching_pixels=difference,actual_sha256=sha(actual),expected_sha256=sha(expected),commands=seq))
   pending=next(exports,None)
 require(pending is None and len(results)==len(observed['swaps']),'Uncompared terminal-prefix paint')
 require(results and any(v['world'] for v in results),'No compared pre-fault World')
 # Reference ownership is intentionally retained here; abort, not a fabricated
 # END, retires the failed native epoch. Its actual typed terminal proof is above.
 tail=observed['commands'][at:]
 return dict(success=all(v['mismatching_pixels']==0 for v in results),comparisons=results,pixels_compared=sum(v['pixels'] for v in results),mismatching_pixels=sum(v['mismatching_pixels'] for v in results),world_presentations=sum(bool(v['world']) for v in results),
  required_prefix_commands=limit,required_prefix_bytes=at,uncredited_terminal_tail_bytes=len(tail),uncredited_terminal_tail_sha256=sha(tail),reference_live_surfaces_before_declared_abort=len(history.handles),reference_live_palettes_before_declared_abort=len(history.palette),synthetic_end_added=False,normal_completion_claimed=False,pre_fault_cpu_replies=reads)

def assert_epoch_partition(view,epochs):
 require(len(epochs)==2 and epochs[0]['failed'] and not epochs[1]['failed'],'Expected one failed and one fresh epoch')
 require(counter(epochs[0]['closing']['host_us'])<=counter(epochs[1]['opening']['host_us']),'Overlapping producer epochs')
 require(epochs[0]['binding']['producer']!=epochs[1]['binding']['producer'] and epochs[0]['binding']['channel']!=epochs[1]['binding']['channel'],'Reused producer identity')
 native={i for i in view['swaps'] if counter(view['paints'][i][0]['lease'])}
 assigned=[i for e in epochs for i in e['swaps']]
 require(len(assigned)==len(set(assigned)) and set(assigned)==native,'Unassigned or multiply assigned native swap')
 return dict(actual_native_swaps=len(native),distinct_producers=True,all_swaps_uniquely_bound=True)
