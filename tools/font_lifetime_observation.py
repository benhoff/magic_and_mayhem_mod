"""Strict installed SFT generation/glyph custody; no acceptance from a trace alone."""
import struct
from scene_actor_observation import crc32c
from rendering_pilot import require

HEADER=(1,64,0x40209ca7,64*1024**2,128,131072,128)

def word(raw,at):
 require(0<=at<=len(raw)-4,'Font source word extent')
 return struct.unpack_from('<I',raw,at)[0]

def source(raw):
 """Validate portable normalized installed v3 input, independent of heap addresses."""
 require(len(raw)>=40 and word(raw,4)==len(raw) and word(raw,8)==3,'Font source version/size')
 count,rows,profiles=word(raw,12),word(raw,16),word(raw,36)
 require(1<=count<=256 and 1<=rows<=128 and 1<=profiles<=256 and word(raw,32), 'Font source contour domain')
 contours=808;table=contours+profiles*rows*8;base=table+count*4
 require(base<=len(raw),'Font source table extent');frames=[]
 for ordinal in range(count):
  at=base+word(raw,table+ordinal*4);size=word(raw,at)
  require(40<=size<=len(raw)-at and word(raw,at+28)==0,'Font source normalized glyph extent')
  frames.append(raw[at:at+size])
 return dict(count=count,rows=rows,contours=contours,table=table,base=base,frames=frames)

def decode(raw,*,complete=True):
 require(len(raw)>=64 and raw[:8]==b'MNMFLT01','Font custody header')
 header=struct.unpack_from('<16I',raw);version=header[2];require(header[2]in(1,2) and header[3:9]==HEADER[1:] and not any(header[9:]),'Font custody schema/bounds')
 require(len(raw)<=64*1024**2+128,'Font custody byte bound')
 at=64;owner=0;rows=[];pending=[];fonts={};current={};glyphs=[];terminal=None
 while at<len(raw):
  if len(raw)-at<128:
   require(not complete,'Truncated font record');break
  h=list(struct.unpack_from('<32I',raw,at));size,number,kind,thread=h[:4]
  require(128<=size<=1048960 and number==len(rows)+1<=131072 and kind in range(1,7),'Font custody record envelope/order')
  if size>len(raw)-at:
   require(not complete,'Truncated font payload');break
  body=raw[at+128:at+size]
  allowed=set(range(7))|{22}|({7,8,9,10,11,21}if kind==1 else set(range(7,22))if kind==2 else set(range(7,22))|{23}|(set(range(24,28))if version==2 else set())if kind==3 else{7,8,9,10,11}if kind in(4,5)else set(range(7,14)))
  require(not any(h[i]for i in range(32)if i not in allowed),'Font custody reserved fields')
  require(h[22]==(crc32c(body)if body else 0),'Font custody payload CRC')
  require(thread and (not owner or thread==owner),'Font custody foreign owner');owner=thread
  if kind==6:
   require(size==128 and at+size==len(raw) and not h[5] and not h[6] and not h[7] and h[8]==len(fonts) and h[9]==len(glyphs),'Font custody loss/terminal counts')
   require(h[10]==len(current)==0 and h[11]==len(pending)==0 and h[12]==at,'Font custody unretired owner or active call')
   require(h[13]==sum(f.get('superseded',False)and not f.get('retired',False)for f in fonts.values()),'Font custody original retained-source count')
   terminal=h
  elif kind==1:
   require(h[7]==len(fonts)+1 and 1<=h[8]<=4 and h[9]==current.get(h[8],0) and h[10] and h[11]<=31 and h[21]==len(body)==256,'Font load entry identity/path')
   end=body.find(b'\0');require(0<end<256 and not any(body[end:]),'Font source path terminator/tail')
   f=dict(id=h[7],role=h[8],previous=h[9],path=body[:end].decode('ascii'),entry=number,loaded=False,retired=False,superseded=False)
   fonts[h[7]]=f;pending.append((kind,h,body))
  elif kind==2:
   require(pending and pending[-1][0]==1,'Font load return without entry');_,before,path=pending.pop()
   require(h[7:10]==before[7:10] and body[:256]==path and h[21]==256,'Font load return identity/path')
   f=fonts[h[7]];f['result']=h[10];f['returned']=number
   if not h[10]:require(len(body)==256 and not any(h[12:21]),'Failed font load owns a source')
   else:
    image=body[256:];parsed=source(image)
    require(h[11]in(15,31) and h[12]==len(image) and h[13:18]==[parsed[k]for k in('count','rows','contours','table','base')],'Font load derived offsets/counts')
    require(h[18]==word(image,20)+word(image,24),'Font line height differs from source')
    f.update(loaded=True,image=image,frames=parsed['frames'],glyph_count=parsed['count'],rows=parsed['rows'],tab=h[19],tracking=h[20])
    if h[9]:fonts[h[9]]['superseded']=True
    current[h[8]]=h[7]
  elif kind==3:
   require(size==128 and h[7]in fonts and fonts[h[7]]['loaded'] and not fonts[h[7]]['retired'],'Font glyph uses missing/unloaded/retired generation')
   f=fonts[h[7]]
   require(h[8]==f['role'] and h[21]==current.get(f['role'],0) and h[21] and h[23]==int(f['superseded']),'Font glyph current/reload owner differs')
   require(h[9]<len(f['frames']) and h[12]==len(f['frames'][h[9]]) and h[13]==crc32c(f['frames'][h[9]]),'Font glyph source differs from loaded SFT')
   require(h[10]>0 and h[11]>0 and h[14]==h[5] and h[20]>0,'Font glyph CPS identity missing')
   if version==2:require((h[24]or h[25])and(h[26]or h[27]),'Font glyph source context missing')
   glyphs.append(dict(world_generation=h[24]|h[25]<<32,source_ordinal=h[26]|h[27]<<32,font=h[7],role=h[8],ordinal=h[9],sequence=h[10],canvas=h[11],queue=h[14],frame_crc=h[13],header=h))
  elif kind==4:
   require(size==128 and 1<=h[8]<=4 and h[7]==h[9]==current.get(h[8],0) and h[7] and h[10] and h[11]in(15,31),'Font destructor entry identity')
   pending.append((kind,h,body))
  else:
   require(size==128 and pending and pending[-1][0]==4,'Font destructor return without entry');_,before,_=pending.pop()
   require(h[7:10]==before[7:10] and h[11]==0,'Font destructor failed to clear owned fields')
   f=fonts[h[7]];require(not f['retired'],'Font generation retired twice');f['retired']=True;f['retirement']=number;del current[h[8]]
  rows.append(dict(header=h,payload=body));at+=size
  if terminal:break
 require(not complete or terminal is not None,'Font custody closure missing')
 return dict(version=version,complete=terminal is not None,owner=owner,rows=rows,fonts=fonts,glyphs=glyphs,terminal=terminal,original_reload_retained_generations=terminal[13]if terminal else None,closes_acceptance=False)
