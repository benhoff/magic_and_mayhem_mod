#!/usr/bin/env python3
"""Retain pinned actor service instructions, ABI returns and explicit call frontiers."""
import argparse
import hashlib
import json
from pathlib import Path
import re
import subprocess
from original_x86 import Decoder, PE32
from coverage_evidence import read_json, evidence_digest
ROOT=Path(__file__).resolve().parents[1]
PIN='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--output',type=Path,required=True);p.add_argument('--extra-entries',nargs='*',type=lambda value:int(value,0),default=[]);a=p.parse_args();out=a.output.resolve()
 if out.exists() or not out.is_relative_to(ROOT/'working/tests'):p.error('Require new retained output under working/tests')
 out.mkdir(parents=True)
 paths=['tools/inspect-actor-downstream.py','tools/original_x86.py','tools/coverage_evidence.py','tests/actor-lifecycle-providers.hpp','tests/actor-lifecycle-services.inc']
 sources={n:sha(ROOT/n) for n in paths};r=json.loads((ROOT/'research/runtime/coverage/register.json').read_text());build=next(b for b in r['builds'] if b['id']=='nocd-40209ca7');ip=ROOT/build['inventory'];inv=read_json(ip,root=ROOT);fs={f['entry']:f for f in inv['functions']}
 roots={int(v,16) for v in re.findall(r'\{(0x[0-9a-f]+),',(ROOT/'tests/actor-lifecycle-providers.hpp').read_text())}
 roots|={int(v,16) for v in re.findall(r'ACTOR_SERVICE\(([0-9a-f]+),',(ROOT/'tests/actor-lifecycle-services.inc').read_text())};roots|={0x597850,0x597870,0x5979b0,0x59c441,0x411e50,0x506080};roots={v for v in roots if v<0x5c0000}
 report=dict(schema=1,success=False,static_only=True,scope=__doc__,sources=sources,original_sha256=PIN,inputs={str(ip.relative_to(ROOT)):evidence_digest(ip,root=ROOT)},semantic_or_transitive_closure_proved=False)
 (out/'prospective.json').write_text(json.dumps(report,indent=2)+'\n');verified=False;decoder=None
 try:
  with (out/'original-before.log').open('x') as log:subprocess.run([ROOT/'tools/original-manifest.sh','verify'],stdout=log,stderr=subprocess.STDOUT,check=True)
  verified=True;image=PE32((ROOT/'original/Arcane_Nocd/Chaos.exe').read_bytes());assert image.sha256==PIN;decoder=Decoder()
  def decode(entry):
   f=fs[entry];rows=[]
   for first,end in f['ranges']:
    at=first
    while at<end:
     row=decoder.decode(image.read(at,min(15,end-at)),at);rows.append(row);at+=row['size']
   return rows
  instructions={};helpers=set();frontier=[]
  for entry in sorted(roots|{0x506290,0x51f5e0}|set(a.extra_entries)):
   code=decode(entry);instructions[hex(entry)]=code
   for i in code:
    if i['mnemonic']=='call' or (i['mnemonic']=='jmp' and i['operand'].startswith('0x') and not any(lo<=int(i['operand'],16)<hi for lo,hi in fs[entry]['ranges'])):
     if i['operand'].startswith('0x'):
      target=int(i['operand'],16);helpers.add(target);frontier.append({'owner':entry,'site':i['address'],'target':target,'selected_root':target in roots,'transfer':i['mnemonic']})
     else:frontier.append({'owner':entry,'site':i['address'],'unresolved_indirect':i['operand']})
  abi={}
  for entry in sorted(roots|helpers):
   if entry not in fs:abi[hex(entry)]={'unresolved_entry':True};continue
   code=instructions.get(hex(entry)) or decode(entry)
   abi[hex(entry)]={'entry_bytes':image.read(entry,8).hex(),'ranges':fs[entry]['ranges'],'return_pop_bytes':sorted({int(i['operand'],0) if i['operand'] else 0 for i in code if i['mnemonic'] in {'ret','retf'}}),'contains_seh_setup':any('fs:' in i['operand'] for i in code)}
  for name,obj in [('instructions.json',instructions),('provider-abi.json',abi),('call-frontier.json',frontier)]:
   (out/name).write_text(json.dumps(obj,indent=2)+'\n')
  (out/'instructions.txt').write_text('\n'.join(f"{i['address']:08x} {i['bytes']:30} {i['mnemonic']:8} {i['operand']}" for rows in instructions.values() for i in rows)+'\n')
  report.update(success=True,roots=sorted(roots),extra_entries=sorted(set(a.extra_entries)),selected_original_functions=len(instructions),direct_frontier_entries=len(helpers-roots),unresolved_indirect_sites=sum('unresolved_indirect' in x for x in frontier),decoder_version=decoder.version,artifacts={n:sha(out/n) for n in ['instructions.json','instructions.txt','provider-abi.json','call-frontier.json']})
 except (OSError,ValueError,KeyError,AssertionError,subprocess.SubprocessError) as error:report['error']=str(error)
 finally:
  if decoder:decoder.close()
  if verified:
   with (out/'original-after.log').open('x') as log:subprocess.run([ROOT/'tools/original-manifest.sh','verify'],stdout=log,stderr=subprocess.STDOUT,check=True)
   report['original_manifest_verified_before_after']=True
  report['sources_stable']=all(sha(ROOT/n)==h for n,h in sources.items());report['success']&=report['sources_stable'];(out/'report.json').write_text(json.dumps(report,indent=2)+'\n')
 print(out/'report.json');return 0 if report['success'] else 1
if __name__=='__main__':raise SystemExit(main())
