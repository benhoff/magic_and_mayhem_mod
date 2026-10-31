"""Strict admission of the actual pilot collector, separate from final acceptance.

Captured UPLOAD bytes are owned operation inputs. Their CPU producer semantics
remain separately scoped; no destination snapshots are used to seed the oracle.
"""
import hashlib,json,struct,zlib
from collections import Counter
from pathlib import Path
import numpy as np

def require(value,message):
 if not value:raise ValueError(message)
def number(value,low=0,high=2**64-1):
 require(type(value) is int and low<=value<=high,'Invalid typed bounded integer');return value
def counter(value):
 require(type(value) is str and value.isascii() and value.isdecimal() and str(int(value))==value,'Invalid decimal identity');return number(int(value))
def sha(data):return hashlib.sha256(data).hexdigest()
def inflate(raw,limit=32*1024**2):
 require(len(raw)>=5,'Truncated qCompress plane');size=struct.unpack_from('>I',raw)[0];require(size<=limit,'Inflation budget')
 d=zlib.decompressobj();body=d.decompress(raw[4:],size+1)
 require(len(body)==size and d.eof and not d.unused_data and not d.unconsumed_tail,'Incomplete compressed plane');return body

def capture(raw):
 require(raw[:8]==b'MNMACE01' and len(raw)<=256*1024**2+8,'Capture envelope')
 rows=[];at=8;total=0;last_time=0
 while at<len(raw):
  require(len(rows)<16384 and len(raw)-at>=8,'Capture row budget/header')
  m,n=struct.unpack_from('<II',raw,at);at+=8;require(1<=m<=65536 and 0<=n<=33*1024**2 and m+n<=len(raw)-at,'Capture row extent')
  row=json.loads(raw[at:at+m]);at+=m;data=inflate(raw[at:at+n]) if n else b'';at+=n
  require(isinstance(row,dict) and number(row['index'])==len(rows) and type(row['kind']) is str,'Capture row identity')
  observed=counter(row['host_us']);require(observed>=last_time,'Capture clock regressed');last_time=observed
  total+=len(data);require(total<=8*1024**3,'Capture raw budget');rows.append((row,data))
 require(len(rows)>=2 and rows[0][0]['kind']=='open' and rows[-1][0]['kind']=='close','Capture is not closed')
 first,last=rows[0][0],rows[-1][0]
 require(type(first['version']) is int and first['version']==1 and first['role'] in ('viewport','exports','commands') and type(first['session']) is str and 1<=len(first['session'])<=128,'Capture open identity')
 require(last['complete'] is True and type(last['error']) is str and last['error']=='' and number(last['prior_rows'])==len(rows)-1 and counter(last['raw_bytes'])==total,'Capture incomplete or truncated')
 require(all(r['kind'] not in ('open','close') for r,_ in rows[1:-1]),'Nested capture lifetime')
 return rows

def acquisition(streams,session):
 admitted={}
 for raw in streams:
  rows=capture(raw);first=rows[0][0];require(first['session']==session and first['role'] not in admitted,'Mixed or duplicate collector generations');admitted[first['role']]=rows
 require(set(admitted) in ({'commands','exports','viewport'},{'commands','viewport'}),'Incomplete collector roles')
 exports={};previous_commands=0;retired=False
 for row,body in admitted.get('exports',[])[1:-1]:
  require(not body,'Unexpected export pixels')
  if row['kind']=='retired':
   require(not retired and row['export_leases']==0 and type(row['export_leases']) is int and row['command_owner'] is False and row['world_owner'] is False,'Owner retirement incomplete');retired=True;continue
  require(row['kind']=='export' and not retired,'Export after retirement')
  serial=counter(row['lease']);require(serial==len(exports)+1,'Export serial gap')
  require(number(row['commands'])>=previous_commands and number(row['mode'],0,1) in (0,1),'Export command order/mode');previous_commands=row['commands']
  counter(row['world']);counter(row['sealed_world']);number(row['width'],1,2048);number(row['height'],1,2048);exports[serial]=row
 require('exports' not in admitted or (exports and retired),'No complete owner lifetime')
 paints={};swaps=[];latest=0
 for row,body in admitted['viewport'][1:-1]:
  if row['kind']=='swap':
   index=counter(row['paint']);require(index==latest,'Swap not bound to most recent completed paint')
   if index:swaps.append(index)
   continue
  require(row['kind']=='paint','Unknown viewport observation');index=counter(row['paint']);require(index==latest+1,'Paint history gap');latest=index
  w,h=number(row['width'],1,4096),number(row['height'],1,4096);require((row['pixels_captured'] is True and len(body)==w*h*4) or (row['native_gpu_frame'] is False and row['pixels_captured'] is False and not body),'Whole native viewport plane required')
  lease,world=counter(row['lease']),counter(row['world'])
  if lease and row['presentation_owner']=='gui':
   require(row['immutable_lease'] is False and row['native_gpu_frame'] is True,'GUI mutable source identity')
   number(row['commands'],1,2**32-1);number(row['mode'],0,1);counter(row['sealed_world'])
   binding=index;exports[binding]=dict(row,lease=str(binding),source_lease=row['lease'],width=row['frame_width'],height=row['frame_height']);row['reference_binding']=binding
  elif lease:
   row['reference_binding']=lease
   require(lease in exports and world==counter(exports[lease]['world']) and counter(exports[lease]['host_us'])<=counter(row['host_us']),'Paint/export source identity')
   require(row['transport']=='opengl' and row['gpu_read_complete'] is True,'GPU completion missing')
   require((row['frame_width'],row['frame_height'])==(exports[lease]['width'],exports[lease]['height']),'Paint/export dimensions')
  else:require(world==0 and row['native_gpu_frame'] is False,'Native paint lacks a source lease')
  number(row['scaling'],0,2);require(not lease or (row['transport']=='opengl' and row['gpu_read_complete'] is True),'GPU read completion missing')
  paints[index]=(row,body)
 require(swaps and any(counter(paints[i][0]['world']) for i in swaps),'No actual composed World swap')
 command_rows=admitted['commands'];require(command_rows[-2][0]['kind']=='ended' and all(r['kind'] in ('fragment','cpu-read','ended') for r,_ in command_rows[1:-1]),'Command history did not end')
 command_bytes=b''.join(body for row,body in command_rows if row['kind']=='fragment')
 return dict(exports=exports,paints=paints,swaps=swaps,commands=command_bytes,command_rows=command_rows,
  observations=dict(exports=len(exports),paints=len(paints),swaps=len(swaps),world_swaps=sum(bool(counter(paints[i][0]['world'])) for i in swaps),unpresented_paints=len(set(paints)-set(swaps)),coalesced_exports=len(set(exports)-{paints[i][0].get('reference_binding') for i in swaps})))

def commands(raw):
 require(len(raw)>=16 and raw[:8]==b'MNMCMD05' and struct.unpack_from('<II',raw,8)==(5,16),'Owned V5 command history')
 at=16;seq=0;ended=False
 while at<len(raw):
  require(not ended and len(raw)-at>=12,'Command after END or truncated header')
  op,index,n=struct.unpack_from('<III',raw,at);at+=12;require(index==seq+1 and n<=32*1024**2 and n<=len(raw)-at,'Command history gap/extent')
  payload=raw[at:at+n];at+=n;seq=index;ended=op==8
  yield op,index,payload
 require(ended,'Missing command END')

def rgba(pixels,bits,masks,palette):
 if bits==8:
  require(palette is not None,'Indexed presentation has no palette');return palette[pixels]
 channels=[]
 for i,mask in enumerate(masks):
  low=mask&-mask;maximum=mask//low;value=(pixels&mask)//low
  if masks==(0xf800,0x7e0,31):channels.append((value<<[3,2,3][i])|(value>>[2,4,2][i]))
  else:channels.append(value*255//maximum)
 return np.stack([*channels,np.full(pixels.shape,255)],axis=2).astype('uint8')

class History:
 """Independent NumPy surface history and coordinate lineage; no GL backend."""
 def __init__(self,worlds):
  self.worlds=worlds;self.handles={};self.palette={};self.bindings={};self.world_id=0;self.generation=0;self.sealed=None;self.display=None;self.last_sequence=0;self.reads={};self.counts=Counter()
 def words(self,p,n):require(len(p)>=4*n,'Short command');return list(struct.unpack_from('<'+str(n)+'I',p))
 def image(self,raw,w,h,bits):
  require(1<=w<=2048 and 1<=h<=2048 and bits in (8,16,32) and len(raw)==w*h*(bits//8),'Owned image extent/format');return np.frombuffer(raw,dtype={8:'u1',16:'<u2',32:'<u4'}[bits]).astype('uint32').reshape(h,w)
 def tags(self,s):
  if s['tags'] is None:s['tags']=np.full(s['pixels'].shape,-1,dtype='int64')
  return s['tags']
 def copy(self,w):
  source,target=self.handles[w[0]],self.handles[w[1]];l,t,r,b,x,y=w[2:8];require(0<=l<r<=source['pixels'].shape[1] and 0<=t<b<=source['pixels'].shape[0] and x+r-l<=target['pixels'].shape[1] and y+b-t<=target['pixels'].shape[0],'Copy extent')
  raw=source['pixels'][t:b,l:r].copy();mask=np.ones(raw.shape,dtype=bool) if not w[8] else raw!=w[9]
  if source['tags'] is not None or target['tags'] is not None:
   tags=self.tags(source)[t:b,l:r].copy();dest=self.tags(target)[y:y+b-t,x:x+r-l];dest[mask]=tags[mask]
  if source['bits']==8 and target['bits']==16:
   colors=source['palette'][raw].astype('uint32');raw=((colors[:,:,0]>>3)<<11)|((colors[:,:,1]>>2)<<5)|(colors[:,:,2]>>3)
  else:require(source['bits']==target['bits'] and source['masks']==target['masks'],'Unsupported cross-format copy')
  dest=target['pixels'][y:y+b-t,x:x+r-l];dest[mask]=raw[mask]
 def apply(self,op,seq,p):
  require(seq==self.last_sequence+1,'Reference sequence gap');self.last_sequence=seq;self.counts[op]+=1
  if op==1:
   w=self.words(p,7);require(w[0] not in self.handles,'Duplicate native handle');self.handles[w[0]]=dict(pixels=self.image(p[28:],w[1],w[2],w[3]),bits=w[3],masks=tuple(w[4:]),palette=None,tags=None,clipper=None);return
  if op in (2,20):
   w=self.words(p,5);s=self.handles[w[0]];n=w[3]*w[4]*(s['bits']//8);patch=self.image(p[20:20+n],w[3],w[4],s['bits']);x,y=w[1:3];target=s['pixels'][y:y+w[4],x:x+w[3]];require(target.shape==patch.shape,'Update outside surface')
   mask=np.ones(patch.shape,dtype=bool) if op==2 else np.frombuffer(p[20+n:],dtype='u1').reshape(patch.shape).astype(bool)
   require(op!=20 or len(p)==20+n+w[3]*w[4],'Masked update length');target[mask]=patch[mask]
   if s['tags'] is not None:s['tags'][y:y+w[4],x:x+w[3]][mask]=-1
   return
  if op==3:self.copy(self.words(p,10));return
  if op==4:
   w=self.words(p,3);s=self.handles[w[0]]
   if s['palette'] is None:s['palette']=np.zeros((256,4),dtype='uint8');s['palette'][:,3]=255
   s['palette'][w[1]:w[1]+w[2],:3]=np.frombuffer(p[12:],dtype='u1').reshape(w[2],3);return
  if op==6:
   w=self.words(p,1);s=self.handles[w[0]];self.display=(w[0],s);self.presented=rgba(s["pixels"],s["bits"],s["masks"],s["palette"]).copy();self.presented_generation=self.generation
   if self.world_id:
    self.sealed=dict(surface=w[0],identity=s,pixels=s['pixels'].copy(),tags=None if s['tags'] is None else s['tags'].copy(),generation=self.generation,bits=s['bits'],masks=s['masks'],palette=s['palette'])
   return
  if op==7:
   w=self.words(p,1);s=self.handles.pop(w[0]);self.bindings.pop(w[0],None)
   if self.sealed and (self.sealed['identity'] is s or w[0]==self.world_id):self.sealed=None
   if w[0]==self.world_id:self.world_id=0
   return
  if op==11:
   a,b=self.words(p,2);sa,sb=self.handles[a],self.handles[b];require(sa['pixels'].shape==sb['pixels'].shape,'Swap extent')
   sa['pixels'],sb['pixels']=sb['pixels'],sa['pixels'];sa['tags'],sb['tags']=sb['tags'],sa['tags'];return
  if op==12:
   w=self.words(p,2);require(w[0] not in self.palette,'Palette generation collision');colors=np.zeros((256,4),dtype='uint8');colors[:,:3]=np.frombuffer(p[8:],dtype='u1').reshape(256,3);colors[:,3]=255;self.palette[w[0]]=(w[1],colors);return
  if op==13:
   w=self.words(p,4);generation,colors=self.palette[w[0]];require(w[1]==generation,'Palette generation mismatch');colors[w[2]:w[2]+w[3],:3]=np.frombuffer(p[16:],dtype='u1').reshape(w[3],3)
   for sid,pid in self.bindings.items():
    if pid==w[0]:self.handles[sid]['palette']=colors.copy()
   return
  if op==14:
   w=self.words(p,3)
   if w[1]:
    generation,colors=self.palette[w[1]];require(generation==w[2],'Palette bind generation');self.handles[w[0]]['palette']=colors.copy();self.bindings[w[0]]=w[1]
   else:self.bindings.pop(w[0],None)
   return
  if op==15:self.palette.pop(self.words(p,2)[0]);return
  if op==19:
   w=self.words(p,5);g=w[1]|w[2]<<32;require(g==self.generation+1 and g in self.worlds,'World import generation')
   carry=self.sealed is not None and self.world_id==w[0] and self.handles.get(self.sealed['surface']) is self.sealed['identity'] and self.sealed['tags'] is not None
   if not carry:self.sealed=None
   for s in self.handles.values():s['tags']=None
   s=self.handles[w[0]];require(s['pixels'].shape==self.worlds[g].shape and s['bits']==16,'World import extent/format');s['pixels'][:]=self.worlds[g];s['tags']=np.arange(s['pixels'].size,dtype='int64').reshape(s['pixels'].shape);self.world_id=w[0];self.generation=g;return
  if op==21:
   w=self.words(p,10);self.reads[seq]=self.handles[w[0]]['pixels'].astype('<u2').tobytes();return
  if op in (5,10):raise ValueError('Original diagnostic pixels are not admitted to native pilot input')
  if op in (8,18):return
  if op==16:
   w=self.words(p,3);require(w[1]==0 or (w[1]==1 and w[2]==0),'Pilot reference needs explicit clipper support for this input');self.handles[w[0]]['clipper']=w[1];return
  if op==17:raise ValueError('Pilot reference lacks V3 surface-copy execution; retain the actual scope')
  raise ValueError('Unimplemented actual command '+str(op))
 def render(self,e):
  if e['mode']==0:
   require(self.display is not None and self.display[0]==e['surface'],'Direct output identity');s=self.display[1];return self.presented.copy()
  else:
   require(self.sealed is not None and self.sealed['generation']==counter(e['sealed_world']),'Sealed UI generation identity');s=self.sealed;pixels=s['pixels'].copy();tags=s['tags'];g=counter(e['world'])
   if tags is not None:
    require(g in self.worlds,'Sealed World generation missing');mask=tags>=0;require(np.all(tags[mask]<self.worlds[g].size),'Coordinate lineage extent');pixels[mask]=self.worlds[g].ravel()[tags[mask]]
   bits,masks,palette=s['bits'],s['masks'],s['palette']
  return rgba(pixels,bits,masks,palette)

def viewport(frame,row):
 require(row['scaling'] in (0,2),'Smooth presentation needs separate rounding reference')
 width,height=row['width'],row['height'];x,y,w,h=[number(row[k]) for k in ('left','top','image_width','image_height')]
 require(w>0 and h>0 and x+w<=width and y+h<=height,'Viewport geometry')
 output=np.zeros((height,width,4),dtype='uint8');output[:,:,3]=255
 xs=np.minimum(((np.arange(w)+.5)*frame.shape[1]/w).astype(int),frame.shape[1]-1)
 ys=np.minimum(((np.arange(h)+.5)*frame.shape[0]/h).astype(int),frame.shape[0]-1)
 output[y:y+h,x:x+w]=frame[ys[:,None],xs[None,:]];return output.tobytes()


def compare(admitted,worlds):
 history=History(worlds);exports=iter(admitted['exports'].values());pending=next(exports,None)
 by_lease={}
 for index in admitted['swaps']:
  row,body=admitted['paints'][index];lease=row.get('reference_binding')
  if lease:by_lease.setdefault(lease,[]).append((index,row,body))
 comparisons=[];read_cases=[];captured_reads={}
 for row,body in admitted['command_rows']:
  if row['kind']=='cpu-read':
   sequence=number(row['sequence'],1,2**32-1);require(sequence not in captured_reads,'Duplicate CPU read observation');captured_reads[sequence]=body
 for op,seq,payload in commands(admitted['commands']):
  history.apply(op,seq,payload)
  if op==21:
   require(seq in captured_reads,'Missing GPU-to-CPU return observation');actual=captured_reads.pop(seq);w=history.words(payload,10)
   header=b'MNMCREAD'+struct.pack('<14I',1,64,w[3],w[1],w[2],w[0],w[4],w[5],w[4]*w[5]*2,*w[6:10],1)
   expected=header+history.reads.pop(seq);require(actual==expected,'Independent GPU-to-CPU reply/history mismatch at '+str(seq));read_cases.append(dict(sequence=seq,bytes=len(actual),sha256=sha(actual)))
  while pending is not None and pending['commands']==seq:
   lease=counter(pending['lease'])
   if lease in by_lease:
    frame=history.render(pending);require(frame.shape[:2]==(pending['height'],pending['width']),'Reference export plane extent')
    for index,row,actual in by_lease[lease]:
     expected=viewport(frame,row);a=np.frombuffer(actual,dtype='u1').reshape(row['height'],row['width'],4);b=np.frombuffer(expected,dtype='u1').reshape(a.shape);different=np.any(a!=b,axis=2);count=int(np.count_nonzero(different))
     example=[]
     if count:
      for y,x in np.argwhere(different)[:8]:example.append(dict(x=int(x),y=int(y),actual=a[y,x].tolist(),expected=b[y,x].tolist()))
     comparisons.append(dict(paint=index,lease=counter(row['lease']),reference_binding=lease,immutable_lease=row['immutable_lease'],world=counter(row['world']),pixels=row['width']*row['height'],mismatching_pixels=count,actual_sha256=sha(actual),expected_sha256=sha(expected),examples=example))
   pending=next(exports,None)
 require(pending is None and not captured_reads,'Unconsumed export or CPU read identity')
 require(not history.handles and not history.palette,'Command surfaces/palettes did not retire')
 require(comparisons and any(c['world'] for c in comparisons),'No independently compared World presentation')
 return dict(success=all(not c['mismatching_pixels'] for c in comparisons),comparisons=comparisons,
  world_presentations=sum(bool(c['world']) for c in comparisons),pixels_compared=sum(c['pixels'] for c in comparisons),
  mismatching_pixels=sum(c['mismatching_pixels'] for c in comparisons),cpu_reads=read_cases,command_operations={str(k):v for k,v in sorted(history.counts.items())},remaining_surfaces=len(history.handles),
  scope='Every captured native Qt swap, bound to the immutable export or GUI owner state at paint, including full viewport geometry and composed World/HUD/cursor command history. GPU-to-CPU returns are checked if present; this run reports their actual count. Owned CPU upload bytes are operation inputs; their producer semantics, movie planes without native leases, hidden Qt widget policy and physical scanout remain separate.')


def input_acknowledgements(raw,flow,observed):
 require(len(raw)>=112 and raw[:8]==b'MNMIAK01','Missing original input acknowledgement')
 h=struct.unpack_from('<16I',raw);require(h[2:4]==(1,48) and 1<=h[4]<h[5]==8 and not any(h[6:]) and len(raw)==64+h[4]*48,'Input acknowledgement extent/closure')
 rows=list(struct.iter_unpack('<12I',raw[64:]));require([r[0] for r in rows]==list(range(1,len(rows)+1)),'Input acknowledgement sequence')
 require(all(r[2]==39 and r[4] in (0,1) and 0x400000<=r[5]<0x5d0000 and r[6] and not any(r[8:]) for r in rows),'Input acknowledgement original caller')
 downs=[r for r in rows if r[3]&0x8000];require(len(rows)==4 and len(downs)==2 and all(bool(r[3]&0x8000)==(i%2==0) for i,r in enumerate(rows)),'Incomplete original Right down/release transitions')
 published=flow['inputs'];require([p['phase'] for p in published]==['right-down','focus-release']*2,'Published input phases differ')
 cases=[]
 for index,down in enumerate(downs):
  start,end=published[index*2:index*2+2];require(down[4]==1 and number(start['sequence'])<=down[1]<number(end['sequence']),'Original poll is not the held owned input')
  world,paint=counter(start['presented_world']),counter(start['presented_paint']);require(world>0 and paint in observed['swaps'] and counter(observed['paints'][paint][0]['world'])==world,'Action did not follow an actual World swap')
  cases.append(dict(channel_sequence=down[1],published_sequence=start['sequence'],world_before_action=world,paint_before_action=paint,caller_return_address=hex(down[5]),original_thread=down[6],result=down[3]))
 return dict(verified=True,right_downs=2,releases=2,cases=cases,scope='Actual original import callbacks consumed the Qt-owned Right high bit after a composed World swap; release transitions and independent output follow. No general camera/input semantics claim.')
