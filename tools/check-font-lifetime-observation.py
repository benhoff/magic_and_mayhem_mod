#!/usr/bin/env python3
"""Retain production font observer ABI/reader controls and exact tested artifacts."""
import argparse,hashlib,json,os,subprocess,sys
from pathlib import Path
from coverage_evidence import Reader,write_report
from coverage_claims import behavior_contract,required_sources,scenario_contract
ROOT=Path(__file__).resolve().parents[1];PREFIX='research/runtime/font-dc-lifetime-results-20261108/'
def main():
 p=argparse.ArgumentParser();p.add_argument('--report',required=True);a=p.parse_args()
 if not a.report.startswith(PREFIX)or '..'in Path(a.report).parts or(ROOT/a.report).exists():p.error('Fresh owned SQLite report required')
 out=ROOT/'working/font-dc-controls'/Path(a.report).stem;out.mkdir(parents=True,exist_ok=False);(out/'.retain').write_text('Exact tested observer/fixture binaries, source, controls and failures. Objects are temporary.\n')
 reg=json.loads((ROOT/'research/runtime/coverage/register.json').read_text());b=next(b for b in reg['behaviors']if b['id']=='NP.font-lifetime-observer');s=next(s for s in reg['scenarios']if s['id']=='native.font-lifetime-observer-controls')
 names=required_sources(b)|set(s['tests'])|{str(p.relative_to(ROOT))for p in(ROOT/'runtime/scene').glob('*')if p.is_file()and p.suffix in('.c','.h','.S')}
 sources={n:hashlib.sha256((ROOT/n).read_bytes()).hexdigest()for n in sorted(names)}
 report=dict(schema=1,kind='production_font_lifetime_observation_controls',success=False,sources=sources,commands=[],artifacts=[],original_game_runs=0,claims=[dict(behavior=b['id'],contract_sha256=behavior_contract(b,{v['id']:v for v in reg['builds']}),scenarios={s['id']:scenario_contract(s)})],scope='Authored SFT inputs and production i386 entry/return assembly; RET8/RET0, nonvolatile registers, DF, x87/SSE and LastError. Strict decoder custody/refusal controls. No original game, original function-body equivalence, real installed lifetime or acceptance row credit.')
 def put(path,label=None):
  n=a.report.removesuffix('.json')+'-'+(label or path.relative_to(out).as_posix().replace('/','_'))+'.bin';data=path.read_bytes();h=write_report(ROOT,n,data);report['artifacts'].append(dict(record=n,sha256=h,size=len(data)));return n
 try:
  for n in sources:put(ROOT/n,'source-'+n.replace('/','_'))
  cmds=[('controls',[sys.executable,'-B',str(ROOT/'tests/test-font-lifetime-observation.py'),'-v']),('production-build',[sys.executable,str(ROOT/'tools/build-scene-observer.py')]),('selftest-build',[sys.executable,str(ROOT/'tools/build-scene-observer.py'),'--selftest'])]
  for label,cmd in cmds:
   x=subprocess.run(cmd,cwd=ROOT,capture_output=True,timeout=120,env=dict(os.environ,MNM_FONT_CONTROL_OUTPUT=str(out/'fixture')));log=out/(label+'.log');log.write_bytes(x.stdout+x.stderr);put(log);report['commands'].append(dict(command=cmd,returncode=x.returncode))
   if x.returncode:raise ValueError(label+': '+(x.stdout+x.stderr).decode(errors='replace')[-3000:])
  report.update(success=True,control_cases=17,production_hook_abi_cases=24,production_and_selftest_builds=True)
 except Exception as e:report['error']=str(e)
 finally:
  for p in sorted(out.rglob('*')):
   if p.is_file()and p.name!='.retain':put(p)
  for directory in ['scene-observer','scene-observer-selftest']:
   for p in (ROOT/'working/build'/directory).glob('*'):
    if p.is_file()and p.suffix in('.dll','.exe','.json'):put(p,'tested-'+directory+'-'+p.name)
  report['sources_stable']=all(hashlib.sha256((ROOT/n).read_bytes()).hexdigest()==h for n,h in sources.items());report['success']&=report['sources_stable']
  write_report(ROOT,a.report,(json.dumps(report,indent=2)+'\n').encode())
 print(json.dumps({k:report.get(k)for k in ['success','error','control_cases','production_hook_abi_cases','sources_stable']}),flush=True);return int(not report['success'])
if __name__=='__main__':raise SystemExit(main())
