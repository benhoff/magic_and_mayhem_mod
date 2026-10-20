#!/usr/bin/env python3
"""Generate a reviewed, build-pinned World raster caller whitelist from observed sources."""
import argparse,hashlib,importlib.util,json,struct,subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
HASH='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
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
  report={'success':True,'source_executable_sha256':HASH,'caller_count':len(rows),'callers':rows,'sources':{str(Path(__file__).relative_to(ROOT)):hashlib.sha256(Path(__file__).read_bytes()).hexdigest()},'input_stream':{'path':str(a.stream),'sha256':hashlib.sha256(a.stream.read_bytes()).hexdigest()},'generated_header_sha256':hashlib.sha256(a.header.read_bytes()).hexdigest(),'scope':'Static observed-caller whitelist and bytes; not pixel equivalence or whole-function recovery.'};a.report.open('x').write(json.dumps(report,indent=2)+'\n')
 finally:subprocess.run([ROOT/'tools/original-manifest.sh','verify'],check=True)
if __name__=='__main__':main()
