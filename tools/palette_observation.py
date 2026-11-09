"""Decode passive original palette custody; no inferred cycling acceptance."""
import struct
from scene_actor_observation import crc32c
from rendering_pilot import require

def decode(raw, *, complete=True):
 require(len(raw)>=64 and raw[:8]in(b'MNMPAL01',b'MNMPAL02',b'MNMPAL03'),'Palette header missing')
 header=struct.unpack_from('<16I',raw)
 version=header[2];require(version in(1,2,3)and raw[:8]=={1:b'MNMPAL01',2:b'MNMPAL02',3:b'MNMPAL03'}[version]and header[3:8]==(64,0x40209ca7,128*1024**2,4096,65536) and (header[8]in(0,4)if version==3 else header[8]==0)and not any(header[9:]),'Palette header contract')
 require(len(raw)<=header[5]+128,'Palette stream exceeds bound')
 at=64;owner=0;rows=[];nodes={};pending=[];terminal=None;last_queue=0
 while at<len(raw):
  if len(raw)-at<128:
   require(not complete,'Truncated palette record');break
  h=struct.unpack_from('<32I',raw,at);size,ordinal,kind,thread=h[:4]
  require(128<=size<=263040 and ordinal==len(rows)+1 and kind in range(1,9 if version==3 else 7) and not h[31],'Palette record envelope/order')
  if at+size>len(raw):
   require(not complete,'Truncated palette payload');break
  body=raw[at+128:at+size]
  allowed={1:{9,10,13,14},2:{8,9,10,11,13,14,15},3:{8,13,14},4:{8,13,14,15},5:set(range(8,30)),6:{7,8,9,10,11,12},7:{7,8,9,10,11,13,14},8:{7,8,9,10,11,13,14,15}}[kind]|set(range(7))|{30}
  if version>=2 and kind in(1,2,3,4,7,8):allowed|={16,17,18,19};require(0<h[16]<=65535 and h[17]<=65535 and h[18]<=65535 and h[19]<=255,'Palette floating input extent')
  if version==3 and kind==5:allowed.add(7)
  require(not any(h[i]for i in range(32)if i not in allowed),'Palette nonzero reserved field')
  if kind==6:
   require(size==128 and at+128==len(raw) and not any(h[4:8]) and h[8]==len(nodes) and h[9]==at and h[11]==sum(n['live']for n in nodes.values()) and h[12]==len(pending)==0 and not any(h[13:]),'Palette loss/terminal custody')
   require(thread==owner,'Palette terminal owner');terminal=list(h)
  else:
   require(thread and (owner==0 or owner==thread),'Foreign palette owner');owner=thread
   if h[5]:require(h[5]>=last_queue,'Palette queue regression');last_queue=h[5]
   require(h[30]==(crc32c(body)if body else 0),'Palette payload CRC')
   if kind==1:
    require(size==976 and h[13]<10 and h[9]in(2,4,8,16,32,64,128,256) and h[10]in(0,1),'Palette builder input')
    pending.append(dict(kind=kind,header=list(h),input=body))
   elif kind==2:
    require(pending and pending[-1]['kind']==1,'Palette builder return without entry');entry=pending.pop()
    require(h[13:15]==tuple(entry['header'][13:15]) and h[9]==entry['header'][9],'Palette builder return identity')
    if not h[8]:require(not body and h[11]==0,'Failed builder owns output')
    else:
     require(h[8]==len(nodes)+1 and h[15] and h[10]==entry['header'][10],'Palette allocation identity/result')
     count=h[9];mode=h[13];tables=count-1 if mode<8 else count//2;stride=2 if mode<8 else 4
     require(h[11]==12+tables*256*stride and len(body)==h[11]+768,'Palette table extent')
     next_node,shift,neutral=struct.unpack_from('<3I',body)
     require(next_node==0 and shift==8-(count.bit_length()-1) and neutral==(count-1)//2,'Palette node fields')
     nodes[h[8]]=dict(id=h[8],mode=mode,count=count,stride=stride,tables=tables,shift=shift,neutral=neutral,
                     entry_record=entry['header'][1],input=entry['input'],floating_input=entry['header'][16:20]if version>=2 else None,floating_return=list(h[16:20])if version>=2 else None,data=body[:h[11]],initial_data=body[:h[11]],source_after=body[h[11]:],live=True,birth=ordinal,revision=0,refreshes=[])
   elif kind==3:
    require(size==128 and h[13]==10 and h[8]in nodes and nodes[h[8]]['live'],'Palette retirement entry')
    pending.append(dict(kind=kind,header=list(h)))
   elif kind==4:
    require(size==128 and pending and pending[-1]['kind']==3,'Palette retirement return without entry');entry=pending.pop()
    require(h[8]==entry['header'][8] and h[13:15]==tuple(entry['header'][13:15]) and nodes[h[8]]['live'],'Palette retirement identity')
    nodes[h[8]]['live']=False;nodes[h[8]]['retirement']=ordinal
   elif kind==7:
    require(h[8]in nodes and nodes[h[8]]['live'],'Refresh unknown/retired allocation')
    node=nodes[h[8]]
    require(h[13]==(12 if node['mode']==1 else 13 if node['mode']==9 else 0)and h[9]==node['count']and h[7]==node['revision']and h[11]==len(node['data'])and len(body)==872+h[11]and body[872:]==node['data'],'Refresh input/version/preimage differs')
    require(h[10]in(0,1),'Refresh format')
    pending.append(dict(kind=kind,header=list(h),input=body[:872],before=body[872:]))
   elif kind==8:
    require(pending and pending[-1]['kind']==7,'Refresh return without entry');entry=pending.pop();before=entry['header'];node=nodes[h[8]]
    require(h[8:15]==tuple(before[8:15])and h[7]==before[7]+bool(h[15])and len(body)==h[11]+768,'Refresh return identity/version')
    require(body[:4]==bytes(4)and struct.unpack_from('<2I',body,4)==(node['shift'],node['neutral']),'Refresh relink/header changed')
    require(h[15]or body[:h[11]]==entry['before'],'Failed refresh mutated output')
    node['refreshes'].append(dict(entry_record=before[1],return_record=ordinal,revision=h[7],input=entry['input'],before=entry['before'],after=body[:h[11]],source_after=body[h[11]:],floating_input=before[16:20],floating_return=list(h[16:20]),result=h[15],caller=h[14],queue=h[5]))
    node['data']=body[:h[11]];node['revision']=h[7]
   else:
    require(size==128+(8 if version==3 else 4)*h[27] and h[27]==h[10]+1 and 1<=h[27]<=19 and h[25]>=40,'Palette selection chain extent')
    links=struct.unpack('<'+'I'*(h[27]*(2 if version==3 else 1)),body);chain=links[::2]if version==3 else links
    if version==3:require(all(i in nodes and nodes[i]['revision']==v for i,v in zip(chain,links[1::2]))and h[7]==nodes[h[8]]['revision'],'Palette selection lost refreshed version')
    require(len(set(chain))==len(chain) and chain[0]==h[9] and chain[-1]==h[8] and all(i in nodes and nodes[i]['live']for i in chain),'Palette selection uses unknown/retired/cyclic chain')
    node=nodes[h[8]];require(h[28:30]==(node['shift'],node['neutral']) and h[11]<node['tables'],'Palette selection changed header/table')
    data=node['data'][12+h[11]*256*node['stride']:12+(h[11]+1)*256*node['stride']]
    if node['stride']==4:
     words=struct.unpack('<256I',data);require(all(w<=65535 for w in words),'Palette DWORD high bits');data=struct.pack('<256H',*words)
    require(crc32c(data)==h[12],'Selected palette differs from recorded production')
  rows.append(dict(header=list(h),payload=body));at+=size
  if terminal:break
 require(not complete or terminal is not None,'Palette closure missing')
 return dict(version=version,full_world_prefix=header[8]if version==3 else 0,complete=terminal is not None,owner=owner,nodes=nodes,rows=rows,terminal=terminal,
             original_refresh_proved=False,cycling_rows_passed=False)

def decode_profiles(raw,observation,*,complete=True):
 """Actual platform transcript, bound to original builder entry ordinals."""
 require(64<=len(raw)<=32*1024**2 and raw[:8]==b'MNMPPR01','Palette profile header')
 h=struct.unpack_from('<16I',raw);require(h[2:8]==(1,64,0x40209ca7,20480,1344,32*1024**2)and not any(h[8:]),'Palette profile bounds')
 entries={r['header'][1]:r['header']for r in observation['rows']if r['header'][2]==1};nodes={n['entry_record']:n['id']for n in observation['nodes'].values()};at=64;rows=[];groups={};terminal=None
 while at<len(raw):
  if raw[at:at+8]==b'MNMPPEND':
   require(len(raw)-at==64,'Palette profile terminal extent');t=struct.unpack_from('<16I',raw,at)
   require(t[2:8]==(1,64,len(rows),at,0,observation['owner'])and not any(t[8:]),'Palette profile loss/terminal');terminal=list(t);at+=64;break
  if len(raw)-at<1344:
   require(not complete,'Truncated palette profile record');break
  f=struct.unpack_from('<16I',raw,at);body=raw[at+64:at+1344]
  require(f[0]==1344 and f[1]==len(rows)+1<=20480 and f[2]in entries and f[3]in(4,5,7)and entries[f[2]][13]==f[3]and f[4]==observation['owner']and f[9]<f[10]==256 and f[12]==crc32c(body)and not any(f[13:]),'Palette profile record identity/result/CRC')
  texts=[]
  for i in range(5):
   block=body[i*256:(i+1)*256];end=block.find(b'\0');require(end>=0 and not any(block[end:]),'Palette profile string extent');texts.append(block[:end])
  require(texts[0]and texts[1]and texts[3]and len(texts[4])==f[9],'Palette profile returned text length')
  row=dict(header=list(f),texts=texts,node=nodes.get(f[2]));rows.append(row);groups.setdefault(f[2],[]).append(row);at+=1344
 require(not complete or terminal is not None,'Palette profile closure missing')
 if complete:require(all(entry in nodes for entry in groups),'Profile parent lacks normal builder return')
 return dict(complete=terminal is not None,rows=rows,groups=groups,terminal=terminal)
