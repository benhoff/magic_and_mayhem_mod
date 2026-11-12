#!/usr/bin/env python3
"""Retain a fresh original/native glyph comparison using the exact prior corpus."""
import hashlib,importlib.util,io,json,subprocess,sys,tarfile
from pathlib import Path
from coverage_claims import behavior_contract,scenario_contract,required_sources
from coverage_evidence import Reader,write_report
ROOT=Path(__file__).resolve().parents[1]
PREFIX='research/runtime/caller-state-results-20261108/glyph-full-state-01'
def main():
 sha=lambda b:hashlib.sha256(b).hexdigest()
 with Reader(ROOT)as r:
  reg=r.json('research/runtime/coverage/register.json');b=next(b for b in reg['behaviors']if b['id']=='RS.glyph-backend');builds={b['id']:b for b in reg['builds']}
  scenarios=[s for s in reg['scenarios']if b['id']in s['behaviors']and s['level']=='headless']
  sources={n:r.digest(n)for n in required_sources(b)|{'tools/test-rendering-glyph-caller-state.py','tools/test-font-glyph-raster.py'}}
 declaration=dict(sources=sources,claims=[dict(behavior=b['id'],contract_sha256=behavior_contract(b,builds),scenarios={s['id']:scenario_contract(s)for s in scenarios})])
 tmp=ROOT/'working/caller-state-20261108/glyph-inputs';claims=tmp/'claims.json';claims.write_text(json.dumps(declaration,indent=2)+'\n')
 preimages={}
 for n,h in sources.items():
  rec=PREFIX+'-source-'+sha(n.encode())[:16]+'.bin';raw=(ROOT/n).read_bytes();assert sha(raw)==h;write_report(ROOT,rec,raw);preimages[n]=dict(record=rec,sha256=h)
 for n,h in [('cases.bin','512ce3e354f91c2d80434fec7825513ed07d3223f8b5c072f6bf63a0d1f92a32'),('cases.json','55e76ca7326ec855f94c05cd5b6dcc8769581f695f5f462453f8de5028c6ad9c')]:
  raw=(tmp/n).read_bytes();assert sha(raw)==h;write_report(ROOT,PREFIX+'-input-'+n+'.bin',raw)
 write_report(ROOT,PREFIX+'-prospective-claims.bin',claims.read_bytes())
 parent=ROOT/'working/tests/glyph-backend';parent.mkdir(parents=True,exist_ok=True);before=set(parent.iterdir())
 original_run=subprocess.run;executables={}
 def run(argv,*args,**kwargs):
  p=Path(str(argv[0]))
  if p.is_file()and parent in p.parents:
   raw=p.read_bytes();rec=PREFIX+'-tested-'+p.name+'.bin';h=sha(raw)
   if p.name not in executables:write_report(ROOT,rec,raw);executables[p.name]=dict(record=rec,sha256=h)
   else:assert executables[p.name]['sha256']==h
  return original_run(argv,*args,**kwargs)
 spec=importlib.util.spec_from_file_location('legacy_glyph_comparison',ROOT/'tools/test-glyph-backend.py');legacy=importlib.util.module_from_spec(spec);spec.loader.exec_module(legacy)
 sys.argv=[str(ROOT/'tools/test-glyph-backend.py'),'--producer-entry','--claims',str(claims),'--corpus',str(tmp/'cases.bin'),'--index',str(tmp/'cases.json')]
 subprocess.run=run
 try:code=legacy.main()
 finally:subprocess.run=original_run
 created=set(parent.iterdir())-before;assert len(created)==1;out=created.pop();report=json.loads((out/'report.json').read_text())
 buf=io.BytesIO()
 with tarfile.open(fileobj=buf,mode='w:gz')as tar:
  for p in sorted(out.iterdir()):
   if p.is_file():tar.add(p,arcname=p.name,recursive=False)
 raw=buf.getvalue();rec=PREFIX+'-outputs.bin';write_report(ROOT,rec,raw)
 report.update(kind='original_native_glyph_caller_state',source_preimages=preimages,tested_binaries=executables,output_archive=dict(record=rec,sha256=sha(raw),bytes=len(raw)),original_game_runs=0,retained_input_hashes_verified=True)
 write_report(ROOT,PREFIX+'.json',(json.dumps(report,indent=2)+'\n').encode());print(json.dumps(dict(success=report['success'],case_count=report.get('case_count'),executions=report.get('execution_count'),error=report.get('error'))),flush=True)
 return code
if __name__=='__main__':raise SystemExit(main())
