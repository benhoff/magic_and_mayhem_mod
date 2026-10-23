#!/usr/bin/env python3
"""Inspect resource stripping in observation refresh and live actor reset/destruction."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess
from coverage_evidence import read_json, evidence_digest, write_report
from original_x86 import PE32, Decoder
ROOT=Path(__file__).resolve().parents[1]
PIN='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
ENTRIES=[0x418b60,0x418d50,0x505f70,0x42a5d0,0x42a6e0,0x42f2e0,0x4181d0,0x407c10,0x413330,0x40f7f0,0x417e60,0x407650,0x58f730,0x58f6f0,0x54da70,0x54dac0,0x42cde0,0x52cb60,0x52cbb0,0x42fb40,0x5060a0,0x52cbe0,0x52cc00,0x52cc70]

def main():
 p=argparse.ArgumentParser(description=__doc__)
 p.add_argument('--output',type=Path,required=True);p.add_argument('--report',type=Path,required=True)
 p.add_argument('--headless',type=Path,required=True);p.add_argument('--project',type=Path,required=True)
 a=p.parse_args();out=a.output.resolve()
 if out.exists() or not out.is_relative_to(ROOT/'working/tests'):p.error('Require new retained output below working/tests')
 out.mkdir(parents=True);(out/'tmp').mkdir();(out/'.retain').write_text('Pinned original task-owned source vector instructions, references, decompilation and manifests.\n')
 names=['tools/inspect-snapshot-resource-refresh.py','tools/ghidra/ExportFontProviders.java','tools/original_x86.py','tools/coverage_evidence.py']
 report=dict(schema=1,success=False,static_only=True,scope=__doc__,sources={n:evidence_digest(ROOT/n,root=ROOT) for n in names},original_sha256=PIN)
 verified=False
 try:
  with (out/'original-before.log').open('x') as f:subprocess.run([ROOT/'tools/original-manifest.sh','verify'],stdout=f,stderr=subprocess.STDOUT,check=True)
  verified=True;image=PE32((ROOT/'original/Arcane_Nocd/Chaos.exe').read_bytes());assert image.sha256==PIN
  register=read_json(ROOT/'research/runtime/coverage/register.json',root=ROOT)
  build=next(b for b in register['builds'] if b['id']=='nocd-40209ca7')
  inv=read_json(ROOT/build['inventory'],root=ROOT);functions={f['entry']:f for f in inv['functions']}
  exceptions=[e for e in register['recovered_ranges'] if e['build']==build['id'] and e['start'] in ENTRIES]
  report['existing_recovered_range_exceptions']=exceptions
  for e in exceptions:
   if e['start'] not in functions:functions[e['start']]=dict(entry=e['start'],ranges=[[e['start'],e['end']]],existing_exception=e['id'])
  report['inventory']=dict(path=build['inventory'],sha256=evidence_digest(ROOT/build['inventory'],root=ROOT))
  report['callback_vtable']={hex(0x5c7264+4*i):hex(image.word(0x5c7264+4*i)) for i in range(2)}
  report['kind6_vtable']={hex(0x5c596c+4*i):hex(image.word(0x5c596c+4*i)) for i in range(7)}
  decoder=Decoder();instructions={};frontier=[]
  try:
   for entry in ENTRIES:
    code=[]
    for first,end in functions[entry]['ranges']:
     while first<end:
      row=decoder.decode(image.read(first,min(15,end-first)),first);code.append(row);first+=row['size']
    instructions[hex(entry)]=code
    for row in code:
     if row['mnemonic']=='call':frontier.append(dict(owner=entry,site=row['address'],operand=row['operand']))
  finally:decoder.close()
  command=[str(a.headless),str(a.project),'MagicMayhem','-process','Chaos.exe','-readOnly','-noanalysis','-scriptPath',str(ROOT/'tools/ghidra'),'-postScript','ExportFontProviders.java',str(out/'c'),*[format(e,'x') for e in ENTRIES],'-max-cpu','2']
  env=dict(os.environ,TMPDIR=str(out/'tmp'),XDG_CONFIG_HOME=str(out/'config'),XDG_CACHE_HOME=str(out/'cache'),GHIDRA_HEADLESS_JAVA_OPTIONS=f'-Dapplication.cachedir={out}/cache -Dapplication.tempdir={out}/tmp -Djava.io.tmpdir={out}/tmp')
  with (out/'ghidra.log').open('x') as f:subprocess.run(command,env=env,stdout=f,stderr=subprocess.STDOUT,check=True,timeout=300)
  texts={q.name:q.read_text() for q in sorted((out/'c').iterdir())}
  missing=[format(e,'08x')+'.c' for e in ENTRIES if format(e,'08x')+'.c' not in texts]
  report['missing_project_decompilations']=missing
  report['older_project_boundary']='Missing decompilations retain central instruction ranges or existing recovered-range exceptions. Read-only project unchanged; no new discovery or boundary promotion.'
  report.update(success=True,instructions=instructions,frontier=frontier,functions=[functions[e] for e in ENTRIES],anchors={hex(e):image.read(e,8).hex() for e in ENTRIES},ghidra_command=command,texts=texts)
 except (OSError,ValueError,AssertionError,subprocess.SubprocessError) as e:report['error']=str(e)
 finally:
  if verified:
   with (out/'original-after.log').open('x') as f:subprocess.run([ROOT/'tools/original-manifest.sh','verify'],stdout=f,stderr=subprocess.STDOUT,check=True)
   report['original_manifest_verified_before_after']=True
  report['sources_stable']=all(evidence_digest(ROOT/n,root=ROOT)==h for n,h in report['sources'].items());report['success'] &= report['sources_stable']
  write_report(ROOT,a.report,(json.dumps(report,indent=2)+'\n').encode())
 print(a.report);return 0 if report['success'] else 1
if __name__=='__main__':raise SystemExit(main())
