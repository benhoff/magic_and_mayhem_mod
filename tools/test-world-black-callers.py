#!/usr/bin/env python3
"""Pinned black-call continuations and the complete production signature guard."""
import argparse,hashlib,io,json,subprocess,tarfile
from pathlib import Path
from coverage_claims import behavior_contract,required_sources
from coverage_evidence import write_report
ROOT=Path(__file__).resolve().parents[1]
BUILD='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def main():
 p=argparse.ArgumentParser(description=__doc__)
 for name in ['output','report']:p.add_argument('--'+name,type=Path,required=True)
 a=p.parse_args();out=a.output.resolve()
 if out.exists() or a.report.exists() or not out.is_relative_to(ROOT/'working'):p.error('Fresh owned output/report required')
 out.mkdir(parents=True);(out/'.retain').write_text('Original black continuation controls and exact sources; preserve.\n')
 r=json.loads((ROOT/'research/runtime/coverage/register.json').read_text());b=next(b for b in r['behaviors'] if b['id']=='NP.world-black-caller-continuations')
 paths=required_sources(b)
 for d in ['runtime/scene','runtime/shadow','protocols/include']:
  paths.update(str(f.relative_to(ROOT)) for f in (ROOT/d).rglob('*') if f.is_file() and f.suffix in ['.c','.h','.S'])
 sources={n:sha(ROOT/n) for n in sorted(paths)}
 report=dict(schema=1,success=False,sources=sources,claims=[dict(behavior=b['id'],contract_sha256=behavior_contract(b,{v['id']:v for v in r['builds']}),scenarios={})],original_sha256=BUILD,original_game_executed=False,scope=__doc__,commands=[])
 def run(cmd,timeout=600):
  x=subprocess.run(list(map(str,cmd)),cwd=ROOT,capture_output=True,text=True,timeout=timeout)
  report['commands'].append(dict(command=list(map(str,cmd)),exit_code=x.returncode,stdout=x.stdout,stderr=x.stderr))
  if x.returncode:raise ValueError(x.stderr[-4000:] or x.stdout[-4000:] or 'Control failed')
  return x.stdout
 try:
  run([ROOT/'tools/original-manifest.sh','verify'])
  exe=ROOT/'original/Arcane_Nocd/Chaos.exe'
  if sha(exe)!=BUILD:raise ValueError('Original hash changed')
  binary=out/'black-original'
  run(['g++','-m32','-std=c++17','-O2','-Wall','-Wextra','-Werror','-fno-pie','-no-pie',ROOT/'tests/world-black-continuation-original.cpp','-o',binary])
  report['original_fragments']=json.loads(run([binary,exe]))
  if report['original_fragments']['cases']!=393216:raise ValueError('Incomplete AX domain')
  run(['python3',ROOT/'tools/test-world-raster-callers.py','--output',out/'caller-controls'])
  report['caller_controls']=json.loads((out/'caller-controls/report.json').read_text())
  if not report['caller_controls']['success'] or report['caller_controls']['callers']!=47 or report['caller_controls']['admitted_pairs']!=54:raise ValueError('Incomplete production guard controls')
  # Exercise both production and regenerated ordinary tables through the actual
  # primary-sprite wrapper, retaining its owner/class/guard-page refusals.
  source=(ROOT/'runtime/scene/world_producer_bypass.c').read_text();start=source.index('static int raster_caller(');stop=source.index('static u32 bypass_hash',start);batch=source.index('static int raster_batch_caller(')
  (out/'sprite-batch-caller.inc').write_text(source[start:stop]+source[batch:])
  source=(ROOT/'runtime/scene/world_trace.c').read_text();start=source.index('static int kind8_supported(');stop=source.index('void world_begin(',start);(out/'sprite-observer-caller.inc').write_text(source[start:stop])
  (out/'generated.h').write_bytes((out/'caller-controls/generated.h').read_bytes())
  report['sprite_guard_controls']=[]
  for mode in ['production','generated']:
   binary=out/(mode+'-guard');extra=[] if mode=='production' else ['-DMNM_SPRITE_CALLER_HEADER="generated.h"','-DMNM_EXPECT_CALLERS=16','-DMNM_EXPECT_PAIRS=16']
   run(['g++','-m32','-std=c++17','-O2','-Wall','-Wextra','-Werror','-fno-pie','-no-pie','-I'+str(out),*extra,ROOT/'tests/kind8-sprite-admission-test.cpp','-o',binary])
   report['sprite_guard_controls'].append(json.loads(run([binary,exe])))
  report['success']=all(v['success'] and v['protected_short_spans']==628 for v in report['sprite_guard_controls'])
 except Exception as e:report['error']=str(e)
 finally:
  try:run([ROOT/'tools/original-manifest.sh','verify']);report['original_verified_after']=True
  except Exception as e:report.update(success=False,original_manifest_error=str(e))
  report['sources_stable']=all(sha(ROOT/n)==h for n,h in sources.items());report['success'] &= report['sources_stable']
  raw=io.BytesIO()
  with tarfile.open(fileobj=raw,mode='w:gz') as tar:
   for name in sources:tar.add(ROOT/name,arcname='sources/'+name,recursive=False)
   for f in out.rglob('*'):
    if f.is_file():tar.add(f,arcname='outputs/'+str(f.relative_to(out)),recursive=False)
  rawpath=a.report.with_name(a.report.stem+'-raw.tar.gz');write_report(ROOT,rawpath,raw.getvalue());report['raw_record']=str(rawpath)
  write_report(ROOT,a.report,(json.dumps(report,indent=2)+'\n').encode())
 print(json.dumps({k:report.get(k) for k in ['success','error','original_fragments','sources_stable']}),flush=True)
 return int(not report['success'])
if __name__=='__main__':raise SystemExit(main())
