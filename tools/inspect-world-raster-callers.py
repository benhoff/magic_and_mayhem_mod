#!/usr/bin/env python3
"""Generate a reviewed, build-pinned World raster caller whitelist from observed sources."""
import argparse,hashlib,importlib.util,json,re,struct,subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
HASH='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
TRAIL_CONTEXT = '/* Pinned trail leaf continuations require the enclosing queue return too. */\nstatic const struct TrailRasterSite {u32 address;u8 bytes[10];} trail_raster_sites[]={\n {0x4df8b3,{0x8b,0x4c,0x24,0x10,0x8b,0x44,0x24,0x24,0x3,0xf9}},\n {0x4df8e8,{0x8b,0x4c,0x24,0x14,0x8b,0x44,0x24,0x20,0x3,0xf1}},\n {0x4df920,{0x8b,0x4c,0x24,0x24,0x8b,0x54,0x24,0x14,0x3,0xe9}},\n {0x4df95f,{0x3,0xeb,0x8b,0x44,0x24,0x10,0x3,0xf8,0x8b,0x44}},\n {0x4df995,{0x5f,0x5e,0x5d,0x5b,0x83,0xc4,0xc,0xc2,0x14,0x0}},\n};\nstatic const struct TrailQueueReturn {u32 address;u8 bytes[10];} trail_queue_returns[]={\n {0x5006e0,{0xe9,0x67,0x1,0x0,0x0,0x8b,0x56,0xc,0x8b,0x4e}},\n {0x500f51,{0xe9,0xd0,0x1,0x0,0x0,0xf,0xbf,0x46,0x6,0x8b}},\n};\nstatic inline int mnm_world_trail_site(u32 caller){\n for(u32 i=0;i<sizeof(trail_raster_sites)/sizeof(trail_raster_sites[0]);++i)if(caller==trail_raster_sites[i].address)return 1;\n return 0;\n}\nstatic inline int mnm_world_trail_raster_caller(u32 caller,u32 tag,const u8* bytes,u32 outer,const u8* outer_bytes){\n if(tag!=57||!bytes||!outer_bytes)return 0;\n int leaf=0,parent=0;\n for(u32 i=0;i<sizeof(trail_raster_sites)/sizeof(trail_raster_sites[0]);++i)if(caller==trail_raster_sites[i].address){\n  for(u32 j=0;j<10;++j)if(bytes[j]!=trail_raster_sites[i].bytes[j])return 0;\n  leaf=1;break;\n }\n if(!leaf)return 0;\n for(u32 i=0;i<sizeof(trail_queue_returns)/sizeof(trail_queue_returns[0]);++i)if(outer==trail_queue_returns[i].address){\n  for(u32 j=0;j<10;++j)if(outer_bytes[j]!=trail_queue_returns[i].bytes[j])return 0;\n  parent=1;break;\n }\n return parent;\n}\nstatic inline u32 mnm_world_trail_parent(u32 caller,u32 tag,const u8* prologue,const u32* stack,u32 stack_bytes){\n const u8 signature[7]={0x83,0xec,0x0c,0x53,0x55,0x56,0x57};\n if(tag!=57||!mnm_world_trail_site(caller)||!prologue||!stack||stack_bytes<48)return 0;\n for(u32 i=0;i<7;++i)if(prologue[i]!=signature[i])return 0;\n return stack[11];\n}\n'
def main():
 p=argparse.ArgumentParser();p.add_argument('--stream',type=Path,required=True);p.add_argument('--header',type=Path,required=True);p.add_argument('--report',type=Path,required=True);a=p.parse_args()
 subprocess.run([ROOT/'tools/original-manifest.sh','verify'],check=True)
 try:
  exe=ROOT/'original/Arcane_Nocd/Chaos.exe';raw=exe.read_bytes();assert hashlib.sha256(raw).hexdigest()==HASH
  pe=struct.unpack_from('<I',raw,60)[0];ns,opt=struct.unpack_from('<H',raw,pe+6)[0],struct.unpack_from('<H',raw,pe+20)[0]
  def at(va,n):
   for i in range(ns):
    row=pe+24+opt+i*40;size,rva,rs,off=struct.unpack_from('<4I',raw,row+8);d=va-0x400000-rva
    if 0<=d and d+n<=rs:return raw[off+d:off+d+n]
   raise ValueError('Unbacked caller')
  s=importlib.util.spec_from_file_location('inspector',ROOT/'tools/inspect-canvas-producers.py');m=importlib.util.module_from_spec(s);s.loader.exec_module(m);_,ops=m.decode(a.stream.read_bytes())
  callers={};active=False;tags={0:[12],1:[13,14],2:[15,16],3:[17],4:[18],5:[19],6:[20],7:[21],8:[22,36],9:[23,24],10:[25],13:[57],14:[58],15:[59],16:[60]}
  for r,_ in ops:
   if r[2]==11:active=True
   elif r[2]==12:active=False
   elif active and r[2] in (14,26):
    allowed={14:{0x54ab16,0x54aab2,0x54657c},26:{0x54ab72}}
    if r[22] not in allowed[r[2]]:raise ValueError("Primitive caller outside pinned wrapper return")
    callers.setdefault(r[22],set()).add(26 if r[2]==14 else 52)
   elif active and r[2]==9:
    if r[15] not in tags:raise ValueError('Unreviewed raster backend')
    callers.setdefault(r[22],set()).update(tags[r[15]])
  # This statically pinned inherited class invokes the same validated additive
  # raster. Preserve it even when one finite observed stream has no such row.
  if at(0x5c73fc,4)!=struct.pack('<I',0x54aaa0) or at(0x54aaa0,21).hex()!='8b4424088b542404505283c134e85eb1ffffc20800':raise ValueError('Inherited kind8 wrapper changed')
  callers.setdefault(0x54aab2,set()).add(26)
  # Checked direct call/return bindings are retained independently of capture.
  effects={5244405: 58, 5244435: 57, 5244465: 60, 5244502: 59, 5244536: 59, 5244573: 59, 5245707: 57, 5245741: 60, 5245782: 59, 5245820: 59, 5245861: 59}
  targets={57:0x580a90,58:0x57fe10,59:0x581200,60:0x580260}
  for va,tag in effects.items():
   call=at(va-5,5)
   if call[0]!=0xe8 or va+struct.unpack('<i',call[1:])[0]!=targets[tag]:raise ValueError('Effect direct call binding changed')
   callers.setdefault(va,set()).add(tag)
  rows=[];header=['/* Pinned World continuations and the checked inherited kind8 extension. */','static const struct RasterCaller {u32 address;unsigned long long tags;u8 bytes[10];} raster_callers[]={']
  for va,ts in sorted(callers.items()):
   # These ranges were reviewed through the AX stores/masks and queue loop.
   if not (0x5002a0<=va<0x501150 or 0x57e8e0<=va<0x57eb00 or va in (0x54ab16,0x54aab2,0x54657c,0x54ab72)):raise ValueError('Caller outside reviewed World/wrapper range')
   sig=at(va,10);mask=sum(1<<t for t in ts);header.append(' {0x%x,0x%xull,{%s}},'%(va,mask,','.join(hex(x) for x in sig)))
   asm=subprocess.check_output(['objdump','-d','-Mintel','--start-address='+hex(va),'--stop-address='+hex(min(va+80,0x501150 if va<0x501150 else va+80)),str(exe)],text=True)
   origin='checked_inherited_wrapper' if va==0x54aab2 else 'checked_effect_return' if va in effects else 'observed_stream'
   scope='Checked direct effect call and AX-only continuation; this static extension is not natural reachability.' if va in effects else 'Pinned54aaa0 RET8 continuation to virtual caller; this static extension alone is not a natural-live witness.' if va==0x54aab2 else 'Observed return path consumes low16 AX only; callee-saved registers preserved, volatile registers/flags overwritten before subsequent use on reviewed path.'
   rows.append({'address':hex(va),'tags':sorted(ts),'return_signature':sig.hex(),'disassembly':asm,'origin':origin,'scope':scope})
  header+=['};', '''/* Bytes are an explicitly readable ten-byte continuation. */
static inline int mnm_world_raster_caller(u32 caller,u32 tag,const u8* bytes){
 if(tag>=64||!bytes)return 0;
 for(u32 i=0;i<sizeof(raster_callers)/sizeof(raster_callers[0]);++i)
  if(caller==raster_callers[i].address&&(raster_callers[i].tags&(1ull<<tag))){
   for(u32 j=0;j<10;++j)if(bytes[j]!=raster_callers[i].bytes[j])return 0;
   return 1;
  }
 return 0;
}'''];a.header.open('x').write('\n'.join(header)+'\n')
  trail_sites=[0x4df8b3,0x4df8e8,0x4df920,0x4df95f,0x4df995];trail_parents=[0x5006e0,0x500f51]
  if at(0x4df840,7)!=bytes.fromhex('83ec0c53555657'):raise ValueError('Trail stack prologue changed')
  trail_rows=[]
  for addresses,target,kind in [(trail_sites,0x580a90,'leaf'),(trail_parents,0x4df840,'parent')]:
   for va in addresses:
    call=at(va-5,5)
    if call[0]!=0xe8 or va+struct.unpack('<i',call[1:])[0]!=target:raise ValueError('Trail direct binding changed')
    trail_rows.append(dict(address=hex(va),kind=kind,return_signature=at(va,10).hex()))
  # This reviewed guard is part of the generator, independently of observations.
  trailer=TRAIL_CONTEXT
  for row in trail_rows:
   literal=' {0x%x,{%s}},'%(int(row['address'],16),','.join(hex(x) for x in bytes.fromhex(row['return_signature'])))
   if literal not in trailer:raise ValueError('Trail guard template does not match original bytes')
  with a.header.open('a') as output:output.write('\n'+trailer)
  # Keep the reviewed primary-only sprite policy independent of observations.
  policy_path=ROOT/'runtime/scene/world_raster_callers.h';policy=policy_path.read_text()
  marker='/* Selected sprite virtual methods require the primary kind8 row/call context. */'
  sprite_policy=policy[policy.index(marker):].strip()+'\n'
  expected=[(0x549333,0x5c74e4,0x5492b0,0x6f2150,7,0x1000400000),(0x549354,0x5c74e4,0x5492b0,0x6f2150,9,0x6000),(0x549369,0x5c74e4,0x5492b0,0x6f2150,9,0x20000),(0x5497d3,0x5c74fc,0x549730,0x6f2268,7,0x1000400000),(0x5497f4,0x5c74fc,0x549730,0x6f2268,9,0x6000),(0x549809,0x5c74fc,0x549730,0x6f2268,9,0x20000)]
  matches=re.findall(r'\{(0x[0-9a-f]+),(0x[0-9a-f]+),(0x[0-9a-f]+),(0x[0-9a-f]+),([0-9]+),(0x[0-9a-f]+)ull,\{([^}]+)\}\}',sprite_policy)
  if [tuple(int(x,0) for x in row[:6]) for row in matches]!=expected:raise ValueError('Sprite context policy bindings changed')
  if at(0x5006a2,16).hex()!='8b560c8b4e04528b56088b0152ff500c' or at(0x5006b2,10).hex()!='e9910100000fbf4e120f':raise ValueError('Primary virtual call context changed')
  sprite_rows=[]
  for fields,row in zip(expected,matches):
   va,table,method,bank,parent_word,mask=fields;sig=bytes(int(x,0) for x in row[6].split(','))
   if at(va,10)!=sig or at(table+12,4)!=struct.pack('<I',method) or at(method,9)!=b'\x8b\x15'+struct.pack('<I',bank)+b'\x53\x56\x57':raise ValueError('Sprite method/caller identity changed')
   if mask==0x20000:
    call=at(va-5,5)
    if call[0]!=0xe8 or va+struct.unpack('<i',call[1:])[0]!=0x57de00:raise ValueError('Sprite clipped call binding changed')
   elif at(va-6,6)!=b'\xff\x15'+struct.pack('<I',0x6903dc if parent_word==7 else 0x6c4e3c):raise ValueError('Sprite indirect call slot changed')
   sprite_rows.append(dict(address=hex(va),table=hex(table),method=hex(method),tags=[tag for tag in range(64) if mask&(1<<tag)],parent_word=parent_word,row_word=parent_word-2,outer='0x5006b2',return_signature=sig.hex(),origin='checked_sprite_primary_context'))
  with a.header.open('a') as output:output.write('\n'+sprite_policy)
  report={'success':True,'source_executable_sha256':HASH,'caller_count':len(rows),'callers':rows,'trail_context':trail_rows,'kind8_sprite_context':sprite_rows,'sources':{str(Path(__file__).relative_to(ROOT)):hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),str(policy_path.relative_to(ROOT)):hashlib.sha256(policy_path.read_bytes()).hexdigest()},'input_stream':{'path':str(a.stream),'sha256':hashlib.sha256(a.stream.read_bytes()).hexdigest()},'generated_header_sha256':hashlib.sha256(a.header.read_bytes()).hexdigest(),'scope':'Static observed-caller whitelist and bytes; not pixel equivalence or whole-function recovery.'};a.report.open('x').write(json.dumps(report,indent=2)+'\n')
 finally:subprocess.run([ROOT/'tools/original-manifest.sh','verify'],check=True)
if __name__=='__main__':main()
