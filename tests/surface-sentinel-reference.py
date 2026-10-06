"""Unchanged x86 wrappers/bitmap loader in Unicorn; explicit CRT/COM/GDI boundaries."""
import hashlib,struct
import unicorn
from unicorn import Uc,UC_ARCH_X86,UC_MODE_32,UC_HOOK_CODE
from unicorn.x86_const import UC_X86_REG_EAX,UC_X86_REG_ECX,UC_X86_REG_ESP,UC_X86_REG_EIP
BUILD_HASH='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
RANGES=[(0x486950,0x486960),(0x486960,0x486991),(0x486a80,0x486ad5),(0x486ae0,0x486c7c),(0x4a2f90,0x4a2fa2),(0x58d1a0,0x58d238),(0x58bac0,0x58bb87),(0x58bc10,0x58bd08),(0x58c4a0,0x58c8a0),(0x58c8a0,0x58ca90)]
STOP=0x1000f000
class Reference:
 def __init__(self,image):
  if hashlib.sha256(image).hexdigest()!=BUILD_HASH:raise ValueError('Unsupported image')
  pe=struct.unpack_from('<I',image,60)[0];opt=pe+24;base=struct.unpack_from('<I',image,opt+28)[0];length=struct.unpack_from('<I',image,opt+56)[0];assert base==0x400000 and length<32*1024*1024
  self.sections=[];at=opt+struct.unpack_from('<H',image,pe+20)[0]
  for i in range(struct.unpack_from('<H',image,pe+6)[0]):
   _,rva,n,raw=struct.unpack_from('<4I',image,at+i*40+8);assert raw+n<=len(image) and rva+n<=length;self.sections.append((base+rva,image[raw:raw+n]))
  self.length=(length+4095)&~4095
 def run(self,c,asset,budget=12000):
  u=Uc(UC_ARCH_X86,UC_MODE_32);u.mem_map(0,4096);u.mem_map(0x400000,self.length);u.mem_map(0x10000000,0x20000);u.mem_map(0x11000000,0x400000);u.mem_map(0x20000000,0x100000)
  for at,data in self.sections:u.mem_write(at,data)
  originals={a:bytes(u.mem_read(a,b-a)) for a,b in RANGES};events=[];executed=set();allocs={};heap=0x11100000;fileat=0;drawat=0;instructions=0
  def words(at,n):return list(struct.unpack('<'+'I'*n,u.mem_read(at,n*4)))
  def put(at,*values):u.mem_write(at,struct.pack('<'+'I'*len(values),*values))
  def string(at):
   raw=bytes(u.mem_read(at,256));return raw.split(b'\0',1)[0].decode('ascii')
  def ev(kind,**fields):
   if len(events)>=256:raise ValueError('Endpoint budget')
   events.append(dict(kind=kind,**fields))
  table=0x11000000
  def object(id):return 0x11001000+id*256
  def com(id):return 0x11002000+id*16
  for id in (1,2,3):put(com(id),table,id);put(object(id)+8,com(id));put(object(id)+44,0x12345678 if id==1 else 0xabcde123)
  for slot in [5,7,17,26,27,29]:put(table+slot*4,0x10000000+slot*16)
  put(0x642008,com(3) if c['global_surface'] else 0)
  for id in (1,2):
   mode=c['source_reload' if id==1 else 'destination_reload'];put(object(id)+40,0xffffffff if mode==2 else 0x11003000+id*16 if mode==1 else 0)
   put(0x11003000+id*16,0x11003100,id)
  put(0x11003100,0x10001000);put(0x6f68d8,c['fast']);put(0x6f98d8,c['no_wait']);put(0x6f98c0,0);put(0x6de6dd,c['key_enabled']);put(0x5c5034,0x10002000)
  rect=0x11004000;put(rect,1,1,5,4)
  # Bound the original error dispatch at its call boundary; no modal UI here.
  backends={0x59cae2:'open',0x59c359:'read',0x59c4c3:'close',0x597880:'alloc',0x5979b0:'free',0x484c00:'report'}
  def hook(mu,address,size,data):
   nonlocal fileat,heap,drawat,instructions
   instructions+=1
   if instructions>budget:raise ValueError('Instruction budget exhausted')
   if address==STOP:mu.emu_stop();return
   sp=mu.reg_read(UC_X86_REG_ESP);args=words(sp+4,14);returnto=words(sp,1)[0];result=0;pop=0
   if address==0x4a2f90:ev('cursor_reload',surface=3);executed.add(address)
   elif address==0x58d1a0:ev('bitmap_reload',surface=3 if mu.reg_read(UC_X86_REG_ECX)==0x642000 else 2,path=string(args[0]));executed.add(address)
   elif address in [a for a,b in RANGES]:executed.add(address)
   if address in backends:
    kind=backends[address]
    if kind=='open':ev(kind,path=string(args[0]),mode=string(args[1]),present=asset is not None);fileat=0;result=0x11005000 if asset is not None else 0
    elif kind=='read':
     target,each,count,file=args[:4];assert file==0x11005000 and each==1;size=each*count;assert size<=2*1024*1024;chunk=asset[fileat:fileat+size];fileat+=len(chunk)
     if chunk:mu.mem_write(target,chunk)
     result=len(chunk);ev(kind,requested=size,returned=result)
    elif kind=='close':assert args[0]==0x11005000;ev(kind)
    elif kind=='alloc':
     n=args[0];assert 0<n<=2*1024*1024
     if c['allocation_failure'] and not allocs:result=0
     else:result=heap;heap+=(n+15)&~15;assert heap<0x11400000;allocs[result]=n
     ev(kind,bytes=n,success=bool(result))
    elif kind=='free':assert args[0] in allocs;ev(kind,bytes=allocs.pop(args[0]))
    elif kind=='report':pop=4;ev(kind,result=args[0])
   elif 0x10000000<=address<0x10001000:
    slot=(address-0x10000000)//16;id=words(args[0]+4,1)[0]
    if slot in (5,7):
     assert drawat<len(c['draw_results']);result=c['draw_results'][drawat];drawat+=1;pop=24
     flags=args[4] if slot==5 else args[5];fill=slot==5 and args[2]==0;ev('draw',surface=id,fast=slot==7,flags=flags,result=result,color=words(args[5]+80,1)[0] if fill else 0)
    elif slot==27:result=c['source_restore' if id==1 else 'destination_restore'];pop=4;ev('restore',surface=id,result=result)
    elif slot==29:result=c['key_result'];pop=12;ev('key',surface=id,flags=args[1],range=words(args[2],2),result=result)
    elif slot==17:
     result=c['get_dc'];put(args[1],0x1234);pop=8;ev('get_dc',surface=id,result=result)
    elif slot==26:result=c['release_dc'];pop=8;assert args[1]==0x1234;ev('release_dc',surface=id,result=result)
    else:raise ValueError('Unknown COM endpoint')
   elif address==0x10001000:
    id=words(mu.reg_read(UC_X86_REG_ECX)+4,1)[0];pop=4;ev('callback',surface=id,argument=args[0])
   elif address==0x10002000:
    hdc,x,y,w,h,sx,sy,sw,sh,bits,info,usage,rop=args[:13];assert hdc==0x1234 and x==y==sx==sy==0 and sw==w and sh==h and rop==0xcc0020
    header=bytes(mu.mem_read(info,40));bpp=struct.unpack_from('<H',header,14)[0];palcount={1:2,4:16,8:256}.get(bpp,0);pal=bytes(mu.mem_read(info+40,palcount*4));data=bytes(mu.mem_read(bits,allocs[bits]));result=c['dib_result'];pop=52
    ev('dib',width=w,height=h,bits=bpp,usage=usage,rop=rop,info_sha256=hashlib.sha256(header).hexdigest(),palette_sha256=hashlib.sha256(pal).hexdigest(),pixels_sha256=hashlib.sha256(data).hexdigest(),result=result)
   else:
    if not any(a<=address<b for a,b in RANGES):raise ValueError(f'Unbounded instruction target {address:x}')
    return
   mu.reg_write(UC_X86_REG_EAX,result&0xffffffff);mu.reg_write(UC_X86_REG_ESP,sp+4+pop);mu.reg_write(UC_X86_REG_EIP,returnto)
  u.hook_add(UC_HOOK_CODE,hook)
  entry=c['entry'];args=([rect,c['color']] if entry==0x58bac0 else [c['color']] if entry==0x58bc10 else [object(2),rect,2,2] if entry in (0x58c4a0,0x58c8a0) else [0x5e04d0] if entry==0x58d1a0 else [])
  sp=0x200ff000;put(sp,STOP,*args);u.reg_write(UC_X86_REG_ESP,sp);u.reg_write(UC_X86_REG_ECX,object(1) if entry in (0x58c4a0,0x58c8a0) else 0x642000 if entry==0x58d1a0 else object(2))
  u.emu_start(entry,STOP+1,timeout=1000000,count=budget+1)
  if u.reg_read(UC_X86_REG_EIP)!=STOP:raise ValueError('Original did not return within budget')
  assert not allocs and words(0,1)==[0] and all(bytes(u.mem_read(a,len(v)))==v for a,v in originals.items()),'Original text/SEH/allocation cleanup differs'
  return dict(id=c['id'],events=events,draws_consumed=drawat,return_value=u.reg_read(UC_X86_REG_EAX),executed_entries=sorted(executed),instructions=instructions)
