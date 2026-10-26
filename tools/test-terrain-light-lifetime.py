#!/usr/bin/env python3
"""Inject native allocation failures and validate owned lighting restore."""
import argparse,hashlib,json,os,subprocess
from pathlib import Path
from coverage_claims import behavior_contract,required_sources
from coverage_evidence import write_report
ROOT=Path(__file__).resolve().parents[1]
def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def main():
 p=argparse.ArgumentParser();p.add_argument('--output',type=Path,required=True);p.add_argument('--report',type=Path,required=True);a=p.parse_args()
 out=a.output.absolute();target=a.report.absolute()
 if out.exists() or target.exists() or not out.is_relative_to(ROOT/'working') or not target.is_relative_to(ROOT/'research/runtime'):p.error('Require new task-owned working output and research report')
 out.mkdir(parents=True);(out/'.retain').write_text('Native resource lifetime evidence.\n');(out/'tmp').mkdir()
 reg=json.loads((ROOT/'research/runtime/coverage/register.json').read_text());b=next(b for b in reg['behaviors'] if b['id']=='TL.owned-light-resources')
 paths=required_sources(b)|set(b['documents'])|{'tools/coverage_claims.py','tools/coverage_evidence.py'};sources={n:sha(ROOT/n) for n in sorted(paths)}
 report=dict(schema=1,success=False,sources=sources,claims=[dict(behavior=b['id'],contract_sha256=behavior_contract(b,{x['id']:x for x in reg['builds']}),scenarios={})],results=[])
 env=dict(os.environ,TMPDIR=str(out/'tmp'),ASAN_OPTIONS='detect_leaks=1:halt_on_error=1',UBSAN_OPTIONS='halt_on_error=1')
 try:
  for variant,flags in [('native',['-O2']),('sanitized',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer','-fno-pie','-no-pie'])]:
   units=[ROOT/'tests/terrain-light-lifetime-test.cpp',*[ROOT/'reconstruction/rendering'/('terrain_'+n+'.cpp') for n in ['lighting','creature_lighting','static_lighting','lighting_cycle']]]
   binary=out/variant;cmd=['g++','-std=c++17',*flags,'-I'+str(ROOT/'reconstruction/rendering'),*map(str,units),'-o',str(binary)]
   build=subprocess.run(cmd,cwd=ROOT,env=env,capture_output=True,text=True);(out/(variant+'-build.log')).write_text(build.stdout+build.stderr);build.check_returncode()
   result=subprocess.run([str(binary)],cwd=ROOT,env=env,capture_output=True,text=True);(out/(variant+'.log')).write_text(result.stdout+result.stderr);result.check_returncode()
   report['results'].append(dict(variant=variant,stats=json.loads(result.stdout),binary_sha256=sha(binary),log_sha256=sha(out/(variant+'.log'))))
  assert report['results'][0]['stats']==report['results'][1]['stats'];report['success']=True
 except Exception as exc:report['error']=str(exc)
 finally:
  report['sources_stable']=all(sha(ROOT/n)==h for n,h in sources.items());report['success'] &= report['sources_stable'];h=write_report(ROOT,target,(json.dumps(report,indent=2)+'\n').encode());print(json.dumps(dict(success=report['success'],sha256=h,results=report['results'],error=report.get('error'))),flush=True)
 return 0 if report['success'] else 1
if __name__=='__main__':raise SystemExit(main())
