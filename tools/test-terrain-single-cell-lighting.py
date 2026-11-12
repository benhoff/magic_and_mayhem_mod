#!/usr/bin/env python3
"""Size-one kernel and production-capture/decoder comparisons with unchanged PE32."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess
import sys
from coverage_claims import behavior_contract, required_sources
from coverage_evidence import evidence_digest, read_bytes, write_report

ROOT=Path(__file__).resolve().parents[1]
BUILD='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
IDS=['RE.terrain-single-cell-light','RE.terrain-single-cell-captured-cycle']

def main():
 p=argparse.ArgumentParser(description=__doc__)
 p.add_argument('--main-root',type=Path,required=True)
 p.add_argument('--output',type=Path,required=True)
 p.add_argument('--report',type=Path,required=True)
 a=p.parse_args();main_root=a.main_root.resolve();out=a.output.resolve();target=a.report.absolute()
 if out.exists() or target.exists() or not target.is_relative_to(ROOT/'research/runtime'):
  p.error('Require new execution and immutable research report paths')
 out.mkdir(parents=True);(out/'tmp').mkdir()
 env=dict(os.environ,TMPDIR=str(out/'tmp'),ASAN_OPTIONS='detect_leaks=1:halt_on_error=1',UBSAN_OPTIONS='halt_on_error=1')
 register=json.loads((ROOT/'research/runtime/coverage/register.json').read_bytes())
 behaviors=[b for b in register['behaviors'] if b['id'] in IDS];assert len(behaviors)==len(IDS)
 builds={b['id']:b for b in register['builds']}
 paths=set().union(*(required_sources(b) for b in behaviors))
 paths.update(['tools/coverage_claims.py','tools/coverage_evidence.py','tools/original-manifest.sh','research/original-files.tsv'])
 sources={n:evidence_digest(ROOT/n,root=ROOT) for n in sorted(paths)}
 pe=main_root/'original/Arcane_Nocd/Chaos.exe'
 report=dict(schema=1,success=False,sources=sources,
  claims=[dict(behavior=b['id'],contract_sha256=behavior_contract(b,builds)) for b in behaviors],
  original_sha256=BUILD,commands=[],artifacts=[],results={},installed_game_executed=False,
  installed_heap_provenance_recovered=False,natural_size_one_caller_observed=False,
  scope='Unchanged original initializer/index-one maximum/additive stamps plus actual production capture, V1 decode and original outer static-light update/publication. All 256 explicit inherited bytes, authored records and API endpoints; no installed heap provenance, live natural caller, driver or replacement claim.')
 def retain(label,data):
  path=target.parent/(target.stem+'-'+label)
  digest=write_report(ROOT,path,data);assert hashlib.sha256(read_bytes(path,root=ROOT)).hexdigest()==digest
  row=dict(path=path.relative_to(ROOT).as_posix(),sha256=digest,bytes=len(data));report['artifacts'].append(row);return row
 # Retain exact source variants before any compiler/original execution, including
 # failures. SQLite deduplicates content; no expanded research JSON is written.
 variants=[]
 for n in sorted(paths):
  data=read_bytes(ROOT/n,root=ROOT)
  assert hashlib.sha256(data).hexdigest()==sources[n]
  variants.append(dict(source=n,**retain('source-'+hashlib.sha256(n.encode()).hexdigest()[:16]+'.bin',data)))
 retain('source-catalog.json',(json.dumps(variants,indent=2)+'\n').encode())
 def run(label,command,timeout=600):
  print(label,flush=True)
  r=subprocess.run([str(x) for x in command],cwd=ROOT,env=env,capture_output=True,text=True,timeout=timeout)
  log=retain(label+'.log',(r.stdout+r.stderr).encode())
  report['commands'].append(dict(label=label,command=list(map(str,command)),exit_code=r.returncode,log=log))
  if r.returncode:raise RuntimeError(label+' failed: '+(r.stdout+r.stderr)[-3000:])
  return r.stdout
 before=False
 try:
  run('manifest-before',[main_root/'tools/original-manifest.sh','verify']);before=True
  if hashlib.sha256(pe.read_bytes()).hexdigest()!=BUILD:raise ValueError('Unsupported original executable')
  model=ROOT/'reconstruction/rendering'
  common=['g++','-std=c++17','-I'+str(model)]
  san=['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer','-fno-pie','-no-pie']
  variants=[('original',['-O2','-m32','-fno-pie','-no-pie','-DORIGINAL_REFERENCE']),
            ('native32',['-O2','-m32']),('native64',['-O2']),('sanitized32',['-m32',*san]),('sanitized64',san)]
  rows=[]
  for name,flags in variants:
   binary=out/('kernel-'+name);data=out/('kernel-'+name+'.bin')
   run('kernel-'+name+'-build',[*common,*flags,ROOT/'tests/terrain-single-cell-lighting-reference.cpp',model/'terrain_lighting.cpp','-o',binary])
   stats=json.loads(run('kernel-'+name,[binary,*([pe] if name=='original' else []),data]))
   rows.append(dict(variant=name,stats=stats,result=retain('kernel-'+name+'.bin',data.read_bytes()),binary=retain('kernel-'+name+'-executable.bin',binary.read_bytes())))
  if any((r['stats'],r['result']['sha256'])!=(rows[0]['stats'],rows[0]['result']['sha256']) for r in rows):raise ValueError('Kernel/stamp comparison differs')
  report['results']['kernel']=dict(all_match=True,variants=rows)
  capture=out/'capture.o';oracle=out/'capture-original';bundle=out/'capture-original.bin'
  run('capture-build',['gcc','-std=c11','-O2','-m32','-ffunction-sections','-fdata-sections','-c',ROOT/'tests/terrain-single-cell-capture.c','-o',capture])
  run('oracle-build',[*common,'-O2','-m32','-fno-pie','-no-pie','-Wl,--gc-sections',ROOT/'tests/terrain-single-cell-capture-reference.cpp',capture,'-o',oracle])
  stats=json.loads(run('oracle',[oracle,pe,bundle]))
  assert stats['requests']==8192 and stats['inherited_bytes']==256 and stats['initializations']==2048
  report['results']['capture']=dict(stats=stats,bundle=retain('capture-original.bin',bundle.read_bytes()),binary=retain('capture-original-executable.bin',oracle.read_bytes()))
  rows=[]
  models=[model/n for n in ['terrain_lighting.cpp','terrain_creature_lighting.cpp','terrain_static_lighting.cpp','terrain_lighting_cycle.cpp']]
  for name,flags in variants[1:]:
   binary=out/('wire-'+name);data=out/('wire-'+name+'.bin')
   run('wire-'+name+'-build',[*common,*flags,ROOT/'tests/terrain-single-cell-wire-replay.cpp',*models,'-o',binary])
   stats=json.loads(run('wire-'+name,[binary,bundle,data]))
   assert stats['requests']==8192 and stats['inherited_bytes']==256 and stats['refusals']==6
   rows.append(dict(variant=name,stats=stats,result=retain('wire-'+name+'.bin',data.read_bytes()),binary=retain('wire-'+name+'-executable.bin',binary.read_bytes())))
  if any((r['stats'],r['result']['sha256'])!=(rows[0]['stats'],rows[0]['result']['sha256']) for r in rows):raise ValueError('Native decoder variants differ')
  report['results']['wire']=dict(all_match=True,variants=rows)
  report['success']=True
 except Exception as exc:
  report['error']=str(exc)
 finally:
  # Retain partial execution artifacts as negative evidence before RAM staging
  # disappears. Duplicate payloads become references to the same SQLite object.
  retained={row['sha256'] for row in report['artifacts']}
  for f in sorted(out.iterdir()):
   if f.is_file() and hashlib.sha256(f.read_bytes()).hexdigest() not in retained:
    retain('partial-'+f.name+'.bin',f.read_bytes())
  try:
   run('manifest-after',[main_root/'tools/original-manifest.sh','verify'])
   report['original_manifest_verified_before_after']=before
  except Exception as exc:report['success']=False;report['manifest_error']=str(exc)
  report['sources_stable']=all(evidence_digest(ROOT/n,root=ROOT)==h for n,h in sources.items())
  report['original_input_stable']=pe.is_file() and hashlib.sha256(pe.read_bytes()).hexdigest()==BUILD
  report['success'] &= report['sources_stable'] and report['original_input_stable']
  digest=write_report(ROOT,target,(json.dumps(report,indent=2)+'\n').encode())
  assert hashlib.sha256(read_bytes(target,root=ROOT)).hexdigest()==digest
  print(json.dumps(dict(success=report['success'],report=str(target),sha256=digest,
      results={k:(v.get('stats') or v['variants'][0]['stats']) for k,v in report['results'].items()},error=report.get('error'))),flush=True)
 return 0 if report['success'] else 1

if __name__=='__main__':raise SystemExit(main())
