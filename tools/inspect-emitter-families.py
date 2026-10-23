#!/usr/bin/env python3
"""Retain pinned emitter and manager update instructions, callers and read-only decompilation."""
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
ENTRIES=[5526928, 5527520, 5527728, 5527824, 5531184, 5531216, 5531232, 5531808, 5534224, 5534336, 5534368, 5534384, 5534400, 5534528, 5534560, 5534576, 5534592, 5535888, 5536448, 5536480, 5536592, 5536624, 5539168, 5539328, 5539360, 5539408, 5539824, 5539856, 5539904, 5539936, 5540544, 5540752, 5540784, 5540800, 5545328, 5545520, 5545552, 5546304, 5547504, 5548512, 5548528, 5548592, 5548640, 5548656, 5548736, 5548752, 5548768, 5548832, 5548848, 5548864, 5548928, 5549056, 5549072, 5549136, 5549184, 5549200, 5549296, 5549328, 5549344, 5562880]

def main():
 p=argparse.ArgumentParser(description=__doc__)
 p.add_argument('--output',type=Path,required=True);p.add_argument('--report',type=Path,required=True)
 p.add_argument('--headless',type=Path,required=True);p.add_argument('--project',type=Path,required=True)
 a=p.parse_args();out=a.output.resolve()
 if out.exists() or not out.is_relative_to(ROOT/'working/tests'):p.error('Require new retained output below working/tests')
 out.mkdir(parents=True);(out/'tmp').mkdir();(out/'.retain').write_text('Pinned original support instructions, references, decompilation and manifests.\n')
 names=['tools/inspect-emitter-families.py','tools/ghidra/ExportFontProviders.java','tools/original_x86.py','tools/coverage_evidence.py']
 report=dict(schema=1,success=False,static_only=True,scope=__doc__,sources={n:evidence_digest(ROOT/n,root=ROOT) for n in names},original_sha256=PIN)
 verified=False
 try:
  with (out/'original-before.log').open('x') as f:subprocess.run([ROOT/'tools/original-manifest.sh','verify'],stdout=f,stderr=subprocess.STDOUT,check=True)
  verified=True;image=PE32((ROOT/'original/Arcane_Nocd/Chaos.exe').read_bytes());assert image.sha256==PIN
  register=read_json(ROOT/'research/runtime/coverage/register.json',root=ROOT)
  build=next(b for b in register['builds'] if b['id']=='nocd-40209ca7')
  inv=read_json(ROOT/build['inventory'],root=ROOT);functions={f['entry']:f for f in inv['functions']}
  report['inventory']=dict(path=build['inventory'],sha256=evidence_digest(ROOT/build['inventory'],root=ROOT))
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
  missing_project={e for e in ENTRIES if format(e,'08x')+'.c' not in texts}
  expected={format(e,'08x')+'.c' for e in ENTRIES if e not in missing_project}
  if not expected:raise ValueError('No selected read-only decompilation available')
  report['missing_project_decompilations']=sorted(missing_project)
  report['missing_project_scope']='All selected entries have pinned central ranges and complete decoded instructions. Missing older-project decompilations are recorded explicitly; no inventory/project mutation or execution claim.'
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
