#!/usr/bin/env python3
"""Compare actor service bodies and composed scheduler/audio against pinned PE32."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import subprocess
from original_x86 import PE32, Decoder
from coverage_claims import behavior_contract, required_sources
ROOT=Path(__file__).resolve().parents[1]
PIN='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
BIDS={'values':'RS.actor-downstream-value-services','controllers':'RS.actor-downstream-controller-services','world':'RS.actor-downstream-world-services','ui':'RS.actor-downstream-ui-services','dependencies':'RS.actor-downstream-dependency-services','popups':'RS.actor-downstream-popup-services'}
HEADERS={'values':'reconstruction/rendering/actor_services.hpp','controllers':'reconstruction/rendering/actor_service_controllers.hpp','world':'reconstruction/rendering/actor_service_world.hpp','ui':'reconstruction/rendering/actor_service_ui.hpp','dependencies':'reconstruction/rendering/actor_service_dependencies.hpp','popups':'reconstruction/rendering/actor_service_popups.hpp'}
# id, stack argc, callee-pop bytes, meaningful ECX, meaningful EDX.
PROVIDERS=[(0x58b660,0,0,True,False),(0x57dc20,1,4,True,True),(0x57dc60,2,8,True,True),(0x57de00,5,0,False,False),(0x4d72d0,0,0,True,False),(0x4d53d0,2,8,True,False),(0x58bc10,1,4,True,False),(0x58d1a0,1,4,True,False),(0x4d47f0,0,0,True,False),(0x4a5a90,2,8,True,False),(0x4a5ce0,7,28,True,False),(0x56f000,7,28,True,False),(0x1001f280,2,8,False,False),(0x1001f2c0,3,0,False,False),(0x1001f300,5,0,False,False),(0x4d2e50,0,0,True,False),(0x4d6e90,0,0,True,False),(0x4de6c0,0,0,True,False),(0x597d40,0,0,False,False),(0x598a10,3,12,False,False),(0x597df0,2,0,False,False),(0x597b40,0,0,True,True),(0x1001f1c0,11,44,True,False),(0x1001f200,0,0,True,False),(0x1001f240,0,0,False,False),(0x4f4330,4,16,True,False),(0x58c8a0,4,16,True,False),(0x58d5b0,6,24,True,False),(0x4d3c90,0,0,True,False),(0x49cdb0,0,0,True,False),(0x493f20,2,8,True,False),(0x4d5640,0,0,True,False),(0x4d6620,2,8,True,False),(0x4d9f30,1,4,True,False),(0x59c1fd,4,0,False,False),(0x4644d0,2,8,True,False),(0x5050d0,0,0,True,False),(0x57d310,2,8,True,False),(0x57d690,2,8,True,False),(0x505220,1,4,True,False),(0x5c50c0,4,16,False,False),(0x5c520c,4,16,False,False),(0x1001f140,0,0,True,False),(0x1001f180,0,0,True,False),(0x488330,1,4,True,False),(0x59cae2,2,0,False,False),(0x59d50b,7,0,False,False),(0x59c4c3,1,0,False,False),(0x468470,0,0,True,True),(0x4eb820,1,4,True,False),(0x564e90,6,24,True,False),(0x464350,0,0,True,False),(0x464ec0,0,0,True,False),(0x465000,0,0,True,False),(0x465130,0,0,True,False),
 (0x4d6c60,0,0,True,False),(0x4d39b0,0,0,True,False),(0x4cb9b0,0,0,True,False),(0x4f7970,1,4,True,False),(0x4d7a90,0,0,True,False),
 (0x54dce0,1,4,True,False),(0x50aa30,1,4,True,False),(0x54e200,0,0,True,False),(0x485b10,2,8,True,False),
 (0x50e400,1,4,True,False),(0x513f50,0,0,True,False),(0x50e3d0,2,8,True,False),(0x510480,0,0,True,False),(0x50d060,3,12,True,False),
 (0x4dea00,1,4,True,False),(0x4d7a00,0,0,True,False),(0x4d78b0,0,0,True,False),(0x4d3b20,1,4,True,False),(0x4d6c80,2,8,True,False),(0x4d3b40,0,0,True,False),(0x4667c0,1,4,True,False),(0x469000,2,8,True,True),
 (0x490260,0,0,True,False),(0x4943f0,0,0,True,False),
 (0x4a5000,0,0,True,False),(0x514e40,1,4,True,False),
 (0x5338c0,1,4,True,False),(0x58a680,1,4,True,False),
 (0x5977d0,3,0,False,False),(0x597880,1,0,False,False),
 (0x598f70,1,4,False,False),
 (0x1001f000,2,8,False,False),(0x1001f040,1,4,False,False),
 (0x1001f080,2,8,False,False),(0x1001f100,0,0,True,False)]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--output',type=Path,required=True);p.add_argument('--suite',choices=BIDS,default='values');a=p.parse_args();out=a.output.resolve()
 if out.exists() or not out.is_relative_to(ROOT/'working/tests'):p.error('Require new retained output under working/tests')
 (out/'tmp').mkdir(parents=True);env=dict(os.environ,TMPDIR=str(out/'tmp'))
 register=json.loads((ROOT/'research/runtime/coverage/register.json').read_text());b=next(b for b in register['behaviors'] if b['id']==BIDS[a.suite]);builds={b['id']:b for b in register['builds']}
 roots=sorted({int(x,16) for x in re.findall(r'case (0x[0-9a-f]+):',(ROOT/HEADERS[a.suite]).read_text())})
 sources={s:sha(ROOT/s) for s in sorted(required_sources(b)|{'tools/coverage_claims.py','tools/original_x86.py'})}
 for s in sources:
  target=out/'sources'/s;target.parent.mkdir(parents=True,exist_ok=True);target.write_bytes((ROOT/s).read_bytes())
 report=dict(schema=1,success=False,roots=roots,cases_per_root=64,sources=sources,original_sha256=PIN,claims=[dict(behavior=BIDS[a.suite],contract_sha256=behavior_contract(b,builds))],scope=b['scope'],original_output_used_as_native_input=False,live_replacement=False,physical_driver_equivalence=False)
 (out/'prospective.json').write_text(json.dumps(report,indent=2)+'\n');verified=False
 def run(label,command,stream=False):
  print(label,flush=True)
  with (out/(label+'.log')).open('x') as log:
   if not stream:subprocess.run(list(map(str,command)),cwd=ROOT,env=env,stdout=log,stderr=subprocess.STDOUT,check=True,timeout=360);return
   with (out/(label+'.bin.zst')).open('xb') as target:
    encoder=subprocess.Popen(['zstd','-q','-3','-c'],stdin=subprocess.PIPE,stdout=target,stderr=log)
    child=subprocess.Popen(list(map(str,command)),cwd=ROOT,env=env,stdout=encoder.stdin,stderr=log);encoder.stdin.close()
    try:
     if child.wait(timeout=360) or encoder.wait(timeout=30):raise ValueError(label+' failed; inspect retained log')
    finally:
     for proc in [child,encoder]:
      if proc.poll() is None:proc.kill()
      proc.wait()
 try:
  run('original-before',[ROOT/'tools/original-manifest.sh','verify']);verified=True
  image=PE32((ROOT/'original/Arcane_Nocd/Chaos.exe').read_bytes());assert image.sha256==PIN
  inventory_path=ROOT/builds['nocd-40209ca7']['inventory'];inventory=json.loads(inventory_path.read_text());functions={f['entry']:f for f in inventory['functions']}
  report['inputs']={str(inventory_path.relative_to(ROOT)):sha(inventory_path)};abi={};decoder=Decoder()
  try:
   def resolve_abi(entry,visiting=frozenset()):
    if hex(entry) in abi:return abi[hex(entry)]
    if entry in visiting:raise ValueError('Cyclic tail ABI: '+hex(entry))
    rows=[];f=functions[entry]
    for lo,hi in f['ranges']:
     while lo<hi:
      row=decoder.decode(image.read(lo,min(15,hi-lo)),lo);rows.append(row);lo+=row['size']
    pops={int(i['operand'],0) if i['operand'] else 0 for i in rows if i['mnemonic']=='ret'}
    if not pops:
     for i in rows:
      if i['mnemonic']=='jmp' and i['operand'].startswith('0x'):
       target=int(i['operand'],16)
       if not any(lo<=target<hi for lo,hi in f['ranges']):pops.update(resolve_abi(target,visiting|{entry})['return_pop_bytes'])
    result={'ranges':f['ranges'],'return_pop_bytes':sorted(pops)};abi[hex(entry)]=result;return result
   for entry in roots:resolve_abi(entry)
  finally:decoder.close()
  assert len(roots)=={'values':42,'controllers':10,'world':9,'ui':10,'dependencies':18,'popups':9}[a.suite]
  lines=['#pragma once','#include <cstdint>','struct ServiceAbi{unsigned id,pop;};','static constexpr unsigned ServiceRoots[]={'+','.join(hex(v) for v in roots)+'};','static constexpr ServiceAbi ServiceAbis[]={']
  for entry in roots:
   pops=abi[hex(entry)]['return_pop_bytes'];assert len(pops)==1;lines.append('{'+hex(entry)+','+str(pops[0])+'},')
  
  for suite,header in HEADERS.items():
   supplied=sorted({int(x,16) for x in re.findall(r'case (0x[0-9a-f]+):',(ROOT/header).read_text())});lines+=['};','static constexpr unsigned '+{'values':'ValueRoots','controllers':'ControllerRoots','world':'WorldRoots','ui':'UiRoots','dependencies':'DependencyRoots','popups':'PopupRoots'}[suite]+'[]={'+','.join(hex(v) for v in supplied)]
  lines+=['};','static constexpr bool UseWorldServices='+str(a.suite=='world').lower()+';','static constexpr bool UseUiServices='+str(a.suite=='ui').lower()+';','static constexpr bool UseDependencies='+str(a.suite in {'dependencies','popups'}).lower()+';','static constexpr bool UsePopups='+str(a.suite=='popups').lower()+';','struct ServiceProvider{unsigned id,arguments,pop;bool receiver,second;unsigned char bytes[8];};','static constexpr ServiceProvider ServiceProviders[]={']
  composed=set(roots)
  if a.suite in {'dependencies','popups'}:composed|={int(x,16) for x in re.findall(r'case (0x[0-9a-f]+):',(ROOT/HEADERS['dependencies']).read_text())}
  actual_providers=[]
  for entry,argc,pop,receiver,second in PROVIDERS:
   if entry in composed:continue
   actual_providers.append(entry)
   data=image.read(entry,8) if entry<0x600000 else bytes(8)
   lines.append('{'+','.join([hex(entry),str(argc),str(pop),str(receiver).lower(),str(second).lower(),'{'+','.join(hex(v) for v in data)+'}'])+'},')
  lines.append('};')
  protected=set(roots)
  for header in HEADERS.values():
   protected|={int(x,16) for x in re.findall(r'case (0x[0-9a-f]+):',(ROOT/header).read_text())}
  # All reused bodies stay unchanged unless explicitly outside this suite's
  # scope as an owned provider. Generated exact bytes also protect callbacks.
  protected-=set(actual_providers)
  if a.suite=='controllers':protected|={0x50da50,0x594490}
  if a.suite=='world':protected|={0x40e290,0x40e4e0,0x40e650,0x40e8a0,0x40eb70,0x4132e0,0x4eac10,0x4eaca0,0x4eade0,0x59bee0}
  if a.suite=='ui':protected|={0x40e2c0,0x40e8d0,0x40eec0,0x509790}
  regions=[]
  for entry in sorted(protected):
   for lo,hi in functions[entry]['ranges']:
    index=len(regions);data=image.read(lo,hi-lo);lines.append('static constexpr unsigned char Body'+str(index)+'[]={'+','.join(hex(v) for v in data)+'};');regions.append((lo,len(data),index))
  lines+=['struct ProtectedBody{unsigned address,length;const unsigned char* expected;};','static constexpr ProtectedBody ProtectedBodies[]={']
  lines.extend('{'+','.join([hex(lo),str(size),'Body'+str(index)])+'},' for lo,size,index in regions);lines.append('};')
  report['protected_original_entries']=sorted(protected)
  (out/'actor-services-generated.hpp').write_text('\n'.join(lines)+'\n')
  report['generated_inputs']={'actor-services-generated.hpp':sha(out/'actor-services-generated.hpp')}
  report['original_body_ranges']={hex(v):[{'first':lo,'end':hi,'sha256':hashlib.sha256(image.read(lo,hi-lo)).hexdigest()} for lo,hi in abi[hex(v)]['ranges']] for v in roots}
  for name,flags in [('native',[]),('original',['-m32','-fno-pie','-no-pie','-DMNM_ORIGINAL_REFERENCE'])]:
   run(name+'-build',['g++','-std=c++17','-O2','-Wall','-Wextra','-Werror',*flags,'-I',out,'-MD','-MF',out/(name+'.d'),ROOT/'tests/actor-services-reference.cpp',*([ROOT/'tests/actor-services-call.S'] if name=='original' else []),'-o',out/name])
   for path in (out/(name+'.d')).read_text().replace('\\\n',' ').split()[1:]:
    q=Path(path)
    if q.is_file() and q.resolve().is_relative_to(ROOT) and not q.resolve().is_relative_to(out) and q.resolve().relative_to(ROOT).as_posix() not in sources:raise ValueError('Undeclared compiler input: '+path)
   command=['bwrap','--die-with-parent','--unshare-net','--ro-bind','/','/','--bind',out,out,'--dev','/dev','--proc','/proc','--',out/name]
   if name=='original':command.append(ROOT/'original/Arcane_Nocd/Chaos.exe')
   run(name+'-execute',command,True)
   if 'cases '+str(len(roots)*64) not in (out/(name+'-execute.log')).read_text():raise ValueError('Incomplete matrix: '+name)
  children=[subprocess.Popen(['zstd','-qdc',out/(name+'-execute.bin.zst')],stdout=subprocess.PIPE) for name in ['native','original']];count=0;digest=hashlib.sha256()
  try:
   while True:
    left=children[0].stdout.read(65536);right=children[1].stdout.read(65536)
    if left!=right:
     at=next((i for i,(l,r) in enumerate(zip(left,right)) if l!=r),min(len(left),len(right)));report['mismatch']={'byte':count+at,'native':left[max(0,at-16):at+32].hex(),'original':right[max(0,at-16):at+32].hex()};raise ValueError('Actor service bytes differ')
    if not left:break
    digest.update(left);count+=len(left)
   if any(proc.wait(timeout=10) for proc in children):raise ValueError('Output decompression failed')
  finally:
   for proc in children:
    if proc.poll() is None:proc.kill()
    proc.wait();proc.stdout.close()
  log=(out/'original-execute.log').read_text();snapshot_bytes=int(re.search(r'snapshot_bytes (\d+)',log)[1]);outcomes={}
  proc=subprocess.Popen(['zstd','-qdc',out/'original-execute.bin.zst'],stdout=subprocess.PIPE)
  import struct
  try:
   for expected_entry in roots:
    counts={'returned':0,'access_fault':0,'division_fault':0}
    for variant in range(64):
     entry,seed,result,events,outcome=struct.unpack('<5I',proc.stdout.read(20));assert (entry,seed)==(expected_entry,variant);assert outcome in {0,1,2};assert proc.stdout.read(snapshot_bytes+events*4).__len__()==snapshot_bytes+events*4
     counts[('returned','access_fault','division_fault')[outcome]]+=1
    outcomes[hex(expected_entry)]=counts
   assert not proc.stdout.read(1);assert proc.wait(timeout=10)==0
  finally:
   if proc.poll() is None:proc.kill()
   proc.wait();proc.stdout.close()
  report.update(success=True,outcomes=outcomes,cases=len(roots)*64,bytes_compared=count,comparison_sha256=digest.hexdigest(),selected_original_bodies_unchanged=True,binaries_sha256={name:sha(out/name) for name in ['native','original']},outputs_sha256={name+'-execute.bin.zst':sha(out/(name+'-execute.bin.zst')) for name in ['native','original']})
 except (OSError,ValueError,KeyError,AssertionError,subprocess.SubprocessError) as error:report['error']=type(error).__name__+': '+str(error)
 finally:
  if verified:run('original-after',[ROOT/'tools/original-manifest.sh','verify']);report['original_manifest_verified_before_after']=True
  report['sources_stable']=all(sha(ROOT/s)==h for s,h in sources.items());report['success']&=report['sources_stable'];(out/'report.json').write_text(json.dumps(report,indent=2)+'\n')
 print(out/'report.json');return 0 if report['success'] else 1
if __name__=='__main__':raise SystemExit(main())
