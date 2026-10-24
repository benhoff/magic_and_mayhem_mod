#!/usr/bin/env python3
"""Compare independent borrowed canvas composition with the original window."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shlex
import subprocess
from coverage_claims import behavior_contract,required_sources
from coverage_evidence import evidence_digest,read_json,write_report
ROOT=Path(__file__).resolve().parents[1]
BUILD='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
IDS=['RS.alternate-canvas-dispatch-composition','NP.alternate-canvas-dispatch-admission']
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--output',type=Path,required=True);p.add_argument('--report',required=True);a=p.parse_args();out=a.output.resolve()
 if out.exists() or not out.is_relative_to(ROOT/'working/tests'):p.error('Require new owned output')
 out.mkdir(parents=True);(out/'.retain').write_text('Independent native and unchanged original dispatcher-window comparisons.\n')
 j=read_json(ROOT/'research/runtime/coverage/register.json',root=ROOT);bs={b['id']:b for b in j['behaviors']};b=bs[IDS[0]]
 sources={n:evidence_digest(ROOT/n,root=ROOT) for n in sorted(set().union(*(required_sources(bs[i]) for i in IDS)))}
 r=dict(schema=1,success=False,sources=sources,claims=[dict(behavior=i,contract_sha256=behavior_contract(bs[i],{v['id']:v for v in j['builds']})) for i in IDS],original_sha256=BUILD,commands=[],original_pixels_used_as_native_inputs=False,original_game_executed=False,live_replacement=False,allocation_recovered=False,scope=b['scope'])
 env=os.environ|dict(ASAN_OPTIONS='detect_leaks=1:halt_on_error=1',UBSAN_OPTIONS='halt_on_error=1:print_stacktrace=1')
 def run(label,command):
  print(label,flush=True);log=out/(label+'.log')
  with log.open('x') as f:result=subprocess.run(list(map(str,command)),env=env,cwd=ROOT,stdout=f,stderr=subprocess.STDOUT,timeout=240)
  text=log.read_text();r['commands'].append(dict(label=label,argv=list(map(str,command)),exit=result.returncode,log_sha256=hashlib.sha256(log.read_bytes()).hexdigest()))
  if result.returncode:raise ValueError(label+': '+text[-3000:])
  return text
 try:
  run('manifest-before',[ROOT/'tools/original-manifest.sh','verify']);image=ROOT/'original/Arcane_Nocd/Chaos.exe'
  if hashlib.sha256(image.read_bytes()).hexdigest()!=BUILD:raise ValueError('Original hash')
  for name,flags in [('reference',['-m32','-O2','-fno-pie','-no-pie','-DMNM_ORIGINAL_REFERENCE']),('sanitized',['-m32','-O1','-fno-pie','-no-pie','-DMNM_ORIGINAL_REFERENCE','-fsanitize=address,undefined','-fno-omit-frame-pointer']),('native64',['-O1','-fno-pie','-no-pie','-fsanitize=address,undefined','-fno-omit-frame-pointer'])]:
   run(name+'-build',['g++','-std=c++17','-Wall','-Wextra','-Werror',*flags,'-MMD','-MF',out/(name+'.d'),ROOT/'tests/alternate-canvas-dispatch-reference.cpp','-o',out/name])
   for token in shlex.split((out/(name+'.d')).read_text().replace('\\\n',' ').split(':',1)[1]):
    path=Path(token).resolve()
    if path.is_relative_to(ROOT) and path.relative_to(ROOT).as_posix() not in sources:raise ValueError('Undeclared compiler source '+token)
   r[name]=json.loads(run(name,[out/name,*([image] if name!='native64' else [])]));r[name]['binary_sha256']=hashlib.sha256((out/name).read_bytes()).hexdigest()
  for name in ['sanitized','native64']:
   for field in ['cases','owned_words_checked','atomic_refusals','checksum']:
    if r[name][field]!=r['reference'][field]:raise ValueError(name+' differs: '+field)
  r['success']=True
 except (OSError,ValueError,subprocess.SubprocessError) as error:r['error']=str(error)
 finally:
  try:run('manifest-after',[ROOT/'tools/original-manifest.sh','verify']);r['original_manifest_verified_before_after']=True
  except (OSError,ValueError,subprocess.SubprocessError) as error:r.update(success=False,manifest_error=str(error))
  r['sources_stable']=all(evidence_digest(ROOT/n,root=ROOT)==h for n,h in sources.items());r['success']&=r['sources_stable']
  sha=write_report(ROOT,a.report,(json.dumps(r,indent=2)+'\n').encode());print(json.dumps(dict(success=r['success'],sha256=sha,error=r.get('error'))),flush=True)
 return 0 if r['success'] else 1
if __name__=='__main__':raise SystemExit(main())
